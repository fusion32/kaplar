#ifndef KAPLAR_COMMON_HH_
#define KAPLAR_COMMON_HH_ 1

// ----------------------------------------------------------------
// Preamble
// ----------------------------------------------------------------

// ensure we're compiling in 64bit
static_assert(sizeof(void*) == 8, "sizeof(void*) != 8");

// stdlib base
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// int types
typedef int8_t		i8;
typedef uint8_t		u8;
typedef int16_t		i16;
typedef uint16_t	u16;
typedef int32_t		i32;
typedef uint32_t	u32;
typedef int64_t		i64;
typedef uint64_t	u64;
typedef size_t		usize;

// arch settings
#ifdef ARCH_X64
#	define ARCH_UNALIGNED_ACCESS 1
#	define ARCH_BIG_ENDIAN 0
#else
#	error "add arch settings"
#endif

// os settings
#if !OS_WINDOWS && !OS_LINUX && !OS_POSIX
#	error "no os defined"
#endif

// debug settings
#ifdef _DEBUG
#	define BUILD_DEBUG 1
#endif
#define ASSERT(expr)									\
	do{ if(!(expr)){									\
		LOG_ERROR("assertion failed \"%s\"", #expr);	\
		abort();										\
	} } while(0)

// compiler settings
#if defined(_MSC_VER)
#	define INLINE __forceinline
#	define UNREACHABLE abort()
#	define FALLTHROUGH ((void)0)
#elif defined(__GNUC__)
#	define INLINE __attribute__((always_inline)) inline
#	define UNREACHABLE abort()
#	define FALLTHROUGH __attribute__((fallthrough))
#else
#	error "add compiler settings"
#endif

// common macros
#define NARRAY(arr) (sizeof(arr)/sizeof((arr)[0]))
#define IS_POWER_OF_TWO(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))

// ----------------------------------------------------------------
// Logging
// ----------------------------------------------------------------
void log_info(const char *file, i32 line,
		const char *function, const char *fmt, ...);
void log_error(const char *file, i32 line,
		const char *function, const char *fmt, ...);
void panic(const char *file, i32 line,
		const char *function, const char *fmt, ...);
#define LOG(...)		log_info(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...)	log_error(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define PANIC(...)		panic(__FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

// ----------------------------------------------------------------
// Memory Arena
// ----------------------------------------------------------------
struct MemArena;
void *arena_alloc_raw(MemArena *arena, usize size, usize alignment);
MemArena *arena_init(usize virtual_size, usize granularity);

template<typename T>
constexpr void type_alignment_check(void){
	static_assert((sizeof(T) & (alignof(T) - 1)) == 0, "type alignment check failed");
}

template<typename T>
static INLINE T *arena_alloc(MemArena *arena, usize num){
	type_alignment_check<T>();

	// NOTE: Enforce at least pointer alignment when allocating an array.
	usize alignment = alignof(T);
	if(num > 1 && alignment < alignof(void*))
		alignment = alignof(void*);
	return (T*)arena_alloc_raw(arena, sizeof(T) * num, alignment);
}

template<typename T>
static INLINE T *arena_alloc_init(MemArena *arena, usize num, T initial_value){
	T *result = arena_alloc<T>(arena, num);
	for(usize i = 0; i < num; i += 1)
		result[i] = initial_value;
	return result;
}

// ----------------------------------------------------------------
// sys / stdlib wrappers
// ----------------------------------------------------------------
void *malloc_no_fail(usize size);
i64 sys_clock_monotonic_msec(void);
void sys_sleep_msec(i64 ms);

// ----------------------------------------------------------------
// Debug Utility
// ----------------------------------------------------------------
void debug_printf(char *fmt, ...);
void debug_print_buf(char *debug_name, u8 *buf, i32 buflen);
void debug_print_buf_hex(char *debug_name, u8 *buf, i32 buflen);

// ----------------------------------------------------------------
// File Utility
// ----------------------------------------------------------------
u8 *read_entire_file(MemArena *arena, const char *filename, i32 *out_size);

// ----------------------------------------------------------------
// Config
// ----------------------------------------------------------------
struct Config{
	usize arena_vsize;
	usize arena_granularity;

	//const char *login_rsa_pem_file;
	u16 login_port;
	u16 login_max_connections;

	//const char *game_rsa_pem_file;
	const char *game_world_file;
	//const char *game_client_data_file;
	u16 game_port;
	u16 game_max_connections;
	i64 game_frame_interval;
};

#endif //KAPLAR_COMMON_HH_
