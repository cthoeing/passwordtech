// PasswMngColSelect.h
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

#ifndef PasswMngColSelectH
#define PasswMngColSelectH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.CheckLst.hpp>
//---------------------------------------------------------------------------
#include "UnicodeUtil.h"

class TPasswMngColDlg : public TForm
{
__published:	// IDE-managed Components
  TCheckListBox *OptionsList;
  TButton *OKBtn;
  TButton *CancelBtn;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall OKBtnClick(TObject *Sender);
private:	// User declarations
  TForm* m_pParentForm;
  const WString* m_pColNames;
public:		// User declarations
  __fastcall TPasswMngColDlg(TComponent* Owner);
  void __fastcall SetColNames(const WString* pColNames);
  int __fastcall Execute(TForm* pParentForm, int nFlags, bool blStringsOnly);
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswMngColDlg *PasswMngColDlg;
//---------------------------------------------------------------------------
#endif
