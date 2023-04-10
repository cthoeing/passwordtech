// PasswOptions.h
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
PASSWOPTIONS_NUM              = 17,

PASSWOPTION_EXCLUDEAMBIG      = 0x00001,
PASSWOPTION_FIRSTCHARNOTLC    = 0x00002,
PASSWOPTION_DONTSEPWORDS      = 0x00004,
PASSWOPTION_DONTSEPWORDSCHARS = 0x00008,
PASSWOPTION_REVERSEWCHORDER   = 0x00010,
PASSWOPTION_INCLUDEUPPERCASE  = 0x00020,
PASSWOPTION_INCLUDELOWERCASE  = 0x00040,
PASSWOPTION_INCLUDEDIGIT      = 0x00080,
PASSWOPTION_INCLUDESYMBOL     = 0x00100,
PASSWOPTION_INCLUDESUBSET     = 0x00200,
PASSWOPTION_EXCLUDEREPCHARS   = 0x00400,
PASSWOPTION_EXCLUDEDUPLICATES = 0x00800,
PASSWOPTION_LOWERCASEWORDS    = 0x01000,
PASSWOPTION_EACHCHARONLYONCE  = 0x02000,
PASSWOPTION_EACHWORDONLYONCE  = 0x04000,
PASSWOPTION_CAPITALIZEWORDS   = 0x08000,
PASSWOPTION_CHECKEACHPASSW    = 0x10000;

const bool PASSWOPTIONS_STARRED[PASSWOPTIONS_NUM] =
{ true, true, false, false, false, true, true, true, true, false, true,
  true, true, true, true, false
};

template<int N>
constexpr int _calcStarredBitField()
{
  //return PASSWOPTIONS_STARRED[i] ? 0 : 1 << i;
  int nFlags = 0;
  for (auto i = 0; i < N; i++) {
    if (PASSWOPTIONS_STARRED[i])
      nFlags |= 1 << i;
  }
  return nFlags;
}

const int PASSWOPTIONS_STARRED_BITFIELD = _calcStarredBitField<PASSWOPTIONS_NUM>();

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
