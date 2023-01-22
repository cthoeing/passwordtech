// FastPRNG.cpp
//
// PASSWORD TECH
// Copyright (c) 2002-2023 by Christian Thoeing <c.thoeing@web.de>
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
#include <vcl.h>
#pragma hdrstop

#include "FastPRNG.h"
#include "SecureMem.h"
#include "sha256.h"
#include "chacha.h"
#include "hrtimer.h"
#include "MemUtil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


// Implementation of the IA (Indirection, Addition) random number generator
// by R.J. Jenkins.
// It can be considered as a "predecessor" of the cryptographically secure
// generator ISAAC:
// http://burtleburtle.net/bob/rand/isaac.html
// IA is inspired by RC4, but seems to be faster (since operating on 32-bit
// words instead of 8-bit bytes as in RC4) and more secure, although
// its output is, like RC4, to a certain degree biased, according to Jenkins.
// The bias can be detected in the correlated gap test.
// Nevertheless, IA seems to be an excellent choice for the purpose of a
// fast PRNG here, because it is very simple and additionally offers a
// certain degree of security against state recovery attacks (unlike other
// popular PRNGs).

// Note that we do NOT expect this generator to have cryptographic security,
// and PWGen does NOT rely on its security in critical situations (i.e., when
// generating random numbers for passwords).
// But a certain degree of security is certainly nice to protect the random
// pool from being located in RAM.

//static word32 m[256];
//static word32 x; // x and y are temporarily assigned to values in m
//static word32 y;
//static word32 r; // output 32-bit random number
//static word8 c;  // 8-bit counter (0..255) for indexing m

//#define NEXT_RAND x=m[c]; m[c]=y=m[x&0xFF]+r; r=m[(y>>8)&0xFF]+x; c++

FastPRNG g_fastRandGen;

//---------------------------------------------------------------------------
FastPRNG::FastPRNG(const FastPRNG& src)
{
  memcpy(m_m, src.m_m, sizeof(m_m));
  m_r = src.m_r;
  Randomize();
}
//---------------------------------------------------------------------------
void FastPRNG::Randomize(void)
{
  // we have to ensure that the m array is filled with good pseudorandom data;
  // otherwise, we can't really expect good results...
  // first, hash some timers to generate a nice 128-bit key & IV
  sha256_context hashCtx;

  sha256_init(&hashCtx);
  sha256_starts(&hashCtx, 0);

  word64 qTimer;
  HighResTimer(&qTimer);
  sha256_update(&hashCtx, reinterpret_cast<word8*>(&qTimer), sizeof(word64));

  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  sha256_update(&hashCtx, reinterpret_cast<word8*>(&ft), sizeof(FILETIME));

  word32 lCounter = GetTickCount();
  sha256_update(&hashCtx, reinterpret_cast<word8*>(&lCounter), sizeof(word32));

  LARGE_INTEGER qpc;
  if (QueryPerformanceCounter(&qpc))
    sha256_update(&hashCtx, reinterpret_cast<word8*>(&qpc), sizeof(LARGE_INTEGER));

  SecureMem<word8> hash(32);
  sha256_finish(&hashCtx, hash);

  // initialize ChaCha using the 256-bit hash as key
  // no need to set the IV, we won't have to decrypt anything...
  chacha_ctx cryptCtx;
  chacha_keysetup(&cryptCtx, hash, 256);

  // encrypt the m buffer
  word8* m8 = reinterpret_cast<word8*>(m_m);
  chacha_encrypt_bytes(&cryptCtx, m8, m8, sizeof(m_m));

  // initialize the counter c
  m_c = 0;

  // clean up...
  memzero(&hashCtx,  sizeof(sha256_context));
  memzero(&qTimer,   sizeof(word64));
  memzero(&ft,       sizeof(FILETIME));
  memzero(&lCounter, sizeof(word32));
  memzero(&qpc,      sizeof(LARGE_INTEGER));
  memzero(&cryptCtx, sizeof(chacha_ctx));
}
//---------------------------------------------------------------------------
word32 FastPRNG::GetWord32(void)
{
  word32 x = m_m[m_c], y;
  m_m[m_c] = y = m_m[x&0xFF] + m_r;
  m_r = m_m[(y>>8)&0xFF] + x;
  m_c++;

  return m_r;
}
//---------------------------------------------------------------------------
void FastPRNG::GetData(void* pMem, word32 lSize)
{
  word32* pBuf = reinterpret_cast<word32*>(pMem);

  for ( ; lSize >= 4; lSize -= 4)
    *pBuf++ = GetWord32();

  if (lSize != 0) {
    word32 lRand = GetWord32();
    memcpy(pBuf, &lRand, lSize);
  }
}
//---------------------------------------------------------------------------
void FastPRNG::Reset(void)
{
  memzero(m_m, sizeof(m_m));
  m_r = 0;
  m_c = 0;
}
//---------------------------------------------------------------------------
void SplitMix64::Seed(const void* pData, word32 lNumBytes)
{
  m_qState = 0;
  memcpy(&m_qState, pData, std::min<size_t>(lNumBytes, sizeof(word64)));
}
//---------------------------------------------------------------------------
void SplitMix64::GetData(void* pData, word32 lNumBytes)
{
  word64* pBuf = reinterpret_cast<word64*>(pData);

  for ( ; lNumBytes >= 8; lNumBytes -= 8)
    *pBuf++ = GetWord64();

  if (lNumBytes != 0) {
    word64 qRand = GetWord64();
    memcpy(pBuf, &qRand, lNumBytes);
  }
}
//---------------------------------------------------------------------------
word64 SplitMix64::GetWord64(void)
{
  m_qState += 0x9e3779b97f4a7c15ull;
  word64 z = m_qState;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  return z ^ (z >> 31);
}
//---------------------------------------------------------------------------
#if 0
Xoshiro256StarStar::~Xoshiro256StarStar()
{
  memzero(m_qState, sizeof(m_qState));
}
//---------------------------------------------------------------------------
void Xoshiro256StarStar::Seed(const void* pData, word32 lNumBytes)
{
  memzero(m_qState, sizeof(m_qState));
  memcpy(m_qState, pData, std::min<size_t>(sizeof(m_qState), lNumBytes));
}
//---------------------------------------------------------------------------
static inline uint64_t rotl(const word64 x, int k) {
	return (x << k) | (x >> (64 - k));
}

word64 Xoshiro256StarStar::GetWord64(void)
{
  word64 result = rotl(m_qState[1] * 5, 7) * 9;

  const word64 t = m_qState[1] << 17;

  m_qState[2] ^= m_qState[0];
  m_qState[3] ^= m_qState[1];
  m_qState[1] ^= m_qState[2];
  m_qState[0] ^= m_qState[3];

  m_qState[2] ^= t;

  m_qState[3] = rotl(m_qState[3], 45);

  return result;
}
//---------------------------------------------------------------------------
void Xoshiro256StarStar::GetData(void* pData, word32 lNumBytes)
{
  word64* pBuf = reinterpret_cast<word64*>(pData);

  for ( ; lNumBytes >= 8; lNumBytes -= 8)
    *pBuf++ = GetWord64();

  if (lNumBytes != 0) {
    word64 qRand = GetWord64();
    memcpy(pBuf, &qRand, lNumBytes);
  }
}
#endif
