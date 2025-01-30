// MemUtil.cpp
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
#include <mem.h>
#pragma hdrstop

#include "MemUtil.h"
#include "chacha.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


[[clang::optnone]] void memzero(void* pMem, size_t size)
{
  memset(pMem, 0, size);
}
//---------------------------------------------------------------------------
void memcrypt(const word8* pSrc,
  word8* pDest,
  word32 lSize,
  const word8* pKey,
  word32 lKeySize)
{
  if (lKeySize < 16)
    throw std::runtime_error("memcrypt: Key too small");

  chacha_ctx cryptCtx;
  chacha_keysetup(&cryptCtx, pKey, (lKeySize >= 32) ? 256 : 128);

  // encryption and decryption are the same for stream cipher ChaCha
  chacha_encrypt_bytes(&cryptCtx, pSrc, pDest, lSize);

  memzero(&cryptCtx, sizeof(chacha_ctx));
}
//---------------------------------------------------------------------------
[[clang::optnone]] void eraseVclString(UnicodeString& s)
{
  if (!s.IsEmpty()) {
    for (auto& ch : s)
      ch = 'x';
    s = UnicodeString();
  }
}
//---------------------------------------------------------------------------
