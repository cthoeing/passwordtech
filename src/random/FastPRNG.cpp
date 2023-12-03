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


Jsf32RandGen g_fastRandGen;

void Jsf32RandGen::Randomize()
{
  m_a = 0xf1ea5eed; // avoid short cycles
  word64 timer;
  GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&timer));
  m_b = WORD64_HI(timer);
  m_c = WORD64_LO(timer);
  QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&timer));
  m_d = WORD64_HI(timer) + WORD64_LO(timer);

  for (int i = 0; i < 20; i++) {
    (void) GetWord32();
  }
}
//---------------------------------------------------------------------------
void Jsf32RandGen::Seed(const void* pSeed, word32 lSize)
{
  m_a = 0xf1ea5eed;
  word32 seed[3] = {0, 0, 0};
  memcpy(seed, pSeed, std::min<size_t>(sizeof(seed), lSize));
  m_b = seed[0];
  m_c = seed[1];
  m_d = seed[2];
}
//---------------------------------------------------------------------------
word32 Jsf32RandGen::GetWord32()
{
  return NextRand();
}
//---------------------------------------------------------------------------
inline word32 Jsf32RandGen::NextRand()
{
  const word32 e = m_a - _lrotl(m_b, 27);
  m_a = m_b ^ _lrotl(m_c, 17);
  m_b = m_c + m_d;
  m_c = m_d + e;
  m_d = e + m_a;
  return m_d;
}
//---------------------------------------------------------------------------
void Jsf32RandGen::GetData(void* pMem, word32 lSize)
{
  word32* pBuf = reinterpret_cast<word32*>(pMem);

  for ( ; lSize >= 4; lSize -= 4)
    *pBuf++ = NextRand();

  if (lSize != 0) {
    word32 lRand = GetWord32();
    memcpy(pBuf, &lRand, lSize);
  }
}

#if 0
IARandGen::IARandGen(const IARandGen& src)
{
  memcpy(m_m, src.m_m, sizeof(m_m));
  m_r = src.m_r;
  Randomize();
}
//---------------------------------------------------------------------------
void IARandGen::Randomize(void)
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
word32 IARandGen::GetWord32(void)
{
  word32 x = m_m[m_c], y;
  m_m[m_c] = y = m_m[x&0xFF] + m_r;
  m_r = m_m[(y>>8)&0xFF] + x;
  m_c++;

  return m_r;
}
//---------------------------------------------------------------------------
void IARandGen::GetData(void* pMem, word32 lSize)
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
void IARandGen::Reset(void)
{
  memzero(m_m, sizeof(m_m));
  m_r = 0;
  m_c = 0;
}
//---------------------------------------------------------------------------
void Xoroshiro128::Randomize()
{
  HighResTimer(m_state);
  GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(m_state+2));

  // avoid state of all zeros
  if (m_state[0] == 0 && m_state[1] == 0 && m_state[2] == 0 && m_state[3] == 0)
    m_state[1] = 0x9E3779B9; // golden ratio decimals
}
//---------------------------------------------------------------------------
word32 Xoroshiro128::GetWord32()
{
  const word32 result = _lrotl(m_state[1] * 5, 7) * 9;

	const word32 t = m_state[1] << 9;

	m_state[2] ^= m_state[0];
	m_state[3] ^= m_state[1];
	m_state[1] ^= m_state[2];
	m_state[0] ^= m_state[3];

	m_state[2] ^= t;

	m_state[3] = _lrotl(m_state[3], 11);

	return result;
}
//---------------------------------------------------------------------------
void Xoroshiro128::GetData(void* pMem, word32 lSize)
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
#endif

