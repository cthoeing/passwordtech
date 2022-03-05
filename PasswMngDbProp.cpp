// PasswMngDbProp.cpp
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

#include "PasswMngDbProp.h"
#include "PasswManager.h"
#include "TopMostManager.h"
#include "Main.h"
#include "Language.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswMngDbPropDlg *PasswMngDbPropDlg;

const WString CONFIG_ID = "PasswMngDbProp";

//---------------------------------------------------------------------------
__fastcall TPasswMngDbPropDlg::TPasswMngDbPropDlg(TComponent* Owner)
    : TForm(Owner)
{
  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(CloseBtn);
    TRLCaption(PropView->Columns->Items[0]);
    TRLCaption(PropView->Columns->Items[1]);
    PropView->Groups->Items[0]->Header = TRL(PropView->Groups->Items[0]->Header);
    PropView->Groups->Items[1]->Header = TRL(PropView->Groups->Items[1]->Header);
    for (int i = 0; i < PropView->Items->Count; i++)
      TRLCaption(PropView->Items->Item[i]);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngDbPropDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);

  AnsiString asColWidths = g_pIni->ReadString(CONFIG_ID, "ColWidths", "");
  if (asColWidths.Length() >= 3) {
    int nPos = asColWidths.Pos(";");
    if (nPos > 0) {
      PropView->Columns->Items[0]->Width =
        StrToIntDef(asColWidths.SubString(1, nPos - 1), 20);
      PropView->Columns->Items[1]->Width = StrToIntDef(asColWidths.SubString(nPos + 1,
        asColWidths.Length() - nPos), 20);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngDbPropDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteString(CONFIG_ID, "ColWidths", IntToStr(
    PropView->Columns->Items[0]->Width) + ";" + IntToStr(
    PropView->Columns->Items[1]->Width));
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngDbPropDlg::FormClose(TObject *Sender, TCloseAction &Action)

{
  for (int i = 0; i < PropView->Items->Count; i++) {
    PropView->Items->Item[i]->SubItems->Clear();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngDbPropDlg::SetProperty(DbProperty prop, const WString& sVal)
{
  if (sVal.IsEmpty()) return;
  auto pSubItems = PropView->Items->Item[static_cast<int>(prop)]->SubItems;
  if (pSubItems->Count == 0)
    pSubItems->Add(sVal);
  else
    pSubItems->Strings[0] = sVal;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngDbPropDlg::FormShow(TObject *Sender)
{
  static bool blFirstTime = true;
  if (blFirstTime) {
    // dirty hack to ensure that the first group header ("File") is displayed
    int nWidth = Width;
    Width = nWidth + 1;
    Width = nWidth;
    blFirstTime = false;
  }

  Top = PasswMngForm->Top + (PasswMngForm->Height - Height) / 2;
  Left = PasswMngForm->Left + (PasswMngForm->Width - Width) / 2;

  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------

