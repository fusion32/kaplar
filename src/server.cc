// NOTE: The server is supposed to run on Linux but, for
// testing, this is a simple and minimal implementation
// of a 'poll' based server for Windows.

#if OS_WINDOWS

#include "server.hh"

#include "common.hh"
#include "buffer_util.hh"

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>

static
int setsock_reuseaddr(SOCKET s, int reuseaddr){
	int result = setsockopt(s, SOL_SOCKET,
		SO_REUSEADDR, (char*)&reuseaddr, sizeof(int));
	return result;
}

static
int setsock_nonblocking(SOCKET s){
	u_long non_blocking = 1;
	int result = ioctlsocket(s, FIONBIO, &non_blocking);
	return result;
}

static
int setsock_linger(SOCKET s, int onoff, int seconds){
	linger l;
	l.l_onoff = onoff;
	l.l_linger = seconds;
	int result = setsockopt(s, SOL_SOCKET,
		SO_LINGER, (char*)&l, sizeof(linger));
	return result;
}

static
SOCKET server_socket(int port){
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	if(s == INVALID_SOCKET){
		LOG_ERROR("failed to create socket"
			" (error = %d)", WSAGetLastError());
		return INVALID_SOCKET;
	}

	if(setsock_reuseaddr(s, 1) == SOCKET_ERROR){
		LOG_ERROR("failed to set socket reuseaddr option"
			" (error = %d)", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	if(setsock_nonblocking(s) == SOCKET_ERROR){
		LOG_ERROR("failed to set socket nonblocking mode"
			" (error = %d)", WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(s, (sockaddr*)&addr, sizeof(sockaddr_in)) == SOCKET_ERROR){
		LOG_ERROR("failed to bind socket to port %d"
			" (error = %d)", port, WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}

	if(listen(s, SOMAXCONN) == SOCKET_ERROR){
		LOG_ERROR("failed to listen on port %d"
			" (error = %d)", port, WSAGetLastError());
		closesocket(s);
		return INVALID_SOCKET;
	}
	return s;
}

static
void tcp_close(SOCKET s){
	setsock_linger(s, 0, 0);
	closesocket(s);
}

static
void tcp_abort(SOCKET s){
	setsock_linger(s, 1, 0);
	closesocket(s);
}

enum{
	CONNECTION_CLOSED			= 0x01,
	CONNECTION_READING_LENGTH	= 0x02,
};

static
u32 make_connection_id(i16 index){
	return (u32)index;
}

static
u32 bump_connection_id(u32 cid){
	u32 index = cid & 0x0000FFFF;
	u32 counter = (cid + 0x00010000) & 0xFFFF0000;
	return counter | index;
}

struct Connection{
	i32 freelist_next;
	u32 connection_id;

	SOCKET s;
	sockaddr_in addr;
	u32 flags;

	u8 *readbuf;
	u16 readbuf_size;
	u16 readbuf_pos;
	u16 bytes_to_read;
	u16 message_length;
};

struct Server{
	SOCKET s;
	u16 port;
	u16 max_connections;
	i32 freelist_head;
	WSAPOLLFD *pollfds;
	Connection *connections;
};

Server *server_init(MemArena *arena, u16 port,
		u16 max_connections, u16 readbuf_size){
	SOCKET s = server_socket(port);
	if(s == INVALID_SOCKET)
		return NULL;

	Server *server = arena_alloc<Server>(arena, 1);
	server->s = s;
	server->port = port;
	server->max_connections = max_connections;
	server->freelist_head = 0;

	server->pollfds = arena_alloc<WSAPOLLFD>(arena, max_connections);
	server->connections = arena_alloc<Connection>(arena, max_connections);
	for(u16 i = 0; i < max_connections; i += 1){
		server->pollfds[i].fd = INVALID_SOCKET;
		server->pollfds[i].events = POLLIN;

		Connection *cptr = &server->connections[i];
		cptr->freelist_next = i + 1;
		cptr->connection_id = make_connection_id(i);
		cptr->readbuf = arena_alloc<u8>(arena, readbuf_size);
		cptr->readbuf_size = readbuf_size;
	}
	server->connections[max_connections - 1].freelist_next = -1;

	return server;
}

static
i32 server_alloc_connection(Server *server){
	if(server->freelist_head == -1)
		return -1;
	i32 c = server->freelist_head;
	Connection *cptr = &server->connections[c];
	server->freelist_head = cptr->freelist_next;
	return c;
}

static
void server_free_connection(Server *server, i32 c){
	ASSERT(c >= 0 && c < server->max_connections);
	server->pollfds[c].fd = INVALID_SOCKET;
	server->connections[c].freelist_next = server->freelist_head;
	server->connections[c].connection_id =
		bump_connection_id(server->connections[c].connection_id);
	server->freelist_head = c;
}

static
void server_accept_connections(Server *server,
		OnConnectionAccept on_accept, void *userdata){
	while(1){
		sockaddr_in addr;
		int addrlen = sizeof(sockaddr_in);
		SOCKET s = accept(server->s, (sockaddr*)&addr, &addrlen);
		if(s == INVALID_SOCKET){
			// TODO: Should we recreate the server socket if there
			// is an error other than WSAEWOULDBLOCK?
			//if(WSAGetLastError() == WSAEWOULDBLOCK)
			//	return;
			return;
		}
		LOG("accepted socket = %d", s);

		// PARANOID: Can only happen if there is a bug in the OS?
		if(addrlen != sizeof(sockaddr_in)){
			tcp_abort(s);
			LOG_ERROR("accept returned an unexpected address length");
			continue;
		}

		// TODO: I think setsock_linger(s, 0, 0) is already the default
		// for SO_LINGER so I'm not sure if we should touch it here.

		if(setsock_nonblocking(s) == SOCKET_ERROR){
			tcp_abort(s);
			LOG_ERROR("failed to set socket nonblocking mode"
				" (error = %d)", WSAGetLastError());
			continue;
		}

		i32 c = server_alloc_connection(server);
		if(c == -1){
			tcp_abort(s);
			LOG_ERROR("new connection rejected due to connection"
				" number limit (%d)", server->max_connections);
			continue;
		}

		server->pollfds[c].fd = s;
		server->connections[c].s = s;
		server->connections[c].addr = addr;
		server->connections[c].flags = CONNECTION_READING_LENGTH;
		server->connections[c].readbuf_pos = 0;
		server->connections[c].bytes_to_read = 2;

		on_accept(userdata, server->connections[c].connection_id);
	}
}

static
void connection_close(Connection *cptr){
	if(!(cptr->flags & CONNECTION_CLOSED)){
		tcp_close(cptr->s);
		cptr->flags |= CONNECTION_CLOSED;
	}
}

static
void connection_abort(Connection *cptr){
	if(!(cptr->flags & CONNECTION_CLOSED)){
		tcp_abort(cptr->s);
		cptr->flags |= CONNECTION_CLOSED;
	}
}

static
void connection_resume_reading(Connection *cptr,
		OnConnectionRead on_read, void *userdata){
	if(cptr->flags & CONNECTION_CLOSED)
		return;

	while(1){
		u8 *read_ptr = cptr->readbuf + cptr->readbuf_pos;
		int ret = recv(cptr->s, (char*)read_ptr, cptr->bytes_to_read, 0);
		if(ret == SOCKET_ERROR){
			if(WSAGetLastError() != WSAEWOULDBLOCK)
				connection_abort(cptr);
			return;
		}else if(ret == 0){
			LOG("connection close on recv 0");
			connection_close(cptr);
			return;
		}

		cptr->readbuf_pos += ret;
		cptr->bytes_to_read -= ret;
		if(cptr->bytes_to_read == 0){
			if(cptr->flags & CONNECTION_READING_LENGTH){
				i32 message_length = buffer_read_u16_le(cptr->readbuf);
				if(message_length > cptr->readbuf_size){
					connection_abort(cptr);
					return;
				}
				cptr->readbuf_pos = 2;
				cptr->bytes_to_read = message_length;
				cptr->message_length = message_length;
				cptr->flags &= ~CONNECTION_READING_LENGTH;
			}else{
				on_read(userdata, cptr->connection_id,
					cptr->readbuf, cptr->message_length);
				cptr->readbuf_pos = 0;
				cptr->bytes_to_read = 2;
				cptr->flags |= CONNECTION_READING_LENGTH;
			}
		}
	}
}

static
void connection_resume_writing(Connection *cptr,
		OnConnectionWrite on_write, void *userdata){
	if(cptr->flags & CONNECTION_CLOSED)
		return;

	// TODO
}

void server_poll(Server *server,
		OnConnectionAccept on_accept,
		OnConnectionDrop on_drop,
		OnConnectionRead on_read,
		OnConnectionWrite on_write,
		void *userdata){

	server_accept_connections(server, on_accept, userdata);

	WSAPOLLFD *fds = server->pollfds;
	int nfds = server->max_connections;
	int poll_result = WSAPoll(fds, nfds, 0);
	if(poll_result > 0){
		for(int i = 0; i < nfds; i += 1){
			// NOTE: Uhh... When you set fds[i].fd to INVALID_SOCKET
			// on windows, WSAPoll will set fds[i].revents to POLLNVAL.
			// Whereas on linux, poll will set fds[i].revents to zero.

			if(fds[i].fd == INVALID_SOCKET)
				continue;

			Connection *cptr = &server->connections[i];
			if(fds[i].revents){
				ASSERT(!(fds[i].revents & POLLNVAL));
				if(fds[i].revents & (POLLHUP | POLLERR)){
					connection_close(cptr);
				}else{
					if(fds[i].revents & POLLIN)
						connection_resume_reading(cptr, on_read, userdata);
					if(fds[i].revents & POLLOUT)
						connection_resume_writing(cptr, on_write, userdata);
				}

				if(cptr->flags & CONNECTION_CLOSED){
					on_drop(userdata, cptr->connection_id);
					server_free_connection(server, i);
				}
			}
		}
	}
}

// NOTE: This is a windows hack to avoid ever calling
// WSAStartup and WSACleanup.

struct WSAInit{
	WSAInit(void){
		WSADATA dummy;
		if(WSAStartup(MAKEWORD(2, 2), &dummy) != 0)
			PANIC("failed to initialize windows sockets");
	}
	~WSAInit(void){
		WSACleanup();
	}
};
static WSAInit wsa_init;

#endif // OS_WINDOWS
