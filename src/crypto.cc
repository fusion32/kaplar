#include "crypto.hh"
#include "buffer_util.hh"

// ----------------------------------------------------------------
// CHECKSUM - ADLER32
// ----------------------------------------------------------------
#define ADLER32_BASE 65521U
#define ADLER32_NMAX 5552

#define ADLER32_DO1(buf, i)	{a += buf[i]; b += a;}
#define ADLER32_DO2(buf, i)	ADLER32_DO1(buf,i); ADLER32_DO1(buf,i+1);
#define ADLER32_DO4(buf, i)	ADLER32_DO2(buf,i); ADLER32_DO2(buf,i+2);
#define ADLER32_DO8(buf, i)	ADLER32_DO4(buf,i); ADLER32_DO4(buf,i+4);
#define ADLER32_DO16(buf)	ADLER32_DO8(buf,0); ADLER32_DO8(buf,8);

u32 adler32_accumulate(u32 acc, u8 *data, usize len){
	u32 a = acc & 0xFFFF;
	u32 b = (acc >> 16) & 0xFFFF;
	int k;
	while(len > 0){
		k = (int)(len > ADLER32_NMAX ? ADLER32_NMAX : len);
		len -= k;
		while(k >= 16){
			ADLER32_DO16(data);
			data += 16;
			k -= 16;
		}
		while(k-- != 0){
			a += *data++;
			b += a;
		}
		a %= ADLER32_BASE;
		b %= ADLER32_BASE;
	}
	return a | (b << 16);
}

// ----------------------------------------------------------------
// RSA
// ----------------------------------------------------------------

#if USE_FULL_GMP

#include <gmp.h>

#else //USE_FULL_GMP

// NOTE: GMP is annoying to build on windows. Fortunately there is now
// a "mini-gmp" version that we can include in the source tree and is
// contained in a single source/header pair. There is a disclaimer saying
// that the performance is not on par with the full version of GMP but
// this is only for testing since the server will ultimately run on Linux
// where it is trivial to build/install GMP.

#include "mini-gmp/mini-gmp.h"
#include <stdarg.h>

static
void mpz_inits(mpz_ptr x, ...){
	va_list ap;
	va_start(ap, x);
	do{
		mpz_init(x);
		x = va_arg(ap, mpz_ptr);
	}while(x != NULL);
	va_end(ap);
}

static
void mpz_clears(mpz_ptr x, ...){
	va_list ap;
	va_start(ap, x);
	do{
		mpz_clear(x);
		x = va_arg(ap, mpz_ptr);
	}while(x != NULL);
	va_end(ap);
}

#endif //USE_FULL_GMP

struct RSA{
	mpz_t p, q, n, e;		// key vars
	mpz_t dp, dq, qi;		// decoding vars
	mpz_t x0, x1, x2, x3;	// aux vars
};

RSA *rsa_alloc(void){
	RSA *r = (RSA*)kpl_alloc(sizeof(RSA));
	mpz_inits(r->p, r->q, r->n, r->e,
		r->dp, r->dq, r->qi,
		r->x0, r->x1, r->x2, r->x3, NULL);
	return r;
}

void rsa_free(RSA *r){
	mpz_clears(r->p, r->q, r->n, r->e,
		r->dp, r->dq, r->qi,
		r->x0, r->x1, r->x2, r->x3, NULL);
	free(r);
}

bool rsa_setkey(RSA *r, const char *p, const char *q, const char *e){
	mpz_set_str(r->p, p, 10);
	mpz_set_str(r->q, q, 10);
	mpz_set_str(r->e, e, 10);
	mpz_mul(r->n, r->p, r->q);

	// calculate the Carmichael's totient (lambda)
	mpz_sub_ui(r->x0, r->p, 1);		// x0 = p - 1
	mpz_sub_ui(r->x1, r->q, 1);		// x1 = q - 1
	mpz_lcm(r->x2, r->x0, r->x1);	// x2 = lcm(x0, x1) <-- lambda

	// `e` must be smaller than lambda
	if(mpz_cmp(r->x2, r->e) <= 0)
		return false;

	// lambda and `e` must be coprimes
	mpz_gcd(r->x3, r->x2, r->e);	// x3 = gcd(x2, e)
	if(mpz_cmp_ui(r->x3, 1) != 0)
		return false;

	// calculate `d`
	mpz_invert(r->x3, r->e, r->x2);	// x3 = invert(e, x2) <-- d

	// this next step is an optimization based on the
	// Chinese remainder theorem used for decoding
	mpz_mod(r->dp, r->x3, r->x0);	// dp = x3 mod (p - 1)
	mpz_mod(r->dq, r->x3, r->x1);	// dq = x3 mod (q - 1)
	mpz_invert(r->qi, r->q, r->p);	// qi = invert(q, p)

	return true;
}

bool rsa_encode(RSA *r, u8 *data, usize *len, usize maxlen){
	mpz_import(r->x0, *len, 1, 1, 0, 0, data);		// x0 = import(data)
	mpz_powm(r->x1, r->x0, r->e, r->n);				// x1 = (x0 ^ e) mod n

	usize outlen = (mpz_sizeinbase(r->x1, 2) + 7) / 8;
	if(outlen > maxlen){
		*len = outlen;
		return false;
	}
	mpz_export(data, len, 1, 1, 0, 0, r->x1);		// data = export(x1);
	return true;
}

bool rsa_decode(RSA *r, u8 *data, usize *len, usize maxlen){
	mpz_import(r->x0, *len, 1, 1, 0, 0, data);		// x0 = import(data)
	mpz_powm(r->x1, r->x0, r->dp, r->p);			// x1 = (x0 ^ dp) mod p
	mpz_powm(r->x2, r->x0, r->dq, r->q);			// x2 = (x0 ^ dq) mod q
	mpz_sub(r->x3, r->x1, r->x2);					// x3 = x1 - x2
	if(mpz_cmp(r->x1, r->x2) < 0)					// if x1 < x2
		mpz_add(r->x3, r->x3, r->p);				//	x3 = x3 + p
	mpz_mul(r->x3, r->x3, r->qi);					//
	mpz_mod(r->x3, r->x3, r->p);					// x3 = (x3 * qi) mod p
	mpz_addmul(r->x2, r->x3, r->q);					// x2 = x2 + x3 * q

	// NOTE: The maximum message length that can be encoded with
	// this key is roughly:
	//		encoding_limit = mpz_sizeinbase(r->n, 2) / 8
	//	Note that we truncate the number of bits to the byte size
	// boundary because we are interested in messages composed of
	// whole bytes. For example, a 1024 bits key will be able to
	// encode 127 bytes plus some extra bits.

	usize outlen = (mpz_sizeinbase(r->x2, 2) + 7) / 8;
	if(outlen > maxlen){
		*len = outlen;
		return false;
	}
	mpz_export(data, len, 1, 1, 0, 0, r->x2);		// data = export(x2)
	return true;
}


// ----------------------------------------------------------------
// XTEA
// ----------------------------------------------------------------

#define IS_MULT_OF_8(x) (((x) & 7) == 0)

void xtea_encode(u32 *k, u8 *data, usize len){
	ASSERT(IS_MULT_OF_8(len));
	u32 v0, v1, delta, sum, i;
	while(len > 0){
		v0 = buffer_read_u32_le(data);
		v1 = buffer_read_u32_le(data + 4);
		delta = 0x9E3779B9UL; sum = 0UL;
		for(i = 0; i < 32; ++i){
			v0 += ((v1<<4 ^ v1>>5) + v1) ^ (sum + k[sum & 3]);
			sum += delta;
			v1 += ((v0<<4 ^ v0>>5) + v0) ^ (sum + k[sum>>11 & 3]);
		}
		buffer_write_u32_le(data, v0);
		buffer_write_u32_le(data + 4, v1);
		len -= 8; data += 8;
	}
}

void xtea_decode(u32 *k, u8 *data, usize len){
	ASSERT(IS_MULT_OF_8(len));
	u32 v0, v1, delta, sum, i;
	while(len > 0){
		v0 = buffer_read_u32_le(data);
		v1 = buffer_read_u32_le(data + 4);
		delta = 0x9E3779B9UL; sum = 0xC6EF3720UL;
		for(i = 0; i < 32; ++i){
			v1 -= ((v0<<4 ^ v0>>5) + v0) ^ (sum + k[sum>>11 & 3]);
			sum -= delta;
			v0 -= ((v1<<4 ^ v1>>5) + v1) ^ (sum + k[sum & 3]);
		}
		buffer_write_u32_le(data, v0);
		buffer_write_u32_le(data + 4, v1);
		len -= 8; data += 8;
	}
}
