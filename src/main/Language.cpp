// Language.cpp
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
#include <vcl.h>
#include <stdio.h>
#include <memory>
#include <regex>
#pragma hdrstop

#include "Language.h"
#include "UnicodeUtil.h"
#include "StringFileStreamW.h"
#include "ProgramDef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

std::unique_ptr<LanguageSupport> g_pLangSupp;

const wchar_t
 LNG_LANGUAGE_NAME[]      = L"[LANGUAGENAME]",
 LNG_LANGUAGE_VERSION[]   = L"[LANGUAGEVERSION]",
 LNG_LANGUAGE_AUTHOR[]    = L"[LANGUAGEAUTHOR]",
 LNG_LANGUAGE_HELPFILE[]  = L"[LANGUAGEHELPFILE]",

 PO_MSG_ID[]              = L"msgid",
 PO_MSG_CTXT[]            = L"msgctxt",
 PO_MSG_STR[]             = L"msgstr";


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

  while ((ch = *pStr++) != '\0') {
    lHash = toupper(ch) + (lHash << 6) + (lHash << 16) - lHash;
  }

  return lHash;
}

const wchar_t WHITESPACE_CHARS[] = L" \n\r\t";

std::wstring trimStr(const std::wstring& s)
{
  if (s.empty())
    return s;

  auto pos1 = s.find_first_not_of(WHITESPACE_CHARS);
  if (pos1 == s.npos)
    return std::wstring();

  auto pos2 = s.find_last_not_of(WHITESPACE_CHARS);

  return s.substr(pos1, pos2 - pos1 + 1);
}

std::wstring extractFormatSpec(const std::wstring& s)
{
  static const std::wregex re(L"%[-+ #0]{0,5}(?:\\d{1,4}|\\*)?(?:\\.\\d{1,2}|\\.\\*)?"
    "(h|hh|l|ll|j|z|t|L)?([diuoxXfFeEgGaAcspn])");
  std::wsregex_iterator it(s.begin(), s.end(), re), endIt;
  std::wstring sFormatSpec;

  for (; it != endIt; it++) {
    auto match = *it;
    const int n = match.size();
    for (int i = 1; i < n; i++) {
      sFormatSpec += match[i].str();
    }
  }

  return sFormatSpec;
}

//---------------------------------------------------------------------------
LanguageSupport::LanguageSupport(const WString& sFileName,
  bool blLoadHeaderOnly, bool blConvertOldFormat)
{
  std::unique_ptr<TStringFileStreamW> pFile(
    new TStringFileStreamW(sFileName, fmOpenRead, ceUtf8));
  const int MSG_BUF_SIZE = 1024;
  wchar_t wszMsg[MSG_BUF_SIZE];
  wchar_t wszParsed[MSG_BUF_SIZE];
  int nMsgLen, nState = 0;

  const char* pszUnknown = "(Unknown)";

  if (SameText(ExtractFileExt(sFileName), ".po")) {
    m_format = FileFormat::PO;

    std::wstring sMsgId, sMsgStr, sMsgCtxt;
    std::wstring* psDest = nullptr;
    int nLine = 0;
    bool blHeader = false;

    auto addTranslEntry = [&]()
    {
      if (sMsgStr.empty())
        throw ELanguageError(Format("Empty field \"msgstr\" (parsing line %d)",
          ARRAYOFCONST((nLine))));

      if (sMsgId.empty())
         throw ELanguageError(Format("Empty field \"msgid\" (parsing line %d)",
           ARRAYOFCONST((nLine))));

      if (extractFormatSpec(sMsgStr) != extractFormatSpec(sMsgId)) {
        throw ELanguageError(Format("Incompatible format specifiers "
          "(parsing line %d):\nOriginal: \"%s\"\nTranslation: \"%s\"",
          ARRAYOFCONST((nLine, WString(sMsgId.c_str()), WString(sMsgStr.c_str())))));
      }

      if (!sMsgCtxt.empty())
        sMsgId += std::wstring(L"||") + sMsgCtxt;

      auto ret = m_transl.emplace(strihash(sMsgId.c_str()), sMsgStr);
#ifdef _DEBUG
      if (!ret.second) {
        ShowMessage("Collision found:\n" + WString(sMsgId.c_str()));
      }
#endif

      sMsgId.clear();
      sMsgStr.clear();
      sMsgCtxt.clear();
      psDest = nullptr;
    };

    while ((nMsgLen = pFile->ReadString(wszMsg, MSG_BUF_SIZE)) > 0) {
      nLine++;
      //if (wszMsg[0] == '#') // skip comment
      //  continue;

      int i = 0;
      const auto* psPrev = psDest;
      if (wcsncmp(wszMsg, PO_MSG_ID, sizeof(PO_MSG_ID)/2-1) == 0) {
        psDest = &sMsgId;
        i += 5;
      }
      else if (wcsncmp(wszMsg, PO_MSG_STR, sizeof(PO_MSG_STR)/2-1) == 0) {
        psDest = &sMsgStr;
        i += 6;
      }
      else if (wcsncmp(wszMsg, PO_MSG_CTXT, sizeof(PO_MSG_CTXT)/2-1) == 0) {
        psDest = &sMsgCtxt;
        i += 7;
      }
      else if (wszMsg[0] != '"')
        psDest = nullptr;

      if (psPrev == &sMsgStr && psDest != psPrev) {
        if (sMsgId.empty() && !blHeader) {
          if (sMsgStr.empty())
            break;

          // prepare regex search
          std::wregex re(L"([a-zA-Z-]+)\\s*:\\s*(.+)");
          std::wsregex_iterator keyValIt(sMsgStr.begin(),
            sMsgStr.end(), re),
            keyValEnd;
          std::unordered_map<std::wstring, std::wstring> keyVal;

          // iterate over regex matches
          for (; keyValIt != keyValEnd; keyValIt++) {
            auto m = *keyValIt;
            if (m.size() >= 3) {
              auto sVal = trimStr(m[2]);
              if (!sVal.empty())
                keyVal.emplace(m[1], sVal);
            }
          }

          // project ID and version
          auto it = keyVal.find(L"Project-Id-Version");
          if (it == keyVal.end())
            throw ELanguageError("Missing \"Project-Id-Version\" information");

          std::wstring sIdVersion = it->second;
          re = std::wregex(L"(\\d+\\.\\d+\\.\\d+)");
          std::wsmatch m;
          if (std::regex_search(sIdVersion, m, re) && m.size() >= 2)
            m_sLanguageVersion = WString(m[1].str().c_str());
          else
            throw ELanguageError("Missing or invalid language version number");

          if (pFile->CharEncoding == ceUtf8 && pFile->BOMLength == 0) {
            bool blValid = false;
            it = keyVal.find(L"Content-Type");
            if (it != keyVal.end()) {
              re = std::wregex(L"charset=(UTF-8|utf-8)");
              blValid = std::regex_search(it->second, m, re);
            }
            if (!blValid)
              throw ELanguageError("Unknown charset encoding");
          }

          // language name: English name and native name
          it = keyVal.find(L"Language");
          if (it != keyVal.end()) {
            std::wstring sCode = it->second;
            m_sLanguageCode = WString(sCode.c_str());
            wchar_t buf[256];
            int nLen = GetLocaleInfoEx(sCode.c_str(),
              LOCALE_SENGLISHLANGUAGENAME, buf, 256);
            if (nLen > 0) {
              m_sLanguageName = WString(buf);
              nLen = GetLocaleInfoEx(sCode.c_str(),
                LOCALE_SNATIVELANGUAGENAME, buf, 256);
              if (nLen > 0)
                m_sLanguageName += " (" + WString(buf) + ")";
            }
          }

          if (m_sLanguageName.IsEmpty()) {
            // set language name to file name without extension
            WString sName = ExtractFileName(sFileName);
            m_sLanguageName = sName.SubString(1, sName.Length() - 3);
          }

          // translator name
          it = keyVal.find(L"Last-Translator");
          if (it != keyVal.end())
            m_sTranslatorName = WString(it->second.c_str());
          else
            m_sTranslatorName = pszUnknown;

          // help file name
          it = keyVal.find(L"X-PasswordTech-Manual");
          if (it != keyVal.end())
            m_sHelpFileName = WString(it->second.c_str());

          blHeader = true;
          sMsgId.clear();
          sMsgStr.clear();
          sMsgCtxt.clear();
          psDest = nullptr;

          if (blLoadHeaderOnly)
            break;
        }
        else if (!blHeader)
          break;
        else
          addTranslEntry();
      }

      if (!psDest)
        continue;

      int nParsedLen = 0;
      bool blMsgStarted = false;
      for ( ; i < nMsgLen; i++) {
        if (blMsgStarted) {
          if (wszMsg[i] == '"')
            break;
          else if (wszMsg[i] == '\\') {
            wchar_t wch = wszMsg[i+1];
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
            i++;
          }
          else
            wszParsed[nParsedLen++] = wszMsg[i];
        }
        else if (wszMsg[i] == '"')
          blMsgStarted = true;
      }

      if (nParsedLen > 0) {
        //if (!psDest)
        //  throw ELanguageError(Format("Unknown message type (parsing line %d)",
        //    ARRAYOFCONST((nLine))));

        wszParsed[nParsedLen] = '\0';
        *psDest += std::wstring(wszParsed);
      }
    }

    if (!blHeader)
      throw ELanguageError("Missing header information");

    if (!sMsgStr.empty())
      addTranslEntry();
  }

  // legacy .lng file format
  else {
    m_format = FileFormat::LNG;

    if (blConvertOldFormat)
      m_pFullTransl.reset(new std::vector<std::pair<std::wstring, std::wstring>>);

    std::wstring sMsgId;
    int nNumMsg = 0;
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

      std::wstring sMsg;
      if (nParsedLen > 0) {
        wszParsed[nParsedLen] = '\0';
        sMsg = trimStr(std::wstring(wszParsed));
      }
      if (sMsg.empty()) {
        if (nNumMsg == 0)
          throw ELanguageError("Missing header information");
        nState = 0;
        continue;
      }

      if (nState == 0) {
        sMsgId = sMsg;
        nState = 1;
      }
      else {
        nState = 0;
        nNumMsg++;
        if (nNumMsg == 1) {
          if (sMsgId != LNG_LANGUAGE_NAME)
            throw ELanguageError("Missing language name information");
          m_sLanguageName = WString(sMsg.c_str());
          m_sLanguageCode = m_sLanguageName; // no language code in .lng files
          continue;
        }
        else if (nNumMsg == 2) {
          if (sMsgId != LNG_LANGUAGE_VERSION)
            throw ELanguageError("Missing language version information");
          m_sLanguageVersion = WString(sMsg.c_str());
          if (blLoadHeaderOnly)
            break;
          continue;
        }
        else if (sMsgId == LNG_LANGUAGE_AUTHOR) {
          m_sTranslatorName = WString(sMsg.c_str());
          continue;
        }
        else if (sMsgId == LNG_LANGUAGE_HELPFILE) {
          m_sHelpFileName = WString(sMsg.c_str());
          continue;
        }
        auto ret = m_transl.emplace(strihash(sMsgId.c_str()), sMsg);
  #ifdef _DEBUG
        if (!ret.second) {
          ShowMessage("Collision found:\n" + WString(sMsg.c_str()));
        }
  #endif

        if (m_pFullTransl) {
          m_pFullTransl->emplace_back(sMsgId, sMsg);
        }
      }
    }

    if (m_sTranslatorName.IsEmpty())
      m_sTranslatorName = pszUnknown;
  }

  if (!blLoadHeaderOnly && m_transl.empty())
    throw ELanguageError("Language file does not contain any valid entries");

  //m_lastEntry = m_transl.end();
}
//---------------------------------------------------------------------------
void LanguageSupport::SaveToPOFileFormat(const WString& sFileName,
  CharacterEncoding charEnc)
{
  if (!m_pFullTransl)
    throw ELanguageError("Full translation table not available");

  std::unique_ptr<TStringFileStreamW> pFile(new TStringFileStreamW(sFileName,
    fmCreate, charEnc));

  WString sHeader = Format(
    "# \"%s\" translation of %s\n"
    "# Converted by %s %s\n\n"
    "# NOTE: You have to provide the appropriate language code in the "
    "\"Language\" section of the following header.\n"
    "# If the language code is missing or invalid, PwTech will derive the "
    "language name from the file name.\n"
    "%s \"\"\n"
    "%s \"\"\n"
    "\"Project-Id-Version: %s %s\\n\"\n"
    "\"Last-Translator: %s\\n\"\n"
    "\"Language: \\n\"\n"
    "\"X-PasswordTech-Manual: manual.pdf\\n\"\n\n"
    ,
    ARRAYOFCONST((
    m_sLanguageName, PROGRAM_NAME,
    PROGRAM_NAME, PROGRAM_VERSION,
    PO_MSG_ID,
    PO_MSG_STR,
    PROGRAM_NAME, m_sLanguageVersion,
    m_sTranslatorName)));

  pFile->WriteString(sHeader.c_str(), sHeader.Length());

  auto convertMsg = [](const std::wstring& sSrc, const wchar_t* pType)
  {
    std::wstring sMsg;
    sMsg.reserve(sSrc.length() + 10);
    sMsg.push_back('"');
    bool blMultiLine = false;
    for (auto c : sSrc) {
      wchar_t esc = 0;
      switch (c) {
      case '\n':
        esc = 'n'; break;
      case '\r':
        esc = 'r'; break;
      case '\t':
        esc = 't'; break;
      case '"':
        esc = '"'; break;
      }
      if (esc != 0) {
        sMsg.push_back('\\');
        sMsg.push_back(esc);
        if (esc == 'n') {
          sMsg.insert(sMsg.length(), L"\"\n\"");
          blMultiLine = true;
        }
      }
      else
        sMsg.push_back(c);
    }
    sMsg.push_back('"');
    std::wstring sDest(pType);
    sDest.push_back(' ');
    if (blMultiLine)
      sDest += L"\"\"\n";
    sDest += sMsg;
    return sDest;
  };


  for (const auto& entry : *m_pFullTransl) {
    std::wstring sMsgId = entry.first;

    auto pos2 = sMsgId.find(L"))");
    if (pos2 == sMsgId.length() - 2) {
      auto pos1 = sMsgId.find(L"((");
      if (pos1 != sMsgId.npos) {
        if (pos2 - pos1 > 2) {
          std::wstring sMsgCtxt =
            convertMsg(sMsgId.substr(pos1 + 2, pos2 - pos1 - 2), PO_MSG_CTXT) +
            L"\n";
          pFile->WriteString(sMsgCtxt.c_str(), sMsgCtxt.length());
        }
        sMsgId = trimStr(sMsgId.substr(0, pos1));
      }
    }

    std::wstring sMsg = convertMsg(sMsgId, PO_MSG_ID) + L"\n" +
      convertMsg(entry.second, PO_MSG_STR) + L"\n\n";
    pFile->WriteString(sMsg.c_str(), sMsg.length());
  }
}
//---------------------------------------------------------------------------
WString LanguageSupport::Translate(const AnsiString& asStr) const
{
  if (!asStr.IsEmpty()) {
    word32 lHash = strihash(asStr.c_str());

    auto it = m_transl.find(lHash);
    if (it != m_transl.end())
      return WString(it->second.c_str());
  }

  return asStr;
}
//---------------------------------------------------------------------------
WString LanguageSupport::Translate(const WString& sStr) const
{
  if (!sStr.IsEmpty()) {
    word32 lHash = strihash(sStr.c_str());

    auto it = m_transl.find(lHash);
    if (it != m_transl.end())
      return WString(it->second.c_str());
  }

  return sStr;
}
//---------------------------------------------------------------------------
WString LanguageSupport::Translate(word32 lHash) const
{
  auto it = m_transl.find(lHash);
  return (it != m_transl.end()) ? WString(it->second.c_str()) : WString();
}
//---------------------------------------------------------------------------
WString LanguageSupport::TranslateDef(const AnsiString& asStr,
  const AnsiString& asDefault) const
{
  if (asStr.IsEmpty())
    return WString();

  word32 lHash = strihash(asStr.c_str());

  auto it = m_transl.find(lHash);
  return (it != m_transl.end()) ? WString(it->second.c_str()) :
    WString(asDefault);
}
//---------------------------------------------------------------------------
std::vector<word32> s_faultyTransl;

WString TRLFormat(const WString sFormat, ...)
{
  va_list argptr;
  va_start(argptr, sFormat);

  WString sTransl, sResult;
  word32 lHash;

  if (g_pLangSupp) {
    lHash = strihash(sFormat.c_str());
    auto it = std::find(s_faultyTransl.begin(), s_faultyTransl.end(), lHash);
    if (it == s_faultyTransl.end()) {
      sTransl = g_pLangSupp->Translate(lHash);
    }
  }

  try {
    sResult = FormatW_ArgList(!sTransl.IsEmpty() ? sTransl : sFormat, argptr);
  }
  catch (Exception& e)
  {
    va_end(argptr);

    bool blRethrow = true;

    if (!sTransl.IsEmpty()) {
      s_faultyTransl.push_back(lHash);

      WString sMsg = "Error while formatting translated string\n\"" + sTransl +
        "\":\n" + e.Message + ".";
      Application->MessageBox(sMsg.c_str(), L"Error", MB_ICONERROR);

      va_start(argptr, sFormat);
      try {
        sResult = FormatW_ArgList(sFormat, argptr);
        blRethrow = false;
      }
      catch (...) {
		  va_end(argptr);
      }
    }

    if (blRethrow)
      throw;
  }

  va_end(argptr);

  return sResult;
}
//---------------------------------------------------------------------------

WString TRL(const AnsiString& asStr)
{
  if (g_pLangSupp)
    return g_pLangSupp->Translate(asStr);
  return asStr;
}

WString TRL(const WString& sStr)
{
  if (g_pLangSupp)
    return g_pLangSupp->Translate(sStr);
  return sStr;
}

WString TRL(const char* pszStr, const char* pszContext)
{
  if (!g_pLangSupp)
    return WString(pszStr);
  AnsiString asStr(pszStr);
  if (pszContext != nullptr) {
    AnsiString asStrCtxt = asStr;
    if (g_pLangSupp->FormatType == LanguageSupport::FileFormat::LNG)
      asStrCtxt += " ((" + AnsiString(pszContext) + "))";
    else
      asStrCtxt += "||" + AnsiString(pszContext);
    return g_pLangSupp->TranslateDef(asStrCtxt, asStr);
  }
  return g_pLangSupp->Translate(asStr);
}

WString TRL(const wchar_t* pwszStr)
{
  if (g_pLangSupp)
    return g_pLangSupp->Translate(WString(pwszStr));
  return WString(pwszStr);
}

WString TRLDEF(const AnsiString& asStr, const AnsiString& asDefault)
{
  if (g_pLangSupp)
    return g_pLangSupp->TranslateDef(asStr, asDefault);
  return asDefault;
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
}

