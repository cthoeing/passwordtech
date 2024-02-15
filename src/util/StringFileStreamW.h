// StringFileStreamW.h
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
#ifndef StringFileStreamWH
#define StringFileStreamWH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "SecureMem.h"
#include "UnicodeUtil.h"

// class TStringFileStreamW: allows for reading & writing Unicode text strings
// features:
//   - Unicode transcoding: UTF-16 wide strings are converted to UTF-8 or ANSI
//     depending on the desired character encoding, as specified in the constructor
//     by the user
//   - identification of character encoding by byte-order mark (BOM) at the
//     beginning of the file; when reading from the file, all strings are
//     converted to UTF-16 wide strings
//   - strings are read one-by-one by locating separator characters (e.g.,
//     '\n', '\t', etc.) in the file, similar to the fgets() function of the
//     C standard library

class EStringFileStreamError : public EStreamError
{
public:

  __fastcall EStringFileStreamError(const WString& sMsg)
	: EStreamError(sMsg)
  {
  }
};


class TStringFileStreamW : public TFileStream
{
private:
  CharacterEncoding m_enc;
  SecureMem<char> m_buf;
  int m_nBufLen;
  int m_nBufPos;
  AnsiString m_asSepChars;
  WString m_sSepChars;
  int m_nCodeUnitSize;
  int m_nBOMLen;

public:

  // constructor
  // -> name of the file to be opened/created
  // -> open mode
  // -> desired Unicode character encoding (UTF-16, UTF-8 or ANSI)
  // -> in write mode: 'true' if the byte-order mark (BOM) is to be written to
  //                   the beginning of the file
  //    in read mode:  'true' if encoding of the file is to be auto-detected
  //                   via the BOM (then, the setting of the character encoding
  //                   is ignored)
  // -> size (in bytes) of the internal buffer; must be greater than the
  //    maximum length of the (Unicode) strings contained in the file
  // -> list of those characters which are used to separate single strings
  //    in the file (usually '\n', '\t', etc.); must be known when reading
  //    strings; in write mode, the separator character has to be added manually
  //    to the string
  __fastcall TStringFileStreamW(const WString& sFileName,
    Word wMode,
    CharacterEncoding enc = ceAnsi,
    bool blWriteOrAutodetectBOM = true,
    int nBufSize = 65536,
    const AnsiString& asSepChars = "\n");

  // read single string from file
  // -> pointer to the dest. buffer (wide string)
  // -> size (no. of characters, including '\0') of dest. buffer
  // <- no. of bytes read
  int __fastcall ReadString(wchar_t* pwszDest,
    int nDestBufSize);

  // write string to file
  // -> pointer to the source buffer (wide string)
  // -> string length (no. of characters)
  // -> variable which obtains the number of bytes written to the file
  // <- 'true':  write operation was successful
  //    'false': write error
  bool __fastcall WriteString(const wchar_t* pwszSrc,
    int nStrLen,
    int* pnBytesWritten = nullptr);

  // set file pointer to beginning of file
  void __fastcall FileBeginning(void)
  {
    m_nBufLen = m_nBufPos = 0;
    Seek(m_nBOMLen, soFromBeginning);
  }

  // set file pointer to end of file
  // <- file size minus BOM length
  __int64 __fastcall FileEnd(void)
  {
    return Seek(0, soFromEnd) - m_nBOMLen;
  }

  __property CharacterEncoding CharEncoding =
  { read=m_enc };

  __property int BOMLength =
  { read=m_nBOMLen };
};


#endif
