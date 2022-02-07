#ifndef KAPLAR_SERVER_HH_
#define KAPLAR_SERVER_HH_

#include "common.hh"

typedef void (*OnConnectionAccept)(void *userdata, u32 connection);
typedef void (*OnConnectionDrop)(void *userdata, u32 connection);
typedef void (*OnConnectionRead)(void *userdata, u32 connection, u8 *data, i32 datalen);
typedef void (*OnConnectionWrite)(void *userdata, u32 connection, u8 **out_data, i32 *out_datalen);

// TODO: Set the top bits of the connection id in a way
// to differentiate connections from different servers.
// For example:
//	login server:	0b00...    0b000...
//	game server:	0b01... or 0b001...
//	info server:	0b10...    0b010...

struct ServerParams{
	u16 port;
	u16 max_connections;
	u16 readbuf_size;
	OnConnectionAccept on_accept;
	OnConnectionDrop on_drop;
	OnConnectionRead on_read;
	OnConnectionWrite on_write;
};

struct Server;
Server *server_init(MemArena *arena, ServerParams *params);

static INLINE
u32 connection_index(u32 connection_id){
	// NOTE: This is a way of mapping a connection to its
	// underlying object.
	return connection_id & 0xFFFF;
}

void server_poll(Server *server, void *userdata,
		u32 *connections_closing, i32 num_connections_closing);

#endif // KAPLAR_SERVER_HH_
