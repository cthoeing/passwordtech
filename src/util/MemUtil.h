// MemUtil.h
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
#ifndef MemUtilH
#define MemUtilH
//---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <System.hpp>
#include "types.h"

// erases a memory block, i.e., fills it with binary zeros
// (don't use memset() directly, because it might get optimized out of
// existence by some C++ compilers...)
// -> pointer to the memory block
// -> size in bytes
[[clang::optnone]] void memzero(void* pMem, size_t size);

// encrypts/decrypts a memory buffer
// -> source buffer
// -> destination buffer
// -> number of bytes to process
// -> encryption key
// -> size of key in bytes (must be 16 or 32)
void memcrypt(const word8* pSrc,
  word8* pDest,
  word32 lSize,
  const word8* pKey,
  word32 lKeySize);

// overwrite contents of vector and clear it
template<class T> [[clang::optnone]] void eraseVector(std::vector<T>& v)
{
  if (!v.empty()) {
    //v.resize(v.capacity());
    memzero(v.data(), v.size() * sizeof(T));
    v.clear();
  }
}

// overwrite contents of STL string (derived from basic_string) and clear it
template<class T> [[clang::optnone]] void eraseStlString(std::basic_string<T>& s)
{
  if (!s.empty()) {
    //s.resize(s.capacity());
    //memzero(s.data(), s.length() * sizeof(T));
    std::fill(s.begin(), s.end(), 0);
    s.clear();
  }
}

// overwrite contents of AnsiString and clear it
//[[clang::optnone]] void eraseVclString(AnsiString& s);

// overwrite contents of UnicodeString and clear it
[[clang::optnone]] void eraseVclString(UnicodeString& s);

#endif
