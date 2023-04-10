// PasswMngKeyValEdit.cpp
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

#include "PasswMngKeyValEdit.h"
#include "TopMostManager.h"
#include "Main.h"
#include "Language.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswMngKeyValDlg *PasswMngKeyValDlg;

const WString CONFIG_ID = "PasswMngKeyValEdit";

//---------------------------------------------------------------------------
__fastcall TPasswMngKeyValDlg::TPasswMngKeyValDlg(TComponent* Owner)
  : TForm(Owner)
{
  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngKeyValDlg::FormShow(TObject *Sender)
{
  //Top = PasswMngForm->Top + (PasswMngForm->Height - Height) / 2;
  //Left = PasswMngForm->Left + (PasswMngForm->Width - Width) / 2;
  KeyValueGrid->Row = 1;
  KeyValueGrid->Col = 1;
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngKeyValDlg::LoadConfig(void)
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
      KeyValueGrid->ColWidths[0] = StrToIntDef(asColWidths.SubString(1, nPos - 1), 20);
      KeyValueGrid->ColWidths[1] = StrToIntDef(asColWidths.SubString(nPos + 1,
        asColWidths.Length() - nPos), 20);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngKeyValDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  WString sColWidths = IntToStr(KeyValueGrid->ColWidths[0]) + ";" +
    IntToStr(KeyValueGrid->ColWidths[1]);
  g_pIni->WriteString(CONFIG_ID, "ColWidths", sColWidths);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngKeyValDlg::Clear(void)
{
  for (int i = 1; i < KeyValueGrid->RowCount; i++)
    KeyValueGrid->Cells[1][i] = WString();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngKeyValDlg::KeyValueGridSelectCell(TObject *Sender, int ACol,
          int ARow, bool &CanSelect)
{
 if (ACol == 0) {
    KeyValueGrid->Options = KeyValueGrid->Options >> goEditing;
  }
  else {
    KeyValueGrid->Options = KeyValueGrid->Options << goEditing;
  }
  CanSelect = true;
}
//---------------------------------------------------------------------------

