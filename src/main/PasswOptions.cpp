// PasswOptions.cpp
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
#pragma hdrstop

#include "PasswOptions.h"
#include "Main.h"
#include "Language.h"
#include "Util.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswOptionsDlg *PasswOptionsDlg;

const WString
  CONFIG_ID = "PasswOptions";

const int
  OPTION_INDEX_TO_BIT[PASSWOPTIONS_NUM] =
    { 0, 1, 13, 10, 18, 2, 3, 4, 15, 12, 14, 5, 6, 7, 8, 17, 9, 11, 16 };

template<int N>
class _bitToOptionIndex {
public:
    constexpr _bitToOptionIndex() : m_table() {
        for (auto i = 0; i != N; ++i)
            m_table[OPTION_INDEX_TO_BIT[i]] = i;
    }
    int operator[](int i) const
    {
      return m_table[i];
    }
private:
    int m_table[N];
};

//int BIT_TO_OPTION_INDEX[PASSWOPTIONS_NUM];
const _bitToOptionIndex BIT_TO_OPTION_INDEX = _bitToOptionIndex<PASSWOPTIONS_NUM>();

//---------------------------------------------------------------------------
__fastcall TPasswOptionsDlg::TPasswOptionsDlg(TComponent* Owner)
  : TForm(Owner)
{
  SetFormComponentsAnchors(this);

  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  TStrings* pStrList = PasswOptionsList->Items;
  m_pOptionsList.reset(new TStringList);

  if (g_pLangSupp) {
    for (int i = 0; i < pStrList->Count; i++)
      m_pOptionsList->Add(TRL(pStrList->Strings[i]));
    pStrList->Assign(m_pOptionsList.get());
    TRLCaption(this);
    TRLCaption(AmbigCharsLbl);
    TRLCaption(SpecialSymLbl);
    TRLCaption(MinMaxWordLenLbl);
    TRLCaption(TrigramFileLbl);
    TRLCaption(SeparatorsLbl);
    TRLCaption(WordSepLbl);
    TRLCaption(WordCharSepLbl);
    TRLHint(BrowseBtn);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
    TRLMenu(ListMenu);
  }
  else
    m_pOptionsList->Assign(pStrList);

  int i = 0;
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " (B8G6I1l|0OQDS5Z2) [1-3]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1-4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [2]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [2]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,2]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,3]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1-4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [2,3]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [2]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [2]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1-4]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1]";
  pStrList->Strings[BIT_TO_OPTION_INDEX[i++]] += " [1,3]";

  for (i = 0; i < PASSWOPTIONS_NUM; i++) {
    if (PASSWOPTIONS_STARRED[i]) {
      pStrList->Strings[BIT_TO_OPTION_INDEX[i]] += " *";
    }
  }

  InfoLbl->Caption = FormatW("[1] %1 [2] %2 [3] %3 [4] %4\n* %5",
    { TRL("Applies to pass_words_."),
      TRL("Applies to pass_phrases_."),
      TRL("Applies to formatted passwords."),
      TRL("Applies to phonetic passwords."),
      TRL("Selecting one of these options might reduce the security of "
        "generated passwords.")
    }
    );

  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::GetOptions(PasswOptions& passwOptions)
{
  passwOptions.Flags = 0;
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    passwOptions.Flags |= PasswOptionsList->Checked[nI] ?
      (1 << OPTION_INDEX_TO_BIT[nI]) : 0;
  passwOptions.AmbigChars = AmbigCharsBox->Text;
  passwOptions.SpecialSymbols = SpecialSymBox->Text;
  passwOptions.MinWordLen = MinWordLenSpinBtn->Position;
  passwOptions.MaxWordLen = std::max(passwOptions.MinWordLen,
    MaxWordLenSpinBtn->Position);
  passwOptions.WordSeparator = WordSepBox->Text;
  passwOptions.WordCharSeparator = WordCharSepBox->Text;
  passwOptions.TrigramFileName = TrigramFileBox->Text;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::SetOptions(const PasswOptions& passwOptions)
{
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    PasswOptionsList->Checked[BIT_TO_OPTION_INDEX[nI]] = passwOptions.Flags & (1 << nI);
  AmbigCharsBox->Text = passwOptions.AmbigChars;
  SpecialSymBox->Text = passwOptions.SpecialSymbols;
  MinWordLenSpinBtn->Position = passwOptions.MinWordLen;
  MaxWordLenSpinBtn->Position = passwOptions.MaxWordLen;
  WordSepBox->Text = passwOptions.WordSeparator;
  WordCharSepBox->Text = passwOptions.WordCharSeparator;
  TrigramFileBox->Text = passwOptions.TrigramFileName;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::MinMaxWordLenBoxExit(TObject *Sender)
{
  TEdit* pEditBox;
  TUpDown* pSpinBtn;
  if (Sender == MinWordLenBox) {
    pEditBox = MinWordLenBox;
    pSpinBtn = MinWordLenSpinBtn;
  }
  else {
    pEditBox = MaxWordLenBox;
    pSpinBtn = MaxWordLenSpinBtn;
  }

  int nValue = StrToIntDef(pEditBox->Text, 0);

  if (nValue < pSpinBtn->Min || nValue > pSpinBtn->Max)
    pEditBox->Text = IntToStr(pSpinBtn->Position);
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::FormActivate(TObject *Sender)
{
  PasswOptionsList->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::BrowseBtnClick(TObject *Sender)
{
  MainForm->OpenDlg->FilterIndex = 3;

  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = MainForm->OpenDlg->Execute();
  TopMostManager::GetInstance().NormalizeTopMosts(this);

  if (!blSuccess)
    return;

  TrigramFileBox->Text = MainForm->OpenDlg->FileName;
  TrigramFileBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::ListMenu_SelectAllClick(TObject *Sender)
{
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    PasswOptionsList->Checked[nI] = true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::ListMenu_DeselectAllClick(
  TObject *Sender)
{
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    PasswOptionsList->Checked[nI] = false;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::ListMenu_InvertSelectionClick(
  TObject *Sender)
{
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    PasswOptionsList->Checked[nI] = !PasswOptionsList->Checked[nI];
}
//---------------------------------------------------------------------------
WString __fastcall TPasswOptionsDlg::BitToString(int nBitPos)
{
  return (nBitPos < PASSWOPTIONS_NUM) ? m_pOptionsList->Strings[
    BIT_TO_OPTION_INDEX[nBitPos]] : WString();
}
//---------------------------------------------------------------------------

