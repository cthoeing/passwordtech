// Language.h
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
#ifndef LanguageH
#define LanguageH
//---------------------------------------------------------------------------
#include <unordered_map>
#include <memory>
#include "UnicodeUtil.h"

class LanguageSupport
{
private:
  std::unordered_map<word32, std::wstring> m_transl;
  std::unordered_map<word32, std::wstring>::iterator m_lastEntry;

  bool FindTransl(const AnsiString& asStr);
  bool FindTransl(const WString& sStr);

public:

  // constructor
  // -> language file name
  LanguageSupport(const WString& sFileName);

  // translation
  // -> string to translate
  // <- translated string
  WString Translate(const AnsiString& asStr);
  WString Translate(const WString& sStr);

  // translation
  // -> string to translate
  // -> default string to return if translation couldn't be found
  // <- translated string
  WString TranslateDef(const AnsiString& asStr,
    const AnsiString& asDefault);

  // call this function if the last translation failed (i.e., caused an exception)
  // -> error message to display
  // -> if 'true', remove the erroneous entry from the list
  void LastTranslError(const AnsiString& asErrMsg,
    bool blRemoveEntry = true);
};

extern std::unique_ptr<LanguageSupport> g_pLangSupp;


// some useful functions

WString TRL(const AnsiString& asStr);

WString TRL(const WString& sStr);

WString TRL(const char* pszStr, const char* pszComment = nullptr);

WString TRL(const wchar_t* pwszStr);

WString TRLDEF(const AnsiString& asStr, const AnsiString& asDefault);

void TRLS(WString& sStr);

template <class T> void TRLCaption(T* pControl)
{
  pControl->Caption = TRL(pControl->Caption);
}

void TRLMenuItem(TMenuItem* pItem);

void TRLMenu(TMenu* pMenu);

template <class T> void TRLHint(T* pControl)
{
  pControl->Hint = TRL(pControl->Hint);
}

WString TRLFormat(const AnsiString asFormat, ...);


#endif
