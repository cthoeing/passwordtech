// StringFileStreamW.cpp
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
#pragma hdrstop

#include "StringFileStreamW.h"
#include "types.h"
#include "Language.h"
#include "Util.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


inline void swapUtf16ByteOrder(wchar_t* pwszBuf, int nLen)
{
  for (int i = 0; i < nLen; i++)
    pwszBuf[i] = (pwszBuf[i] << 8) | (pwszBuf[i] >> 8);
}


//---------------------------------------------------------------------------
__fastcall TStringFileStreamW::TStringFileStreamW(const WString& sFileName,
  Word wMode,
  CharacterEncoding enc,
  bool blWriteOrAutodetectBOM,
  int nBufSize,
  const AnsiString& asSepChars)
  : TFileStream(sFileName, wMode), m_enc(enc), m_nBufLen(0), m_nBufPos(0),
    m_asSepChars(asSepChars), m_sSepChars(asSepChars), m_nBOMLen(0)
{
  if (blWriteOrAutodetectBOM) {
    if (wMode == fmOpenRead || wMode == fmOpenReadWrite) {
      // only UTF-8 or ANSI allowed if BOM not specified
      if (m_enc != ceUtf8)
        m_enc = ceAnsi;
      word8 bom[3] = {0,0,0};
      if (Read(bom, 3) >= 2) {
        if (memcmp(bom, UNICODE_BOM, 2) == 0) {
          m_enc = ceUtf16;
          m_nBOMLen = 2;
        }
        else if (memcmp(bom, UNICODE_BOM_SWAPPED, 2) == 0) {
          m_enc = ceUtf16BigEndian;
          m_nBOMLen = 2;
        }
        else if (memcmp(bom, UTF8_BOM, 3) == 0) {
          m_enc = ceUtf8;
          m_nBOMLen = 3;
        }
      }
      FileBeginning();
    }
    else {
      switch (m_enc) {
      case ceAnsi:
        break;
      case ceUtf16:
        Write(UNICODE_BOM, 2);
        m_nBOMLen = 2;
        break;
      case ceUtf16BigEndian:
        Write(UNICODE_BOM_SWAPPED, 2);
        m_nBOMLen = 2;
        break;
      case ceUtf8:
        Write(UTF8_BOM, 3);
        m_nBOMLen = 3;
        break;
      }
    }
  }
  if (m_enc == ceAnsi || m_enc == ceUtf8)
    m_cbuf.New(nBufSize);
  else
    m_wbuf.New(nBufSize);
}
//---------------------------------------------------------------------------
int __fastcall TStringFileStreamW::ReadString(wchar_t* pwszDest,
  int nDestBufSize)
{
  if (nDestBufSize < 1)
    return 0;

  const int nCodeUnitSize = (m_enc == ceAnsi || m_enc == ceUtf8) ? 1 : 2;
  while (true) {
    if (m_nBufPos < m_nBufLen) {
      int nStrLen;
      if (nCodeUnitSize == 1)
        nStrLen = strcspn(&m_cbuf[m_nBufPos], m_asSepChars.c_str());
      else
        nStrLen = wcscspn(&m_wbuf[m_nBufPos], m_sSepChars.c_str());

      bool blInBuf = nStrLen < m_nBufLen - m_nBufPos;

      if (blInBuf || Position == Size) {
        if (blInBuf)
          nStrLen++;

        int nResult;

        if (m_enc == ceAnsi || m_enc == ceUtf8) {
          nResult = MultiByteToWideChar((m_enc == ceAnsi) ? CP_ACP : CP_UTF8,
              0, &m_cbuf[m_nBufPos], nStrLen, pwszDest, nDestBufSize - 1);
        }
        else {
          wcsncpy(pwszDest, &m_wbuf[m_nBufPos], nDestBufSize - 1);
          nResult = std::min(nStrLen, nDestBufSize - 1);
        }

        m_nBufPos += nStrLen;

        if (nResult == 0)
          throw EStringFileStreamError(
            TRL("Invalid ANSI or UTF-8 character encoding, or Unicode string too long"));

        pwszDest[nResult] = '\0';

        return nResult;
      }
    }

    if (m_nBufPos == 0 && m_nBufLen != 0)
      throw EStringFileStreamError(TRL("Unicode string too long"));

    Seek((m_nBufPos - m_nBufLen) * nCodeUnitSize, soFromCurrent);

    const int nBytesRead = (nCodeUnitSize == 1) ?
      Read(m_cbuf, static_cast<int>(m_cbuf.Size() - 1)) :
      Read(m_wbuf, static_cast<int>((m_wbuf.Size() - 1) * 2));

    if (nBytesRead == 0)
      return 0;

    if (nCodeUnitSize == 2 && nBytesRead % 2 != 0)
      throw EStringFileStreamError(TRL("Invalid UTF-16 character encoding"));

    m_nBufLen = (nCodeUnitSize == 1) ? nBytesRead : nBytesRead / 2;

    if (m_enc == ceUtf16BigEndian)
      swapUtf16ByteOrder(m_wbuf, m_nBufLen);

    if (nCodeUnitSize == 1)
      m_cbuf[m_nBufLen] = '\0';
    else
      m_wbuf[m_nBufLen] = '\0';

    m_nBufPos = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TStringFileStreamW::WriteString(const wchar_t* pwszSrc,
  int nStrLen)
{
  if (nStrLen < 1)
    return;

  switch (m_enc) {
  case ceAnsi:
  case ceUtf8:
    {
      const int nEncBytes = WideCharToMultiByte(
        (m_enc == ceAnsi) ? CP_ACP : CP_UTF8,
        0, pwszSrc, nStrLen, nullptr, 0, nullptr, nullptr);

      if (nEncBytes == 0)
        throw EStringFileStreamError(TRL("Error while encoding Unicode string"));

      SecureAnsiString sEncBuf(nEncBytes);

      WideCharToMultiByte(
        (m_enc == ceAnsi) ? CP_ACP : CP_UTF8, 0, pwszSrc, nStrLen,
        sEncBuf, nEncBytes, nullptr, nullptr);

      if (Write(sEncBuf, nEncBytes) != nEncBytes)
        OutOfDiskSpaceError();
      break;
    }
  case ceUtf16:
    {
      // write the data directly to the file
      const int nBytes = nStrLen * 2;
      if (Write(pwszSrc, nBytes) != nBytes)
        OutOfDiskSpaceError();
      break;
    }
  case ceUtf16BigEndian:
    {
      SecureWString sEncBuf(pwszSrc, nStrLen);
      swapUtf16ByteOrder(sEncBuf, nStrLen);
      if (Write(sEncBuf, static_cast<int>(sEncBuf.SizeBytes())) != sEncBuf.SizeBytes())
        OutOfDiskSpaceError();
      break;
    }
  }
}
//---------------------------------------------------------------------------
