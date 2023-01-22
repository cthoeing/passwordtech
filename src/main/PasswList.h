// PasswList.h
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
#ifndef PasswListH
#define PasswListH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include "SecureMem.h"

class TPasswListForm : public TForm
{
__published:	// IDE-managed Components
  TPopupMenu *PasswListMenu;
  TMenuItem *PasswListMenu_Copy;
  TMenuItem *PasswListMenu_SelectAll;
  TMenuItem *PasswListMenu_N1;
  TMenuItem *PasswListMenu_SaveAsFile;
  TMenuItem *PasswListMenu_N2;
  TMenuItem *PasswListMenu_ChangeFont;
  TFontDialog *FontDlg;
  TRichEdit *PasswList;
  TMenuItem *PasswListMenu_EncryptCopy;
  TMenuItem *PasswListMenu_AddToDb;
  void __fastcall PasswListMenu_CopyClick(TObject *Sender);
  void __fastcall PasswListMenu_SelectAllClick(TObject *Sender);
  void __fastcall PasswListMenu_SaveAsFileClick(TObject *Sender);
  void __fastcall PasswListMenuPopup(TObject *Sender);
  void __fastcall PasswListMenu_ChangeFontClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormKeyPress(TObject *Sender, char &Key);
  void __fastcall PasswListMenu_EncryptCopyClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall PasswListStartDrag(TObject *Sender,
    TDragObject *&DragObject);
  void __fastcall PasswListMenu_AddToDbClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TPasswListForm(TComponent* Owner);
  __fastcall ~TPasswListForm();
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
  void __fastcall Execute(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswListForm *PasswListForm;
//---------------------------------------------------------------------------
#endif
