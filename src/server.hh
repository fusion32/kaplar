#ifndef KAPLAR_SERVER_HH_
#define KAPLAR_SERVER_HH_ 1

#include "common.hh"

enum ConnectionStatus : u32 {
	CONNECTION_STATUS_ALIVE = 0,
	CONNECTION_STATUS_CLOSING,
};
typedef void (*OnAccept)(void *userdata, i32 connection);
typedef void (*OnDrop)(void *userdata, i32 connection);
typedef void (*OnRead)(void *userdata, i32 connection, u8 *data, i32 datalen);
typedef void (*RequestOutput)(void *userdata, i32 connection, u8 **output, i32 *output_len);
typedef void (*RequestStatus)(void *userdata, i32 connection, ConnectionStatus *out_status);

struct ServerParams{
	u16 port;
	u16 max_connections;
	u16 readbuf_size;
	OnAccept on_accept;
	OnDrop on_drop;
	OnRead on_read;
	RequestOutput request_output;
	RequestStatus request_status;
};

struct Server;
Server *server_init(MemArena *arena, ServerParams *params);
void server_poll(Server *server, void *userdata);

#if 0
// TODO: Move this to another place when we need it. The
// purpose of these ids are to avoid having a send/close
// on an wrong connection. We assign a connection to an
// index internally so if in a single frame a connection
// gets dropped and a new one gets accepted and is assigned
// that same index we'll send/close on the wrong connection.
//	Right now we have only two layers: the "server" layer,
// and the "protocol" layer. They work in sync with each
// other so there is no need for connection ids there. Soon
// enough we'll need to add the "game" layer which does its
// own "thing" (not in sync with the other layers) and thats
// when we'll need a connection id.


// TODO: Set the top bits of the connection id in a way
// to differentiate connections from different servers.
// For example:
//	login server:	0b00...    0b000...
//	game server:	0b01... or 0b001...
//	info server:	0b10...    0b010...

static
u32 make_connection_id(u32 index){
	return index;
}

static INLINE
u32 connection_index(u32 connection_id){
	// NOTE: This is a way of mapping a connection to its
	// underlying object.
	return connection_id & 0xFFFF;
}

static
u32 bump_connection_id(u32 cid){
	u32 index = cid & 0x0000FFFF;
	u32 counter = (cid + 0x00010000) & 0xFFFF0000;
	return counter | index;
}
#endif

#endif // KAPLAR_SERVER_HH_
