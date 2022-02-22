#ifndef KAPLAR_GAME_HH_
#define KAPLAR_GAME_HH_ 1

#include "common.hh"

struct ItemType{
	// NOTE: Every item has a name. Few items have plurals or descriptions
	// so can get away with allocating them in a separate string buffer.
	char name[32];
	char *plural;
	char *description;

	u32 flags;
	u16 client_id;
	// attributes
};

struct Container;
struct Readable;
struct Item{
	u16 type;
	u16 subtype;
	union{
		Container *container;
		Readable *readable;
	};

#if 0
// TODO: Whereas in other games items may have random attributes and values,
// items in Tibia have them fixed. This means that an intance of an item is
// simply a reference to an item "type" which is the base item and a "subtype"
// which is a fluid type / color for fluids or a stack count for stackable
// items like runes, potions, etc...
//	In the future, we would like to perhaps add some flavour to items and
// make them have random and unique attributes.
	enum : u8 {
		ITEM_ATTRIBUTE_ATTACK = 0,
		ITEM_ATTRIBUTE_SPEED,
		// ...
	};
	struct ItemAttribute{
		u8 attrib;
		u8 value;
	};
	u16 num_attributes;
	ItemAttribute attributes[4];
#endif
};

struct Tile{
	Item ground;

	// TODO: Most tiles don't have more than 2 items
	// and a single creature on it. But we still need
	// to handle cases where they have.

	i32 num_items;
	union{
		Item items[2];
		Item *vitems;
	};
};

#define WORLD_CHUNK_W 32
#define WORLD_CHUNK_H 32
#define WORLD_LAYERS 15

struct WorldChunk{
	Tile tiles[WORLD_LAYERS][WORLD_CHUNK_H][WORLD_CHUNK_W];
};

struct World{
	//
	
};

struct Game{
	MemArena *arena;

	//ItemAllocator
	//CreatureAllocator

	i32 num_item_types;
	ItemType *item_types;
	u16 *client_to_server_id;

	//i32 num_monster_types;
	//MonsterType *monster_types;

	World *world;
};

#endif //KAPLAR_GAME_HH_
