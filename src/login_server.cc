#include "login_server.hh"

#include "common.hh"
#include "crypto.hh"
#include "packet.hh"
#include "server.hh"

enum LoginState : u32 {
	LOGIN_STATE_READING = 0,
	//TODO: LOGIN_STATE_WAITING_DATABASE,
	LOGIN_STATE_WRITING,
	LOGIN_STATE_WAITING_WRITE,
	LOGIN_STATE_DISCONNECTING,
};

struct Login{
	LoginState state;
	u32 connection;
	u32 xtea[4];
	char accname[32];
	char password[32];

	u8 writebuf[512];
	i32 writelen;
};

struct LoginServer{
	// NOTE: Because the login server doesn't have to
	// handle a queue system, the connection_index has
	// a one-to-one relation with the logins array.

	i32 max_logins;
	Login *logins;
	RSA *rsa;
	Server *server;

	i32 max_connections_closing;
	i32 num_connections_closing;
	u32 *connections_closing;
};

static
Login *get_connection_login(LoginServer *lserver, u32 connection){
	u32 index = connection_index(connection);
	ASSERT(index < (u32)lserver->max_logins);
	return &lserver->logins[index];
}

static
OutPacket packet_prepare(Login *login){
	OutPacket result;
	result.next = NULL;
	result.buf = login->writebuf;
	result.bufend = sizeof(login->writebuf);
	// reserve 8 bytes for the header
	result.bufpos = 8;
	return result;
}

static
bool packet_wrap(OutPacket *p, u32 *xtea){
	u8 *buf = packet_buf(p);
	i32 len = packet_written_len(p);

	i32 payload_len = len - 8;
	if(payload_len <= 0) // PARANOID
		PANIC("trying to send empty message");

	// xtea encode
	u8 *xtea_payload = buf + 6;
	i32 xtea_payload_len = len - 6;
	i32 padding = -xtea_payload_len & 7;
	// NOTE: If padding ends up being zero, packet_can_write(p, 0)
	// is the same as packet_ok(p).
	if(!packet_can_write(p, padding))
		return false;
	buffer_write_u16_le(xtea_payload, payload_len);
	xtea_payload_len += padding;
	while(padding-- > 0)
		packet_write_u8(p, 0x33);
	xtea_encode(xtea, xtea_payload, xtea_payload_len);

	// add message length and checksum
	u32 checksum = adler32(xtea_payload, xtea_payload_len);
	buffer_write_u16_le(buf, xtea_payload_len + 4);
	buffer_write_u32_le(buf + 2, checksum);
	return true;
}


static
void disconnect(LoginServer *lserver, Login *login){
	if(login->state == LOGIN_STATE_DISCONNECTING)
		return;

	i32 i = lserver->num_connections_closing;
	if(i < lserver->max_connections_closing){
		lserver->connections_closing[i] = login->connection;
		lserver->num_connections_closing += 1;
	}
	login->state = LOGIN_STATE_DISCONNECTING;
}

static
void send_disconnect(LoginServer *lserver, Login *login, const char *message){
	OutPacket p = packet_prepare(login);
	packet_write_u8(&p, 0x0A);
	packet_write_str(&p, message);

	if(packet_wrap(&p, login->xtea)){
		login->writelen = packet_written_len(&p);
		login->state = LOGIN_STATE_WRITING;
	}else{
		disconnect(lserver, login);
	}
}

static
void send_charlist(LoginServer *lserver, Login *login){
	OutPacket p = packet_prepare(login);
	// motd
	packet_write_u8(&p, 0x14);
	packet_write_str(&p, "1\nKaplar!");
	// charlist
	packet_write_u8(&p, 0x64);
	packet_write_u8(&p, 1); // num_characters
		packet_write_str(&p, "Player");	// player_name
		packet_write_str(&p, "World");	// world_name
		packet_write_u32(&p, 16777343); // game_server_addr
		packet_write_u16(&p, 7172);		// game_server_port
	packet_write_u16(&p, 1); // premium_days

	if(packet_wrap(&p, login->xtea)){
		login->writelen = packet_written_len(&p);
		login->state = LOGIN_STATE_WRITING;
	}else{
		disconnect(lserver, login);
	}
}

static
void login_server_on_accept(void *userdata, u32 connection){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = get_connection_login(lserver, connection);
	login->state = LOGIN_STATE_READING;
	login->connection = connection;
}

static
void login_server_on_drop(void *userdata, u32 connection){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = get_connection_login(lserver, connection);
	// NOTE: Zero out login memory because it may contain sensible information.
	memset(login, 0, sizeof(Login));
}

static
void login_server_on_read(void *userdata, u32 connection, u8 *data, i32 datalen){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	RSA *rsa = lserver->rsa;
	Login *login = get_connection_login(lserver, connection);

	if(login->state != LOGIN_STATE_READING){
		LOG_ERROR("unexpected message");
		disconnect(lserver, login);
		return;
	}

	if(datalen != 149){
		LOG_ERROR("invalid login message length"
			" (expected = 149, got = %d)", datalen);
		disconnect(lserver, login);
		return;
	}

	u32 checksum = adler32(data + 4, datalen - 4);
	if(buffer_read_u32_le(data) != checksum){
		LOG_ERROR("invalid login message checksum");
		disconnect(lserver, login);
		return;
	}

	if(buffer_read_u8(data + 4) != 0x01){
		LOG_ERROR("invalid login message id");
		disconnect(lserver, login);
		return;
	}

	//buffer_read_u16_le(data + 5);		// os
	u16 version = buffer_read_u16_le(data + 7);

	// .DAT .SPR .PIC SIGNATURES (?)
	//buffer_read_u32_le(data +  9);
	//buffer_read_u32_le(data + 13);
	//buffer_read_u32_le(data + 17);


	// NOTE: RSA DECODE the remaining 128 bytes. This can only fail if
	// the result contains more than 128 bytes which is impossible if
	// the key has 1024 bits but is always good to calm your paranoid
	// side and in case you derived a bad key unknowingly.
	u8 *decoded = data + 21;
	usize decoded_len = 128;
	if(!rsa_decode(rsa, decoded, &decoded_len, 128)){
		PANIC("RSA DECODE step produced a bad result. This is only"
			" possible if the RSA key has more than 1024 bits.");
	}
	if(decoded_len != 127){
		LOG_ERROR("RSA DECODE invalid message");
		disconnect(lserver, login);
		return;
	}

	InPacket p = in_packet(decoded, 127);
	login->xtea[0] = packet_read_u32(&p);
	login->xtea[1] = packet_read_u32(&p);
	login->xtea[2] = packet_read_u32(&p);
	login->xtea[3] = packet_read_u32(&p);

	if(version != 860){
		send_disconnect(lserver, login,
			"This server requires client version 8.60.");
		return;
	}

	packet_read_string(&p, sizeof(login->accname), login->accname);
	packet_read_string(&p, sizeof(login->password), login->password);

	LOG("account_login");
	LOG("xtea = {%08X, %08X, %08X, %08X}",
		login->xtea[0], login->xtea[1],
		login->xtea[2], login->xtea[3]);
	LOG("accname = \"%s\", password = \"%s\"",
		login->accname, login->password);

	if(strcmp(login->accname, "account") == 0){
		send_charlist(lserver, login);
	}else{
		send_disconnect(lserver, login,
			"Invalid account name or password.");
	}
}

static
void login_server_on_write(void *userdata, u32 connection, u8 **out_write_ptr, i32 *out_write_len){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = get_connection_login(lserver, connection);
	if(login->state == LOGIN_STATE_WRITING){
		*out_write_ptr = login->writebuf;
		*out_write_len = login->writelen;
		login->state = LOGIN_STATE_WAITING_WRITE;
		LOG("writing: %d", login->writelen);
	}else if(login->state == LOGIN_STATE_WAITING_WRITE){
		disconnect(lserver, login);
	}
}

// ----------------------------------------------------------------

LoginServer *login_server_init(MemArena *arena,
		RSA *server_rsa, u16 port, u16 max_connections){
	LoginServer *lserver = arena_alloc<LoginServer>(arena, 1);
	lserver->max_logins = max_connections;
	lserver->logins = arena_alloc<Login>(arena, max_connections);
	lserver->rsa = server_rsa;

	ServerParams server_params;
	server_params.port = port;
	server_params.max_connections = max_connections;
	server_params.readbuf_size = 256;
	server_params.on_accept = login_server_on_accept;
	server_params.on_drop = login_server_on_drop;
	server_params.on_read = login_server_on_read;
	server_params.on_write = login_server_on_write;
	lserver->server = server_init(arena, &server_params);
	if(!lserver->server)
		PANIC("failed to initialize login server");

	lserver->max_connections_closing = max_connections;
	lserver->num_connections_closing = 0;
	lserver->connections_closing = arena_alloc<u32>(arena, max_connections);
	return lserver;
}

void login_server_poll(LoginServer *lserver){
	// NOTE: Because the server processes the connections in order,
	// this array should already be sorted by connection_index.
	i32 num_connections_closing = lserver->num_connections_closing;
	u32 *connections_closing = lserver->connections_closing;

#if BUILD_DEBUG
	if(num_connections_closing > 0){
		for(i32 i = 1; i < num_connections_closing; i += 1){
			u32 prev = connection_index(connections_closing[i - 1]);
			u32 cur = connection_index(connections_closing[i]);
			ASSERT(prev <= cur);
		}
	}
#endif

	// NOTE: Reset these before hand. Server callbacks will fill them back up.
	lserver->num_connections_closing = 0;

	server_poll(lserver->server, lserver,
		connections_closing, num_connections_closing);
}
