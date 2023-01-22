// PasswOptions.cpp
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

static const WString
  CONFIG_ID = "PasswOptions";

static const int
  OPTION_INDEX_TO_BIT[PASSWOPTIONS_NUM] =
    { 0, 1, 13, 10, 2, 3, 4, 15, 12, 14, 5, 6, 7, 8, 9, 11 };

static int BIT_TO_OPTION_INDEX[PASSWOPTIONS_NUM];

//---------------------------------------------------------------------------
__fastcall TPasswOptionsDlg::TPasswOptionsDlg(TComponent* Owner)
  : TForm(Owner)
{
  int i;
  for (i = 0; i < PASSWOPTIONS_NUM; i++)
    BIT_TO_OPTION_INDEX[OPTION_INDEX_TO_BIT[i]] = i;

  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  TStrings* pStrList = PasswOptionsList->Items;

  if (g_pLangSupp) {
    TRLCaption(this);
    for (i = 0; i < pStrList->Count; i++)
      pStrList->Strings[i] = TRL(pStrList->Strings[i]);
    TRLCaption(AmbigCharsLbl);
    TRLCaption(SpecialSymLbl);
    TRLCaption(MaxWordLenLbl);
    TRLCaption(TrigramFileLbl);
    TRLHint(BrowseBtn);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
    TRLMenu(ListMenu);
  }

  i = BIT_TO_OPTION_INDEX[0];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" (B8G6I1l|0OQDS5Z2) [1-3] *");
  i = BIT_TO_OPTION_INDEX[1];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,2,4] *");
  i = BIT_TO_OPTION_INDEX[2];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [2]");
  i = BIT_TO_OPTION_INDEX[3];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [2]");
  i = BIT_TO_OPTION_INDEX[4];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,2]");
  i = BIT_TO_OPTION_INDEX[5];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,4] *");
  i = BIT_TO_OPTION_INDEX[6];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,4] *");
  i = BIT_TO_OPTION_INDEX[7];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,4] *");
  i = BIT_TO_OPTION_INDEX[8];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,4] *");
  i = BIT_TO_OPTION_INDEX[9];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1]");
  i = BIT_TO_OPTION_INDEX[10];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1,3] *");
  i = BIT_TO_OPTION_INDEX[11];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1-4] *");
  i = BIT_TO_OPTION_INDEX[12];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [2,3] *");
  i = BIT_TO_OPTION_INDEX[13];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [1] *");
  i = BIT_TO_OPTION_INDEX[14];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [2] *");
  i = BIT_TO_OPTION_INDEX[15];
  pStrList->Strings[i] = pStrList->Strings[i] + WString(" [2]");

  InfoLbl->Caption = FormatW("[1] %s [2] %s [3] %s [4] %s\n* %s",
      TRL("Applies to pass_words_.").c_str(),
      TRL("Applies to pass_phrases_.").c_str(),
      TRL("Applies to formatted passwords.").c_str(),
      TRL("Applies to phonetic passwords.").c_str(),
      TRL("Selecting one of these options might reduce the security of "
        "generated passwords.").c_str()
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
  passwOptions.MaxWordLen = MaxWordLenSpinBtn->Position;
  passwOptions.TrigramFileName = TrigramFileBox->Text;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::SetOptions(const PasswOptions& passwOptions)
{
  for (int nI = 0; nI < PASSWOPTIONS_NUM; nI++)
    PasswOptionsList->Checked[BIT_TO_OPTION_INDEX[nI]] = passwOptions.Flags & (1 << nI);
  AmbigCharsBox->Text = passwOptions.AmbigChars;
  SpecialSymBox->Text = passwOptions.SpecialSymbols;
  MaxWordLenSpinBtn->Position = short(passwOptions.MaxWordLen);
  TrigramFileBox->Text = passwOptions.TrigramFileName;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::MaxWordLenBoxExit(TObject *Sender)
{
  int nValue = StrToIntDef(MaxWordLenBox->Text, 0);

  if (nValue < MaxWordLenSpinBtn->Min || nValue > MaxWordLenSpinBtn->Max)
    MaxWordLenBox->Text = WString(MaxWordLenSpinBtn->Position);
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
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

  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = MainForm->OpenDlg->Execute();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);

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

