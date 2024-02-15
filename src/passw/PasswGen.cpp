// PasswGen.cpp
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
#include <vcl.h>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <functional>
#include <Math.hpp>
#pragma hdrstop

#include "PasswGen.h"
#include "Main.h"
#include "PhoneticTrigram.h"
#include "Language.h"
#include "StringFileStreamW.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

extern "C" char* getDiceWd(int n);

// we have an extended number of codes/placeholders due to abbreviations
const int PASSWGEN_NUMCHARSETCODES_EXT = 18;

const char* CHARSET_CODES[PASSWGEN_NUMCHARSETCODES_EXT] =
{ "AZ", "az", "09", "Hex", "HEX", "hex", "base64", "b64", "easytoread", "etr",
  "symbols", "sym", "brackets", "brac", "punctuation", "punct",
  "highansi", "high"
};

const int CHARSET_CODES_MAP[PASSWGEN_NUMCHARSETCODES_EXT] =
{ 0, 1, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10 };

const char* CHARSET_DECODES[PASSWGEN_NUMCHARSETCODES] =
{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "abcdefghijklmnopqrstuvwxyz",
  "0123456789",
  "0123456789ABCDEF",
  "0123456789abcdef",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",
  "",
  "",
  "()[]{}<>",
  ",.;:",
  ""
};

const char
CHARSET_CODE_PHONETIC[] = "<phonetic>",
CHARSET_CODE_PHONETIC_UPPERCASE[] = "<phoneticu>",
CHARSET_CODE_PHONETIC_MIXEDCASE[] = "<phoneticx>";

const int
CHARSET_CODES_AZ          = 0,
CHARSET_CODES_az          = 1,
CHARSET_CODES_09          = 2,
CHARSET_CODES_Hex         = 3,
CHARSET_CODES_hex         = 4,
CHARSET_CODES_BASE64      = 5,
CHARSET_CODES_EASYTOREAD  = 6,
CHARSET_CODES_SYMBOLS     = 7,
CHARSET_CODES_BRACKETS    = 8,
CHARSET_CODES_PUNCTUATION = 9,
CHARSET_CODES_HIGHANSI    = 10;

const int
CHARSET_INCLUDE_AZ      = 0,
CHARSET_INCLUDE_az      = 1,
CHARSET_INCLUDE_09      = 2,
CHARSET_INCLUDE_SPECIAL = 3;

const char CHARSET_AMBIGUOUS[] =
  "B8G6I1l|0OQDS5Z2";

const char CHARSET_SYMBOLS[] =
  "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";

const char FORMAT_PLACEHOLDERS[] =
  "xaAUEdhHlLuvVZcCzpbsSy";

const char* CHARSET_FORMAT[PASSWGEN_NUMFORMATCHARSETS] =
{ "", // 'x' = custom char set
  "abcdefghijklmnopqrstuvwxyz0123456789",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
  "", // 'E' = <easytoread> character set
  "0123456789",
  "0123456789abcdef",
  "0123456789ABCDEF",
  "abcdefghijklmnopqrstuvwxyz",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
  "aeiou",
  "AEIOUaeiou",
  "AEIOU",
  "bcdfghjklmnpqrstvwxyz",
  "BCDFGHJKLMNPQRSTVWXYZbcdfghjklmnpqrstvwxyz",
  "BCDFGHJKLMNPQRSTVWXYZ",
  ",.;:",
  "()[]{}<>",
  "", // 's' = special symbols
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", // 'S' + special symbols
  ""  // 'y' = higher ANSI characters
};

const int
CHARSET_FORMAT_x = 0,
CHARSET_FORMAT_E = 4,
CHARSET_FORMAT_s = 19,
CHARSET_FORMAT_S = 20,
CHARSET_FORMAT_y = 21;

const int CHARSET_FORMAT_CONST[] =
{ 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };

const int FORMAT_REPEAT_MAXDEPTH = 4;


template<class T> inline int strchpos(const T* pStr, T c)
{
  for (int nI = 0; pStr[nI] != '\0'; nI++) {
    if (pStr[nI] == c)
      return nI;
  }
  return -1;
}

template<class T> inline int strchpos(const T* pStr, int nLen, T c)
{
  for (int nI = 0; nI < nLen; nI++) {
    if (pStr[nI] == c)
      return nI;
  }
  return -1;
}


static w32string s_charSetCodes[PASSWGEN_NUMCHARSETCODES_EXT];

//---------------------------------------------------------------------------
PasswordGenerator::PasswordGenerator(RandomGenerator* pRandGen)
  : m_pRandGen(pRandGen)
{
  if (s_charSetCodes[0].empty()) {
    for (int nI = 0; nI < PASSWGEN_NUMCHARSETCODES_EXT; nI++)
      s_charSetCodes[nI] = AsciiCharToW32String(CHARSET_CODES[nI]);
  }

  for (int nI = 0; nI < PASSWGEN_NUMCHARSETCODES; nI++)
    m_charSetDecodes[nI] = AsciiCharToW32String(CHARSET_DECODES[nI]);

  SetupCharSets(WString(CHARSET_FORMAT[2]));
  LoadWordListFile();
  LoadTrigramFile();
};
//---------------------------------------------------------------------------
PasswordGenerator::~PasswordGenerator()
{
}
//---------------------------------------------------------------------------
w32string PasswordGenerator::MakeCharSetUnique(const w32string& sSrc,
  const w32string* psAmbigChars,
  const std::vector<w32string>* pAmbigGroups,
  w32string* psRemovedAmbigChars)
{
  std::set<word32> chset(sSrc.begin(), sSrc.end());

  if (pAmbigGroups != nullptr && pAmbigGroups->size() >= 2) {
    for (auto g_it = pAmbigGroups->begin(); g_it != pAmbigGroups->end(); g_it++)
    {
      int nMatches = 0;
      for (auto it = g_it->begin(); it != g_it->end(); it++) {
        if (chset.count(*it))
          nMatches++;
      }

      if (nMatches >= 2) {
        for (auto it = g_it->begin(); it != g_it->end(); it++) {
          if (chset.erase(*it) && psRemovedAmbigChars)
            psRemovedAmbigChars->push_back(*it);
        }
      }
    }
  }
  else if (psAmbigChars != nullptr) {
    for (auto it = psAmbigChars->begin(); it != psAmbigChars->end(); it++) {
      if (chset.erase(*it) && psRemovedAmbigChars)
        psRemovedAmbigChars->push_back(*it);
    }
  }

//  w32string y(chset.begin(),chset.end());
//  WString x=W32StringToWString(y);

  return w32string(chset.begin(), chset.end());
}
//---------------------------------------------------------------------------
w32string PasswordGenerator::CreateSetOfAmbiguousChars(
  const w32string& sAmbigChars,
  std::vector<w32string>& ambigGroups)
{
  std::set<word32> chset;
  std::vector<w32string> groups;
  int nSepPos;

  if ((nSepPos = sAmbigChars.find(' ', 0)) >= 2 &&
    nSepPos <= static_cast<int>(sAmbigChars.length()) - 3)
  {
    w32string sGroup;
    //const word32* p = sAmbigChars.c_str();
    auto it = sAmbigChars.begin();
    do {
      if (it == sAmbigChars.end() || *it == ' ') {
        if (sGroup.length() >= 2)
          groups.push_back(sGroup);
        sGroup.clear();
      }
      else {
        auto ret = chset.insert(*it);
        if (ret.second)
          sGroup.push_back(*it);
      }
    }
    while (it++ != sAmbigChars.end());
  }
  else
    chset.insert(sAmbigChars.begin(), sAmbigChars.end());

  if (groups.size() >= 2)
    ambigGroups = groups;

  return w32string(chset.begin(), chset.end());
}
//---------------------------------------------------------------------------
std::optional<std::pair<w32string,CharSetType>> PasswordGenerator::ParseCharSet(
  w32string sInput,
  bool blIncludeCharFromEachSubset,
  std::optional<CharSetFreq>* pCharSetFreq) const
{
  if (sInput.length() < 2)
    return {};

  if (sInput[0] == '[') {
    int nPos;
    if ((nPos = sInput.find(']')) >= 2)
      sInput.erase(0, nPos + 1);
  }

  if (sInput == AsciiCharToW32String(CHARSET_CODE_PHONETIC)) {
    return std::make_optional(std::make_pair(
      AsciiCharToW32String(CHARSET_DECODES[1]), cstPhonetic)); // lower-case letters a..z
  }
  if (sInput == AsciiCharToW32String(CHARSET_CODE_PHONETIC_UPPERCASE)) {
    return std::make_optional(std::make_pair(
      AsciiCharToW32String(CHARSET_DECODES[0]), cstPhoneticUpperCase));
  }
  if (sInput == AsciiCharToW32String(CHARSET_CODE_PHONETIC_MIXEDCASE)) {
    return std::make_optional(std::make_pair(AsciiCharToW32String(CHARSET_DECODES[0]) +
      AsciiCharToW32String(CHARSET_DECODES[1]), cstPhoneticMixedCase));
  }

  std::map<w32string,int> charSetFreq;

  int nState = 0;
  w32string sPotCode, sChars;
  for (auto it = sInput.begin(); it != sInput.end(); it++) {
    if (*it == '<' && nState != 2) {
      if (nState != 0) {
        sChars += sPotCode;
        sChars.push_back('<');
        sPotCode.clear();
      }
      if (it+1 != sInput.end() && *(it+1) == '<') {
        nState = 2;
        it++;
      }
      else
        nState = 1;
      continue;
    }

    if (*it == '>' && nState != 0) {
      auto mapIt = charSetFreq.end();
      bool blValidCode = false;

      if (nState == 1 && sPotCode.length() >= 2) {
        for (int i = 0; i < PASSWGEN_NUMCHARSETCODES_EXT; i++) {
          if (sPotCode == s_charSetCodes[i]) {
            mapIt = charSetFreq.emplace(
              m_charSetDecodes[CHARSET_CODES_MAP[i]], 0).first;
            blValidCode = true;
            break;
          }
        }
        if (!blValidCode && sPotCode.length() == 2) {
          word32 lChar1 = sPotCode[0];
          word32 lChar2 = sPotCode[1];
          if (lChar2 > lChar1 && lChar2 < 0xd800 && lChar2 - lChar1 <= 256) {
            w32string sCustomSet;
            for (word32 lChar = lChar1; lChar <= lChar2; lChar++)
              sCustomSet.push_back(lChar);
            mapIt = charSetFreq.emplace(sCustomSet, 0).first;
            blValidCode = true;
          }
        }
      }
      else if (nState == 2 && !sPotCode.empty()) {
        if (it+1 != sInput.end() && *(it+1) == '>' &&
            !(it+2 != sInput.end() && *(it+2) == '>')) {
          w32string chset = MakeCharSetUnique(sPotCode);
          if (!chset.empty()) {
            mapIt = charSetFreq.emplace(chset, 0).first;
            it++;
            blValidCode = true;
          }
        }
        else {
          sPotCode.push_back('>');
          continue;
        }
      }

      if (mapIt != charSetFreq.end()) {
        if (sInput.end() - it >= 3 && *(it+1) == ':' && isdigit(*(it+2))) {
          std::string sFreq;
          bool blAtLeast = false;
          for (auto j = it + 2; j != sInput.end() && sFreq.length() <= 5; j++) {
            if (isdigit(*j))
              sFreq.push_back(*j);
            else if (*j == '+') {
              blAtLeast = true;
              break;
            }
            else break;
          }
          if (!sFreq.empty()) {
            int nFreq = std::stoi(sFreq);
            if (blAtLeast) {
              nFreq *= -1;
              it++;
            }
            mapIt->second = nFreq;

            it += 1 + sFreq.length();
          }
        }
        if (blIncludeCharFromEachSubset && mapIt->second == 0)
          mapIt->second = -1;
      }
      else if (!blValidCode) {
        sChars += sPotCode;
        sChars.push_back('<');
        sChars.push_back('>');
      }

      nState = 0;
      sPotCode.clear();
    }
    else if (nState != 0)
      sPotCode.push_back(*it);
    else
      sChars.push_back(*it);
  }

  if (nState != 0) {
    sChars += sPotCode;
    sChars.push_back('<');
  }

  /*w32string sStartSeq, sEndSeq;
  sStartSeq.push_back('<');
  sStartSeq.push_back('<');
  sEndSeq.push_back('>');
  sEndSeq.push_back('>');
  int nStartPos = 0, nState = 0, nMinChars = 1;
  while (sInput.length() > sStartSeq.length() + sEndSeq.length()) {
    if (nState == 1) {
      sStartSeq = w32string(1, '<');
      sEndSeq = w32string(1, '>');
      nMinChars = 2;
      nStartPos = 0;
      nState++;
    }
    else if (nState > 2)
      break;
    nStartPos = sInput.find(sStartSeq, nStartPos);
    if (nStartPos < 0) {
      nState++;
      continue;
    }
    int nEndPos = sInput.find(sEndSeq, nStartPos + sStartSeq.length());
    if (nEndPos < 0) {
      nState++;
      continue;
    }
    int nLen = nEndPos - nStartPos + sEndSeq.length();
    if (nLen < sStartSeq.length() + sEndSeq.length() + nMinChars) {
      nStartPos += sStartSeq.length();
      continue;
    }
    w32string sSubStr = sInput.substr(nStartPos + sStartSeq.length(),
      nLen - sStartSeq.length() - sEndSeq.length());
    auto mapIt = charSetFreq.end();
    if (sStartSeq.length() == 1) {
      for (int nI = 0; nI < PASSWGEN_NUMCHARSETCODES_EXT; nI++) {
        //w32string sCode = AsciiCharToW32String(CHARSET_CODES[nI]);
        if (sSubStr == s_charSetCodes[nI]) {
          //sParsed += m_charSetDecodes[CHARSET_CODES_MAP[nI]];
          mapIt = charSetFreq.emplace(
            m_charSetDecodes[CHARSET_CODES_MAP[nI]], 0).first;
          break;
        }
      }
      if (mapIt == charSetFreq.end() && sSubStr.length() == 2) {
        word32 lChar1 = sSubStr[0];
        word32 lChar2 = sSubStr[1];
        if (lChar2 > lChar1 && lChar2 < 0xd800 && lChar2 - lChar1 <= 256) {
          w32string sCustomSet;
          for (word32 lChar = lChar1; lChar <= lChar2; lChar++)
            sCustomSet.push_back(lChar);
          mapIt = charSetFreq.emplace(sCustomSet, 0).first;
        }
      }
    }
    else {
      w32string chset = MakeCharSetUnique(sSubStr);
      if (!chset.empty())
        mapIt = charSetFreq.emplace(chset, 0).first;
    }
    if (mapIt != charSetFreq.end()) {
      nEndPos += sEndSeq.length() - 1;
      if (sInput.length() - 1 - nEndPos >= 2 && sInput[nEndPos+1] == ':' &&
          isdigit(sInput[nEndPos+2])) {
        std::string sFreq;
        bool blAtLeast = false;
        for (auto it = sInput.begin()+nEndPos+2; it != sInput.end() &&
             sFreq.length() <= 5; it++) {
          if (isdigit(*it))
            sFreq.push_back(*it);
          else if (*it == '+') {
            blAtLeast = true;
            break;
          }
          else break;
        }
        if (!sFreq.empty()) {
          int nFreq = std::stoi(sFreq);
          if (blAtLeast)
            nFreq *= -1;
          mapIt->second = nFreq;

          nLen += sFreq.length() + 1;
          if (blAtLeast)
            nLen++;
        }
      }
      if (blIncludeCharFromEachSubset && mapIt->second == 0)
        mapIt->second = -1;
      sInput.erase(nStartPos, nLen);
      //nStartPos = 0;
    }
    else
      nStartPos += sStartSeq.length();
  }*/

  w32string sFullCharSet = sChars;
  bool blHasFreq = false;
  for (const auto& kv : charSetFreq) {
    sFullCharSet += kv.first;
    if (pCharSetFreq && kv.second != 0)
      blHasFreq = true;
  }
  if (blHasFreq)
    blHasFreq = (charSetFreq.size() + (sChars.length() >= 2 ? 1 : 0)) >= 2;

  w32string sRemovedAmbigChars;
  sFullCharSet = MakeCharSetUnique(sFullCharSet, &m_sAmbigCharSet,
    &m_ambigGroups, blHasFreq ? &sRemovedAmbigChars : nullptr);

  if (sFullCharSet.length() < 2)
    return {};

  if (blHasFreq) {
    //blHasFreq = false;
    std::map<w32string, int> uniqueCharSetFreq;
    if (sRemovedAmbigChars.empty())
      uniqueCharSetFreq = charSetFreq;
    else {
      for (const auto& kv : charSetFreq) {
        w32string sCurrCharSet = MakeCharSetUnique(kv.first, &sRemovedAmbigChars);
        if (!sCurrCharSet.empty())
          uniqueCharSetFreq.emplace(sCurrCharSet, kv.second);
      }
    }
    if (!sChars.empty()) {
      w32string sInputCharSet = MakeCharSetUnique(sChars, &sRemovedAmbigChars);
      if (!sInputCharSet.empty())
        uniqueCharSetFreq.emplace(sInputCharSet, 0);
    }
    if (uniqueCharSetFreq.size() >= 2) {
      w32string sCommonCharSet;
      std::vector<std::pair<w32string,int>> freqList;
      for (const auto& kv : uniqueCharSetFreq) {
        int nNumChars = abs(kv.second);
        if (nNumChars > 0) // && kv.first.length() >= 2)
          freqList.emplace_back(kv.first, nNumChars);
        if (kv.second <= 0)
          sCommonCharSet += kv.first;
      }
      if (sCommonCharSet.length() < 2) {
        for (const auto& kv : uniqueCharSetFreq)
          sCommonCharSet += kv.first;
      }
      sCommonCharSet = MakeCharSetUnique(sCommonCharSet, &sRemovedAmbigChars);
      if (sCommonCharSet.length() >= 2) {
        std::sort(freqList.begin(), freqList.end(),
          [](const std::pair<w32string,int>& a, const std::pair<w32string,int>& b)
        {
          return a.second > b.second;
        });
        freqList.push_back(std::make_pair(sCommonCharSet, INT_MAX));
        *pCharSetFreq = std::make_optional(std::move(freqList));
        return std::make_optional(std::make_pair(
          sFullCharSet, cstStandardWithFreq));
      }
    }
  }

  if (pCharSetFreq)
    pCharSetFreq->reset();

  return std::make_optional(std::make_pair(sFullCharSet, cstStandard));
}
//---------------------------------------------------------------------------
WString PasswordGenerator::SetupCharSets(const WString& sCustomChars,
  const WString& sAmbigChars,
  const WString& sSpecialSymbols,
  bool blIncludeCharFromEachSubset,
  bool blExcludeAmbigChars,
  bool blDetermineSubsets)
{
  w32string sAmbigCharSet, sSpecialSymCharSet;
  std::vector<w32string> ambigGroups;

  if (!sAmbigChars.IsEmpty())
    sAmbigCharSet = CreateSetOfAmbiguousChars(WStringToW32String(sAmbigChars),
      ambigGroups);
  else
    sAmbigCharSet = AsciiCharToW32String(CHARSET_AMBIGUOUS);

  if (blExcludeAmbigChars) {
    m_sAmbigCharSet = sAmbigCharSet;
    m_ambigGroups = ambigGroups;
  }
  else {
    m_sAmbigCharSet.clear();
    m_ambigGroups.clear();
  }

  if (!sSpecialSymbols.IsEmpty())
    sSpecialSymCharSet = MakeCharSetUnique(WStringToW32String(sSpecialSymbols));
  else
    sSpecialSymCharSet = AsciiCharToW32String(CHARSET_SYMBOLS);

  m_charSetDecodes[CHARSET_CODES_EASYTOREAD] = MakeCharSetUnique(
      AsciiCharToW32String(CHARSET_DECODES[CHARSET_CODES_AZ]) +
      AsciiCharToW32String(CHARSET_DECODES[CHARSET_CODES_az]) +
      AsciiCharToW32String(CHARSET_DECODES[CHARSET_CODES_09]),
      &sAmbigCharSet);

  m_charSetDecodes[CHARSET_CODES_SYMBOLS] = sSpecialSymCharSet;

  if (m_charSetDecodes[CHARSET_CODES_HIGHANSI].empty()) {
    const int HIGHANSI_NUM = 129;

    char szHighAnsi[HIGHANSI_NUM+1];
    for (int i = 0; i < HIGHANSI_NUM; i++)
      szHighAnsi[i] = static_cast<char>(127 + i);
    szHighAnsi[HIGHANSI_NUM] = '\0';

    int nWLen = MultiByteToWideChar(CP_ACP, 0, szHighAnsi, -1, nullptr, 0);
    WString sWStr;
    sWStr.SetLength(nWLen-1);
    MultiByteToWideChar(CP_ACP, 0, szHighAnsi, -1, &sWStr[1], nWLen);

    m_charSetDecodes[CHARSET_CODES_HIGHANSI] = WStringToW32String(sWStr);
  }

  CharSetType charSetType;
  std::optional<CharSetFreq> charSetFreq;
  auto customCharSetResult = ParseCharSet(
    WStringToW32String(sCustomChars),
    blIncludeCharFromEachSubset,
    &charSetFreq);

  // are there any non-lowercase letters in the set?
  m_blCustomCharSetNonLC = false;
  if (customCharSetResult) {
    for (auto ch : customCharSetResult->first) {
      if (ch < 'a' || ch > 'z') {
        m_blCustomCharSetNonLC = true;
        break;
      }
    }
  }

  WString sCustomCharSet;
  if (customCharSetResult)
    sCustomCharSet = W32StringToWString(customCharSetResult->first);
  m_nCustomCharSetUniqueSize = 0;

  if (customCharSetResult) {
    m_sCustomCharSet = customCharSetResult->first;
    m_customCharSetType = customCharSetResult->second;
    m_customCharSetFreq = charSetFreq;
    //m_nCustomCharSetSize = m_sCustomCharSet.length();
    switch (m_customCharSetType) {
    case cstStandard:
      m_dCustomCharSetEntropy = Log2(static_cast<double>(
        m_sCustomCharSet.length()));
      m_nCustomCharSetUniqueSize = m_sCustomCharSet.length();
      break;
    case cstStandardWithFreq:
      {
        m_dCustomCharSetEntropy = Log2(static_cast<double>(
          m_sCustomCharSet.length())); // rough estimate
        std::set<word32> chset;
        int nUniqueSize = 0;
        for (const auto& p : m_customCharSetFreq.value()) {
          int nCurrSize = 0;
          for (auto ch : p.first) {
            auto ret = chset.insert(ch);
            if (ret.second && ++nCurrSize == p.second)
              break;
          }
          nUniqueSize += nCurrSize;
        }
        m_nCustomCharSetUniqueSize = nUniqueSize;
      }
      break;
    case cstPhonetic:
    case cstPhoneticUpperCase:
      m_dCustomCharSetEntropy = m_dPhoneticEntropy;
      m_nCustomCharSetUniqueSize = INT_MAX;
      break;
    case cstPhoneticMixedCase:
      m_dCustomCharSetEntropy = m_dPhoneticEntropy + 1;
      m_nCustomCharSetUniqueSize = INT_MAX;
    }
  }

  m_includeCharSets[CHARSET_INCLUDE_AZ] = m_charSetDecodes[CHARSET_CODES_AZ];
  m_includeCharSets[CHARSET_INCLUDE_az] = m_charSetDecodes[CHARSET_CODES_az];
  m_includeCharSets[CHARSET_INCLUDE_09] = m_charSetDecodes[CHARSET_CODES_09];
  m_includeCharSets[CHARSET_INCLUDE_SPECIAL] = sSpecialSymCharSet;

  if (blExcludeAmbigChars) {
    for (int i = 0; i < PASSWGEN_NUMINCLUDECHARSETS; i++) {
      w32string sNew = MakeCharSetUnique(m_includeCharSets[i], &sAmbigCharSet);
      if (sNew.length() >= 2)
        m_includeCharSets[i] = sNew;
    }
  }

  for (int i = 0; i < PASSWGEN_NUMFORMATCHARSETS; i++)
    m_formatCharSets[i] = AsciiCharToW32String(CHARSET_FORMAT[i]);

  m_formatCharSets[CHARSET_FORMAT_x] = m_sCustomCharSet;
  m_formatCharSets[CHARSET_FORMAT_E] = m_charSetDecodes[CHARSET_CODES_EASYTOREAD];
  m_formatCharSets[CHARSET_FORMAT_s] = sSpecialSymCharSet;
  m_formatCharSets[CHARSET_FORMAT_S] = MakeCharSetUnique(
      AsciiCharToW32String(CHARSET_FORMAT[CHARSET_FORMAT_S]) + sSpecialSymCharSet);
  m_formatCharSets[CHARSET_FORMAT_y] = m_charSetDecodes[CHARSET_CODES_HIGHANSI];

  if (blExcludeAmbigChars) {
    //for (nI = 0; nI < sizeof(CHARSET_FORMAT_CONST)/sizeof(int); nI++) {
    for (int nSetIdx : CHARSET_FORMAT_CONST) {
      //int nSetIdx = chset[nI];
      w32string sNew = MakeCharSetUnique(m_formatCharSets[nSetIdx], &sAmbigCharSet);
      if (sNew.length() >= 2)
        m_formatCharSets[nSetIdx] = sNew;
    }
  }

  // determine character subsets in the custom character set
  if (blDetermineSubsets) {
    for (int i = 0; i < PASSWGEN_NUMINCLUDECHARSETS; i++) {
      w32string sSubset;
      size_t pos = 0;
      while ((pos = m_sCustomCharSet.find_first_of(m_includeCharSets[i], pos))
               != w32string::npos)
        sSubset.push_back(m_sCustomCharSet[pos++]);
      m_customSubsets[i] = sSubset;
    }
  }

  return sCustomCharSet;
}
//---------------------------------------------------------------------------
int PasswordGenerator::LoadWordListFile(WString sFileName,
  int nMinWordLen,
  int nMaxWordLen,
  bool blConvertToLC)
{
  int nNumOfWords = WORDLIST_DEFAULT_SIZE;

  if (!sFileName.IsEmpty()) {
    if (ExtractFilePath(sFileName).IsEmpty())
      sFileName = g_sExePath + sFileName;

    //TStringFileStreamW* pFile = nullptr;
    std::unordered_set<std::wstring> wordListSet;
    std::vector<std::wstring> wordListVec;

    try {
      auto pFile = std::make_unique<TStringFileStreamW>(
          sFileName, fmOpenRead, ceAnsi, true, 65536, "\n\t ");

      const int WORDBUF_SIZE = 1024;
      wchar_t wszWord[WORDBUF_SIZE];
      nMinWordLen = std::max(1, nMinWordLen);
      nMaxWordLen = std::max(nMinWordLen, nMaxWordLen);
      int nWordLen;

      while ((nWordLen = pFile->ReadString(wszWord, WORDBUF_SIZE)) > 0 &&
        wordListVec.size() < WORDLIST_MAX_SIZE)
      {
        WString sWord = WString(wszWord, nWordLen).Trim();

        if (sWord.IsEmpty())
          continue;

        nWordLen = GetNumOfUnicodeChars(sWord.c_str());

        if (nWordLen < nMinWordLen || nWordLen > nMaxWordLen)
          continue;

        if (blConvertToLC)
          sWord = LowerCase(sWord);

        auto ret = wordListSet.emplace(sWord.c_str(), sWord.Length());
        if (ret.second)
          wordListVec.push_back(*ret.first);
      }
    }
    catch (EStreamError& e) {
      return -1;
    }
    catch (...) {
      throw;
    }

    if ((nNumOfWords = wordListVec.size()) < 2)
      return 0;

    m_wordList.swap(wordListVec);
  }
  else if (!m_wordList.empty()) {
    m_wordList.clear();

    // force reallocation of vector by replacing its contents by an empty
    // vector (clear() doesn't necessarily free the buffer but may only
    // reduce the vector size to 0)
    std::vector<std::wstring> temp;
    m_wordList.swap(temp);
  }

  m_nWordListSize = nNumOfWords;
  m_dWordListEntropy = Log2(static_cast<double>(nNumOfWords));

  return nNumOfWords;
}
//---------------------------------------------------------------------------
int PasswordGenerator::GetPassword(SecureW32String& sDest,
  int nLength,
  int nFlags) const
{
  if (nLength <= 0)
    return 0;

  word32 lChar;
  std::unique_ptr<std::set<word32>> pPasswCharSet;

  if (static_cast<word32>(nLength + 1) < sDest.Size())
    sDest.New(nLength + 1);

  if (m_customCharSetFreq) {
    nFlags &= ~PASSW_FLAG_EXCLUDEREPCHARS;
    auto charSetFreq = m_customCharSetFreq.value();
    int nPos = 0;
    int nItemIdx = 0;

    /*auto removeChar = [](w32string& sCharSet, word32 lChar)
    {
      // characters are sorted since std::set was used for creating the set
      // hence, we may perform a binary search
      auto it = std::lower_bound(sCharSet.begin(), sCharSet.end(), lChar);
      if (it != sCharSet.end() && *it == lChar)
        sCharSet.erase(it);
    };*/

    for (auto& p : charSetFreq) {
      for (int i = 0; i < p.second && !p.first.empty() && nPos < nLength; i++) {
        lChar = p.first[m_pRandGen->GetNumRange(p.first.length())];
        sDest[nPos++] = lChar;
        if ((nFlags & PASSW_FLAG_EACHCHARONLYONCE) && nPos < nLength) {
          for (int j = nItemIdx; j < charSetFreq.size(); j++) {
            //removeChar(charSetFreq[j].first, lChar);
            // characters are sorted since std::set was used for creating the set
            // hence, we may perform a binary search
            auto& sCharSet = charSetFreq[j].first;
            auto it = std::lower_bound(sCharSet.begin(), sCharSet.end(), lChar);
            if (it != sCharSet.end() && *it == lChar)
              sCharSet.erase(it);
          }
        }
      }
      if (nPos >= nLength)
        break;
      nItemIdx++;
    }

    /*auto& sCommonCharSet = charSetFreq.sCommonCharSet;
    while (!sCommonCharSet.empty() && nPos < nLength) {
      lChar = sCommonCharSet[m_pRandGen->GetNumRange(sCommonCharSet.length())];
      pDest[nPos++] = lChar;
      if (nFlags & PASSW_FLAG_EACHCHARONLYONCE)
        removeChar(sCommonCharSet, lChar);
    }*/

    nLength = nPos;
    if (nLength >= 2)
      m_pRandGen->Permute<word32>(sDest, nLength);
  }
  else {
    if ((nFlags & PASSW_FLAG_EACHCHARONLYONCE) &&
        (nFlags & PASSW_FLAG_CHECKDUPLICATESBYSET))
      pPasswCharSet.reset(new std::set<word32>);

    int nSetSize = m_sCustomCharSet.length();
    for (int nI = 0; nI < nLength; ) {
      lChar = m_sCustomCharSet[m_pRandGen->GetNumRange(nSetSize)];
      if (nI == 0 && m_blCustomCharSetNonLC && nFlags & PASSW_FLAG_FIRSTCHARNOTLC
        && lChar >= 'a' && lChar <= 'z')
        continue;
      if (nFlags & PASSW_FLAG_EACHCHARONLYONCE) {
        if (pPasswCharSet) {
          auto ret = pPasswCharSet->insert(lChar);
          if (!ret.second)
            continue;
        }
        else if (nI > 0 && strchpos(sDest.begin(), nI, lChar) >= 0)
          continue;
      }
      else if (nI > 0 && nFlags & PASSW_FLAG_EXCLUDEREPCHARS && lChar == sDest[nI-1])
        continue;
      sDest[nI++] = lChar;
    }
  }

  sDest[nLength] = '\0';

  if (nFlags >= PASSW_FLAG_INCLUDEUPPERCASE) {
    SecureMem<int> randPerm(PASSWGEN_NUMINCLUDECHARSETS);
    const w32string* psCharSets = (nFlags & PASSW_FLAG_INCLUDESUBSET) ?
      m_customSubsets : m_includeCharSets;

    int nRand;
    for (int nI = 0, nJ = 0; nI < PASSWGEN_NUMINCLUDECHARSETS && nJ < nLength; nI++) {
      int nFlagVal = PASSW_FLAG_INCLUDEUPPERCASE << nI;

      if (!(nFlags & nFlagVal) || psCharSets[nI].empty())
        continue;

//        if (psCharSets[nI].find_first_of(pDest) != w32string::npos)
//          continue;

      do {
        if (nFlagVal == PASSW_FLAG_INCLUDELOWERCASE &&
            (nFlags & PASSW_FLAG_FIRSTCHARNOTLC)) {
          if (nLength-nJ >= 2)
            nRand = 1 + m_pRandGen->GetNumRange(nLength-1);
          else {
            nRand = -1;
            break;
          }
        }
        else
          nRand = m_pRandGen->GetNumRange(nLength);
      } while (randPerm.Find(nRand, 0, nJ) != randPerm.npos);

      if (nRand < 0)
        continue;

      randPerm[nJ++] = nRand;

      if (psCharSets[nI].find(sDest[nRand]) != w32string::npos)
        continue;

      if (nFlags & PASSW_FLAG_EACHCHARONLYONCE &&
        psCharSets[nI].find_first_not_of(sDest) == w32string::npos)
        continue;

      int nSetSize = psCharSets[nI].length();
      while (true) {
        lChar = psCharSets[nI][m_pRandGen->GetNumRange(nSetSize)];
        if (nFlags & PASSW_FLAG_EACHCHARONLYONCE) {
          if (pPasswCharSet) {
            std::pair<std::set<word32>::iterator, bool> ret =
              pPasswCharSet->insert(lChar);
            if (!ret.second)
              continue;
          }
          else if (strchpos(sDest.begin(), nLength, lChar) >= 0)
            continue;
        }
        else if (nFlags & PASSW_FLAG_EXCLUDEREPCHARS && nSetSize >= 3) {
          if (nRand > 0 && lChar == sDest[nRand-1])
            continue;
          if (nRand < nLength-1 && lChar == sDest[nRand+1])
            continue;
        }
        break;
      }
      sDest[nRand] = lChar;
    }

    nRand = 0;
  }

  lChar = 0;

#if 0 //ifdef _DEBUG
  if (nFlags & (PASSW_FLAG_EACHCHARONLYONCE | PASSW_FLAG_EXCLUDEREPCHARS)) {
    std::set<word32> testSet;
    for (int i = 0; i < nLength; i++) {
      bool blFail;
      if (nFlags & PASSW_FLAG_EACHCHARONLYONCE) {
        auto ret = testSet.insert(sDest[i]);
        blFail = !ret.second;
      }
      else
        blFail = i > 0 && sDest[i] == sDest[i-1];
      if (blFail) {
        WString s;
        s.SetLength(nLength);
        for (int j = 0; j < nLength; j++)
          s[j+1] = static_cast<wchar_t>(sDest[j]);
        throw Exception("Duplicate or rep. char in password " + s);
      }
    }
  }
#endif

  return nLength;
}
//---------------------------------------------------------------------------
int PasswordGenerator::GetPassphrase(SecureW32String& sDest,
  int nWords,
  const word32* pChars,
  int nCharsLen,
  int nFlags,
  int* pnNetWordsLen) const
{
  if (nWords <= 0)
    return 0;

  //int nCharsLen = (pChars != nullptr) ? w32strlen(pChars) : 0;
  //int nLength = 0;
  word32 lPos = 0;
  bool blAppendChars = false;

  if (nCharsLen > 0 && !(nFlags & PASSPHR_FLAG_COMBINEWCH)) {
    if (nFlags & PASSPHR_FLAG_REVERSEWCHORDER)
      blAppendChars = true;
    else {
      //memcpy(pDest, pChars, nCharsLen * sizeof(word32));
      //nLength = nCharsLen;
      //pDest[nLength++] = ' ';
      sDest.StrCat(pChars, nCharsLen, lPos);
      if (m_sWordSep.empty())
        sDest.StrCat(' ', lPos);
      else
        sDest.StrCat(m_sWordSep.c_str(), m_sWordSep.length(), lPos);
    }
  }

  int nRand;
  const int nCharsPerWord = (nCharsLen > 0) ? nCharsLen / nWords : 0;
  const int nCharsRest = (nCharsLen > 0) ? nCharsLen % nWords : 0;
  int nCharsPos = 0;
  SecureW32String sWord(WORDLIST_MAX_WORDLEN + 1);
  std::unique_ptr<std::set<int>> pUniqueWordIdx;

  if (nFlags & PASSPHR_FLAG_EACHWORDONLYONCE)
    pUniqueWordIdx.reset(new std::set<int>);

  int nNetWordsLen = 0;

  for (int i = 0; i < nWords; ) {
    nRand = m_pRandGen->GetNumRange(m_nWordListSize);

    if (pUniqueWordIdx) {
      auto ret = pUniqueWordIdx->insert(nRand);
      if (!ret.second)
        continue;
    }

    int nWordLen;
    if (m_wordList.empty())
      nWordLen = AsciiCharToW32Char(getDiceWd(nRand), sWord);
    else
      nWordLen = WCharToW32Char(m_wordList[nRand].c_str(), sWord);

    if (nFlags & PASSPHR_FLAG_CAPITALIZEWORDS)
      sWord[0] = toupper(sWord[0]);

    if (nFlags & PASSPHR_FLAG_COMBINEWCH && nCharsPos < nCharsLen) {
      int nToCopy = nCharsPerWord;
      if (i < nCharsRest)
        nToCopy++;

      if (nFlags & PASSPHR_FLAG_REVERSEWCHORDER) {
        //memcpy(pDest + nLength, pChars + nCharsPos, nToCopy * sizeof(word32));
        //nLength += nToCopy;
        sDest.StrCat(pChars + nCharsPos, nToCopy, lPos);

        if (!(nFlags & PASSPHR_FLAG_DONTSEPWCH)) {
          if (m_sWordCharSep.empty())
            //pDest[nLength++] = '-';
            sDest.StrCat('-', lPos);
          else {
            //memcpy(pDest + nLength, m_sWordCharSep.c_str(),
            //  m_sWordCharSep.length() * sizeof(word32));
            //nLength += m_sWordCharSep.length();
            sDest.StrCat(m_sWordCharSep.c_str(), m_sWordCharSep.length(), lPos);
          }
        }

        //memcpy(pDest + nLength, sWord, nWordLen * sizeof(word32));
        //nLength += nWordLen;
        sDest.StrCat(sWord, lPos);
      }
      else {
        //memcpy(pDest + nLength, sWord, nWordLen * sizeof(word32));
        //nLength += nWordLen;
        sDest.StrCat(sWord, lPos);

        if (!(nFlags & PASSPHR_FLAG_DONTSEPWCH)) {
          if (m_sWordCharSep.empty())
            //pDest[nLength++] = '-';
            sDest.StrCat('-', lPos);
          else {
            //memmcpy(pDest + nLength, m_sWordCharSep.c_str(),
            //m_sWordCharSep.length() * sizeof(word32));
            //nLength += m_sWordCharSep.length();
            sDest.StrCat(m_sWordCharSep.c_str(), m_sWordCharSep.length(), lPos);
          }
        }

        //memcpy(pDest + nLength, pChars + nCharsPos, nToCopy * sizeof(word32));
        //nLength += nToCopy;
        sDest.StrCat(pChars + nCharsPos, nToCopy, lPos);
      }

      nCharsPos += nToCopy;
    }
    else {
      //memcpy(pDest + nLength, sWord, nWordLen * sizeof(word32));
      //nLength += nWordLen;
      sDest.StrCat(sWord, lPos);
    }

    nNetWordsLen += nWordLen;

    if (++i < nWords) {
      if (!(nFlags & PASSPHR_FLAG_DONTSEPWORDS)) {
        if (m_sWordSep.empty()) {
          //pDest[nLength++] = ' ';
          sDest.StrCat(' ', lPos);
          nNetWordsLen++;
        }
        else {
          //memcpy(pDest + nLength, m_sWordSep.c_str(),
          //  m_sWordSep.length() * sizeof(word32));
          //nLength += m_sWordSep.length();
          sDest.StrCat(m_sWordSep.c_str(), m_sWordSep.length(), lPos);
          nNetWordsLen += m_sWordSep.length();
        }
      }
    }
  }

  if (blAppendChars) {
    //pDest[nLength++] = ' ';
    //memcpy(pDest + nLength, pChars, nCharsLen * sizeof(word32));
    //nLength += nCharsLen;
    if (m_sWordSep.empty())
      sDest.StrCat(' ', lPos);
    else
      sDest.StrCat(m_sWordSep.c_str(), m_sWordSep.length(), lPos);
    sDest.StrCat(pChars, nCharsLen, lPos);
  }

  //pDest[nLength] = '\0';

  if (pnNetWordsLen != nullptr)
    *pnNetWordsLen = nNetWordsLen;

  nRand = 0;

  return lPos; //nLength;
}
//---------------------------------------------------------------------------
int PasswordGenerator::GetFormatPassw(SecureW32String& sDest,
  const w32string& sFormat,
  int nFlags,
  const word32* pPassw,
  int* pnPasswUsed,
  w32string* pInvalidSpec,
  double* pdSecurity)
{
  if (sDest.Size() < 2 || sFormat.empty())
    return 0;

  word32* pDest = sDest.begin();
  const int nMaxDestLen = std::min(1'000'000'000u, sDest.Size() - 1);
  const int nFormatLen = sFormat.length();
  int nSrcIdx = 0, nDestIdx = 0, nI;
  bool blComment = false;
  bool blVerbatim = false;
  //bool blSpecMode = true;
  bool blUnique = false;
  bool blSecondNum = false;
  char szNum[] = "00000";
  bool blNumDefault = true;
  int nNum;
  int nNumIdx = 0;
  int nRepeatIdx = 0;
  int repeatNum[FORMAT_REPEAT_MAXDEPTH];
  int repeatStart[FORMAT_REPEAT_MAXDEPTH];
  int nPermNum = 0;
  int nPermStart;
  int nUserCharSetNum = 0;
  int nUserCharSetStart;
  int nToCopy;
  word32 lRand;
  double dPermSecurity;

  if (pnPasswUsed != nullptr)
    *pnPasswUsed = pPassw ? PASSFORMAT_PWUSED_NOSPECIFIER : 0;

  //if (plInvalidSpec != nullptr)
  //  *plInvalidSpec = 0;

  if (sFormat[0] == '[') {
    nSrcIdx++;
    for ( ; nSrcIdx < nFormatLen && sFormat[nSrcIdx] != ']'; nSrcIdx++);
    nSrcIdx++;
  }

  for ( ; nSrcIdx < nFormatLen && nDestIdx < nMaxDestLen; nSrcIdx++) {
    const word32 lChar = sFormat[nSrcIdx];

    if (nUserCharSetNum > 0) {
      bool blCharSetEnds = false;
      while (lChar == '>' && nSrcIdx < nFormatLen-1 && sFormat[nSrcIdx+1] == '>')
      {
        nSrcIdx++;
        blCharSetEnds = true;
      }
      if (!blCharSetEnds)
        continue;
    }

    if (lChar == '"') {
      if (nSrcIdx < nFormatLen-1 && sFormat[nSrcIdx+1] == '"')
        nSrcIdx++;
      else {
        blVerbatim = !blVerbatim;
        continue;
      }
    }

    if (blVerbatim) {
      pDest[nDestIdx++] = lChar;
      continue;
    }

    if (lChar == '*') {
      blUnique = true;
      continue;
    }

    if (lChar >= '0' && lChar <= '9') {
      if (nNumIdx < 5) {
        szNum[nNumIdx++] = lChar;
        continue;
      }
    }

    int nParsedNum = 1;
    if (nNumIdx > 0) {
      szNum[nNumIdx] = '\0';
      nParsedNum = atoi(szNum);
      if (nParsedNum > 0)
        blNumDefault = false;
      else
        nParsedNum = 1;
    }

    if (blSecondNum) {
      if (!blNumDefault && nNum != nParsedNum) {
        nNum = std::min(nNum, nParsedNum) + m_pRandGen->GetNumRange(
            abs(nParsedNum - nNum) + 1);
      }
    }
    else {
      nNum = nParsedNum;
      if (!blNumDefault && lChar == '-') {
        blSecondNum = true;
        nNumIdx = 0;
        continue;
      }
    }

    w32string sUserCharSet;
    const w32string* psCharSet = nullptr;
    CharSetType charSetType = cstStandard;

    switch (lChar) {
    case '<': // begin user-defined character set
      if (nSrcIdx < nFormatLen-1 && sFormat[nSrcIdx+1] == '<') {
        nUserCharSetNum = nNum;
        nSrcIdx++;
        nUserCharSetStart = nSrcIdx + 1;
      }
      break;

    case '>': // end
      if (nUserCharSetNum > 0) {
        int nUserCharSetLen = nSrcIdx - nUserCharSetStart - 1;
        if (nUserCharSetLen >= 2) {
          sUserCharSet = sFormat.substr(nUserCharSetStart, nUserCharSetLen);
          auto userCharSetResult = ParseCharSet(sUserCharSet);
          if (userCharSetResult) {
            sUserCharSet = userCharSetResult->first;
            psCharSet = &sUserCharSet;
            nNum = nUserCharSetNum;
          }
        }
      }
      nUserCharSetNum = 0;
      break;

    case 'P': // copy password to dest
      if (pPassw != nullptr) {
        nToCopy = std::min<int>(w32strlen(pPassw), nMaxDestLen - nDestIdx);
        memcpy(pDest + nDestIdx, pPassw, nToCopy * sizeof(word32));
        nDestIdx += nToCopy;
        if (pnPasswUsed != nullptr) {
          *pnPasswUsed = nToCopy;
          pnPasswUsed = nullptr;
        }
        //pPassw = nullptr; // must be used only once
      }
      else if (pnPasswUsed != nullptr) {
        *pnPasswUsed = PASSFORMAT_PWUSED_EMPTYPASSW;
        pnPasswUsed = nullptr;
      }
      break;

    case 'q':
      charSetType = cstPhonetic;
      break;

    case 'Q':
      charSetType = cstPhoneticUpperCase;
      break;

    case 'r':
      charSetType = cstPhoneticMixedCase;
      break;

    case 'W': // add word
    case 'w': // add word + separator string
    {
      SecureW32String sWord(WORDLIST_MAX_WORDLEN + 1);
      std::unique_ptr<std::set<word32>> pUniqueWordIdx;

      if (blUnique) {
        nNum = blNumDefault ? m_nWordListSize : std::min(nNum, m_nWordListSize);
        pUniqueWordIdx.reset(new std::set<word32>);
      }
      for (nI = 0; nI < nNum && nDestIdx < nMaxDestLen; ) {
        lRand = m_pRandGen->GetNumRange(m_nWordListSize);
        int nWordLen;
        if (m_wordList.empty())
          nWordLen = AsciiCharToW32Char(getDiceWd(lRand), sWord);
        else
          nWordLen = WCharToW32Char(m_wordList[lRand].c_str(), sWord);
        if (blUnique) {
          auto ret = pUniqueWordIdx->insert(lRand);
          if (!ret.second)
            continue;
        }
        nToCopy = std::min(nWordLen, nMaxDestLen - nDestIdx);
        memcpy(pDest + nDestIdx, sWord, nToCopy * sizeof(word32));
        nDestIdx += nToCopy;
        if (lChar == 'w' && nI < nNum-1 && nDestIdx < nMaxDestLen) {
          if (m_sWordSep.empty())
            pDest[nDestIdx++] = ' ';
          else {
            nToCopy = std::min<int>(m_sWordSep.length(), nMaxDestLen - nDestIdx);
            memcpy(pDest + nDestIdx, m_sWordSep.c_str(), nToCopy * sizeof(word32));
            nDestIdx += nToCopy;
          }
        }
        nI++;
      }
      if (pdSecurity != nullptr) {
        if (blUnique)
          *pdSecurity += CalcPermSetEntropy(m_nWordListSize, nI);
        else
          *pdSecurity += m_dWordListEntropy * nI;
      }
      break;
    }
    case '[': // start index for repeating a sequence in sFormat
      if (nRepeatIdx < FORMAT_REPEAT_MAXDEPTH) {
        repeatNum[nRepeatIdx] = nNum - 1;
        repeatStart[nRepeatIdx] = nSrcIdx;
        nRepeatIdx++;
      }
      break;

    case ']': // end
      if (nRepeatIdx > 0) {
        if (repeatNum[nRepeatIdx-1] == 0 ||
          nSrcIdx - repeatStart[nRepeatIdx-1] < 3)
          nRepeatIdx--;
        else if (repeatNum[nRepeatIdx-1]-- > 0)
          nSrcIdx = repeatStart[nRepeatIdx-1];
      }
      break;

    case '{': // start index for permuting a sequence in pszDest
      nPermNum = blNumDefault ? 0 : nNum;
      nPermStart = nDestIdx;
      if (pdSecurity != nullptr)
        dPermSecurity = *pdSecurity;
      break;

    case '}': // end
      if (nPermNum >= 0) {
        int nPermSize = nDestIdx - nPermStart;
        if (nPermSize >= 2) { // now permute!
          m_pRandGen->Permute<word32>(pDest + nPermStart, nPermSize);
          int nToUse = (nPermNum == 0) ? nPermSize : std::min(nPermNum, nPermSize);
          nDestIdx = nPermStart + nToUse;
          if (pdSecurity != nullptr && nToUse < nPermSize)
            *pdSecurity = dPermSecurity + (*pdSecurity - dPermSecurity) * nToUse / nPermSize;
        }
        nPermNum = -1;
      }
      break;

    default:
      if (isalpha(lChar)) {
        int nPlaceholder = strchpos(FORMAT_PLACEHOLDERS, static_cast<char>(lChar));
        if (nPlaceholder >= 0) {
          if (nPlaceholder == CHARSET_FORMAT_x) {
            psCharSet = &m_sCustomCharSet;
            charSetType = charSetType;
          }
          else
            psCharSet = &m_formatCharSets[nPlaceholder];
        }
        else if (pInvalidSpec != nullptr) {
          pInvalidSpec->push_back(lChar);
        }
      }
      else {
        pDest[nDestIdx++] = lChar;
      }
    }

    if (charSetType == cstPhonetic || charSetType == cstPhoneticUpperCase ||
      charSetType == cstPhoneticMixedCase) {
      int nFlags = 0;

      if (charSetType == cstPhoneticUpperCase)
        nFlags |= PASSW_FLAG_PHONETICUPPERCASE;
      else if (charSetType == cstPhoneticMixedCase)
        nFlags |= PASSW_FLAG_PHONETICMIXEDCASE;

      int nLen = std::min(nNum, nMaxDestLen - nDestIdx);
      SecureW32String phoneticPassw(nLen + 1);
      nLen = GetPhoneticPassw(phoneticPassw, nLen, nFlags);
      memcpy(pDest + nDestIdx, phoneticPassw, nLen * sizeof(word32));

      nDestIdx += nLen;

      if (pdSecurity != nullptr)
        *pdSecurity += (m_dPhoneticEntropy +
            ((charSetType == cstPhoneticMixedCase) ? 1 : 0)) * nLen;
    }
    else if (psCharSet != nullptr && psCharSet->length() >= 2) {
      int nSetSize = psCharSet->length();
      int nStartIdx = nDestIdx;

      if (blUnique)
        nNum = (blNumDefault) ? nSetSize : std::min(nNum, nSetSize);

      for (nI = 0; nI < nNum && nDestIdx < nMaxDestLen; ) {
        lRand = (*psCharSet)[m_pRandGen->GetNumRange(nSetSize)];
        //bool checkRep = nDestIdx > 0;
        if (blUnique && nI > 0) {
          if (strchpos(pDest + nStartIdx, nI, lRand) >= 0)
            continue;
          //checkRep = false;
        }
        else if (nFlags & PASSFORMAT_FLAG_EXCLUDEREPCHARS &&
          nDestIdx > 0 &&
          lRand == pDest[nDestIdx-1])
          continue;
        pDest[nDestIdx++] = lRand;
        nI++;
      }

      if (pdSecurity != nullptr) {
        if (blUnique)
          *pdSecurity += CalcPermSetEntropy(nSetSize, nI);
        else
          *pdSecurity += Log2(static_cast<double>(nSetSize)) * nI;
      }
    }

    // do not reset certain flags when parsing a custom character set
    if (nUserCharSetNum == 0) {
      blUnique = false;
      blNumDefault = true;
    }
    blSecondNum = false;
    nNumIdx = 0;
  }

  pDest[nDestIdx] = '\0';

  lRand = 0;

  return nDestIdx;
}
//---------------------------------------------------------------------------
WString PasswordGenerator::GetWord(int nIndex) const
{
  if (m_wordList.empty())
    return getDiceWd(nIndex);

  const auto& sWord = m_wordList[nIndex];
  return WString(sWord.c_str(), sWord.length());
}
//---------------------------------------------------------------------------
void PasswordGenerator::CreateTrigramFile(const WString& sSrcFileName,
  const WString& sDestFileName,
  word32* plNumOfTris,
  double* pdEntropy)
{
  auto pSrcFile = std::make_unique<TStringFileStreamW>(
    sSrcFileName, fmOpenRead, ceAnsi, true, 65536, "\n\t ");

  const int WORDBUF_SIZE = 1024;
  wchar_t wszWord[WORDBUF_SIZE];
  std::vector<word32> tris(PHONETIC_TRIS_NUM, 0);
  word32 lSigma = 0;

  int nWordLen;
  while ((nWordLen = pSrcFile->ReadString(wszWord, WORDBUF_SIZE)) > 0) {
    WString sWord = Trim(WString(wszWord, nWordLen));
    nWordLen = sWord.Length();

    if (nWordLen < 3)
      continue;

    const wchar_t* pwszWord = sWord.c_str();
    wchar_t wch;
    char ch1 = -1, ch2 = -1, ch3;

    while ((wch = *pwszWord++) != '\0') {
      if (wch <= 'z' && (ch3 = tolower(wch) - 'a') >= 0 && ch3 <= 25) {
        if (ch1 >= 0) {
          tris[676*ch1+26*ch2+ch3]++;
          lSigma++;
        }

        ch1 = ch2;
        ch2 = ch3;
      }
    }
  }

  double dEntropy = 0;
  if (lSigma != 0) {
    double dProb;
    for (int nI = 0; nI < PHONETIC_TRIS_NUM; nI++) {
      dProb = static_cast<double>(tris[nI]) / lSigma;
      if (dProb > 0)
        dEntropy += dProb * Log2(dProb);
    }
    dEntropy = -dEntropy / 3;
  }

  if (plNumOfTris != nullptr)
    *plNumOfTris = lSigma;

  if (pdEntropy != nullptr)
    *pdEntropy = dEntropy;

  if (lSigma == 0 || dEntropy < 1.0)
    throw Exception("Source file does not contain enough trigrams");

  auto pDestFile = std::make_unique<TFileStream>(sDestFileName, fmCreate);

  pDestFile->Write(&lSigma, sizeof(word32));
  pDestFile->Write(&dEntropy, sizeof(double));
  pDestFile->Write(&tris[0], tris.size() * sizeof(word32));
}
//---------------------------------------------------------------------------
int PasswordGenerator::LoadTrigramFile(WString sFileName)
{
  std::vector<word32> tris;
  word32 lSigma = PHONETIC_SIGMA;
  double dEntropy = PHONETIC_ENTROPY;

  if (!sFileName.IsEmpty()) {
    if (ExtractFilePath(sFileName).IsEmpty())
      sFileName = g_sExePath + sFileName;

    try {
      auto pFile = std::make_unique<TFileStream>(sFileName, fmOpenRead);

      if (pFile->Size != PHONETIC_TRIS_NUM * sizeof(word32) +
          sizeof(word32) + sizeof(double))
        return 0;

      tris.resize(PHONETIC_TRIS_NUM);

      pFile->Read(&lSigma, sizeof(word32));
      pFile->Read(&dEntropy, sizeof(double));
      pFile->Read(&tris[0], PHONETIC_TRIS_NUM * sizeof(word32));

      pFile.reset(); // delete object and set ptr to nullptr

      if (lSigma == 0 || dEntropy < 1.0 || dEntropy > Log2(26.0))
        return 0;

      word32 lCheck = 0;
      for (int nI = 0; nI < PHONETIC_TRIS_NUM; nI++)
        lCheck += tris[nI];

      if (lCheck != lSigma)
        return 0;
    }
    catch (EStreamError& e) {
      return -1;
    }
  }

  m_phoneticTris = std::move(tris);
  m_lPhoneticSigma = lSigma;
  m_dPhoneticEntropy = dEntropy;

  return 1;
}
//---------------------------------------------------------------------------
int PasswordGenerator::GetPhoneticPassw(SecureW32String& sDest,
  int nLength,
  int nFlags) const
{
  if (nLength <= 0)
    return 0;

//  word32* pTris = (m_pPhoneticTris == nullptr) ? (word32*) PHONETIC_TRIS : m_pPhoneticTris;
//#define getLetter(x)  x + (blMixedCase ? ((m_pRandGen->GetByte() & 1) ? 'a' : 'A') : base)
  const bool blDefaultTris = m_phoneticTris.empty();
  const bool blMixedCase = nFlags & PASSW_FLAG_PHONETICMIXEDCASE;
  word32 lSumFreq = m_lPhoneticSigma;
  word32 lRand = m_pRandGen->GetNumRange(lSumFreq);
  word32 lSum = 0;
  int nChars = 0, nI;
  char base = (nFlags & PASSW_FLAG_PHONETICUPPERCASE) ? 'A' : 'a';
  char ch1, ch2, ch3;

  if (static_cast<word32>(nLength + 1) < sDest.Size())
    sDest.New(nLength + 1);

  std::function<char(char)> getLetter;
  if (blMixedCase)
    getLetter = [this](char c) { return c + ((m_pRandGen->GetByte() & 1) ? 'a' : 'A'); };
  else
    getLetter = [base](char c) { return c + base; };

  for (nI = 0; nI < PHONETIC_TRIS_NUM; nI++) {
    if (blDefaultTris)
      lSum += PHONETIC_TRIS[nI];
    else
      lSum += m_phoneticTris[nI];
    if (lSum > lRand) {
      ch1 = nI / 676;
      ch2 = (nI / 26) % 26;
      ch3 = nI % 26;
      break;
    }
  }

  if (nLength >= 1)
    sDest[nChars++] = getLetter(ch1);
  if (nLength >= 2)
    sDest[nChars++] = getLetter(ch2);
  if (nLength >= 3)
    sDest[nChars++] = getLetter(ch3);

  while (nChars < nLength) {
    ch1 = ch2;
    ch2 = ch3;

    lSumFreq = 0;
    for (ch3 = 0; ch3 < 26; ch3++) {
      int nIndex = 676*ch1+26*ch2+ch3;
      if (blDefaultTris)
        lSumFreq += PHONETIC_TRIS[nIndex];
      else
        lSumFreq += m_phoneticTris[nIndex];
    }

    if (lSumFreq == 0) {
      // if we can't find anything, just insert a vowel...
      static const char VOWELS[5] = { 0, 4, 8, 14, 20 };
      ch3 = VOWELS[m_pRandGen->GetNumRange(5)];
    }
    else {
      lRand = m_pRandGen->GetNumRange(lSumFreq);
      lSum = 0;

      for (ch3 = 0; ch3 < 26; ch3++) {
        int nIndex = 676*ch1+26*ch2+ch3;
        if (blDefaultTris)
          lSum += PHONETIC_TRIS[nIndex];
        else
          lSum += m_phoneticTris[nIndex];
        if (lSum > lRand)
          break;
      }
    }

    sDest[nChars++] = getLetter(ch3);
  }

  sDest[nChars] = '\0';
  lRand = 0;

  if (nFlags >= PASSW_FLAG_INCLUDEUPPERCASE) {
    SecureMem<int> randPerm(PASSWGEN_NUMINCLUDECHARSETS);
    int nJ, nRand;

    for (nI = nJ = 0; nI < PASSWGEN_NUMINCLUDECHARSETS && nJ < nLength; nI++) {
      int nFlagVal = PASSW_FLAG_INCLUDEUPPERCASE << nI;

      if (!(nFlags & nFlagVal))
        continue;

      // include lower case:
      // if neither "upper case" nor "mixed case" flag is active,
      // there's no need to explicitly convert a letter
      if (nFlagVal == PASSW_FLAG_INCLUDELOWERCASE &&
          !(nFlags & (PASSW_FLAG_PHONETICUPPERCASE | PASSW_FLAG_PHONETICMIXEDCASE)))
        continue;

      // include upper case:
      // if "upper case" flag is active but "mixed case" flag is not,
      // again there's no need for explicit conversion
      if (nFlagVal == PASSW_FLAG_INCLUDEUPPERCASE &&
          (nFlags & PASSW_FLAG_PHONETICUPPERCASE) && !blMixedCase)
        continue;

      /*
      if (nFlagVal == PASSW_FLAG_INCLUDEUPPERCASE) {
        w32string sCharSetLC = m_includeCharSets[nI];
        for (w32string::iterator it = sCharSetLC.begin();
             it != sCharSetLC.end(); it++)
          // we can use char-based tolower() here because the char set
          // can only include letters A..Z and no higher Unicode chars
          *it = tolower(*it);

        if (sCharSetLC.find_first_of(pDest) == w32string::npos)
          continue;
      }
      */

      do {
        nRand = m_pRandGen->GetNumRange(nLength);
      } while (randPerm.Find(nRand, 0, nJ) != randPerm.npos);

      randPerm[nJ++] = nRand;

      if (nFlagVal == PASSW_FLAG_INCLUDEUPPERCASE)
        sDest[nRand] = toupper(sDest[nRand]);
      else if (nFlagVal == PASSW_FLAG_INCLUDELOWERCASE)
        sDest[nRand] = tolower(sDest[nRand]);
      else
        sDest[nRand] = m_includeCharSets[nI][m_pRandGen->GetNumRange(
          m_includeCharSets[nI].length())];
    }

    nRand = 0;
  }

  return nChars;
}
//---------------------------------------------------------------------------
static double logFactorial(int nNum)
{
  if (nNum < 2)
    return 0;

  static const double TWOPI = 2 * acos(-1.0);

  const double dNum = nNum;
  return dNum * log(dNum) - dNum + 0.5 * log(TWOPI * dNum) + 1 / (12 * dNum);
}
//---------------------------------------------------------------------------
double PasswordGenerator::CalcPermSetEntropy(int nSetSize,
  int nNumOfSamples)
{
  static const double LOG2 = log(2.0);
  return (logFactorial(nSetSize) - logFactorial(nSetSize - nNumOfSamples)) / LOG2;
}
//---------------------------------------------------------------------------
double PasswordGenerator::EstimatePasswSecurity(const wchar_t* pwszPassw)
{
  if (pwszPassw == nullptr || pwszPassw[0] == '\0')
    return 0;

  int nPasswLen = wcslen(pwszPassw);

  SecureW32String sPassw(nPasswLen + 1);
  WCharToW32Char(pwszPassw, sPassw);

  // The following code is based on the function EstimatePasswordBits()
  // from the KeePass source code (http://keepass.org) by Dominik Reichl.
  // This algorithm tends to *under*rate random character sequences and
  // *over*rate passphrases containing words from a word list. Nevertheless,
  // it's really simple, fast and fully sufficient for our purpose.
  // For longer random sequences, applying some statistical tests for randomness
  // (autocorrelation coefficient, Runs test, entropy estimation, etc.) would be
  // appropriate, but this is simply not realistic for short sequences like
  // passwords/passphrases...

  std::unordered_map<word32, int> chcount;
  std::unordered_map<int, int> chdiff;

  bool blControl = false, blUpper = false, blLower = false, blDigits = false,
       blSpecial = false;
  int nCharSpace = 0;
  double dEffectiveLength = 0;

  for (int nI = 0; nI < nPasswLen; nI++)
  {
    word32 lChar = sPassw[nI];
    bool blOtherCharSet = false;

    if (lChar > 0 && lChar < ' ') blControl = true;          // control characters
    else if (lChar >= 'A' && lChar <= 'Z') blUpper = true;   // upper-case
    else if (lChar >= 'a' && lChar <= 'z') blLower = true;   // lower-case
    else if (lChar >= '0' && lChar <= '9') blDigits = true;  // digits
    else if (lChar >= ' ' && lChar <= '/') blSpecial = true; // special, including space
    else if (lChar >= ':' && lChar <= '@') blSpecial = true;
    else if (lChar >= '[' && lChar <= '`') blSpecial = true;
    else if (lChar >= '{' && lChar <= '~') blSpecial = true;
    else blOtherCharSet = true;

    double dDiffFactor = 1.0;
    int nCount;

    if (nI > 0) {
      int nDiff = lChar - sPassw[nI-1];
      auto ret = chdiff.emplace(nDiff, 0);
      ret.first->second++;
      nCount = ret.first->second;
      dDiffFactor /= nCount;
    }

    auto ret = chcount.emplace(lChar, 0);
    ret.first->second++;
    nCount = ret.first->second;
    if (ret.second) {
      if (blOtherCharSet)
        nCharSpace++;
    }

    dEffectiveLength += dDiffFactor / nCount;
  }

  if (blControl) nCharSpace += 2;  // control characters
  // (only 2 accessible in edit controls: #30 and #31)
  if (blUpper) nCharSpace += 26;   // upper-case
  if (blLower) nCharSpace += 26;   // lower-case
  if (blDigits) nCharSpace += 10;  // digits
  if (blSpecial) nCharSpace += 33; // special
  // maximum ANSI space = 97

  return Log2(static_cast<double>(nCharSpace)) * dEffectiveLength;
}
//---------------------------------------------------------------------------
