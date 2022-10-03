// PasswOptions.h
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
#ifndef PasswOptionsH
#define PasswOptionsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <CheckLst.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
//---------------------------------------------------------------------------
#include "UnicodeUtil.h"

const int
PASSWOPTIONS_NUM              = 16,

PASSWOPTION_EXCLUDEAMBIG      = 0x0001,
PASSWOPTION_FIRSTCHARNOTLC    = 0x0002,
PASSWOPTION_DONTSEPWORDS      = 0x0004,
PASSWOPTION_DONTSEPWORDSCHARS = 0x0008,
PASSWOPTION_REVERSEWCHORDER   = 0x0010,
PASSWOPTION_INCLUDEUPPERCASE  = 0x0020,
PASSWOPTION_INCLUDELOWERCASE  = 0x0040,
PASSWOPTION_INCLUDEDIGIT      = 0x0080,
PASSWOPTION_INCLUDESYMBOL     = 0x0100,
PASSWOPTION_INCLUDESUBSET     = 0x0200,
PASSWOPTION_EXCLUDEREPCHARS   = 0x0400,
PASSWOPTION_EXCLUDEDUPLICATES = 0x0800,
PASSWOPTION_LOWERCASEWORDS    = 0x1000,
PASSWOPTION_EACHCHARONLYONCE  = 0x2000,
PASSWOPTION_EACHWORDONLYONCE  = 0x4000,
PASSWOPTION_CAPITALIZEWORDS   = 0x8000;

const bool PASSWOPTIONS_STARRED[PASSWOPTIONS_NUM] =
{ true, true, false, false, false, true, true, true, true, false, true,
  true, true, true, true, false
};


struct PasswOptions {
  int Flags;
  WString AmbigChars;
  WString SpecialSymbols;
  int MaxWordLen;
  WString TrigramFileName;
};

class TPasswOptionsDlg : public TForm
{
__published:	// IDE-managed Components
  TCheckListBox *PasswOptionsList;
  TLabel *InfoLbl;
  TLabel *AmbigCharsLbl;
  TLabel *SpecialSymLbl;
  TLabel *MaxWordLenLbl;
  TUpDown *MaxWordLenSpinBtn;
  TLabel *TrigramFileLbl;
  TBevel *Separator;
  TButton *OKBtn;
  TButton *CancelBtn;
  TButton *BrowseBtn;
  TEdit *AmbigCharsBox;
  TEdit *SpecialSymBox;
  TEdit *MaxWordLenBox;
  TEdit *TrigramFileBox;
  TPopupMenu *ListMenu;
  TMenuItem *ListMenu_SelectAll;
  TMenuItem *ListMenu_DeselectAll;
  TMenuItem *ListMenu_InvertSelection;
  TMenuItem *ListMenu_N1;
  void __fastcall MaxWordLenBoxExit(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall BrowseBtnClick(TObject *Sender);
  void __fastcall ListMenu_SelectAllClick(TObject *Sender);
  void __fastcall ListMenu_DeselectAllClick(TObject *Sender);
  void __fastcall ListMenu_InvertSelectionClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TPasswOptionsDlg(TComponent* Owner);
  void __fastcall GetOptions(PasswOptions& passwOptions);
  void __fastcall SetOptions(const PasswOptions& passwOptions);
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswOptionsDlg *PasswOptionsDlg;
//---------------------------------------------------------------------------
#endif
