// SecureMem.h
//
// PASSWORD TECH
// Copyright (c) 2002-2023 by Christian Thoeing <c.thoeing@web.de>
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
#ifndef SecureMemH
#define SecureMemH
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstring>
#include <cwchar>
#include "types.h"
#include "MemUtil.h"

class SecureMemError : public std::runtime_error
{
public:
  SecureMemError(const char* pMsg)
    : std::runtime_error(pMsg)
  {}
};

class SecureMemSizeError : public SecureMemError
{
public:
  SecureMemSizeError(const char* pMsg = nullptr)
    : SecureMemError(pMsg ? pMsg : "SecureMem: Size exceeds limit")
  {}
};


// class for secure memory operations, fast implementation
// inspired by secblock.h in the Crypto++ package by Wei Dai

template <class T>
class SecureMem
{
private:
  // members
  T* m_pData;
  word32 m_lSize;

  static word32 _tcslen(const T* pStr);

public:

  static const word32 npos = -1;

  static word32 max_size()
  {
    return 0xfffffffe / sizeof(T);
  }

  // constructor
  SecureMem()
    : m_pData(nullptr), m_lSize(0)
  {
  }

  // constructor
  // -> size of the data array
  SecureMem(word32 lSize)
    : m_pData(nullptr), m_lSize(lSize)
  {
    if (m_lSize != 0) {
      if (m_lSize > max_size())
        throw SecureMemSizeError();
      m_pData = new T[m_lSize];
    }
  }

  // constructor
  // -> pointer to the data
  // -> size of this data
  SecureMem(const T* pData,
    word32 lSize)
    : m_pData(nullptr), m_lSize(lSize)
  {
    if (m_lSize != 0) {
      if (m_lSize > max_size())
        throw SecureMemSizeError();
      m_pData = new T[m_lSize];
      memcpy(m_pData, pData, SizeBytes());
    }
  }

  // copy constructor
  // -> object instance
  SecureMem(const SecureMem& src)
    : m_pData(nullptr), m_lSize(src.m_lSize)
  {
    if (m_lSize != 0) {
      m_pData = new T[m_lSize];
      memcpy(m_pData, src.m_pData, SizeBytes());
    }
  }

  // move constructor
  SecureMem(SecureMem&& src)
    : m_pData(src.m_pData), m_lSize(src.m_lSize)
  {
    src.m_pData = nullptr;
    src.m_lSize = 0;
  }

  // destructor
  ~SecureMem()
  {
    Clear();
  }

  // empties the object and securely wipes the data array
  void Clear(void)
  {
    if (!IsEmpty())
    {
      Zeroize();
      delete [] m_pData;
      m_pData = nullptr;
      m_lSize = 0;
    }
  }

  // fills the data array with binary zeros
  void Zeroize(void)
  {
    if (!IsEmpty())
      memzero(m_pData, SizeBytes());
  }

  // copies pSrc to the data array
  // -> pointer to the source array
  // -> size of the array
  void Assign(const T* pSrc,
    word32 lSize)
  {
    New(lSize);
    if (lSize != 0)
      memcpy(m_pData, pSrc, lSize * sizeof(T));
  }

  // copies string into data array and adds terminating zero
  // -> pointer to string to copy
  // -> string length/number of characters to copy
  void AssignStr(const T* pSrc,
    word32 lStrLen)
  {
    NewStr(lStrLen);
    if (lStrLen != 0)
      memcpy(m_pData, pSrc, lStrLen * sizeof(T));
  }

  // copies string into data array, including terminating zero
  // -> pointer to zero-terminated string
  void AssignStr(const T* pSrc)
  {
    return AssignStr(pSrc, _tcslen(pSrc));
  }

  void Swap(SecureMem& other)
  {
    std::swap(m_pData, other.m_pData);
    std::swap(m_lSize, other.m_lSize);
  }

  // resizes the data array (see below for the implementation)
  // -> new size
  // -> true  : preserve the old data
  //    false : just create a new array
  void Resize(word32 lNewSize,
    bool blPreserve = true);

  // resizes the data array to hold a zero-terminated string
  // -> string length (without terminating zero)
  // -> preserve or discard existing data
  void ResizeStr(word32 lStrLen,
    bool blPreserve = true)
  {
    if (lStrLen != 0) {
      Resize(lStrLen + 1, blPreserve);
      back() = '\0';
    }
    else
      Clear();
  }

  // creates a new data array of the given size
  // existing data will NOT be preserved
  // -> new size of the data array
  void New(word32 lNewSize)
  {
    Resize(lNewSize, false);
  }

  // creates new data array for holding a zero-terminated string
  // -> string length (without terminating zero)
  void NewStr(word32 lStrLen)
  {
    ResizeStr(lStrLen, false);
  }

  // expands the data array (contents will be preserved)
  // -> new size
  void Grow(word32 lNewSize)
  {
    if (lNewSize > m_lSize)
      Resize(lNewSize, true);
  }

  // expands the array by a specific amount
  // -> number of elements to add
  void GrowBy(word32 lAddSize)
  {
    if (lAddSize > 0)
      Resize(m_lSize + lAddSize, true);
  }

  // expands the array to a size corresponding to the next higher power of 2
  // to reduce number of reallocations if the array grows continuously
  // -> new minimum size
  void BufferedGrow(word32 lNewSize)
  {
    if (lNewSize > m_lSize) {
      if (lNewSize > max_size())
        throw SecureMemSizeError();
      word32 lNewSizeLog = __builtin_clz(lNewSize) ^ 31;
      lNewSize = (lNewSizeLog < 31) ? std::min(max_size(),
        1u << (lNewSizeLog + 1)) : 0xfffffffe;
      Resize(lNewSize, true);
    }
  }

  // shrinks the array (contents will be preserved)
  // -> new size
  void Shrink(word32 lNewSize)
  {
    if (lNewSize < m_lSize)
      Resize(lNewSize, true);
  }

  // shrinks the array by a specific amount
  // -> number of elements by which to shrink
  void ShrinkBy(word32 lSubSize)
  {
    if (lSubSize > 0 && lSubSize <= m_lSize)
      Resize(m_lSize - lSubSize, true);
  }

  // returns the data pointer
  // <- pointer to the secure memory block
  const T* Data(void) const
  {
    return m_pData;
  }

  T* Data(void)
  {
    return m_pData;
  }

  const word8* Bytes(void) const
  {
    return reinterpret_cast<const word8*>(m_pData);
  }

  word8* Bytes(void)
  {
    return reinterpret_cast<word8*>(m_pData);
  }


  // returns the data size
  // <- size of the secured data
  word32 Size(void) const
  {
    return m_lSize;
  }

  // returns data size in bytes
  // <- number of bytes
  word32 SizeBytes(void) const
  {
    return m_lSize * sizeof(T);
  }

  // returns string length
  // (size without terminating zero)
  // <- string length
  word32 StrLen(void) const;

  // returns string length in bytes
  // <- string length as number of bytes
  word32 StrLenBytes(void) const
  {
    return StrLen() * sizeof(T);
  }

  // checks whether the data array is empty
  bool IsEmpty(void) const
  {
    return m_lSize == 0;
  }

  // checks whether the string is empty
  bool IsStrEmpty(void) const
  {
    return m_lSize < 2 || m_pData[0] == '\0';
  }

  // return C-style string
  // ensure that a valid pointer is returned even if the container is empty
  const T* c_str(void) const
  {
    if (IsEmpty()) {
      static const T t = 0;
      return &t;
    }
    return m_pData;
  }

  // search container for the first occurrence of an element
  // -> element to search for
  // -> index at which search is started
  // -> number of elements to search (npos = search until end)
  // <- index of first element (npos = element not found)
  word32 Find(const T& element,
    word32 lStart = 0,
    word32 lLen = npos) const;

  // append another string to this string
  // -> pointer to string to append (must be zero-terminated if length
  //    is not specified)
  // -> length of string to append (-1 = determine string length)
  // -> position/index at which string is to be appended
  //    (-1 = append at the end of this string);
  //    receives new string length
  void StrCat(const T* pStr,
    word32 lLen,
    word32& lPos);

  void StrCat(const SecureMem<T>& src,
    word32& lPos)
  {
    return StrCat(src.m_pData, src.StrLen(), lPos);
  }

  // begin/end functions to enable range-based for loops
  T* begin(void)
  {
    return m_pData;
  }

  T* end(void)
  {
    return m_pData + m_lSize;
  }

  const T* begin(void) const
  {
    return m_pData;
  }

  const T* end(void) const
  {
    return m_pData + m_lSize;
  }


  // front/back: Access first/last element as a reference
  T& front(void)
  {
    if (IsEmpty())
      throw SecureMemError("SecureMem::front(): Array is empty");
    return m_pData[0];
  }

  const T& front(void) const
  {
    if (IsEmpty())
      throw SecureMemError("SecureMem::front(): Array is empty");
    return m_pData[0];
  }

  T& back(void)
  {
    if (IsEmpty())
      throw SecureMemError("SecureMem::back(): Array is empty");
    return m_pData[m_lSize - 1];
  }

  const T& back(void) const
  {
    if (IsEmpty())
      throw SecureMemError("SecureMem::back(): Array is empty");
    return m_pData[m_lSize - 1];
  }


  // operators

  operator const void* (void) const
  {
    return m_pData;
  }

  operator void* (void)
  {
    return m_pData;
  }

  operator const T* (void) const
  {
    return m_pData;
  }

  operator T* (void)
  {
    return m_pData;
  }

  const T* operator+ (word32 lOffset) const
  {
    return m_pData + lOffset;
  }

  T* operator+ (word32 lOffset)
  {
    return m_pData + lOffset;
  }

  T& operator[] (word32 lIndex)
  {
    return m_pData[lIndex];
  }

  const T& operator[] (word32 lIndex) const
  {
    return m_pData[lIndex];
  }

  T& operator[] (int nIndex)
  {
    return m_pData[nIndex];
  }

  const T& operator[] (int nIndex) const
  {
    return m_pData[nIndex];
  }

  SecureMem& operator= (const SecureMem& src)
  {
    if (this != &src) {
      Assign(src.m_pData, src.m_lSize);
	}
    return *this;
  }

  SecureMem& operator= (SecureMem&& src)
  {
    if (this != &src) {
      Swap(src);
    }
    return *this;
  }

  SecureMem& operator+= (const SecureMem& src)
  {
    if (!src.IsEmpty()) {
      word32 lOldSize = m_lSize;
      Grow(m_lSize + src.m_lSize);
      memcpy(m_pData + lOldSize, src.m_pData, src.SizeBytes());
    }
    return *this;
  }

  friend SecureMem operator+ (SecureMem& a, const SecureMem& b)
  {
    return a += b;
  }
};

template <class T>
void SecureMem<T>::Resize(word32 lNewSize,
  bool blPreserve)
{
  if (lNewSize == 0)
    Clear();
  else if (lNewSize != m_lSize)
  {
    if (lNewSize > max_size())
        throw SecureMemSizeError();

    T* pNewData = new T[lNewSize];

    if (blPreserve && !IsEmpty())
      memcpy(pNewData, m_pData, std::min(m_lSize, lNewSize) * sizeof(T));

    Clear();

    m_pData = pNewData;
    m_lSize = lNewSize;
  }
}

template<class T>
word32 SecureMem<T>::Find(const T& element,
  word32 lStart,
  word32 lLen) const
{
  if (IsEmpty() || lLen == 0 || lStart >= m_lSize)
    return npos;
  if (lLen == npos)
    lLen = m_lSize;
  lLen = std::min(m_lSize - lStart, lLen);
  while (lLen--) {
    if (m_pData[lStart] == element)
      return lStart;
    lStart++;
  }
  return npos;
}

template<class T>
word32 SecureMem<T>::_tcslen(const T* pStr)
{
  if constexpr(std::is_same<T, char>::value)
    return strlen(pStr);
  if constexpr(std::is_same<T, wchar_t>::value)
    return wcslen(pStr);

  const T* pEnd = pStr;
  while (*pEnd != '\0') pEnd++;
  return static_cast<word32>(pEnd - pStr);
}

template<class T>
word32 SecureMem<T>::StrLen(void) const
{
  if (m_lSize < 2)
    return 0;

  word32 lStrLen = 0;
  if constexpr(std::is_same<T, char>::value)
    lStrLen = strnlen_s(m_pData, m_lSize);
  else if constexpr(std::is_same<T, wchar_t>::value)
    lStrLen = wcsnlen_s(m_pData, m_lSize);
  else {
    for ( ; lStrLen < m_lSize && m_pData[lStrLen] != '\0'; lStrLen++);
  }

  if (lStrLen >= m_lSize)
    throw SecureMemError("SecureMem::StrLen() overflow");

  return lStrLen;
}

template<class T>
void SecureMem<T>::StrCat(const T* pStr, word32 lLen, word32& lPos)
{
  if (lLen == npos)
    lLen = _tcslen(pStr);
  if (lPos == npos)
    lPos = StrLen();
  else if (lPos != 0 && lPos >= m_lSize)
    throw SecureMemError("SecureMem::StrCat(): Invalid position");
  if (lLen == 0)
    return;
  BufferedGrow(lPos + lLen + 1);
  memcpy(m_pData + lPos, pStr, lLen * sizeof(T));
  lPos += lLen;
  m_pData[lPos] = '\0';
}

template<class T> inline bool operator== (const SecureMem<T>& a,
  const SecureMem<T>& b)
{
  // avoid applying memcmp() to null pointers
  if (a.IsEmpty() && b.IsEmpty())
    return true;
  return a.Size() == b.Size() &&
    memcmp(a.Data(), b.Data(), a.SizeBytes()) == 0;
}

template<class T> inline bool operator!= (const SecureMem<T>& a,
  const SecureMem<T>& b)
{
  return !(a == b);
}

template<class T> inline bool operator< (const SecureMem<T>& a,
  const SecureMem<T>& b)
{
  // only call memcmp() if both data pointers are valid
  return (!a.IsEmpty() && a.Size() == b.Size()) ?
    memcmp(a.Data(), b.Data(), a.SizeBytes()) < 0 : a.Size() < b.Size();
}

template<class T> inline bool operator> (const SecureMem<T>& a,
  const SecureMem<T>& b)
{
  return b < a;
}


typedef SecureMem<char> SecureAnsiString;
typedef SecureMem<wchar_t> SecureWString;
typedef SecureMem<word32> SecureW32String;


#include <string_view>

struct SecureStringHashFunction
{
  size_t operator()(const SecureWString& s) const
  {
    return std::hash<std::wstring_view>{}(std::wstring_view(s.c_str()));
  }
  size_t operator()(const SecureAnsiString& s) const
  {
    return std::hash<std::string_view>{}(std::string_view(s.c_str()));
  }
};


#endif
