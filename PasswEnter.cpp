// PasswEnter.cpp
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
//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "PasswEnter.h"
#include "Util.h"
#include "Main.h"
#include "Language.h"
#include "TopMostManager.h"
#include "MemUtil.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TPasswEnterDlg *PasswEnterDlg;

static const char
PASSWORD_CHAR = '*';

static const WString
CONFIG_ID     = "PasswEnter";

static word8 memcryptKey[16];

//static const int
//  PASSWBOX_REM_CHANGING  = 1,
//  PASSWBOX_REM_CHANGED   = 2,
//  PASSWBOX_REM_UNCHANGED = 3;

//---------------------------------------------------------------------
__fastcall TPasswEnterDlg::TPasswEnterDlg(TComponent* AOwner)
  : TForm(AOwner), m_pParentForm(MainForm), m_nExpiryCountdown(0)
{
  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp != NULL) {
    TRLCaption(PasswLbl);
    TRLCaption(ConfirmPasswLbl);
    TRLCaption(DisplayPasswCheck);
    TRLCaption(RememberPasswCheck);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
__fastcall TPasswEnterDlg::~TPasswEnterDlg()
{
  ClearPasswCache();
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::LoadConfig(void)
{
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
  DisplayPasswCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "DisplayPassw", false);
  DisplayPasswCheckClick(this);
  //RememberPasswCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "RememberPassw", false);
  RememberPasswTimeSpinBtn->Position = g_pIni->ReadInteger(CONFIG_ID,
      "RememberPasswTime", 60);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteBool(CONFIG_ID, "DisplayPassw", DisplayPasswCheck->Checked);
  //g_pIni->WriteBool(CONFIG_ID, "RememberPassw", RememberPasswCheck->Checked);
  g_pIni->WriteInteger(CONFIG_ID, "RememberPasswTime",
    RememberPasswTimeSpinBtn->Position);
}
//---------------------------------------------------------------------
void __fastcall TPasswEnterDlg::ClearPasswCache(void)
{
  if (!m_sEncryptedPassw.IsEmpty()) {
    m_sEncryptedPassw.Empty();
    memzero(memcryptKey, sizeof(memcryptKey));
    m_nExpiryCountdown = 0;
    KeyExpiryTimer->Enabled = false;
  }
}
//---------------------------------------------------------------------
void __fastcall TPasswEnterDlg::DisplayPasswCheckClick(TObject *Sender)
{
  PasswBox->PasswordChar = DisplayPasswCheck->Checked ? '\0' : PASSWORD_CHAR;
  ConfirmPasswBox->PasswordChar = PasswBox->PasswordChar;
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::OKBtnClick(TObject *Sender)
{
  if (m_nFlags & PASSWENTER_FLAG_CONFIRMPASSW) {
    SecureWString sPassw1, sPassw2;
    GetPassw(sPassw1, 0);
    GetPassw(sPassw2, 1);
    if (sPassw1 != sPassw2) {
      MsgBox(TRL("Passwords are not identical."), MB_ICONWARNING);
      PasswBox->SetFocus();
      return;
    }
  }

  if ((m_nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE) && RememberPasswCheck->Checked)
  {
    GetPassw(m_sEncryptedPassw);

    RandomPool::GetInstance()->GetData(memcryptKey, sizeof(memcryptKey));
    memcrypt_128bit(m_sEncryptedPassw.Bytes(), m_sEncryptedPassw.Bytes(),
      m_sEncryptedPassw.SizeBytes(), memcryptKey, true);

    if (RememberPasswTimeSpinBtn->Position > 0) {
      m_nExpiryCountdown = RememberPasswTimeSpinBtn->Position;
      KeyExpiryTimer->Enabled = true;
    }
  }

  ModalResult = mrOk;
}
//---------------------------------------------------------------------------
int __fastcall TPasswEnterDlg::Execute(int nFlags,
  const WString& sTitle,
  TForm* pParentForm)
{
  //if (nFlags & PASSWENTER_FLAG_DISABLEPASSWCACHE)
  //  ClearPasswCache();
  bool blPasswCache = nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE;

  if (blPasswCache && !m_sEncryptedPassw.IsEmpty()) {
    // reset countdown upon every request
    m_nExpiryCountdown = RememberPasswTimeSpinBtn->Position;
    return mrOk;
  }

  if (nFlags & PASSWENTER_FLAG_ENCRYPT) {
    Caption = sTitle.IsEmpty() ? TRL("Encrypt") : sTitle;
    //PasswLbl->Color = clBlack;//clYellow;
    //PasswLbl->Font->Color = clBlack;
  }
  else if (nFlags & PASSWENTER_FLAG_DECRYPT) {
    Caption = sTitle.IsEmpty() ? TRL("Decrypt") : sTitle;
    //PasswLbl->Color = clHighlight;
    //PasswLbl->Font->Color = clBlack;
  }
  else {
    Caption = sTitle;
    //PasswLbl->Color = clWindow;
    //PasswLbl->Font->Color = clWindowText;
  }

  ConfirmPasswLbl->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;
  ConfirmPasswBox->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;

  OldVersionCheck->Checked = false;
  OldVersionCheck->Enabled = nFlags & PASSWENTER_FLAG_ENABLEOLDVER;

  RememberPasswCheck->Enabled = blPasswCache;
  RememberPasswTimeBox->Enabled = blPasswCache;
  RememberPasswTimeSpinBtn->Enabled = blPasswCache;

  m_nFlags = nFlags;
  m_pParentForm = (pParentForm == NULL) ? MainForm : pParentForm;

  return ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::GetPassw(SecureWString& sDest,
  int nPassw)
{
  if ((m_nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE) && !m_sEncryptedPassw.IsEmpty())
  {
    sDest.Resize(m_sEncryptedPassw.Size());
    memcrypt_128bit(m_sEncryptedPassw.Bytes(), sDest.Bytes(),
      m_sEncryptedPassw.SizeBytes(), memcryptKey, false);
  }
  else if (nPassw == 0)
    GetEditBoxTextBuf(PasswBox, sDest);
  else
    GetEditBoxTextBuf(ConfirmPasswBox, sDest);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::GetPassw(SecureMem<word8>& sDest)
{
  SecureWString sStr;
  GetPassw(sStr);
  if (!sStr.IsEmpty())
    sDest.Assign(sStr.Bytes(), sStr.StrLenBytes()); // *without* terminating zero!
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::Clear(void)
{
  ClearEditBoxTextBuf(PasswBox, 256);
  if (m_nFlags & PASSWENTER_FLAG_CONFIRMPASSW)
    ClearEditBoxTextBuf(ConfirmPasswBox, 256);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::PasswBoxChange(TObject *Sender)
{
  OKBtn->Enabled = GetEditBoxTextLen(PasswBox) != 0;
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::FormActivate(TObject *Sender)
{
  PasswBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::FormShow(TObject *Sender)
{
  Top = m_pParentForm->Top + (m_pParentForm->Height - Height) / 2;
  Left = m_pParentForm->Left + (m_pParentForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::RememberPasswCheckClick(TObject *Sender)
{
  bool blEnabled = RememberPasswCheck->Checked;
  RememberPasswTimeBox->Enabled = blEnabled;
  RememberPasswTimeSpinBtn->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::CancelBtnClick(TObject *Sender)
{
  ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::KeyExpiryTimerTimer(TObject *Sender)
{
  if (--m_nExpiryCountdown == 0)
    ClearPasswCache();
}
//---------------------------------------------------------------------------

