#include "common.hh"
#include <ctype.h>
#include <stdio.h>

#if OS_WINDOWS
#	define WIN32_LEAN_AND_MEAN 1
#	include <windows.h>
#else
#	include <time.h>
#	include <unistd.h>
#	include <sys/mman.h>
#endif

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
	usize commit_granularity;
	usize virtual_size;
	usize committed_size;
	u8 *memend;
	u8 *memptr;
};

static
usize align_up(usize size, usize alignment){
	ASSERT(IS_POWER_OF_TWO(alignment));
	usize alignment_mask = alignment - 1;
	return (size + alignment_mask) & ~alignment_mask;
}

static
void *ptr_align_up(void *ptr, usize alignment){
	return (void*)align_up((usize)ptr, alignment);
}

static
void arena_commit(MemArena *arena){
	LOG("arena->memend = %p", arena->memend);
	usize commit_granularity = arena->commit_granularity;
	usize virtual_size = arena->virtual_size;
	usize committed_size = arena->committed_size;
	usize new_committed_size = committed_size + commit_granularity;
	if(new_committed_size > virtual_size){
		usize overflow = (new_committed_size - virtual_size) + 1;
		PANIC("arena overflow: arena = %p, overflow = %zu", arena, overflow);
	}

	void *commit_base = arena->memend;
#if OS_WINDOWS
	void *ret = VirtualAlloc(commit_base, commit_granularity, MEM_COMMIT, PAGE_READWRITE);
	if(ret != commit_base)
		PANIC("VirtualAlloc failed (error = %d)", GetLastError());
#else
	int ret = mprotect(commit_base, commit_size, PROT_READ | PROT_WRITE);
	if(ret == -1)
		PANIC("mprotect failed (error = %d)", errno);
#endif
	arena->memend += arena->commit_granularity;
}

void *arena_alloc_raw(MemArena *arena, usize size, usize alignment){
	ASSERT(IS_POWER_OF_TWO(alignment));
	u8 *ptr = (u8*)ptr_align_up(arena->memptr, alignment);
	u8 *new_memptr = ptr + size;
	while(new_memptr >= arena->memend)
		arena_commit(arena);
	arena->memptr = new_memptr;
	return ptr;
}

static
usize os_page_size(void){
#if OS_WINDOWS
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (usize)sysinfo.dwPageSize;
#else
	return (usize)sysconf(_SC_PAGESIZE);
#endif
}

MemArena *arena_init(usize virtual_size, usize commit_granularity){
	ASSERT(commit_granularity > 0);
	usize page_size = os_page_size();
	virtual_size = align_up(virtual_size, page_size);
	commit_granularity = align_up(commit_granularity, page_size);

#if OS_WINDOWS
	void *mem = VirtualAlloc(NULL, virtual_size, MEM_RESERVE, PAGE_NOACCESS);
	if(mem == NULL)
		PANIC("VirtualAlloc failed (error = %d)", GetLastError());
#else
	void *mem = mmap(NULL, virtual_size, PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if(mem == MAP_FAILED)
		PANIC("mmap failed (error = %d)", errno);
#endif

	MemArena tmp;
	tmp.virtual_size = virtual_size;
	tmp.committed_size = 0;
	tmp.commit_granularity = commit_granularity;
	tmp.memptr = (u8*)mem;
	tmp.memend = (u8*)mem;

	MemArena *result = arena_alloc<MemArena>(&tmp, 1);
	ASSERT(result == mem);
	*result = tmp;
	return result;
}

// ----------------------------------------------------------------
// OS / stdlib wrappers
// ----------------------------------------------------------------

void *malloc_no_fail(usize size){
	void *mem = malloc(size);
	if(!mem)
		PANIC("out of memory");
	return mem;
}

i64 sys_clock_monotonic_msec(void){
#if OS_WINDOWS
	return (i64)GetTickCount64();
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (i64)ts.tv_sec * 1000 +
		(i64)ts.tv_nsec / 1000000;
#endif
}

void sys_sleep_msec(i64 ms){
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

// ----------------------------------------------------------------
// Debug Utility
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
// File Utility
// ----------------------------------------------------------------

u8 *read_entire_file(MemArena *arena, const char *filename, i32 *out_size){
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
