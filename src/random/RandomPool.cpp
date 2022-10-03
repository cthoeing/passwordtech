// RandomPool.cpp
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
#include <stdio.h>
#pragma hdrstop

#include "RandomPool.h"
#include "CryptUtil.h"
#include "hrtimer.h"
#include "FastPRNG.h"
#include "aes.h"
#include "chacha.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//RandomPool* g_pRandPool = nullptr;

// sizeof(aes_context) = 280 bytes
// sizeof(chacha_ctx) = 68 bytes
// Speck128 round keys = 272 bytes
// sizeof(sha256_context) = 236 bytes

static const int
POOLPAGE_SIZE    = 4096, // size of the entire pool page in RAM
KEY_SIZE         = 32, // AES key size used
CTR_SIZE         = 16, // AES block size
ADDBUF_SIZE      = 556, // size of add buffer (size optimized for minimal padding in SHA-2)
GETBUF_SIZE      = 64, // size of get buffer
TEMPBUF_SIZE     = CTR_SIZE, // size of temporary buffer
POOL_OFFSET      = 0,
HASHCTX_OFFSET   = POOL_OFFSET + RandomPool::POOL_SIZE,
ADDBUF_OFFSET    = HASHCTX_OFFSET + sizeof(sha256_context),
CIPHERKEY_OFFSET = ADDBUF_OFFSET + ADDBUF_SIZE,
SECCTR_OFFSET    = CIPHERKEY_OFFSET + KEY_SIZE,
CIPHERCTX_OFFSET = SECCTR_OFFSET + CTR_SIZE,
GETBUF_OFFSET    = CIPHERCTX_OFFSET + sizeof(aes_context),
TEMPBUF_OFFSET   = GETBUF_OFFSET + GETBUF_SIZE,
UNUSED_SIZE_MAX  = POOLPAGE_SIZE - (TEMPBUF_OFFSET + TEMPBUF_SIZE),
UNUSED_SIZE_MIN  = 64;

namespace RandPoolCipher {
class AES_CTR : public CtrBasedCipher
{
private:
  aes_context* m_pCtx;

public:
  AES_CTR(aes_context* pCtx)
    : m_pCtx(pCtx)
  {}

  ~AES_CTR()
  {
    m_pCtx = nullptr;
  }

  void SetKey(const word8 pKey[32])
  {
    aes_setkey_enc(m_pCtx, pKey, 256);
  }

  void SelfKey(word8 pKey[32], word8* pCounter)
  {
    FillBlocks(pKey, pCounter, 2);
    SetKey(pKey);
  }

  void ProcessCounterOrIV(word8 pCounter[16])
  {
    aes_crypt_ecb(m_pCtx, AES_ENCRYPT, pCounter, pCounter);
  }

  void FillBlocks(word8* pBuf, word8* pCounter, word32 lNumOfBlocks)
  {
    while (lNumOfBlocks--) {
      aes_crypt_ecb(m_pCtx, AES_ENCRYPT, pCounter, pBuf);
      incrementCounter<128>(pCounter);
      pBuf += 16;
    }
  }

  word32 GetBlockSize(void) const
  {
    return 16;
  }

  word32 GetMaxNumOfBlocks(void) const
  {
    return 65536;
  }

  void Move(void* pNew)
  {
    m_pCtx = reinterpret_cast<aes_context*>(pNew);
    m_pCtx->rk = m_pCtx->buf;
  }
};

class ChaCha : public CtrBasedCipher
{
private:
  chacha_ctx* m_pCtx;
  int m_nRounds;

public:
  ChaCha(chacha_ctx* pCtx, int nRounds)
    : m_pCtx(pCtx), m_nRounds(nRounds)
  {}

  ~ChaCha()
  {
    m_pCtx = nullptr;
    m_nRounds = 0;
  }

  void SetKey(const word8 pKey[32])
  {
    // set ChaCha key, IV can be arbitrary for now,
    // because we're using the cipher in only one direction
    chacha_keysetup(m_pCtx, pKey, 256);
    chacha_nrounds(m_pCtx, m_nRounds);
    //word8 iv[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //chacha_ivsetup(m_pCtx, iv);
  }

  void SelfKey(word8 pKey[32], word8* pCounter)
  {
    chacha_keystream_bytes(m_pCtx, pKey, 32);
    SetKey(pKey);
  }

  void ProcessCounterOrIV(word8 pIV[16])
  {
    chacha_encrypt_bytes(m_pCtx, pIV, pIV, 16);
    chacha_ivsetup(m_pCtx, pIV, pIV + 8);
  }

  void FillBlocks(word8* pBuf, word8* pCounter, word32 lNumOfBlocks)
  {
    chacha_keystream_bytes(m_pCtx, pBuf, lNumOfBlocks * 64);
  }

  word32 GetBlockSize(void) const
  {
    return 64;
  }

  word32 GetMaxNumOfBlocks(void) const
  {
    return 0xffffffff;
  }

  void Move(void* pNew)
  {
    m_pCtx = reinterpret_cast<chacha_ctx*>(pNew);
  }
};

#if 0
class Speck128 : public CtrBasedCipher
{
private:
  word64* m_pRoundKeys;

public:

  Speck128(word8* pBuf)
  {
    m_pRoundKeys = reinterpret_cast<word64*>(pBuf);
  }

  ~Speck128()
  {
    m_pRoundKeys = nullptr;
  }

  #define ER64(x,y,k) (x=_rotr64(x,8), x+=y, x^=k, y=_rotl64(y,3), y^=x)

  void SetKey(const word8 pKey[32])
  {
    const word64* K = reinterpret_cast<const word64*>(pKey);
    word64 i, D = K[3], C = K[2], B = K[1], A = K[0];
	for (i = 0; i < 33;) {
		m_pRoundKeys[i] = A; ER64(B, A, i++);
		m_pRoundKeys[i] = A; ER64(C, A, i++);
		m_pRoundKeys[i] = A; ER64(D, A, i++);
	}
	m_pRoundKeys[i] = A;
  }

  void Encrypt(word64 Pt[2], word64 Ct[2]) const
  {
	//Ct[0] = Pt[0]; Ct[1] = Pt[1];
    word64 A = Pt[0], B = Pt[1];
	for (int i = 0; i < 34; ) ER64(A, B, m_pRoundKeys[i++]);
    Ct[0] = A;
    Ct[1] = B;
  }

  #undef ER64

  void SelfKey(word8 pKey[32], word8* pCounter)
  {
    FillBlocks(pKey, pCounter, 2);
    SetKey(pKey);
  }

  void ProcessCounterOrIV(word8 pIV[16])
  {
    Encrypt(reinterpret_cast<word64*>(pIV), reinterpret_cast<word64*>(pIV));
  }

  void FillBlocks(word8* pBuf, word8* pCounter, word32 lNumOfBlocks)
  {
    while (lNumOfBlocks--) {
      Encrypt(reinterpret_cast<word64*>(pCounter), reinterpret_cast<word64*>(pBuf));
      incrementCounter<128>(pCounter);
      pBuf += 16;
    }
  }

  word32 GetBlockSize(void) const
  {
    return 16;
  }

  word32 GetMaxNumOfBlocks(void) const
  {
    return 65536;
  }

  void Move(void* pNew)
  {
    m_pRoundKeys = reinterpret_cast<word64*>(pNew);
  }

};
#endif
}

using namespace RandPoolCipher;

//---------------------------------------------------------------------------
RandomPool::RandomPool(Cipher cipher, RandomGenerator& fastRandGen,
  bool blLockPhysMem)
  : m_lAddBufPos(0), m_lGetBufPos(0), m_blKeySet(false), m_blCryptProv(false),
    m_fastRandGen(fastRandGen), m_blLockPhysMem(blLockPhysMem)
{
  m_pPoolPage = AllocPoolPage();
  if (m_pPoolPage == nullptr)
    OutOfMemoryError();

  // hide the pool contents *somewhere* within the memory page
  m_lUnusedSize = UNUSED_SIZE_MIN + (m_fastRandGen.GetWord32() %
    (UNUSED_SIZE_MAX - UNUSED_SIZE_MIN + 1));
  SetPoolPointers();

  ChangeCipher(cipher, true);

  if (CryptAcquireContext(&m_cryptProv, nullptr, nullptr, PROV_RSA_FULL, 0))
    m_blCryptProv = true;
  else if (static_cast<signed int>(GetLastError()) == NTE_BAD_KEYSET)
    m_blCryptProv = CryptAcquireContext(
        &m_cryptProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_NEWKEYSET);
}
//---------------------------------------------------------------------------
RandomPool::RandomPool(RandomPool& src, RandomGenerator& fastRandGen,
  bool blLockPhysMem)
  : RandomPool(src.m_cipherType, fastRandGen, blLockPhysMem)
{
  SecureMem<word8> entropy(POOL_SIZE);
  src.GetData(entropy, POOL_SIZE);
  AddData(entropy, POOL_SIZE);
  src.Flush();
}
//---------------------------------------------------------------------------
RandomPool::~RandomPool()
{
  if (m_blCryptProv)
    CryptReleaseContext(m_cryptProv, 0);
  FreePoolPage();
  m_pPoolPage = nullptr;
  m_pPool = nullptr;
  m_pHashCtx = nullptr;
  m_pAddBuf = nullptr;
  m_pCipherKey = nullptr;
  m_pSecCtr = nullptr;
  m_pCipherCtx = nullptr;
  m_pGetBuf = nullptr;
  m_pTempBuf = nullptr;
  m_lUnusedSize = 0;
}
//---------------------------------------------------------------------------
RandomPool* RandomPool::GetInstance(void)
{
  static RandomPool inst(Cipher::ChaCha20, g_fastRandGen, true);
  return &inst;
}
//---------------------------------------------------------------------------
word8* RandomPool::AllocPoolPage(void)
{
  word8* pPoolPage = nullptr;
  if (m_blLockPhysMem) {
    pPoolPage =
      reinterpret_cast<word8*>(VirtualAlloc(nullptr, POOLPAGE_SIZE,
        MEM_COMMIT, PAGE_READWRITE));
    if (pPoolPage != nullptr)
      VirtualLock(pPoolPage, POOLPAGE_SIZE);
  }
  else
    pPoolPage = new word8[POOLPAGE_SIZE];
  if (pPoolPage != nullptr)
    ClearPoolBuf(pPoolPage, POOLPAGE_SIZE);
  return pPoolPage;
}
//---------------------------------------------------------------------------
void RandomPool::FreePoolPage(void)
{
  if (m_pPoolPage != nullptr) {
    ClearPoolBuf(m_pPoolPage, POOLPAGE_SIZE);
    if (m_blLockPhysMem) {
      VirtualUnlock(m_pPoolPage, POOLPAGE_SIZE);
      VirtualFree(m_pPoolPage, 0, MEM_RELEASE);
    }
    else
      delete [] m_pPoolPage;
  }
}
//---------------------------------------------------------------------------
void RandomPool::SetPoolPointers(void)
{
  m_pPool      = m_pPoolPage + m_lUnusedSize + POOL_OFFSET;
  m_pHashCtx   = reinterpret_cast<sha256_context*>
    (m_pPoolPage + m_lUnusedSize + HASHCTX_OFFSET);
  m_pAddBuf    = m_pPoolPage + m_lUnusedSize + ADDBUF_OFFSET;
  m_pCipherKey = m_pPoolPage + m_lUnusedSize + CIPHERKEY_OFFSET;
  m_pSecCtr    = m_pPoolPage + m_lUnusedSize + SECCTR_OFFSET;
  m_pCipherCtx = m_pPoolPage + m_lUnusedSize + CIPHERCTX_OFFSET;
  m_pGetBuf    = m_pPoolPage + m_lUnusedSize + GETBUF_OFFSET;
  m_pTempBuf   = m_pPoolPage + m_lUnusedSize + TEMPBUF_OFFSET;
}
//---------------------------------------------------------------------------
void RandomPool::TouchPool(void)
{
  static word32 lIndex = 0;

  if (lIndex == m_lUnusedSize)
    lIndex = 0;
  m_pPoolPage[lIndex++]--;
}
//---------------------------------------------------------------------------
bool RandomPool::MovePool(void)
{
  word8* pNewPoolPage = AllocPoolPage();
  if (pNewPoolPage == nullptr)
    return false;

  // only copy the relevant portion of the pool page
  memcpy(pNewPoolPage + m_lUnusedSize, m_pPoolPage + m_lUnusedSize,
    POOLPAGE_SIZE - m_lUnusedSize);

  FreePoolPage();
  m_pPoolPage = pNewPoolPage;
  pNewPoolPage = nullptr;
  SetPoolPointers();
  m_pCipher->Move(m_pCipherCtx);

  // IMPORTANT: The AES context contains a pointer to its
  // internal key buffer. We have to change this pointer to
  // the new address to avoid an access violation!!
  //if (m_blKeySet)
  //  m_pCipherCtx->rk = m_pCipherCtx->buf;

  return true;
}
//---------------------------------------------------------------------------
void RandomPool::ChangeCipher(Cipher cipher,
  bool blForce)
{
  if (blForce || cipher != m_cipherType) {
    switch (cipher) {
    case Cipher::AES_CTR:
      m_pCipher.reset(new AES_CTR(reinterpret_cast<aes_context*>(m_pCipherCtx)));
      break;
    case Cipher::ChaCha20:
    case Cipher::ChaCha8:
      m_pCipher.reset(new ChaCha(reinterpret_cast<chacha_ctx*>(m_pCipherCtx),
        cipher == Cipher::ChaCha20 ? 20 : 8));
      break;
#if 0
    case Cipher::Speck128:
      m_pCipher.reset(new Speck128(m_pCipherCtx));
      break;
#endif
    default:
      throw Exception("RandomPool: Cipher not supported");
    }
    m_cipherType = cipher;

    // if key has already been set, do a key setup with the new cipher
    if (m_blKeySet)
      SetKey();
  }
}
//---------------------------------------------------------------------------
void RandomPool::UpdatePool(void)
{
  // update the pool by hashing the pool, the add buffer and a time-stamp
  sha256_init(m_pHashCtx);
  sha256_hmac_starts(m_pHashCtx, m_pPool, POOL_SIZE, 0);

  if (m_lAddBufPos != 0) {
    sha256_hmac_update(m_pHashCtx, m_pAddBuf, m_lAddBufPos);
    ClearPoolBuf(m_pAddBuf, m_lAddBufPos);
    m_lAddBufPos = 0;
  }

  if (m_blKeySet) {
    // the counter has accumulated some entropy, which we should incorporate
    // into the pool now
    sha256_hmac_update(m_pHashCtx, m_pSecCtr, CTR_SIZE);
  }

  // ALWAYS add some entropy (independent of the add buffer)
  HighResTimer(m_pTempBuf);
  sha256_hmac_update(m_pHashCtx, m_pTempBuf, sizeof(word64));

  // get new pool
  sha256_hmac_finish(m_pHashCtx, m_pPool);

  // clear sensitive data
  ClearPoolBuf(m_pHashCtx, sizeof(sha256_context));
  ClearPoolBuf(m_pTempBuf, sizeof(word64));

  if (m_blKeySet) {
    // destroy sensitive data used for generating random numbers
    ClearPoolBuf(m_pSecCtr, CTR_SIZE);
    ClearPoolBuf(m_pCipherCtx, sizeof(aes_context));
    ClearPoolBuf(m_pGetBuf, GETBUF_SIZE);
    m_lGetBufPos = 0;
    m_lNumOfBlocks = 0;
    m_blKeySet = false;
  }
}
//---------------------------------------------------------------------------
void RandomPool::SetKey(void)
{
  // always update the pool before generating a new key
  UpdatePool();

  // don't use the pool directly as a key, but derive a new key from the pool
  // using a time stamp as a "key" for HMAC-SHA-256
  HighResTimer(m_pTempBuf);
  sha256_init(m_pHashCtx);
  sha256_hmac_starts(m_pHashCtx, m_pTempBuf, TEMPBUF_SIZE, 0); // we simply use
  // the full buffer contents here instead of only 8 bytes; it doesn't cost
  // anything, and the rest of the buffer contains some useful pseudorandom data
  sha256_hmac_update(m_pHashCtx, m_pPool, POOL_SIZE);
  sha256_hmac_finish(m_pHashCtx, m_pCipherKey);

  ClearPoolBuf(m_pTempBuf, TEMPBUF_SIZE);

  // perform the cipher's key setup
  //aes_setkey_enc(m_pCipherCtx, m_pCipherKey, KEY_SIZE*8);
  m_pCipher->SetKey(m_pCipherKey);

  // destroy the key
  ClearPoolBuf(m_pCipherKey, KEY_SIZE);

  // is it necessary to update the pool again to ensure that the pool contents
  // can't be derived from the AES key and vice versa...?
  // The AES key has been derived from the pool AND a time stamp, which should
  // provide sufficient protection against key recovery from the pool
  // ... so the answer is "no" (for the moment)
//  UpdatePool();

  // get new counter/IV, encrypt it, and set up cipher with it
  GetNewCounterOrIV(m_pSecCtr);
  m_pCipher->ProcessCounterOrIV(m_pSecCtr);

  m_lGetBufPos = GETBUF_SIZE; // get buffer is yet to be filled
  m_lNumOfBlocks = 0;
  m_blKeySet = true;
}
//---------------------------------------------------------------------------
void RandomPool::GetNewCounterOrIV(word8* pCounter)
{
  // get a new counter with timer values
  HighResTimer(pCounter);
  GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(pCounter+8));
  //m_pCipher->EncryptIv(pCounter);
  //aes_crypt_ecb(m_pCipherCtx, AES_ENCRYPT, pCounter, pCounter);
}
//---------------------------------------------------------------------------
void RandomPool::GeneratorGate(void)
{
  // let the cipher re-key itself
  m_pCipher->SelfKey(m_pCipherKey, m_pSecCtr);

  // set key and destroy it
  //m_pCipher->SetKey(m_pGetBuf);
  //aes_setkey_enc(m_pCipherCtx, m_pCipherKey, KEY_SIZE*8);

  ClearPoolBuf(m_pCipherKey, KEY_SIZE);

  // get a new IV/timestamp
  GetNewCounterOrIV(m_pTempBuf);

  // xor the old counter with the current timestamp
  for (int nI = 0; nI < CTR_SIZE; nI++)
    m_pSecCtr[nI] ^= m_pTempBuf[nI];

  ClearPoolBuf(m_pTempBuf, CTR_SIZE);

  // encrypt the xor'ed counter and set a new IV
  m_pCipher->ProcessCounterOrIV(m_pSecCtr);
  //aes_crypt_ecb(m_pCipherCtx, AES_ENCRYPT, m_pSecCtr, m_pSecCtr);

  m_lNumOfBlocks = 0;
}
//---------------------------------------------------------------------------
word32 RandomPool::FillBuf(word8* pBuf,
  word32 lNumOfBytes)
{
  bool blGetBuf = pBuf == nullptr;

  if (blGetBuf) {
    pBuf = m_pGetBuf;
    lNumOfBytes = GETBUF_SIZE;
  }

  // fill the buffer by encrypting multiple counter values
  const word32 lBlockSize = m_pCipher->GetBlockSize();
  word32 lNumFullBlocks = lNumOfBytes / lBlockSize;

  if (lNumFullBlocks == 0)
    throw Exception("RandomPool: Not filling any random data");

  while (lNumFullBlocks != 0)  {
    word32 lBlocksToFill = std::min(lNumFullBlocks,
      m_pCipher->GetMaxNumOfBlocks() - m_lNumOfBlocks);

    m_pCipher->FillBlocks(pBuf, m_pSecCtr, lBlocksToFill);

    pBuf += lBlocksToFill * lBlockSize;
    lNumFullBlocks -= lBlocksToFill;
    m_lNumOfBlocks += lBlocksToFill;

    if (m_lNumOfBlocks >= m_pCipher->GetMaxNumOfBlocks())
      GeneratorGate();
  }

  if (blGetBuf) {
    GeneratorGate();
    m_lGetBufPos = 0;
  }

  return (lNumOfBytes / lBlockSize) * lBlockSize;
}
//---------------------------------------------------------------------------
void RandomPool::AddData(const void* pBuf,
  word32 lNumOfBytes)
{
  const word8* pSrcBuf = reinterpret_cast<const word8*>(pBuf);

  while (lNumOfBytes-- != 0) {
    if (m_lAddBufPos == ADDBUF_SIZE)
      UpdatePool();

    m_pAddBuf[m_lAddBufPos++] ^= *pSrcBuf++;
  }
}
//---------------------------------------------------------------------------
void RandomPool::GetData(void* pBuf,
  word32 lNumOfBytes)
{
  if (!m_blKeySet)
    SetKey();

  word8* pDestBuf = reinterpret_cast<word8*>(pBuf);
  bool blGetBufFilled = false;

  while (lNumOfBytes != 0) {
    if (m_lGetBufPos == GETBUF_SIZE) {
      if (lNumOfBytes >= GETBUF_SIZE) {
        // fill the buffer directly without using memcpy()
        word32 lBytesGen = FillBuf(pDestBuf, lNumOfBytes);

        // already finished?
        if (lBytesGen == lNumOfBytes) {
          // perform generator gate before leaving
          GeneratorGate();

          // destroy old random data and say goodbye
          if (blGetBufFilled)
            ClearPoolBuf(m_pGetBuf, GETBUF_SIZE);

          return;
        }

        pDestBuf += lBytesGen;
        lNumOfBytes -= lBytesGen;
      }

      // refill get buffer
      FillBuf();
    }

    word32 lToCopy = std::min(lNumOfBytes, GETBUF_SIZE - m_lGetBufPos);
    memcpy(pDestBuf, m_pGetBuf + m_lGetBufPos, lToCopy);

    pDestBuf += lToCopy;
    m_lGetBufPos += lToCopy;
    lNumOfBytes -= lToCopy;

    blGetBufFilled = true;
  }
}
//---------------------------------------------------------------------------
word8 RandomPool::GetByte(void)
{
  if (!m_blKeySet)
    SetKey();

  if (m_lGetBufPos == GETBUF_SIZE)
    FillBuf();

  return m_pGetBuf[m_lGetBufPos++];
}
//---------------------------------------------------------------------------
void RandomPool::Randomize(void)
{
  // the following code is based on Random.cpp from Sami Tolvanen's "Eraser"
  // and rndw32.c from "libgcrypt"
#define addEntropy(source, size)  AddData(source, size)
#define addEntropyValue(value) \
                lEntropyValue = word32(value); \
                addEntropy(&lEntropyValue, sizeof(lEntropyValue))

  static bool blFixedItemsAdded = false;
  word32 lEntropyValue;

  word64 qTimer;
  HighResTimer(&qTimer);
  addEntropy(&qTimer, sizeof(word64));

  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  addEntropy(&ft, sizeof(FILETIME));

  __int64 qFreeSpace;
  qFreeSpace = DiskFree(0); // 0 = current drive
  if (qFreeSpace != -1)
    addEntropy(&qFreeSpace, sizeof(__int64));

  POINT pt;
  GetCaretPos(&pt);
  addEntropy(&pt, sizeof(POINT));
  GetCursorPos(&pt);
  addEntropy(&pt, sizeof(POINT));

  MEMORYSTATUS ms;
  ms.dwLength = sizeof(MEMORYSTATUS);
  GlobalMemoryStatus(&ms);
  addEntropy(&ms, sizeof(MEMORYSTATUS));

  addEntropyValue(GetActiveWindow());
  addEntropyValue(GetCapture());
  addEntropyValue(GetClipboardOwner());
  addEntropyValue(GetClipboardViewer());
  addEntropyValue(GetCurrentProcess());
  addEntropyValue(GetCurrentProcessId());
  addEntropyValue(GetCurrentThread());
  addEntropyValue(GetCurrentThreadId());
  addEntropyValue(GetDesktopWindow());
  addEntropyValue(GetFocus());
  addEntropyValue(GetForegroundWindow());
  addEntropyValue(GetInputState());
  addEntropyValue(GetMessagePos());
  addEntropyValue(GetMessageTime());
  addEntropyValue(GetOpenClipboardWindow());
  addEntropyValue(GetProcessHeap());
  addEntropyValue(GetProcessWindowStation());
  addEntropyValue(GetQueueStatus(QS_ALLEVENTS));
  addEntropyValue(GetTickCount());

  // these exist on NT
  FILETIME ftCreationTime;
  FILETIME ftExitTime;
  FILETIME ftKernelTime;
  FILETIME ftUserTime;
  ULARGE_INTEGER uSpace;
  if (GetThreadTimes(GetCurrentThread(), &ftCreationTime, &ftExitTime,
      &ftKernelTime, &ftUserTime)) {
    addEntropy(&ftCreationTime, sizeof(FILETIME));
    addEntropy(&ftExitTime,     sizeof(FILETIME));
    addEntropy(&ftKernelTime,   sizeof(FILETIME));
    addEntropy(&ftUserTime,     sizeof(FILETIME));
  }
  if (GetProcessTimes(GetCurrentProcess(), &ftCreationTime, &ftExitTime,
      &ftKernelTime, &ftUserTime)) {
    addEntropy(&ftCreationTime, sizeof(FILETIME));
    addEntropy(&ftExitTime,     sizeof(FILETIME));
    addEntropy(&ftKernelTime,   sizeof(FILETIME));
    addEntropy(&ftUserTime,     sizeof(FILETIME));
  }

#ifndef _WIN64
  if (GetProcessWorkingSetSize(GetCurrentProcess(), &uSpace.LowPart,
	  &uSpace.HighPart)) {
	addEntropy(&uSpace, sizeof(ULARGE_INTEGER));
  }
#endif

  if (!blFixedItemsAdded) {
    STARTUPINFO startupInfo;
    TIME_ZONE_INFORMATION tzi;
    SYSTEM_INFO systemInfo;
    //OSVERSIONINFO versionInfo;

    addEntropyValue(GetUserDefaultLangID());
    addEntropyValue(GetUserDefaultLCID());

    // desktop geometry and colours
    addEntropyValue(GetSystemMetrics(SM_CXSCREEN));
    addEntropyValue(GetSystemMetrics(SM_CYSCREEN));
    addEntropyValue(GetSystemMetrics(SM_CXHSCROLL));
    addEntropyValue(GetSystemMetrics(SM_CYHSCROLL));
    addEntropyValue(GetSystemMetrics(SM_CXMAXIMIZED));
    addEntropyValue(GetSystemMetrics(SM_CYMAXIMIZED));
    addEntropyValue(GetSysColor(COLOR_3DFACE));
    addEntropyValue(GetSysColor(COLOR_DESKTOP));
    addEntropyValue(GetSysColor(COLOR_INFOBK));
    addEntropyValue(GetSysColor(COLOR_WINDOW));
    addEntropyValue(GetDialogBaseUnits());

    // System information
    if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
      addEntropy(&tzi, sizeof(TIME_ZONE_INFORMATION));
    }
    addEntropyValue(GetSystemDefaultLangID());
    addEntropyValue(GetSystemDefaultLCID());
    addEntropyValue(GetOEMCP());
    addEntropyValue(GetACP());
    addEntropyValue(GetKeyboardLayout(0));
    addEntropyValue(GetKeyboardType(0));
    addEntropyValue(GetKeyboardType(1));
    addEntropyValue(GetKeyboardType(2));
    addEntropyValue(GetDoubleClickTime());
    addEntropyValue(GetCaretBlinkTime());
    addEntropyValue(GetLogicalDrives());

    // GetVersionEx is deprecated
    //versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    //if (GetVersionEx(&versionInfo)) {
    //  addEntropy(&versionInfo, sizeof(OSVERSIONINFO));
    //}

    GetSystemInfo(&systemInfo);
    addEntropy(&systemInfo, sizeof(SYSTEM_INFO));

    // Process startup info
    startupInfo.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&startupInfo);
    addEntropy(&startupInfo, sizeof(STARTUPINFO));

    memzero(&startupInfo, sizeof(STARTUPINFO));
    memzero(&tzi,         sizeof(TIME_ZONE_INFORMATION));
    memzero(&systemInfo,  sizeof(SYSTEM_INFO));
    //memzero(&versionInfo, sizeof(OSVERSIONINFO));

    blFixedItemsAdded = true;
  }

  // get some random data from the Windows API...
  SecureMem<word8> mswinRand(POOL_SIZE);
  if (m_blCryptProv &&
    CryptGenRandom(m_cryptProv, POOL_SIZE, mswinRand))
    addEntropy(mswinRand, POOL_SIZE);

  // finally, add Windows' QPC high-resolution counter
  // (not necessarily identical to the RDTSC value)
  LARGE_INTEGER qpc;
  if (QueryPerformanceCounter(&qpc))
    addEntropy(&qpc, sizeof(LARGE_INTEGER));

  // the notorious cleanup ...
  memzero(&qTimer,         sizeof(word64));
  memzero(&ft,             sizeof(FILETIME));
  memzero(&qFreeSpace,     sizeof(word64));
  memzero(&pt,             sizeof(POINT));
  memzero(&ms,             sizeof(MEMORYSTATUS));
  memzero(&ftCreationTime, sizeof(FILETIME));
  memzero(&ftExitTime,     sizeof(FILETIME));
  memzero(&ftKernelTime,   sizeof(FILETIME));
  memzero(&ftUserTime,     sizeof(FILETIME));
  memzero(&uSpace,         sizeof(ULARGE_INTEGER));
  memzero(&qpc,            sizeof(LARGE_INTEGER));
  memzero(&lEntropyValue,  sizeof(word32));
}
//---------------------------------------------------------------------------
bool RandomPool::WriteSeedFile(const WString& sFileName)
{
  try {
    std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmCreate));

    // initialize the PRNG
    SetKey();

    // get enough entropy and flush the pool afterwards
    SecureMem<word8> buf(2 * POOL_SIZE);
    GetData(buf, buf.Size());
    Flush();

    if (pFile->Write(buf, buf.Size()) != buf.Size())
      return false;
  }
  catch (...) {
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
bool RandomPool::ReadSeedFile(const WString& sFileName)
{
  int nBytesRead = 0;

  try {
    std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmOpenRead));

    SecureMem<word8> buf(2 * POOL_SIZE);
    nBytesRead = pFile->Read(buf, buf.Size());

    AddData(buf, nBytesRead);
  }
  catch (...) {
    return false;
  }

  // reading at least POOL_SIZE bytes should be enough
  return nBytesRead >= POOL_SIZE;
}
//---------------------------------------------------------------------------
