#include "common.hh"
#include "server.hh"

struct Login{
	u32 connection;
	u32 num_reads;
	u32 num_writes;
	u32 xtea_key[4];
	char accname[32];
	char password[32];

	bool writebuf_inuse;
	u8 writebuf[512];
	i32 writelen;
};

struct LoginServer{
	// NOTE: Because the login server doesn't have to
	// handle a queue system, the connection_index has
	// a one-to-one relation with the logins array.

	i32 max_logins;
	Login *logins;
	Server *server;

	i32 max_connections_writing;
	i32 num_connections_writing;
	u32 *connections_writing;

	i32 max_connections_closing;
	i32 num_connections_closing;
	u32 *connections_closing;
};

static
Login *login_server_get_connection_login(void *userdata, u32 connection){
	LoginServer *lserver = (LoginServer*)userdata;
	i32 index = connection_index(connection);
	ASSERT(index < lserver->max_logins);
	return &lserver->logins[index];
}

static
void login_server_add_connection_closing(void *userdata, u32 connection){
	LoginServer *lserver = (LoginServer*)userdata;
	i32 idx = lserver->num_connections_closing;
	if(idx < lserver->max_connections_closing){
		lserver->connections_closing[idx] = connection;
		lserver->num_connections_closing += 1;
	}
}

static
void login_server_add_connection_writing(void *userdata, u32 connection){
	LoginServer *lserver = (LoginServer*)userdata;
	i32 idx = lserver->num_connections_writing;
	if(idx < lserver->max_connections_writing){
		lserver->connections_writing[idx] = connection;
		lserver->num_connections_writing += 1;
	}
}

static
void login_server_accept(void *userdata, u32 connection){
	LOG("%08X", connection);

	Login *login = login_server_get_connection_login(userdata, connection);
	login->connection = connection;
	login->num_reads = 0;
	login->num_writes = 0;
	login->writebuf_inuse = false;
}

static
void login_server_drop(void *userdata, u32 connection){
	LOG("%08X", connection);

	Login *login = login_server_get_connection_login(userdata, connection);
	// NOTE: Zero out login memory because it may contain sensible information.
	memset(login, 0, sizeof(Login));
}

static
void login_server_read(void *userdata, u32 connection, u8 *data, i32 datalen){
	LOG("%08X", connection);

	Login *login = login_server_get_connection_login(userdata, connection);
	//if(login->num_reads > 0){
	//	login_server_add_connection_closing(userdata, connection);
	//	return;
	//}

	if(login->writebuf_inuse)
		return;

	login->writelen = (datalen > sizeof(login->writebuf))
		? sizeof(login->writebuf) : datalen;
	memcpy(login->writebuf, data, login->writelen);

	login_server_add_connection_closing(userdata, connection);
	login_server_add_connection_writing(userdata, connection);
}

static
void login_server_write(void *userdata, u32 connection, u8 **out_write_ptr, i32 *out_write_len){
	LOG("%08X", connection);

	Login *login = login_server_get_connection_login(userdata, connection);
	//if(login->num_writes > 0){
	//	login_server_add_connection_closing(userdata, connection);
	//	return;
	//}

	if(login->writelen > 0){
		if(!login->writebuf_inuse){
			*out_write_ptr = login->writebuf;
			*out_write_len = login->writelen;
			login->writebuf_inuse = true;
		}else{
			login->writebuf_inuse = false;
			login->writelen = 0;
		}
	}
}

LoginServer *login_server_init(MemArena *arena, u16 port, u16 max_connections){
	LoginServer *lserver = arena_alloc<LoginServer>(arena, 1);
	lserver->max_logins = max_connections;
	lserver->logins = arena_alloc<Login>(arena, max_connections);

	ServerParams server_params;
	server_params.port = port;
	server_params.max_connections = max_connections;
	server_params.readbuf_size = 256;
	server_params.on_accept = login_server_accept;
	server_params.on_drop = login_server_drop;
	server_params.on_read = login_server_read;
	server_params.on_write = login_server_write;
	lserver->server = server_init(arena, &server_params);
	if(!lserver->server)
		PANIC("failed to initialize login server");

	i32 max_updates = 4 * (i32)max_connections;

	lserver->max_connections_closing = max_updates;
	lserver->num_connections_closing = 0;
	lserver->connections_closing = arena_alloc<u32>(arena, max_updates);

	lserver->max_connections_writing = max_updates;
	lserver->num_connections_writing = 0;
	lserver->connections_writing = arena_alloc<u32>(arena, max_updates);

	return lserver;
}

static int cmp_connections(const void *a, const void *b){
	return (i32)connection_index(*(u32*)a) - (i32)connection_index(*(u32*)b);
}
static void sort_connections(u32 *connections, i32 num_connections){
	qsort(connections, num_connections, sizeof(u32), cmp_connections);
}

void login_server_poll(LoginServer *lserver){
	i32 num_connections_closing = lserver->num_connections_closing;
	u32 *connections_closing = lserver->connections_closing;
	if(num_connections_closing > 0){
		sort_connections(connections_closing, num_connections_closing);
		i32 j = 1;
		for(i32 i = 1; i < num_connections_closing; i += 1){
			if(connections_closing[i] != connections_closing[i - 1])
				connections_closing[j++] = connections_closing[i];
		}
		num_connections_closing = j;
	}

	i32 num_connections_writing = lserver->num_connections_writing;
	u32 *connections_writing = lserver->connections_writing;
	if(num_connections_writing > 0){
		sort_connections(connections_writing, num_connections_writing);
		i32 j = 1;
		for(i32 i = 1; i < num_connections_writing; i += 1){
			if(connections_writing[i] != connections_writing[i - 1])
				connections_writing[j++] = connections_writing[i];
		}
		num_connections_writing = j;
	}

	// NOTE: Reset these before hand. Server callbacks will fill them back up.
	lserver->num_connections_closing = 0;
	lserver->num_connections_writing = 0;

	server_poll(lserver->server, lserver,
		connections_closing, num_connections_closing,
		connections_writing, num_connections_writing);
}
