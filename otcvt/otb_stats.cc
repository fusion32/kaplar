#include "common.hh"

enum OTB_ClientVersion : u8 {
	OTB_CLIENT_VERSION_750 = 1,
	OTB_CLIENT_VERSION_755 = 2,
	OTB_CLIENT_VERSION_760 = 3,
	OTB_CLIENT_VERSION_770 = 3,
	OTB_CLIENT_VERSION_780 = 4,
	OTB_CLIENT_VERSION_790 = 5,
	OTB_CLIENT_VERSION_792 = 6,
	OTB_CLIENT_VERSION_800 = 7,
	OTB_CLIENT_VERSION_810 = 8,
	OTB_CLIENT_VERSION_811 = 9,
	OTB_CLIENT_VERSION_820 = 10,
	OTB_CLIENT_VERSION_830 = 11,
	OTB_CLIENT_VERSION_840 = 12,
	OTB_CLIENT_VERSION_841 = 13,
	OTB_CLIENT_VERSION_842 = 14,
	OTB_CLIENT_VERSION_850 = 15,
	OTB_CLIENT_VERSION_854_BAD = 16,
	OTB_CLIENT_VERSION_854 = 17,
	OTB_CLIENT_VERSION_855 = 18,
	OTB_CLIENT_VERSION_860_OLD = 19,
	OTB_CLIENT_VERSION_860 = 20,
	OTB_CLIENT_VERSION_861 = 21,
	OTB_CLIENT_VERSION_862 = 22,
	OTB_CLIENT_VERSION_870 = 23,
};

enum OTB_ItemGroup : u8 {
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
	OTB_GROUP_DEPRECATED,
	OTB_GROUP_LAST,
};

enum OTB_ItemAttrib : u8 {
	OTB_ATTRIB_FIRST = 0x10,
	OTB_ATTRIB_SERVER_ID = OTB_ATTRIB_FIRST,
	OTB_ATTRIB_CLIENT_ID,
	OTB_ATTRIB_NAME,		// deprecated
	OTB_ATTRIB_DESCR,		// deprecated
	OTB_ATTRIB_SPEED,
	OTB_ATTRIB_SLOT,		// deprecated
	OTB_ATTRIB_MAXITEMS,	// deprecated
	OTB_ATTRIB_WEIGHT,		// deprecated
	OTB_ATTRIB_WEAPON,		// deprecated
	OTB_ATTRIB_AMU,			// deprecated
	OTB_ATTRIB_ARMOR,		// deprecated
	OTB_ATTRIB_MAGLEVEL,	// deprecated
	OTB_ATTRIB_MAGFIELD_TYPE,	// deprecated
	OTB_ATTRIB_WRITEABLE,	// deprecated
	OTB_ATTRIB_ROTATETO,	// deprecated
	OTB_ATTRIB_DECAY,		// deprecated
	OTB_ATTRIB_SPRITE_HASH,
	OTB_ATTRIB_MINIMAP_COLOR,
	OTB_ATTRIB_07,
	OTB_ATTRIB_08,
	OTB_ATTRIB_LIGHT,
	OTB_ATTRIB_DECAY2,		// deprecated
	OTB_ATTRIB_WEAPON2,		// deprecated
	OTB_ATTRIB_AMU2,		// deprecated
	OTB_ATTRIB_ARMOR2,		// deprecated
	OTB_ATTRIB_WRITEABLE2,	// deprecated
	OTB_ATTRIB_LIGHT2,
	OTB_ATTRIB_TOP_ORDER,
	OTB_ATTRIB_WRITEABLE3,	// deprecated
	OTB_ATTRIB_LAST,
};

enum OTB_ItemFlags : u32 {
	OTB_FLAG_BLOCK_SOLID = (1 << 0),
	OTB_FLAG_BLOCK_PROJECTILE = (1 << 1),
	OTB_FLAG_BLOCK_PATHFIND = (1 << 2),
	OTB_FLAG_HAS_HEIGHT = (1 << 3),
	OTB_FLAG_USEABLE = (1 << 4),
	OTB_FLAG_PICKUPABLE = (1 << 5),
	OTB_FLAG_MOVEABLE = (1 << 6),
	OTB_FLAG_STACKABLE = (1 << 7),
	OTB_FLAG_FLOOR_CHANGE_DOWN = (1 << 8),
	OTB_FLAG_FLOOR_CHANGE_NORTH = (1 << 9),
	OTB_FLAG_FLOOR_CHANGE_EAST = (1 << 10),
	OTB_FLAG_FLOOR_CHANGE_SOUTH = (1 << 11),
	OTB_FLAG_FLOOR_CHANGE_WEST = (1 << 12),
	OTB_FLAG_ALWAYS_ON_TOP = (1 << 13),
	OTB_FLAG_READABLE = (1 << 14),
	OTB_FLAG_ROTABLE = (1 << 15),
	OTB_FLAG_HANGABLE = (1 << 16),
	OTB_FLAG_VERTICAL = (1 << 17),
	OTB_FLAG_HORIZONTAL = (1 << 18),
	OTB_FLAG_CANNOT_DECAY = (1 << 19),
	OTB_FLAG_ALLOW_DIST_READ = (1 << 20),
	OTB_FLAG_UNUSED = (1 << 21),
	OTB_FLAG_CLIENT_CHARGES = (1 << 22),	// deprecated
	OTB_FLAG_LOOKTHROUGH = (1 << 23),

	OTB_FLAG_LAST = OTB_FLAG_LOOKTHROUGH,
	OTB_FLAGS_MASK = (OTB_FLAG_LAST << 1) - 1,
};

struct OTB_LightInfo{
	u16 light_level;
	u16 light_color;
};

struct OTB_Reader{
	u8 *buf;
	i32 bufend;
	i32 bufpos;
};

static
bool otb_ok(OTB_Reader *r){
	return r->bufpos <= r->bufend;
}

static
bool otb_can_read(OTB_Reader *r, i32 bytes){
	return (r->bufend - r->bufpos) >= bytes;
}

static
void otb_skip(OTB_Reader *r, i32 bytes){
	r->bufpos += bytes;
}

static
u8 otb_read_u8(OTB_Reader *r){
	u8 result = 0;
	if(otb_can_read(r, 1))
		result = buffer_read_u8(r->buf + r->bufpos);
	r->bufpos += 1;
	return result;
}

static
u16 otb_read_u16(OTB_Reader *r){
	u16 result = 0;
	if(otb_can_read(r, 2))
		result = buffer_read_u16_le(r->buf + r->bufpos);
	r->bufpos += 2;
	return result;
}

static
u32 otb_read_u32(OTB_Reader *r){
	u32 result = 0;
	if(otb_can_read(r, 4))
		result = buffer_read_u32_le(r->buf + r->bufpos);
	r->bufpos += 4;
	return result;
}

static
void otb_read_string(OTB_Reader *r, u16 maxlen, char *outstr){
	// NOTE: Either read the whole string or none of it.
	u16 len = otb_read_u16(r);
	if(len > 0 && maxlen > len && otb_can_read(r, len)){
		memcpy(outstr, r->buf + r->bufpos, len);
		outstr[len] = 0;
	}else if(maxlen > 0 && outstr){
		outstr[0] = 0;
	}
	r->bufpos += len;
}

int main(int argc, char **argv){
	const char *filename = "items.otb";
	if(argc >= 2)
		filename = argv[1];

	enum{
		OTB_ESCAPE = 0xFD,
		OTB_NODE_START = 0xFE,
		OTB_NODE_END = 0xFF,
	};

	OTB_Reader otb = {};
	otb.buf = (u8*)read_entire_file(filename, 0, &otb.bufend);
	if(!otb.buf){
		printf("failed to open file \"%s\"\n", filename);
		return -1;
	}

	{
		// NOTE: Remove all OTB_ESCAPE characters. We can do this
		// because the node structure is known and because their
		// attributes have their size prepended.
		i32 insert_pos = 0;
		i32 scan_pos = 0;
		while(scan_pos < (otb.bufend - 1)){
			if(otb.buf[scan_pos] == OTB_ESCAPE){
				otb.buf[insert_pos] = otb.buf[scan_pos + 1];
				scan_pos += 2;
			}else{
				otb.buf[insert_pos] = otb.buf[scan_pos];
				scan_pos += 1;
			}
			insert_pos += 1;
		}
		// NOTE: Handle last byte.
		if(scan_pos < otb.bufend && otb.buf[scan_pos] != OTB_ESCAPE){
			otb.buf[insert_pos] = otb.buf[scan_pos];
			insert_pos += 1;
		}
		otb.bufend = insert_pos;
	}

	if(otb_read_u32(&otb) != 0				// some kind of version (?), always zero
	|| otb_read_u8(&otb) != OTB_NODE_START	// start of root node
	|| otb_read_u8(&otb) != OTB_GROUP_NONE	// type of root node
	|| otb_read_u32(&otb) != 0				// flags for root node (?), always zero
	|| otb_read_u8(&otb) != 1				// attrib version
	|| otb_read_u16(&otb) != 140){			// attrib version size
		printf("invalid OTB format\n");
		return -1;
	}

	u32 version = otb_read_u32(&otb);
	u32 client_version = otb_read_u32(&otb);
	otb_read_u32(&otb); // same as client_version (?)
	otb_skip(&otb, 128); // ?

	if(version != 3 && client_version != OTB_CLIENT_VERSION_870){
		printf("invalid OTB version\n");
		return -1;
	}

	// group stats
	i32 num_group_none = 0;
	i32 num_group_ground = 0;
	i32 num_group_container = 0;
	i32 num_group_charges = 0;
	i32 num_group_splash = 0;
	i32 num_group_fluid = 0;
	i32 num_group_deprecated = 0;
	i32 num_group_other = 0;

	// attrib stats
	i32 num_attrib_server_id = 0;
	i32 num_attrib_client_id = 0;
	i32 num_attrib_speed = 0;
	i32 num_attrib_sprite_hash = 0;
	i32 num_attrib_minimap_color = 0;
	i32 num_attrib_07 = 0;
	i32 num_attrib_08 = 0;
	i32 num_attrib_light = 0;
	i32 num_attrib_light2 = 0;
	i32 num_attrib_top_order = 0;
	i32 num_attrib_other = 0;

	// flag stats
	i32 num_flag_block_solid = 0;
	i32 num_flag_block_projectile = 0;
	i32 num_flag_block_pathfind = 0;
	i32 num_flag_has_height = 0;
	i32 num_flag_useable = 0;
	i32 num_flag_pickupable = 0;
	i32 num_flag_moveable = 0;
	i32 num_flag_stackable = 0;
	i32 num_flag_floor_change_down = 0;
	i32 num_flag_floor_change_north = 0;
	i32 num_flag_floor_change_east = 0;
	i32 num_flag_floor_change_south = 0;
	i32 num_flag_floor_change_west = 0;
	i32 num_flag_always_on_top = 0;
	i32 num_flag_readable = 0;
	i32 num_flag_rotable = 0;
	i32 num_flag_hangable = 0;
	i32 num_flag_vertical = 0;
	i32 num_flag_horizontal = 0;
	i32 num_flag_cannot_decay = 0;
	i32 num_flag_allow_dist_read = 0;
	i32 num_flag_unused = 0;
	i32 num_flag_client_charges = 0;
	i32 num_flag_lookthrough = 0;
	i32 num_flag_other = 0;

	// overall stats
	i32 num_nodes = 0;
	u16 max_server_id = 0;
	u16 max_client_id = 0;
	u16 max_speed = 0;
	u16 max_light_level = 0;
	u16 max_light_color = 0;
	u16 max_top_order = 0;

	while(otb_read_u8(&otb) == OTB_NODE_START && otb_ok(&otb)){
		u8 type = otb_read_u8(&otb);
		u32 flags = otb_read_u32(&otb);

		switch(type){
			case OTB_GROUP_NONE:
				num_group_none += 1;
				break;
			case OTB_GROUP_GROUND:
				num_group_ground += 1;
				break;
			case OTB_GROUP_CONTAINER:
				num_group_container += 1;
				break;
			case OTB_GROUP_CHARGES:
				num_group_charges += 1;
				break;
			case OTB_GROUP_SPLASH:
				num_group_splash += 1;
				break;
			case OTB_GROUP_FLUID:
				num_group_fluid += 1;
				break;
			case OTB_GROUP_DEPRECATED:
				num_group_deprecated += 1;
				break;
			default:
				num_group_other += 1;
				break;
		}

		if(flags & OTB_FLAG_BLOCK_SOLID)
			num_flag_block_solid += 1;
		if(flags & OTB_FLAG_BLOCK_PROJECTILE)
			num_flag_block_projectile += 1;
		if(flags & OTB_FLAG_BLOCK_PATHFIND)
			num_flag_block_pathfind += 1;
		if(flags & OTB_FLAG_HAS_HEIGHT)
			num_flag_has_height += 1;
		if(flags & OTB_FLAG_USEABLE)
			num_flag_useable += 1;
		if(flags & OTB_FLAG_PICKUPABLE)
			num_flag_pickupable += 1;
		if(flags & OTB_FLAG_MOVEABLE)
			num_flag_moveable += 1;
		if(flags & OTB_FLAG_STACKABLE)
			num_flag_stackable += 1;
		if(flags & OTB_FLAG_FLOOR_CHANGE_DOWN)
			num_flag_floor_change_down += 1;
		if(flags & OTB_FLAG_FLOOR_CHANGE_NORTH)
			num_flag_floor_change_north += 1;
		if(flags & OTB_FLAG_FLOOR_CHANGE_EAST)
			num_flag_floor_change_east += 1;
		if(flags & OTB_FLAG_FLOOR_CHANGE_SOUTH)
			num_flag_floor_change_south += 1;
		if(flags & OTB_FLAG_FLOOR_CHANGE_WEST)
			num_flag_floor_change_west += 1;
		if(flags & OTB_FLAG_ALWAYS_ON_TOP)
			num_flag_always_on_top += 1;
		if(flags & OTB_FLAG_READABLE)
			num_flag_readable += 1;
		if(flags & OTB_FLAG_ROTABLE)
			num_flag_rotable += 1;
		if(flags & OTB_FLAG_HANGABLE)
			num_flag_hangable += 1;
		if(flags & OTB_FLAG_VERTICAL)
			num_flag_vertical += 1;
		if(flags & OTB_FLAG_HORIZONTAL)
			num_flag_horizontal += 1;
		if(flags & OTB_FLAG_CANNOT_DECAY)
			num_flag_cannot_decay += 1;
		if(flags & OTB_FLAG_ALLOW_DIST_READ)
			num_flag_allow_dist_read += 1;
		if(flags & OTB_FLAG_UNUSED)
			num_flag_unused += 1;
		if(flags & OTB_FLAG_CLIENT_CHARGES)
			num_flag_client_charges += 1;
		if(flags & OTB_FLAG_LOOKTHROUGH)
			num_flag_lookthrough += 1;
		if(flags & ~OTB_FLAGS_MASK)
			num_flag_other += 1;

		u16 server_id = 0;
		u16 client_id = 0;
		u16 speed = 0;
		u16 light_level = 0;
		u16 light_color = 0;
		u8 top_order = 0;
		while(otb_ok(&otb)){
			u8 attrib = otb_read_u8(&otb);
			if(attrib == OTB_NODE_END)
				break;
			u16 attrib_size = otb_read_u16(&otb);
			switch(attrib){
				case OTB_ATTRIB_SERVER_ID: {
					if(attrib_size != 2){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_SERVER_ID (%u)\n", attrib_size);
						return -1;
					}
					server_id = otb_read_u16(&otb);
					num_attrib_server_id += 1;
					break;
				}

				case OTB_ATTRIB_CLIENT_ID: {
					if(attrib_size != 2){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_CLIENT_ID (%u)\n", attrib_size);
						return -1;
					}
					client_id = otb_read_u16(&otb);
					num_attrib_client_id += 1;
					break;
				}

				case OTB_ATTRIB_SPEED: {
					if(attrib_size != 2){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_SPEED (%u)\n", attrib_size);
						return -1;
					}
					speed = otb_read_u16(&otb);
					num_attrib_speed += 1;
					break;
				}

				case OTB_ATTRIB_LIGHT2: {
					if(attrib_size != 4){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_LIGHT2 (%u)\n", attrib_size);
						return -1;
					}
					light_level = otb_read_u16(&otb);
					light_color = otb_read_u16(&otb);
					num_attrib_light2 += 1;
					break;
				}

				case OTB_ATTRIB_TOP_ORDER: {
					if(attrib_size != 1){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_TOP_ORDER (%u)\n", attrib_size);
						return -1;
					}
					top_order = otb_read_u8(&otb);
					num_attrib_top_order += 1;
					break;
				}

				default: {
					switch(attrib){
						case OTB_ATTRIB_SPRITE_HASH:
							num_attrib_sprite_hash += 1;
							break;
						case OTB_ATTRIB_MINIMAP_COLOR:
							num_attrib_minimap_color += 1;
							break;
						case OTB_ATTRIB_07:
							num_attrib_07 += 1;
							break;
						case OTB_ATTRIB_08:
							num_attrib_08 += 1;
							break;
						case OTB_ATTRIB_LIGHT:
							num_attrib_light += 1;
							break;
						default:
							num_attrib_other += 1;
							break;
					}

					otb_skip(&otb, attrib_size);
					break;
				}
			}
		}

		if(server_id > max_server_id)
			max_server_id = server_id;
		if(client_id > max_client_id)
			max_client_id = client_id;
		if(speed > max_speed)
			max_speed = speed;
		if(light_level > max_light_level)
			max_light_level = light_level;
		if(light_color > max_light_color)
			max_light_color = light_color;
		if(top_order > max_top_order)
			max_top_order = top_order;

		num_nodes += 1;
	}

	// NOTE: Check that we parsed the whole file after removing escape codes
	// and that the last byte is an OTB_NODE_END related to the root node.
	if(otb.bufpos != otb.bufend || otb.buf[otb.bufpos - 1] != OTB_NODE_END)
		printf("OTB file parsed but got unexpected file ending\n");

	// group stats
	printf("group stats:\n");
	printf("\tnum_group_none = %d\n", num_group_none);
	printf("\tnum_group_ground = %d\n", num_group_ground);
	printf("\tnum_group_container = %d\n", num_group_container);
	printf("\tnum_group_charges = %d\n", num_group_charges);
	printf("\tnum_group_splash = %d\n", num_group_splash);
	printf("\tnum_group_fluid = %d\n", num_group_fluid);
	printf("\tnum_group_deprecated = %d\n", num_group_deprecated);
	printf("\tnum_group_other = %d\n", num_group_other);

	// attrib stats
	printf("attrib stats:\n");
	printf("\tnum_attrib_server_id = %d\n", num_attrib_server_id);
	printf("\tnum_attrib_client_id = %d\n", num_attrib_client_id);
	printf("\tnum_attrib_speed = %d\n", num_attrib_speed);
	printf("\tnum_attrib_sprite_hash = %d\n", num_attrib_sprite_hash);
	printf("\tnum_attrib_minimap_color = %d\n", num_attrib_minimap_color);
	printf("\tnum_attrib_07 = %d\n", num_attrib_07);
	printf("\tnum_attrib_08 = %d\n", num_attrib_08);
	printf("\tnum_attrib_light = %d\n", num_attrib_light);
	printf("\tnum_attrib_light2 = %d\n", num_attrib_light2);
	printf("\tnum_attrib_top_order = %d\n", num_attrib_top_order);
	printf("\tnum_attrib_other = %d\n", num_attrib_other);

	// flag stats
	printf("flag stats:\n");
	printf("\tnum_flag_block_solid = %d\n", num_flag_block_solid);
	printf("\tnum_flag_block_projectile = %d\n", num_flag_block_projectile);
	printf("\tnum_flag_block_pathfind = %d\n", num_flag_block_pathfind);
	printf("\tnum_flag_has_height = %d\n", num_flag_has_height);
	printf("\tnum_flag_useable = %d\n", num_flag_useable);
	printf("\tnum_flag_pickupable = %d\n", num_flag_pickupable);
	printf("\tnum_flag_moveable = %d\n", num_flag_moveable);
	printf("\tnum_flag_stackable = %d\n", num_flag_stackable);
	printf("\tnum_flag_floor_change_down = %d\n", num_flag_floor_change_down);
	printf("\tnum_flag_floor_change_north = %d\n", num_flag_floor_change_north);
	printf("\tnum_flag_floor_change_east = %d\n", num_flag_floor_change_east);
	printf("\tnum_flag_floor_change_south = %d\n", num_flag_floor_change_south);
	printf("\tnum_flag_floor_change_west = %d\n", num_flag_floor_change_west);
	printf("\tnum_flag_always_on_top = %d\n", num_flag_always_on_top);
	printf("\tnum_flag_readable = %d\n", num_flag_readable);
	printf("\tnum_flag_rotable = %d\n", num_flag_rotable);
	printf("\tnum_flag_hangable = %d\n", num_flag_hangable);
	printf("\tnum_flag_vertical = %d\n", num_flag_vertical);
	printf("\tnum_flag_horizontal = %d\n", num_flag_horizontal);
	printf("\tnum_flag_cannot_decay = %d\n", num_flag_cannot_decay);
	printf("\tnum_flag_allow_dist_read = %d\n", num_flag_allow_dist_read);
	printf("\tnum_flag_unused = %d\n", num_flag_unused);
	printf("\tnum_flag_client_charges = %d\n", num_flag_client_charges);
	printf("\tnum_flag_lookthrough = %d\n", num_flag_lookthrough);
	printf("\tnum_flag_other = %d\n", num_flag_other);

	// overall stats
	printf("overall stats:\n");
	printf("\tnum_nodes = %d\n", num_nodes);
	printf("\tmax_server_id = %u\n", max_server_id);
	printf("\tmax_client_id = %u\n", max_client_id);
	printf("\tmax_speed = %u\n", max_speed);
	printf("\tmax_light_level = %u\n", max_light_level);
	printf("\tmax_light_color = %u\n", max_light_color);
	printf("\tmax_top_order = %u\n", max_top_order);

	return 0;
}
