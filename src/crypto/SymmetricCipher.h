// SymmetricCipher.h
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
#ifndef SymmetricCipherH
#define SymmetricCipherH
//---------------------------------------------------------------------------
#include "aes.h"
#include "chacha.h"
#include "MemUtil.h"

namespace EncryptionAlgorithm {

enum class Mode {
  ENCRYPT, DECRYPT
};

class SymmetricCipher
{
public:
  virtual ~SymmetricCipher() {};

  // encrypt/decrypt block of bytes
  // for block ciphers, size must be a multiple of the block size
  // -> input buffer
  // -> output buffer
  // -> key size in bytes
  virtual void Encrypt(const word8* pIn, word8* pOut, word32 lNumBytes) = 0;
  virtual void Decrypt(const word8* pIn, word8* pOut, word32 lNumBytes) = 0;

  // set initialization vector (IV)
  virtual void SetIV(const word8* pIV) {};

  // get supported key sizes in bytes
  virtual std::vector<word32> GetKeySizes(void) const = 0;

  // get block size in bytes
  // can be >1 for stream ciphers, depending on the implementation
  // (for example, if stream cipher is based on a 128-bit block cipher and the
  // key stream is not buffered for consecutive function calls, sequential
  // encryption/decryption of data blocks that are not multiples of 128-bit
  // is not possible)
  virtual word32 GetBlockSize(void) const = 0;

  // get size of initialization vector in bytes
  virtual word32 GetIVSize(void) const = 0;

  // indicates whether cipher is a stream cipher (TRUE) or block cipher (FALSE)
  virtual bool IsStreamCipher(void) const = 0;

  // indicates whether data to be encrypted/decrypted has to be aligned
  // to block size
  virtual bool AlignToBlockSize(void) const = 0;
};

class AES_CBC : public SymmetricCipher
{
public:

  AES_CBC(const word8* pKey, word32 lKeySize, Mode mode)
  {
    if (mode == Mode::ENCRYPT)
      aes_setkey_enc(&m_ctx, pKey, lKeySize*8);
    else
      aes_setkey_dec(&m_ctx, pKey, lKeySize*8);
    memzero(m_iv, 16); // avoid an undefined state
  }

  ~AES_CBC()
  {
    memzero(&m_ctx, sizeof(aes_context));
    memzero(m_iv, 16);
  }

  std::vector<word32> GetKeySizes(void) const override
  {
    return {16, 24, 32};
  }

  word32 GetBlockSize(void) const override
  {
    return 16;
  }

  word32 GetIVSize(void) const override
  {
    return 16;
  }

  bool IsStreamCipher(void) const override
  {
    return false;
  }

  bool AlignToBlockSize(void) const override
  {
    return true;
  }

  void Encrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    aes_crypt_cbc(&m_ctx, AES_ENCRYPT, lNumBytes, m_iv, pIn, pOut);
  }

  void Decrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    aes_crypt_cbc(&m_ctx, AES_DECRYPT, lNumBytes, m_iv, pIn, pOut);
  }

  void SetIV(const word8* pIV) override
  {
    memcpy(m_iv, pIV, 16);
  }

protected:
  aes_context m_ctx;
  word8 m_iv[16];
};

class ChaCha20 : public SymmetricCipher
{
public:

  ChaCha20(const word8* pKey, word32 lKeySize)
  {
    chacha_keysetup(&m_ctx, pKey, lKeySize*8);
  }

  ~ChaCha20()
  {
    memzero(&m_ctx, sizeof(chacha_ctx));
  }

  std::vector<word32> GetKeySizes(void) const override
  {
    return {16, 32};
  }

  word32 GetBlockSize(void) const override
  {
    return 64;
  }

  word32 GetIVSize(void) const override
  {
    return 8;
  }

  bool IsStreamCipher(void) const override
  {
    return true;
  }

  bool AlignToBlockSize(void) const override
  {
    return false;
  }

  void Encrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    chacha_encrypt_bytes(&m_ctx, pIn, pOut, lNumBytes);
  }

  void Decrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    Encrypt(pIn, pOut, lNumBytes);
  }

  void SetIV(const word8* pIV) override
  {
    chacha_ivsetup(&m_ctx, pIV, nullptr);
  }

protected:
  chacha_ctx m_ctx;
};

#ifdef _DEBUG
class NullCipher : public SymmetricCipher
{
public:
  NullCipher()
  {
  }

  ~NullCipher()
  {
  }

  std::vector<word32> GetKeySizes(void) const override
  {
    return {0};
  }

  word32 GetBlockSize(void) const override
  {
    return 16;
  }

  word32 GetIVSize(void) const override
  {
    return 16;
  }

  bool IsStreamCipher(void) const override
  {
    return false;
  }

  bool AlignToBlockSize(void) const override
  {
    return true;
  }

  void Encrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    if (pIn != pOut) {
      while (lNumBytes--)
        *pOut++ = *pIn++;
    }
  }

  void Decrypt(const word8* pIn, word8* pOut, word32 lNumBytes) override
  {
    Encrypt(pIn, pOut, lNumBytes);
  }
};

#endif

}

#endif
