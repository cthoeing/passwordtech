// PasswMngPwHistory.cpp
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
#include <Clipbrd.hpp>
#pragma hdrstop

#include "PasswMngPwHistory.h"
#include "PasswDatabase.h"
#include "TopMostManager.h"
#include "Main.h"
#include "Language.h"
#include "Util.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswHistoryDlg *PasswHistoryDlg;

const WString CONFIG_ID = "PasswHistoryDlg";

//---------------------------------------------------------------------------
__fastcall TPasswHistoryDlg::TPasswHistoryDlg(TComponent* Owner)
    : TForm(Owner)
{
  SetFormComponentsAnchors(this);
  HistorySizeSpinBtn->Max = PasswDatabase::MAX_PASSW_HISTORY_SIZE;
  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(EnableHistoryCheck);
    HistoryView->Columns->Items[0]->Caption = TRL(
      HistoryView->Columns->Items[0]->Caption);
    HistoryView->Columns->Items[1]->Caption = TRL(
      HistoryView->Columns->Items[1]->Caption);
    TRLCaption(ClearBtn);
    TRLCaption(CopyBtn);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::LoadConfig(void)
{
  int nTop = g_pIni->ReadInteger(CONFIG_ID, "WindowTop", INT_MAX);
  int nLeft = g_pIni->ReadInteger(CONFIG_ID, "WindowLeft", INT_MAX);

  if (nTop == INT_MAX || nLeft == INT_MAX)
    Position = poScreenCenter;
  else {
    Top = nTop;
    Left = nLeft;
  }

  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);

  AnsiString asColWidths = g_pIni->ReadString(CONFIG_ID, "ColWidths", "");
  if (asColWidths.Length() >= 3) {
    int nPos = asColWidths.Pos(";");
    if (nPos > 0) {
      HistoryView->Columns->Items[0]->Width =
        StrToIntDef(asColWidths.SubString(1, nPos - 1), 20);
      HistoryView->Columns->Items[1]->Width =
        StrToIntDef(asColWidths.SubString(nPos + 1, asColWidths.Length() - nPos), 20);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  WString sColWidths = IntToStr(HistoryView->Columns->Items[0]->Width) + ";" +
    IntToStr(HistoryView->Columns->Items[1]->Width);
  g_pIni->WriteString(CONFIG_ID, "ColWidths", sColWidths);
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::ClearBtnClick(TObject *Sender)
{
  HistoryView->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::CopyBtnClick(TObject *Sender)
{
  int nItems = HistoryView->Items->Count;
  if (nItems == 0)
    return;
  bool blAll = HistoryView->SelCount == 0;
  WString sToCopy;
  for (int i = 0; i < nItems; i++) {
    auto pItem = HistoryView->Items->Item[i];
    if (blAll || pItem->Selected) {
      if (!sToCopy.IsEmpty())
        sToCopy += "\n";
      sToCopy += pItem->Caption + "\t" + pItem->SubItems->Strings[0];
    }
  }
  Clipboard()->AsText = sToCopy;
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::EnableHistoryCheckClick(TObject *Sender)
{
  bool blEnabled = EnableHistoryCheck->Checked;
  HistorySizeBox->Enabled = blEnabled;
  HistorySizeSpinBtn->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::FormShow(TObject *Sender)
{
  bool blEnabled = HistoryView->Items->Count > 0;
  ClearBtn->Enabled = blEnabled;
  CopyBtn->Enabled = blEnabled;
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswHistoryDlg::HistoryViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
  if (Key == 'C' && Shift.Contains(ssCtrl))
    CopyBtnClick(this);
}
//---------------------------------------------------------------------------

