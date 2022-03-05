// RandomPool.h
//
// PASSWORD TECH
// Copyright (c) 2002-2022 by Christian Thoeing <c.thoeing@web.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//---------------------------------------------------------------------------
#ifndef RandomPoolH
#define RandomPoolH
//---------------------------------------------------------------------------
#include <wincrypt.h>
#include <memory>
#include "UnicodeUtil.h"
#include "SecureMem.h"
#include "RandomGenerator.h"
#include "sha256.h"
//#include "aes.h"

// This is an implementation of a random pool and a cryptographically secure
// pseudorandom number generator (CSPRNG). Design is inspired by:
// - RandPool class in Crypto++ package by W. Dai
// - Yarrow PRNG by J. Kelsey, B. Schneier and N. Ferguson;
//   http://www.schneier.com/paper-yarrow.pdf
// - paper "Software Generation of Practically Strong Random Numbers"
//   by P. Gutmann; http://www.cs.auckland.ac.nz/~pgut001/pubs/usenix98.pdf
// - ANSI X9.17 PRNG
//
// HMAC-SHA-256 is used to "distil" randomness from entropy provided by the user
// and by system parameters.
// AES in counter (CTR) mode with a key derived from the pool is used to
// generate pseudorandom data.
//
// The pool consists of a 32-byte (= SHA-256 digest length) buffer filled
// with (pseudo)random data. The entropy of this data approaches 256 bits as
// more and more entropy is incorporated.
// Entropy (e.g., from keystrokes, mouse movements/clicks, etc.) is collected in
// a buffer S. When the buffer is full, the pool P is updated as follows:
// (1)  P <- HMAC(P, S || T),     (|| denotes concatenation)
// where T is a time-stamp from a high-resolution timer. It is added in every
// pool update no matter how full the entropy buffer is.
//
//
// When random data is requested, a 256-bit key K is derived from the pool:
// (2a) PoolUpdate (eq. (1))
// (2b) K <- HMAC(T*, P)
// T* is an extended time-stamp with some additional pseudorandom data.
// This procedure ensures that P and K cannot be derived from each other.
//
// Next, a 128-bit random counter C is chosen as follows:
// (3)  C <- E_K(T1 || T2)
// where E_K is the encryption function with K as key. T1 and T2 are two
// different timestamps.
//
// To generate random data, the counter is encrypted and incremented by 1
// afterwards:
// (4a) R <- E_K(C)
// (4b) C <- C + 1,
// where R is the next pseudorandom 16-byte output block.
//
// After generating a certain amount of random data, a key change ("generator
// gate") is performed:
// (5a) K_new,a <- E_K(C)
// (5b) C <- C + 1
// (5c) K_new,b <- E_K(C)
// (5d) K <- K_new,a || K_new,b
// (5e) C <- E_K(C ^ E_K(T))
// This key change protects the previously generated random blocks in case
// of a state compromise. Additionally, the counter is changed by xor'ing it
// with an encrypted time-stamp and encrypting the result (the procedure
// described in eq. (5e) is similar to the ANSI X9.17 generator).
//
//
// A special buffer ("get buffer") is filled with pre-computed random data.
// This buffer is used when the caller requests less than [get buffer size]
// bytes, and a generator gate is performed when refilling this buffer.
// When the caller requests more bytes, however, the caller's buffer is
// filled directly with random data, and a generator gate is performed
// afterwards. In theory, up to ~2^32 bytes could be generated without
// a key change. Although this can be considered a safe limit for a 128-bit
// block cipher, it is not recommended, and PWGen will never reach this limit
// in a single GetData() call.
//
//
// The TRandomPool class allocates memory in the virtual address space of
// the system. This "pool page" consists of the following data:
//
// [unused][pool][hash context][add buffer][AES key][counter][AES context][get buffer][temp. buffer]
//
// "unused" is just used to fill the entire memory page.
// total: 4096 bytes (corresponds to default memory page size in Windows).
//
// NOTE: As a further protection, the pool page is made indistinguishable
// from random, which is accomplished by clearing all buffers in the page
// with random data from a fast PRNG (-> FastPRNG.cpp).

// wrapper for CSPRNGs based on block ciphers or similar designs
// (such as stream ciphers using block counters, e.g. ChaCha),
// which are implemented in C and use typical context structures
namespace RandPoolCipher {
class CtrBasedCipher
{
public:

  virtual ~CtrBasedCipher()
  {}

  // set 256-bit key
  virtual void SetKey(const word8 pKey[32]) = 0;

  // generate and set new random key from output
  // -> buffer to store key
  // -> counter (if necessary) for generating key
  virtual void SelfKey(word8 pKey[32], word8* pCounter) = 0;

  // process buffer to generate new counter or
  // set new initialization vector (IV)
  // buffer must hold at least 16 bytes (128-bit)
  virtual void ProcessCounterOrIV(word8 pCounter[16]) = 0;

  // fill blocks with random data, incrementing counter if necessary
  // (counter may be NULL)
  // -> buffer to fill
  // -> 128-bit counter
  // -> number of blocks (not bytes) to fill
  virtual void FillBlocks(word8* pBuf, word8* pCounter,
    word32 lNumOfBlocks) = 0;

  // returns block size in bytes
  virtual word32 GetBlockSize(void) const = 0;

  // returns max. number of blocks before changing key
  virtual word32 GetMaxNumOfBlocks(void) const = 0;

  // change memory address of internal context
  virtual void Move(void* pNew) = 0;
};
}

class RandomPool : public RandomGenerator
{
public:
  enum class Cipher {
    AES_CTR,
    ChaCha20,
    ChaCha8,
    //Speck128
  };

private:
  bool m_blLockPhysMem;
  word8* m_pPoolPage;
  word8* m_pPool;
  word8* m_pAddBuf;
  word8* m_pCipherKey;
  word8* m_pSecCtr;
  word8* m_pGetBuf;
  word8* m_pTempBuf;
  sha256_context* m_pHashCtx;
  word8* m_pCipherCtx; // variable, supports different ciphers
  Cipher m_cipherType;
  std::unique_ptr<RandPoolCipher::CtrBasedCipher> m_pCipher;
  word32 m_lUnusedSize;
  word32 m_lAddBufPos;
  word32 m_lGetBufPos;
  word32 m_lNumOfBlocks;
  bool m_blKeySet;
  bool m_blCryptProv;
  HCRYPTPROV m_cryptProv;
  RandomGenerator& m_fastRandGen;

  // allocate pool page in virtual address space
  // <- pointer to the page, NULL if allocation failed
  word8* AllocPoolPage(void);

  // destroy & free the pool page
  void FreePoolPage(void);

  // set pointers m_pPool, m_pAddBuf, ... to the corresponding
  // positions in the pool page
  void SetPoolPointers(void);

  // rehash pool with the entropy buffer
  void UpdatePool(void);

  // set the AES key using the pool contents
  void SetKey(void);

  // create a new secure 128-bit IV/counter by encrypting two 64-bit timer values
  // -> where to store the counter (16 bytes)
  void GetNewCounterOrIV(word8* pCounter);

  // change key & counter
  void GeneratorGate(void);

  // fill get buffer with encrypted (i.e., pseudorandom) data
  // -> buffer to fill; if NULL, fill get buffer
  // -> number of bytes
  word32 FillBuf(word8* pBuf = NULL,
    word32 lNumOfBytes = 0);

  // fill buffer with random data
  void ClearPoolBuf(void* pBuf, word32 lSize)
  {
    m_fastRandGen.GetData(pBuf, lSize);
  }

public:

  enum {
    POOL_SIZE   = 32,          // pool size (=SHA-256 digest length)
    MAX_ENTROPY = POOL_SIZE*8, // max. entropy the RNG can provide
  };

  // default constructor
  // -> encryption algorithm for generating random data
  // -> fast (not necessarily cryptographically secure) PRNG for wiping memory etc.
  RandomPool(Cipher cipher, RandomGenerator& fastRandGen, bool blLockPhysMem);

  // create new random pool from another instance
  RandomPool(RandomPool& src, RandomGenerator& fastRandGen, bool blLockPhysMem);

  // destructor
  ~RandomPool();

  // singleton access
  static RandomPool* GetInstance(void);

  // change encryption algorithm for generating random numbers
  // -> new cipher type
  // -> force change (e.g. for constructor)
  void ChangeCipher(Cipher cipher,
    bool blForce = false);

  Cipher GetCipher(void) const
  {
    return m_cipherType;
  }

  // add data of any kind to the pool
  // -> data buffer
  // -> number of bytes
  void AddData(const void* pBuf,
    word32 lNumOfBytes);

  // for compatibility with RandomGenerator
  void Seed(const void* pBuf,
    word32 lNumOfBytes)
  {
    AddData(pBuf, lNumOfBytes);
  }

  // prepare for generating random numbers
  // (_should_ be called before retrieving data, but this is not mandatory)
  void RandReady(void)
  {
    SetKey();
  }

  // fill the buffer with random data
  // -> target buffer
  // -> number of bytes to copy
  void GetData(void* pBuf,
    word32 lNumOfBytes);

  // return a random byte
  word8 GetByte(void);

  // "flush" the contents of the add buffer (if filled with sensitive data)
  //  and destroy the PRNG state (AES context, counter, get buffer)
  // (_should_ be called after generating random data)
  void Flush(void)
  {
    UpdatePool();
  }

  // "randomize" the pool by adding system entropy
  void Randomize(void);

  // touch the pool (to prevent it from being swapped out)
  void TouchPool(void);

  // move the pool to another memory location
  // <- 'true': success, 'false': memory error
  bool MovePool(void);

  // create or overwrite seed file with entropy derived from the pool
  // (POOL_SIZE bytes)
  // -> name of the seed file
  // <- 'true': success, 'false': I/O error
  bool WriteSeedFile(const WString& sFileName);

  // read contents of the seed file and incorporate it into the pool
  // -> name of the seed file
  // <- 'true': success, 'false': I/O error
  bool ReadSeedFile(const WString& sFileName);
};

//extern RandomPool* g_pRandPool;

#endif
