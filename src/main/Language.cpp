// Language.cpp
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
#include <stdio.h>
#include <memory>
#pragma hdrstop

#include "Language.h"
#include "UnicodeUtil.h"
#include "StringFileStreamW.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

std::unique_ptr<LanguageSupport> g_pLangSupp;


// calculate hash of a string without case sensitivity
// function "sdbm" from http://www.cse.yorku.ca/~oz/hash.htm
// very fast & reliable with respect to collision resistance
// (alternatives: FNV1 (better than FNV1a!), CRC-32; both are a little bit
// better regarding collision resistance, but slower)

template<class T>
static word32 strihash(const T* pStr)
{
  word32 lHash = 0;
  int ch;

  while ((ch = *pStr++) != '\0')
    lHash = toupper(ch) + (lHash << 6) + (lHash << 16) - lHash;

  return lHash;
}

//---------------------------------------------------------------------------
LanguageSupport::LanguageSupport(const WString& sFileName)
{
  std::unique_ptr<TStringFileStreamW> pFile(new TStringFileStreamW(sFileName,
      fmOpenRead));
  static const int MSG_BUF_SIZE = 1024;
  wchar_t wszMsg[MSG_BUF_SIZE];
  wchar_t wszParsed[MSG_BUF_SIZE];
  int nMsgLen, nState = 0;
  word32 lHash;

  while ((nMsgLen = pFile->ReadString(wszMsg, MSG_BUF_SIZE)) > 0) {
    int nI, nParsedLen;

    for (nI = nParsedLen = 0; nI < nMsgLen; nI++) {
      if (wszMsg[nI] == '\\') {
        wchar_t wch = wszMsg[nI+1];
        switch (wch) {
        case 'n':
          wch = '\n';
          break;
        case 'r':
          wch = '\r';
          break;
        case 't':
          wch = '\t';
          break;
        case '\\':
        case '\'':
        case '\"':
          break;
        default:
          continue;
        }
        wszParsed[nParsedLen++] = wch;
        nI++;
      }
      else if (wszMsg[nI] == '/' && wszMsg[nI+1] == '/')
        // comment sequence = "//"
        break;
      else
        wszParsed[nParsedLen++] = wszMsg[nI];
    }

    if (nParsedLen == 0)
      continue;

    wszParsed[nParsedLen] = '\0';

    WString sMsg = Trim(WString(wszParsed));
    if (sMsg.IsEmpty()) {
      nState = 0;
      continue;
    }

    if (nState == 0) {
      lHash = strihash(sMsg.c_str());
      nState = 1;
    }
    else {
      auto ret =
        m_transl.insert(std::pair<word32,std::wstring>(lHash, sMsg.c_str()));
#ifdef _DEBUG
      if (!ret.second) {
        ShowMessage("Collision found:\n" + sMsg);
      }
#endif
      nState = 0;
    }
  }

  if (m_transl.empty())
    throw Exception("Language file does not contain any valid entries");

  m_lastEntry = m_transl.end();
}
//---------------------------------------------------------------------------
bool LanguageSupport::FindTransl(const AnsiString& asStr)
{
  if (!asStr.IsEmpty()) {
    word32 lHash = strihash(asStr.c_str());

    if ((m_lastEntry = m_transl.find(lHash)) != m_transl.end())
      return true;
  }

  return false;
}
//---------------------------------------------------------------------------
bool LanguageSupport::FindTransl(const WString& sStr)
{
  if (!sStr.IsEmpty()) {
    word32 lHash = strihash(sStr.c_str());

    if ((m_lastEntry = m_transl.find(lHash)) != m_transl.end())
      return true;
  }

  return false;
}
//---------------------------------------------------------------------------
WString LanguageSupport::Translate(const AnsiString& asStr)
{
  // not found? return the original string
  if (!FindTransl(asStr))
    return asStr;
  return WString(m_lastEntry->second.c_str());
}
//---------------------------------------------------------------------------
WString LanguageSupport::Translate(const WString& sStr)
{
  if (!FindTransl(sStr))
    return sStr;
  return WString(m_lastEntry->second.c_str());
}
//---------------------------------------------------------------------------
WString LanguageSupport::TranslateDef(const AnsiString& asStr,
  const AnsiString& asDefault)
{
  // not found? return the default string
  if (!FindTransl(asStr))
    return asDefault;
  return WString(m_lastEntry->second.c_str());
}
//---------------------------------------------------------------------------
void LanguageSupport::LastTranslError(const AnsiString& asErrMsg,
  bool blRemoveEntry)
{
  if (m_lastEntry != m_transl.end()) {
    Application->MessageBox(
      FormatW("An error has occurred while trying to process\nthe translated "
        "message\n\n\"%s\":\n\n%s.", m_lastEntry->second.c_str(),
        asErrMsg.c_str()).c_str(),
      L"Error", MB_ICONERROR);
    if (blRemoveEntry) {
      m_transl.erase(m_lastEntry);
      m_lastEntry = m_transl.end();
    }
  }
}
//---------------------------------------------------------------------------
WString TRLFormat(const AnsiString asFormat, ...)
{
  va_list argptr;
  va_start(argptr, asFormat);

  WString sResult;
  try {
    sResult = FormatW_AL(TRL(asFormat), argptr);
  }
  catch (...)
  {
    g_pLangSupp->LastTranslError("Error while formatting translated string");
    sResult = FormatW(asFormat, argptr);
  }

  va_end(argptr);

  return sResult;
}
//---------------------------------------------------------------------------

WString TRL(const AnsiString& asStr)
{
  if (!g_pLangSupp)
    return asStr;
  return g_pLangSupp->Translate(asStr);
}

WString TRL(const WString& sStr)
{
  if (!g_pLangSupp)
    return sStr;
  return g_pLangSupp->Translate(sStr);
}

WString TRL(const char* pszStr, const char* pszComment)
{
  if (!g_pLangSupp)
    return WString(pszStr);
  AnsiString asStr(pszStr);
  if (pszComment != nullptr) {
    AnsiString asStrComm = asStr + " ((" + AnsiString(pszComment) + "))";
    return g_pLangSupp->TranslateDef(asStrComm, asStr);
  }
  return g_pLangSupp->Translate(asStr);
}

WString TRL(const wchar_t* pwszStr)
{
  if (!g_pLangSupp)
    return WString(pwszStr);
  return g_pLangSupp->Translate(WString(pwszStr));
}

WString TRLDEF(const AnsiString& asStr, const AnsiString& asDefault)
{
  if (!g_pLangSupp)
    return asDefault;
  return g_pLangSupp->TranslateDef(asStr, asDefault);
}

void TRLS(WString& sStr)
{
  if (g_pLangSupp)
    sStr = g_pLangSupp->Translate(sStr);
}

void TRLMenuItem(TMenuItem* pItem)
{
  const WString sCaption = pItem->Caption;
  if (sCaption.IsEmpty() || sCaption == "-")
    return;
  const wchar_t* pBuf = sCaption.c_str();
  std::wstring sConv;
  sConv.reserve(sCaption.Length());
  while (*pBuf != '\0') {
    if (*pBuf == '&') {
      pBuf++;
      if (*pBuf == '&') {
        sConv.push_back('&');
        sConv.push_back('&');
        pBuf++;
      }
    }
    else
      sConv.push_back(*pBuf++);
  }
  pItem->Caption = TRL(sConv.c_str());
}

static void TRLMenuItems(TMenuItem* pItems)
{
  for (int nI = 0; nI < pItems->Count; nI++) {
    TMenuItem* pSingleItem = pItems->Items[nI];
    TRLMenuItem(pSingleItem);
    TRLMenuItems(pSingleItem);
  }
}

void TRLMenu(TMenu* pMenu)
{
  TRLMenuItems(pMenu->Items);
  /*for (int nI = 0; nI < pMenu->Items->Count; nI++) {
      TMenuItem* pItem = reinterpret_cast<TMenuItem*>(pMenu->Items->Items[nI]);
      TRLMenuItem(pItem);
      for (int nJ = 0; nJ < pItem->Count; nJ++) {
        TMenuItem* pSubItem = reinterpret_cast<TMenuItem*>(pItem->Items[nJ]);
        TRLCaption(pSubItem);
        for (int nK = 0; nK < pSubItem->Count; nK++) {
          TMenuItem* pSubItem2 = reinterpret_cast<TMenuItem*>(pSubItem->Items[nK]);
          if (pSubItem2->Caption != WString("-"))
            TRLCaption(pSubItem2);
        }
      }
  }*/
}

