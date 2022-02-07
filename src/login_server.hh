#ifndef KAPLAR_LOGIN_SERVER_HH_
#define KAPLAR_LOGIN_SERVER_HH_ 1

#include "common.hh"

struct RSA;
struct LoginServer;
LoginServer *login_server_init(MemArena *arena,
		RSA *server_rsa, u16 port, u16 max_connections);
void login_server_poll(LoginServer *lserver);

#endif //KAPLAR_LOGIN_SERVER_HH_
