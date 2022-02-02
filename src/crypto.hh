#ifndef KAPLAR_CRYPTO_HH_
#define KAPLAR_CRYPTO_HH_ 1

#include "common.hh"

// ----------------------------------------------------------------
// RSA
// ----------------------------------------------------------------
struct RSA;

RSA *rsa_alloc(void);
void rsa_free(RSA *r);
bool rsa_setkey(RSA *r, const char *p, const char *q, const char *e);
bool rsa_encode(RSA *r, u8 *data, usize *len, usize maxlen);
bool rsa_decode(RSA *r, u8 *data, usize *len, usize maxlen);


// ----------------------------------------------------------------
// XTEA
// ----------------------------------------------------------------
void xtea_encode(u32 *k, u8 *data, usize len);
void xtea_decode(u32 *k, u8 *data, usize len);

#endif //KAPLAR_CRYPTO_HH_
