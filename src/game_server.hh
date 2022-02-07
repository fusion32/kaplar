#ifndef KAPLAR_GAME_SERVER_HH_
#define KAPLAR_GAME_SERVER_HH_ 1

#include "common.hh"

struct RSA;
struct GameServer;
GameServer *game_server_init(MemArena *arena,
		RSA *server_rsa, u16 port, u16 max_connections);
void game_server_poll(GameServer *gserver);

#endif //KAPLAR_GAME_SERVER_HH_
