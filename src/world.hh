#ifndef KAPLAR_WORLD_HH_
#define KAPLAR_WORLD_HH_ 1

#include "common.hh"

#if 0
	OTB_GROUP_NONE = 0,
	OTB_GROUP_GROUND,
	OTB_GROUP_CONTAINER,
	OTB_GROUP_WEAPON,		// deprecated
	OTB_GROUP_AMMUNITION,	// deprecated
	OTB_GROUP_ARMOR,		// deprecated
	OTB_GROUP_CHARGES,
	OTB_GROUP_TELEPORT,		// deprecated
	OTB_GROUP_MAGICFIELD,	// deprecated
	OTB_GROUP_WRITEABLE,	// deprecated
	OTB_GROUP_KEY,			// deprecated
	OTB_GROUP_SPLASH,
	OTB_GROUP_FLUID,
	OTB_GROUP_DOOR,			// deprecated

enum GroundChangeFloor : u8 {
	GROUND_CHANGE_FLOOR_NONE = 0,
	GROUND_CHANGE_FLOOR_UP,
	GROUND_CHANGE_FLOOR_DOWN,
};
enum GroundMoveDir : u8 {
	GROUND_MOVE_DIR_N = 0,
	GROUND_MOVE_DIR_E,
	GROUND_MOVE_DIR_S,
	GROUND_MOVE_DIR_W,
};

struct BaseGround{
	u16 speed;
	u8 change_floor;
	u8 move_dir;
};
#endif

enum ItemType : u16 {
	ITEM_TYPE_GROUND = 0,
	//ITEM_TYPE_EQUIPMENT,
};

struct BaseItem{
	// NOTE: Every item has a name. Few items have plurals or descriptions
	// so can get away with allocating them in a separate string buffer.
	char name[32];
	char *plural;
	char *description;

	u32 flags;
	u16 client_id;
	ItemType type;

	union{
		struct{ u16 speed; } ground;
	};
};

struct Container;
struct Readable;
struct Item{
	u16 id;
	u16 count_or_subtype;
	//ItemType type;
	union{
		Container *container;
		Readable *readable;
	};
};

struct Tile{
	Item ground;

	// TODO: Most tiles don't have more than 2 items
	// and a single creature on it. But we still need
	// to handle cases where they have. Let's use linked
	// lists for now and we can improve the design later.
	i32 num_items;
	Item *items_head;
	Item *items_tail;
};

struct World{
	u16 client_version;
	u16 client_data_version;

	u16 chunk_dim_x;
	u16 chunk_dim_y;
	u8 chunk_dim_z;

	u16 world_dim_in_chunks_x;
	u16 world_dim_in_chunks_y;
	u8 world_dim_in_chunks_z;

	u32 max_chunks;
	u32 num_tiles_per_chunk;

	Tile *dense_array;
	u32 *sparse_array;
};

void world_load(MemArena *arena, World *world, const char *filename);
Tile *world_get_tile(World *world, u16 x, u16 y, u8 z);

#endif //KAPLAR_WORLD_HH_