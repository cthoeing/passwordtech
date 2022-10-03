// ProvideEntropy.h
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

#ifndef ProvideEntropyH
#define ProvideEntropyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TProvideEntropyDlg : public TForm
{
__published:	// IDE-managed Components
  TLabel *InfoLbl;
  TButton *CancelBtn;
  TButton *OKBtn;
  TRichEdit *TextBox;
  void __fastcall OKBtnClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall TextBoxChange(TObject *Sender);
  void __fastcall TextBoxStartDrag(TObject *Sender,
    TDragObject *&DragObject);
private:	// User declarations
  IDropTarget* m_pTextBoxDropTarget;
public:		// User declarations
  __fastcall TProvideEntropyDlg(TComponent* Owner);
  __fastcall ~TProvideEntropyDlg();
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TProvideEntropyDlg *ProvideEntropyDlg;
//---------------------------------------------------------------------------
#endif
