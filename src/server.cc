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

struct Connection{
	i32 freelist_next;
	u32 closed : 1;
	u32 closing : 1;

	SOCKET s;
	sockaddr_in addr;

	// connection input
	u8 *readbuf;
	u16 readbuf_size;
	u16 readbuf_pos;
	u16 bytes_to_read;
	u16 message_length;

	// connection output
	u8 *write_ptr;
	i32 bytes_to_write;
};

struct Server{
	SOCKET s;
	u16 port;
	u16 max_connections;
	i32 freelist_head;
	WSAPOLLFD *pollfds;
	Connection *connections;
	OnAccept on_accept;
	OnDrop on_drop;
	OnRead on_read;
	RequestOutput request_output;
	RequestStatus request_status;
};

Server *server_init(MemArena *arena, ServerParams *params){
	ASSERT(params->on_accept);
	ASSERT(params->on_drop);
	ASSERT(params->on_read);
	ASSERT(params->request_output);
	ASSERT(params->request_status);

	u16 port = params->port;
	u16 max_connections = params->max_connections;
	u16 readbuf_size = params->readbuf_size;

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
		server->pollfds[i].events = POLLIN | POLLOUT;

		Connection *cptr = &server->connections[i];
		cptr->freelist_next = i + 1;
		cptr->readbuf = arena_alloc<u8>(arena, readbuf_size);
		cptr->readbuf_size = readbuf_size;
	}
	server->connections[max_connections - 1].freelist_next = -1;

	server->on_accept = params->on_accept;
	server->on_drop = params->on_drop;
	server->on_read = params->on_read;
	server->request_output = params->request_output;
	server->request_status = params->request_status;
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
	server->freelist_head = c;
}

static
void server_accept_connections(Server *server,
		OnAccept on_accept, void *userdata){
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

		Connection *cptr = &server->connections[c];
		cptr->s = s;
		cptr->addr = addr;
		cptr->closed = 0;
		cptr->closing = 0;
		cptr->readbuf_pos = 0;
		cptr->bytes_to_read = 2;
		cptr->message_length = 0;
		cptr->write_ptr = NULL;
		cptr->bytes_to_write = 0;
		on_accept(userdata, c);
	}
}

static
void connection_close(Connection *cptr){
	if(!cptr->closed){
		tcp_close(cptr->s);
		cptr->closed = 1;
	}
}

static
void connection_abort(Connection *cptr){
	if(!cptr->closed){
		tcp_abort(cptr->s);
		cptr->closed = 1;
	}
}

static
void connection_resume_reading(i32 c, Connection *cptr,
		OnRead on_read, void *userdata){
	if(cptr->closed)
		return;

	while(1){
		u8 *read_ptr = cptr->readbuf + cptr->readbuf_pos;
		int ret = recv(cptr->s, (char*)read_ptr, cptr->bytes_to_read, 0);
		if(ret == SOCKET_ERROR){
			if(WSAGetLastError() != WSAEWOULDBLOCK)
				connection_abort(cptr);
			return;
		}else if(ret == 0){
			connection_close(cptr);
			return;
		}

		cptr->readbuf_pos += ret;
		cptr->bytes_to_read -= ret;
		if(cptr->bytes_to_read == 0){
			if(cptr->message_length == 0){
				i32 message_length = buffer_read_u16_le(cptr->readbuf);
				if(message_length > cptr->readbuf_size){
					connection_abort(cptr);
					return;
				}
				cptr->readbuf_pos = 0;
				cptr->bytes_to_read = message_length;
				cptr->message_length = message_length;
			}else{
				on_read(userdata, c, cptr->readbuf, cptr->message_length);
				cptr->readbuf_pos = 0;
				cptr->bytes_to_read = 2;
				cptr->message_length = 0;
			}
		}
	}
}

static
void connection_resume_writing(i32 c, Connection *cptr,
		RequestOutput request_output, void *userdata){
	if(cptr->closed)
		return;
	while(1){
		if(cptr->bytes_to_write == 0){
			request_output(userdata, c, &cptr->write_ptr, &cptr->bytes_to_write);
			if(cptr->bytes_to_write == 0)
				return;
		}

		int ret = send(cptr->s, (char*)cptr->write_ptr, cptr->bytes_to_write, 0);
		if(ret == SOCKET_ERROR){
			if(WSAGetLastError() != WSAEWOULDBLOCK)
				connection_abort(cptr);
			return;
		}
		cptr->write_ptr += ret;
		cptr->bytes_to_write -= ret;
	}
}

void server_poll(Server *server, void *userdata){
	server_accept_connections(server, server->on_accept, userdata);

	WSAPOLLFD *fds = server->pollfds;
	i32 nfds = server->max_connections;
	WSAPoll(fds, nfds, 0);
	for(i32 c = 0; c < nfds; c += 1){
		// NOTE: Uhh... When you set fds[c].fd to INVALID_SOCKET
		// on windows, WSAPoll will set fds[c].revents to POLLNVAL.
		// Whereas on linux, poll will set fds[c].revents to zero.

		if(fds[c].fd == INVALID_SOCKET)
			continue;

		Connection *cptr = &server->connections[c];
		if(fds[c].revents){
			ASSERT(!(fds[c].revents & POLLNVAL));
			if(fds[c].revents & (POLLHUP | POLLERR)){
				connection_close(cptr);
			}else{
				if(fds[c].revents & POLLIN)
					connection_resume_reading(c, cptr, server->on_read, userdata);
				if(fds[c].revents & POLLOUT)
					connection_resume_writing(c, cptr, server->request_output, userdata);
			}
		}

		// TODO: add connection timeout here

		if(!cptr->closing){
			ConnectionStatus status = CONNECTION_STATUS_ALIVE;
			server->request_status(userdata, c, &status);
			cptr->closing = (status == CONNECTION_STATUS_CLOSING);
		}

		if(cptr->closing && cptr->bytes_to_write == 0){
			// TODO: A connection_abort here won't always work because
			// it will cause packet loss in cases where we write and
			// disconnect. What can be done instead is to have a delayed
			// connection_abort. Note that having the server (us) start
			// a TCP close will lead to sockets sitting in TIME_WAIT for
			// ~5 mins which could be used for a DDOS attack.
			//connection_abort(cptr);

			connection_close(cptr);
		}

		if(cptr->closed){
			server->on_drop(userdata, c);
			server_free_connection(server, c);
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
