// Language.h
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
#ifndef LanguageH
#define LanguageH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include "UnicodeUtil.h"

class ELanguageError : public Exception
{
public:
  ELanguageError(const WString& sMsg) :
    Exception(sMsg)
  {}
};

class LanguageSupport
{
public:
  enum class FileFormat {
    LNG, PO
  };

  // constructor
  // -> language file name
  LanguageSupport(const WString& sFileName, bool blLoadHeaderOnly = false,
    bool blConvertOldFormat = false);

  // translation
  // -> string to translate
  // <- translated string
  WString Translate(const AnsiString& asStr) const;
  WString Translate(const WString& sStr) const;
  WString Translate(word32 lHash) const;

  // translation
  // -> string to translate
  // -> default string to return if translation couldn't be found
  // <- translated string
  WString TranslateDef(const AnsiString& asStr,
    const AnsiString& asDefault) const;

  // call this function if the last translation failed (i.e., caused an exception)
  // -> error message to display
  // -> if 'true', remove the erroneous entry from the list
  //void LastTranslError(const AnsiString& asErrMsg,
  //  bool blRemoveEntry = true);

  void SaveToPOFileFormat(const WString& sFileName, CharacterEncoding charEnc);

  __property FileFormat FormatType =
    { read=m_format };

  __property WString LanguageCode =
    { read=m_sLanguageCode };

  __property WString LanguageName =
    { read=m_sLanguageName };

  __property WString LanguageVersion =
    { read=m_sLanguageVersion };

  __property WString TranslatorName =
    { read=m_sTranslatorName };

  __property WString HelpFileName =
    { read=m_sHelpFileName };

private:
  std::unordered_map<word32, std::wstring> m_transl;
  //std::unordered_map<word32, std::wstring>::iterator m_lastEntry;
  std::unique_ptr<std::vector<std::pair<std::wstring, std::wstring>>> m_pFullTransl;
  FileFormat m_format;
  WString m_sLanguageCode;
  WString m_sLanguageName;
  WString m_sLanguageVersion;
  WString m_sTranslatorName;
  WString m_sHelpFileName;
};

extern std::unique_ptr<LanguageSupport> g_pLangSupp;


// some useful functions

WString TRL(const AnsiString& asStr);

WString TRL(const WString& sStr);

WString TRL(const char* pszStr, const char* pszContext = nullptr);

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

//WString TRLFormat(const WString asFormat, ...);
WString TRLFormat(const WString& sFormat,
  const std::vector<WString>& args);


#endif
