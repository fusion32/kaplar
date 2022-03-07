#ifndef OTCVT_COMMON_HH_
#define OTCVT_COMMON_HH_ 1

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t i32;
typedef size_t usize;

#if defined(__GNUC__)
#	define FALLTHROUGH __attribute__((fallthrough))
#else
#	define FALLTHROUGH ((void)0)
#endif

#define ASSERT(expr)								\
	do{ if(!(expr)){								\
		printf("assertion failed \"%s\"", #expr);	\
		abort();									\
	} } while(0)

#define NARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

static
u8 buffer_read_u8(u8 *buf){
	return buf[0];
}

static
u16 buffer_read_u16_le(u8 *buf){
	return (u16)buf[0] | ((u16)buf[1] << 8);
}

static
u32 buffer_read_u32_le(u8 *buf){
	return (u32)buf[0] | ((u32)buf[1] << 8)
		| ((u32)buf[2] << 16) | ((u32)buf[3] << 24);
}

void *malloc_no_fail(usize size);
u8 *read_entire_file(const char *filename, i32 trailing_zeros, i32 *out_fsize);
void debug_print_buf_hex(char *debug_name, u8 *buf, i32 buflen);
void string_copy(char *dst, i32 dstlen, const char *src);
void string_ascii_tolower(char *dst, i32 dstlen, const char *src);
bool string_eq(const char *s1, const char *s2);

void panic(const char *fmt, ...);
#define PANIC(...) panic(__VA_ARGS__)

#endif //OTCVT_COMMON_HH_
