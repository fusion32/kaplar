#include "game.hh"

static
void game_load_item_types(Game *game, const char *path){
	//
}

Game *game_init(MemArena *arena){
	Game *game = arena_alloc<Game>(arena, 1);
	game->arena = arena;

	u16 max_item_types = 15000;
	game->max_item_types = max_item_types;
	game->item_types = arena_allocz<ItemType>(arena, max_item_types);
	game->client_to_server_id = arena_allocz<u16>(arena, max_item_types);
}


