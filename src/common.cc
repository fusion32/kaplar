#include "common.hh"
#include <ctype.h>
#include <stdio.h>

// ----------------------------------------------------------------
// Logging
// ----------------------------------------------------------------

void log_info(const char *file, i32 line,
		const char *function, const char *fmt, ...){
	fprintf(stdout, "%s: ", function);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputc('\n', stdout);
}
void log_error(const char *file, i32 line,
		const char *function, const char *fmt, ...){
	fprintf(stdout, "[ERROR] %s:%d %s: ", file, line, function);
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputc('\n', stdout);
}

void panic(const char *file, i32 line,
		const char *function, const char *fmt, ...){
	fprintf(stdout, "==== PANIC ====\n");
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
	fputc('\n', stdout);
	abort();
}

// ----------------------------------------------------------------
// Memory Arena
// ----------------------------------------------------------------

struct MemArena{
	u8 *memend;
	u8 *memptr;
};

static constexpr void *ptr_align_up(void *ptr, usize alignment){
	usize alignment_mask = alignment - 1;
	usize addr = (usize)ptr;
	return (void*)((addr + alignment_mask) & ~alignment_mask);
}

static constexpr void *ptr_align_down(void *ptr, usize alignment){
	usize alignment_mask = alignment - 1;
	usize addr = (usize)ptr;
	return (void*)(addr & ~alignment_mask);
}

void *arena_alloc_raw(MemArena *arena, usize size, usize alignment){
	ASSERT(IS_POWER_OF_TWO(alignment));
	u8 *ptr = (u8*)ptr_align_up(arena->memptr, alignment);
	u8 *new_memptr = ptr + size;
	if(new_memptr >= arena->memend){
		usize overflow = (new_memptr - arena->memend) + 1;
		PANIC("arena overflow: arena = %p,"
			" overflow = %zu", arena, overflow);
	}
	arena->memptr = new_memptr;
	return ptr;
}

void *arena_alloc_raw_tmp(MemArena *arena, usize size, usize alignment){
	ASSERT(IS_POWER_OF_TWO(alignment));
	u8 *ptr = (u8*)ptr_align_up(arena->memptr, alignment);
	u8 *new_memptr = ptr + size;
	if(new_memptr >= arena->memend){
		usize overflow = (new_memptr - arena->memend) + 1;
		PANIC("arena overflow: arena = %p,"
			" overflow = %zu", arena, overflow);
	}
	return ptr;
}

void arena_reset(MemArena *arena){
	arena->memptr = (u8*)(arena + 1);
}

MemArena *arena_init(void *memory, usize size){
	ASSERT(size >= 4096);
	MemArena *arena = (MemArena*)ptr_align_up(memory, alignof(MemArena));
	arena->memend = (u8*)memory + size;
	arena->memptr = (u8*)(arena + 1);
	return arena;
}

MemArena *arena_tmp(MemArena *arena){
	return arena_init(arena->memptr,
		arena->memend - arena->memptr);
}

void arena_commit_tmp(MemArena *arena, MemArena *tmp){
	ASSERT(arena->memend == tmp->memend);
	arena->memptr = tmp->memptr;
}

usize arena_memory_left(MemArena *arena){
	return arena->memend - arena->memptr;
}

// ----------------------------------------------------------------
// Common Utility
// ----------------------------------------------------------------

void debug_printf(char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void debug_print_buf(char *debug_name, u8 *buf, i32 buflen){
	printf("buf (%s, len = %d):\n", debug_name, buflen);
	for(i32 i = 0; i < buflen; i += 1){
		i32 c = isprint(buf[i]) ? buf[i] : '.';
		if((i & 15) == 15)
			printf("%c\n", c);
		else
			printf("%c ", c);
	}

	if(buflen & 15)
		putchar('\n');
}

void debug_print_buf_hex(char *debug_name, u8 *buf, i32 buflen){
	printf("buf (%s, len = %d):\n", debug_name, buflen);
	for(i32 i = 0; i < buflen; i += 1){
		if((i & 15) == 15)
			printf("%02X\n", buf[i]);
		else
			printf("%02X ", buf[i]);
	}

	if(buflen & 15)
		putchar('\n');
}

// ----------------------------------------------------------------
// stdlib / system wrappers
// ----------------------------------------------------------------

#if OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN 1
#	include <windows.h>
#else
#	include <time.h>
#	include <unistd.h>
#endif

void *kpl_alloc(usize size){
	void *mem = malloc(size);
	if(!mem)
		PANIC("out of memory");
	return mem;
}

void kpl_free(void *mem, usize size){
	free(mem);
}

i64 kpl_clock_monotonic_msec(void){
#if OS_WINDOWS
	return (i64)GetTickCount64();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (i64)ts.tv_sec * 1000 +
		(i64)ts.tv_nsec / 1000000;
#endif
}

void kpl_sleep_msec(i64 ms){
#if OS_WINDOWS
	ASSERT(ms < MAXDWORD && "windows limitation");
	Sleep((DWORD)ms);
#else
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;
	nanosleep(&ts, NULL);
#endif
}

u8 *kpl_read_entire_file(MemArena *arena, const char *filename, i32 *out_size){
	FILE *fp = fopen(filename, "rb");
	if(!fp) return NULL;

	fseek(fp, 0, SEEK_END);
	i32 size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// NOTE: Always include a few nul bytes at the end of the
	// returned buffer because it can simplify parsing in most
	// cases. For text files you can treat the buffer as a nul
	// terminated string. For binary files you can parse ints
	// and floats without having to check for the end of file
	// while doing so. So there is no reason for this not to
	// happen.
	u8 *mem = arena_alloc<u8>(arena, size + 16);
	memset(mem + size, 0, 16);
	if(fread(mem, 1, size, fp) != size){
		PANIC("error while reading file \"%s\""
			" (ferror = %d, feof = %d)\n",
			filename, ferror(fp), feof(fp));
	}
	fclose(fp);

	if(out_size)
		*out_size = size;
	return mem;
}
