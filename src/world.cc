#include "world.hh"

// ----------------------------------------------------------------
// World Loading and Initialization
// ----------------------------------------------------------------

// - WORLD_INFO
// -- WORLD_VERSION
// -- CLIENT_VERSION
// -- CLIENT_DATA_VERSION
// -- CHUNK_DIM X Y Z
// -- WORLD_DIM_IN_CHUNKS X Y Z
// -- NUM_DENSE_CHUNKS
// -- NUM_SPARSE_PAIRS
// -- NUM_SPAWNS (?)
// -- NUM_TEMPLES (?)

// NOTE: Both dense data and sparse data can be split into one or more
// WORLD_DENSE_DATA and WORLD_SPARSE_DATA elements, respectively.

// - WORLD_DENSE_DATA
// -- TILES IN SEQUENCE

// - WORLD_SPARSE_DATA
// -- (CHUNK_INDEX, DENSE_INDEX) PAIRS


#include "file.hh"

static
bool world_sparse_data(KPB_Element *elem, u32 *sparse, u32 max_sparse_index){

}

void world_load(MemArena *arena, World *world, const char *filename){
	// TODO: Use a temp arena or allocate file memory independently.
	i32 fsize;
	u8 *fbuf = read_entire_file(arena, filename, &fsize);
	if(!fbuf){
		PANIC("%s: failed to load world file", filename);
	}

	KPB_Element elem;
	KPB_Element top = kpb_top_level(fbuf, fsize);

	// NOTE: The first element on a world file should be the world info
	// so we know what to do with the data that comes after.
	if(!kpb_next_element(&top, &elem)){
		PANIC("%s: failed to read WORLD element", filename);
	}

	if(elem.elem_id != KPB_ID_WORLD_INFO){
		PANIC("%s: expected WORLD_INFO (%08X) as first element (got %08X)",
			filename, KPB_ID_WORLD_INFO, elem.elem_id);
	}

	if(kpb_element_remainder(&elem) != 16){
		PANIC("%s: unexpected size for world info element (expected = 16, got = %d)",
			filename, kpb_element_remainder(&elem));
	}

	u16 world_version = kpb_element_read_u16(&elem);
	u16 client_version = kpb_element_read_u16(&elem);

	// TODO: Verify world version and client version here.

	world->client_data_version = kpb_element_read_u16(&elem);
	world->chunk_dim_x = kpb_element_read_u16(&elem);
	world->chunk_dim_y = kpb_element_read_u16(&elem);
	world->chunk_dim_z = kpb_element_read_u8(&elem);
	world->world_dim_in_chunks_x = kpb_element_read_u16(&elem);
	world->world_dim_in_chunks_y = kpb_element_read_u16(&elem);
	world->world_dim_in_chunks_z = kpb_element_read_u8(&elem);

	u32 num_dense_chunks = kpb_element_read_u32(&elem);
	u32 num_sparse_pairs = kpb_element_read_u32(&elem);
	u32 num_spawns = kpb_element_read_u32(&elem);
	u32 num_temples = kpb_element_read_u32(&elem);

	u32 num_tiles_per_chunk =
			(u32)world->chunk_dim_x
			* (u32)world->chunk_dim_y
			* (u32)world->chunk_dim_z;

	u32 max_chunks =
			(u32)world->world_dim_in_chunks_x
			* (u32)world->world_dim_in_chunks_y
			* (u32)world->world_dim_in_chunks_z;

	if(num_dense_chunks > max_chunks){
		PANIC("%s: number of dense chunks exceeds the maximum"
			" number of chunks (num_dense_chunks = %u, max_chunks = %u)",
			filename, num_dense_chunks, max_chunks);
	}

	// TODO: Should we have room for a extra chunks? I don't think we
	// should change the world structure on the fly but I don't know.
	usize num_tiles = (usize)num_dense_chunks * (usize)world->num_tiles_per_chunk;
	world->dense_array = arena_alloc<Tile>(arena, num_tiles);
	world->sparse_array = arena_alloc<u32>(arena, max_chunks);

	while(kpb_next_element(&top, &elem)){
		switch(elem.elem_id){
			case KPB_ID_WORLD_SPARSE_DATA:{
				i32 remainder = kpb_element_remainder(&elem);
				if((remainder % 8) != 0){
					PANIC("%s: WORLD_SPARSE_DATA element expected to"
						"have a whole number of sparse pairs", filename);
				}

				while(remainder > 0){
					u32 chunk_index = kpb_element_read_u32(&elem);
					u32 dense_index = kpb_element_read_u32(&elem);
					if(chunk_index >= max_chunks){
						PANIC("%s: chunk index exceeds maximum expected value"
							" (chunk_index = %u, max_chunks = %u)",
							chunk_index, max_chunks);
					}
					world->sparse_array[chunk_index] = dense_index;
					remainder -= 8;
				}
				break;
			}

			case KPB_ID_WORLD_DENSE_DATA:{
				//
				break;
			}

			// TODO:
			case KPB_ID_WORLD_SPAWN:
			case KPB_ID_WORLD_TEMPLE:
				break;

			default:
				LOG("%s: unexpected element (%08X)", filename, elem.elem_id);
				break;
		}
	}

	// TODO: Check end of file, check if the amount of sparse pairs,
	// dense chunks, spawns, and temples match with the numbers in
	// WORLD_INFO.
}

// ----------------------------------------------------------------
// World Utility
// ----------------------------------------------------------------
Tile *world_get_tile(World *world, u16 x, u16 y, u8 z){
	u16 chunk_x = x / world->chunk_dim_x;
	u16 tile_x = x % world->chunk_dim_x;
	u16 chunk_y = y / world->chunk_dim_y;
	u16 tile_y = y % world->chunk_dim_y;
	u8 chunk_z = z / world->chunk_dim_z;
	u8 tile_z = z % world->chunk_dim_z;

	if(chunk_x > world->world_dim_in_chunks_x
	|| chunk_y > world->world_dim_in_chunks_y
	|| chunk_z > world->world_dim_in_chunks_z)
		return NULL;

	u32 chunk_pitch_y = world->world_dim_in_chunks_x;
	u32 chunk_pitch_z = world->world_dim_in_chunks_x * world->world_dim_in_chunks_y;
	u32 chunk_index = chunk_z * chunk_pitch_z
			+ chunk_y * chunk_pitch_y;
			+ chunk_x;

	u32 chunk_dense_index = world->sparse_array[chunk_index];
	if(chunk_dense_index == 0xFFFFFFFF)
		return NULL;

	// TODO: chunk_first_tile will need to be an u64 if we have enough tiles.

	u32 chunk_first_tile = chunk_dense_index * world->num_tiles_per_chunk;
	u32 tile_pitch_y = world->chunk_dim_x;
	u32 tile_pitch_z = world->chunk_dim_x * world->chunk_dim_y;
	Tile *tile = world->dense_array
			+ chunk_first_tile
			+ tile_z * tile_pitch_z
			+ tile_y * tile_pitch_y
			+ tile_x;
	
	return tile;
}

// TODO: Players (and maybe creatures?) will be kept in a separate tree structure.
// This means that we'll need to merge both tiles and creatures before sending world
// data to the client. On a quick thought, query the creatures on the same area we'll
// send the client and sort them by position. This way, when we iterate over the
// tiles, we just need to check the first creature on the sorted list and check if
// we're sending the tile on that position.

/*
void world_get_area_description(World *world, BVH *creature_bvh, WorldArea area, OutPacket *outp){
	creatures := bvh_query(creature_bvh, area);
	sort_creatures_by_position(creatures);

	creature_ptr := 0;
	for(pos in area){
		tile := world_get_tile(world, pos);
		if(!tile)
			continue;

		creature_start := creature_ptr;
		while(creatures[creature_ptr].pos == pos)
			creature_ptr += 1;
		creature_end := creature_ptr;
		packet_append_tile(outp, tile, &creatures[creature_start], creature_end - creature_start);
	}
}
*/

