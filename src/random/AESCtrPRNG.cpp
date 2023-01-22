// AESCtrPRNG.cpp
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

#include "AESCtrPRNG.h"
#include "sha256.h"
#include "CryptUtil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


//---------------------------------------------------------------------------
void AESCtrPRNG::Seed(const void* pSeed,
  word32 lSeedLen)
{
  // derive a 256-bit key from the seed value
  sha256(reinterpret_cast<const word8*>(pSeed), lSeedLen, m_initialKey, 0);

  Reset();
}
//---------------------------------------------------------------------------
void AESCtrPRNG::SeedWithKey(const word8* pKey,
  word32 lKeySize,
  const word8* pParam,
  word32 lParamSize)
{
  pbkdf2_256bit(pKey, lKeySize, pParam, lParamSize, m_initialKey);

  Reset();
}
//---------------------------------------------------------------------------
void AESCtrPRNG::Reset(void)
{
  aes_setkey_enc(&m_cipherCtx, m_initialKey, KEY_SIZE*8);
  m_counter.Zeroize();
  m_lGetBufPos = GETBUF_SIZE;
  m_lNumOfBlocks = 0;
}
//---------------------------------------------------------------------------
void AESCtrPRNG::FillGetBuf(void)
{
  for (word32 i = 0; i < GETBUF_SIZE; i += BLOCK_SIZE) {
    // encrypt current counter value and increment it by 1 afterwards
    aes_crypt_ecb(&m_cipherCtx, AES_ENCRYPT, m_counter, m_getBuf + i);
    incrementCounter<128>(m_counter);
    m_lNumOfBlocks++;
  }

  if (m_lNumOfBlocks >= MAX_BLOCKS) {
    SecureMem<word8> newKey(KEY_SIZE);
    aes_crypt_ecb(&m_cipherCtx, AES_ENCRYPT, m_counter, newKey);
    incrementCounter<128>(m_counter);
    aes_crypt_ecb(&m_cipherCtx, AES_ENCRYPT, m_counter, newKey +
      static_cast<word32>(BLOCK_SIZE));
    incrementCounter<128>(m_counter);

    aes_setkey_enc(&m_cipherCtx, newKey, KEY_SIZE*8);
    m_lNumOfBlocks = 0;
  }

  m_lGetBufPos = 0;
}
//---------------------------------------------------------------------------
void AESCtrPRNG::GetData(void* pBuf,
  word32 lNumOfBytes)
{
  word8* pDestBuf = reinterpret_cast<word8*>(pBuf);

  while (lNumOfBytes != 0) {
    if (m_lGetBufPos == GETBUF_SIZE)
      FillGetBuf();

    word32 lToCopy = std::min(lNumOfBytes, GETBUF_SIZE - m_lGetBufPos);
    memcpy(pDestBuf, m_getBuf + m_lGetBufPos, lToCopy);

    pDestBuf += lToCopy;
    m_lGetBufPos += lToCopy;
    lNumOfBytes -= lToCopy;
  }
}
//---------------------------------------------------------------------------
word8 AESCtrPRNG::GetByte(void)
{
  if (m_lGetBufPos == GETBUF_SIZE)
    FillGetBuf();

  return m_getBuf[m_lGetBufPos++];
}
//---------------------------------------------------------------------------

