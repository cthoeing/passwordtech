// FastPRNG.h
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
#ifndef FastPRNGH
#define FastPRNGH
//---------------------------------------------------------------------------
#include "types.h"
#include "RandomGenerator.h"

class FastPRNG : public RandomGenerator
{
public:
  FastPRNG()
  {
    Reset();
    Randomize();
  }

  FastPRNG(const FastPRNG& src);

  ~FastPRNG()
  {
    Reset();
  }

  void Seed(const void*, word32) override {}

  void Reset(void) override;

  void Randomize(void) override;

  word32 GetWord32(void) override;

  void GetData(void* pMem, word32 lSize) override;

private:
  word32 m_m[256], m_r;
  word8 m_c;
};

extern FastPRNG g_fastRandGen;

// lets the PRNG reseed itself with a nice seed derived from timers
//void fprng_randomize(void);

// returns a random 32-bit value
inline word32 fprng_rand(void)
{
  return g_fastRandGen.GetWord32();
}

inline word32 fprng_rand(word32 lNum)
{
  return g_fastRandGen.GetWord32() % lNum;
}

// fills buffer with random data, optimized for speed
// -> pointer to the buffer
// -> number of bytes to fill
//void fprng_fill_buf(void* pMem, word32 lSize);

class SplitMix64 : public RandomGenerator
{
public:
  SplitMix64(word64 qSeed) : m_qState(qSeed) {}

  ~SplitMix64() { m_qState = 0; }

  void Seed(const void* pData, word32 lNumBytes) override;

  void GetData(void* pDest, word32 lNumBytes) override;

  word64 GetWord64(void) override;

private:
  word64 m_qState;
};

#if 0
class Xoshiro256StarStar : public RandomGenerator
{
public:
  Xoshiro256StarStar(word64* qSeed = nullptr)
  {
    if (qSeed) Seed(qSeed, sizeof(m_qState));
  }

  ~Xoshiro256StarStar();

  void Seed(const void* pData, word32 lNumBytes) override;

  void GetData(void* pDest, word32 lNumBytes) override;

  word64 GetWord64(void) override;

private:
  word64 m_qState[4];
};
#endif

#endif
