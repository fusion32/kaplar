
// NOTE: Since we are using a different approach to world tile management
// we might as well do a simple "custom" file format that will hold the data
// in the format we want/expect. The only thing left to do is to write a tool
// to convert from the old OTBM format.
//	As for the name, I'm thinking of:
//		.KWR - Kaplar World for the world file.
//		.KCD - Kaplar Client Data to serve the same purpose of the
//			old items.otb which was to basically hold client info
//			about the items. (client_id to server_id, is stackable,
//			blocks the path, etc...
//
//	For the file structure, I'm thinking of something like EBML elements.
// They should have an identifier and a size followed by either binary data
// or inner elements. For example:
//	+--------+---+-----------------------------------------+
//	| ID_FOO | 8 | 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 |
//	+--------+---+-----------------------------------------+

#ifndef KAPLAR_FILE_HH_
#define KAPLAR_FILE_HH_ 1

#include "common.hh"
#include "buffer_util.hh"

struct KPB_Element{
	u8 *buf;
	i32 bufend;
	i32 bufpos;
	u32 elem_id;
};

static INLINE
i32 kpb_element_remainder(KPB_Element *elem){
	return (elem->bufend >= elem->bufpos) ? (elem->bufend - elem->bufpos) : 0;
}

static INLINE
bool kpb_element_can_read(KPB_Element *elem, i32 bytes){
	return (elem->bufend - elem->bufpos) >= bytes;
}

static INLINE
u8 kpb_element_read_u8(KPB_Element *elem){
	u8 result = 0;
	if(kpb_element_can_read(elem, 1))
		result = buffer_read_u8(elem->buf + elem->bufpos);
	elem->bufpos += 1;
	return result;
}

static INLINE
u16 kpb_element_read_u16(KPB_Element *elem){
	u16 result = 0;
	if(kpb_element_can_read(elem, 2))
		result = buffer_read_u16_le(elem->buf + elem->bufpos);
	elem->bufpos += 2;
	return result;
}

static INLINE
u32 kpb_element_read_u32(KPB_Element *elem){
	u32 result = 0;
	if(kpb_element_can_read(elem, 4))
		result = buffer_read_u32_le(elem->buf + elem->bufpos);
	elem->bufpos += 4;
	return result;
}

static INLINE
u64 kpb_element_read_u64(KPB_Element *elem){
	u64 result = 0;
	if(kpb_element_can_read(elem, 8))
		result = buffer_read_u64_le(elem->buf + elem->bufpos);
	elem->bufpos += 8;
	return result;
}

static INLINE
KPB_Element kpb_top_level(u8 *fdata, i32 fsize){
	KPB_Element elem;
	elem.buf = fdata;
	elem.bufend = fsize;
	elem.bufpos = 0;
	elem.elem_id = 0;
	return elem;
}

static
bool kpb_next_element(KPB_Element *parent, KPB_Element *out_elem){
	ASSERT(out_elem != NULL);
	if(!kpb_element_can_read(parent, 8))
		return false;
	u32 elem_id = kpb_element_read_u32(parent);
	u32 elem_size = kpb_element_read_u32(parent);
	if(!kpb_element_can_read(parent, elem_size))
		return false;
	u8 *elem_start = parent->buf + parent->bufpos;
	parent->bufpos += elem_size;

	out_elem->buf = elem_start;
	out_elem->bufend = elem_size;
	out_elem->bufpos = 0;
	out_elem->elem_id = elem_id;
	return true;
}

// NOTE: It is better to have all element ids in one place to avoid collisions.

// Common IDs (0x??4D4F43)
#define KPB_ID_CHECKSUM				0xFF4D4F43

// World IDs (0x??52574B)
#define KPB_ID_WORLD_INFO			0x0052574B
#define KPB_ID_WORLD_SPARSE_DATA	0x0152574B
#define KPB_ID_WORLD_DENSE_DATA		0x0252574B
#define KPB_ID_WORLD_SPAWN			0x0352574B
#define KPB_ID_WORLD_TEMPLE			0x0452574B

// Client Data IDs (0x??44434B)
#define KPB_ID_CLDATA				0x0044434B

#endif // KAPLAR_FILE_HH_
