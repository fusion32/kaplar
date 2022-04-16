#ifndef KAPLAR_LOGIN_SERVER_HH_
#define KAPLAR_LOGIN_SERVER_HH_ 1

#include "common.hh"

struct RSA;
struct LoginServer;
LoginServer *login_server_init(MemArena *arena, Config *cfg, RSA *login_rsa);
void login_server_poll(LoginServer *lserver);

#endif //KAPLAR_LOGIN_SERVER_HH_
