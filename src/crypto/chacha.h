/* chacha.h */

/* 
 * Header file for synchronous stream ciphers without authentication
 * mechanism.
 * 
 * *** Please only edit parts marked with "[edit]". ***
 */

#ifndef CHACHA_H
#define CHACHA_H

#ifdef __cplusplus
extern "C" {
#endif

/* Data structures */

typedef unsigned __int8 u8;
typedef unsigned __int32 u32;

typedef struct
{
  u32 input[16]; /* could be compressed */
  int nrounds;
} chacha_ctx;

/* ------------------------------------------------------------------------- */

/*
 * Key setup. It is the user's responsibility to select the values of
 * keysize and ivsize from the set of supported values specified
 * above.
 */

void chacha_keysetup(
  chacha_ctx* ctx,
  const u8* key,
  u32 keysize);

/*
 * Set number of rounds (default is 20)
 */
void chacha_nrounds(
  chacha_ctx* ctx,
  int nrounds);

/*
 * Set 64-bit IV (nonce) and 64-bit counter (optional).
 */
void chacha_ivsetup(
  chacha_ctx* ctx,
  const u8* iv,
  const u8* ctr);

/*
 * Encryption/decryption of arbitrary length messages.
 */

void chacha_encrypt_bytes(
  chacha_ctx* ctx,
  const u8* plaintext,
  u8* ciphertext,
  u32 msglen);                /* Message length in bytes. */

void chacha_decrypt_bytes(
  chacha_ctx* ctx,
  const u8* ciphertext,
  u8* plaintext,
  u32 msglen);                /* Message length in bytes. */ 

/* ------------------------------------------------------------------------- */

/* Optional features */

/* 
 * For testing purposes it can sometimes be useful to have a function
 * which immediately generates keystream without having to provide it
 * with a zero plaintext. If your cipher cannot provide this function
 * (e.g., because it is not strictly a synchronous cipher), please
 * reset the ECRYPT_GENERATES_KEYSTREAM flag.
 */

void chacha_keystream_bytes(
  chacha_ctx* ctx,
  u8* keystream,
  u32 length);                /* Length of keystream in bytes. */

int chacha_self_test(void);
/* ------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif
