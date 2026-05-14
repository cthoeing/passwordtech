// PasswEnter.cpp
//
// PASSWORD TECH
// Copyright (c) 2002-2026 by Christian Thoeing <c.thoeing@web.de>
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
#include "RandomPool.h"
#include "zxcvbn.h"
#include "UnicodeUtil.h"
//---------------------------------------------------------------------
#pragma link "cgauges"
#pragma resource "*.dfm"
TPasswEnterDlg *PasswEnterDlg;

const char
PASSWORD_CHAR = '*';

const WString
CONFIG_ID     = "PasswEnter";

static word8 memcryptKey[16];

void AutoClearPasswDlg::Clear()
{
  if (!m_blCleared) {
    PasswEnterDlg->Clear();
    RandomPool::GetInstance().Flush();
    m_blCleared = true;
  }
}

//---------------------------------------------------------------------
__fastcall TPasswEnterDlg::TPasswEnterDlg(TComponent* AOwner)
  : TForm(AOwner), m_nExpiryCountdown(0), m_blEstPasswQuality(false)
{
  SetFormComponentsAnchors(this);

  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp) {
    TRLCaption(PasswLbl);
    TRLCaption(ConfirmPasswLbl);
    TRLCaption(RememberPasswCheck);
    TRLCaption(PasswQualityCheck);
    TRLCaption(KeyFileLbl);
    TRLCaption(SkipBtn);
    KeyFileBox->Items->Strings[0] = TRL(KeyFileBox->Items->Strings[0]);
    KeyFileBox->ItemIndex = 0;
    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
    TRLHint(TogglePasswBtn);
    TRLHint(BrowseBtn);
    TRLHint(CreateKeyFileBtn);
  }

  OpenDlg->Filter = FormatW("%1 (*.key)|*.key|%2 (*.*)|*.*",
    { TRL("Key files"), TRL("All files") });
  SaveDlg->Filter = OpenDlg->Filter;

  LoadConfig();
}
//---------------------------------------------------------------------------
__fastcall TPasswEnterDlg::~TPasswEnterDlg()
{
  ClearPasswCache();
  Clear();
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::LoadConfig(void)
{
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
  RememberPasswTimeSpinBtn->Position = g_pIni->ReadInteger(CONFIG_ID,
      "RememberPasswTime", 60);
  PasswQualityCheck->Checked = g_pIni->ReadBool(CONFIG_ID,
    "EstimatePasswQuality", true);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteInteger(CONFIG_ID, "RememberPasswTime",
    RememberPasswTimeSpinBtn->Position);
  g_pIni->WriteBool(CONFIG_ID, "EstimatePasswQuality", PasswQualityCheck->Checked);
}
//---------------------------------------------------------------------
void __fastcall TPasswEnterDlg::ClearPasswCache(void)
{
  if (!m_sEncryptedPassw.IsEmpty()) {
    m_sEncryptedPassw.Clear();
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
    if (GetEditBoxTextLen(PasswBox) == 0 && KeyFileBox->ItemIndex <= 0)
      sErrMsg = TRL("Enter a password and/or select a key file.");
    else if (KeyFileBox->ItemIndex > 0 && !FileExists(KeyFileBox->Text)) {
      sErrMsg = TRLFormat("The selected key file does not exist:\n%1",
        { KeyFileBox->Text });
    }
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

    RandomPool::GetInstance().GetData(memcryptKey, sizeof(memcryptKey));
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
  TForm* pParentForm,
  bool blUpdateScreenPos)
{
  if (PasswBox->GetTextLen() != 0) {
    Clear();
#ifdef _DEBUG
    ShowMessage("Passwords not cleared after closing!");
#endif
  }

  bool blPasswCache = nFlags & PASSWENTER_FLAG_ENABLEPASSWCACHE;

  if (blPasswCache && !m_sEncryptedPassw.IsEmpty()) {
    // reset countdown upon every request
    m_nExpiryCountdown = RememberPasswTimeSpinBtn->Position;
    return mrOk;
  }

  if (!sTitle.IsEmpty()) {
    Caption = sTitle;
  }
  else if (nFlags & PASSWENTER_FLAG_ENCRYPT) {
    Caption = TRL("Encrypt");
  }
  else if (nFlags & PASSWENTER_FLAG_DECRYPT) {
    Caption = TRL("Decrypt");
  }
  else {
    Caption = TRL("Enter password");
  }

  PasswQualityCheck->Enabled = nFlags & (PASSWENTER_FLAG_PASSWQUALITY |
    PASSWENTER_FLAG_ENCRYPT);
  PasswGauge->Progress = 0;
  PasswInfoLbl->Caption = WString();

  TogglePasswBtn->Down = true;
  TogglePasswBtnClick(this);

  ConfirmPasswLbl->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;
  ConfirmPasswBox->Enabled = nFlags & PASSWENTER_FLAG_CONFIRMPASSW;

  OldVersionCheck->Checked = false;
  OldVersionCheck->Enabled = nFlags & PASSWENTER_FLAG_ENABLEOLDVER;

  RememberPasswCheck->Checked = false;
  RememberPasswCheck->Enabled = blPasswCache;
  RememberPasswTimeBox->Enabled = blPasswCache;
  RememberPasswTimeSpinBtn->Enabled = blPasswCache;
  RememberPasswCheckClick(this);

  bool blKeyFile = nFlags & PASSWENTER_FLAG_ENABLEKEYFILE;
  KeyFileLbl->Enabled = blKeyFile;
  KeyFileBox->Enabled = blKeyFile;
  BrowseBtn->Enabled = blKeyFile;
  CreateKeyFileBtn->Enabled = blKeyFile &&
    (nFlags & PASSWENTER_FLAG_ENABLEKEYFILECREATION);

  SkipBtn->Visible = nFlags & PASSWENTER_FLAG_ALLOWSKIP;

  m_nFlags = nFlags;
  if (!pParentForm)
    pParentForm = MainForm;

  int nTop, nLeft, nHeight, nWidth;
  if (!blUpdateScreenPos) {
    auto it = m_formDim.find(pParentForm);
    if (it != m_formDim.end()) {
      nTop = it->second[0];
      nLeft = it->second[1];
      nHeight = it->second[2];
      nWidth = it->second[3];
    }
    else
      blUpdateScreenPos = true;
  }

  if (blUpdateScreenPos) {
    nTop = pParentForm->Top;
    nLeft = pParentForm->Left;
    nHeight = pParentForm->Height;
    nWidth = pParentForm->Width;
    m_formDim[pParentForm] = std::array<int,4>{nTop,nLeft,nHeight,nWidth};
  }

  Top = nTop + (nHeight - Height) / 2;
  Left = nLeft + (nWidth - Width) / 2;

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
SecureMem<word8> __fastcall TPasswEnterDlg::GetPasswBinary(bool blUtf8)
{
  SecureWString sPassw = GetPassw();
  if (sPassw.IsStrEmpty())
    return SecureMem<word8>();
  // get password data without terminating zero!
  if (blUtf8) {
    SecureAnsiString sUtf8 = WStringToUtf8_s(sPassw);
    return SecureMem<word8>(sUtf8.Bytes(), sUtf8.StrLenBytes());
  }
  return SecureMem<word8>(sPassw.Bytes(), sPassw.StrLenBytes());
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::Clear(void)
{
  PasswBox->Tag = 1;
  ClearEditBoxTextBuf(PasswBox, 256);
  PasswBox->Tag = 0;
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
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::RememberPasswCheckClick(TObject *Sender)
{
  bool blEnabled = RememberPasswCheck->Checked;
  RememberPasswTimeBox->Enabled = blEnabled;
  RememberPasswTimeSpinBtn->Enabled = blEnabled;
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
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = OpenDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

  if (blSuccess) {
    int nIndex = KeyFileBox->Items->IndexOf(OpenDlg->FileName);
    KeyFileBox->ItemIndex = (nIndex > 0) ? nIndex :
      KeyFileBox->Items->Add(OpenDlg->FileName);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::CreateKeyFileBtnClick(TObject *Sender)
{
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

  if (blSuccess) {
    try {
      WString sFileName = SaveDlg->FileName;
      PasswDatabase::CreateKeyFile(sFileName);
      MsgBox(TRL("Key file successfully created."), MB_ICONINFORMATION);
      int nIndex = KeyFileBox->Items->IndexOf(sFileName);
      KeyFileBox->ItemIndex = (nIndex > 0) ? nIndex :
        KeyFileBox->Items->Add(sFileName);
    }
    catch (Exception& e) {
      MsgBox(TRLFormat("Error while creating key file:\n%1.", { e.Message }),
        MB_ICONERROR);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::TogglePasswBtnClick(TObject *Sender)
{
  PasswBox->Tag = 1;
  PasswBox->PasswordChar = TogglePasswBtn->Down ? PASSWORD_CHAR : '\0';
  PasswBox->Tag = 0;
  ConfirmPasswBox->PasswordChar = PasswBox->PasswordChar;
}
//---------------------------------------------------------------------------
WString __fastcall TPasswEnterDlg::GetKeyFileName(void)
{
  return ((m_nFlags & PASSWENTER_FLAG_ENABLEKEYFILE) &&
    KeyFileBox->ItemIndex > 0) ? KeyFileBox->Text : WString();
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::OnEndSession(void)
{
  if (!m_sEncryptedPassw.IsEmpty()) {
    m_sEncryptedPassw.Clear();
    memzero(memcryptKey, sizeof(memcryptKey));
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::PasswBoxChange(TObject *Sender)
{
  if (!(m_nFlags & (PASSWENTER_FLAG_PASSWQUALITY | PASSWENTER_FLAG_ENCRYPT)) ||
      PasswBox->Tag != 0 || !Visible)
    return;

  if (PasswBox->GetTextLen() == 0) {
    PasswGauge->Progress = 0;
    PasswInfoLbl->Caption = WString();
    return;
  }

  WString sInfo;
  SecureWString sPassw = GetPassw();
  if (m_blEstPasswQuality) {
    double dPasswEntropyBits = g_config.UseAdvancedPasswEst ?
      ZxcvbnMatch(WStringToUtf8_s(sPassw).c_str(), nullptr, nullptr) :
      PasswordGenerator::EstimatePasswSecurity(sPassw);

    SetPasswQualityBar(PasswGauge, dPasswEntropyBits);

    sInfo = IntToStr(FloorEntropyBits(dPasswEntropyBits)) + " / ";
  }
  sInfo += TRLFormat("%1 ch.", { IntToStr(GetNumOfUnicodeChars(sPassw)) });
  PasswInfoLbl->Caption = sInfo;
}
//---------------------------------------------------------------------------
void __fastcall TPasswEnterDlg::PasswQualityCheckClick(TObject *Sender)
{
  m_blEstPasswQuality = PasswQualityCheck->Enabled && PasswQualityCheck->Checked;
  PasswGauge->Enabled = m_blEstPasswQuality;
  if (!m_blEstPasswQuality)
    PasswGauge->Progress = 0;
  PasswBoxChange(this);
}
//---------------------------------------------------------------------------

