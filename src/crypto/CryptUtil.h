// CryptClip.cpp
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
#ifndef CryptUtilH
#define CryptUtilH
//---------------------------------------------------------------------------
#include <atomic>
#include "types.h"

// derives a 256-bit key from a password and salt using PBKDF2
// this implementation uses HMAC-SHA-256 as the "pseudorandom function" (PRF)
// -> password
// -> password length in bytes
// -> salt
// -> salt length in bytes
// -> where to store the derived key
// -> number of iterations (default: 8192)
void pbkdf2_256bit(const word8* pPassw,
  word32 lPasswLen,
  const word8* pSalt,
  word32 lSaltLen,
  word8* pDerivedKey,
  word32 lIterations = 8192,
  std::atomic<bool>* pCancelFlag = nullptr);

template<int Nbits> void incrementCounter(word8* pCounter)
{
  for (int i = Nbits/8-1; i >= 0 && ++pCounter[i] == 0; i--);
}

inline word32 alignToBlockSize(word32 lLen, word32 lBlockSize)
{
  //return ((lLen + lBlockSize - 1) / lBlockSize) * lBlockSize;
  word32 lRest = lLen % lBlockSize;
  return lRest ? lLen + lBlockSize - lRest : lLen;
}


// increment 128-bit block counter by 1
//inline void incrementCounter_128bit(word8* pCounter)
//{
//  for (int nI = 15; nI >= 0 && ++pCounter[nI] == 0; nI--);
//}


#endif
