// hmac.h
//
// PASSWORD TECH
// Copyright (c) 2002-2026 by Christian Thoeing <c.thoeing@web.de>
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
#ifndef hmacH
#define hmacH
//---------------------------------------------------------------------------
#include "sha256.h"
#include "sha512.h"
#include "SecureMem.h"

namespace HMAC {

class Base
{
public:
  virtual ~Base() {};
  virtual void Update(const word8* pData, word32 lNumBytes) = 0;
  virtual SecureMem<word8> Finish() = 0;
  virtual void Reset() = 0;
  virtual word32 GetLength() const = 0;
};

class SHA256 : public Base
{
public:
  SHA256(const word8* pKey, word32 lKeyLen)
  {
    sha256_init(&m_ctx);
    sha256_hmac_starts(&m_ctx, pKey, lKeyLen, 0);
  }

  ~SHA256()
  {
    memzero(&m_ctx, sizeof(m_ctx));
  }

  void Update(const word8* pData, word32 lNumBytes) override
  {
    sha256_hmac_update(&m_ctx, pData, lNumBytes);
  }

  SecureMem<word8> Finish() override
  {
    SecureMem<word8> result(32);
    sha256_hmac_finish(&m_ctx, result);
    return result;
  }

  void Reset() override
  {
    sha256_hmac_reset(&m_ctx);
  }

  word32 GetLength() const override
  {
    return 32;
  }

private:
  sha256_context m_ctx;
};

class SHA512 : public Base
{
public:
  SHA512(const word8* pKey, word32 lKeyLen)
  {
    sha512_init(&m_ctx);
    sha512_hmac_starts(&m_ctx, pKey, lKeyLen, 0);
  }

  ~SHA512()
  {
    memzero(&m_ctx, sizeof(m_ctx));
  }

  void Update(const word8* pData, word32 lNumBytes) override
  {
    sha512_hmac_update(&m_ctx, pData, lNumBytes);
  }

  SecureMem<word8> Finish() override
  {
    SecureMem<word8> result(64);
    sha512_hmac_finish(&m_ctx, result);
    return result;
  }

  void Reset() override
  {
    sha512_hmac_reset(&m_ctx);
  }

  word32 GetLength() const override
  {
    return 64;
  }

private:
  sha512_context m_ctx;
};

}


#endif
