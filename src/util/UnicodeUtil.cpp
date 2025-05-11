// UnicodeUtil.cpp
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
#include <vector>
#include <stdio.h>
#pragma hdrstop

#include "UnicodeUtil.h"
#include "Language.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//const int FORMAT_MAX_LEN = 1500;

static void formatError(void)
{
  throw EUnicodeError("Error while formatting string");
}

static void utf8EncodeError(void)
{
  throw EUnicodeError("Error while converting string to UTF-8");
}

static void utf8DecodeError(void)
{
  throw EUnicodeError("Error while decoding UTF-8 string");
}

//---------------------------------------------------------------------------
std::wstring FormatW_(const WString& sFormat,
  const std::vector<WString>& args)
{
  std::wstring sDest;
  int nBufSize = sFormat.Length();
  for (const auto& a : args)
    nBufSize += a.Length();
  sDest.reserve(nBufSize);

  for (auto it = sFormat.begin(); it != sFormat.end(); it++) {
    if (*it == '%') {
      if (it+1 != sFormat.end() && *(it+1) == '%') {
        sDest.push_back('%');
        it++;
      }
      else if (it+1 != sFormat.end() && *(it+1) >= '0' && *(it+1) <= '9') {
        it++;
        word32 n = *it - '0';
        if (it+1 != sFormat.end() && *(it+1) >= '0' && *(it+1) <= '9') {
          it++;
          n = n * 10 + *it - '0';
        }
        if (n >= 1 && n <= args.size()) {
          sDest.append(args[n - 1].c_str());
        }
      }
      else
        sDest.push_back(*it);
    }
    else
      sDest.push_back(*it);
  }

  return sDest;
}
//---------------------------------------------------------------------------
WString FormatW(const WString& sFormat,
  const std::vector<WString>& args)
{
  if (sFormat.IsEmpty())
    return WString();

  std::wstring sDest = FormatW_(sFormat, args);
  return WString(sDest.c_str(), sDest.length());
}
//---------------------------------------------------------------------------
SecureWString FormatW_s(const WString& sFormat,
  const std::vector<WString>& args)
{
  if (sFormat.IsEmpty())
    return SecureWString();
  std::wstring sTemp(FormatW_(sFormat, args));
  SecureWString sDest;
  sDest.AssignStr(sTemp.c_str(), sTemp.length());
  eraseStlString(sTemp);
  return sDest;
}
//---------------------------------------------------------------------------
int GetNumOfUnicodeChars(const wchar_t* pwszStr)
{
  int nChars = 0;

  for (; *pwszStr != '\0'; pwszStr++, nChars++) {
    //if (word32(*pwszStr - 0xD800) <= 0x3FF)
    if (*pwszStr >= 0xd800 && *pwszStr <= 0xdbff)
      pwszStr++;
  }

  return nChars;
}
//---------------------------------------------------------------------------
int GetNumOfUtf16Chars(const word32* pStr)
{
  int nChars = 0;

  for (; *pStr != '\0'; pStr++, nChars++) {
    if (*pStr > 0xffff)
      nChars++;
  }

  return nChars;
}
//---------------------------------------------------------------------------
int WCharToW32Char(const wchar_t* pwszSrc, word32* pDest)
{
  const word32* pDestStart = pDest;

  while (*pwszSrc != '\0') {
    if (pwszSrc[0] >= 0xd800 && pwszSrc[0] <= 0xdbff) {
      if (pwszSrc[1] >= 0xdc00 && pwszSrc[1] <= 0xdfff)
        *pDest++ = (pwszSrc[1] << 16) | pwszSrc[0];
      else
        throw EUnicodeError("Invalid UTF-16 character encoding");
      pwszSrc += 2;
    }
    else
      *pDest++ = *pwszSrc++;
  }

  *pDest = '\0';

  return pDest - pDestStart;
}
//---------------------------------------------------------------------------
int AsciiCharToW32Char(const char* pszSrc, word32* pDest)
{
  const char* pszSrcStart = pszSrc;

  //for (nLen = 0; pszSrc[nLen] != '\0'; nLen++)
  //  pDest[nLen] = pszSrc[nLen];
  while (*pszSrc != '\0')
    *pDest++ = *pszSrc++;

  *pDest = '\0';

  return pszSrc - pszSrcStart;
}
//---------------------------------------------------------------------------
w32string WStringToW32String(const WString& sSrc)
{
  if (sSrc.IsEmpty())
    return w32string();

  // The following code presumes that the STL string implementation
  // uses a contiguous block of memory which is always null-terminated!
  w32string sDest(GetNumOfUnicodeChars(sSrc.c_str()), 0);

  WCharToW32Char(sSrc.c_str(), &sDest[0]);

  return sDest;
}
//---------------------------------------------------------------------------
w32string WCharToW32String(const wchar_t* pwszSrc)
{
  if (*pwszSrc == '\0')
    return w32string();

  w32string sDest(GetNumOfUnicodeChars(pwszSrc), 0);

  WCharToW32Char(pwszSrc, &sDest[0]);

  return sDest;
}
//---------------------------------------------------------------------------
void W32CharToWCharInternal(void* pBuf)
{
  word32* pBuf32 = reinterpret_cast<word32*>(pBuf);
  wchar_t* pBuf16 = reinterpret_cast<wchar_t*>(pBuf);

  for (; *pBuf32 != '\0'; pBuf32++) {
    *pBuf16++ = *pBuf32;
    if (*pBuf32 > 0xffff)
      *pBuf16++ = *pBuf32 >> 16;
  }

  *pBuf16 = '\0';
}
//---------------------------------------------------------------------------
WString W32StringToWString(const w32string& sSrc)
{
  if (sSrc.empty())
    return WString();

  WString sDest;
  const word32* pSrc = sSrc.c_str();
  sDest.SetLength(GetNumOfUtf16Chars(pSrc));

  for (wchar_t* pwszDest = sDest.FirstChar(); *pSrc != '\0'; pSrc++) {
    *pwszDest++ = *pSrc;
    if (*pSrc > 0xffff)
      *pwszDest++ = *pSrc >> 16;
  }

  return sDest;
}
//---------------------------------------------------------------------------
w32string AsciiCharToW32String(const char* pszStr)
{
  int nStrLen = strlen(pszStr);
  if (nStrLen == 0)
    return w32string();

  w32string sDest;
  sDest.reserve(nStrLen);

  while (*pszStr != '\0')
    sDest.push_back(*pszStr++);

  return sDest;
}
//---------------------------------------------------------------------------
AnsiString WStringToUtf8(const WString& sSrc)
{
  if (sSrc.IsEmpty())
    return AnsiString();

  int nDestLen = WideCharToMultiByte(CP_UTF8, 0, sSrc.c_str(), -1,
    nullptr, 0, nullptr, nullptr);
  if (nDestLen == 0)
    utf8EncodeError();

  AnsiString asDest;
  asDest.SetLength(nDestLen - 1);

  WideCharToMultiByte(CP_UTF8, 0, sSrc.c_str(), -1, &asDest[1], nDestLen,
    nullptr, nullptr);

  return asDest;
}
//---------------------------------------------------------------------------
SecureAnsiString WStringToUtf8_s(const wchar_t* pwszSrc)
{
  if (pwszSrc == nullptr || *pwszSrc == '\0')
    return SecureAnsiString();

  int nLen = WideCharToMultiByte(CP_UTF8, 0, pwszSrc, -1, nullptr, 0,
    nullptr, nullptr);
  if (nLen == 0)
    utf8EncodeError();

  SecureAnsiString asDest(nLen);

  WideCharToMultiByte(CP_UTF8, 0, pwszSrc, -1, asDest, nLen, nullptr, nullptr);
  return asDest;
}
//---------------------------------------------------------------------------
WString Utf8ToWString(const AnsiString& asSrc)
{
  if (asSrc.IsEmpty())
    return WString();

  int nDestLen = MultiByteToWideChar(CP_UTF8, 0, asSrc.c_str(), -1, nullptr, 0);
  if (nDestLen == 0)
    utf8DecodeError();

  WString sDest;
  sDest.SetLength(nDestLen - 1);

  MultiByteToWideChar(CP_UTF8, 0, asSrc.c_str(), -1, sDest.FirstChar(), nDestLen);

  return sDest;
}
//---------------------------------------------------------------------------
SecureWString Utf8ToWString_s(const char* pszSrc)
{
  if (pszSrc == nullptr || *pszSrc == '\0')
    return SecureWString();

  int nLen = MultiByteToWideChar(CP_UTF8, 0, pszSrc, -1, nullptr, 0);
  if (nLen == 0)
    utf8DecodeError();

  SecureWString sDest(nLen);

  MultiByteToWideChar(CP_UTF8, 0, pszSrc, -1, sDest, nLen);
  return sDest;
}
//---------------------------------------------------------------------------
size_t w32strlen(const word32* pStr)
{
  const word32* pStart = pStr;
  while (*pStr != '\0') pStr++;
  return static_cast<size_t>(pStr - pStart);
}
//---------------------------------------------------------------------------
const wchar_t WHITESPACE_CHARS[] = L" \n\r\t";

std::wstring TrimWString(const std::wstring& s)
{
  if (s.empty())
    return s;

  auto pos1 = s.find_first_not_of(WHITESPACE_CHARS);
  if (pos1 == s.npos)
    return std::wstring();

  auto pos2 = s.find_last_not_of(WHITESPACE_CHARS);

  return s.substr(pos1, pos2 - pos1 + 1);
}
//---------------------------------------------------------------------------

