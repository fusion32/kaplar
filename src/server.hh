#ifndef KAPLAR_SERVER_HH_
#define KAPLAR_SERVER_HH_ 1

#include "common.hh"

// ----------------------------------------------------------------
// Server
// ----------------------------------------------------------------
enum ConnectionStatus : u32 {
	CONNECTION_STATUS_ALIVE = 0,
	CONNECTION_STATUS_CLOSING,
};
typedef void (*OnAccept)(void *userdata, u32 index);
typedef void (*OnDrop)(void *userdata, u32 index);
typedef void (*OnRead)(void *userdata, u32 index, u8 *data, i32 datalen);
typedef void (*RequestOutput)(void *userdata, u32 index, u8 **output, i32 *output_len);
typedef void (*RequestStatus)(void *userdata, u32 index, ConnectionStatus *out_status);

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

#endif // KAPLAR_SERVER_HH_
