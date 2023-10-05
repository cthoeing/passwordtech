// PasswEnter.h
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
//----------------------------------------------------------------------------
#ifndef PasswEnterH
#define PasswEnterH
//----------------------------------------------------------------------------
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Controls.hpp>
#include <Forms.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <SysUtils.hpp>
#include <Windows.hpp>
#include <System.hpp>
//----------------------------------------------------------------------------
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include "SecureMem.h"
#include "UnicodeUtil.h"
#include <Vcl.Dialogs.hpp>
#include <map>
#include <array>

const int
PASSWENTER_FLAG_ENCRYPT               = 0x0001,
PASSWENTER_FLAG_DECRYPT               = 0x0002,
PASSWENTER_FLAG_CONFIRMPASSW          = 0x0004,
PASSWENTER_FLAG_ENABLEOLDVER          = 0x0008,
PASSWENTER_FLAG_ENABLEPASSWCACHE      = 0x0010,
PASSWENTER_FLAG_ENABLEKEYFILE         = 0x0020,
PASSWENTER_FLAG_ENABLEKEYFILECREATION = 0x0040;

class TPasswEnterDlg : public TForm
{
__published:
  TLabel *PasswLbl;
  TLabel *ConfirmPasswLbl;
  TCheckBox *OldVersionCheck;
  TButton *OKBtn;
  TButton *CancelBtn;
  TEdit *PasswBox;
  TEdit *ConfirmPasswBox;
  TCheckBox *RememberPasswCheck;
  TEdit *RememberPasswTimeBox;
  TUpDown *RememberPasswTimeSpinBtn;
  TTimer *KeyExpiryTimer;
    TSpeedButton *TogglePasswBtn;
    TLabel *KeyFileLbl;
    TSpeedButton *BrowseBtn;
    TSpeedButton *CreateKeyFileBtn;
    TComboBox *KeyFileBox;
    TOpenDialog *OpenDlg;
    TSaveDialog *SaveDlg;
  void __fastcall OKBtnClick(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall RememberPasswCheckClick(TObject *Sender);
  void __fastcall KeyExpiryTimerTimer(TObject *Sender);
    void __fastcall BrowseBtnClick(TObject *Sender);
    void __fastcall CreateKeyFileBtnClick(TObject *Sender);
    void __fastcall TogglePasswBtnClick(TObject *Sender);
private:
  int m_nFlags;
  int m_nExpiryCountdown;
  SecureWString m_sEncryptedPassw;
  //TForm* m_pParentForm;
  std::map<TForm*,std::array<int,4>> m_formDim;

public:
  virtual __fastcall TPasswEnterDlg(TComponent* AOwner);
  virtual __fastcall ~TPasswEnterDlg();
  int __fastcall Execute(int nFlags,
    const WString& sTitle = "",
    TForm* pParentForm = nullptr,
    bool blUpdateScreenPos = true);
  SecureWString __fastcall GetPassw(int nPassw = 0);
  SecureMem<word8> __fastcall GetPasswBinary(void);
  WString __fastcall GetKeyFileName(void);
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
  void __fastcall Clear(void);
  void __fastcall ClearPasswCache(void);
  void __fastcall OnEndSession(void);
  int __fastcall GetPasswCacheExpiryCountdown(void)
  {
    return m_sEncryptedPassw.IsEmpty() ? -1 :
      (KeyExpiryTimer->Enabled ? m_nExpiryCountdown : 0);
  }
};
//----------------------------------------------------------------------------
extern PACKAGE TPasswEnterDlg *PasswEnterDlg;
//----------------------------------------------------------------------------
#endif
