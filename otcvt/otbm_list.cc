//
// NOTE: When reading the OTB file we could completely ignore the file
// format, element format, etc... because it was very simple (almost an
// array inside a file). With an OTBM, we'll need to take into account
// the proper file structure. After a short inspection, it seems like
// an OTB file is almost like an EBML file but with some caveats:
//	1 - An element is enclosed by OTB_NODE_START = 0xFE and
//		OTB_NODE_END = 0xFF bytes.
//	2 - Because we are using the bytes 0xFE and 0xFF for structuring
//		the file, we'll need an escape byte OTB_ESCAPE = 0xFD in case
//		we have any of those bytes in our data.
//	3 - Every element has an id (or type) which is the byte right after 0xFE.
//	4 - Every element may have some sort of attribute list after it's
//		id (or type) but before it's children that can be structured
//		in a way specific to that element.
//
//	So each element structure looks something like this:
//		+------+----+------------+----------+------+
//		| 0xFE | id | attributes | children | 0xFF |
//		+------+----+------------+----------+------+
//
//	Note that children are elements so they are also structured in the
// same way.
//

#include "common.hh"

enum : u8 {
	OTB_ESCAPE = 0xFD,
	OTB_ELEMENT_START = 0xFE,
	OTB_ELEMENT_END = 0xFF,
};

enum OTBM_Version{
	OTBM_VERSION_1 = 0,
	OTBM_VERSION_2 = 1,
	OTBM_VERSION_3 = 2,
	OTBM_VERSION_4 = 3,
};

enum OTBM_ID{
	OTBM_ID_ROOTV1 = 1,
	OTBM_ID_MAP_DATA = 2,
	OTBM_ID_ITEM_DEF = 3,
	OTBM_ID_TILE_AREA = 4,
	OTBM_ID_TILE = 5,
	OTBM_ID_ITEM = 6,
	OTBM_ID_TILE_SQUARE = 7,
	OTBM_ID_TILE_REF = 8,
	OTBM_ID_SPAWNS = 9,
	OTBM_ID_SPAWN_AREA = 10,
	OTBM_ID_MONSTER = 11,
	OTBM_ID_TOWNS = 12,
	OTBM_ID_TOWN = 13,
	OTBM_ID_HOUSETILE = 14,
	OTBM_ID_WAYPOINTS = 15,
	OTBM_ID_WAYPOINT = 16
};

enum OTBM_AttrTypes{
	OTBM_ATTR_DESCRIPTION = 1,
	OTBM_ATTR_EXT_FILE = 2,
	OTBM_ATTR_TILE_FLAGS = 3,
	OTBM_ATTR_ACTION_ID = 4,
	OTBM_ATTR_UNIQUE_ID = 5,
	OTBM_ATTR_TEXT = 6,
	OTBM_ATTR_DESC = 7,
	OTBM_ATTR_TELE_DEST = 8,
	OTBM_ATTR_ITEM = 9,
	OTBM_ATTR_DEPOT_ID = 10,
	OTBM_ATTR_EXT_SPAWN_FILE = 11,
	OTBM_ATTR_RUNE_CHARGES = 12,
	OTBM_ATTR_EXT_HOUSE_FILE = 13,
	OTBM_ATTR_HOUSEDOORID = 14,
	OTBM_ATTR_COUNT = 15,
	OTBM_ATTR_DURATION = 16,
	OTBM_ATTR_DECAYING_STATE = 17,
	OTBM_ATTR_WRITTENDATE = 18,
	OTBM_ATTR_WRITTENBY = 19,
	OTBM_ATTR_SLEEPERGUID = 20,
	OTBM_ATTR_SLEEPSTART = 21,
	OTBM_ATTR_CHARGES = 22,
};

#if 0
#pragma pack(1)

struct OTBM_root_header{
  uint32_t version;
  uint16_t width;
  uint16_t height;
  uint32_t majorVersionItems;
  uint32_t minorVersionItems;
};

struct OTBM_TeleportDest{
  uint16_t _x;
  uint16_t _y;
  uint8_t _z;
};

struct OTBM_Tile_area_coords{
  uint16_t _x;
  uint16_t _y;
  uint8_t _z;
};

struct OTBM_Tile_coords{
  uint8_t _x;
  uint8_t _y;
};

struct OTBM_TownTemple_coords{
  uint16_t _x;
  uint16_t _y;
  uint8_t _z;
};

struct OTBM_HouseTile_coords{
  uint8_t _x;
  uint8_t _y;
  uint32_t _houseid;
};

#pragma pack()
#endif

// ----------------------------------------------------------------
// OTB Buffer
// ----------------------------------------------------------------

struct OTB_Buf{
	u8 *buf;
	i32 bufend;
	i32 bufpos;
};

OTB_Buf otb_buf(u8 *buf, i32 bufend){
	OTB_Buf b;
	b.buf = buf;
	b.bufend = bufend;
	b.bufpos = 0;
	return b;
}

static
i32 otb_buf_remainder(OTB_Buf *b){
	return b->bufend - b->bufpos;
}

static
bool otb_buf_can_read(OTB_Buf *b, i32 bytes){
	return (b->bufend - b->bufpos) >= bytes;
}

static
u8 *otb_buf_current(OTB_Buf *b){
	return b->buf + b->bufpos;
}

static
u8 otb_buf_read_u8(OTB_Buf *b){
	u8 result = 0;
	if(otb_buf_can_read(b, 1))
		result = buffer_read_u8(b->buf + b->bufpos);
	b->bufpos += 1;
	return result;
}

static
u16 otb_buf_read_u16(OTB_Buf *b){
	u16 result = 0;
	if(otb_buf_can_read(b, 2))
		result = buffer_read_u16_le(b->buf + b->bufpos);
	b->bufpos += 2;
	return result;
}

static
u32 otb_buf_read_u32(OTB_Buf *b){
	u32 result = 0;
	if(otb_buf_can_read(b, 4))
		result = buffer_read_u32_le(b->buf + b->bufpos);
	b->bufpos += 4;
	return result;
}

static
void otb_buf_read_string(OTB_Buf *b, u16 maxlen, char *outstr){
	// NOTE: Either read the whole string or none of it.
	u16 len = otb_buf_read_u16(b);
	if(len > 0 && maxlen > len && otb_buf_can_read(b, len)){
		memcpy(outstr, b->buf + b->bufpos, len);
		outstr[len] = 0;
	}else if(maxlen > 0 && outstr){
		outstr[0] = 0;
	}
	b->bufpos += len;
}

// ----------------------------------------------------------------
// OTB Element
// ----------------------------------------------------------------

struct OTB_Element{
	u8 elem_id;
	OTB_Buf attributes;
	OTB_Buf children;
};

static
OTB_Buf otb_buf_unescaped(u8 *buf, i32 bufend){
	i32 insert_pos = 0;
	i32 scan_pos = 0;
	while(scan_pos < (bufend - 1)){
		if(buf[scan_pos] == OTB_ESCAPE){
			buf[insert_pos] = buf[scan_pos + 1];
			scan_pos += 2;
		}else{
			buf[insert_pos] = buf[scan_pos];
			scan_pos += 1;
		}
		insert_pos += 1;
	}

	// handle last byte.
	if(scan_pos < bufend && buf[scan_pos] != OTB_ESCAPE){
		buf[insert_pos] = buf[scan_pos];
		insert_pos += 1;
	}
	bufend = insert_pos;
	return otb_buf(buf, bufend);
}

static
bool otb_buf_read_element(OTB_Buf *b, OTB_Element *out_elem){
	OTB_Buf tmp_b = *b;

	if(otb_buf_read_u8(&tmp_b) != OTB_ELEMENT_START)
		return false;

	u8 elem_id = otb_buf_read_u8(&tmp_b);
	u8 *attrib_start = otb_buf_current(&tmp_b);

	u8 byte = 0x00;
	while(otb_buf_remainder(&tmp_b) > 0){
		byte = otb_buf_read_u8(&tmp_b);
		if(byte == OTB_ELEMENT_START || byte == OTB_ELEMENT_END)
			break;
		if(byte == OTB_ESCAPE)
			otb_buf_read_u8(&tmp_b);
	}

	if(otb_buf_remainder(&tmp_b) < 0)
		return false;

	u8 *attrib_end = otb_buf_current(&tmp_b) - 1;
	u8 *children_start = attrib_end;
	out_elem->elem_id = elem_id;
	out_elem->attributes = otb_buf_unescaped(attrib_start,
			(i32)(attrib_end - attrib_start));

	if(byte == OTB_ELEMENT_END){
		out_elem->children = otb_buf(children_start, 0);
	}else if(byte == OTB_ELEMENT_START){
		// NOTE: We start at element depth 2 because we found two
		// OTB_ELEMENT_STARTs: one for the element we are reading
		// and one for the first child.
		i32 element_depth = 2;
		byte = 0x00;
		while(otb_buf_remainder(&tmp_b) > 0 && element_depth != 0){
			byte = otb_buf_read_u8(&tmp_b);
			switch(byte){
				case OTB_ESCAPE:
					otb_buf_read_u8(&tmp_b);
					break;
				case OTB_ELEMENT_START:
					element_depth += 1;
					break;
				case OTB_ELEMENT_END:
					element_depth -= 1;
					break;
			}
		}

		if(element_depth != 0)
			return false;

		ASSERT(byte == OTB_ELEMENT_END);
		u8 *children_end = otb_buf_current(&tmp_b) - 1;
		out_elem->children = otb_buf(children_start,
				(i32)(children_end - children_start));
	}

	*b = tmp_b;
	return true;
}

static
bool otb_root(u8 *fbuf, i32 fsize, u32 *out_magic, OTB_Element *out_root){
	OTB_Buf b = otb_buf(fbuf, fsize);
	*out_magic = otb_buf_read_u32(&b);
	return otb_buf_read_element(&b, out_root)
		&& otb_buf_remainder(&b) == 0;
}

static
bool otb_element_next_child(OTB_Element *elem, OTB_Element *out_elem){
	return otb_buf_read_element(&elem->children, out_elem);
}

// ----------------------------------------------------------------
// OTBM List
// ----------------------------------------------------------------

int main(int argc, char **argv){
	const char *filename = "map.otbm";
	if(argc >= 2)
		filename = argv[1];

	printf("Listing OTBM file \"%s\"\n", filename);

	i32 fsize;
	u8 *fbuf = (u8*)read_entire_file(filename, 0, &fsize);
	if(!fbuf)
		PANIC("%s: failed to open file", filename);

	u32 magic;
	OTB_Element root;
	if(!otb_root(fbuf, fsize, &magic, &root))
		PANIC("%s: failed to parse root element", filename);

	// root attributes
	{
		OTB_Buf *attributes = &root.attributes;
		if(otb_buf_remainder(attributes) != 16){
			PANIC("%s: unexpected size for root attributes"
				"(expected = 16, got = %d)", filename,
				otb_buf_remainder(attributes));
		}

		u32 version = otb_buf_read_u32(attributes);
		u16 width = otb_buf_read_u16(attributes);
		u16 height = otb_buf_read_u16(attributes);
		u32 major_version_items = otb_buf_read_u32(attributes);
		u32 minor_version_items = otb_buf_read_u32(attributes);

		printf("root attributes:\n");
		printf("\tversion = %u\n", version);
		printf("\twidth = %u\n", width);
		printf("\theight = %u\n", height);
		printf("\tmajor_version_items = %u\n", major_version_items);
		printf("\tminor_version_items = %u\n", minor_version_items);
	}

	OTB_Element map_data;
	if(!otb_element_next_child(&root, &map_data))
		PANIC("%s: failed to read MAP_DATA element", filename);

	if(map_data.elem_id != OTBM_ID_MAP_DATA){
		PANIC("%s: expected MAP_DATA (%02X) as first child of root (got %02X)",
			filename, OTBM_ID_MAP_DATA, map_data.elem_id);
	}

	// map_data attributes
	{
		printf("map attributes:\n");
		
		OTB_Buf *attributes = &map_data.attributes;
		while(otb_buf_remainder(attributes) > 0){
			u8 attr = otb_buf_read_u8(attributes);
			char attr_str[256];
			switch(attr){
				case OTBM_ATTR_DESCRIPTION:
					otb_buf_read_string(attributes, sizeof(attr_str), attr_str);
					printf("\tdescription = \"%s\"\n", attr_str);
					break;
				case OTBM_ATTR_EXT_SPAWN_FILE:
					otb_buf_read_string(attributes, sizeof(attr_str), attr_str);
					printf("\text_spawn_file = \"%s\"\n", attr_str);
					break;
				case OTBM_ATTR_EXT_HOUSE_FILE:
					otb_buf_read_string(attributes, sizeof(attr_str), attr_str);
					printf("\text_house_file = \"%s\"\n", attr_str);
					break;
				default:
					PANIC("%s: unknown MAP_DATA attribute %02X", filename, attr);
			}
		}
	}
	return 0;
}
