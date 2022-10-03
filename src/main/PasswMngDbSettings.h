// PasswMngDbSettings.h
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

#ifndef PasswMngDbSettingsH
#define PasswMngDbSettingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
#include "UnicodeUtil.h"
#include <Vcl.Buttons.hpp>

struct PasswDbSettings {
  SecureWString DefaultUserName;
  SecureWString PasswFormatSeq;
  word32 DefaultExpiryDays = 0;
  word32 CipherType = 0;
  word32 NumKdfRounds = 0;
};

class TPasswDbSettingsDlg : public TForm
{
__published:	// IDE-managed Components
  TPageControl *ConfigPages;
  TTabSheet *GeneralSheet;
  TTabSheet *SecuritySheet;
  TLabel *DefUserNameLbl;
  TLabel *PasswFormatSeqLbl;
  TEdit *PasswFormatSeqBox;
  TEdit *DefUserNameBox;
  TLabel *EncryptionAlgoLbl;
  TComboBox *EncryptionAlgoList;
  TLabel *NumKdfRoundsLbl;
  TEdit *NumKdfRoundsBox;
  TButton *OKBtn;
  TButton *CancelBtn;
  TSpeedButton *CalcRoundsBtn;
  TSpeedButton *PasswGenTestBtn;
  TLabel *DefaultExpiryLbl;
  TEdit *Default…xpiryBox;
  TUpDown *DefaultExpiryUpDown;
  TEdit *PasswGenTestBox;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall OKBtnClick(TObject *Sender);
  void __fastcall CalcRoundsBtnClick(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall PasswGenTestBtnClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
  void __fastcall LoadConfig(void);
public:		// User declarations
  __fastcall TPasswDbSettingsDlg(TComponent* Owner);
  void __fastcall GetSettings(PasswDbSettings& settings);
  void __fastcall SetSettings(const PasswDbSettings& settings,
    bool blHasRecoveryPassw);
  void __fastcall SaveConfig(void);
  void __fastcall Clear(void)
  {
    PasswDbSettings s;
    SetSettings(s, false);
  }
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswDbSettingsDlg *PasswDbSettingsDlg;
//---------------------------------------------------------------------------
#endif
