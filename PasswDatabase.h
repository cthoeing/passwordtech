// PasswDatabase.h
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
#ifndef PasswDatabaseH
#define PasswDatabaseH
//---------------------------------------------------------------------------
#include <vector>
#include <memory>
#include <set>
#include <Classes.hpp>
#include "UnicodeUtil.h"
#include "SecureMem.h"
//#include "chacha.h"
//#include "aes.h"
#include "SymmetricCipher.h"
#include "sha256.h"

// class for password database entry
class PasswDbEntry {
public:
  friend class PasswDatabase;

  typedef std::pair<SecureWString,SecureWString> KeyValue;
  typedef std::vector<KeyValue> KeyValueList;

  enum {
    NUM_FIELDS = 11,
    NUM_STRING_FIELDS = 8
  };

  enum FieldType {
    TITLE,
    USERNAME,
    PASSWORD,
    URL,
    KEYWORD,
    NOTES,
    KEYVALUELIST,
    TAGS,
    CREATIONTIME,
    MODIFICATIONTIME,
    PASSWEXPIRYDATE,

    END = 0xff
  };

  SecureWString Strings[NUM_STRING_FIELDS];
  SecureWString CreationTimeString;
  SecureWString ModificationTimeString;
  SecureWString PasswExpiryDateString;
  FILETIME CreationTime;
  FILETIME ModificationTime;
  word32 PasswExpiryDate;
  word32 UserFlags;
  int UserTag;

  ~PasswDbEntry()
  {
    memzero(&CreationTime, sizeof(CreationTime));
    memzero(&ModificationTime, sizeof(ModificationTime));
    PasswExpiryDate = 0;
    UserFlags = 0;
    UserTag = 0;
  }

  // get unique 32-bit identifier
  word32 GetId(void) const
  {
    return m_lId;
  }

  // get index of entry within database
  word32 GetIndex(void) const
  {
    return m_lIndex;
  }

  // check if password is empty
  bool IsPasswEmpty(void) const
  {
    return m_encPassw.IsEmpty();
  }

  // password can be stored as plaintext or in encrypted form;
  // check if plaintext password is available
  bool HasPlaintextPassw(void) const
  {
    return !m_encPassw.IsEmpty() && !Strings[PASSWORD].IsEmpty();
  }

  // update timestamp of last modification
  void UpdateModificationTime(void)
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    SystemTimeToFileTime(&st, &ModificationTime);
    TimeStampToString(ModificationTime, ModificationTimeString);
  }

  // access to key-value list
  const KeyValueList& GetKeyValueList(void) const
  {
    return m_keyValueList;
  }

  void SetKeyValueList(const KeyValueList& src)
  {
    m_keyValueList = src;
    UpdateKeyValueString();
  }

  // get/set "value" item corresponding to "key" item
  const SecureWString* GetKeyValue(const wchar_t* pwszKey) const;
  void SetKeyValue(const wchar_t* pwszKey, const wchar_t* pwszValue);

  // get key-value list as single string, with key-value pairs separated
  // by "sep" character
  SecureWString GetKeyValueListAsString(wchar_t sep = '\n') const;

  // clear (empty) key-value list
  void ClearKeyValueList(void)
  {
    m_keyValueList.clear();
    Strings[KEYVALUELIST].Empty();
  }

  // update internal key-value list string, key-value pairs separated by ','
  void UpdateKeyValueString(void)
  {
    Strings[KEYVALUELIST] = GetKeyValueListAsString(',');
  }

  // get/set list of tags
  const std::set<SecureWString>& GetTagList(void) const
  {
    return m_tags;
  }

  void SetTagList(const std::set<SecureWString>& tags)
  {
    m_tags = tags;
    UpdateTagsString();
  }

  // check if specified tag is available
  bool CheckTag(const SecureWString& sTag) const;

  // add tag to entry
  // <- 'true' if tag was added successfully, 'false' if tag already exists
  bool AddTag(const SecureWString& sTag);

  // get list of tags as single string, with tags separated by "sep" character
  SecureWString GetTagsAsString(wchar_t sep = '\n') const;

  // update internal list of tags, tags separated by ','
  void UpdateTagsString(void)
  {
    Strings[TAGS] = GetTagsAsString(',');
  }

  // clear (empty) list of tags
  void ClearTagList(void)
  {
    m_tags.clear();
    Strings[TAGS].Empty();
  }

  // convert timestamp to string using Windows API
  static bool TimeStampToString(const FILETIME& ft, SecureWString& sDest);

  // convert expiry date to string
  static bool ExpiryDateToString(word32 lDate, SecureWString& sDest);

  // decode 32-bit expiry data into year/month/date format
  static bool DecodeExpiryDate(word32 lDate, int& nYear, int& nMonth, int& nDay)
  {
    nYear = lDate >> 16;
    nMonth = (lDate >> 8) & 0xff;
    nDay = lDate & 0xff;
    return nYear > 0 && nMonth >= 1 && nMonth <= 12 && nDay >= 1 && nDay <= 31;
  }

  // encode year/month/date specification into 32-bit expiry date
  static word32 EncodeExpiryDate(int nYear, int nMonth, int nDay)
  {
    return (nYear << 16) | (nMonth << 8) | nDay;
  }

  // get field name of field type as string
  static const char* GetFieldName(FieldType type);

private:
  // private constructor
  // -> unique 32-bit identifier
  // -> index of entry within database
  // -> 'true': set "creation" and "last modification" timestamps
  PasswDbEntry(word32 lId, word32 lIndex, bool blSetTimeStamps)
    : m_lId(lId), m_lIndex(lIndex), m_passwHash(20), UserFlags(0), UserTag(0),
    PasswExpiryDate(0)
  {
    if (blSetTimeStamps) {
      SYSTEMTIME st;
      GetLocalTime(&st);
      SystemTimeToFileTime(&st, &CreationTime);
      TimeStampToString(CreationTime, CreationTimeString);
      ModificationTime = CreationTime;
      ModificationTimeString = CreationTimeString;
    }
    else {
      CreationTime.dwLowDateTime = CreationTime.dwHighDateTime = 0;
      ModificationTime.dwLowDateTime = ModificationTime.dwHighDateTime = 0;
    }
  }

  // parse key-value list specified as string
  void ParseKeyValueList(const SecureWString& sList);

  // parse list of tags specified as string
  void ParseTagList(const SecureWString& sList);

  word32 m_lId;
  word32 m_lIndex;
  SecureMem<wchar_t> m_encPassw;
  SecureMem<word8> m_passwHash;
  std::vector<KeyValue> m_keyValueList;
  std::set<SecureWString> m_tags;
};


typedef std::vector<PasswDbEntry*> PasswDbList;


// base class for exceptions related to password databases
class EPasswDbError : public Exception
{
public:
  __fastcall EPasswDbError(const WString& sMsg)
    : Exception(sMsg)
  {}
};

class EPasswDbInvalidFormat : public EPasswDbError
{
private:
  int m_nVersion;

public:
  __fastcall EPasswDbInvalidFormat(const WString& sMsg, int nVersion = 0);

  __property int Version =
  { read=m_nVersion };
};

// db not encrypted, invalid key, or authentication error (HMAC mismatch)
class EPasswDbInvalidKey : public EPasswDbError
{
public:
  __fastcall EPasswDbInvalidKey(const WString& sMsg)
    : EPasswDbError(sMsg)
  {}
};

const WString E_INVALID_FORMAT = "Invalid/unknown file format (version: %d.%d)";

class PasswDatabase {
private:
  PasswDbList m_db;
  int m_nLastVersion;
  word8 m_bCipherType;
  word32 m_lKdfIterations;
  word32 m_lDbEntryId;
  word8* m_pSecMem;
  chacha_ctx* m_pMemCipherCtx;
  word8* m_pMemSalt;
  word8* m_pDbSalt;
  word8* m_pDbKey;
  //SecureMem<aes_context> m_dbCipherCtx;
  std::unique_ptr<EncryptionAlgorithm::SymmetricCipher> m_dbCipher;
  SecureMem<sha256_context> m_dbHashCtx;
  SecureMem<word8> m_dbIV;
  SecureMem<word8> m_cryptBuf;
  word32 m_lCryptBufPos;
  std::unique_ptr<TFileStream> m_pFile;
  SecureWString m_sDefaultUserName;
  SecureWString m_sPasswFormatSeq;
  word32 m_lDefaultPasswExpiryDays;
  bool m_blPlaintextPassw;
  bool m_blOpen;

  enum {
    VERSION_HIGH = 1,
    VERSION_LOW = 2,
    VERSION = (VERSION_HIGH << 8) | VERSION_LOW,

    SECMEM_SIZE = 4096,
    SECMEM_KEY_LENGTH = 32,
    SECMEM_SALT_LENGTH = 16,
    SECMEM_IV_LENGTH = 8,

    DB_KEY_LENGTH = 32,
    //DB_BLOCK_LENGTH = 16,
    DB_SALT_LENGTH = 32,
    //DB_IV_LENGTH = 16,
    //DB_SALT_IV_LENGTH = DB_SALT_LENGTH + DB_IV_LENGTH,
    DB_HMAC_LENGTH = 32,

    HASH_SHA256 = 0,

    KDF_PBKDF2_SHA256 = 0,

    IO_BUF_SIZE = 65536
  };

  void Write(const void* pBuf, word32 lNumOfBytes);

  void Flush(void);

  void WriteFieldBuf(const void* pBuf, word32 lNumOfBytes, int nIndex = -1);

  template<typename T> void WriteField(const T& t, int nIndex = -1)
  {
    WriteFieldBuf(&t, sizeof(T), nIndex);
  }

  template<typename T> void WriteType(const T& t)
  {
    Write(&t, sizeof(T));
  }

  void WriteString(const char* pszStr, int nIndex = -1);

  void WriteString(const SecureWString& sStr, int nIndex = -1);

  //void WriteTimeStamp(const FILETIME& ft, int nIndex = -1);

  int ReadFieldIndex(void);

  word32 ReadFieldSize(void);

  SecureAnsiString ReadAnsiString(void);

  SecureWString ReadString(void);

  //void ReadTimeStamp(FILETIME& ft);

  template<typename T> T ReadField(void)
  {
    word32 lSize = ReadFieldSize();
    if (lSize != sizeof(T))
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
    T t;
    memcpy(&t, &m_cryptBuf[m_lCryptBufPos], lSize);
    m_lCryptBufPos += lSize;
    return t;
  }

  template<typename T> T ReadType(void)
  {
    if (m_lCryptBufPos + sizeof(T) > m_cryptBuf.Size())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT, m_nLastVersion);
    T t;
    memcpy(&t, &m_cryptBuf[m_lCryptBufPos], sizeof(T));
    m_lCryptBufPos += sizeof(T);
    return t;
  }


  void SkipField(void);

  int GetLastVersion(void)
  {
    return m_nLastVersion;
  }

  void SetCipherType(word8 bType)
  {
    if (bType > CIPHER_CHACHA20)
      throw EPasswDbError("Cipher not supported");
    m_bCipherType = bType;
  }

  void SetKdfIterations(word32 lIter)
  {
    if (lIter == 0)
      throw EPasswDbError("Invalid number of KDF iterations");
    m_lKdfIterations = lIter;
  }

  void ReassignIndices(void)
  {
    word32 lIndex = 0;
    for (auto pEntry : m_db)
      pEntry->m_lIndex = lIndex++;
  }

public:

  enum {
    KEY_HASH_ITERATIONS = 16384,

    CIPHER_AES256 = 0,
    CIPHER_CHACHA20 = 1,
  };

  PasswDatabase(bool blGetPlaintextPassw = false);
  virtual ~PasswDatabase();

  void New(const SecureMem<word8>& key);

  void Open(const SecureMem<word8>& key,
    const WString& sFileName = "");

  void Close(void);

  bool IsOpen(void) const
  {
    return m_blOpen;
  }

  void SaveToFile(const WString& sFileName);

  void ReleaseFile(void)
  {
    m_pFile.reset();
  }

  PasswDbList& GetDatabase(void)
  {
    return m_db;
  }

  word32 GetSize(void) const
  {
    return m_db.size();
  }

  PasswDbEntry* AddDbEntry(bool blSetTimeStamp = true,
    bool blSetDefUserName = true);

  void DeleteDbEntry(PasswDbEntry& pEntry);

  void MoveDbEntry(word32 lCurrPos, word32 lNewPos);

  void SetDbEntryPassw(PasswDbEntry& pEntry,
    const SecureWString& sPassw);

  SecureWString GetDbEntryPassw(const PasswDbEntry& pEntry);

  void SetPlaintextPassw(bool blPlaintextPassw);

  bool CheckMasterKey(const SecureMem<word8>& key);

  void ChangeMasterKey(const SecureMem<word8>& newKey);

  void ExportToCsv(const WString& sFileName, int nColMask, const WString* pColNames);

  __property SecureWString DefaultUserName =
  { read=m_sDefaultUserName, write=m_sDefaultUserName };

  __property SecureWString PasswFormatSeq =
  { read=m_sPasswFormatSeq, write=m_sPasswFormatSeq };

  __property word32 DefaultPasswExpiryDays =
  { read=m_lDefaultPasswExpiryDays, write=m_lDefaultPasswExpiryDays };

  __property word32 Size =
  { read=GetSize };

  __property int Version =
  { read=m_nLastVersion };

  __property word8 CipherType =
  { read=m_bCipherType, write=SetCipherType };

  __property word32 KdfIterations =
  { read=m_lKdfIterations, write=SetKdfIterations };
};


#endif
