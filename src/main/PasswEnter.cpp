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
#include "PasswDatabase.h"
//---------------------------------------------------------------------
#pragma resource "*.dfm"
TPasswEnterDlg *PasswEnterDlg;

static const char
PASSWORD_CHAR = '*';

static const WString
CONFIG_ID     = "PasswEnter";

static word8 memcryptKey[16];

//---------------------------------------------------------------------
__fastcall TPasswEnterDlg::TPasswEnterDlg(TComponent* AOwner)
  : TForm(AOwner), m_pParentForm(MainForm), m_nExpiryCountdown(0)
{
  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp) {
    TRLCaption(PasswLbl);
    TRLCaption(ConfirmPasswLbl);
    TRLCaption(RememberPasswCheck);
    TRLCaption(KeyFileLbl);
    KeyFileBox->Items->Strings[0] = TRL(KeyFileBox->Items->Strings[0]);
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
    TRLHint(BrowseBtn);
    TRLHint(CreateKeyFileBtn);
  }

  OpenDlg->Filter = FormatW("%s (*.key)|*.key|%s (*.*)|*.*",
    TRL("Key files").c_str(),
    TRL("All files").c_str());
  SaveDlg->Filter = OpenDlg->Filter;

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
  //DisplayPasswCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "DisplayPassw", false);
  //DisplayPasswCheckClick(this);
  //RememberPasswCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "RememberPassw", false);
  RememberPasswTimeSpinBtn->Position = g_pIni->ReadInteger(CONFIG_ID,
      "RememberPasswTime", 60);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  //g_pIni->WriteBool(CONFIG_ID, "DisplayPassw", DisplayPasswCheck->Checked);
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
void __fastcall TPasswEnterDlg::OKBtnClick(TObject *Sender)
{
  WString sErrMsg;
  if (m_nFlags & PASSWENTER_FLAG_ENABLEKEYFILE) {
    if (GetEditBoxTextLen(PasswBox) == 0 && KeyFileBox->ItemIndex == 0)
      sErrMsg = TRL("Enter a password and/or select a key file.");
    else if (KeyFileBox->ItemIndex > 0 && !FileExists(KeyFileBox->Text))
      sErrMsg = TRL("The selected key file does not exist.");
  }
  else if (GetEditBoxTextLen(PasswBox) == 0)
    sErrMsg = TRL("Password is empty.");

  if (!sErrMsg.IsEmpty()) {
    MsgBox(sErrMsg, MB_ICONERROR);
    PasswBox->SetFocus();
    return;
  }

  if (m_nFlags & PASSWENTER_FLAG_CONFIRMPASSW) {
    if (GetPassw(0) != GetPassw(1)) {
      MsgBox(TRL("Passwords are not identical."), MB_ICONERROR);
      PasswBox->SetFocus();
      return;
    }
  }

  if ((m_nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE) && RememberPasswCheck->Checked)
  {
    m_sEncryptedPassw = GetPassw();

    RandomPool::GetInstance()->GetData(memcryptKey, sizeof(memcryptKey));
    memcrypt(m_sEncryptedPassw.Bytes(), m_sEncryptedPassw.Bytes(),
      m_sEncryptedPassw.SizeBytes(), memcryptKey, sizeof(memcryptKey));

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

  TogglePasswBtn->Down = true;
  TogglePasswBtnClick(this);

  ConfirmPasswLbl->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;
  ConfirmPasswBox->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;

  OldVersionCheck->Checked = false;
  OldVersionCheck->Enabled = nFlags & PASSWENTER_FLAG_ENABLEOLDVER;

  RememberPasswCheck->Enabled = blPasswCache;
  RememberPasswTimeBox->Enabled = blPasswCache;
  RememberPasswTimeSpinBtn->Enabled = blPasswCache;

  bool blKeyFile = nFlags & PASSWENTER_FLAG_ENABLEKEYFILE;
  KeyFileLbl->Enabled = blKeyFile;
  KeyFileBox->Enabled = blKeyFile;
  BrowseBtn->Enabled = blKeyFile;
  CreateKeyFileBtn->Enabled = blKeyFile &&
    (nFlags & PASSWENTER_FLAG_ENABLEKEYFILECREATION);
  //KeyFileBox->ItemIndex = 0;

  m_nFlags = nFlags;
  m_pParentForm = (pParentForm == NULL) ? MainForm : pParentForm;

  return ShowModal();
}
//---------------------------------------------------------------------------
SecureWString __fastcall TPasswEnterDlg::GetPassw(int nPassw)
{
  SecureWString sDest;
  if ((m_nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE) && !m_sEncryptedPassw.IsEmpty())
  {
    sDest.New(m_sEncryptedPassw.Size());
    memcrypt(m_sEncryptedPassw.Bytes(), sDest.Bytes(),
      m_sEncryptedPassw.SizeBytes(), memcryptKey, sizeof(memcryptKey));
  }
  else if (nPassw == 0)
    sDest = GetEditBoxTextBuf(PasswBox);
  else
    sDest = GetEditBoxTextBuf(ConfirmPasswBox);
  return sDest;
}
//---------------------------------------------------------------------------
SecureMem<word8> __fastcall TPasswEnterDlg::GetPasswBinary(void)
{
  SecureWString sPassw = GetPassw();
  return sPassw.IsStrEmpty() ? SecureMem<word8>() :
    SecureMem<word8>(sPassw.Bytes(), sPassw.StrLenBytes()); // *without* terminating zero!
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::Clear(void)
{
  ClearEditBoxTextBuf(PasswBox, 256);
  if (m_nFlags & PASSWENTER_FLAG_CONFIRMPASSW)
    ClearEditBoxTextBuf(ConfirmPasswBox, 256);
  KeyFileBox->ItemIndex = 0;
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
void __fastcall TPasswEnterDlg::BrowseBtnClick(TObject *Sender)
{
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = OpenDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (blSuccess) {
    int nIndex = KeyFileBox->Items->IndexOf(OpenDlg->FileName);
    KeyFileBox->ItemIndex = (nIndex < 0) ?
      nIndex = KeyFileBox->Items->Add(OpenDlg->FileName) : nIndex;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::CreateKeyFileBtnClick(TObject *Sender)
{
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (blSuccess) {
    try {
      WString sFileName = SaveDlg->FileName;
      PasswDatabase::CreateKeyFile(sFileName);
      MsgBox(TRL("Key file successfully created."), MB_ICONINFORMATION);
      int nIndex = KeyFileBox->Items->IndexOf(sFileName);
      KeyFileBox->ItemIndex = (nIndex < 0) ?
        KeyFileBox->Items->Add(sFileName) : nIndex;
    }
    catch (Exception& e) {
      MsgBox(TRLFormat("Error while creating key file:\n%s.", e.Message.c_str()),
        MB_ICONERROR);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::TogglePasswBtnClick(TObject *Sender)
{
  PasswBox->PasswordChar = TogglePasswBtn->Down ? PASSWORD_CHAR : '\0';
  ConfirmPasswBox->PasswordChar = PasswBox->PasswordChar;
}
//---------------------------------------------------------------------------
WString __fastcall TPasswEnterDlg::GetKeyFileName(void)
{
  return ((m_nFlags & PASSWENTER_FLAG_ENABLEKEYFILE) &&
    KeyFileBox->ItemIndex > 0) ? KeyFileBox->Text : WString();
}
//---------------------------------------------------------------------------
