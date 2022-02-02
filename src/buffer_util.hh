#ifndef KAPLAR_BUFFER_UTIL_HH_
#define KAPLAR_BUFFER_UTIL_HH_ 1

#include "common.hh"

//
// NOTE: These swap functions should be converted to
// BSWAP instructions by the compiler. If we find that
// not to be the case we can use:
//		clang/gcc
//			__builtin_bswap16
//			__builtin_bswap32
//			__builtin_bswap64
//		msvc
//			_byteswap_ushort
//			_byteswap_ulong
//			_byteswap_uint64
//

static INLINE u16 u16_bswap(u16 x){
	return (x & 0xFF00) >> 8
		| (x & 0x00FF) << 8;
}

static INLINE u32 u32_bswap(u32 x){
	return (x & 0xFF000000) >> 24
		| (x & 0x00FF0000) >> 8
		| (x & 0x0000FF00) << 8
		| (x & 0x000000FF) << 24;
}

static INLINE u64 u64_bswap(u64 x){
	return (x & 0xFF00000000000000) >> 56
		| (x & 0x00FF000000000000) >> 40
		| (x & 0x0000FF0000000000) >> 24
		| (x & 0x000000FF00000000) >> 8
		| (x & 0x00000000FF000000) << 8
		| (x & 0x0000000000FF0000) << 24
		| (x & 0x000000000000FF00) << 40
		| (x & 0x00000000000000FF) << 56;
}

#if ARCH_BIG_ENDIAN
#define u16_be_to_host(x)	(x)
#define u16_le_to_host(x)	u16_bswap(x)
#define u32_be_to_host(x)	(x)
#define u32_le_to_host(x)	u32_bswap(x)
#define u64_be_to_host(x)	(x)
#define u64_le_to_host(x)	u64_bswap(x)
#define u16_host_to_be(x)	(x)
#define u16_host_to_le(x)	u16_bswap(x)
#define u32_host_to_be(x)	(x)
#define u32_host_to_le(x)	u32_bswap(x)
#define u64_host_to_be(x)	(x)
#define u64_host_to_le(x)	u64_bswap(x)
#else //ARCH_BIG_ENDIAN
#define u16_be_to_host(x)	u16_bswap(x)
#define u16_le_to_host(x)	(x)
#define u32_be_to_host(x)	u32_bswap(x)
#define u32_le_to_host(x)	(x)
#define u64_be_to_host(x)	u64_bswap(x)
#define u64_le_to_host(x)	(x)
#define u16_host_to_be(x)	u16_bswap(x)
#define u16_host_to_le(x)	(x)
#define u32_host_to_be(x)	u32_bswap(x)
#define u32_host_to_le(x)	(x)
#define u64_host_to_be(x)	u64_bswap(x)
#define u64_host_to_le(x)	(x)
#endif //ARCH_BIG_ENDIAN

static INLINE void buffer_write_u8(u8 *buf, u8 val){
	buf[0] = val;
}

static INLINE u8 buffer_read_u8(u8 *buf){
	return buf[0];
}

#if ARCH_UNALIGNED_ACCESS
static INLINE void buffer_write_u16_be(u8 *buf, u16 val){
	*(u16*)(buf) = u16_host_to_be(val);
}

static INLINE u16 buffer_read_u16_be(u8 *buf){
	u16 val = *(u16*)(buf);
	return u16_be_to_host(val);
}

static INLINE void buffer_write_u16_le(u8 *buf, u16 val){
	*(u16*)(buf) = u16_host_to_le(val);
}

static INLINE u16 buffer_read_u16_le(u8 *buf){
	u16 val = *(u16*)(buf);
	return u16_le_to_host(val);
}

static INLINE void buffer_write_u32_be(u8 *buf, u32 val){
	*(u32*)(buf) = u32_host_to_be(val);
}

static INLINE u32 buffer_read_u32_be(u8 *buf){
	u32 val = *(u32*)(buf);
	return u32_be_to_host(val);
}

static INLINE void buffer_write_u32_le(u8 *buf, u32 val){
	*(u32*)(buf) = u32_host_to_le(val);
}

static INLINE u32 buffer_read_u32_le(u8 *buf){
	u32 val = *(u32*)(buf);
	return u32_le_to_host(val);
}

static INLINE void buffer_write_u64_be(u8 *buf, u64 val){
	*(u64*)(buf) = u64_host_to_be(val);
}

static INLINE u64 buffer_read_u64_be(u8 *buf){
	u64 val = *(u64*)(buf);
	return u64_be_to_host(val);
}

static INLINE void buffer_write_u64_le(u8 *buf, u64 val){
	*(u64*)(buf) = u64_host_to_le(val);
}

static INLINE u64 buffer_read_u64_le(u8 *buf){
	u64 val = *(u64*)(buf);
	return u64_le_to_host(val);
}

#else //ARCH_UNALIGNED_ACCESS

static INLINE void buffer_write_u16_be(u8 *buf, u16 val){
	buf[0] = (u8)(val >> 8);
	buf[1] = (u8)(val);
}

static INLINE u16 buffer_read_u16_be(u8 *buf){
	return ((u16)(buf[0]) << 8) |
		((u16)(buf[1]));
}

static INLINE void buffer_write_u16_le(u8 *buf, u16 val){
	buf[0] = (u8)(val);
	buf[1] = (u8)(val >> 8);
}

static INLINE u16 buffer_read_u16_le(u8 *buf){
	return ((u16)(buf[0])) |
		((u16)(buf[1]) << 8);
}

static INLINE void buffer_write_u32_be(u8 *buf, u32 val){
	buf[0] = (u8)(val >> 24);
	buf[1] = (u8)(val >> 16);
	buf[2] = (u8)(val >> 8);
	buf[3] = (u8)(val);
}

static INLINE u32 buffer_read_u32_be(u8 *buf){
	return ((u32)(buf[0]) << 24) |
		((u32)(buf[1]) << 16) |
		((u32)(buf[2]) << 8) |
		((u32)(buf[3]));
}

static INLINE void buffer_write_u32_le(u8 *buf, u32 val){
	buf[0] = (u8)(val);
	buf[1] = (u8)(val >> 8);
	buf[2] = (u8)(val >> 16);
	buf[3] = (u8)(val >> 24);
}

static INLINE u32 buffer_read_u32_le(u8 *buf){
	return ((u32)(buf[0])) |
		((u32)(buf[1]) << 8) |
		((u32)(buf[2]) << 16) |
		((u32)(buf[3]) << 24);
}

static INLINE void buffer_write_u64_be(u8 *buf, u64 val){
	buf[0] = (u8)(val >> 56);
	buf[1] = (u8)(val >> 48);
	buf[2] = (u8)(val >> 40);
	buf[3] = (u8)(val >> 32);
	buf[4] = (u8)(val >> 24);
	buf[5] = (u8)(val >> 16);
	buf[6] = (u8)(val >> 8);
	buf[7] = (u8)(val);
}

static INLINE u64 buffer_read_u64_be(u8 *buf){
	return ((u64)(buf[0]) << 56) |
		((u64)(buf[1]) << 48) |
		((u64)(buf[2]) << 40) |
		((u64)(buf[3]) << 32) |
		((u64)(buf[4]) << 24) |
		((u64)(buf[5]) << 16) |
		((u64)(buf[6]) << 8) |
		((u64)(buf[7]));
}

static INLINE void buffer_write_u64_le(u8 *buf, u64 val){
	buf[0] = (u8)(val);
	buf[1] = (u8)(val >> 8);
	buf[2] = (u8)(val >> 16);
	buf[3] = (u8)(val >> 24);
	buf[4] = (u8)(val >> 32);
	buf[5] = (u8)(val >> 40);
	buf[6] = (u8)(val >> 48);
	buf[7] = (u8)(val >> 56);
}

static INLINE u64 buffer_read_u64_le(u8 *buf){
	return ((u64)(buf[0])) |
		((u64)(buf[1]) << 8) |
		((u64)(buf[2]) << 16) |
		((u64)(buf[3]) << 24) |
		((u64)(buf[4]) << 32) |
		((u64)(buf[5]) << 40) |
		((u64)(buf[6]) << 48) |
		((u64)(buf[7]) << 56);
}

#endif //ARCH_UNALIGNED_ACCESS

#endif //KAPLAR_BUFFER_UTIL_HH_
