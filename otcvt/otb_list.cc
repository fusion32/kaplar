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

	printf("Listing OTB file \"%s\"\n", filename);

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

	//debug_print_buf_hex("otb.buf", otb.buf,
	//	otb.bufend > 128 ? 128 : otb.bufend);

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

	printf("version: %u\n", version);
	printf("client_version: %u\n", client_version);

	if(version != 3 && client_version != OTB_CLIENT_VERSION_870){
		printf("invalid OTB version\n");
		return -1;
	}

	while(otb_read_u8(&otb) == OTB_NODE_START && otb_ok(&otb)){
		u8 type = otb_read_u8(&otb);
		u32 flags = otb_read_u32(&otb);

		printf("node: type = %u flags = %08X ", type, flags);

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
					printf("server_id = %u ", otb_read_u16(&otb));
					break;
				}

				case OTB_ATTRIB_CLIENT_ID: {
					if(attrib_size != 2){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_CLIENT_ID (%u)\n", attrib_size);
						return -1;
					}
					printf("client_id = %u ", otb_read_u16(&otb));
					break;
				}

				case OTB_ATTRIB_SPEED: {
					if(attrib_size != 2){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_SPEED (%u)\n", attrib_size);
						return -1;
					}
					printf("speed = %u ", otb_read_u16(&otb));
					break;
				}

				case OTB_ATTRIB_LIGHT2: {
					if(attrib_size != 4){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_LIGHT2 (%u)\n", attrib_size);
						return -1;
					}
					printf("light_level = %u ", otb_read_u16(&otb));
					printf("light_color = %u ", otb_read_u16(&otb));
					break;
				}

				case OTB_ATTRIB_TOP_ORDER: {
					if(attrib_size != 1){
						printf("\nunexpected attribute size for"
							" OTB_ATTRIB_TOP_ORDER (%u)\n", attrib_size);
						return -1;
					}
					printf("top_order = %u ", otb_read_u8(&otb));
					break;
				}

				default: {
					printf("attrib %u (ignored, size = %u) ", attrib, attrib_size);
					otb_skip(&otb, attrib_size);
					break;
				}
			}
		}
		printf("\n");
	}

	// NOTE: Check that we parsed the whole file after removing escape codes
	// and that the last byte is an OTB_NODE_END related to the root node.
	if(otb.bufpos != otb.bufend || otb.buf[otb.bufpos - 1] != OTB_NODE_END)
		printf("OTB file parsed but got unexpected file ending\n");

	return 0;
}
