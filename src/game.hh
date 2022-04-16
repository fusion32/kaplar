#ifndef KAPLAR_GAME_HH_
#define KAPLAR_GAME_HH_ 1

#include "common.hh"

struct Client;
struct Game;
struct OutPacket;
struct RSA;
struct Server;
struct World;

// ----------------------------------------------------------------
// Game Protocol
// ----------------------------------------------------------------
void game_init_server(Game *game, Config *cfg, RSA *game_rsa);
Client *game_get_client(Game *game, u32 client_id);
void game_send_disconnect(Game *game, Client *client, const char *message);

// ----------------------------------------------------------------
// Game - game.cc
// ----------------------------------------------------------------
struct Game{
	MemArena *arena;

	// server
	u32 max_clients;
	//u32 max_players;
	Client *clients;
	//Player *players;
	RSA *rsa;
	Server *server;
	MemArena *output_arena;
	OutPacket *output_head;

	// game

	//ItemAllocator
	//CreatureAllocator

	//u16 max_server_id;
	//u16 max_client_id;
	//BaseItem *base_items;
	//u16 *client_to_server_id;

	//i32 max_base_monsters;
	//i32 num_base_monsters;
	//BaseMonster *base_monsters;

	World *world;
};

Game *game_init(MemArena *arena, Config *cfg, RSA *game_rsa);
void game_update(Game *game);

#endif //KAPLAR_GAME_HH_
