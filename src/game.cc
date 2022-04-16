#include "game.hh"

static
void game_load_base_items(Game *game){
	MemArena *arena = game->arena;
	u16 max_server_id = 5000;
	u16 max_client_id = 5000;

	//game->max_server_id = max_server_id;
	//game->max_client_id = max_client_id;
	//game->base_items = arena_allocz<BaseItem>(arena, max_server_id);
	//game->client_to_server_id = arena_allocz<u16>(arena, max_client_id);

	// TODO: load items from tables or files
}

static
void game_load_base_monsters(Game *game){
	// TODO: load monsters from tables or files
}


#include "world.hh"
static
void game_load_world(Game *game){
	// TODO: load map from file
	//game->world = arena_alloc<World>(game->arena, 1);
}

Game *game_init(MemArena *arena, Config *cfg, RSA *game_rsa){
	Game *game = arena_alloc<Game>(arena, 1);
	game->arena = arena;

	game_load_base_items(game);
	game_load_base_monsters(game);
	game_load_world(game);
	game_init_server(game, cfg, game_rsa);
	//world_load(arena,
	return game;
}

#include "server.hh"
void game_update(Game *game){
	server_poll(game->server, game);
	// player_update
	// creature_update
	// pathfind_update (?)
}


