// RandomGenerator.h
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
#ifndef RandomGeneratorH
#define RandomGeneratorH
//---------------------------------------------------------------------------
#include "types.h"

class RandomGenerator
{
public:

  RandomGenerator()
  {}

  virtual ~RandomGenerator()
  {}

  // seeds the PRNG
  // -> pointer to the seed data
  // -> size of this data
  virtual void Seed(const void* pSeed,
    word32 lSeedLen) = 0;

  // seeds PRNG with key & parameter
  // (in contrast to Seed() this function does NOT have to be implemented
  // for derived classes!)
  // -> key
  // -> key size
  // -> parameter
  // -> parameter size
  virtual void SeedWithKey(const word8* pKey,
    word32 lKeySize,
    const word8* pParam,
    word32 lParamSize)
  {
    throw Exception("SeedWithKey() not implemented");
  }

  // resets internal state to the last seed value
  virtual void Reset(void)
  {
    throw Exception("Reset() not implemented");
  }

  // reseeds the PRNG with system parameters
  virtual void Randomize(void);

  // copies random bytes into the target buffer
  // -> target buffer
  // -> desired number of bytes
  virtual void GetData(void* pDest,
    word32 lNumOfBytes) = 0;

  // returns a random integer (64-bit)
  virtual word64 GetWord64(void) {
    word64 qRand;
    GetData(&qRand, sizeof(qRand));
    return qRand;
  }

  // returns a random integer (32-bit)
  virtual word32 GetWord32(void) {
    word32 lRand;
    GetData(&lRand, sizeof(lRand));
    return lRand;
  }

  // returns a random integer (16-bit)
  virtual word16 GetWord16(void) {
    word16 wRand;
    GetData(&wRand, sizeof(wRand));
    return wRand;
  }

  // returns a random byte (8-bit)
  virtual word8 GetByte(void) {
    word8 bRand;
    GetData(&bRand, sizeof(bRand));
    return bRand;
  }

  word32 GetNumRange(word32 lNum) {
    if (lNum == 0)
      throw Exception("Invalid range");

    if (lNum == 1)
      return 0;

    // NOTE: actually, in a 100% correct implementation, in the last case
    // it should be
    //   lRandMax = lNum * (0x100000000 / lNum),
    // but handling a 32-bit integer here is much faster than
    // handling a 64-bit integer.
    // We can get along with the disadvantage that
    // for each lNum being a power of 2, some random numbers
    // may be discarded erroneously. The speed advantage of 32-bit
    // vs. 64-bit far outweighs this disadvantage.
    word32 lRandMax, lRand;

    if (lNum <= 256) {
      lRandMax = lNum * (256 / lNum);
      while ((lRand = GetByte()) >= lRandMax);
    }
    else if (lNum <= 65536) {
      lRandMax = lNum * (65536 / lNum);
      while ((lRand = GetWord16()) >= lRandMax);
    }
    else {
#ifdef _WIN64
      lRandMax = lNum * (0x100000000ll / lNum);
#else
      lRandMax = lNum * (0xFFFFFFFF / lNum); // see above
#endif
      while ((lRand = GetWord32()) >= lRandMax);
    }

    return lRand % lNum;
  }

  word32 GetNumRange(word32 lBegin, word32 lEnd)
  {
    if (lEnd <= lBegin)
      throw Exception("Invalid range");

    return lBegin + GetNumRange(lEnd - lBegin);
  }

  template<class T> void Permute(T* pArray,
    word32 lSize)
  {
    for (word32 i = lSize - 1; i > 0; i--) {
      word32 lRand = GetNumRange(i + 1);
      if (lRand != i)
        std::swap(pArray[i], pArray[lRand]);
    }
  }
};

#endif
