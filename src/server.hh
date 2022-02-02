#ifndef KAPLAR_SERVER_HH_
#define KAPLAR_SERVER_HH_

#include "common.hh"

struct Server;
Server *server_init(MemArena *arena, u16 port,
		u16 max_connections, u16 max_readbuf_size);

typedef void (*OnConnectionAccept)(void *userdata, u32 connection);
typedef void (*OnConnectionDrop)(void *userdata, u32 connection);
typedef void (*OnConnectionRead)(void *userdata, u32 connection, u8 *data, i32 datalen);
typedef void (*OnConnectionWrite)(void *userdata, u32 connection /* TODO */);
void server_poll(Server *server,
		OnConnectionAccept on_accept,
		OnConnectionDrop on_drop,
		OnConnectionRead on_read,
		OnConnectionWrite on_write,
		void *userdata);

#endif // KAPLAR_SERVER_HH_
