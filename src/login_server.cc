#include "common.hh"
#include "crypto.hh"
#include "packet.hh"
#include "server.hh"

enum LoginState{
	LOGIN_STATE_WAITING_READ = 0,
	//TODO: LOGIN_STATE_WAITING_DATABASE,
	LOGIN_STATE_READY_TO_WRITE,
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
Login *lserver_login(LoginServer *lserver, u32 connection){
	i32 index = connection_index(connection);
	ASSERT(index < lserver->max_logins);
	return &lserver->logins[index];
}

static
void lserver_connection_closing(LoginServer *lserver, u32 connection){
	i32 idx = lserver->num_connections_closing;
	if(idx < lserver->max_connections_closing){
		lserver->connections_closing[idx] = connection;
		lserver->num_connections_closing += 1;
	}
}

static
void lserver_packet_prepare(Packet *p){
	// reserve 8 bytes for the header
	p->bufpos = 8;
}

static
void lserver_packet_wrap(Packet *p, u32 *xtea){
	u8 *buf = packet_buf(p);
	i32 len = packet_len(p);

	i32 payload_len = len - 8;
	if(payload_len <= 0)
		PANIC("trying to send empty message");

	// TODO: We should check for packet_ok after appending the padding bytes.

	// xtea encode
	u8 *xtea_payload = buf + 6;
	i32 xtea_payload_len = len - 6;
	i32 padding = -xtea_payload_len & 7;
	buffer_write_u16_le(xtea_payload, payload_len);
	xtea_payload_len += padding;
	while(padding-- > 0)
		packet_write_u8(p, 0x33);
	xtea_encode(xtea, xtea_payload, xtea_payload_len);

	// add message header
	u32 checksum = adler32(xtea_payload, xtea_payload_len);
	buffer_write_u16_le(buf, xtea_payload_len + 4);
	buffer_write_u32_le(buf + 2, checksum);
}


static
void lserver_disconnect(LoginServer *lserver, Login *login){
	lserver_connection_closing(lserver, login->connection);
	login->state = LOGIN_STATE_DISCONNECTING;
}

static
void lserver_send_disconnect(LoginServer *lserver, Login *login, const char *message){
	Packet p = make_packet(login->writebuf, sizeof(login->writebuf));
	lserver_packet_prepare(&p);
	packet_write_u8(&p, 0x0A);
	packet_write_str(&p, message);
	lserver_packet_wrap(&p, login->xtea);
	login->state = LOGIN_STATE_READY_TO_WRITE;
	login->writelen = packet_len(&p);
}

static
void lserver_send_charlist(LoginServer *lserver, Login *login){
	Packet p = make_packet(login->writebuf, sizeof(login->writebuf));
	lserver_packet_prepare(&p);
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
	lserver_packet_wrap(&p, login->xtea);
	login->state = LOGIN_STATE_READY_TO_WRITE;
	login->writelen = packet_len(&p);
}

static
void lserver_on_accept(void *userdata, u32 connection){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = lserver_login(lserver, connection);
	login->connection = connection;
	login->state = LOGIN_STATE_WAITING_READ;
}

static
void lserver_on_drop(void *userdata, u32 connection){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = lserver_login(lserver, connection);
	// NOTE: Zero out login memory because it may contain sensible information.
	memset(login, 0, sizeof(Login));
}

static
void lserver_on_read(void *userdata, u32 connection, u8 *data, i32 datalen){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	RSA *rsa = lserver->rsa;
	Login *login = lserver_login(lserver, connection);

	if(login->state != LOGIN_STATE_WAITING_READ){
		LOG_ERROR("unexpected message");
		lserver_disconnect(lserver, login);
		return;
	}

	if(datalen != 149){
		LOG_ERROR("invalid login message length"
			" (expected = 149, got = %d)", datalen);
		lserver_disconnect(lserver, login);
		return;
	}

	u32 real_checksum = adler32(data + 4, datalen - 4);
	if(buffer_read_u32_le(data) != real_checksum){
		LOG_ERROR("invalid login message checksum");
		lserver_disconnect(lserver, login);
		return;
	}

	if(buffer_read_u8(data + 4) != 0x01){
		LOG_ERROR("invalid login message id");
		lserver_disconnect(lserver, login);
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
		lserver_disconnect(lserver, login);
		return;
	}

	Packet p = make_packet(decoded, 127);
	login->xtea[0] = packet_read_u32(&p);
	login->xtea[1] = packet_read_u32(&p);
	login->xtea[2] = packet_read_u32(&p);
	login->xtea[3] = packet_read_u32(&p);

	if(version != 860){
		lserver_send_disconnect(lserver, login,
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
		lserver_send_charlist(lserver, login);
	}else{
		lserver_send_disconnect(lserver, login,
			"Invalid account name or password.");
	}

	LOG("OK");
	return;
}

static
void lserver_on_write(void *userdata, u32 connection, u8 **out_write_ptr, i32 *out_write_len){
	LOG("%08X", connection);

	LoginServer *lserver = (LoginServer*)userdata;
	Login *login = lserver_login(lserver, connection);
	if(login->state == LOGIN_STATE_READY_TO_WRITE){
		*out_write_ptr = login->writebuf;
		*out_write_len = login->writelen;
		login->state = LOGIN_STATE_WAITING_WRITE;
		LOG("writing: %d", login->writelen);
	}else if(login->state == LOGIN_STATE_WAITING_WRITE){
		login->state = LOGIN_STATE_DISCONNECTING;
		lserver_connection_closing(lserver, connection);
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
	server_params.on_accept = lserver_on_accept;
	server_params.on_drop = lserver_on_drop;
	server_params.on_read = lserver_on_read;
	server_params.on_write = lserver_on_write;
	lserver->server = server_init(arena, &server_params);
	if(!lserver->server)
		PANIC("failed to initialize login server");

	// NOTE: Leave enough room for duplicates.
	i32 max_connections_closing = 2 * (i32)max_connections;
	lserver->max_connections_closing = max_connections_closing;
	lserver->num_connections_closing = 0;
	lserver->connections_closing = arena_alloc<u32>(arena, max_connections_closing);
	return lserver;
}

void login_server_poll(LoginServer *lserver){
	// NOTE: Because the server processes the connections in order,
	// this array should already be sorted by connection_index but
	// may contain duplicates.
	i32 num_connections_closing = lserver->num_connections_closing;
	u32 *connections_closing = lserver->connections_closing;
	if(num_connections_closing > 0){
#if BUILD_DEBUG
		for(i32 i = 1; i < num_connections_closing; i += 1){
			u32 prev = connection_index(connections_closing[i - 1]);
			u32 cur = connection_index(connections_closing[i]);
			ASSERT(prev <= cur);
		}
#endif
		i32 j = 1;
		for(i32 i = 1; i < num_connections_closing; i += 1){
			if(connections_closing[i] != connections_closing[i - 1])
				connections_closing[j++] = connections_closing[i];
		}
		num_connections_closing = j;
	}

	// NOTE: Reset these before hand. Server callbacks will fill them back up.
	lserver->num_connections_closing = 0;

	server_poll(lserver->server, lserver,
		connections_closing, num_connections_closing);
}
