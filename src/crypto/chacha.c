/*
chacha-merged.c version 20080118
D. J. Bernstein
Public domain.
Modified for Password Tech by C.T.
*/
#include <stddef.h>
#include <string.h>
#include "chacha.h"

#if 1
#define ROTL32(v, n) _lrotl(v, n)
#define ROTR32(v, n) _lrotr(v, n)
#else
#define ROTL32(v, n) (U32V((v) << (n)) | ((v) >> (32 - (n))))
#define ROTR32(v, n) (U32V((v) >> (n)) | ((v) << (32 - (n))))
#endif

#define U32V(v) ((u32)(v) & 0xFFFFFFFF)

#define U32TO32_LITTLE(v) (v)
#define U8TO32_LITTLE(p) U32TO32_LITTLE(((u32*)(p))[0])
#define U32TO8_LITTLE(p, v) (((u32*)(p))[0] = U32TO32_LITTLE(v))

#define ROTATE(v,c) (ROTL32(v,c))
#define XOR(v,w) ((v) ^ (w))
#define PLUS(v,w) (U32V((v) + (w)))
#define PLUSONE(v) (PLUS((v),1))
#define QUARTERROUND(a,b,c,d) \
  a = PLUS(a,b); d = ROTATE(XOR(d,a),16); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c),12); \
  a = PLUS(a,b); d = ROTATE(XOR(d,a), 8); \
  c = PLUS(c,d); b = ROTATE(XOR(b,c), 7);
#define DEFAULT_NROUNDS 20
static const char sigma[16] = "expand 32-byte k";
static const char tau[16] = "expand 16-byte k";
void chacha_keysetup(chacha_ctx *x,const u8 *k,u32 kbits)
{
  const char *constants;
  x->input[4] = U8TO32_LITTLE(k + 0);
  x->input[5] = U8TO32_LITTLE(k + 4);
  x->input[6] = U8TO32_LITTLE(k + 8);
  x->input[7] = U8TO32_LITTLE(k + 12);
  if (kbits == 256) { /* recommended */
    k += 16;
    constants = sigma;
  } else { /* kbits == 128 */
    constants = tau;
  }
  x->input[8] = U8TO32_LITTLE(k + 0);
  x->input[9] = U8TO32_LITTLE(k + 4);
  x->input[10] = U8TO32_LITTLE(k + 8);
  x->input[11] = U8TO32_LITTLE(k + 12);
  x->input[0] = U8TO32_LITTLE(constants + 0);
  x->input[1] = U8TO32_LITTLE(constants + 4);
  x->input[2] = U8TO32_LITTLE(constants + 8);
  x->input[3] = U8TO32_LITTLE(constants + 12);
  /* Set counter and IV to zero to ensure reproducibility
     if _ivsetup() is not called */
  x->input[12] = 0;
  x->input[13] = 0;
  x->input[14] = 0;
  x->input[15] = 0;

  x->nrounds = DEFAULT_NROUNDS;
}
void chacha_nrounds(chacha_ctx *x, int nrounds)
{
  x->nrounds = nrounds < 6 ? 6 : nrounds;
}
void chacha_ivsetup(chacha_ctx *x,const u8 *iv, const u8* ctr)
{
  x->input[12] = ctr == NULL ? 0 : U8TO32_LITTLE(ctr + 0);
  x->input[13] = ctr == NULL ? 0 : U8TO32_LITTLE(ctr + 4);
  x->input[14] = U8TO32_LITTLE(iv + 0);
  x->input[15] = U8TO32_LITTLE(iv + 4);
}
void chacha_encrypt_bytes(chacha_ctx *x,const u8 *m,u8 *c,u32 bytes)
{
  u32 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
  u32 j0, j1, j2, j3, j4, j5, j6, j7, j8, j9, j10, j11, j12, j13, j14, j15;
  u8 *ctarget;
  u8 tmp[64];
  int i;
  if (!bytes) return;
  j0 = x->input[0];
  j1 = x->input[1];
  j2 = x->input[2];
  j3 = x->input[3];
  j4 = x->input[4];
  j5 = x->input[5];
  j6 = x->input[6];
  j7 = x->input[7];
  j8 = x->input[8];
  j9 = x->input[9];
  j10 = x->input[10];
  j11 = x->input[11];
  j12 = x->input[12];
  j13 = x->input[13];
  j14 = x->input[14];
  j15 = x->input[15];
  for (;;) {
    if (bytes < 64) {
      for (i = 0;i < bytes;++i) tmp[i] = m[i];
      m = tmp;
      ctarget = c;
      c = tmp;
    }
    x0 = j0;
    x1 = j1;
    x2 = j2;
    x3 = j3;
    x4 = j4;
    x5 = j5;
    x6 = j6;
    x7 = j7;
    x8 = j8;
    x9 = j9;
    x10 = j10;
    x11 = j11;
    x12 = j12;
    x13 = j13;
    x14 = j14;
    x15 = j15;
    /* C.T.: Reference implementation by djb uses 8 rounds */
    for (i = x->nrounds;i > 0;i -= 2) {
      QUARTERROUND( x0, x4, x8,x12)
      QUARTERROUND( x1, x5, x9,x13)
      QUARTERROUND( x2, x6,x10,x14)
      QUARTERROUND( x3, x7,x11,x15)
      QUARTERROUND( x0, x5,x10,x15)
      QUARTERROUND( x1, x6,x11,x12)
      QUARTERROUND( x2, x7, x8,x13)
      QUARTERROUND( x3, x4, x9,x14)
    }
    x0 = PLUS(x0,j0);
    x1 = PLUS(x1,j1);
    x2 = PLUS(x2,j2);
    x3 = PLUS(x3,j3);
    x4 = PLUS(x4,j4);
    x5 = PLUS(x5,j5);
    x6 = PLUS(x6,j6);
    x7 = PLUS(x7,j7);
    x8 = PLUS(x8,j8);
    x9 = PLUS(x9,j9);
    x10 = PLUS(x10,j10);
    x11 = PLUS(x11,j11);
    x12 = PLUS(x12,j12);
    x13 = PLUS(x13,j13);
    x14 = PLUS(x14,j14);
    x15 = PLUS(x15,j15);
    x0 = XOR(x0,U8TO32_LITTLE(m + 0));
    x1 = XOR(x1,U8TO32_LITTLE(m + 4));
    x2 = XOR(x2,U8TO32_LITTLE(m + 8));
    x3 = XOR(x3,U8TO32_LITTLE(m + 12));
    x4 = XOR(x4,U8TO32_LITTLE(m + 16));
    x5 = XOR(x5,U8TO32_LITTLE(m + 20));
    x6 = XOR(x6,U8TO32_LITTLE(m + 24));
    x7 = XOR(x7,U8TO32_LITTLE(m + 28));
    x8 = XOR(x8,U8TO32_LITTLE(m + 32));
    x9 = XOR(x9,U8TO32_LITTLE(m + 36));
    x10 = XOR(x10,U8TO32_LITTLE(m + 40));
    x11 = XOR(x11,U8TO32_LITTLE(m + 44));
    x12 = XOR(x12,U8TO32_LITTLE(m + 48));
    x13 = XOR(x13,U8TO32_LITTLE(m + 52));
    x14 = XOR(x14,U8TO32_LITTLE(m + 56));
    x15 = XOR(x15,U8TO32_LITTLE(m + 60));
    j12 = PLUSONE(j12);
    if (!j12) {
      j13 = PLUSONE(j13);
      /* stopping at 2^70 bytes per nonce is user's responsibility */
    }
    U32TO8_LITTLE(c + 0,x0);
    U32TO8_LITTLE(c + 4,x1);
    U32TO8_LITTLE(c + 8,x2);
    U32TO8_LITTLE(c + 12,x3);
    U32TO8_LITTLE(c + 16,x4);
    U32TO8_LITTLE(c + 20,x5);
    U32TO8_LITTLE(c + 24,x6);
    U32TO8_LITTLE(c + 28,x7);
    U32TO8_LITTLE(c + 32,x8);
    U32TO8_LITTLE(c + 36,x9);
    U32TO8_LITTLE(c + 40,x10);
    U32TO8_LITTLE(c + 44,x11);
    U32TO8_LITTLE(c + 48,x12);
    U32TO8_LITTLE(c + 52,x13);
    U32TO8_LITTLE(c + 56,x14);
    U32TO8_LITTLE(c + 60,x15);
    if (bytes <= 64) {
      if (bytes < 64) {
        for (i = 0;i < bytes;++i) ctarget[i] = c[i];
      }
      x->input[12] = j12;
      x->input[13] = j13;
      return;
    }
    bytes -= 64;
    c += 64;
    m += 64;
  }
}
void chacha_decrypt_bytes(chacha_ctx *x,const u8 *c,u8 *m,u32 bytes)
{
  chacha_encrypt_bytes(x,c,m,bytes);
}
void chacha_keystream_bytes(chacha_ctx *x,u8 *stream,u32 bytes)
{
  memset(stream, 0, bytes);
  chacha_encrypt_bytes(x,stream,stream,bytes);
}
/* Test vectors taken from RFC 7539 */
static const u8 test_key[3][32] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {0,0xff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
/* nonce comprises block counter and IV */
static const u8 test_nonce[3][16] = {
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
static const u8 test_keystream[3][64] = {
  {0x76,0xb8,0xe0,0xad,0xa0,0xf1,0x3d,0x90,0x40,0x5d,
  0x6a,0xe5,0x53,0x86,0xbd,0x28,0xbd,0xd2,0x19,0xb8,
  0xa0,0x8d,0xed,0x1a,0xa8,0x36,0xef,0xcc,0x8b,0x77,
  0x0d,0xc7,0xda,0x41,0x59,0x7c,0x51,0x57,0x48,0x8d,
  0x77,0x24,0xe0,0x3f,0xb8,0xd8,0x4a,0x37,0x6a,0x43,
  0xb8,0xf4,0x15,0x18,0xa1,0x1c,0xc3,0x87,0xb6,0x69,
  0xb2,0xee,0x65,0x86},
  {0x3a,0xeb,0x52,0x24,0xec,0xf8,0x49,0x92,0x9b,0x9d,
  0x82,0x8d,0xb1,0xce,0xd4,0xdd,0x83,0x20,0x25,0xe8,
  0x01,0x8b,0x81,0x60,0xb8,0x22,0x84,0xf3,0xc9,0x49,
  0xaa,0x5a,0x8e,0xca,0x00,0xbb,0xb4,0xa7,0x3b,0xda,
  0xd1,0x92,0xb5,0xc4,0x2f,0x73,0xf2,0xfd,0x4e,0x27,
  0x36,0x44,0xc8,0xb3,0x61,0x25,0xa6,0x4a,0xdd,0xeb,
  0x00,0x6c,0x13,0xa0},
  {0x72,0xd5,0x4d,0xfb,0xf1,0x2e,0xc4,0x4b,0x36,0x26,
  0x92,0xdf,0x94,0x13,0x7f,0x32,0x8f,0xea,0x8d,0xa7,
  0x39,0x90,0x26,0x5e,0xc1,0xbb,0xbe,0xa1,0xae,0x9a,
  0xf0,0xca,0x13,0xb2,0x5a,0xa2,0x6c,0xb4,0xa6,0x48,
  0xcb,0x9b,0x9d,0x1b,0xe6,0x5b,0x2c,0x09,0x24,0xa6,
  0x6c,0x54,0xd5,0x45,0xec,0x1b,0x73,0x74,0xf4,0x87,
  0x2e,0x99,0xf0,0x96}
};

int chacha_self_test(void)
{
  chacha_ctx ctx;
  u8 keystream[64];
  int i;
  for (i = 0; i < 3; i++) {
    chacha_keysetup(&ctx, test_key[i], 256);
    chacha_ivsetup(&ctx, test_nonce[i] + 8, test_nonce[i]);
    chacha_keystream_bytes(&ctx, keystream, 64);
    if (memcmp(keystream, test_keystream[i], 64) != 0)
      return 1;
  }
  return 0;
}
