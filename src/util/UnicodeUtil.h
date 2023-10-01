// UnicodeUtil.h
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
#ifndef UnicodeUtilH
#define UnicodeUtilH
//---------------------------------------------------------------------------
#include <string>
#include <System.hpp>
#include <SysUtils.hpp>
#include "SecureMem.h"

// new VCL's unicode string
typedef UnicodeString WString;

// w32string (STL) stores Unicode characters as single 32-bit units
// NOTE: the underlying 32-bit encoding is NOT compatible to UTF-32;
// it is a special but simple encoding to allow fast conversion to UTF-16
typedef std::basic_string<word32> w32string; // basic_string is STL -> lower-case

const wchar_t CRLF[] = L"\r\n";
const word8 UNICODE_BOM[2] = { 0xFF, 0xFE };
const word8 UNICODE_BOM_SWAPPED[2] = { 0xFE, 0xFF };
const word8 UTF8_BOM[3] = { 0xEF, 0xBB, 0xBF };

class EUnicodeError : public Exception
{
public:

  __fastcall EUnicodeError(const WString& sMsg)
    : Exception(sMsg)
  {
  }

};

enum CharacterEncoding { ceAnsi, ceUtf16, ceUtf16BigEndian, ceUtf8 };

// format wide string (16-bit)
// -> format string
// -> series of arguments
// <- formatted wide string
WString FormatW(const WString sFormat, ...);

// format wide string & store result in secure buffer
// -> format string
// -> series of arguments
// <- resulting formatted string
SecureWString FormatW_Secure(const WString sFormat, ...);

// format wide string using argument list
// -> format string
// -> argument list
// <- formatted wide string
WString FormatW_ArgList(const WString sFormat, va_list arglist);

// count Unicode characters in wide string (16-bit)
// -> pointer to wide string
// <- number of Unicode characters
int GetNumOfUnicodeChars(const wchar_t* pwszStr);

// count UTF-16-encoded characters in wide string (32-bit)
// -> pointer to wide string
// <- number of UTF-16 characters
int GetNumOfUtf16Chars(const word32* pStr);

// determine length of wide string (32-bit)
// -> pointer to wide string
// <- string length
size_t w32strlen(const word32* pStr);

// convert 16-bit wide string to 32-bit string
// -> source buffer (16-bit)
// -> dest. buffer (32-bit)
// <- dest. string length
int WCharToW32Char(const wchar_t* pwszSrc, word32* pDest);

// convert 8-bit ASCII string to 32-bit wide string
// -> source buffer (8-bit)
// -> dest. buffer (32-bit)
// <- dest. string length
int AsciiCharToW32Char(const char* pszSrc, word32* pDest);

// convert 16-bit wide string to 32-bit string
// -> source wide string (16-bit)
// <- resulting wide string (32-bit)
w32string WStringToW32String(const WString& sSrc);
w32string WCharToW32String(const wchar_t* pwszSrc);

// convert 32-bit wide string to 16-bit _within_ a given buffer
// this method saves memory and is nevertheless quite fast
// -> buffer containing 32-bit characters to be converted to a 16-bit string
void W32CharToWCharInternal(void* pBuf);

// convert 32-bit wide string to 16-bit string
// -> source wide string (32-bit)
// <- resulting wide string (16-bit)
WString W32StringToWString(const w32string& sSrc);

// convert 8-bit ASCII string to 32-bit wide string
// -> source 8-bit string
// <- resulting wide string (32-bit)
w32string AsciiCharToW32String(const char* pszStr);

// convert 16-bit wide string to UTF-8 characters which can be stored
// in an 8-bit AnsiString
// -> source 16-bit wide string
// <- resulting UTF-8-encoded string
AnsiString WStringToUtf8(const WString& sSrc);

// version for storing result in SecureString
SecureAnsiString WStringToUtf8(const wchar_t* pwszSrc);
//SecureAnsiString WStringToUtf8(const SecureWString& sSrc);

// convert UTF-8 encoded string to wide string (16-bit)
// -> source UTF-8-encoded string
// <- resulting wide string
WString Utf8ToWString(const AnsiString& asSrc);

// version for storing result in SecureString
SecureWString Utf8ToWString(const char* pszSrc);


#endif
