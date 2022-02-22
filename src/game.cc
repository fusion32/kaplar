#include "game.hh"

static
void game_load_item_types(Game *game, const char *path){
	//
}

Game *game_init(MemArena *arena){
	Game *game = arena_alloc<Game>(arena, 1);
	game->arena = arena;

	//
}


