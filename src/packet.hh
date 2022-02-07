// NOTE: This is a helper struct to make it simpler to parse and write
// Tibia client packets that contains variable size structures like
// strings and arrays. It assumes multi-byte data is in little endian.

#ifndef KAPLAR_PACKET_HH_
#define KAPLAR_PACKET_HH_ 1

#include "common.hh"
#include "buffer_util.hh"

struct Packet{
	u8 *buf;
	i32 bufend;
	i32 bufpos;
};

static INLINE
Packet make_packet(u8 *buf, i32 buflen){
	Packet result;
	result.buf = buf;
	result.bufend = buflen;
	result.bufpos = 0;
	return result;
}

static INLINE
bool packet_ok(Packet *p){
	return p->bufpos <= p->bufend;
}

static INLINE
u8 *packet_buf(Packet *p){
	return p->buf;
}

static INLINE
i32 packet_written_len(Packet *p){
	return packet_ok(p) ? p->bufpos : p->bufend;
}

static INLINE
i32 packet_remainder(Packet *p){
	return packet_ok(p) ? (p->bufend - p->bufpos) : 0;
}

static INLINE
bool packet_can_read(Packet *p, i32 bytes){
	return (p->bufend - p->bufpos) >= bytes;
}

static INLINE
bool packet_can_write(Packet *p, i32 bytes){
	return (p->bufend - p->bufpos) >= bytes;
}

static INLINE
u8 packet_read_u8(Packet *p){
	u8 result = 0;
	if(packet_can_read(p, 1))
		result = buffer_read_u8(p->buf + p->bufpos);
	p->bufpos += 1;
	return result;
}

static INLINE
u16 packet_read_u16(Packet *p){
	u16 result = 0;
	if(packet_can_read(p, 2))
		result = buffer_read_u16_le(p->buf + p->bufpos);
	p->bufpos += 2;
	return result;
}

static INLINE
u32 packet_read_u32(Packet *p){
	u32 result = 0;
	if(packet_can_read(p, 4))
		result = buffer_read_u32_le(p->buf + p->bufpos);
	p->bufpos += 4;
	return result;
}

static INLINE
void packet_read_string(Packet *p, u16 maxlen, char *outstr){
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

static INLINE
void packet_write_u8(Packet *p, u8 val){
	if(packet_can_write(p, 1))
		buffer_write_u8(p->buf + p->bufpos, val);
	p->bufpos += 1;
}

static INLINE
void packet_write_u16(Packet *p, u16 val){
	if(packet_can_write(p, 2))
		buffer_write_u16_le(p->buf + p->bufpos, val);
	p->bufpos += 2;
}

static INLINE
void packet_write_u32(Packet *p, u32 val){
	if(packet_can_write(p, 4))
		buffer_write_u32_le(p->buf + p->bufpos, val);
	p->bufpos += 4;
}

static INLINE
void packet_write_lstr(Packet *p, const char *s, u16 len){
	// NOTE: Either write the whole string or none of it.
	// Same considerations as in packet_read_str.

	packet_write_u16(p, len);
	if(len > 0 && packet_can_write(p, len))
		memcpy(p->buf + p->bufpos, s, len);
	p->bufpos += len;
}

static INLINE
void packet_write_str(Packet *p, const char *s){
	packet_write_lstr(p, s, (u16)strlen(s));
}

#endif //KAPLAR_PACKET_HH_