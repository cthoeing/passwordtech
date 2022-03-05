// CryptClip.cpp
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
#include <vcl.h>
#include <clipbrd.hpp>
#pragma hdrstop

#include "CryptText.h"
#include "aes.h"
#include "sha256.h"
#include "base64.h"
#include "minilzo.h"
#include "Util.h"
#include "CryptUtil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

static const word8
CRYPTTEXT_MAGIC[3] = { 'P', 'W', 'G' };

struct CryptTextHeader {
  word8 Magic[3];
  word8 Version;
  word32 TextBytes;
};

static const int
HMAC_LENGTH        = 32,
HEADER_SIZE        = sizeof(CryptTextHeader),
BASE64_LINE_LENGTH = 76;

//---------------------------------------------------------------------------
static void initCrypto(aes_context* pCryptCtx,
  sha256_context* pHashCtx,
  const word8* pPassw,
  int nPasswLen,
  const word8* pSalt,
  bool blEncrypt,
  int nVersion = CRYPTTEXT_VERSION)
{
  SecureMem<word8> derivedKey(32);

  if (nVersion == 0) {
    derivedKey.Clear();
    memcpy(derivedKey, pSalt, 16);

    // hash the salt and the key together 8192 times
    for (int nI = 0; nI < 8192; nI++) {
      sha256_starts(pHashCtx, 0);
      sha256_update(pHashCtx, derivedKey, 32);
      sha256_update(pHashCtx, pPassw, nPasswLen);
      sha256_finish(pHashCtx, derivedKey);
    }
  }
  else
    pbkdf2_256bit(pPassw, nPasswLen, pSalt, 16, derivedKey);

  if (blEncrypt)
    aes_setkey_enc(pCryptCtx, derivedKey, 256);
  else
    aes_setkey_dec(pCryptCtx, derivedKey, 256);

  sha256_hmac_starts(pHashCtx, derivedKey, 32, 0);
}


//---------------------------------------------------------------------------
int EncryptText(const SecureWString* psText,
  const word8* pPassw,
  int nPasswLen,
  RandomGenerator* pRandGen)
{
  try {
    SecureWString sText;

    if (psText != NULL)
      sText = *psText;
    else if (!GetClipboardTextBuf(&sText, NULL))
      return CRYPTTEXT_ERROR_CLIPBOARD;

    if (sText.Size() <= 1)
      return CRYPTTEXT_ERROR_NOTEXT;

    // get UTF-8 text length
    word32 lTextLen = WideCharToMultiByte(CP_UTF8, 0, sText, -1, NULL, 0, NULL, NULL);

    // convert text length to length in BYTES
//    lTextLen *= sizeof(wchar_t);

    if (lTextLen > CRYPTTEXT_MAXTEXTBYTES)
      return CRYPTTEXT_ERROR_TEXTTOOLONG;

    SecureAnsiString asTextUtf8(lTextLen);

    // convert string to UTF-8
    WideCharToMultiByte(CP_UTF8, 0, sText, -1, asTextUtf8, lTextLen, NULL, NULL);
    lTextLen--; // ignore the terminating '\0' from now onwards

    sText.Empty();

    word32 lBufSize = HEADER_SIZE + lTextLen + lTextLen / 16 + 64 + 3 + HMAC_LENGTH;
    //                ^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //                  header      security margins for compression
    lBufSize = 16 + 16 * ((lBufSize + 15) / 16);
    //         ^^   ^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //         IV   adjust to blocklength

    // allocate some memory for the output buffer and the work buffer
    SecureMem<word8> buf(lBufSize), workBuf(LZO1X_1_MEM_COMPRESS);

    // get a new initialization vector (IV)
    pRandGen->GetData(buf, 16);

    word8* pCryptBuf = buf + 16ul;

    // create header and copy it to the buffer
    CryptTextHeader header;
    memcpy(header.Magic, CRYPTTEXT_MAGIC, sizeof(CRYPTTEXT_MAGIC));
    header.Version = static_cast<word8>(CRYPTTEXT_VERSION);
    header.TextBytes = lTextLen;
    memcpy(pCryptBuf, &header, HEADER_SIZE);

    // compress the text
    lzo_uint comprLen;
    lzo1x_1_compress(asTextUtf8.Bytes(), lTextLen, pCryptBuf + HEADER_SIZE,
      &comprLen, workBuf);

    // destroy the text buffer and the work memory
    asTextUtf8.Empty();
    workBuf.Empty();

    // prepare the crypto stuff...
    word32 lCryptLen = HEADER_SIZE + comprLen + HMAC_LENGTH;

    // store the length modulo 16 in the 16th byte of the IV
    word8 bLastBlock = lCryptLen & 0x0F;
    buf[15] = (buf[15] & 0xF0) | bLastBlock;

    if (bLastBlock != 0) {
      pRandGen->GetData(buf + 16ul + lCryptLen, 16 - bLastBlock);
      lCryptLen += 16 - bLastBlock;
    }

    // create a buffer for the IV
    word8 iv[16];
    memcpy(iv, buf, 16);

    // initialize the cipher and the HMAC
    SecureMem<aes_context> cryptCtx(1);
    SecureMem<sha256_context> hashCtx(1);
    initCrypto(cryptCtx, hashCtx, pPassw, nPasswLen, iv, true);

    // compute the HMAC of the compressed message
    word8 hmac[32];
    sha256_hmac_update(hashCtx, pCryptBuf, HEADER_SIZE + comprLen);
    sha256_hmac_finish(hashCtx, hmac);

    hashCtx.Empty();

    // copy the HMAC to the buffer
    memcpy(pCryptBuf + HEADER_SIZE + comprLen, hmac, HMAC_LENGTH);

    // now encrypt the buffer (*with* HMAC)
    aes_crypt_cbc(cryptCtx, AES_ENCRYPT, lCryptLen, iv, pCryptBuf, pCryptBuf);

    cryptCtx.Empty();

    word32 lConvertLen = 16 + lCryptLen;

    // get the required base64 buffer size
    size_t outBufSize = 0;
    base64_encode(NULL, &outBufSize, NULL, lConvertLen, BASE64_LINE_LENGTH);

    // create a new buffer for base64
    SecureMem<word8> outBuf(outBufSize + 1);
    base64_encode(outBuf, &outBufSize, buf, lConvertLen, BASE64_LINE_LENGTH);

    outBuf[static_cast<word32>(outBufSize)] = '\0';

    // copy the output buffer to the clipboard
    SetClipboardTextBuf(NULL, reinterpret_cast<char*>(outBuf.Data()));
  }
  catch (std::bad_alloc& e) {
    return CRYPTTEXT_ERROR_OUTOFMEMORY;
  }
  catch (EOutOfMemory& e) {
    return CRYPTTEXT_ERROR_OUTOFMEMORY;
  }
  catch (EFOpenError& e) {
    return CRYPTTEXT_ERROR_CLIPBOARD;
  }
  catch (...) {
    throw;
  }

  return CRYPTTEXT_OK;
}
//---------------------------------------------------------------------------
int DecryptText(const SecureWString* psText,
  const word8* pPassw,
  int nPasswLen,
  int nVersion)
{
  try {
    SecureAnsiString asText;
    word32 lTextLen;

    if (psText != NULL) {
      if (psText->Size() <= 1)
        return CRYPTTEXT_ERROR_NOTEXT;
      const wchar_t* pwszBuf = psText->c_str();
      lTextLen = wcslen(pwszBuf) + 1;
      asText.New(lTextLen);
      for (word32 lI = 0; lI < lTextLen; lI++)
        asText[lI] = pwszBuf[lI];
    }
    else {
      if (!GetClipboardTextBuf(NULL, &asText))
        return CRYPTTEXT_ERROR_CLIPBOARD;
      if (asText.IsEmpty())
        return CRYPTTEXT_ERROR_NOTEXT;
      lTextLen = asText.StrLen();
    }

    // base64-decode the text
    size_t bufSize = 0;
    word32 lMinLen = (nVersion == 0) ? 48 : 64;
    if (base64_decode(NULL, &bufSize, asText.Bytes(), lTextLen)
      == POLARSSL_ERR_BASE64_INVALID_CHARACTER || bufSize < lMinLen)
      return CRYPTTEXT_ERROR_TEXTCORRUPTED;

    SecureMem<word8> buf(bufSize);
    base64_decode(buf, &bufSize, reinterpret_cast<word8*>(asText.Data()), lTextLen);

    asText.Empty();

    // length must be a multiple of 16!
    if ((bufSize & 0x0F) != 0)
      return CRYPTTEXT_ERROR_TEXTCORRUPTED;

    word8* pCryptBuf = buf + 16ul;
    word32 lHmacLen = (nVersion == 0) ? 16 : HMAC_LENGTH;
    word32 lCryptLen = bufSize - 16;

    if (lCryptLen < 16 + lHmacLen)
      return CRYPTTEXT_ERROR_TEXTCORRUPTED;

    CryptTextHeader header;
    word32 lHeaderSize = (nVersion == 0) ? 4 : HEADER_SIZE;

    // time to start the crypto engine...
    word8 iv[16];
    memcpy(iv, buf, 16);
    SecureMem<aes_context> cryptCtx(1);
    SecureMem<sha256_context> hashCtx(1);
    initCrypto(cryptCtx, hashCtx, pPassw, nPasswLen, iv, false, nVersion);

    word8 bLastBlock = buf[15] & 0x0F;
    if (bLastBlock == 0)
      bLastBlock = 16;

    word8 hmac[32];
    bool encHmac;

    if (nVersion == 0) {
      lCryptLen -= lHmacLen;
      encHmac = false;

      // compute the HMAC of the ciphertext
      sha256_hmac_update(hashCtx, buf, bufSize - 16);
      sha256_hmac_finish(hashCtx, hmac);

      hashCtx.Empty();

      // verify the HMAC
      if (memcmp(pCryptBuf + lCryptLen, hmac, lHmacLen) != 0)
        return CRYPTTEXT_ERROR_BADKEY;

      // decrypt the buffer
      aes_crypt_cbc(cryptCtx, AES_DECRYPT, lCryptLen, iv, pCryptBuf, pCryptBuf);

      // get the text length
      memcpy(&header.TextBytes, pCryptBuf, lHeaderSize);
    }
    else {
      // decrypt the first block and check the magic string
      aes_crypt_cbc(cryptCtx, AES_DECRYPT, 16, iv, pCryptBuf, pCryptBuf);

      memcpy(&header, pCryptBuf, HEADER_SIZE);

      if (memcmp(header.Magic, CRYPTTEXT_MAGIC, sizeof(CRYPTTEXT_MAGIC)) != 0)
        return CRYPTTEXT_ERROR_BADKEY;

      if (header.TextBytes == 0 || header.TextBytes > CRYPTTEXT_MAXTEXTBYTES)
        return CRYPTTEXT_ERROR_BADKEY;

      encHmac = header.Version >= 3;
      if (!encHmac)
        lCryptLen -= lHmacLen;

      // now decrypt the rest
      if (lCryptLen > 16)
        aes_crypt_cbc(cryptCtx, AES_DECRYPT, lCryptLen - 16, iv,
          pCryptBuf + 16, pCryptBuf + 16);

      if (encHmac)
        lCryptLen -= 16 - bLastBlock;

      // compute the HMAC of the plaintext
      sha256_hmac_update(hashCtx, pCryptBuf, (encHmac) ?
        lCryptLen - lHmacLen : lCryptLen);
      sha256_hmac_finish(hashCtx, hmac);

      hashCtx.Empty();

      // verify the HMAC
      if (memcmp((encHmac) ? pCryptBuf + lCryptLen - lHmacLen :
          pCryptBuf + lCryptLen, hmac, lHmacLen) != 0)
        return CRYPTTEXT_ERROR_BADKEY;
    }

    cryptCtx.Empty();

    // create the output buffer
    SecureAnsiString asOutBuf(header.TextBytes + 1);

    // get the exact length of the compressed data
    word32 lToDecompr = lCryptLen - lHeaderSize;
    if (encHmac)
      lToDecompr -= lHmacLen;
    else
      lToDecompr -= 16 - bLastBlock;

//    ShowMessage(nToDecompr); // debug

    // decompress this buffer
    lzo_uint decomprLen = header.TextBytes;
    if (lzo1x_decompress_safe(pCryptBuf + lHeaderSize, lToDecompr,
         reinterpret_cast<word8*>(asOutBuf.Data()), &decomprLen, NULL) != LZO_E_OK)
      return CRYPTTEXT_ERROR_DECOMPRFAILED;

    asOutBuf[static_cast<word32>(decomprLen)] = '\0';

    // copy the output buffer to the clipboard
    if (nVersion >= 2) {
      // convert UTF-8-encoded string to UTF-16
      lTextLen =
        MultiByteToWideChar(CP_UTF8, 0, asOutBuf, -1, NULL, 0);

      if (lTextLen == 0)
        return CRYPTTEXT_ERROR_TEXTCORRUPTED;

      SecureWString sTextUtf16(lTextLen);

      MultiByteToWideChar(CP_UTF8, 0, asOutBuf, -1, sTextUtf16, lTextLen);

      SetClipboardTextBuf(sTextUtf16, NULL);
    }
    else
      SetClipboardTextBuf(NULL, asOutBuf);
  }
  catch (std::bad_alloc& e) {
    return CRYPTTEXT_ERROR_OUTOFMEMORY;
  }
  catch (EOutOfMemory& e) {
    return CRYPTTEXT_ERROR_OUTOFMEMORY;
  }
  catch (EFOpenError& e) {
    return CRYPTTEXT_ERROR_CLIPBOARD;
  }
  catch (...) {
    throw;
  }

  return CRYPTTEXT_OK;
}
//---------------------------------------------------------------------------

