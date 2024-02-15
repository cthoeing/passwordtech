// QuickHelp.h
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

#ifndef QuickHelpH
#define QuickHelpH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include "UnicodeUtil.h"

class TQuickHelpForm : public TForm
{
__published:	// IDE-managed Components
  TRichEdit *QuickHelpBox;
  TPopupMenu *QuickHelpBoxMenu;
  TMenuItem *QuickHelpBoxMenu_AutoPosition;
  TMenuItem *QuickHelpBoxMenu_N1;
  TMenuItem *QuickHelpBoxMenu_ChangeFont;
  TFontDialog *FontDlg;
  void __fastcall FormKeyPress(TObject *Sender, char &Key);
  void __fastcall QuickHelpBoxMenu_ChangeFontClick(TObject *Sender);
private:	// User declarations
  //int m_nMinWidth;
  //int m_nMinHeight;
  //int m_nVScrollX;
public:		// User declarations
  __fastcall TQuickHelpForm(TComponent* Owner);
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
  void __fastcall Execute(const WString& sHelp, int nPosX, int nPosY);
  static WString __fastcall FormatString(const WString& sSrc);
};
//---------------------------------------------------------------------------
extern PACKAGE TQuickHelpForm *QuickHelpForm;
//---------------------------------------------------------------------------
#endif
