// PasswOptions.cpp
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

//---------------------------------------------------------------------------
__fastcall TPasswOptionsDlg::TPasswOptionsDlg(TComponent* Owner)
  : TForm(Owner)
{
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  TStrings* pStrList = PasswOptionsList->Items;

  if (g_pLangSupp != NULL) {
    TRLCaption(this);
    int nI;
    for (nI = 0; nI < pStrList->Count; nI++)
      pStrList->Strings[nI] = TRL(pStrList->Strings[nI]);
    TRLCaption(AmbigCharsLbl);
    TRLCaption(SpecialSymLbl);
    TRLCaption(MaxWordLenLbl);
    TRLCaption(TrigramFileLbl);
    TRLHint(BrowseBtn);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
    TRLMenu(ListMenu);
  }

  pStrList->Strings[0] = pStrList->Strings[0] + WString(" (B8G6I1l|0OQDS5Z2) [1-3] *");
  pStrList->Strings[1] = pStrList->Strings[1] + WString(" [1,2,4] *");
  pStrList->Strings[2] = pStrList->Strings[2] + WString(" [2]");
  pStrList->Strings[3] = pStrList->Strings[3] + WString(" [2]");
  pStrList->Strings[4] = pStrList->Strings[4] + WString(" [1,2]");
  pStrList->Strings[5] = pStrList->Strings[5] + WString(" [1,4] *");
  pStrList->Strings[6] = pStrList->Strings[6] + WString(" [1,4] *");
  pStrList->Strings[7] = pStrList->Strings[7] + WString(" [1,4] *");
  pStrList->Strings[8] = pStrList->Strings[8] + WString(" [1,4] *");
  pStrList->Strings[9] = pStrList->Strings[9] + WString(" [1]");
  pStrList->Strings[10] = pStrList->Strings[10] + WString(" [1,3] *");
  pStrList->Strings[11] = pStrList->Strings[11] + WString(" [1-4] *");
  pStrList->Strings[12] = pStrList->Strings[12] + WString(" [2,3] *");
  pStrList->Strings[13] = pStrList->Strings[13] + WString(" [1] *");
  pStrList->Strings[14] = pStrList->Strings[14] + WString(" [2] *");

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
  for (int nI = 0; nI < PasswOptionsList->Items->Count; nI++)
    passwOptions.Flags |= (PasswOptionsList->Checked[nI]) ? (1 << nI) : 0;
  passwOptions.AmbigChars = AmbigCharsBox->Text;
  passwOptions.SpecialSymbols = SpecialSymBox->Text;
  passwOptions.MaxWordLen = MaxWordLenSpinBtn->Position;
  passwOptions.TrigramFileName = TrigramFileBox->Text;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::SetOptions(const PasswOptions& passwOptions)
{
  for (int nI = 0; nI < PasswOptionsList->Items->Count; nI++)
    PasswOptionsList->Checked[nI] = passwOptions.Flags & (1 << nI);
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
  for (int nI = 0; nI < PasswOptionsList->Items->Count; nI++)
    PasswOptionsList->Checked[nI] = true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::ListMenu_DeselectAllClick(
  TObject *Sender)
{
  for (int nI = 0; nI < PasswOptionsList->Items->Count; nI++)
    PasswOptionsList->Checked[nI] = false;
}
//---------------------------------------------------------------------------
void __fastcall TPasswOptionsDlg::ListMenu_InvertSelectionClick(
  TObject *Sender)
{
  for (int nI = 0; nI < PasswOptionsList->Items->Count; nI++)
    PasswOptionsList->Checked[nI] = !PasswOptionsList->Checked[nI];
}
//---------------------------------------------------------------------------

