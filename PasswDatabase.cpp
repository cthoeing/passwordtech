// PasswDatabase.cpp
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
#include <vector>
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
//---------------------------------------------------------------------------
#pragma package(smart_init)

static const word8
  PASSW_DB_MAGIC[4] = { 'P', 'W', 'd', 'b' };

static const word32
  FLAG_DEFAULT_USER_NAME = 1,
  FLAG_PASSW_FORMAT_SEQ  = 2,
  FLAG_PASSW_EXPIRY_DAYS = 4;

static const char
  DEFAULT_USER_NAME[] = "DefUserName",
  PASSW_FORMAT_SEQ[]  = "PWFormatSeq";

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
};
#pragma pack()

using namespace EncryptionAlgorithm;

__fastcall EPasswDbInvalidFormat::EPasswDbInvalidFormat(
  const WString& sMsg, int nVersion)
  : EPasswDbError(sMsg), m_nVersion(nVersion)
{
  if (nVersion > 0 && sMsg.Pos("%") > 0) {
    try {
      Message = Format(sMsg, ARRAYOFCONST((nVersion>>8, nVersion&0xff)));
    }
    catch (...)
    {}
  }
}

//---------------------------------------------------------------------------
const char* PasswDbEntry::GetFieldName(FieldType type)
{
  static const char* fieldNames[NUM_FIELDS] =
  { "Title", "UserName", "Password", "URL", "Keyword", "Notes", "KeyValueList",
    "Tags", "CreationTime", "ModificationTime", "PasswExpiryDate"
  };
  return fieldNames[type];
}
//---------------------------------------------------------------------------
bool PasswDbEntry::TimeStampToString(const FILETIME& ft, SecureWString& sDest)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);

  const int BUFSIZE = 256;
  wchar_t wszDateTime[BUFSIZE];

  int nDateLen = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL,
      wszDateTime, BUFSIZE);

  if (nDateLen == 0)
    return false;

  wszDateTime[nDateLen - 1] = ' ';

  int nTimeLen = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL,
      &wszDateTime[nDateLen], BUFSIZE - nDateLen);

  if (nTimeLen == 0)
    return false;

  sDest.Assign(wszDateTime, nDateLen + nTimeLen);

  memzero(wszDateTime, BUFSIZE);

  return true;
}
//---------------------------------------------------------------------------
bool PasswDbEntry::ExpiryDateToString(word32 lDate, SecureWString& sDest)
{
  int nYear, nMonth, nDay;
  if (lDate == 0 || !DecodeExpiryDate(lDate, nYear, nMonth, nDay)) {
    sDest.Empty();
    return true;
  }

  SYSTEMTIME st;
  memzero(&st, sizeof(st));

  st.wYear = nYear;
  st.wMonth = nMonth;
  st.wDay = nDay;

  const int BUFSIZE = 256;
  wchar_t wszDate[BUFSIZE];

  int nDateLen = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL,
      wszDate, BUFSIZE);

  if (nDateLen == 0)
    return false;

  sDest.Assign(wszDate, nDateLen);

  memzero(wszDate, BUFSIZE);

  return true;
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

  return NULL;
}
//---------------------------------------------------------------------------
void PasswDbEntry::SetKeyValue(const wchar_t* pwszKey, const wchar_t* pwszValue)
{
  if (*pwszKey == '\0' || *pwszValue == '\0')
    return;

  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
    if (_wcsicmp(it->first.c_str(), pwszKey) == 0) {
      it->second.Assign(pwszValue, wcslen(pwszValue) + 1);
      return;
    }
  }

  m_keyValueList.push_back(std::make_pair(SecureWString(pwszKey,
    wcslen(pwszKey) + 1), SecureWString(pwszValue, wcslen(pwszValue) + 1)));
}
//---------------------------------------------------------------------------
void PasswDbEntry::GetKeyValueListAsString(SecureWString& sDest, wchar_t sep) const
{
  if (m_keyValueList.empty())
    return;

  word32 lSize = 0;
  for (auto it = m_keyValueList.begin(); it != m_keyValueList.end(); it++)
  {
    lSize += it->first.StrLen() + it->second.StrLen() + 2;
  }

  sDest.New(lSize);

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
}
//---------------------------------------------------------------------------
void PasswDbEntry::ParseKeyValueList(const SecureWString& sList)
{
  // min. size due to min. key-value pair "X=Y\0"
  if (sList.Size() < 4)
    return;

  const wchar_t* p = sList.c_str();
  while (*p != '\0') {
    const wchar_t* pEq = wcschr(p, '=');
    if (pEq == NULL || pEq == p)
      break;
    word32 lKeyLen = static_cast<word32>(pEq - p);
    SecureWString sKey(p, lKeyLen + 1);
    sKey[lKeyLen] = '\0';
    p = pEq + 1;
    if (*p == '\0' || *p == '\n')
      break;
    const wchar_t* pSep = wcschr(p, '\n');
    if (pSep == NULL) {
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
  }
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
void PasswDbEntry::GetTagsAsString(SecureWString& sDest, wchar_t sep) const
{
  if (m_tags.empty())
    return;

  word32 lSize = 0;
  for (const auto& s : m_tags)
    lSize += s.StrLen() + 1;

  sDest.New(lSize);
  word32 lPos = 0;
  for (const auto& s : m_tags) {
    wcscpy(&sDest[lPos], s.c_str());
    lPos += s.StrLen();
    sDest[lPos++] = sep;
  }

  sDest[lPos - 1] = '\0';
}
//---------------------------------------------------------------------------
void PasswDbEntry::ParseTagList(const SecureWString& sList)
{
  // min. size due to min. tag "x\0"
  if (sList.Size() < 2)
    return;

  const wchar_t* p = sList.c_str();
  while (*p != '\0') {
    const wchar_t* pSep = wcschr(p, '\n');
    if (pSep == p)
      break;
    if (pSep == NULL) {
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
  }
}


PasswDatabase::PasswDatabase(bool blGetPlaintextPassw)
  : m_pSecMem(NULL), m_blPlaintextPassw(blGetPlaintextPassw),
    m_lDbEntryId(0), m_lCryptBufPos(0), m_nLastVersion(0), m_blOpen(false),
    m_bCipherType(CIPHER_AES256), m_lKdfIterations(KEY_HASH_ITERATIONS),
    m_lDefaultPasswExpiryDays(0)
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
  if (m_pSecMem != NULL) {
    g_fastRandGen.GetData(m_pSecMem, SECMEM_SIZE);
    VirtualUnlock(m_pSecMem, SECMEM_SIZE);
    VirtualFree(m_pSecMem, 0, MEM_RELEASE);
    m_pSecMem = NULL;
  }

  m_bCipherType = CIPHER_AES256;
  m_lKdfIterations = KEY_HASH_ITERATIONS;
  m_lDefaultPasswExpiryDays = 0;
  m_dbCipher.reset();
  m_dbHashCtx.Empty();
  m_dbIV.Empty();
  m_cryptBuf.Empty();

  for (auto pEntry : m_db)
	delete pEntry;

  m_db.clear();
  m_pFile.reset();
  m_lDbEntryId = 0;
  m_lCryptBufPos = 0;
  m_nLastVersion = 0;
  m_sDefaultUserName.Empty();
  m_blOpen = false;
}
//---------------------------------------------------------------------------
void PasswDatabase::New(const SecureMem<word8>& key)
{
  if (m_blOpen)
    throw EPasswDbError("Database already opened");

  m_pSecMem = reinterpret_cast<word8*>(
    VirtualAlloc(NULL, SECMEM_SIZE, MEM_COMMIT, PAGE_READWRITE));
  if (m_pSecMem == NULL)
    OutOfMemoryError();

  VirtualLock(m_pSecMem, SECMEM_SIZE);
  g_fastRandGen.GetData(m_pSecMem, SECMEM_SIZE);

  word8* pMemOffset = m_pSecMem + fprng_rand(2048);
  m_pMemCipherCtx = reinterpret_cast<chacha_ctx*>(pMemOffset);
  pMemOffset += sizeof(chacha_ctx);

  SecureMem<word8> memKey(SECMEM_KEY_LENGTH);
  RandomPool* pRandPool = RandomPool::GetInstance();
  pRandPool->GetData(memKey, SECMEM_KEY_LENGTH);

  //aes_setkey_enc(m_pMemCipherCtx, memKey, SECMEM_KEY_LENGTH*8);
  chacha_keysetup(m_pMemCipherCtx, memKey, SECMEM_KEY_LENGTH*8);

  m_pMemSalt = pMemOffset;
  pMemOffset += SECMEM_SALT_LENGTH;
  pRandPool->GetData(m_pMemSalt, SECMEM_SALT_LENGTH);

  m_pDbKey = pMemOffset;
  pMemOffset += DB_KEY_LENGTH;
  m_pDbSalt = pMemOffset;
  pMemOffset += DB_SALT_LENGTH;

  pRandPool->GetData(m_pDbSalt, DB_SALT_LENGTH);

  pbkdf2_256bit(key, key.Size(), m_pDbSalt, DB_SALT_LENGTH, m_pDbKey, m_lKdfIterations);

  m_nLastVersion = VERSION;
  m_blOpen = true;
}
//---------------------------------------------------------------------------
void PasswDatabase::Open(const SecureMem<word8>& key,
  const WString& sFileName)
{
  if (m_blOpen)
    throw EPasswDbError("Database already opened");

  m_pFile.reset(new TFileStream(sFileName, fmOpenRead | fmShareDenyWrite));
  if (m_pFile->Size > 104857600) // max. 100MB
    throw EPasswDbError("File too large");

  // check file size
  if (m_pFile->Size < sizeof(FileHeader) + DB_SALT_LENGTH + 8 + 16 + DB_HMAC_LENGTH)
    throw EPasswDbError("Invalid file size");

  // read file header (plaintext)
  FileHeader fh;
  m_pFile->Read(&fh, sizeof(fh));

  if (memcmp(fh.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC)) != 0)
    throw EPasswDbInvalidFormat("Unknown file format");

  if (fh.HeaderSize < sizeof(fh))
    throw EPasswDbInvalidFormat("Invalid file format");

  if (fh.CipherType > CIPHER_CHACHA20)
    throw EPasswDbInvalidFormat("Encryption algorithm not supported");

  if (fh.HashType != HASH_SHA256)
    throw EPasswDbInvalidFormat("Hash algorithm not supported");

  if (fh.KdfType != KDF_PBKDF2_SHA256)
    throw EPasswDbInvalidFormat("Key derivation function not supported");

  if (fh.KdfIterations == 0)
    throw EPasswDbInvalidFormat("Invalid number of KDF iterations");

  m_nLastVersion = fh.Version;
  m_bCipherType = fh.CipherType;
  m_lKdfIterations = fh.KdfIterations;

  m_pFile->Seek(fh.HeaderSize, soFromBeginning);

  // read entire file contents without header
  word32 lFileSize = m_pFile->Size - fh.HeaderSize;
  m_cryptBuf.New(std::max(1024u, lFileSize));
  m_pFile->Read(m_cryptBuf, lFileSize);

  // close file
  //m_pFile.reset();

  // derive encryption key from user key and salt
  SecureMem<word8> derivedKey(DB_KEY_LENGTH);
  pbkdf2_256bit(key, key.Size(), m_cryptBuf, DB_SALT_LENGTH, derivedKey,
    fh.KdfIterations);
  word32 lBufPos = DB_SALT_LENGTH;

  // do key setup
  switch (fh.CipherType) {
  case CIPHER_AES256:
    m_dbCipher.reset(new AES_CBC(derivedKey, DB_KEY_LENGTH, false));
    break;
  case CIPHER_CHACHA20:
    m_dbCipher.reset(new ChaCha20(derivedKey, DB_KEY_LENGTH));
    break;
  default:
    throw EPasswDbError("Cipher not implemented");
  }

  //m_dbCipherCtx.New(1);
  //aes_setkey_dec(m_dbCipherCtx, derivedKey, DB_KEY_LENGTH*8);

  // get IV
  //m_dbIV.New(DB_BLOCK_LENGTH);
  //memcpy(m_dbIV, &m_cryptBuf[lBufPos], DB_BLOCK_LENGTH);

  m_dbCipher->SetIV(&m_cryptBuf[lBufPos]);
  lBufPos += m_dbCipher->GetIVSize();

  SecureMem<word8> hmac(DB_HMAC_LENGTH);
  if (fh.Version >= 0x101)
    memcpy(hmac, &m_cryptBuf[lFileSize - DB_HMAC_LENGTH], DB_HMAC_LENGTH);

  // decrypt first N blocks
  PasswDbHeader header;
  word32 lBlockLen = m_dbCipher->GetBlockSize();
  word32 lAlignedHeaderSize = ((sizeof(header) + lBlockLen - 1) / lBlockLen) *
    lBlockLen;

  m_dbCipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos], lAlignedHeaderSize);

  //aes_crypt_cbc(m_dbCipherCtx, AES_DECRYPT, lAlignedHeaderSize, m_dbIV,
  //  &m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos]);

  memcpy(&header, &m_cryptBuf[lBufPos], sizeof(header));

  if (memcmp(header.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC)) != 0)
    throw EPasswDbInvalidKey(TRL("Database not encrypted, or invalid key"));

  lBufPos += lAlignedHeaderSize;

  // decrypt rest
  if (fh.Version < 0x101) {
    if (lFileSize > lBufPos)
      m_dbCipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos],
        lFileSize - lBufPos);
    memcpy(hmac, &m_cryptBuf[lFileSize - DB_HMAC_LENGTH], DB_HMAC_LENGTH);
  }
  else {
    int nRest = lFileSize - lBufPos - DB_HMAC_LENGTH;
    if (nRest > 0)
      m_dbCipher->Decrypt(&m_cryptBuf[lBufPos], &m_cryptBuf[lBufPos], nRest);
  }

  // calculate and check HMAC
  word32 lSaltIVLen = DB_SALT_LENGTH + m_dbCipher->GetIVSize();
  m_dbHashCtx.New(1);
  sha256_hmac_starts(m_dbHashCtx, derivedKey, DB_KEY_LENGTH, 0);
  if (lFileSize > lSaltIVLen + DB_HMAC_LENGTH)
    sha256_hmac_update(m_dbHashCtx,
      &m_cryptBuf[lSaltIVLen],
      lFileSize - lSaltIVLen - DB_HMAC_LENGTH);

  SecureMem<word8> checkHmac(DB_HMAC_LENGTH);
  sha256_hmac_finish(m_dbHashCtx, checkHmac);

  if (checkHmac != hmac)
    throw EPasswDbInvalidKey(TRL("File contents modified, or invalid key"));

  // set position within buffer
  m_lCryptBufPos = lSaltIVLen + header.HeaderSize;

  // clean up
  m_dbHashCtx.Empty();
  m_dbIV.Empty();
  m_dbCipher.reset();

  //m_dbCipherCtx.Empty();

  // initialize crypto engine
  New(key);

  // read global database settings
  if (fh.Version >= 0x102) {
    for (int i = 0; i < header.NumOfVariableParam; i++) {
      word32 lFlag = ReadType<word32>();
      if ((header.Flags & FLAG_DEFAULT_USER_NAME) &&
          lFlag == FLAG_DEFAULT_USER_NAME) {
        ReadString(m_sDefaultUserName);
      }
      else if ((header.Flags & FLAG_PASSW_FORMAT_SEQ) &&
               lFlag == FLAG_PASSW_FORMAT_SEQ) {
        ReadString(m_sPasswFormatSeq);
      }
      else if ((header.Flags & FLAG_PASSW_EXPIRY_DAYS) &&
               lFlag == FLAG_PASSW_EXPIRY_DAYS) {
        m_lDefaultPasswExpiryDays = std::min(3650u, ReadType<word32>());
      }
      else
        SkipField();
    }
  }
  else {
    for (int i = 0; i < header.NumOfVariableParam; i++) {
      SecureAnsiString sParamName;
      ReadString(sParamName);
      if ((header.Flags & FLAG_DEFAULT_USER_NAME) &&
          stricmp(sParamName, DEFAULT_USER_NAME) == 0) {
        ReadString(m_sDefaultUserName);
      }
      else if ((header.Flags & FLAG_PASSW_FORMAT_SEQ) &&
               stricmp(sParamName, PASSW_FORMAT_SEQ) == 0) {
        ReadString(m_sPasswFormatSeq);
      }
      else
        SkipField();
    }
  }

  // read column titles (AnsiStrings)
  std::vector<int> idxConv(header.NumOfFields);
  bool fieldsUsed[PasswDbEntry::NUM_FIELDS];
  memzero(fieldsUsed, sizeof(fieldsUsed));

  for (int nI = 0; nI < header.NumOfFields; nI++) {
    idxConv[nI] = -1;
    SecureAnsiString sStr;
    ReadString(sStr);
    if (sStr.IsEmpty())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
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
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);

  // now read the fields...
  // max. number is NumOfFields + "end of entry" mark
  int nMaxNumFields = header.NumOfFields + 1;
  for (int nI = 0; nI < header.NumOfEntries; nI++) {
    PasswDbEntry* pEntry = AddDbEntry(false, false);
    for (int nJ = 0; nJ < nMaxNumFields; nJ++) {
      int nFieldIndex = ReadFieldIndex();
      if (nFieldIndex == PasswDbEntry::END)
        break;
      if (nFieldIndex < idxConv.size() && idxConv[nFieldIndex] >= 0) {
        SecureWString sField;
        int nIdx = idxConv[nFieldIndex];
        switch (nIdx) {
        case PasswDbEntry::KEYVALUELIST:
          ReadString(sField);
          pEntry->ParseKeyValueList(sField);
          pEntry->UpdateKeyValueString();
          break;
        case PasswDbEntry::TAGS:
          ReadString(sField);
          pEntry->ParseTagList(sField);
          pEntry->UpdateTagsString();
          break;
        case PasswDbEntry::CREATIONTIME:
          pEntry->CreationTime = ReadField<FILETIME>();
          pEntry->TimeStampToString(pEntry->CreationTime, pEntry->CreationTimeString);
          break;
        case PasswDbEntry::MODIFICATIONTIME:
          pEntry->ModificationTime = ReadField<FILETIME>();
          pEntry->TimeStampToString(pEntry->ModificationTime, pEntry->ModificationTimeString);
          break;
        case PasswDbEntry::PASSWEXPIRYDATE:
          pEntry->PasswExpiryDate = ReadField<word32>();
          if (!pEntry->ExpiryDateToString(pEntry->PasswExpiryDate, pEntry->PasswExpiryDateString))
            pEntry->PasswExpiryDate = 0;
          break;
        default:
          ReadString(sField);
          if (nIdx == PasswDbEntry::PASSWORD)
            SetDbEntryPassw(pEntry, sField);
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

  m_cryptBuf.Empty();
  memzero(&header, sizeof(header));
  memzero(&fh, sizeof(fh));
}
//---------------------------------------------------------------------------
void PasswDatabase::Write(const void* pBuf, word32 lNumOfBytes)
{
  const word8* pSrcBuf = reinterpret_cast<const word8*>(pBuf);

  while (lNumOfBytes) {
    word32 lToCopy = std::min(lNumOfBytes, IO_BUF_SIZE - m_lCryptBufPos);
    memcpy(&m_cryptBuf[m_lCryptBufPos], pSrcBuf, lToCopy);
    m_lCryptBufPos += lToCopy;

    if (m_lCryptBufPos == IO_BUF_SIZE)
      Flush();

    pSrcBuf += lToCopy;
    lNumOfBytes -= lToCopy;
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::Flush(void)
{
  if (m_lCryptBufPos == 0)
    return;

  word32 lAlignedSize = m_lCryptBufPos;
  if (m_dbCipher->AlignToBlockSize()) {
    word32 lSize = m_lCryptBufPos;
    word32 lBlockLen = m_dbCipher->GetBlockSize();
    lAlignedSize = ((lSize + lBlockLen - 1) / lBlockLen) * lBlockLen;

    if (lAlignedSize > lSize)
      RandomPool::GetInstance()->GetData(&m_cryptBuf[lSize], lAlignedSize - lSize);
  }

  sha256_hmac_update(m_dbHashCtx, m_cryptBuf, lAlignedSize);

  m_dbCipher->Encrypt(m_cryptBuf, m_cryptBuf, lAlignedSize);
  //aes_crypt_cbc(m_dbCipherCtx, AES_ENCRYPT, lAlignedSize, m_dbIV,
  //  m_cryptBuf, m_cryptBuf);

  m_pFile->Write(m_cryptBuf, lAlignedSize);

  m_lCryptBufPos = 0;
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
void PasswDatabase::WriteString(const char* pszStr, int nIndex)
{
  if (nIndex >= 0) {
    if (pszStr != NULL && *pszStr != '\0')
      WriteFieldBuf(pszStr, strlen(pszStr), nIndex);
  }
  else
    WriteFieldBuf(pszStr, (pszStr != NULL) ? strlen(pszStr) : 0);
}
//---------------------------------------------------------------------------
void PasswDatabase::WriteString(const SecureWString& sStr, int nIndex)
{
  if (sStr.Size() > 1) {
    SecureAnsiString asUtf8;
    WStringToUtf8(sStr.c_str(), asUtf8);
    WriteString(asUtf8.c_str(), nIndex);
  }
  else if (nIndex < 0)
    WriteFieldBuf(nullptr, 0);
}
//---------------------------------------------------------------------------
void PasswDatabase::SaveToFile(const WString& sFileName)
{
  if (m_bCipherType > CIPHER_CHACHA20)
    throw EPasswDbError("Invalid cipher");
  if (m_lKdfIterations == 0)
    throw EPasswDbError("Invalid number of KDF iterations");

  m_pFile.reset();
  m_pFile.reset(new TFileStream(sFileName, fmCreate | fmShareDenyWrite));

  FileHeader fh;
  memcpy(fh.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC));
  fh.HeaderSize = sizeof(FileHeader);
  fh.Version = VERSION;
  fh.Flags = 0;
  fh.CipherType = m_bCipherType; //CIPHER_AES256;
  fh.HashType = HASH_SHA256;
  fh.KdfType = KDF_PBKDF2_SHA256;
  fh.KdfIterations = m_lKdfIterations; //KEY_HASH_ITERATIONS;

  m_pFile->Write(&fh, sizeof(fh));
  m_pFile->Write(m_pDbSalt, DB_SALT_LENGTH);

  switch (m_bCipherType) {
  case CIPHER_AES256:
    m_dbCipher.reset(new AES_CBC(m_pDbKey, DB_KEY_LENGTH, true));
    break;
  case CIPHER_CHACHA20:
    m_dbCipher.reset(new ChaCha20(m_pDbKey, DB_KEY_LENGTH));
    break;
  default:
    throw EPasswDbError("Cipher not implemented");
  }

  //m_dbIV.New(DB_BLOCK_LENGTH);
  word32 lIVLen = m_dbCipher->GetIVSize();
  SecureMem<word8> iv(lIVLen);
  RandomPool::GetInstance()->GetData(iv, lIVLen);
  m_dbCipher->SetIV(iv);
  m_pFile->Write(iv, lIVLen);

  m_cryptBuf.New(IO_BUF_SIZE);
  m_lCryptBufPos = 0;

  //m_dbCipherCtx.New(1);
  //aes_setkey_enc(m_dbCipherCtx, m_pDbKey, DB_KEY_LENGTH*8);

  m_dbHashCtx.New(1);
  sha256_hmac_starts(m_dbHashCtx, m_pDbKey, DB_KEY_LENGTH, 0);

  PasswDbHeader header;
  memcpy(header.Magic, PASSW_DB_MAGIC, sizeof(PASSW_DB_MAGIC));
  header.HeaderSize = sizeof(header);
  //header.Version = VERSION;
  header.Flags = 0;
  header.NumOfVariableParam = 0;
  header.NumOfFields = PasswDbEntry::NUM_FIELDS;
  header.NumOfEntries = m_db.size();

  if (m_sDefaultUserName.Size() > 1) {
    header.Flags |= FLAG_DEFAULT_USER_NAME;
    header.NumOfVariableParam++;
  }

  if (m_sPasswFormatSeq.Size() > 1) {
    header.Flags |= FLAG_PASSW_FORMAT_SEQ;
    header.NumOfVariableParam++;
  }

  if (m_lDefaultPasswExpiryDays != 0) {
    header.Flags |= FLAG_PASSW_EXPIRY_DAYS;
    header.NumOfVariableParam++;
  }

  Write(&header, sizeof(header));

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

  memzero(&header, sizeof(header));

  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    WriteString(PasswDbEntry::GetFieldName(
      static_cast<PasswDbEntry::FieldType>(nI)));
  }

  const word8 bEndOfEntry = PasswDbEntry::END;

  for (auto pEntry : m_db)
  {
	for (int nI = 0; nI < PasswDbEntry::NUM_STRING_FIELDS; nI++) {
      SecureWString sField;
      switch (nI) {
      case PasswDbEntry::PASSWORD:
        GetDbEntryPassw(pEntry, sField);
        WriteString(sField, nI);
        break;
      case PasswDbEntry::KEYVALUELIST:
        pEntry->GetKeyValueListAsString(sField);
        WriteString(sField, nI);
        break;
      case PasswDbEntry::TAGS:
        pEntry->GetTagsAsString(sField);
        WriteString(sField, nI);
        break;
      default:
        WriteString(pEntry->Strings[nI], nI);
      }
    }
    WriteField(pEntry->CreationTime, PasswDbEntry::CREATIONTIME);
    WriteField(pEntry->ModificationTime, PasswDbEntry::MODIFICATIONTIME);
    if (pEntry->PasswExpiryDate != 0)
      WriteField(pEntry->PasswExpiryDate, PasswDbEntry::PASSWEXPIRYDATE);
    Write(&bEndOfEntry, 1);
  }

  Flush();

  SecureMem<word8> hmac(DB_HMAC_LENGTH);
  sha256_hmac_finish(m_dbHashCtx, hmac);

  m_pFile->Write(hmac, DB_HMAC_LENGTH);

  //Write(hmac, DB_HMAC_LENGTH);
  //Flush(false);

  m_dbHashCtx.Empty();
  m_dbCipher.reset();
  //m_dbCipherCtx.Empty();
  //m_dbIV.Empty();

  m_nLastVersion = VERSION;

  m_pFile.reset();
  m_pFile.reset(new TFileStream(sFileName, fmOpenRead | fmShareDenyWrite));
}
//---------------------------------------------------------------------------
word32 PasswDatabase::ReadFieldSize(void)
{
  if (m_lCryptBufPos + 4 > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
  word32 lSize;
  memcpy(&lSize, &m_cryptBuf[m_lCryptBufPos], 4);
  m_lCryptBufPos += 4;
  return lSize;
}
//---------------------------------------------------------------------------
int PasswDatabase::ReadFieldIndex(void)
{
  if (m_lCryptBufPos + 1 > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
  word8 bIndex;
  memcpy(&bIndex, &m_cryptBuf[m_lCryptBufPos++], 1);
  return bIndex;
}
//---------------------------------------------------------------------------
void PasswDatabase::ReadString(SecureAnsiString& sDest)
{
  word32 lSize = ReadFieldSize();
  if (lSize != 0) {
    if (m_lCryptBufPos + lSize > m_cryptBuf.Size())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
    sDest.New(lSize + 1);
    memcpy(sDest, &m_cryptBuf[m_lCryptBufPos], lSize);
    sDest[lSize] = '\0';
    m_lCryptBufPos += lSize;
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::ReadString(SecureWString& sDest)
{
  SecureAnsiString asUtf8;
  ReadString(asUtf8);
  Utf8ToWString(asUtf8, sDest);
}
//---------------------------------------------------------------------------
void PasswDatabase::SkipField(void)
{
  m_lCryptBufPos += ReadFieldSize();
  if (m_lCryptBufPos > m_cryptBuf.Size())
    throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
}
//---------------------------------------------------------------------------
PasswDbEntry* PasswDatabase::AddDbEntry(bool blSetTimeStamp,
                                        bool blSetDefUserName)
{
  PasswDbEntry* pEntry = new PasswDbEntry(m_lDbEntryId++, m_db.size(),
    blSetTimeStamp);
  m_db.push_back(pEntry);
  if (blSetDefUserName)
    pEntry->Strings[PasswDbEntry::USERNAME] = m_sDefaultUserName;
  //PasswDbList::iterator it = --m_db.end();
  //it->m_listPos = it;
  return pEntry;
}
//---------------------------------------------------------------------------
void PasswDatabase::DeleteDbEntry(PasswDbEntry* pEntry)
{
  if (pEntry->m_lIndex < m_db.size()) {
	PasswDbList::iterator it = m_db.begin() + pEntry->m_lIndex;
    delete pEntry;
    m_db.erase(it);

	word32 lIndex = 0;
	for (auto pEntry : m_db)
	  pEntry->m_lIndex = lIndex++;
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::MoveDbEntry(word32 lCurrPos, word32 lNewPos)
{
  word32 lSize = m_db.size();
  if (lCurrPos != lNewPos && lCurrPos < lSize && lNewPos < lSize) {
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
  }
}
//---------------------------------------------------------------------------
void PasswDatabase::SetDbEntryPassw(PasswDbEntry* pEntry,
  const SecureWString& sPassw)
{
  if (sPassw.Size() <= 1) {
    pEntry->m_encPassw.Empty();
    pEntry->Strings[PasswDbEntry::PASSWORD].Empty();
    return;
  }

  sha1_hmac(m_pMemSalt, SECMEM_SALT_LENGTH, sPassw.Bytes(), sPassw.SizeBytes(),
    pEntry->m_passwHash);

  SecureMem<word8> iv(SECMEM_IV_LENGTH);
  iv.Clear();
  *(reinterpret_cast<word32*>(iv.Data())) = pEntry->m_lId;

  pEntry->m_encPassw.New(sPassw.Size());

  chacha_ivsetup(m_pMemCipherCtx, iv, NULL);
  chacha_encrypt_bytes(m_pMemCipherCtx, sPassw.Bytes(), pEntry->m_encPassw.Bytes(),
    sPassw.SizeBytes());
  //size_t iv_off = 0;
  //aes_crypt_cfb128(m_pMemCipherCtx, AES_ENCRYPT, sPassw.SizeBytes(), &iv_off, iv,
  //  sPassw.Bytes(), pEntry->m_encPassw.Bytes());

  if (m_blPlaintextPassw)
    pEntry->Strings[PasswDbEntry::PASSWORD] = sPassw;
}
//---------------------------------------------------------------------------
void PasswDatabase::GetDbEntryPassw(const PasswDbEntry* pEntry,
  SecureWString& sPassw)
{
  if (pEntry->m_encPassw.IsEmpty())
    return;

  if (pEntry->HasPlaintextPassw()) {
    sPassw = pEntry->Strings[PasswDbEntry::PASSWORD];
    return;
  }

  sPassw.New(pEntry->m_encPassw.Size());

  SecureMem<word8> iv(SECMEM_IV_LENGTH);
  iv.Clear();
  *(reinterpret_cast<word32*>(iv.Data())) = pEntry->m_lId;

  //size_t iv_off = 0;
  //aes_crypt_cfb128(m_pMemCipherCtx, AES_DECRYPT, pEntry->m_encPassw.SizeBytes(),
  //  &iv_off, iv, pEntry->m_encPassw.Bytes(), sPassw.Bytes());
  chacha_ivsetup(m_pMemCipherCtx, iv, NULL);
  chacha_encrypt_bytes(m_pMemCipherCtx, pEntry->m_encPassw.Bytes(), sPassw.Bytes(),
    pEntry->m_encPassw.SizeBytes());

  SecureMem<word8> checkHash(pEntry->m_passwHash.Size());
  sha1_hmac(m_pMemSalt, SECMEM_SALT_LENGTH, sPassw.Bytes(), sPassw.SizeBytes(),
    checkHash);

  if (memcmp(pEntry->m_passwHash, checkHash, checkHash.Size()) != 0)
    throw EPasswDbError("Internal error: Password decryption failed");
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
      GetDbEntryPassw(pEntry, pEntry->Strings[PasswDbEntry::PASSWORD]);
    else
      pEntry->Strings[PasswDbEntry::PASSWORD].Empty();
  }
}
//---------------------------------------------------------------------------
bool PasswDatabase::CheckMasterKey(const SecureMem<word8>& key)
{
  SecureMem<word8> checkKey(DB_KEY_LENGTH);
  pbkdf2_256bit(key, key.Size(), m_pDbSalt, DB_SALT_LENGTH, checkKey,
    m_lKdfIterations);
  return memcmp(checkKey, m_pDbKey, DB_KEY_LENGTH) == 0;
}
//---------------------------------------------------------------------------
void PasswDatabase::ChangeMasterKey(const SecureMem<word8>& newKey)
{
  pbkdf2_256bit(newKey, newKey.Size(), m_pDbSalt, DB_SALT_LENGTH, m_pDbKey,
    m_lKdfIterations);
}
//---------------------------------------------------------------------------
void PasswDatabase::ExportToCsv(const WString& sFileName, int nColMask,
  const WString* pColNames)
{
  std::unique_ptr<TStringFileStreamW> pFile(
    new TStringFileStreamW(sFileName, fmCreate, g_config.FileEncoding));

  WString sHeader;
  int nNumCols = 0;
  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    if (nColMask & (1 << nI)) {
      nNumCols++;
      sHeader += "\"" + pColNames[nI] + "\"";
      if (nI < PasswDbEntry::NUM_FIELDS - 1)
        sHeader += ",";
    }
  }

  sHeader += g_sNewline;

  int nWritten;
  pFile->WriteString(sHeader.c_str(), sHeader.Length(), nWritten);

  for (auto pEntry : m_db)
  {
	for (int nI = 0, nJ = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
      if (nColMask & (1 << nI)) {
        WString sField;
        if (nI == PasswDbEntry::PASSWORD && !pEntry->HasPlaintextPassw()) {
          SecureWString sPassw;
          GetDbEntryPassw(pEntry, sPassw);
          sField = sPassw.c_str();
          //pFile->WriteString(pwszPassw, wcslen(pwszPassw), nWritten);
        }
        else {
          sField = pEntry->Strings[nI].c_str();
          if (nI == PasswDbEntry::NOTES)
            sField = ReplaceStr(sField, CRLF, " ");
          //const wchar_t* pwszStr = pEntry->Strings[nI].c_str();
          //pFile->WriteString(pwszStr, wcslen(pwszStr), nWritten);
        }
        sField = "\"" + ReplaceStr(sField, "\"", "\"\"") + "\"";
        if (nJ < nNumCols - 1)
          sField += ",";
        pFile->WriteString(sField.c_str(), sField.Length(), nWritten);
        eraseVclString(sField);
        nJ++;
      }
    }

    pFile->WriteString(g_sNewline.c_str(), g_sNewline.Length(), nWritten);
  }
}
//---------------------------------------------------------------------------

