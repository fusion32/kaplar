// NOTE: This is a helper struct to make it simpler to parse and write
// Tibia client packets that contains variable size structures like
// strings and arrays. It assumes multi-byte data is in little endian.

#ifndef KAPLAR_PACKET_HH_
#define KAPLAR_PACKET_HH_ 1

#include "common.hh"
#include "buffer_util.hh"

// ----------------------------------------------------------------
// In Packet
// ----------------------------------------------------------------

struct InPacket{
	u8 *buf;
	i32 bufend;
	i32 bufpos;
};

static INLINE
InPacket in_packet(u8 *buf, i32 buflen){
	InPacket result;
	result.buf = buf;
	result.bufend = buflen;
	result.bufpos = 0;
	return result;
}

static INLINE
bool packet_ok(InPacket *p){
	return p->bufpos <= p->bufend;
}

static INLINE
u8 *packet_buf(InPacket *p){
	return p->buf;
}

static INLINE
i32 packet_remainder(InPacket *p){
	return packet_ok(p) ? (p->bufend - p->bufpos) : 0;
}

static INLINE
bool packet_can_read(InPacket *p, i32 bytes){
	return (p->bufend - p->bufpos) >= bytes;
}

static INLINE
u8 packet_read_u8(InPacket *p){
	u8 result = 0;
	if(packet_can_read(p, 1))
		result = buffer_read_u8(p->buf + p->bufpos);
	p->bufpos += 1;
	return result;
}

static INLINE
u16 packet_read_u16(InPacket *p){
	u16 result = 0;
	if(packet_can_read(p, 2))
		result = buffer_read_u16_le(p->buf + p->bufpos);
	p->bufpos += 2;
	return result;
}

static INLINE
u32 packet_read_u32(InPacket *p){
	u32 result = 0;
	if(packet_can_read(p, 4))
		result = buffer_read_u32_le(p->buf + p->bufpos);
	p->bufpos += 4;
	return result;
}

static INLINE
void packet_read_string(InPacket *p, u16 maxlen, char *outstr){
	// NOTE1: We either retrieve the whole string or none of it.
	// Not sure if this can/will prevent exploits.

	// NOTE2: maxlen > len is correct because we need to append
	// a nul-terminator.

	u16 len = packet_read_u16(p);
	if(len > 0 && maxlen > len && packet_can_read(p, len)){
		memcpy(outstr, p->buf + p->bufpos, len);
		outstr[len] = 0;
	}else if(maxlen > 0 && outstr){
		outstr[0] = 0;
	}
	p->bufpos += len;
}

// ----------------------------------------------------------------
// Out Packet
// ----------------------------------------------------------------

struct OutPacket{
	OutPacket *next;
	u8 *buf;
	i32 bufend;
	i32 bufpos;
};

static INLINE
bool packet_ok(OutPacket *p){
	return p->bufpos <= p->bufend;
}

static INLINE
u8 *packet_buf(OutPacket *p){
	return p->buf;
}

static INLINE
i32 packet_written_len(OutPacket *p){
	return packet_ok(p) ? p->bufpos : p->bufend;
}

static INLINE
bool packet_can_write(OutPacket *p, i32 bytes){
	return (p->bufend - p->bufpos) >= bytes;
}

static INLINE
void packet_write_u8(OutPacket *p, u8 val){
	if(packet_can_write(p, 1))
		buffer_write_u8(p->buf + p->bufpos, val);
	p->bufpos += 1;
}

static INLINE
void packet_write_u16(OutPacket *p, u16 val){
	if(packet_can_write(p, 2))
		buffer_write_u16_le(p->buf + p->bufpos, val);
	p->bufpos += 2;
}

static INLINE
void packet_write_u32(OutPacket *p, u32 val){
	if(packet_can_write(p, 4))
		buffer_write_u32_le(p->buf + p->bufpos, val);
	p->bufpos += 4;
}

static INLINE
void packet_write_lstr(OutPacket *p, const char *s, u16 len){
	// NOTE: Either write the whole string or none of it.
	// Same considerations as in packet_read_str.

	packet_write_u16(p, len);
	if(len > 0 && packet_can_write(p, len))
		memcpy(p->buf + p->bufpos, s, len);
	p->bufpos += len;
}

static INLINE
void packet_write_str(OutPacket *p, const char *s){
	packet_write_lstr(p, s, (u16)strlen(s));
}

#endif //KAPLAR_PACKET_HH_