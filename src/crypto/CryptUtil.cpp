// CryptUtil.cpp
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
#pragma hdrstop

#include "CryptUtil.h"
#include "sha256.h"
#include "SecureMem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


void pbkdf2_256bit(const word8* pPassw,
  int nPasswLen,
  const word8* pSalt,
  int nSaltLen,
  word8* pDerivedKey,
  int nIterations)
{
  const word8 counter[4] = { 0, 0, 0, 1 };
  sha256_context hashCtx;
  sha256_init(&hashCtx);

  // derive a 256-bit key according to PBKDF2, using HMAC-SHA-256 as the
  // pseudorandom function (PRF)

  // compute U_1 = HMAC(key, salt || counter)
  sha256_hmac_starts(&hashCtx, pPassw, nPasswLen, 0);
  sha256_hmac_update(&hashCtx, pSalt, nSaltLen);
  sha256_hmac_update(&hashCtx, counter, 4);
  sha256_hmac_finish(&hashCtx, pDerivedKey);

  SecureMem<word8> md(pDerivedKey, 32);

  for (int i = 1; i < nIterations; i++) {
    // compute U_i = HMAC(key, U_{i-1})
    sha256_hmac_reset(&hashCtx);
    sha256_hmac_update(&hashCtx, md, 32);
    sha256_hmac_finish(&hashCtx, md);

    // compute U_1 ^ U_2 ^ ... U_c
    for (int j = 0; j < 32; j++)
      pDerivedKey[j] ^= md[j];
  }

  memzero(&hashCtx, sizeof(hashCtx));
}
//---------------------------------------------------------------------------
