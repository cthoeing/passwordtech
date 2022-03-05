// MemUtil.h
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
#ifndef MemUtilH
#define MemUtilH
//---------------------------------------------------------------------------
#include <vector>
#include "types.h"

// erases a memory block, i.e., fills it with binary zeros
// (don't use memset() directly, because it might get optimized out of
// existence by some C++ compilers...)
// -> pointer to the memory block
// -> size in bytes
void memzero(void* pMem, size_t size);

// encrypts or decrypts a memory buffer
// -> source buffer
// -> destination buffer
// -> number of bytes to process
// -> 128-bit key
// -> encrypt or decrypt?
void memcrypt_128bit(const word8* pSrc,
  word8* pDest,
  word32 lSize,
  const word8* pKey,
  bool blEncrypt);

// overwrite contents of STL string (std::string etc.)
template<class T> void eraseStlString(T& s)
{
  if (!s.empty()) {
    std::fill(s.begin(), s.end(), 0);
    s.clear();
  }
}

template<class T> void eraseVector(T& v)
{
  if (!v.empty()) {
    memzero(&v[0], v.size() * sizeof(T));
    v.clear();
  }
}

// overwrite contents of VCL string (AnsiString etc.)
template<class T> void eraseVclString(T& s)
{
  if (!s.IsEmpty()) {
    int nLen = s.Length();
    for (int i = 1; i <= nLen; i++)
      s[i] = 'x';
    s = T();
  }
}

#endif
