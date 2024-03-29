#include "common.hh"

void *malloc_no_fail(usize size){
	void *mem = malloc(size);
	if(!mem) abort();
	return mem;
}

u8 *read_entire_file(const char *filename, i32 trailing_zeros, i32 *out_fsize){
	ASSERT(filename != NULL);
	ASSERT(trailing_zeros >= 0);
	FILE *f = fopen(filename, "rb");
	if(!f)
		return NULL;

	fseek(f, 0, SEEK_END);
	i32 fsize = (i32)ftell(f);
	fseek(f, 0, SEEK_SET);

	u8 *fmem = (u8*)malloc_no_fail(fsize + trailing_zeros);
	if(trailing_zeros > 0)
		memset(fmem + fsize, 0, trailing_zeros);

	if(fread(fmem, 1, fsize, f) != fsize){
		printf("failed to read file \"%s\" (ferror = %d, feof = %d)\n",
			filename, ferror(f), feof(f));
		exit(-1);
	}
	fclose(f);

	if(out_fsize)
		*out_fsize = fsize;
	return fmem;
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

void string_copy(char *dst, i32 dstlen, const char *src){
	i32 i = 0;
	while(src[i] && i < (dstlen - 1)){
		dst[i] = src[i];
		i += 1;
	}
	dst[i] = 0;
}

void string_ascii_tolower(char *dst, i32 dstlen, const char *src){
	i32 i = 0;
	while(src[i] && i < (dstlen - 1)){
		// NOTE: Only touch it if its an uppercase ascii letter.
		if(src[i] >= 65 && src[i] <= 90){
			dst[i] = src[i] + 32;
		}else{
			dst[i] = src[i];
		}
		i += 1;
	}
	dst[i] = 0;
}

bool string_eq(const char *s1, const char *s2){
	i32 i = 0;
	while(s1[i] != 0 && s1[i] == s2[i])
		i += 1;
	return s1[i] == s2[i];
}

void panic(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	printf("======== PANIC ========\n");
	vprintf(fmt, ap);
	printf("\n=======================\n");
	va_end(ap);
	exit(-1);
}
