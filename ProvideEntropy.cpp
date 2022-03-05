// ProvideEntropy.cpp
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

#include "ProvideEntropy.h"
#include "Main.h"
#include "EntropyManager.h"
#include "Util.h"
#include "Language.h"
#include "TopMostManager.h"
#include "dragdrop.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProvideEntropyDlg *ProvideEntropyDlg;

static const WString
CONFIG_ID = "ProvideEntropy";

//---------------------------------------------------------------------------
__fastcall TProvideEntropyDlg::TProvideEntropyDlg(TComponent* Owner)
  : TForm(Owner)
{
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp != NULL) {
    TRLCaption(this);
    TRLCaption(InfoLbl);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }

  RegisterDropWindow(TextBox->Handle, &m_pTextBoxDropTarget);

  LoadConfig();
}
//---------------------------------------------------------------------------
__fastcall TProvideEntropyDlg::~TProvideEntropyDlg()
{
  UnregisterDropWindow(TextBox->Handle, m_pTextBoxDropTarget);
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::OKBtnClick(TObject *Sender)
{
  //if (GetEditBoxTextLen(TextBox) == 0)
  //  return;

  SecureWString sText;
  GetEditBoxTextBuf(TextBox, sText);

  SecureAnsiString sTextUtf8;
  WStringToUtf8(sText.c_str(), sTextUtf8);

  if (!sTextUtf8.IsEmpty()) {
    sText.Empty();

    word32 lEntBits = EntropyManager::GetInstance()->AddData(sTextUtf8,
        sTextUtf8.StrLen(), 0.437, 3.5);

    MsgBox(TRLFormat("%d bits of entropy have been added to the random pool.",
        lEntBits), MB_ICONINFORMATION);
  }

  ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::FormActivate(TObject *Sender)
{
  TextBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::TextBoxChange(TObject *Sender)
{
  OKBtn->Enabled = GetEditBoxTextLen(TextBox) != 0;
}
//---------------------------------------------------------------------------
void __fastcall TProvideEntropyDlg::TextBoxStartDrag(TObject *Sender,
  TDragObject *&DragObject)
{
  StartEditBoxDragDrop(TextBox);
}
//---------------------------------------------------------------------------

