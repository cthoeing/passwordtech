// InfoBox.cpp
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

#include "InfoBox.h"
#include "Main.h"
#include "Language.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TInfoBoxForm *InfoBoxForm;
//---------------------------------------------------------------------------
__fastcall TInfoBoxForm::TInfoBoxForm(TComponent* Owner)
  : TForm(Owner)
{
  Caption = TRL("Info");
}
//---------------------------------------------------------------------------
void __fastcall TInfoBoxForm::ShowInfo(const WString& sInfo,
 TForm* pParentForm)
{
  TextLbl->Caption = sInfo;
  Width = TextLbl->Width + 30;
  if (pParentForm == nullptr)
    pParentForm = MainForm;
  Top = pParentForm->Top + (pParentForm->Height - Height) / 2;
  Left = pParentForm->Left + (pParentForm->Width - Width) / 2;
  Timer->Interval = std::max(750 + sInfo.Length() * 20, 1500);
  //std::min(500 + sInfo.Length() * 20, 3000);
  Timer->Enabled = true;
  Show();
}
//---------------------------------------------------------------------------
void __fastcall TInfoBoxForm::TimerTimer(TObject *Sender)
{
  /*Timer->Tag -= Timer->Interval;
  if (Timer->Tag <= 0) {
  Close();
  //Timer->Enabled = false;
  }*/
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TInfoBoxForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  Timer->Enabled = false;
  TopMostManager::GetInstance()->OnFormClose(this);
}
//---------------------------------------------------------------------------
void __fastcall TInfoBoxForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------

