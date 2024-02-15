// PasswMngColSelect.cpp
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

#include "PasswMngColSelect.h"
#include "TopMostManager.h"
#include "PasswDatabase.h"
#include "Util.h"
#include "Language.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswMngColDlg *PasswMngColDlg;
//---------------------------------------------------------------------------
__fastcall TPasswMngColDlg::TPasswMngColDlg(TComponent* Owner)
  : TForm(Owner)
{
  SetFormComponentsAnchors(this);
  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngColDlg::FormShow(TObject *Sender)
{
  Top = m_pParentForm->Top + (m_pParentForm->Height - Height) / 2;
  Left = m_pParentForm->Left + (m_pParentForm->Width - Width) / 2;
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngColDlg::SetColNames(const WString* pColNames)
{
  m_pColNames = pColNames;
}
//---------------------------------------------------------------------------
int __fastcall TPasswMngColDlg::Execute(TForm* pParentForm, int nFlags,
  bool blStringsOnly)
{
  m_pParentForm = pParentForm;

  int nCols = blStringsOnly ? PasswDbEntry::NUM_STRING_FIELDS :
    PasswDbEntry::NUM_FIELDS;
  OptionsList->Clear();
  for (int nI = 0; nI < nCols; nI++) {
    OptionsList->Items->Add(m_pColNames[nI]);
    OptionsList->Checked[nI] = nFlags & (1 << nI);
  }

  if (ShowModal() == mrOk) {
    nFlags = 0;
    for (int nI = 0; nI < OptionsList->Items->Count; nI++) {
      if (OptionsList->Checked[nI])
        nFlags |= 1 << nI;
    }

    return nFlags;
  }

  return -1;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngColDlg::OKBtnClick(TObject *Sender)
{
  bool blValid = false;
  for (int nI = 0; nI < OptionsList->Items->Count; nI++) {
    if (OptionsList->Checked[nI]) {
      blValid = true;
      break;
    }
  }
  if (blValid)
    ModalResult = mrOk;
  else
    MsgBox(TRL("Select at least one column."), MB_ICONWARNING);
}
//---------------------------------------------------------------------------

