// PasswDatabase.cpp
//
// PASSWORD TECH
// Copyright (c) 2002-2024 by Christian Thoeing <c.thoeing@web.de>
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
#include <vector>
#include <array>
#include <StrUtils.hpp>
#pragma hdrstop

#include "PasswDatabase.h"
#include "FastPRNG.h"
#include "RandomPool.h"
#include "sha1.h"
#include "CryptUtil.h"
#include "Main.h"
#include "StringFileStreamW.h"
#include "Language.h"
#include "sha256.h"
#include "sha512.h"
#include "Util.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#define TEST_DECRYPTION

static const word8
  PASSW_DB_MAGIC[4] = { 'P', 'W', 'd', 'b' };

static const word32
  FH_FLAG_RECOVERY_KEY            = 1, // file header flags

  FLAG_DEFAULT_USER_NAME          = 1, // database header flags
  FLAG_PASSW_FORMAT_SEQ           = 2,
  FLAG_PASSW_EXPIRY_DAYS          = 4,
  FLAG_DEFAULT_PASSW_HISTORY_SIZE = 8,

  MAX_FILE_SIZE = 104857600,
  DEFAULT_BUF_SIZE = 65536;

static const char
  PARAMSTR_DEFAULT_USER_NAME[] = "DefUserName",
  PARAMSTR_PASSW_FORMAT_SEQ[] = "PWFormatSeq";

#pragma pack(1)
struct FileHeader {
  word8 Magic[4];
  word8 HeaderSize;
  word16 Version;
  word32 Flags;
  word8 CipherType;
  word8 HashType;
  word8 KdfType;
  word32 KdfIterations;
};

struct PasswDbHeader {
  word8 Magic[4];
  word16 HeaderSize;
  word32 Flags;
  word8 NumOfVariableParam;
  word8 NumOfFields;
  word32 NumOfEntries;
  word8 CompressionAlgo;
  word8 CompressionLevel;
  word32 UncompressedSize;
  word32 CompressedSize;
};

struct PasswHistoryHeader {
  word32 BlockSize;
  word8 Flags;
  word8 HistorySize;
  word8 MaxHistorySize;
};
#pragma pack()

using namespace EncryptionAlgorithm;

inline void CheckKeyEmpty(const SecureMem<word8>& key)
{
  if (key.IsEmpty())
    throw EPasswDbError("Specified \"key\" parameter is empty");
}

//---------------------------------------------------------------------------
const char* PasswDbEntry::GetFieldName(FieldType type)
{
  static const char* fieldNames[NUM_FIELDS] =
  { "Title", "UserName", "Password", "URL", "Keyword", "Notes", "KeyValueList",
    "Tags", "CreationTime", "ModificationTime", "PasswChangeTime",
    "PasswExpiryDate", "PasswHistory"
  };
  return fieldNames[type];
}
//---------------------------------------------------------------------------
SecureWString PasswDbEntry::TimeStampToString(const FILETIME& ft)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);

  SecureWString sBuf(256);

  int nDateLen = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &st,
      nullptr, sBuf, sBuf.Size(), nullptr);

  if (nDateLen == 0)
    return SecureWString();

  sBuf[nDateLen - 1] = ' ';

  int nTimeLen = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &st, nullptr,
      &sBuf[nDateLen], sBuf.Size() - nDateLen);

  if (nTimeLen == 0)
    return SecureWString();

  sBuf.Resize(nDateLen + nTimeLen);

  return sBuf;
}
//---------------------------------------------------------------------------
SecureWString PasswDbEntry::ExpiryDateToString(word32 lDate)
{
  int nYear, nMonth, nDay;
  if (lDate == 0 || !DecodeExpiryDate(lDate, nYear, nMonth, nDay))
    return SecureWString();

  SYSTEMTIME st;
  memzero(&st, sizeof(st));

  st.wYear = nYear;
  st.wMonth = nMonth;
  st.wDay = nDay;

  SecureWString sBuf(256);

  int nDateLen = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &st,
    nullptr, sBuf, sBuf.Size(), nullptr);

  if (nDateLen == 0)
    return SecureWString();

  sBuf.Resize(nDateLen);

  return sBuf;
}
//---------------------------------------------------------------------------
const SecureWString* PasswDbEntry::GetKeyValue(const wchar_t* pwszKey) const
{
  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
    if (_wcsicmp(it->first.c_str(), pwszKey) == 0) {
      return &it->second;
    }
  }

  return nullptr;
}
//---------------------------------------------------------------------------
void PasswDbEntry::SetKeyValue(const wchar_t* pwszKey, const wchar_t* pwszValue)
{
  if (*pwszKey == '\0' || *pwszValue == '\0')
    return;

  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
    if (_wcsicmp(it->first.c_str(), pwszKey) == 0) {
      it->second.AssignStr(pwszValue);
      return;
    }
  }

  m_keyValueList.push_back(std::make_pair(SecureWString(pwszKey,
    wcslen(pwszKey) + 1), SecureWString(pwszValue, wcslen(pwszValue) + 1)));
}
//---------------------------------------------------------------------------
SecureWString PasswDbEntry::GetKeyValueListAsString(wchar_t sep) const
{
  if (m_keyValueList.empty())
    return SecureWString();

  /*word32 lSize = 0;
  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
    lSize += it->first.StrLen() + it->second.StrLen() + 2;
  }

  SecureWString sDest(lSize);

  word32 lPos = 0;
  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
     wcscpy(&sDest[lPos], it->first.c_str());
     lPos += it->first.StrLen();
     sDest[lPos++] = '=';
     wcscpy(&sDest[lPos], it->second.c_str());
     lPos += it->second.StrLen();
     sDest[lPos++] = sep;
  }

  sDest[lPos - 1] = '\0';
  */

  SecureWString sDest(128);
  word32 lPos = 0;
  for (const auto& kv : m_keyValueList) {
    if (lPos != 0)
      sDest.StrCat(&sep, 1, lPos);
    sDest.StrCat(kv.first, lPos);
    sDest.StrCat(L"=", 1, lPos);
    sDest.StrCat(kv.second, lPos);
  }

  sDest.Shrink(lPos + 1);

  return sDest;
}
//---------------------------------------------------------------------------
void PasswDbEntry::ParseKeyValueList(const SecureWString& sList)
{
  // min. size due to min. key-value pair "X=Y\0"
  if (sList.Size() < 4)
    return;

  auto items = SplitStringBuf(sList, L"\n");
  for (const auto& keyValPair : items) {
    word32 lLen = keyValPair.StrLen();
    word32 lPos = keyValPair.Find('=');
    if (lPos != keyValPair.npos && lPos > 0 && lPos < lLen - 1) {
      SecureWString sKey(&keyValPair[0], lPos + 1),
        sVal(&keyValPair[lPos+1], lLen - lPos);
      sKey.back() = '\0';
      sVal.back() = '\0';
      m_keyValueList.push_back(std::make_pair(sKey, sVal));
    }
  }

  /*const wchar_t* p = sList.c_str();
  while (*p != '\0') {
    const wchar_t* pEq = wcschr(p, '=');
    if (pEq == nullptr || pEq == p)
      break;
    word32 lKeyLen = static_cast<word32>(pEq - p);
    SecureWString sKey(p, lKeyLen + 1);
    sKey[lKeyLen] = '\0';
    p = pEq + 1;
    if (*p == '\0' || *p == '\n')
      break;
    const wchar_t* pSep = wcschr(p, '\n');
    if (pSep == nullptr) {
      pSep = wcschr(p, '\0');
//      if (pSep == p)
//        break;
    }
    word32 lValueLen = static_cast<word32>(pSep - p);
    SecureWString sValue(p, lValueLen + 1);
    sValue[lValueLen] = '\0';
    m_keyValueList.push_back(std::make_pair(sKey, sValue));
    if (*pSep == '\0')
      break;
    p = pSep + 1;
  }*/
}
//---------------------------------------------------------------------------
bool PasswDbEntry::CheckTag(const SecureWString& sTag) const
{
  return m_tags.count(sTag) != 0;
}
//---------------------------------------------------------------------------
bool PasswDbEntry::AddTag(const SecureWString& sTag)
{
  auto ret = m_tags.insert(sTag);
  return ret.second;
}
//---------------------------------------------------------------------------
SecureWString PasswDbEntry::GetTagsAsString(wchar_t sep) const
{
  if (m_tags.empty())
    return SecureWString();

  /*word32 lSize = 0;
  for (const auto& s : m_tags)
    lSize += s.StrLen() + 1;

  SecureWString sDest(lSize);
  word32 lPos = 0;
  for (const auto& s : m_tags) {
    wcscpy(&sDest[lPos], s.c_str());
    lPos += s.StrLen();
    sDest[lPos++] = sep;
  }*/

  SecureWString sDest(128);
  word32 lPos = 0;
  for (const auto& tag : m_tags) {
    if (lPos != 0)
      sDest.StrCat(sep, lPos);
    sDest.StrCat(tag, lPos);
  }

  sDest.Shrink(lPos + 1);

  return sDest;
}
//---------------------------------------------------------------------------
void PasswDbEntry::ParseTagList(const SecureWString& sList)
{
  // min. size due to min. tag "x\0"
  if (sList.Size() < 2)
    return;

  auto items = SplitStringBuf(sList.c_str(), L"\n");
  for (auto& sTag : items) {
    m_tags.insert(std::move(sTag));
  }

  /*const wchar_t* p = sList.c_str();
  while (*p != '\0') {
    const wchar_t* pSep = wcschr(p, '\n');
    if (pSep == p)
      break;
    if (pSep == nullptr) {
      pSep = wcschr(p, '\0');
//      if (pSep == p)
//        break;
    }
    word32 lTagLen = static_cast<word32>(pSep - p);
    SecureWString sTag(p, lTagLen + 1);
    sTag[lTagLen] = '\0';
    m_tags.insert(sTag);
    if (*pSep == '\0')
      break;
    p = pSep + 1;
  }*/
}
//---------------------------------------------------------------------------
void PasswDbEntry::PasswHistory::AddEntry(const PasswHistoryEntry& entry,
  bool blToFront)
{
  if (m_blActive && !entry.second.IsStrEmpty()) {
    m_history.insert(blToFront ? m_history.begin() : m_history.end(), entry);
    if (m_history.size() > m_lMaxSize)
      m_history.erase(m_history.begin() + m_lMaxSize, m_history.end());
  }
}
//---------------------------------------------------------------------------


PasswDatabase::PasswDatabase()
  : m_pSecMem(nullptr), m_blPlaintextPassw(false),
    m_lDbEntryId(0), m_lCryptBufPos(0), m_nLastVersion(0),
    m_dbOpenState(DbOpenState::Closed),
    m_bCipherType(CIPHER_AES256), m_lKdfIterations(KEY_HASH_ITERATIONS),
    m_lDefaultPasswExpiryDays(0), m_lDefaultMaxPasswHistorySize(0),
    m_blRecoveryKey(false), m_blCompressed(false),
    m_nCompressionLevel(0)
{
}
//---------------------------------------------------------------------------
PasswDatabase::~PasswDatabase()
{
  Close();
}
//---------------------------------------------------------------------------
void PasswDatabase::Close(void)
{
  if (m_pSecMem != nullptr) {
    g_fastRandGen.GetData(m_pSecMem, SECMEM_SIZE);
    VirtualUnlock(m_pSecMem, SECMEM_SIZE);
    VirtualFree(m_pSecMem, 0, MEM_RELEASE);
    m_pSecMem = nullptr;
  }

  m_bCipherType = CIPHER_AES256;
  m_lKdfIterations = KEY_HASH_ITERATIONS;
  m_lDefaultPasswExpiryDays = 0;
  m_cryptBuf.Clear();

  for (auto pEntry : m_db)
	  delete pEntry;

  m_db.clear();
  m_pFile.reset();
  m_lDbEntryId = 0;
  m_lCryptBufPos = 0;
  m_sDefaultUserName.Clear();
  m_dbOpenState = DbOpenState::Closed;
  m_blRecoveryKey = false;
  m_blCompressed = false;
  m_nCompressionLevel = 0;
}
//---------------------------------------------------------------------------
void PasswDatabase::Initialize(const SecureMem<word8>& key)
{
  // cryptographic data stored in RAM
  // - ChaCha20 context (68 bytes) for encrypting passwords
  // - salt (16 bytes) for calculating SHA-1 hashes of passwords
  // - database master key (32 bytes)
  // - database salt (32 bytes)
  // - database recovery key block (128 bytes), if recovery key is set

  m_pSecMem = reinterpret_cast<word8*>(
    VirtualAlloc(nullptr, SECMEM_SIZE, MEM_COMMIT, PAGE_READWRITE));
  if (m_pSecMem == nullptr)
    OutOfMemoryError();

  VirtualLock(m_pSecMem, SECMEM_SIZE);
  g_fastRandGen.GetData(m_pSecMem, SECMEM_SIZE);

  word8* pMemOffset = m_pSecMem + fprng_rand(2048);
  m_pMemCipherCtx = reinterpret_cast<chacha_ctx*>(pMemOffset);
  pMemOffset += sizeof(chacha_ctx);

  SecureMem<word8> memKey(SECMEM_KEY_LENGTH);
  RandomPool& randPool = RandomPool::GetInstance();
  randPool.GetData(memKey, SECMEM_KEY_LENGTH);

  //aes_setkey_enc(m_pMemCipherCtx, memKey, SECMEM_KEY_LENGTH*8);
  chacha_keysetup(m_pMemCipherCtx, memKey, SECMEM_KEY_LENGTH*8);

  m_pMemSalt = pMemOffset;
  pMemOffset += SECMEM_SALT_LENGTH;
  randPool.GetData(m_pMemSalt, SECMEM_SALT_LENGTH);

  m_pDbKey = pMemOffset;
  pMemOffset += DB_KEY_LENGTH;
  m_pDbSalt = pMemOffset;
  pMemOffset += DB_SALT_LENGTH;

  randPool.GetData(m_pDbSalt, DB_SALT_LENGTH);

  if (m_blRecoveryKey)
    memcpy(m_pDbKey, key, key.Size());
  else
    pbkdf2_256bit(key, key.Size(), m_pDbSalt, DB_SALT_LENGTH, m_pDbKey, m_lKdfIterations);

  m_pDbRecoveryKeyBlock = pMemOffset;
  pMemOffset += DB_RECOVERY_KEY_BLOCK_LENGTH;

  randPool.Flush();
}
//---------------------------------------------------------------------------
void PasswDatabase::CheckDbNotOpen(void)
{
  switch (m_dbOpenState) {
  case DbOpenState::Incomplete:
    Close();
    break;
  case DbOpenState::Open:
    throw EPasswDbError("Database already opened");
    break;
  default:
    break; // just to please the compiler
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::CheckDbOpen(void) const
{
  if (!IsOpen())
    throw EPasswDbError("Database not open");
}
//---------------------------------------------------------------------------
void PasswDatabase::New(const SecureMem<word8>& key)
{
  CheckDbNotOpen();
  CheckKeyEmpty(key);
  Initialize(key);
  m_nLastVersion = VERSION;
  m_dbOpenState = DbOpenState::Open;
}
//---------------------------------------------------------------------------
std::unique_ptr<EncryptionAlgorithm::SymmetricCipher>
  PasswDatabase::CreateCipher(int nType, const word8* pKey,
    EncryptionAlgorithm::Mode mode)
{
  switch (nType) {
  case CIPHER_AES256:
    return std::make_unique<AES_CBC>(pKey, DB_KEY_LENGTH, mode);
  case CIPHER_CHACHA20:
    return std::make_unique<ChaCha20>(pKey, DB_KEY_LENGTH);
  default:
    throw EPasswDbError("Cipher not supported");
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::Open(const SecureMem<word8>& key,
  const WString& sFileName)
{
  CheckDbNotOpen();
  CheckKeyEmpty(key);

  m_dbOpenState = DbOpenState::Incomplete;

  auto pFile = std::make_unique<TFileStream>(sFileName, fmOpenRead | fmShareDenyWrite);
  if (pFile->Size > MAX_FILE_SIZE) // max. 100MB
    throw EPasswDbError("File too large");

  // check file size
  if (pFile->Size < sizeof(FileHeader) + DB_SALT_LENGTH + 8 + 16 + 32)
    throw EPasswDbError("Invalid file size");

  // read file header (plaintext)
  FileHeader fh;
  pFile->Read(&fh, sizeof(fh));

  if (memcmp(fh.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC)) != 0)
    throw EPasswDbInvalidFormat("Unknown file format");

  if (fh.HeaderSize < sizeof(fh))
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT);

  m_nLastVersion = fh.Version;

  if (fh.Version < 0x100)
    throw EPasswDbInvalidFormat("Invalid version number");

  if (fh.CipherType > CIPHER_CHACHA20)
    throw EPasswDbInvalidFormat("Encryption algorithm not supported");

  if (fh.HashType > HASH_SHA512)
    throw EPasswDbInvalidFormat("Hash algorithm not supported");

  if (fh.KdfType > KDF_PBKDF2_SHA256)
    throw EPasswDbInvalidFormat("Key derivation function not supported");

  if (fh.KdfIterations == 0)
    throw EPasswDbInvalidFormat("Invalid number of KDF iterations");

  m_bCipherType = fh.CipherType;
  m_lKdfIterations = fh.KdfIterations;
  m_blRecoveryKey = fh.Version >= 0x103 && fh.Flags & FH_FLAG_RECOVERY_KEY;

  word32 lFileSize = pFile->Size - fh.HeaderSize;
  m_cryptBuf.New(std::max(1024u, lFileSize));

  SecureMem<word8> masterKey;
  word32 lBufPos, lCryptParamLen, lHmacLen;
  PasswDbHeader header;

  for (int nKeyNum = 0; nKeyNum < 2; nKeyNum++) {
    // read entire file contents without header
    pFile->Seek(fh.HeaderSize, soFromBeginning);
    pFile->Read(m_cryptBuf, lFileSize);

    SecureMem<word8> derivedKey(DB_KEY_LENGTH);

    if (m_blRecoveryKey) {
      lBufPos = (DB_SALT_LENGTH + DB_KEY_LENGTH) * nKeyNum;
      pbkdf2_256bit(key, key.Size(),
        &m_cryptBuf[lBufPos], DB_SALT_LENGTH, derivedKey, fh.KdfIterations);

      // decrypt master key
      auto cipher = CreateCipher(fh.CipherType, derivedKey,
        EncryptionAlgorithm::Mode::DECRYPT);
      cipher->SetIV(&m_cryptBuf[lBufPos]);
      masterKey.New(DB_KEY_LENGTH);
      cipher->Decrypt(
        &m_cryptBuf[lBufPos + DB_SALT_LENGTH], masterKey, DB_KEY_LENGTH);

      lBufPos = DB_RECOVERY_KEY_BLOCK_LENGTH;
    }
    else {
      pbkdf2_256bit(key, key.Size(), m_cryptBuf, DB_SALT_LENGTH, derivedKey,
        fh.KdfIterations);
      lBufPos = DB_SALT_LENGTH;
    }

    // do key setup
    const auto& keySrc = m_blRecoveryKey ? masterKey : derivedKey;
    auto cipher = CreateCipher(fh.CipherType, keySrc,
      EncryptionAlgorithm::Mode::DECRYPT);

    cipher->SetIV(&m_cryptBuf[lBufPos]);
    lBufPos += cipher->GetIVSize();
    lCryptParamLen = lBufPos;

    word32 lHmacLen = fh.HashType == HASH_SHA256 ? SHA256_HMAC_LENGTH :
      SHA512_HMAC_LENGTH;
    SecureMem<word8> hmac(lHmacLen);
    if (fh.Version >= 0x101)
      memcpy(hmac, &m_cryptBuf[lFileSize - lHmacLen], lHmacLen);

    // decrypt first N blocks
    //word32 lBlockLen = cipher->GetBlockSize();
    word32 lAlignedHeaderSize =
      alignToBlockSize(sizeof(header), cipher->GetBlockSize());

    cipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos], lAlignedHeaderSize);

    memcpy(&header, &m_cryptBuf[lBufPos], sizeof(header));

    if (memcmp(header.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC)) != 0) {
      if (m_blRecoveryKey && nKeyNum == 0)
        continue;
      else
        throw EPasswDbInvalidKey(TRL("Database not encrypted, or invalid key"));
    }

    lBufPos += lAlignedHeaderSize;

    // decrypt rest
    if (fh.Version >= 0x101) {
      int nRest = lFileSize - lBufPos - lHmacLen;
      if (nRest > 0)
        cipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos], nRest);
    }
    else {
      if (lFileSize > lBufPos)
        cipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos],
          lFileSize - lBufPos);
      memcpy(hmac, &m_cryptBuf[lFileSize - lHmacLen], lHmacLen);
    }

    // calculate and check HMAC
    SecureMem<word8> checkHmac(lHmacLen);
    checkHmac.Zeroize();
    if (fh.HashType == HASH_SHA256) {
      lHmacLen = SHA256_HMAC_LENGTH;
      SecureMem<sha256_context> hashCtx(1);
      sha256_init(hashCtx);
      sha256_hmac_starts(hashCtx, keySrc, DB_KEY_LENGTH, 0);
      if (lFileSize > lCryptParamLen + SHA256_HMAC_LENGTH)
        sha256_hmac_update(hashCtx,
          &m_cryptBuf[lCryptParamLen],
          lFileSize - lCryptParamLen - lHmacLen);
      sha256_hmac_finish(hashCtx, checkHmac);
    }
    else {
      lHmacLen = SHA512_HMAC_LENGTH;
      SecureMem<sha512_context> hashCtx(1);
      sha512_init(hashCtx);
      sha512_hmac_starts(hashCtx, keySrc, DB_KEY_LENGTH, 0);
      if (lFileSize > lCryptParamLen + SHA512_HMAC_LENGTH)
        sha512_hmac_update(hashCtx,
          &m_cryptBuf[lCryptParamLen],
          lFileSize - lCryptParamLen - lHmacLen);
      sha512_hmac_finish(hashCtx, checkHmac);
    }

    if (checkHmac != hmac) {
      if (m_blRecoveryKey && nKeyNum == 0)
        continue;
      else
        throw EPasswDbInvalidKey(TRL("File contents modified, or invalid key"));
    }

    break;
  }

  // data stream begins after inner header
  m_lCryptBufPos = lCryptParamLen + header.HeaderSize;

  // initialize crypto engine
  Initialize(m_blRecoveryKey ? masterKey : key);

  if (m_blRecoveryKey)
    memcpy(m_pDbRecoveryKeyBlock, m_cryptBuf, DB_RECOVERY_KEY_BLOCK_LENGTH);

  if (fh.Version >= 0x104 && header.CompressionAlgo != 0) {
    if (header.CompressionAlgo > COMPRESSION_DEFLATE)
      throw EPasswDbError("Compression algorithm not supported");

    SecureMem<word8> dataBuf(header.UncompressedSize);
    Inflate decompr;
    word32 lDataSize;
    bool blFinished = decompr.Process(
      m_cryptBuf + m_lCryptBufPos,
      header.CompressedSize,
      dataBuf,
      dataBuf.Size(),
      true,
      lDataSize);

    if (!blFinished || lDataSize != header.UncompressedSize)
      throw EPasswDbError("Error while decompressing data");

    m_cryptBuf.Swap(dataBuf);
    m_lCryptBufPos = 0;
    m_blCompressed = true;
    m_nCompressionLevel = header.CompressionLevel;
  }
  else {
    m_blCompressed = false;
    m_nCompressionLevel = 0;
  }

  // read global database settings
  if (fh.Version >= 0x102) {
    for (int i = 0; i < header.NumOfVariableParam; i++) {
      word32 lFlag = ReadType<word32>();
      if ((header.Flags & FLAG_DEFAULT_USER_NAME) &&
          lFlag == FLAG_DEFAULT_USER_NAME) {
        m_sDefaultUserName = ReadString();
      }
      else if ((header.Flags & FLAG_PASSW_FORMAT_SEQ) &&
               lFlag == FLAG_PASSW_FORMAT_SEQ) {
        m_sPasswFormatSeq = ReadString();
      }
      else if ((header.Flags & FLAG_PASSW_EXPIRY_DAYS) &&
               lFlag == FLAG_PASSW_EXPIRY_DAYS) {
        m_lDefaultPasswExpiryDays = std::min(3650u, ReadType<word32>());
      }
      else if ((header.Flags & FLAG_DEFAULT_PASSW_HISTORY_SIZE) &&
               lFlag == FLAG_DEFAULT_PASSW_HISTORY_SIZE) {
        m_lDefaultMaxPasswHistorySize = ReadType<word8>();
      }
      else
        SkipField();
    }
  }
  else {
    for (int i = 0; i < header.NumOfVariableParam; i++) {
      SecureAnsiString sParamName = ReadAnsiString();
      if ((header.Flags & FLAG_DEFAULT_USER_NAME) &&
          stricmp(sParamName, PARAMSTR_DEFAULT_USER_NAME) == 0) {
        m_sDefaultUserName = ReadString();
      }
      else if ((header.Flags & FLAG_PASSW_FORMAT_SEQ) &&
               stricmp(sParamName, PARAMSTR_PASSW_FORMAT_SEQ) == 0) {
        m_sPasswFormatSeq = ReadString();
      }
      else
        SkipField();
    }
  }

  // read column titles (AnsiStrings)
  std::vector<int> idxConv(header.NumOfFields);
  std::array<bool, PasswDbEntry::NUM_FIELDS> fieldsUsed;
  std::fill(fieldsUsed.begin(), fieldsUsed.end(), false);
  //memzero(fieldsUsed, sizeof(fieldsUsed));

  for (int nI = 0; nI < header.NumOfFields; nI++) {
    idxConv[nI] = -1;
    SecureAnsiString sStr = ReadAnsiString();
    if (sStr.IsEmpty())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
    for (int nJ = 0; nJ < PasswDbEntry::NUM_FIELDS; nJ++) {
      if (stricmp(sStr, PasswDbEntry::GetFieldName(
            static_cast<PasswDbEntry::FieldType>(nJ))) == 0)
      {
        idxConv[nI] = nJ;
        fieldsUsed[nJ] = true;
        break;
      }
    }
  }

  // we need at least a title, user name, and password
  if (!fieldsUsed[PasswDbEntry::TITLE] || !fieldsUsed[PasswDbEntry::USERNAME]
      || !fieldsUsed[PasswDbEntry::PASSWORD])
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT);

  // now read the fields...
  // max. number is NumOfFields + "end of entry" mark
  //int nMaxNumFields = header.NumOfFields + 1;
  for (int nI = 0; nI < header.NumOfEntries; nI++) {
    PasswDbEntry* pEntry = AddDbEntry();
    for (int nJ = 0; nJ <= header.NumOfFields; nJ++) {
      int nFieldIndex = ReadFieldIndex();
      if (nFieldIndex == PasswDbEntry::END)
        break;
      if (nFieldIndex < idxConv.size() && idxConv[nFieldIndex] >= 0) {
        SecureWString sField;
        int nIdx = idxConv[nFieldIndex];
        switch (nIdx) {
        case PasswDbEntry::KEYVALUELIST:
          sField = ReadString();
          pEntry->ParseKeyValueList(sField);
          pEntry->UpdateKeyValueString();
          break;
        case PasswDbEntry::TAGS:
          sField = ReadString();
          pEntry->ParseTagList(sField);
          pEntry->UpdateTagsString();
          break;
        case PasswDbEntry::CREATIONTIME:
          pEntry->CreationTime = ReadField<FILETIME>();
          pEntry->CreationTimeString =
            pEntry->TimeStampToString(pEntry->CreationTime);
          break;
        case PasswDbEntry::MODIFICATIONTIME:
          pEntry->ModificationTime = ReadField<FILETIME>();
          pEntry->ModificationTimeString =
            pEntry->TimeStampToString(pEntry->ModificationTime);
          break;
        case PasswDbEntry::PASSWCHANGETIME:
          pEntry->PasswChangeTime = ReadField<FILETIME>();
          pEntry->PasswChangeTimeString =
            pEntry->TimeStampToString(pEntry->PasswChangeTime);
          break;
        case PasswDbEntry::PASSWEXPIRYDATE:
          pEntry->PasswExpiryDate = ReadField<word32>();
          pEntry->PasswExpiryDateString =
            pEntry->ExpiryDateToString(pEntry->PasswExpiryDate);
          if (pEntry->PasswExpiryDateString.IsStrEmpty())
            pEntry->PasswExpiryDate = 0;
          break;
        case PasswDbEntry::PASSWHISTORY:
          {
            PasswHistoryHeader pwh = ReadType<PasswHistoryHeader>();
            auto& history = pEntry->GetPasswHistory();
            history.SetActive(pwh.Flags & 1);
            history.SetMaxSize(pwh.MaxHistorySize);
            for (word32 i = 0; i < pwh.HistorySize; i++) {
              FILETIME ft = ReadType<FILETIME>();
              sField = ReadString();
              history.AddEntry({ ft, sField }, false);
            }
          }
          break;
        default:
          sField = ReadString();
          if (nIdx == PasswDbEntry::PASSWORD)
            SetDbEntryPassw(*pEntry, sField);
          else
            pEntry->Strings[nIdx] = sField;
        }
      }
      else
        SkipField();
    }
#ifdef _DEBUG
    if (pEntry->Strings[PasswDbEntry::TITLE].IsEmpty() && pEntry->IsPasswEmpty())
      ShowMessage("Entry with empty title and password detected!");
#endif
  }

  m_cryptBuf.Clear();
  memzero(&header, sizeof(header));
  memzero(&fh, sizeof(fh));

  m_dbOpenState = DbOpenState::Open;
  m_pFile.swap(pFile);
}
//---------------------------------------------------------------------------
void PasswDatabase::Write(const void* pBuf, word32 lNumOfBytes)
{
  if (pBuf == nullptr || lNumOfBytes == 0)
    return;

  const word8* pSrcBuf = reinterpret_cast<const word8*>(pBuf);

  m_cryptBuf.BufferedGrow(m_lCryptBufPos + lNumOfBytes);
  //word32 lNewSize = m_lCryptBufPos + lNumOfBytes;
  //if (lNewSize > m_cryptBuf.Size()) {
  //  m_cryptBuf.GrowBy(alignToBlockSize(
  //    std::max(lNumOfBytes, m_cryptBuf.Size()), DEFAULT_BUF_SIZE));
  //}

  //memcpy(&m_cryptBuf[m_lCryptBufPos], pSrcBuf, lNumOfBytes);
  m_cryptBuf.Copy(m_lCryptBufPos, pSrcBuf, lNumOfBytes);
  m_lCryptBufPos += lNumOfBytes;

  m_cryptBuf.GrowClearMark(m_lCryptBufPos);

  if (m_lCryptBufPos > MAX_FILE_SIZE - 1024)
    throw EPasswDbError("Database size exceeds file size limit");
}
//---------------------------------------------------------------------------
void PasswDatabase::WriteFieldBuf(const void* pBuf, word32 lNumOfBytes, int nIndex)
{
  if (nIndex >= 0) {
    word8 bIndex = nIndex;
    Write(&bIndex, 1);
  }
  Write(&lNumOfBytes, 4);
  Write(pBuf, lNumOfBytes);
}
//---------------------------------------------------------------------------
void PasswDatabase::WriteString(const char* pszStr, word32 lLen, int nIndex)
{
  /*if (nIndex >= 0) {
    if (pszStr != nullptr && *pszStr != '\0')
      WriteFieldBuf(pszStr, strlen(pszStr), nIndex);
  }
  else
    WriteFieldBuf(pszStr, (pszStr != nullptr) ? strlen(pszStr) : 0);*/
  if (lLen == -1)
    lLen = strlen(pszStr);
  if (nIndex < 0 || lLen > 0)
    WriteFieldBuf(pszStr, lLen, nIndex);
}
//---------------------------------------------------------------------------
void PasswDatabase::WriteString(const SecureWString& sStr, int nIndex)
{
  if (!sStr.IsStrEmpty()) {
#ifdef _DEBUG
    sStr.StrLen();
#endif
    SecureAnsiString asUtf8 = WStringToUtf8(sStr);
    WriteString(asUtf8.c_str(), asUtf8.StrLen(), nIndex);
  }
  else if (nIndex < 0)
    WriteFieldBuf(nullptr, 0);
}
//---------------------------------------------------------------------------
void PasswDatabase::SaveToFile(const WString& sFileName)
{
  CheckDbOpen();

  if (m_bCipherType > CIPHER_CHACHA20)
    throw EPasswDbError("Invalid cipher");
  if (m_lKdfIterations == 0)
    throw EPasswDbError("Invalid number of KDF iterations");

  FileHeader fh;
  memcpy(fh.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC));
  fh.HeaderSize = sizeof(FileHeader);
  fh.Version = VERSION;
  fh.Flags = 0;
  if (m_blRecoveryKey)
    fh.Flags |= FH_FLAG_RECOVERY_KEY;
  fh.CipherType = m_bCipherType;
  fh.HashType = HASH_SHA512;
  fh.KdfType = KDF_PBKDF2_SHA256;
  fh.KdfIterations = m_lKdfIterations;

  auto cipher = CreateCipher(m_bCipherType, m_pDbKey,
    EncryptionAlgorithm::Mode::ENCRYPT);

  word32 lIVLen = cipher->GetIVSize();
  SecureMem<word8> iv(lIVLen);
  RandomPool::GetInstance().GetData(iv, lIVLen);
  cipher->SetIV(iv);

  PasswDbHeader header;
  memcpy(header.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC));
  header.HeaderSize = sizeof(header);
  //header.Version = VERSION;
  header.Flags = 0;
  header.NumOfVariableParam = 0;
  header.NumOfFields = PasswDbEntry::NUM_FIELDS;
  header.NumOfEntries = m_db.size();

  if (m_blCompressed) {
    header.CompressionAlgo = COMPRESSION_DEFLATE;
    header.CompressionLevel = m_nCompressionLevel =
      m_nCompressionLevel <= 0 ? MZ_DEFAULT_LEVEL :
        std::min<int>(MZ_BEST_COMPRESSION, m_nCompressionLevel);
  }
  else {
    header.CompressionAlgo = 0;
    header.CompressionLevel = 0;
  }

  if (!m_sDefaultUserName.IsStrEmpty()) {
    header.Flags |= FLAG_DEFAULT_USER_NAME;
    header.NumOfVariableParam++;
  }

  if (!m_sPasswFormatSeq.IsStrEmpty()) {
    header.Flags |= FLAG_PASSW_FORMAT_SEQ;
    header.NumOfVariableParam++;
  }

  if (m_lDefaultPasswExpiryDays != 0) {
    header.Flags |= FLAG_PASSW_EXPIRY_DAYS;
    header.NumOfVariableParam++;
  }

  if (m_lDefaultMaxPasswHistorySize != 0) {
    header.Flags |= FLAG_DEFAULT_PASSW_HISTORY_SIZE;
    header.NumOfVariableParam++;
  }

  m_cryptBuf.New(DEFAULT_BUF_SIZE);
  m_lCryptBufPos = sizeof(header);

  word32 lFlag = FLAG_DEFAULT_USER_NAME;
  if (header.Flags & lFlag) {
    //WriteString(DEFAULT_USER_NAME);
    WriteType(lFlag);
    WriteString(m_sDefaultUserName);
  }

  lFlag = FLAG_PASSW_FORMAT_SEQ;
  if (header.Flags & lFlag) {
    //WriteString(PASSW_FORMAT_SEQ);
    WriteType(lFlag);
    WriteString(m_sPasswFormatSeq);
  }

  lFlag = FLAG_PASSW_EXPIRY_DAYS;
  if (header.Flags & lFlag) {
    WriteType(lFlag);
    WriteType(m_lDefaultPasswExpiryDays);
  }

  lFlag = FLAG_DEFAULT_PASSW_HISTORY_SIZE;
  if (header.Flags & lFlag) {
    WriteType(lFlag);
    word8 bVal = m_lDefaultMaxPasswHistorySize;
    WriteType(bVal);
  }

  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    WriteString(PasswDbEntry::GetFieldName(
      static_cast<PasswDbEntry::FieldType>(nI)));
  }

  const word8 bEndOfEntry = PasswDbEntry::END;

  for (auto pEntry : m_db)
  {
	for (int nI = 0; nI < PasswDbEntry::NUM_STRING_FIELDS; nI++) {
      //SecureWString sField;
      switch (nI) {
      case PasswDbEntry::PASSWORD:
        WriteString(GetDbEntryPassw(*pEntry), nI);
        break;
      case PasswDbEntry::KEYVALUELIST:
        WriteString(pEntry->GetKeyValueListAsString(), nI);
        break;
      case PasswDbEntry::TAGS:
        WriteString(pEntry->GetTagsAsString(), nI);
        break;
      default:
        WriteString(pEntry->Strings[nI], nI);
      }
    }

    WriteField(pEntry->CreationTime, PasswDbEntry::CREATIONTIME);
    WriteField(pEntry->ModificationTime, PasswDbEntry::MODIFICATIONTIME);
    if (pEntry->PasswChangeTime.dwLowDateTime != 0 ||
        pEntry->PasswChangeTime.dwHighDateTime != 0)
      WriteField(pEntry->PasswChangeTime, PasswDbEntry::PASSWCHANGETIME);

    const auto& passwHistory = pEntry->GetPasswHistory();
    if (!passwHistory.IsEmpty()) {
      PasswHistoryHeader pwh;
      pwh.BlockSize = sizeof(PasswHistoryHeader);
      pwh.Flags = passwHistory.GetActive() ? 1 : 0;
      pwh.HistorySize = std::min<word32>(MAX_PASSW_HISTORY_SIZE,
        passwHistory.GetSize());
      pwh.MaxHistorySize = std::min<word32>(MAX_PASSW_HISTORY_SIZE,
        passwHistory.GetMaxSize());
      const auto endIt = passwHistory.begin() + pwh.HistorySize;
      for (auto it = passwHistory.begin(); it != endIt; it++) {
        pwh.BlockSize += sizeof(it->first) + 4 + it->second.StrLen();
      }
      const word8 bIndex = PasswDbEntry::PASSWHISTORY;
      WriteType(bIndex);
      WriteType(pwh);
      for (auto it = passwHistory.begin(); it != endIt; it++) {
        WriteType(it->first);
        WriteString(it->second);
      }
    }

    if (pEntry->PasswExpiryDate != 0)
      WriteField(pEntry->PasswExpiryDate, PasswDbEntry::PASSWEXPIRYDATE);

    Write(&bEndOfEntry, 1);
  }

  header.UncompressedSize = header.CompressedSize = m_lCryptBufPos - sizeof(header);

  if (m_blCompressed) {
    word32 lToCompress = header.UncompressedSize;
    SecureMem<word8> workBuf(DEFAULT_BUF_SIZE),
      comprBuf(alignToBlockSize(std::max(DEFAULT_BUF_SIZE, lToCompress), 16));

    Deflate compr(header.CompressionLevel);
    bool blFinished;
    word32 lComprBufPos = sizeof(header);

    do {
      word32 lChunkSize;
      blFinished = compr.Process(
        m_cryptBuf + sizeof(header),
        lToCompress,
        workBuf,
        workBuf.Size(),
        true,
        lChunkSize);
      if (lChunkSize) {
        comprBuf.BufferedGrow(lComprBufPos + lChunkSize);
        //if (lComprBufPos + lChunkSize > comprBuf.Size())
        //  comprBuf.GrowBy(comprBuf.Size());
        //memcpy(comprBuf + lComprBufPos, workBuf, lChunkSize);
        comprBuf.Copy(lComprBufPos, workBuf, lChunkSize);
        lComprBufPos += lChunkSize;
      }
      lToCompress = 0;
    } while (!blFinished);

    m_cryptBuf.Swap(comprBuf);
    m_lCryptBufPos = lComprBufPos;
    header.CompressedSize = lComprBufPos;
  }

  memcpy(m_cryptBuf, &header, sizeof(header));
  memzero(&header, sizeof(header));

  word32 lAlignedSize = m_lCryptBufPos;
  if (cipher->AlignToBlockSize()) {
    lAlignedSize = alignToBlockSize(lAlignedSize, cipher->GetBlockSize());
    if (lAlignedSize > m_lCryptBufPos) {
      m_cryptBuf.Grow(lAlignedSize);
      RandomPool::GetInstance().GetData(m_cryptBuf + m_lCryptBufPos,
        lAlignedSize - m_lCryptBufPos);
    }
  }

  SecureMem<sha512_context> hashCtx(1);
  sha512_init(hashCtx);
  sha512_hmac_starts(hashCtx, m_pDbKey, DB_KEY_LENGTH, 0);
  sha512_hmac_update(hashCtx, m_cryptBuf, lAlignedSize);

  cipher->Encrypt(m_cryptBuf, m_cryptBuf, lAlignedSize);

  // buffer contents are encrypted now, so there's no need to zeroize it anymore
  m_cryptBuf.SetClearMark(0);

  // now open file and write data
  m_pFile.reset();
  m_pFile.reset(new TFileStream(sFileName, fmCreate | fmShareDenyWrite));

  try {
    // file header
    m_pFile->Write(&fh, sizeof(fh));

    // recovery key block or salt
    if (m_blRecoveryKey)
      m_pFile->Write(m_pDbRecoveryKeyBlock, DB_RECOVERY_KEY_BLOCK_LENGTH);
    else
      m_pFile->Write(m_pDbSalt, DB_SALT_LENGTH);

    // initialization vector
    m_pFile->Write(iv, lIVLen);

    // encrypted database contents
    m_pFile->Write(m_cryptBuf, lAlignedSize);

  #if defined(_DEBUG) && defined(TEST_DECRYPTION)
    {
      auto checkCipher = CreateCipher(m_bCipherType, m_pDbKey,
        EncryptionAlgorithm::Mode::DECRYPT);
      checkCipher->SetIV(iv);
      SecureMem<word8> block(checkCipher->GetBlockSize());
      checkCipher->Decrypt(m_cryptBuf, block, block.Size());
      if (memcmp(block, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC)) != 0)
        throw EPasswDbError("Decryption failed!");
    }
  #endif

    SecureMem<word8> hmac(SHA512_HMAC_LENGTH);
    sha512_hmac_finish(hashCtx, hmac);

    m_pFile->Write(hmac, hmac.Size());
  }
  __finally {
    m_cryptBuf.Clear();
    m_pFile.reset();
  }

  m_pFile.reset(new TFileStream(sFileName, fmOpenRead | fmShareDenyWrite));

  m_nLastVersion = VERSION;
}
//---------------------------------------------------------------------------
word32 PasswDatabase::ReadFieldSize(void)
{
  if (m_lCryptBufPos + 4 > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
  word32 lSize;
  memcpy(&lSize, &m_cryptBuf[m_lCryptBufPos], 4);
  m_lCryptBufPos += 4;
  return lSize;
}
//---------------------------------------------------------------------------
int PasswDatabase::ReadFieldIndex(void)
{
  if (m_lCryptBufPos + 1 > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
  return m_cryptBuf[m_lCryptBufPos++];
}
//---------------------------------------------------------------------------
SecureAnsiString PasswDatabase::ReadAnsiString(void)
{
  SecureAnsiString asDest;
  word32 lSize = ReadFieldSize();
  if (lSize != 0) {
    if (m_lCryptBufPos + lSize > m_cryptBuf.Size())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
    //asDest.New(lSize + 1);
    //memcpy(asDest, &m_cryptBuf[m_lCryptBufPos], lSize);
    asDest.AssignStr(reinterpret_cast<char*>(&m_cryptBuf[m_lCryptBufPos]), lSize);
    //asDest[lSize] = '\0';
    m_lCryptBufPos += lSize;
  }
  return asDest;
}
//---------------------------------------------------------------------------
SecureWString PasswDatabase::ReadString(void)
{
  SecureAnsiString asUtf8 = ReadAnsiString();
  return Utf8ToWString(asUtf8);
}
//---------------------------------------------------------------------------
void PasswDatabase::SkipField(void)
{
  m_lCryptBufPos += ReadFieldSize();
  if (m_lCryptBufPos > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
}
//---------------------------------------------------------------------------
PasswDbEntry* PasswDatabase::AddDbEntry(void)
{
  PasswDbEntry* pEntry = new PasswDbEntry(m_lDbEntryId++, m_db.size(),
    false, 1, false);
  m_db.push_back(pEntry);
  return pEntry;
}
//---------------------------------------------------------------------------
PasswDbEntry* PasswDatabase::NewDbEntry(void)
{
  PasswDbEntry* pEntry = new PasswDbEntry(m_lDbEntryId++, m_db.size(),
    true, m_lDefaultMaxPasswHistorySize,
    m_lDefaultMaxPasswHistorySize > 0);
  m_db.push_back(pEntry);
  pEntry->Strings[PasswDbEntry::USERNAME] = m_sDefaultUserName;
  return pEntry;
}
//---------------------------------------------------------------------------
PasswDbEntry* PasswDatabase::DuplicateDbEntry(const PasswDbEntry& original,
  const SecureWString& sTitle)
{
  PasswDbEntry* pDuplicate = new PasswDbEntry(m_lDbEntryId++, m_db.size(),
    true, 1, false);
  m_db.push_back(pDuplicate);

  pDuplicate->Strings[PasswDbEntry::TITLE] = sTitle;
  pDuplicate->Strings[PasswDbEntry::USERNAME] =
    original.Strings[PasswDbEntry::USERNAME];
  pDuplicate->Strings[PasswDbEntry::URL] =
    original.Strings[PasswDbEntry::URL];
  pDuplicate->Strings[PasswDbEntry::KEYWORD] =
    original.Strings[PasswDbEntry::KEYWORD];
  pDuplicate->Strings[PasswDbEntry::NOTES] =
    original.Strings[PasswDbEntry::NOTES];
  SetDbEntryPassw(*pDuplicate, GetDbEntryPassw(original));

  pDuplicate->SetKeyValueList(original.GetKeyValueList());
  pDuplicate->SetTagList(original.GetTagList());
  pDuplicate->GetPasswHistory() = original.GetPasswHistory();
  pDuplicate->PasswExpiryDate = original.PasswExpiryDate;
  pDuplicate->PasswExpiryDateString = original.PasswExpiryDateString;
  pDuplicate->PasswChangeTime = original.PasswChangeTime;

  return pDuplicate;
}
//---------------------------------------------------------------------------
void PasswDatabase::DeleteDbEntry(PasswDbEntry& entry)
{
  word32 lIndex = entry.m_lIndex;
  if (lIndex < m_db.size()) {
	PasswDbList::iterator it = m_db.begin() + lIndex;
    delete &entry;
    it = m_db.erase(it);

    for (; it != m_db.end(); it++)
      (*it)->m_lIndex = lIndex++;
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::MoveDbEntry(word32 lCurrPos, word32 lNewPos)
{
  word32 lSize = m_db.size();
  if (lCurrPos != lNewPos && lCurrPos < lSize && lNewPos < lSize) {
    PasswDbList::iterator first, dest, last;
    if (lCurrPos < lNewPos) {
      first = m_db.begin() + lCurrPos;
      dest = first + 1;
      last = m_db.begin() + lNewPos + 1;
    }
    else {
      first = m_db.begin() + lNewPos;
      dest = m_db.begin() + lCurrPos;
      last = dest + 1;
    }
    std::rotate(first, dest, last);

    word32 lIndex = 0;
    for (auto pEntry : m_db)
      pEntry->m_lIndex = lIndex++;

    /*
    PasswDbList moved(lSize);
    moved[lNewPos] = m_db[lCurrPos];
    moved[lNewPos]->m_lIndex = lNewPos;
    if (lNewPos < lCurrPos) {
      for (word32 lPos = 0; lPos < lSize; lPos++) {
        if (lPos != lNewPos) {
          moved[lPos] = m_db[(lPos < lNewPos || lPos > lCurrPos) ? lPos : lPos - 1];
          moved[lPos]->m_lIndex = lPos;
        }
      }
    }
    else {
      for (word32 lPos = 0; lPos < lSize; lPos++) {
        if (lPos != lNewPos) {
          moved[lPos] = m_db[(lPos < lCurrPos || lPos > lNewPos) ? lPos : lPos + 1];
          moved[lPos]->m_lIndex = lPos;
        }
      }
    }
    m_db = moved;
    */
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::SetDbEntryPassw(PasswDbEntry& entry,
  const SecureWString& sPassw)
{
  if (sPassw.IsStrEmpty()) {
    entry.m_encPassw.Clear();
    entry.Strings[PasswDbEntry::PASSWORD].Clear();
    entry.m_passwHash.Zeroize();
    return;
  }

  sha1_hmac(m_pMemSalt, SECMEM_SALT_LENGTH, sPassw.Bytes(), sPassw.SizeBytes(),
    entry.m_passwHash);

  SecureMem<word8> iv(SECMEM_IV_LENGTH);
  iv.Zeroize();
  *(reinterpret_cast<word32*>(iv.Data())) = entry.m_lId;

  entry.m_encPassw.New(sPassw.Size());

  chacha_ivsetup(m_pMemCipherCtx, iv, nullptr);
  chacha_encrypt_bytes(m_pMemCipherCtx, sPassw.Bytes(), entry.m_encPassw.Bytes(),
    sPassw.SizeBytes());
  //size_t iv_off = 0;
  //aes_crypt_cfb128(m_pMemCipherCtx, AES_ENCRYPT, sPassw.SizeBytes(), &iv_off, iv,
  //  sPassw.Bytes(), pEntry->m_encPassw.Bytes());

  if (m_blPlaintextPassw)
    entry.Strings[PasswDbEntry::PASSWORD] = sPassw;
}
//---------------------------------------------------------------------------
SecureWString PasswDatabase::GetDbEntryPassw(const PasswDbEntry& entry)
{
  if (entry.m_encPassw.IsEmpty())
    return SecureWString();

  if (entry.HasPlaintextPassw())
    return entry.Strings[PasswDbEntry::PASSWORD];

  SecureWString sPassw(entry.m_encPassw.Size());

  SecureMem<word8> iv(SECMEM_IV_LENGTH);
  iv.Zeroize();
  *(reinterpret_cast<word32*>(iv.Data())) = entry.m_lId;

  //size_t iv_off = 0;
  //aes_crypt_cfb128(m_pMemCipherCtx, AES_DECRYPT, pEntry->m_encPassw.SizeBytes(),
  //  &iv_off, iv, pEntry->m_encPassw.Bytes(), sPassw.Bytes());
  chacha_ivsetup(m_pMemCipherCtx, iv, nullptr);
  chacha_encrypt_bytes(m_pMemCipherCtx, entry.m_encPassw.Bytes(), sPassw.Bytes(),
    entry.m_encPassw.SizeBytes());

  SecureMem<word8> checkHash(entry.m_passwHash.Size());
  sha1_hmac(m_pMemSalt, SECMEM_SALT_LENGTH, sPassw.Bytes(), sPassw.SizeBytes(),
    checkHash);

  if (entry.m_passwHash != checkHash)
    throw EPasswDbError("Internal error: Password decryption failed");

  return sPassw;
}
//---------------------------------------------------------------------------
void PasswDatabase::SetPlaintextPassw(bool blPlaintextPassw)
{
  if (blPlaintextPassw == m_blPlaintextPassw)
    return;

  m_blPlaintextPassw = blPlaintextPassw;
  for (auto pEntry : m_db)
  {
    if (blPlaintextPassw)
      pEntry->Strings[PasswDbEntry::PASSWORD] = GetDbEntryPassw(*pEntry);
    else
      pEntry->Strings[PasswDbEntry::PASSWORD].Clear();
  }
}
//---------------------------------------------------------------------------
bool PasswDatabase::CheckMasterKey(const SecureMem<word8>& key)
{
  CheckDbOpen();
  CheckKeyEmpty(key);
  SecureMem<word8> checkKey(DB_KEY_LENGTH);
  pbkdf2_256bit(key, key.Size(),
    m_blRecoveryKey ? m_pDbRecoveryKeyBlock : m_pDbSalt,
    DB_SALT_LENGTH, checkKey, m_lKdfIterations);
  if (m_blRecoveryKey) {
    auto cipher = CreateCipher(m_bCipherType, checkKey,
      EncryptionAlgorithm::Mode::DECRYPT);
    cipher->SetIV(m_pDbRecoveryKeyBlock);
    cipher->Decrypt(m_pDbRecoveryKeyBlock + DB_SALT_LENGTH, checkKey, DB_KEY_LENGTH);
  }
  return memcmp(checkKey, m_pDbKey, DB_KEY_LENGTH) == 0;
}
//---------------------------------------------------------------------------
bool PasswDatabase::CheckRecoveryKey(const SecureMem<word8>& recoveryKey)
{
  CheckDbOpen();
  if (!m_blRecoveryKey)
    throw EPasswDbError("Recovery key not set");
  CheckKeyEmpty(recoveryKey);

  SecureMem<word8> checkKey(DB_KEY_LENGTH);
  word8* pOffset = m_pDbRecoveryKeyBlock + DB_SALT_LENGTH + DB_KEY_LENGTH;
  pbkdf2_256bit(recoveryKey, recoveryKey.Size(), pOffset,
    DB_SALT_LENGTH, checkKey, m_lKdfIterations);

  auto cipher = CreateCipher(m_bCipherType, checkKey,
    EncryptionAlgorithm::Mode::DECRYPT);
  cipher->SetIV(pOffset);
  cipher->Decrypt(pOffset + DB_SALT_LENGTH, checkKey, checkKey.Size());

  return memcmp(checkKey, m_pDbKey, DB_KEY_LENGTH) == 0;
}
//---------------------------------------------------------------------------
void PasswDatabase::ChangeMasterKey(const SecureMem<word8>& newKey,
  word32 lKdfIterOverride,
  std::atomic<bool>* pCancelFlag,
  RandomGenerator* pThreadSafeRandGen)
{
  CheckDbOpen();
  CheckKeyEmpty(newKey);
  if (lKdfIterOverride == 0)
    lKdfIterOverride = m_lKdfIterations;
  if (m_blRecoveryKey) {
    if (pThreadSafeRandGen)
      pThreadSafeRandGen->GetData(m_pDbRecoveryKeyBlock, DB_SALT_LENGTH);
    else
      RandomPool::GetInstance().GetData(m_pDbRecoveryKeyBlock, DB_SALT_LENGTH);

    SecureMem<word8> derivedKey(DB_KEY_LENGTH);
    pbkdf2_256bit(newKey, newKey.Size(), m_pDbRecoveryKeyBlock, DB_SALT_LENGTH,
      derivedKey, lKdfIterOverride, pCancelFlag);

    if (pCancelFlag && *pCancelFlag)
      return;

    auto cipher = CreateCipher(m_bCipherType, derivedKey,
      EncryptionAlgorithm::Mode::ENCRYPT);
    cipher->SetIV(m_pDbRecoveryKeyBlock);
    cipher->Encrypt(m_pDbKey, m_pDbRecoveryKeyBlock + DB_SALT_LENGTH, DB_KEY_LENGTH);
  }
  else {
    if (pCancelFlag) {
      SecureMem<word8> derivedKey(DB_KEY_LENGTH);
      pbkdf2_256bit(newKey, newKey.Size(), m_pDbSalt, DB_SALT_LENGTH, derivedKey,
        lKdfIterOverride, pCancelFlag);
      if (!(*pCancelFlag))
        memcpy(m_pDbKey, derivedKey, DB_KEY_LENGTH);
    }
    else
      pbkdf2_256bit(newKey, newKey.Size(), m_pDbSalt, DB_SALT_LENGTH, m_pDbKey,
        lKdfIterOverride);
  }
  if (!(pCancelFlag && *pCancelFlag)) {
    m_lKdfIterations = lKdfIterOverride;
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::SetRecoveryKey(const SecureMem<word8>& key,
    const SecureMem<word8>& recoveryKey)
{
  CheckDbOpen();
  if (m_blRecoveryKey)
    throw EPasswDbError("Recovery key already set");
  CheckKeyEmpty(key);
  CheckKeyEmpty(recoveryKey);

  m_blRecoveryKey = true;

  RandomPool::GetInstance().GetData(m_pDbKey, DB_KEY_LENGTH);

  word8* pMemOffset = m_pDbRecoveryKeyBlock;
  for (int nKeyNum = 0; nKeyNum < 2; nKeyNum++) {
    RandomPool::GetInstance().GetData(pMemOffset, DB_SALT_LENGTH);

    SecureMem<word8> derivedKey(DB_KEY_LENGTH);
    const auto& keySrc = (nKeyNum == 0) ? key : recoveryKey;
    pbkdf2_256bit(keySrc, keySrc.Size(), pMemOffset, DB_SALT_LENGTH,
      derivedKey, m_lKdfIterations);

    auto cipher = CreateCipher(m_bCipherType, derivedKey,
      EncryptionAlgorithm::Mode::ENCRYPT);
    cipher->SetIV(pMemOffset);
    cipher->Encrypt(m_pDbKey, pMemOffset + DB_SALT_LENGTH, DB_KEY_LENGTH);

    pMemOffset += DB_SALT_LENGTH + DB_KEY_LENGTH;
  }

  RandomPool::GetInstance().Flush();
}
//---------------------------------------------------------------------------
void PasswDatabase::RemoveRecoveryKey(const SecureMem<word8>& key)
{
  CheckDbOpen();
  if (!m_blRecoveryKey)
    throw EPasswDbError("Recovery key not set");
  CheckKeyEmpty(key);

  m_blRecoveryKey = false;
  ChangeMasterKey(key);
  g_fastRandGen.GetData(m_pDbRecoveryKeyBlock, DB_RECOVERY_KEY_BLOCK_LENGTH);
}
//---------------------------------------------------------------------------
void PasswDatabase::ExportToCsv(const WString& sFileName, int nColMask,
  const WString* pColNames)
{
  std::unique_ptr<TStringFileStreamW> pFile(
    new TStringFileStreamW(sFileName, fmCreate, g_config.FileEncoding));

  WString sHeader;
  //int nNumCols = 0;
  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    if (nColMask & (1 << nI)) {
      //nNumCols++;
      if (!sHeader.IsEmpty())
        sHeader += ",";
      sHeader += "\"" + pColNames[nI] + "\"";
    }
  }

  sHeader += g_sNewline;

  pFile->WriteString(sHeader.c_str(), sHeader.Length());

  for (auto pEntry : m_db)
  {
	for (int nI = 0, nJ = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
      if (nColMask & (1 << nI)) {
        WString sField;
        switch (nI) {
        case PasswDbEntry::PASSWORD:
          sField = GetDbEntryPassw(*pEntry).c_str();
          break;
        case PasswDbEntry::CREATIONTIME:
          sField = pEntry->CreationTimeString.c_str();
          break;
        case PasswDbEntry::MODIFICATIONTIME:
          sField = pEntry->ModificationTimeString.c_str();
          break;
        case PasswDbEntry::PASSWCHANGETIME:
          sField = pEntry->PasswChangeTimeString.c_str();
          break;
        case PasswDbEntry::PASSWEXPIRYDATE:
          sField = pEntry->PasswExpiryDateString.c_str();
          break;
        case PasswDbEntry::PASSWHISTORY:
          for (const auto& he : pEntry->GetPasswHistory()) {
            if (!sField.IsEmpty())
              sField += ";";
            WString sTime = (he.first.dwLowDateTime == 0 &&
              he.first.dwHighDateTime == 0) ? "-" :
              WString(PasswDbEntry::TimeStampToString(he.first).c_str());
            sField += sTime + ";" + WString(he.second.c_str());
          }
          break;
        default:
          if (nI < PasswDbEntry::NUM_STRING_FIELDS) {
            sField = pEntry->Strings[nI].c_str();
            if (nI == PasswDbEntry::NOTES)
              sField = ReplaceStr(sField, CRLF, " ");
          }
        }
        WString sFormatted;
        if (nJ > 0)
          sFormatted = ",";
        sFormatted += "\"" + ReplaceStr(sField, "\"", "\"\"") + "\"";
        //if (nJ < nNumCols - 1)
        //  sField += ",";
        pFile->WriteString(sFormatted.c_str(), sFormatted.Length());
        eraseVclString(sField);
        eraseVclString(sFormatted);
        nJ++;
      }
    }

    pFile->WriteString(g_sNewline.c_str(), g_sNewline.Length());
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::CreateKeyFile(const WString& sFileName)
{
  std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmCreate));

  SecureMem<word8> key(DB_KEY_LENGTH), hexKey(2 * DB_KEY_LENGTH);
  RandomPool::GetInstance().GetData(key, key.Size());
  RandomPool::GetInstance().Flush();
  EntropyManager::GetInstance().ConsumeEntropyBits(DB_KEY_LENGTH * 8);

  static const word8 HEX_TABLE[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };

  for (word32 i = 0; i < key.Size(); i++) {
    hexKey[2*i] = HEX_TABLE[(key[i] >> 4) & 0x0f];
    hexKey[2*i+1] = HEX_TABLE[key[i] & 0x0f];
  }

  pFile->Write(hexKey, hexKey.Size());
}
//---------------------------------------------------------------------------
SecureMem<word8> PasswDatabase::GetKeyFromKeyFile(const WString& sFileName)
{
  std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmOpenRead));
  if (pFile->Size == 0)
    throw EPasswDbError("Key file is empty");

  SecureMem<word8> key(DB_KEY_LENGTH);

  // read contents as-is if file size equals key size
  if (pFile->Size == DB_KEY_LENGTH) {
    pFile->Read(key, key.Size());
    return key;
  }

  // check if file contains key in hexadecimal format
  if (pFile->Size == 2 * DB_KEY_LENGTH) {
    SecureMem<word8> hexKey(2 * DB_KEY_LENGTH);
    pFile->Read(hexKey, hexKey.Size());

    word32 i;
    word8 hiPart, c;
    for (i = 0; i < hexKey.Size(); i++) {
      c = hexKey[i];
      if (c >= '0' && c <= '9')
        c -= '0';
      else if (c >= 'A' && c <= 'F')
        c = c - 'A' + 10;
      else if (c >= 'a' && c <= 'f')
        c = c - 'a' + 10;
      else break;
      if (i % 2 == 0)
        hiPart = c;
      else
        key[i/2] = (hiPart << 4) | c;
    }

    hiPart = c = 0;
    if (i == hexKey.Size())
      return key;

    // return to beginning of file
    pFile->Seek(0, soFromBeginning);
  }

  // calculate hash of file contents
  SecureMem<sha256_context> hashCtx(1);
  sha256_init(hashCtx);
  sha256_starts(hashCtx, 0);
  SecureMem<word8> buf(1024);
  int nBytesRead;
  while ((nBytesRead = pFile->Read(buf, buf.Size())) != 0) {
    sha256_update(hashCtx, buf, nBytesRead);
  }
  sha256_finish(hashCtx, key);

  return key;
}
//---------------------------------------------------------------------------
SecureMem<word8> PasswDatabase::CombineKeySources(const SecureMem<word8>& passw,
  const WString& sKeyFileName)
{
  if (passw.IsEmpty() && sKeyFileName.IsEmpty())
    throw EPasswDbError("Neither password nor key file is specified");

  if (sKeyFileName.IsEmpty())
    return passw;

  SecureMem<word8> combinedKey(passw);
  try {
    combinedKey += GetKeyFromKeyFile(sKeyFileName);
  }
  catch (EFOpenError&) {
    throw EPasswDbError("Cannot open key file");
  }

  return combinedKey;
}
