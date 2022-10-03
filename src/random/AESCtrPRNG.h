// AESCtrPRNG.h
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
#ifndef AESCtrPRNGH
#define AESCtrPRNGH
//---------------------------------------------------------------------------
#include "RandomGenerator.h"
#include "aes.h"
#include "SecureMem.h"

class AESCtrPRNG : public RandomGenerator
{
private:
  aes_context m_cipherCtx;
  SecureMem<word8> m_initialKey;
  SecureMem<word8> m_counter;
  SecureMem<word8> m_getBuf;
  word32 m_lGetBufPos;
  word32 m_lNumOfBlocks;

  void FillGetBuf(void);

public:

  enum {
    BLOCK_SIZE  = 16,
    KEY_SIZE    = 32,
    GETBUF_SIZE = 64,
    MAX_BLOCKS  = 65536
  };

  AESCtrPRNG()
    : m_initialKey(KEY_SIZE), m_counter(BLOCK_SIZE), m_getBuf(GETBUF_SIZE)
  {
  }

  ~AESCtrPRNG()
  {
    memzero(&m_cipherCtx, sizeof(m_cipherCtx));
    m_lGetBufPos = 0;
    m_lNumOfBlocks = 0;
  }

  void Seed(const void* pSeed,
    word32 lSeedLen);

  void SeedWithKey(const word8* pKey,
    word32 lKeySize,
    const word8* pParam,
    word32 lParamSize);

  void Reset(void);

  void GetData(void* pDest,
    word32 lNumOfBytes);

  word8 GetByte(void);
};


#endif
