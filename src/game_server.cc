#include "game_server.hh"

#include "common.hh"
#include "crypto.hh"
#include "packet.hh"
#include "server.hh"

enum ClientState : u32 {
	CLIENT_STATE_HANDSHAKE_WRITING = 0,
	CLIENT_STATE_HANDSHAKE_WAITING_WRITE,
	CLIENT_STATE_HANDSHAKE_READING,
	//CLIENT_STATE_HANDSHAKE_WAITING_DATABASE,
	CLIENT_STATE_NORMAL,
	CLIENT_STATE_DISCONNECT_WRITING,
	CLIENT_STATE_DISCONNECT_WAITING_WRITE,
	CLIENT_STATE_DISCONNECTING,
};

struct Client{
	ClientState state;
	u32 xtea[4];
	char accname[32];
	char password[32];
	char character[32];

	// TODO: Instead of a single write buffer like in the
	// login server, we need at least two buffers to avoid
	// not having where to write a client message and thus
	// having to drop the connection.
	//
	//	We should consider two designs:
	//
	//		- The client has 3 or 4 write buffers and we
	//		cycle through them like a circular buffer. This
	//		should be enough to handle every scenario?
	//
	//		- The client has a write queue and all write
	//		buffers are kept in a pool. When we want to
	//		write to the client, we get a buffer from the
	//		pool and append it to the write queue like a
	//		linked list. After the write we return it back
	//		to the pool.
	//
	// NOTE: Each write buffer for the game client should
	// have 32KB. This should really be more than enough
	// to handle every case but we will need statistics
	// of a running server to be 100% sure.

	OutPacket *out_writing;
	OutPacket *out_queue_head;
	OutPacket *out_queue_tail;
};

struct GameServer{
	i32 max_clients;
	Client *clients;
	RSA *rsa;
	Server *server;
	MemArena *output_arena;
	OutPacket *output_head;
};

static
Client *get_connection_client(GameServer *gserver, i32 connection){
	ASSERT(connection >= 0 && connection < gserver->max_clients);
	return &gserver->clients[connection];
}

#define OUT_PACKET_BUFFER_SIZE (16 * 1024)

OutPacket *get_out_packet(GameServer *gserver, Client *client, u32 size){
	ASSERT((client->out_queue_head != NULL && client->out_queue_tail != NULL)
		|| (client->out_queue_head == NULL && client->out_queue_tail == NULL));

	if(client->out_queue_tail){
		// NOTE: Check for size + 7 so that if the writer doesn't surpass
		// `size` bytes, it should be enough for any packet_wrap padding.
		OutPacket *outp = client->out_queue_tail;
		if(size > 0 && packet_can_write(outp, size + 7))
			return outp;
	}

	OutPacket *outp;
	if(gserver->output_head){
		outp = gserver->output_head;
		gserver->output_head = outp->next;
	}else{
		MemArena *output_arena = gserver->output_arena;
		outp = arena_alloc<OutPacket>(output_arena, 1);
		outp->buf = arena_alloc<u8>(output_arena, OUT_PACKET_BUFFER_SIZE);
		outp->bufend = OUT_PACKET_BUFFER_SIZE;
	}

	if(client->out_queue_tail){
		client->out_queue_tail->next = outp;
		client->out_queue_tail = outp;
	}else{
		client->out_queue_head = outp;
		client->out_queue_tail = outp;
	}
	outp->next = NULL;
	outp->bufpos = 8;
	return outp;
}

void release_out_packet(GameServer *gserver, OutPacket *outp){
	ASSERT(outp);
	ASSERT(outp->bufend == OUT_PACKET_BUFFER_SIZE);
	outp->next = gserver->output_head;
	gserver->output_head = outp;
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
bool packet_unwrap(u8 *buf, i32 len, u32 *xtea, InPacket *p){
	// NOTE: Different from wrapping a packet, the message length
	// shouldn't be considered as it is discarded earlier when
	// reading from the socket.

	// NOTE: We need at least 4 bytes for the checksum and 8 bytes
	// for the smallest XTEA encoded message.
	if(len < 12)
		return false;

	u8 *xtea_payload = buf + 4;
	i32 xtea_payload_len = len - 4;

	// check that xtea_payload_len is a multiple of 8
	if(xtea_payload_len & 7)
		return false;

	// verify checksum
	u32 checksum = adler32(xtea_payload, xtea_payload_len);
	if(buffer_read_u32_le(buf) != checksum)
		return false;

	// decode message
	xtea_decode(xtea, xtea_payload, xtea_payload_len);

	// check that the encoded payload length doesn't
	// overflow the packet length
	u8 *payload = xtea_payload + 2;
	i32 max_payload_len = xtea_payload_len - 2;
	i32 payload_len = buffer_read_u16_le(xtea_payload);
	if(payload_len > max_payload_len)
		return false;

	p->buf = payload;
	p->bufend = payload_len;
	p->bufpos = 0;
	return true;
}

static
void disconnect(Client *client){
	client->state = CLIENT_STATE_DISCONNECTING;
}

static
void send_disconnect(GameServer *gserver, Client *client, const char *message){
	client->state = CLIENT_STATE_DISCONNECT_WRITING;

	OutPacket *p = get_out_packet(gserver, client, 256);
	packet_write_u8(p, 0x14);
	packet_write_str(p, message);
}

static
void send_login(GameServer *gserver, Client *client){
	client->state = CLIENT_STATE_NORMAL;

	OutPacket *p = get_out_packet(gserver, client, 0);

	// NOTE: The comments on the side are old annotations from another
	// implementation I did that I didn't check. So they may/will be wrong.

	// login message
	u32 player_id = 0x10000000;
	packet_write_u8(p, 0x0A);
	packet_write_u32(p, player_id);			// creature id (?)
	packet_write_u16(p, 0x0032);				// related to client drawing speed
	packet_write_u8(p, 0x00);					// can report bugs

	// map description
	packet_write_u8(p, 0x64);
	packet_write_u16(p, 100);					// posx
	packet_write_u16(p, 100);					// posy
	packet_write_u8(p, 7);						// posz

	for(i32 z = 7; z >= 0; z -= 1)
	for(i32 x = 0; x < 18; x += 1)
	for(i32 y = 0; y < 14; y += 1){
		packet_write_u16(p, 106);				// grass tile
		// add player at the center
		if(x == 8 && y == 6 && z == 7){
			packet_write_u16(p, 0x61);

			packet_write_u32(p, 0);			// remove known
			packet_write_u32(p, player_id);	// creature id (?)

			packet_write_str(p, client->character);	// creature name
			packet_write_u8(p, 100);			// health %
			packet_write_u8(p, 2);				// direction

			packet_write_u16(p, 136);			// outfit
			packet_write_u8(p, 10);			// look head
			packet_write_u8(p, 10);			// look body
			packet_write_u8(p, 10);			// look legs
			packet_write_u8(p, 10);			// look feet
			packet_write_u8(p, 0);				// look addons

			packet_write_u8(p, 0);				// light level
			packet_write_u8(p, 0);				// light color
			packet_write_u16(p, 100);			// step speed
			packet_write_u8(p, 3);				// skull
			packet_write_u8(p, 3);				// party shield
			packet_write_u8(p, 1);				// war emblem
			packet_write_u8(p, 1);				// will block path
		}else{
			packet_write_u16(p, 2400);
		}
		packet_write_u16(p, 0xFF00);
	}

	// inventory (no items)
	packet_write_u16(p, 0x0179);				// head
	packet_write_u16(p, 0x0279);				// neck
	packet_write_u16(p, 0x0379);				// backpack
	packet_write_u16(p, 0x0479);				// armor
	packet_write_u16(p, 0x0579);				// right (?)
	packet_write_u16(p, 0x0679);				// left (?)
	packet_write_u16(p, 0x0779);				// legs
	packet_write_u16(p, 0x0879);				// feet
	packet_write_u16(p, 0x0979);				// ring
	packet_write_u16(p, 0x0A79);				// ammo

	// stats
	packet_write_u8(p, 0xA0);
	packet_write_u16(p, 125);					// health
	packet_write_u16(p, 150);					// maxhealth
	packet_write_u32(p, 100 * 100);			// free capacity
	packet_write_u32(p, 155);					// experience
	packet_write_u16(p, 15);					// level
	packet_write_u8(p, 50);					// level %
	packet_write_u16(p, 123);					// mana
	packet_write_u16(p, 321);					// maxmana
	packet_write_u8(p, 25);					// magic level
	packet_write_u8(p, 75);					// magic level %
	packet_write_u8(p, 97);					// soul
	packet_write_u16(p, 61);					// stamina (minutes)

	// skills
	packet_write_u8(p, 0xA1);
	packet_write_u8(p, 11);					// fist level
	packet_write_u8(p, 90);					// fist level %
	packet_write_u8(p, 12);					// club level
	packet_write_u8(p, 80);					// club level %
	packet_write_u8(p, 13);					// sword level
	packet_write_u8(p, 70);					// sword level %
	packet_write_u8(p, 14);					// axe level
	packet_write_u8(p, 60);					// axe level %
	packet_write_u8(p, 15);					// dist level
	packet_write_u8(p, 50);					// dist level %
	packet_write_u8(p, 16);					// shield level
	packet_write_u8(p, 40);					// shield level %
	packet_write_u8(p, 17);					// fish level
	packet_write_u8(p, 30);					// fish level %

	// world light
	packet_write_u8(p, 0x82);
	packet_write_u8(p, 250);					// level
	packet_write_u8(p, 0xD7);					// color

	// creature light
	//packet_write_u8(p, 0x8D);
	//packet_write_u32(p, player_id);			// creature id (?)
	//packet_write_u8(p, 0);					// level
	//packet_write_u8(p, 0);					// color

	// vip list
	//packet_write_u8(p, 0xD2);
	//packet_write_u32(p, 0xFFFF0001);			// creature id (?)
	//packet_write_str(p, "Friend");			// name
	//packet_write_u8(p, 0);					// online

	// icons
	//packet_write_u8(p, 0xA2);
	//packet_write_u16(p, 0);
}

static
void send_inventory(GameServer *gserver, Client *client, u8 slot, u16 client_id){
	OutPacket *p = get_out_packet(gserver, client, 4);
	packet_write_u8(p, 0x78);
	packet_write_u8(p, slot);
	packet_write_u16(p, client_id);

	// NOTE: Sending items that are countable or have a subtype will
	// result in a crash because we're not addressing an extra byte
	// needed in those cases.
	//if(item->countable || item->has_subtype)
	//	packet_write_u8(p, item->count_or_subtype);
}

static
void send_relogin_window(GameServer *gserver, Client *client){
	OutPacket *p = get_out_packet(gserver, client, 1);
	packet_write_u8(p, 0x28);
}

static
void game_server_on_accept(void *userdata, i32 connection){
	LOG("%08X", connection);
	GameServer *gserver = (GameServer*)userdata;
	Client *client = get_connection_client(gserver, connection);
	client->state = CLIENT_STATE_HANDSHAKE_WRITING;
	client->out_writing = NULL;
	client->out_queue_head = NULL;
	client->out_queue_tail = NULL;
}

static
void game_server_on_drop(void *userdata, i32 connection){
	LOG("%08X", connection);
	GameServer *gserver = (GameServer*)userdata;
	Client *client = get_connection_client(gserver, connection);

	// NOTE: Release any out packet we're using here.
	ASSERT((client->out_queue_head != NULL && client->out_queue_tail != NULL)
		|| (client->out_queue_head == NULL && client->out_queue_tail == NULL));
	if(client->out_writing)
		release_out_packet(gserver, client->out_writing);
	while(client->out_queue_tail){
		OutPacket *tmp = client->out_queue_tail;
		client->out_queue_tail = tmp->next;
		release_out_packet(gserver, tmp);
	}

	// NOTE: Zero out client memory for security reasons.
	memset(client, 0, sizeof(Client));
}

static
void game_server_on_read(void *userdata, i32 connection, u8 *data, i32 datalen){
	GameServer *gserver = (GameServer*)userdata;
	Client *client = get_connection_client(gserver, connection);
	switch(client->state){
		case CLIENT_STATE_HANDSHAKE_READING:{
			RSA *rsa = gserver->rsa;

			if(datalen != 137){
				disconnect(client);
				return;
			}

			u32 checksum = adler32(data + 4, datalen - 4);
			if(buffer_read_u32_le(data) != checksum){
				disconnect(client);
				return;
			}

			if(buffer_read_u8(data + 4) != 0x0A){
				disconnect(client);
				return;
			}

			//buffer_read_u16_le(data + 5); // os
			u16 version = buffer_read_u16_le(data + 7);

			// NOTE: RSA DECODE the remaining 128 bytes. This can only fail if
			// the result contains more than 128 bytes which is impossible if
			// the key has 1024 bits but is always good to calm your paranoid
			// side and in case you derived a bad key unknowingly.
			u8 *decoded = data + 9;
			usize decoded_len = 128;
			if(!rsa_decode(rsa, decoded, &decoded_len, 128)){
				PANIC("RSA DECODE step produced a bad result. This is only"
					" possible if the RSA key has more than 1024 bits.");
			}
			if(decoded_len != 127){
				disconnect(client);
				return;
			}

			debug_print_buf("decoded", decoded, 127);
			debug_print_buf_hex("decoded", decoded, 127);
			InPacket p = in_packet(decoded, 127);
			client->xtea[0] = packet_read_u32(&p);
			client->xtea[1] = packet_read_u32(&p);
			client->xtea[2] = packet_read_u32(&p);
			client->xtea[3] = packet_read_u32(&p);

			if(version != 860){
				send_disconnect(gserver, client,
					"This server requires client version 8.60.");
				return;
			}

			packet_read_u8(&p); // gm flag or something
			packet_read_string(&p, sizeof(client->accname), client->accname);
			packet_read_string(&p, sizeof(client->character), client->character);
			packet_read_string(&p, sizeof(client->password), client->password);

			LOG("player_login");
			LOG("xtea = {%08X, %08X, %08X, %08X}",
				client->xtea[0], client->xtea[1],
				client->xtea[2], client->xtea[3]);
			LOG("accname = \"%s\", password = \"%s\", character = \"%s\"",
				client->accname, client->password, client->character);

			if(strcmp(client->accname, "account") == 0){
				send_login(gserver, client);
			}else{
				send_disconnect(gserver, client,
					"Invalid account.");
			}
			break;
		}

		case CLIENT_STATE_NORMAL: {
			InPacket p;
			if(!packet_unwrap(data, datalen, client->xtea, &p)){
				disconnect(client);
				return;
			}

			if(packet_remainder(&p) > 0){
				debug_print_buf_hex("client message",
					packet_buf(&p), packet_remainder(&p));

				u8 message = packet_read_u8(&p);

				switch(message){
					case 0x14: {	// logout
						disconnect(client);
						return;
					}

					case 0x96: {	// player_say
						if(packet_read_u8(&p) != 0x01){
							disconnect(client);
							return;
						}

						char say_str[256];
						packet_read_string(&p, sizeof(say_str), say_str);

						u16 client_id = (u16)atoi(say_str);
						if(client_id >= 100)
							send_inventory(gserver, client, 0x06, client_id);
						else if(client_id == 15)
							send_relogin_window(gserver, client);
						break;
					}
				}
			}
			break;
		}

		default: {
			disconnect(client);
			break;
		}
	}
}

static
void game_server_request_output(void *userdata,
		i32 connection, u8 **output, i32 *output_len){
	GameServer *gserver = (GameServer*)userdata;
	Client *client = get_connection_client(gserver, connection);
	switch(client->state){
		case CLIENT_STATE_HANDSHAKE_WRITING: {
			// NOTE: This seems to be some kind of challenge message that
			// is simply echoed back inside the login message after the
			// accname, charname, and password.
			static u8 challenge[] = {
				0x0C, 0x00,							// message total length
				0x23, 0x03, 0xE8, 0x0A,				// message checksum
				0x06, 0x00,							// message data length
				0x1F, 0xFF, 0xFF, 0x00, 0x00, 0xFF	// message data
			};
			*output = challenge;
			*output_len = sizeof(challenge);
			client->state = CLIENT_STATE_HANDSHAKE_WAITING_WRITE;
			break;
		}
		case CLIENT_STATE_HANDSHAKE_WAITING_WRITE: {
			client->state = CLIENT_STATE_HANDSHAKE_READING;
			break;
		}

		case CLIENT_STATE_NORMAL:
		case CLIENT_STATE_DISCONNECT_WRITING: {
			ASSERT((client->out_queue_head != NULL && client->out_queue_tail != NULL)
				|| (client->out_queue_head == NULL && client->out_queue_tail == NULL));

			if(client->out_writing){
				release_out_packet(gserver, client->out_writing);
				client->out_writing = NULL;
			}

			if(client->out_queue_head){
				OutPacket *outp = client->out_queue_head;
				if(packet_wrap(outp, client->xtea)){
					client->out_writing = outp;
					client->out_queue_head = outp->next;
					if(!client->out_queue_head)
						client->out_queue_tail = NULL;
					*output = packet_buf(outp);
					*output_len = packet_written_len(outp);
					LOG("writing %d\n", packet_written_len(outp));
					if(client->state == CLIENT_STATE_DISCONNECT_WRITING)
						client->state = CLIENT_STATE_DISCONNECT_WAITING_WRITE;
				}else{
					disconnect(client);
				}
			}
			break;
		}

		case CLIENT_STATE_DISCONNECT_WAITING_WRITE: {
			disconnect(client);
			break;
		}
	}
}

static
void game_server_request_status(void *userdata, i32 connection, ConnectionStatus *out_status){
	GameServer *gserver = (GameServer*)userdata;
	Client *client = get_connection_client(gserver, connection);
	if(client->state == CLIENT_STATE_DISCONNECTING)
		*out_status = CONNECTION_STATUS_CLOSING;
}

// ----------------------------------------------------------------

GameServer *game_server_init(MemArena *arena,
		RSA *server_rsa, u16 port, u16 max_connections){

	// TODO: We might want a different arena specifically for allocating OutPackets.
	GameServer *gserver = arena_alloc<GameServer>(arena, 1);
	gserver->max_clients = max_connections;
	gserver->clients = arena_alloc<Client>(arena, max_connections);
	gserver->rsa = server_rsa;
	gserver->output_arena = arena;
	gserver->output_head = NULL;

	ServerParams server_params;
	server_params.port = port;
	server_params.max_connections = max_connections;
	server_params.readbuf_size = 2048;
	server_params.on_accept = game_server_on_accept;
	server_params.on_drop = game_server_on_drop;
	server_params.on_read = game_server_on_read;
	server_params.request_output = game_server_request_output;
	server_params.request_status = game_server_request_status;
	gserver->server = server_init(arena, &server_params);
	if(!gserver->server)
		PANIC("failed to initialize game server");
	return gserver;
}

void game_server_poll(GameServer *gserver){
	server_poll(gserver->server, gserver);
}
