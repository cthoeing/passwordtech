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
#include "sha512.h"

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
    ModificationTimeString = TimeStampToString(ModificationTime);
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
  static SecureWString TimeStampToString(const FILETIME& ft);

  // convert expiry date to string
  static SecureWString ExpiryDateToString(word32 lDate);

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
      CreationTimeString = TimeStampToString(CreationTime);
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
public:
  __fastcall EPasswDbInvalidFormat(const WString& sMsg)
    : EPasswDbError(sMsg)
  {}
};

// db not encrypted, invalid key, or authentication error (HMAC mismatch)
class EPasswDbInvalidKey : public EPasswDbError
{
public:
  __fastcall EPasswDbInvalidKey(const WString& sMsg)
    : EPasswDbError(sMsg)
  {}
};

const WString E_INVALID_FORMAT = "Invalid/unknown file format";

class PasswDatabase {
private:
  enum class DbOpenState {
    Closed,
    Incomplete, // database could not be opened successfully
    Open        // database successfully opened
  };

  enum {
    SECMEM_SIZE = 4096,
    SECMEM_KEY_LENGTH = 32,
    SECMEM_SALT_LENGTH = 16,
    SECMEM_IV_LENGTH = 8,

    DB_KEY_LENGTH = 32,
    DB_SALT_LENGTH = 32,
    SHA256_HMAC_LENGTH = 32,
    SHA512_HMAC_LENGTH = 64,

    DB_RECOVERY_KEY_BLOCK_LENGTH = 2 * (DB_SALT_LENGTH + DB_KEY_LENGTH),

    HASH_SHA256 = 0,
    HASH_SHA512 = 1,

    KDF_PBKDF2_SHA256 = 0,

    IO_BUF_SIZE = 65536
  };

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
  word8* m_pDbRecoveryKeyBlock;
  std::unique_ptr<EncryptionAlgorithm::SymmetricCipher> m_dbCipher;
  SecureMem<sha512_context> m_dbHashCtx;
  SecureMem<word8> m_cryptBuf;
  word32 m_lCryptBufPos;
  std::unique_ptr<TFileStream> m_pFile;
  SecureWString m_sDefaultUserName;
  SecureWString m_sPasswFormatSeq;
  word32 m_lDefaultPasswExpiryDays;
  bool m_blPlaintextPassw;
  DbOpenState m_dbOpenState;
  bool m_blRecoveryKey;

  // initializes crypto engine (encryption and hash algorithms),
  // allocates RAM to protect the database master key and passwords
  // -> master key
  void Initialize(const SecureMem<word8>& key);

  // throws exception if database is already open
  void CheckDbNotOpen(void);

  // throw exception if database is not open
  void CheckDbOpen(void) const;

  // creates new instance of a cipher
  // -> cipher type
  // -> pointer to 256-bit key
  // -> cipher mode (encryption/decryption)
  std::unique_ptr<EncryptionAlgorithm::SymmetricCipher> CreateCipher(
    int nType, const word8* pKey, EncryptionAlgorithm::Mode mode);

  // write buffer contents to file
  // -> buffer of any type
  // -> number of bytes to write
  void Write(const void* pBuf, word32 lNumOfBytes);

  // encrypt internal buffer and write data to file
  void Flush(void);

  // write field data (Title, User name, ...) to file
  // -> data buffer
  // -> number of bytes
  // -> index of field (<0: index not applicable)
  void WriteFieldBuf(const void* pBuf, word32 lNumOfBytes, int nIndex = -1);

  // template function for writing field data
  template<typename T> void WriteField(const T& t, int nIndex = -1)
  {
    WriteFieldBuf(&t, sizeof(T), nIndex);
  }

  // template function for writing data type to file
  template<typename T> void WriteType(const T& t)
  {
    Write(&t, sizeof(T));
  }

  // write ASCII (non-Unicode) string to file
  // -> string buffer (zero-terminated)
  // -> index of field (<0: index not applicable)
  void WriteString(const char* pszStr, int nIndex = -1);

  // write Unicode string to file
  // -> string
  // -> index of field (<0: index not applicable)
  void WriteString(const SecureWString& sStr, int nIndex = -1);

  // read index of field
  int ReadFieldIndex(void);

  // read size of field
  word32 ReadFieldSize(void);

  // read ASCII (non-Unicode) string
  SecureAnsiString ReadAnsiString(void);

  // read Unicode string
  SecureWString ReadString(void);

  // template function for reading field of a specific type
  template<typename T> T ReadField(void)
  {
    word32 lSize = ReadFieldSize();
    if (lSize != sizeof(T))
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
    T t;
    memcpy(&t, &m_cryptBuf[m_lCryptBufPos], lSize);
    m_lCryptBufPos += lSize;
    return t;
  }

  // template function for reading a specific data type
  template<typename T> T ReadType(void)
  {
    if (m_lCryptBufPos + sizeof(T) > m_cryptBuf.Size())
      throw EPasswDbInvalidFormat(E_INVALID_FORMAT);
    T t;
    memcpy(&t, &m_cryptBuf[m_lCryptBufPos], sizeof(T));
    m_lCryptBufPos += sizeof(T);
    return t;
  }

  // skip current field
  void SkipField(void);

  // returns version number of last opened/saved database
  int GetLastVersion(void)
  {
    return m_nLastVersion;
  }

  // throws exception if recovery key is set
  void CheckCryptoParam(void)
  {
    if (m_blRecoveryKey)
      throw EPasswDbError("Action not allowed if recovery key is set");
  }

  // changes cipher type - must not be called if recovery key is set
  void SetCipherType(word8 bType)
  {
    CheckCryptoParam();
    if (bType > CIPHER_CHACHA20)
      throw EPasswDbError("Cipher not supported");
    m_bCipherType = bType;
  }

  // changes number of KDF iterations - must not be called if recovery key is set
  void SetKdfIterations(word32 lIter)
  {
    CheckCryptoParam();
    if (lIter == 0)
      throw EPasswDbError("Invalid number of KDF iterations");
    m_lKdfIterations = lIter;
  }

public:

  enum {
    VERSION_HIGH = 1,
    VERSION_LOW = 3,
    VERSION = (VERSION_HIGH << 8) | VERSION_LOW,

    KEY_HASH_ITERATIONS = 16384,

    CIPHER_AES256 = 0,
    CIPHER_CHACHA20 = 1,
  };

  // creation/destruction
  PasswDatabase();
  virtual ~PasswDatabase();

  // creates new database
  // -> master key
  void New(const SecureMem<word8>& key);

  // opens existing database file
  // -> master key
  // -> name of file to be opened
  void Open(const SecureMem<word8>& key,
    const WString& sFileName);

  // closes database, wipes and free memory reserved for crypto engine
  void Close(void);

  // checks whether database is open
  bool IsOpen(void) const
  {
    return m_dbOpenState == DbOpenState::Open;
  }

  // saves database to file
  void SaveToFile(const WString& sFileName);

  // release write protection of opened database file
  void ReleaseFile(void)
  {
    m_pFile.reset();
  }

  // gets list of database entries
  const PasswDbList& GetDatabase(void) const
  {
    return m_db;
  }

  // gets number of database entries
  word32 GetSize(void) const
  {
    return m_db.size();
  }

  // adds new entry to database
  // -> whether to set timestamps automatically
  // -> whether to set user name to default name
  PasswDbEntry* AddDbEntry(bool blSetTimeStamp = true,
    bool blSetDefUserName = true);

  // removes entry from database
  void DeleteDbEntry(PasswDbEntry& pEntry);

  // changes position of an entry within the database
  // -> current position
  // -> desired new position
  void MoveDbEntry(word32 lCurrPos, word32 lNewPos);

  // changes password of a given database entry
  // -> database entry
  // <- password
  void SetDbEntryPassw(PasswDbEntry& pEntry,
    const SecureWString& sPassw);

  // returns password of database entry
  // -> database entry
  SecureWString GetDbEntryPassw(const PasswDbEntry& pEntry);

  // determines whether passwords of all entries are stored in plaintext
  // or ciphertext format in memory (encrypted with a key stored in RAM)
  void SetPlaintextPassw(bool blPlaintextPassw);

  // checks validity of supplied master key
  // <- true/false: key valid/invalid
  bool CheckMasterKey(const SecureMem<word8>& key);

  // checks validity of supplied recovery key
  // <- true/false: key valid/invalid
  bool CheckRecoveryKey(const SecureMem<word8>& recoveryKey);

  // changes master key
  // -> new master key
  void ChangeMasterKey(const SecureMem<word8>& newKey);

  // sets recovery key
  // -> master key (new or old)
  // -> recovery key
  void SetRecoveryKey(const SecureMem<word8>& key,
    const SecureMem<word8>& recoveryKey);

  // removes/unsets recovery key
  // -> new master key
  void RemoveRecoveryKey(const SecureMem<word8>& key);

  // exports database entries to CSV format (plaintext)
  // -> name of CSV file
  // -> fields/columns to export (bit field)
  // -> names of fields/columns
  void ExportToCsv(const WString& sFileName, int nColMask, const WString* pColNames);

  // creates 256-bit key file, key stored in hexadecimal format (64 bytes)
  // -> name of key file
  static void CreateKeyFile(const WString& sFileName);

  // reads key from file and generates 256-bit key - treatment depends on
  // file format:
  // 1) file size = 32 bytes: contents are read as-is
  // 2) file size = 64 bytes and hexadecimal format: hexadecimal values are
  //    converted into bytes (32 hex pairs converted into 32 bytes)
  // 3) in all other cases, contents are hashed into a 256-bit key using SHA-256
  // <- 256-bit key
  static SecureMem<word8> GetKeyFromKeyFile(const WString& sFileName);

  // appends key from key file to password and returns resulting key combination.
  // If only one key source is provided, the result is identical to that.
  static SecureMem<word8> CombineKeySources(const SecureMem<word8>& passw,
    const WString& sKeyFileName);

  // properties
  // default user name
  __property SecureWString DefaultUserName =
  { read=m_sDefaultUserName, write=m_sDefaultUserName };

  // default password format sequence
  __property SecureWString PasswFormatSeq =
  { read=m_sPasswFormatSeq, write=m_sPasswFormatSeq };

  // default number of expiry days for new passwords
  __property word32 DefaultPasswExpiryDays =
  { read=m_lDefaultPasswExpiryDays, write=m_lDefaultPasswExpiryDays };

  // database size
  __property word32 Size =
  { read=GetSize };

  // version number of last opened/saved database
  __property int Version =
  { read=m_nLastVersion };

  // cipher type
  __property word8 CipherType =
  { read=m_bCipherType, write=SetCipherType };

  // number of KDF iterations
  __property word32 KdfIterations =
  { read=m_lKdfIterations, write=SetKdfIterations };

  // recovery key enabled/disabled (true/false)
  __property bool HasRecoveryKey =
  { read=m_blRecoveryKey };
};


#endif
