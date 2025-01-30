// CharSetBuilder.h
//
// PASSWORD TECH
// Copyright (c) 2002-2025 by Christian Thoeing <c.thoeing@web.de>
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

#ifndef CharSetBuilderH
#define CharSetBuilderH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------

class TCharSetBuilderForm : public TForm
{
__published:	// IDE-managed Components
  TGroupBox *CharSetSelGroup;
  TLabel *FromLbl;
  TLabel *ToLbl;
  TCheckBox *LowerCaseCheck;
  TCheckBox *UpperCaseCheck;
  TCheckBox *DigitsCheck;
  TCheckBox *SpecialSymbolsCheck;
  TCheckBox *CharactersRangeCheck;
  TCheckBox *AdditionalCharsCheck;
  TEdit *FromBox;
  TEdit *ToBox;
  TEdit *AdditionalCharsBox;
  TEdit *NumBox1;
  TUpDown *NumSpinBtn1;
  TCheckBox *AtLeastCheck1;
  TEdit *NumBox2;
  TUpDown *NumSpinBtn2;
  TCheckBox *AtLeastCheck2;
  TEdit *NumBox3;
  TUpDown *NumSpinBtn3;
  TCheckBox *AtLeastCheck3;
  TEdit *NumBox4;
  TUpDown *NumSpinBtn4;
  TCheckBox *AtLeastCheck4;
  TEdit *NumBox5;
  TUpDown *NumSpinBtn5;
  TCheckBox *AtLeastCheck5;
  TGroupBox *ResultGroup;
  TEdit *ResultBox;
  TButton *ApplyBtn;
  TEdit *NumBox6;
  TUpDown *NumSpinBtn6;
  TCheckBox *AtLeastCheck6;
  TButton *ResetBtn;
  TGroupBox *TagGroup;
  TEdit *TagBox;
  void __fastcall LowerCaseCheckClick(TObject *Sender);
  void __fastcall UpperCaseCheckClick(TObject *Sender);
  void __fastcall DigitsCheckClick(TObject *Sender);
  void __fastcall SpecialSymbolsCheckClick(TObject *Sender);
  void __fastcall CharactersRangeCheckClick(TObject *Sender);
  void __fastcall AdditionalCharsCheckClick(TObject *Sender);
  void __fastcall CharSetParamChange(TObject *Sender);
  void __fastcall ApplyBtnClick(TObject *Sender);
  void __fastcall ResetBtnClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
private:	// User declarations
  void __fastcall LoadConfig(void);
public:		// User declarations
  __fastcall TCharSetBuilderForm(TComponent* Owner);
  void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TCharSetBuilderForm *CharSetBuilderForm;
//---------------------------------------------------------------------------
#endif
