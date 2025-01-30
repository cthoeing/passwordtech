// FastPRNG.h
//
// PASSWORD TECH
// Copyright (c) 2002-2025 by Christian Thoeing <c.thoeing@web.de>
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

// Bob Jenkin's Small PRNG ("Jenkins Small Fast 32-bit")
// https://burtleburtle.net/bob/rand/smallprng.html
class Jsf32RandGen : public RandomGenerator
{
public:
  Jsf32RandGen()
  {
    Randomize();
  }

  ~Jsf32RandGen()
  {
    m_a = m_b = m_c = m_d = 0;
  }

  void Seed(const void*, word32) override;

  void Randomize() override;

  word32 GetWord32() override;

  void GetData(void*, word32) override;

private:
  inline word32 NextRand();

  word32 m_a, m_b, m_c, m_d;
};

extern Jsf32RandGen g_fastRandGen;

// returns a random 32-bit value
inline word32 fprng_rand(void)
{
  return g_fastRandGen.GetWord32();
}

inline word32 fprng_rand(word32 lNum)
{
  return g_fastRandGen.GetWord32() % lNum;
}


#if 0
// Bob Jenkin's IA PRNG (precursor of ISAAC CSPRNG)
class IARandGen : public RandomGenerator
{
public:
  IARandGen()
  {
    Reset();
    Randomize();
  }

  IARandGen(const IARandGen& src);

  ~IARandGen()
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

// implementation of Xoroshiro128++ or Xoroshiro128**
class Xoroshiro128 : public RandomGenerator
{
public:
  Xoroshiro128()
  {
    Randomize();
  }

  ~Xoroshiro128()
  {
    memset(m_state, 0, sizeof(m_state));
  }

  void Seed(const void*, word32) override {}

  void Randomize() override;

  word32 GetWord32() override;

  void GetData(void* pMem, word32 lSize) override;

private:
  word32 m_state[4];
};

class SplitMix64 : public RandomGenerator
{
public:
  SplitMix64(word64 qSeed = 0) : m_qState(qSeed) {}

  ~SplitMix64() { m_qState = 0; }

  void Seed(const void* pData, word32 lNumBytes) override;

  void GetData(void* pDest, word32 lNumBytes) override;

  word64 GetWord64(void) override;

private:
  word64 m_qState;
};
#endif

#endif
