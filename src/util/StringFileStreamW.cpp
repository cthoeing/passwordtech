// StringFileStreamW.cpp
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
#include <vcl.h>
#pragma hdrstop

#include "StringFileStreamW.h"
#include "types.h"
#include "Language.h"
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
  : TFileStream(sFileName, wMode), m_enc(enc), m_buf(nBufSize + nBufSize % 2 + 2),
    m_nBufLen(0), m_nBufPos(0), m_asSepChars(asSepChars), m_sSepChars(asSepChars),
    m_nBOMLen(0)
{
  if (blWriteOrAutodetectBOM) {
    if (wMode == fmOpenRead || wMode == fmOpenReadWrite) {
      m_enc = ceAnsi;
      word8 bom[3] = {0,0,0};
      if (Read(bom, 3) >= 2) {
        if (memcmp(bom, &UNICODE_BOM, 2) == 0) {
          m_enc = ceUtf16;
          m_nBOMLen = 2;
        }
        else if (memcmp(bom, &UNICODE_BOM_SWAPPED, 2) == 0) {
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
        Write(&UNICODE_BOM, 2);
        m_nBOMLen = 2;
        break;
      case ceUtf16BigEndian:
        Write(&UNICODE_BOM_SWAPPED, 2);
        m_nBOMLen = 2;
        break;
      case ceUtf8:
        Write(UTF8_BOM, 3);
        m_nBOMLen = 3;
        break;
      }
    }
  }
  m_nCodeUnitSize = (m_enc == ceAnsi || m_enc == ceUtf8) ? 1 : 2;
}
//---------------------------------------------------------------------------
int __fastcall TStringFileStreamW::ReadString(wchar_t* pwszDest,
  int nDestBufSize)
{
  wchar_t* pwszBuf = reinterpret_cast<wchar_t*>(m_buf.Data());

  while (true) {
    if (m_nBufPos < m_nBufLen) {
      int nStrBytes;
      if (m_nCodeUnitSize == 1)
        nStrBytes = strcspn(&m_buf[m_nBufPos], m_asSepChars.c_str());
      else
        nStrBytes = wcscspn(&pwszBuf[m_nBufPos/2], m_sSepChars.c_str()) * 2;

      bool blInBuf = nStrBytes < m_nBufLen - m_nBufPos;

      if (blInBuf || Position == Size) {
        if (blInBuf)
          nStrBytes += m_nCodeUnitSize;

        int nResult;

        if (m_enc == ceAnsi || m_enc == ceUtf8)
          nResult = MultiByteToWideChar((m_enc == ceAnsi) ? CP_ACP : CP_UTF8,
              0, &m_buf[m_nBufPos], nStrBytes, pwszDest, nDestBufSize - 1);
        else {
          wcsncpy(pwszDest, &pwszBuf[m_nBufPos/2], nDestBufSize - 1);
          nResult = std::min(nStrBytes/2, nDestBufSize - 1);
        }

        m_nBufPos += nStrBytes;

        if (nResult == 0)
          throw EStringFileStreamError(
            TRL("Invalid ANSI or UTF-8 character encoding, or Unicode string too long"));

        pwszDest[nResult] = '\0';

        return nResult;
      }
    }

    if (m_nBufPos == 0 && m_nBufLen != 0)
      throw EStringFileStreamError(TRL("Unicode string too long"));

    Seek(m_nBufPos - m_nBufLen, soFromCurrent);

    int nBytesRead = Read(m_buf, m_buf.Size() - 2);

    if (nBytesRead == 0)
      return 0;

    if (m_nCodeUnitSize == 2 && nBytesRead % 2 != 0)
      throw EStringFileStreamError(TRL("Invalid UTF-16 character encoding"));

    if (m_enc == ceUtf16BigEndian)
      swapUtf16ByteOrder(pwszBuf, nBytesRead/2);

    m_buf[nBytesRead] = '\0';
    m_buf[nBytesRead+1] = '\0';

    m_nBufLen = nBytesRead;
    m_nBufPos = 0;
  }
}
//---------------------------------------------------------------------------
bool __fastcall TStringFileStreamW::WriteString(const wchar_t* pwszSrc,
  int nStrLen,
  int& nBytesWritten)
{
  if (nStrLen == 0) {
    nBytesWritten = 0;
    return true;
  }

  SecureAnsiString sEncBuf;
  int nEncBytes;

  switch (m_enc) {
  case ceAnsi:
  case ceUtf8:

    nEncBytes = WideCharToMultiByte((m_enc == ceAnsi) ? CP_ACP : CP_UTF8,
        0, pwszSrc, nStrLen, NULL, 0, NULL, NULL);

    if (nEncBytes == 0)
      throw EStringFileStreamError(TRL("Error while encoding Unicode string"));

    sEncBuf.New(nEncBytes);

    WideCharToMultiByte((m_enc == ceAnsi) ? CP_ACP : CP_UTF8, 0, pwszSrc, nStrLen,
      sEncBuf, nEncBytes, NULL, NULL);

    break;

  case ceUtf16:
    nEncBytes = nStrLen * 2;

    // write the data directly to the file
    nBytesWritten = Write(pwszSrc, nEncBytes);
    return nBytesWritten == nEncBytes;

  case ceUtf16BigEndian:
    nEncBytes = nStrLen * 2;
    sEncBuf.Assign(reinterpret_cast<const char*>(pwszSrc), nEncBytes);
    swapUtf16ByteOrder(reinterpret_cast<wchar_t*>(sEncBuf.Data()), nStrLen);
    break;
  }

  nBytesWritten = Write(sEncBuf, nEncBytes);

  return nBytesWritten == nEncBytes;
}
//---------------------------------------------------------------------------
