// SecureMem.h
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
#ifndef SecureMemH
#define SecureMemH
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstring>
#include "types.h"
#include "MemUtil.h"


// class for secure memory operations, fast implementation
// inspired by secblock.h in the Crypto++ package by Wei Dai

template <class T>
class SecureMem
{
private:
  // members
  T* m_pData;
  word32 m_lSize;

public:

  // constructor
  SecureMem()
    : m_pData(NULL), m_lSize(0)
  {
  }

  // constructor
  // -> size of the data array
  SecureMem(word32 lSize)
    : m_pData(NULL), m_lSize(lSize)
  {
    if (m_lSize != 0) {
      m_pData = new T[m_lSize];
    }
  }

  // constructor
  // -> pointer to the data
  // -> size of this data
  SecureMem(const T* pData,
    word32 lSize)
    : m_pData(NULL), m_lSize(lSize)
  {
    if (m_lSize != 0) {
      m_pData = new T[m_lSize];
      memcpy(m_pData, pData, SizeBytes());
    }
  }

  // copy constructor
  // -> object instance
  SecureMem(const SecureMem& src)
    : m_pData(NULL), m_lSize(src.m_lSize)
  {
    if (m_lSize != 0) {
      m_pData = new T[m_lSize];
      memcpy(m_pData, src.m_pData, SizeBytes());
    }
  }

  // destructor
  ~SecureMem()
  {
    Empty();
  }

  // empties the object and securely wipes the data array
  void Empty(void)
  {
    if (!IsEmpty())
    {
      Clear();
      delete [] m_pData;
      m_pData = NULL;
      m_lSize = 0;
    }
  }

  // clears the data array
  void Clear(void)
  {
    memzero(m_pData, SizeBytes());
  }

  // copies pSrc to the data array
  // -> pointer to the source array
  // -> size of the array
  void Assign(const T* pSrc,
    word32 lSize)
  {
    New(lSize);
    memcpy(m_pData, pSrc, lSize * sizeof(T));
  }

  // copies the data of the source object
  // -> object reference
  void Assign(const SecureMem& src)
  {
    New(src.m_lSize);
    memcpy(m_pData, src.m_pData, src.m_lSize * sizeof(T));
  }

  // resizes the data array (see below for the implementation)
  // -> new size
  // -> true  : preserve the old data
  //    false : just create a new array
  void Resize(word32 lNewSize,
    bool blPreserve = true);

  // creates a new data array of the given size
  // existing data will NOT be preserved
  // -> new size of the data array
  void New(word32 lNewSize)
  {
    Resize(lNewSize, false);
  }

  // creates a new array and copies the data from pData
  // -> pointer to the existing array
  // -> size of the array to be copied
  /*void New(const T* pData,
    word32 lSize)
  {
    Resize(lSize, false);
    memcpy(m_pData, pData, lSize * sizeof(T));
  }*/

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
    return reinterpret_cast<word8*>(m_pData);
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
  word32 StrLen(void) const
  {
    if (m_lSize < 2)
      return 0;

    word32 lStrLen = 0;
    if (std::is_same<T, char>::value || std::is_same<T, word8>::value)
      lStrLen = strlen(reinterpret_cast<const char*>(m_pData));
    else if (std::is_same<T, wchar_t>::value)
      lStrLen = wcslen(reinterpret_cast<const wchar_t*>(m_pData));
    else {
      for ( ; lStrLen < m_lSize && m_pData[lStrLen] != '\0'; lStrLen++);
    }

    if (lStrLen >= m_lSize)
#ifdef _DEBUG
      throw std::runtime_error("SecureMem: StrLen() overflow");
#else
      lStrLen = m_lSize - 1;
#endif

    return lStrLen;
  }

  // returns string length in bytes
  // <- string length as number of bytes
  word32 StrLenBytes(void) const
  {
    return StrLen() * sizeof(T);
  }

  // anything in the data array?
  bool IsEmpty(void) const
  {
    return m_lSize == 0;
  }

  const T* c_str(void) const
  {
    if (IsEmpty()) {
      static const T t = 0;
      return &t;
    }
    return m_pData;
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

  /*operator const word8* (void) const
  {
  return reinterpret_cast<word8*>(m_pData);
  }

  operator word8* (void)
  {
  return reinterpret_cast<word8*>(m_pData);
  }*/

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
    if (this != &src)
      Assign(src);
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
    Empty();
  else if (lNewSize != m_lSize)
  {
    T* pNewData = new T[lNewSize];

    if (blPreserve && !IsEmpty())
      memcpy(pNewData, m_pData, std::min(m_lSize, lNewSize) * sizeof(T));

    Empty();

    m_pData = pNewData;
    m_lSize = lNewSize;
  }
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
  return (!a.IsEmpty() && !b.IsEmpty() && a.Size() == b.Size()) ?
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

#endif
