// RandomPool.cpp
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
#include <vcl.h>
#include <stdio.h>
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <bcrypt.h>
#include <ntstatus.h>
#pragma hdrstop

#include "RandomPool.h"
#include "CryptUtil.h"
#include "hrtimer.h"
#include "FastPRNG.h"
#include "aes.h"
#include "chacha.h"
#ifdef _WIN64
#include "../crypto/blake2/blake2.h"
#else
#include "../crypto/blake2/ref/blake2.h"
#endif
//---------------------------------------------------------------------------
#pragma package(smart_init)

#ifdef _WIN64
#pragma link "bcrypt.a"
#else
#pragma link "bcrypt.lib"
#endif

// sizeof(aes_context) = 280 bytes
// sizeof(chacha_ctx) = 68 bytes
// Speck128 round keys = 272 bytes
// sizeof(sha256_context) = 236 bytes

const int
POOLPAGE_SIZE    = 4096, // size of the entire pool page in RAM
KEY_SIZE         = 32, // AES key size used
CTR_SIZE         = 16, // AES block size
ADDBUF_SIZE      = 512, // size of add buffer
GETBUF_SIZE      = 64, // size of get buffer
TEMPBUF_SIZE     = CTR_SIZE, // size of temporary buffer
POOL_OFFSET      = 0,
HASHCTX_OFFSET   = POOL_OFFSET + RandomPool::POOL_SIZE,
ADDBUF_OFFSET    = HASHCTX_OFFSET + sizeof(blake2s_state),
CIPHERKEY_OFFSET = ADDBUF_OFFSET + ADDBUF_SIZE,
SECCTR_OFFSET    = CIPHERKEY_OFFSET + KEY_SIZE,
CIPHERCTX_OFFSET = SECCTR_OFFSET + CTR_SIZE,
GETBUF_OFFSET    = CIPHERCTX_OFFSET + sizeof(aes_context),
TEMPBUF_OFFSET   = GETBUF_OFFSET + GETBUF_SIZE,
POOL_DATA_SIZE   = TEMPBUF_OFFSET + TEMPBUF_SIZE,
UNUSED_SIZE_MAX  = POOLPAGE_SIZE - POOL_DATA_SIZE,
UNUSED_SIZE_MIN  = 64;

const char
ERROR_BLAKE2_INIT[]  = "BLAKE2 initialization failed",
ERROR_BLAKE2_FINAL[] = "BLAKE2 finalization failed";

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

  void SetKey(const word8 pKey[32]) override
  {
    aes_setkey_enc(m_pCtx, pKey, 256);
  }

  void SelfKey(word8 pKey[32], word8* pCounter) override
  {
    FillBlocks(pKey, pCounter, 2);
    SetKey(pKey);
  }

  void ProcessCounterOrIV(word8 pCounter[16]) override
  {
    aes_crypt_ecb(m_pCtx, AES_ENCRYPT, pCounter, pCounter);
  }

  void FillBlocks(word8* pBuf, word8* pCounter, word32 lNumOfBlocks) override
  {
    while (lNumOfBlocks--) {
      aes_crypt_ecb(m_pCtx, AES_ENCRYPT, pCounter, pBuf);
      incrementCounter<128>(pCounter);
      pBuf += 16;
    }
  }

  word32 GetBlockSize(void) const override
  {
    return 16;
  }

  word64 GetMaxNumOfBlocks(void) const override
  {
    return 65536;
  }

  void Move(void* pNew) override
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

  void SetKey(const word8 pKey[32]) override
  {
    // set ChaCha key, IV can be arbitrary for now,
    // because we're using the cipher in only one direction
    chacha_keysetup(m_pCtx, pKey, 256);
    chacha_nrounds(m_pCtx, m_nRounds);
    //word8 iv[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //chacha_ivsetup(m_pCtx, iv);
  }

  void SelfKey(word8 pKey[32], word8* pCounter) override
  {
    chacha_keystream_bytes(m_pCtx, pKey, 32);
    SetKey(pKey);
  }

  void ProcessCounterOrIV(word8 pIV[16]) override
  {
    chacha_encrypt_bytes(m_pCtx, pIV, pIV, 16);
    chacha_ivsetup(m_pCtx, pIV, pIV + 8);
  }

  void FillBlocks(word8* pBuf, word8* pCounter, word32 lNumOfBlocks) override
  {
    chacha_keystream_bytes(m_pCtx, pBuf, lNumOfBlocks * 64);
  }

  word32 GetBlockSize(void) const override
  {
    return 64;
  }

  word64 GetMaxNumOfBlocks(void) const override
  {
    return 0x400000000ull; // generate max. 1TB of data before changing key
  }

  void Move(void* pNew) override
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

  #define ER64(x,y,k) (x=(_rotr64(x,8)+y)^k, y=_rotl64(y,3)^x)

  void SetKey(const word8 pKey[32])
  {
    const word64* K = reinterpret_cast<const word64*>(pKey);
    word64 i, D = K[3], C = K[2], B = K[1], A = K[0];
	  for (i = 0; i < 33; ) {
		  m_pRoundKeys[i] = A; ER64(B, A, i++);
		  m_pRoundKeys[i] = A; ER64(C, A, i++);
		  m_pRoundKeys[i] = A; ER64(D, A, i++);
    }
	  m_pRoundKeys[i] = A;
  }

  void Encrypt(word64 Pt[2], word64 Ct[2]) const
  {
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

  word64 GetMaxNumOfBlocks(void) const
  {
    return 65536ull;
  }

  void Move(void* pNew)
  {
    m_pRoundKeys = reinterpret_cast<word64*>(pNew);
  }

};
#endif
}

using namespace RandPoolCipher;

//RandomPool* g_pRandPool = nullptr;

//---------------------------------------------------------------------------
RandomPool::RandomPool(CipherType cipher,
  std::unique_ptr<RandomGenerator> pFastRandGen,
  bool blLockPhysMem)
  : m_lAddBufPos(0), m_lGetBufPos(0), m_lTouchPoolIndex(0), m_blKeySet(false),
    m_blRandomizeFixedItemsAdded(false),
    m_pFastRandGen(std::move(pFastRandGen)), m_blLockPhysMem(blLockPhysMem)
{
  m_pPoolPage = AllocPoolPage();
  if (m_pPoolPage == nullptr)
    OutOfMemoryError();

  // hide the pool contents *somewhere* within the memory page
  m_lUnusedSize = (m_blLockPhysMem && m_pFastRandGen) ?
    m_pFastRandGen->GetNumRange(UNUSED_SIZE_MIN, UNUSED_SIZE_MAX + 1) : 0;
  SetPoolPointers();

  SetCipher(cipher);
}
//---------------------------------------------------------------------------
RandomPool::RandomPool(RandomPool& src, std::unique_ptr<RandomGenerator> pFastRandGen)
  : RandomPool(src.m_cipherType, std::move(pFastRandGen), false)
{
  SecureMem<word8> entropy(POOL_SIZE);
  src.GetData(entropy, POOL_SIZE);
  AddData(entropy, POOL_SIZE);
  src.Flush();
}
//---------------------------------------------------------------------------
RandomPool::~RandomPool()
{
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
RandomPool& RandomPool::GetInstance(void)
{
  static RandomPool inst(CipherType::ChaCha20,
    std::make_unique<Jsf32RandGen>(), true);
  return inst;
}
//---------------------------------------------------------------------------
word8* RandomPool::AllocPoolPage(void)
{
  word8* pPoolPage = nullptr;
  if (m_blLockPhysMem) {
    pPoolPage = reinterpret_cast<word8*>(VirtualAlloc(nullptr, POOLPAGE_SIZE,
      MEM_COMMIT, PAGE_READWRITE));
    if (pPoolPage != nullptr) {
      VirtualLock(pPoolPage, POOLPAGE_SIZE);
      ClearPoolBuf(pPoolPage, POOLPAGE_SIZE);
    }
  }
  else {
    pPoolPage = new word8[POOL_DATA_SIZE];
    ClearPoolBuf(pPoolPage, POOL_DATA_SIZE);
  }
  return pPoolPage;
}
//---------------------------------------------------------------------------
void RandomPool::FreePoolPage(void)
{
  if (m_pPoolPage != nullptr) {
    ClearPoolBuf(m_pPoolPage, m_blLockPhysMem ? POOLPAGE_SIZE : POOL_DATA_SIZE);
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
  word8* pOffset = m_pPoolPage + m_lUnusedSize;
  m_pPool      = pOffset + POOL_OFFSET;
  m_pHashCtx   = pOffset + HASHCTX_OFFSET;
  m_pAddBuf    = pOffset + ADDBUF_OFFSET;
  m_pCipherKey = pOffset + CIPHERKEY_OFFSET;
  m_pSecCtr    = pOffset + SECCTR_OFFSET;
  m_pCipherCtx = pOffset + CIPHERCTX_OFFSET;
  m_pGetBuf    = pOffset + GETBUF_OFFSET;
  m_pTempBuf   = pOffset + TEMPBUF_OFFSET;
}
//---------------------------------------------------------------------------
void RandomPool::TouchPool(void)
{
  if (m_blLockPhysMem) {
    if (m_lTouchPoolIndex >= m_lUnusedSize)
      m_lTouchPoolIndex = 0;
    m_pPoolPage[m_lTouchPoolIndex++]--;
  }
}
//---------------------------------------------------------------------------
bool RandomPool::MovePool(void)
{
  word8* pNewPoolPage = AllocPoolPage();
  if (pNewPoolPage == nullptr)
    return false;

  // only copy the relevant portion of the pool page
  if (m_blLockPhysMem)
    memcpy(pNewPoolPage + m_lUnusedSize, m_pPoolPage + m_lUnusedSize,
      POOLPAGE_SIZE - m_lUnusedSize);
  else
    memcpy(pNewPoolPage, m_pPoolPage, POOL_DATA_SIZE);

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
void RandomPool::SetCipher(CipherType cipher)
{
  if (!m_pCipher || cipher != m_cipherType) {
    switch (cipher) {
    case CipherType::AES_CTR:
      m_pCipher.reset(new AES_CTR(reinterpret_cast<aes_context*>(m_pCipherCtx)));
      break;
    case CipherType::ChaCha20:
    case CipherType::ChaCha8:
      m_pCipher.reset(new ChaCha(reinterpret_cast<chacha_ctx*>(m_pCipherCtx),
        cipher == CipherType::ChaCha20 ? 20 : 8));
      break;
#if 0
    case CipherType::Speck128:
      m_pCipher.reset(new Speck128(m_pCipherCtx));
      break;
#endif
    default:
      throw RandomGeneratorError("RandomPool: Cipher not supported");
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
  //sha256_init(m_pHashCtx);
  //sha256_hmac_starts(m_pHashCtx, m_pPool, POOL_SIZE, 0);
  blake2s_state* pHashCtx = reinterpret_cast<blake2s_state*>(m_pHashCtx);
  if (blake2s_init_key(pHashCtx, POOL_SIZE, m_pPool, POOL_SIZE) != 0)
    throw RandomGeneratorError(ERROR_BLAKE2_INIT);

  if (m_lAddBufPos != 0) {
    //sha256_hmac_update(m_pHashCtx, m_pAddBuf, m_lAddBufPos);
    blake2s_update(pHashCtx, m_pAddBuf, m_lAddBufPos);
    ClearPoolBuf(m_pAddBuf, m_lAddBufPos);
    m_lAddBufPos = 0;
  }

  if (m_blKeySet) {
    // the counter has accumulated some entropy, which we should incorporate
    // into the pool now
    //sha256_hmac_update(m_pHashCtx, m_pSecCtr, CTR_SIZE);
    blake2s_update(pHashCtx, m_pSecCtr, CTR_SIZE);
  }

  // ALWAYS add some entropy (independent of the add buffer)
  HighResTimer(m_pTempBuf);
  //sha256_hmac_update(m_pHashCtx, m_pTempBuf, sizeof(word64));
  blake2s_update(pHashCtx, m_pTempBuf, sizeof(word64));

  // get new pool
  //sha256_hmac_finish(m_pHashCtx, m_pPool);
  if (blake2s_final(pHashCtx, m_pPool, POOL_SIZE) != 0)
    throw RandomGeneratorError(ERROR_BLAKE2_FINAL);

  // clear sensitive data
  ClearPoolBuf(m_pHashCtx, sizeof(blake2s_state));
  ClearPoolBuf(m_pTempBuf, sizeof(word64));

  if (m_blKeySet) {
    // destroy sensitive data used for generating random numbers
    ClearPoolBuf(m_pSecCtr, CTR_SIZE);
    ClearPoolBuf(m_pCipherCtx, sizeof(aes_context));
    ClearPoolBuf(m_pGetBuf, GETBUF_SIZE);
    m_lGetBufPos = 0;
    m_qNumOfBlocks = 0;
    m_blKeySet = false;
  }
}
//---------------------------------------------------------------------------
void RandomPool::SetKey(void)
{
  // always update the pool before generating a new key
  UpdatePool();

  // don't use the pool directly as a key, but derive a new key from the pool
  // using a time stamp as a "key" for the hash function
  HighResTimer(m_pTempBuf);
  //sha256_init(m_pHashCtx);
  //sha256_hmac_starts(m_pHashCtx, m_pTempBuf, TEMPBUF_SIZE, 0); // we simply use
  blake2s_state* pHashCtx = reinterpret_cast<blake2s_state*>(m_pHashCtx);

  // use the full buffer contents here (16 bytes vs. 8 bytes from the timer);
  // it doesn't cost anything, and the rest of the buffer contains some
  // useful pseudorandom data
  if (blake2s_init_key(pHashCtx, POOL_SIZE, m_pTempBuf, TEMPBUF_SIZE) != 0)
    throw RandomGeneratorError(ERROR_BLAKE2_INIT);

  //sha256_hmac_update(m_pHashCtx, m_pPool, POOL_SIZE);
  //sha256_hmac_finish(m_pHashCtx, m_pCipherKey);
  blake2s_update(pHashCtx, m_pPool, POOL_SIZE);
  if (blake2s_final(pHashCtx, m_pCipherKey, POOL_SIZE) != 0)
    throw RandomGeneratorError(ERROR_BLAKE2_FINAL);

  ClearPoolBuf(m_pHashCtx, sizeof(blake2s_state));
  ClearPoolBuf(m_pTempBuf, TEMPBUF_SIZE);

  // perform the cipher's key setup
  m_pCipher->SetKey(m_pCipherKey);

  // destroy the key
  ClearPoolBuf(m_pCipherKey, KEY_SIZE);

  // is it necessary to update the pool again to ensure that the pool contents
  // can't be derived from the key and vice versa...?
  // The cipher key has been derived from the pool AND a time stamp, which should
  // provide sufficient protection against key recovery from the pool
  // ... so the answer is "no" (for the moment)
//  UpdatePool();

  // get new counter/IV, encrypt it, and set up cipher with it
  GetNewCounterOrIV(m_pSecCtr);
  m_pCipher->ProcessCounterOrIV(m_pSecCtr);

  m_lGetBufPos = GETBUF_SIZE; // get buffer is yet to be filled
  m_qNumOfBlocks = 0;
  m_blKeySet = true;
}
//---------------------------------------------------------------------------
void RandomPool::GetNewCounterOrIV(word8* pCounter)
{
  // get a new counter with timer values
  HighResTimer(pCounter);
  GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(pCounter+8));
}
//---------------------------------------------------------------------------
void RandomPool::GeneratorGate(void)
{
  // let the cipher re-key itself
  m_pCipher->SelfKey(m_pCipherKey, m_pSecCtr);

  ClearPoolBuf(m_pCipherKey, KEY_SIZE);

  // get a new IV/timestamp
  GetNewCounterOrIV(m_pTempBuf);

  // xor the old counter with the current timestamp
  for (int nI = 0; nI < CTR_SIZE; nI++)
    m_pSecCtr[nI] ^= m_pTempBuf[nI];

  ClearPoolBuf(m_pTempBuf, CTR_SIZE);

  // encrypt the xor'ed counter and set a new IV
  m_pCipher->ProcessCounterOrIV(m_pSecCtr);

  m_qNumOfBlocks = 0;
}
//---------------------------------------------------------------------------
word32 RandomPool::FillBuf(word8* pBuf,
  word32 lNumOfBytes)
{
  // fill the buffer by encrypting multiple counter values
  const word32 lNumFullBlocks = lNumOfBytes / m_pCipher->GetBlockSize();

  if (lNumFullBlocks == 0)
    throw RandomGeneratorError("RandomPool::FillBuf(): Number of blocks is zero");

  for (word32 lRemainingBlocks = lNumFullBlocks; lRemainingBlocks != 0; )  {
    word32 lBlocksToFill = std::min<word64>(lRemainingBlocks,
      m_pCipher->GetMaxNumOfBlocks() - m_qNumOfBlocks);

    m_pCipher->FillBlocks(pBuf, m_pSecCtr, lBlocksToFill);

    pBuf += lBlocksToFill * m_pCipher->GetBlockSize();
    lRemainingBlocks -= lBlocksToFill;
    m_qNumOfBlocks += lBlocksToFill;

    if (m_qNumOfBlocks >= m_pCipher->GetMaxNumOfBlocks())
      GeneratorGate();
  }

  return lNumFullBlocks * m_pCipher->GetBlockSize();
}
//---------------------------------------------------------------------------
void RandomPool::FillGetBuf(void)
{
  FillBuf(m_pGetBuf, GETBUF_SIZE);
  GeneratorGate();
  m_lGetBufPos = 0;
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
      FillGetBuf();
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
    FillGetBuf();

  return m_pGetBuf[m_lGetBufPos++];
}
//---------------------------------------------------------------------------
void RandomPool::Randomize(void)
{
  // the following code is based on Random.cpp from Sami Tolvanen's "Eraser"
  // and rndw32.c from "libgcrypt"
  word64 qTimer;
  HighResTimer(&qTimer);
  AddData(qTimer);

  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  AddData(ft);

  //__int64 freeSpace;
  // return value of -1 in case of failure is acceptable here
  AddData(DiskFree(0)); // 0 = current drive

  POINT pt;
  GetCaretPos(&pt);
  AddData(pt);
  GetCursorPos(&pt);
  AddData(pt);

  MEMORYSTATUS ms;
  ms.dwLength = sizeof(MEMORYSTATUS);
  GlobalMemoryStatus(&ms);
  AddData(ms);

  AddData(GetActiveWindow());
  AddData(GetCapture());
  AddData(GetClipboardOwner());
  AddData(GetClipboardViewer());
  AddData(GetCurrentProcess());
  AddData(GetCurrentProcessId());
  AddData(GetCurrentThread());
  AddData(GetCurrentThreadId());
  AddData(GetDesktopWindow());
  AddData(GetFocus());
  AddData(GetForegroundWindow());
  AddData(GetInputState());
  AddData(GetMessagePos());
  AddData(GetMessageTime());
  AddData(GetOpenClipboardWindow());
  AddData(GetProcessHeap());
  AddData(GetProcessWindowStation());
  AddData(GetQueueStatus(QS_ALLEVENTS));
  AddData(GetTickCount());

  // these exist on NT
  FILETIME ftCreationTime;
  FILETIME ftExitTime;
  FILETIME ftKernelTime;
  FILETIME ftUserTime;
  if (GetThreadTimes(GetCurrentThread(), &ftCreationTime, &ftExitTime,
      &ftKernelTime, &ftUserTime)) {
    AddData(ftCreationTime);
    AddData(ftExitTime);
    AddData(ftKernelTime);
    AddData(ftUserTime);
  }
  if (GetProcessTimes(GetCurrentProcess(), &ftCreationTime, &ftExitTime,
      &ftKernelTime, &ftUserTime)) {
    AddData(ftCreationTime);
    AddData(ftExitTime);
    AddData(ftKernelTime);
    AddData(ftUserTime);
  }

  SIZE_T minWorkSetSize, maxWorkSetSize;
  if (GetProcessWorkingSetSize(GetCurrentProcess(), &minWorkSetSize,
	  &maxWorkSetSize)) {
	AddData(minWorkSetSize);
    AddData(maxWorkSetSize);
  }

  if (!m_blRandomizeFixedItemsAdded) {
    STARTUPINFO startupInfo;
    TIME_ZONE_INFORMATION tzi;
    SYSTEM_INFO systemInfo;
    //OSVERSIONINFO versionInfo;

    AddData(GetUserDefaultLangID());
    AddData(GetUserDefaultLCID());

    // desktop geometry and colours
    AddData(GetSystemMetrics(SM_CXSCREEN));
    AddData(GetSystemMetrics(SM_CYSCREEN));
    AddData(GetSystemMetrics(SM_CXHSCROLL));
    AddData(GetSystemMetrics(SM_CYHSCROLL));
    AddData(GetSystemMetrics(SM_CXMAXIMIZED));
    AddData(GetSystemMetrics(SM_CYMAXIMIZED));
    AddData(GetSysColor(COLOR_3DFACE));
    AddData(GetSysColor(COLOR_DESKTOP));
    AddData(GetSysColor(COLOR_INFOBK));
    AddData(GetSysColor(COLOR_WINDOW));
    AddData(GetDialogBaseUnits());

    // System information
    if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
      AddData(tzi);
    }
    AddData(GetSystemDefaultLangID());
    AddData(GetSystemDefaultLCID());
    AddData(GetOEMCP());
    AddData(GetACP());
    AddData(GetKeyboardLayout(0));
    AddData(GetKeyboardType(0));
    AddData(GetKeyboardType(1));
    AddData(GetKeyboardType(2));
    AddData(GetDoubleClickTime());
    AddData(GetCaretBlinkTime());
    AddData(GetLogicalDrives());

    // GetVersionEx is deprecated
    //versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    //if (GetVersionEx(&versionInfo)) {
    //  AddData(&versionInfo, sizeof(OSVERSIONINFO));
    //}

    GetSystemInfo(&systemInfo);
    AddData(systemInfo);

    // Process startup info
    startupInfo.cb = sizeof(STARTUPINFO);
    GetStartupInfo(&startupInfo);
    AddData(startupInfo);

    memzero(&startupInfo, sizeof(STARTUPINFO));
    memzero(&tzi,         sizeof(TIME_ZONE_INFORMATION));
    memzero(&systemInfo,  sizeof(SYSTEM_INFO));
    //memzero(&versionInfo, sizeof(OSVERSIONINFO));

    m_blRandomizeFixedItemsAdded = true;
  }

  // get some random data from the Windows API...
  SecureMem<word8> osRand(POOL_SIZE);
  if (BCryptGenRandom(nullptr, osRand, POOL_SIZE,
      BCRYPT_USE_SYSTEM_PREFERRED_RNG) == STATUS_SUCCESS)
    AddData(osRand, POOL_SIZE);

  // finally, add Windows' QPC high-resolution counter
  // (not necessarily identical to the RDTSC value)
  LARGE_INTEGER qpc;
  if (QueryPerformanceCounter(&qpc))
    AddData(qpc);

  // the notorious cleanup ...
  memzero(&qTimer, sizeof(word64));
  memzero(&ft, sizeof(FILETIME));
  //memzero(&qFreeSpace,     sizeof(word64));
  memzero(&pt, sizeof(POINT));
  memzero(&ms, sizeof(MEMORYSTATUS));
  memzero(&ftCreationTime, sizeof(FILETIME));
  memzero(&ftExitTime, sizeof(FILETIME));
  memzero(&ftKernelTime, sizeof(FILETIME));
  memzero(&ftUserTime, sizeof(FILETIME));
  memzero(&minWorkSetSize, sizeof(size_t));
  memzero(&maxWorkSetSize, sizeof(size_t));
  memzero(&qpc, sizeof(LARGE_INTEGER));
  //memzero(&lEntropyValue,  sizeof(word32));
  //memzero(&qEntropyValue,  sizeof(word64));
}
//---------------------------------------------------------------------------
bool RandomPool::WriteSeedFile(const WString& sFileName)
{
  try {
    auto pFile = std::make_unique<TFileStream>(sFileName, fmCreate);

    // initialize the PRNG
    SetKey();

    // get enough entropy and flush the pool afterwards
    SecureMem<word8> buf(2 * POOL_SIZE);
    GetData(buf, buf.Size());
    Flush();

  if (pFile->Write(buf, static_cast<int>(buf.Size())) != buf.Size())
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
    auto pFile = std::make_unique<TFileStream>(sFileName, fmOpenRead);

    SecureMem<word8> buf(2 * POOL_SIZE);
    nBytesRead = pFile->Read(buf, static_cast<int>(buf.Size()));

    AddData(buf, nBytesRead);
  }
  catch (...) {
    return false;
  }

  // reading at least POOL_SIZE bytes should be enough
  return nBytesRead >= POOL_SIZE;
}
//---------------------------------------------------------------------------
