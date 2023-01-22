// ProfileEditor.h
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

#ifndef ProfileEditorH
#define ProfileEditorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TProfileEditDlg : public TForm
{
__published:	// IDE-managed Components
  TListBox *ProfileList;
  TLabel *ProfileNameLbl;
  TCheckBox *SaveAdvancedOptionsCheck;
  TCheckBox *ConfirmCheck;
  TButton *LoadBtn;
  TButton *DeleteBtn;
  TButton *AddBtn;
  TButton *CloseBtn;
  TEdit *ProfileNameBox;
    TSpeedButton *MoveUpBtn;
    TSpeedButton *MoveDownBtn;
  void __fastcall ProfileListClick(TObject *Sender);
  void __fastcall ProfileNameBoxChange(TObject *Sender);
  void __fastcall LoadBtnClick(TObject *Sender);
  void __fastcall DeleteBtnClick(TObject *Sender);
  void __fastcall AddBtnClick(TObject *Sender);
  void __fastcall ProfileNameBoxKeyPress(TObject *Sender, char &Key);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall ProfileListDblClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
    void __fastcall MoveUpBtnClick(TObject *Sender);
    void __fastcall MoveDownBtnClick(TObject *Sender);
private:	// User declarations
  bool m_blModified;
  bool m_blAdded;
  bool m_blExitAfterAdd;
public:		// User declarations
  __fastcall TProfileEditDlg(TComponent* Owner);
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
  enum {
    MODIFIED = 1,
    ADDED    = 2
  };
  int __fastcall Execute(bool blExitAfterAdd = false);
};
//---------------------------------------------------------------------------
extern PACKAGE TProfileEditDlg *ProfileEditDlg;
//---------------------------------------------------------------------------
#endif
