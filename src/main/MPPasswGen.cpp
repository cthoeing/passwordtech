// MPPasswGen.cpp
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
#include <vcl.h>
#include <Math.hpp>
#include <Clipbrd.hpp>
#pragma hdrstop

#include "MPPasswGen.h"
#include "Main.h"
#include "RandomPool.h"
#include "PasswEnter.h"
#include "Language.h"
#include "Util.h"
#include "CryptUtil.h"
#include "sha1.h"
#include "base64.h"
#include "FastPRNG.h"
#include "dragdrop.h"
#include "TopMostManager.h"
#include "PasswManager.h"
#include "SendKeys.h"
#include "AESCtrPRNG.h"
#include "sha256.h"
#include "SecureClipboard.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMPPasswGenForm *MPPasswGenForm;

const char PASSWORD_CHAR = '*';

const int
  MPPG_CHARSETS_NUM = 6,
  PASSW_MAX_CHARS   = 10000,
  PASSW_DEFAULT_LEN = 16,

  PASSW_HASH_DEC99    = 0,
  PASSW_HASH_DEC9999  = 1,
  PASSW_HASH_HEX16BIT = 2,
  PASSW_HASH_HEX32BIT = 3;

const char* MPPG_CHARSETS[MPPG_CHARSETS_NUM] =
{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
  "ACEFHJKLMNPRTUVWXYabcdefghijkmnopqrstuvwxyz3479",
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
  "ACEFHJKLMNPRTUVWXYabcdefghijkmnopqrstuvwxyz3479!\"#$%&'()*+,-./:;<=>?@[\\]^_`{}~",
  "0123456789abcdef",
  "0123456789ABCDEF"
};


// functions that provide compatibility with "Hashapass"
void unicodeToLSByte(const wchar_t* pwszSrc,
  char* pszDest,
  int nLen)
{
  while (nLen--)
    *pszDest++ = static_cast<char>(*pwszSrc++);
}

void asciiToUnicode(const char* pszSrc,
  wchar_t* pwszDest,
  int nLen)
{
  while (nLen--)
    *pwszDest++ = *pszSrc++;
}

const WString
CONFIG_ID = "MPPasswGen";

static word8 memcryptKey[16];

//---------------------------------------------------------------------------
__fastcall TMPPasswGenForm::TMPPasswGenForm(TComponent* Owner)
  : TForm(Owner)
{
  SetFormComponentsAnchors(this);

  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  PasswLengthSpinBtn->Max = PASSW_MAX_CHARS;
  TStrings* pStrList = CharSetList->Items;

  pStrList->Add(FormatW("1: %1 (A-Z, a-z, 0-9)", { TRL("Alphanumeric") }));
  pStrList->Add(FormatW("2: (1) %1", { TRL("without ambiguous characters") }));
  pStrList->Add(FormatW("3: (1) + %1 (!\"#$%...)", { TRL("special symbols") }));
  pStrList->Add(FormatW("4: (3) %1", { TRL("without ambiguous characters") }));
  pStrList->Add(FormatW("5: %1 (0-9, a-f)", { TRL("Hexadecimal") }));
  pStrList->Add(FormatW("6: %1 (0-9, A-F)", { TRL("Hexadecimal") }));

  MPPasswGenForm->PasswBox->Font = MainForm->PasswBox->Font;

  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(MasterPasswGroup);
    TRLCaption(EnterPasswBtn);
    TRLCaption(ClearKeyBtn);
    TRLCaption(PasswStatusLbl);
    TRLCaption(KeyExpiryInfoLbl);
    TRLCaption(ConfirmPasswCheck);
    TRLCaption(ShowPasswHashCheck);
    TRLCaption(KeyExpiryCheck);
    TRLCaption(HashapassCompatCheck);
    TRLCaption(AddPasswLenToParamCheck);
    TRLCaption(AutotypeLbl);
    TRLCaption(PasswGeneratorGroup);
    TRLCaption(ParameterLbl);
    TRLCaption(ClearParameterBtn);
    TRLCaption(CharSetLbl);
    TRLCaption(LengthLbl);
    TRLCaption(ResultingPasswLbl);
    TRLCaption(GenerateBtn);
    TRLCaption(UseAsDefaultRNGBtn);
    TRLCaption(CloseBtn);
    TRLHint(TogglePasswBtn);

    TRLMenu(PasswBoxMenu);
  }

  PasswSecurityBar->Hint = MainForm->PasswSecurityBar->Hint;

  RegisterDropWindow(PasswBox->Handle, &m_pPasswBoxDropTarget);
  RegisterDropWindow(ParameterBox->Handle, &m_pParamBoxDropTarget);

  LoadConfig();
}
//---------------------------------------------------------------------------
__fastcall TMPPasswGenForm::~TMPPasswGenForm()
{
  UnregisterDropWindow(ParameterBox->Handle, m_pParamBoxDropTarget);
  UnregisterDropWindow(PasswBox->Handle, m_pPasswBoxDropTarget);
  memzero(memcryptKey, sizeof(memcryptKey));
  ClearEditBoxTextBuf(PasswBox, 256);
  ClearEditBoxTextBuf(ParameterBox, 256);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::LoadConfig(void)
{
  int nTop = g_pIni->ReadInteger(CONFIG_ID, "WindowTop", INT_MAX);
  int nLeft = g_pIni->ReadInteger(CONFIG_ID, "WindowLeft", INT_MAX);

  if (nTop == INT_MAX || nLeft == INT_MAX)
    Position = poScreenCenter;
  else {
    Top = nTop;
    Left = nLeft;
  }

  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);

  ConfirmPasswCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "ConfirmPassw", false);
  ShowPasswHashCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "ShowPasswHash", true);

  int nHashIdx = g_pIni->ReadInteger(CONFIG_ID, "PasswHashType", PASSW_HASH_HEX16BIT);
  if (nHashIdx >= 0 && nHashIdx <= PASSW_HASH_HEX32BIT)
    PasswHashList->ItemIndex = nHashIdx;

  ShowPasswHashCheckClick(this);

  KeyExpiryCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "KeyExpiry", true);
  KeyExpiryCheckClick(this);

  KeyExpiryTimeSpinBtn->Position =
    g_pIni->ReadInteger(CONFIG_ID, "KeyExpiryTime", 300);

  HashapassCompatCheck->Checked =
    g_pIni->ReadBool(CONFIG_ID, "HashapassCompatible", false);
  HashapassCompatCheckClick(this);

  AddPasswLenToParamCheck->Checked =
    g_pIni->ReadBool(CONFIG_ID, "AddPasswLenToParam", false);

  AutotypeBox->Text = g_pIni->ReadString(CONFIG_ID, "AutotypeSequence",
    "{parameter}{tab}{password}{enter}");

  int nCharSetIdx = g_pIni->ReadInteger(CONFIG_ID, "CharSetListIdx", 0);
  if (nCharSetIdx < 0 || nCharSetIdx >= MPPG_CHARSETS_NUM)
    nCharSetIdx = 0;
  CharSetList->ItemIndex = nCharSetIdx;
  CharSetListChange(this);

  PasswLengthSpinBtn->Position = g_pIni->ReadInteger(CONFIG_ID, "PasswLength",
      PASSW_DEFAULT_LEN);
  TogglePasswBtn->Down = g_pIni->ReadBool(CONFIG_ID, "HidePassw", false);
  TogglePasswBtnClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteBool(CONFIG_ID, "ConfirmPassw", ConfirmPasswCheck->Checked);
  g_pIni->WriteBool(CONFIG_ID, "ShowPasswHash", ShowPasswHashCheck->Checked);
  g_pIni->WriteInteger(CONFIG_ID, "PasswHashType", PasswHashList->ItemIndex);
  g_pIni->WriteBool(CONFIG_ID, "KeyExpiry", KeyExpiryCheck->Checked);
  g_pIni->WriteInteger(CONFIG_ID, "KeyExpiryTime", KeyExpiryTimeSpinBtn->Position);
  g_pIni->WriteBool(CONFIG_ID, "HashapassCompatible", HashapassCompatCheck->Checked);
  g_pIni->WriteBool(CONFIG_ID, "AddPasswLenToParam", AddPasswLenToParamCheck->Checked);
  g_pIni->WriteString(CONFIG_ID, "AutotypeSequence", AutotypeBox->Text);
  g_pIni->WriteInteger(CONFIG_ID, "CharSetListIdx", CharSetList->ItemIndex);
  g_pIni->WriteInteger(CONFIG_ID, "PasswLength", PasswLengthSpinBtn->Position);
  g_pIni->WriteBool(CONFIG_ID, "HidePassw", TogglePasswBtn->Down);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ClearKey(bool blExpired,
  bool blClearKeyOnly)
{
  m_key.Clear();
  memzero(memcryptKey, sizeof(memcryptKey));

  if (!blClearKeyOnly) {
    ClearEditBoxTextBuf(PasswBox, 256);
    ClearEditBoxTextBuf(ParameterBox, 256);
    PasswSecurityBarPanel->Visible = false;
    PasswInfoLbl->Visible = false;
  }

  KeyExpiryTimer->Enabled = false;
  m_nExpiryCountdown = 0;
  KeyExpiryCountdownLbl->Caption = "";
  GenerateBtn->Enabled = false;
  UseAsDefaultRNGBtn->Enabled = false;

  TDateTime currTime = TDateTime::CurrentTime();
  WString sInfo = currTime.TimeString() + WString(" ");

  if (blExpired)
    sInfo += TRL("Key has expired.");
  else
    sInfo += TRL("Key has been cleared.");

  PasswStatusBox->Text = sInfo;
  HashapassCompatCheck->Enabled = true;
  ClearKeyBtn->Enabled = blClearKeyOnly;
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::SetKeyExpiry(bool blSetup)
{
  bool blExpire = KeyExpiryCheck->Checked;
  bool blTimeZero = KeyExpiryTimeSpinBtn->Position == 0;

  if (blExpire && blTimeZero && !blSetup)
    ClearKey(true, true);
  else if (blExpire && !blTimeZero) {
    KeyExpiryTimer->Enabled = true;
    m_nExpiryCountdown = KeyExpiryTimeSpinBtn->Position;
    KeyExpiryCountdownLbl->Caption = IntToStr(m_nExpiryCountdown);
  }
  else {
    KeyExpiryTimer->Enabled = false;
    KeyExpiryCountdownLbl->Caption = "";
  }
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::EnterPasswBtnClick(TObject *Sender)
{
  SecureWString sPassw;
  int nFlags = 0;
  if (ConfirmPasswCheck->Checked)
    nFlags |= PASSWENTER_FLAG_CONFIRMPASSW;
  bool blSuccess = PasswEnterDlg->Execute(nFlags, TRL("Master password"), this) == mrOk;
  if (blSuccess)
    sPassw = PasswEnterDlg->GetPassw();

  PasswEnterDlg->Clear();
  RandomPool::GetInstance().Flush();

  if (!blSuccess)
    return;

  TDateTime currTime = TDateTime::CurrentTime();

  WString sPasswInfo = currTime.TimeString() + WString(" ") +
    TRL("New key generated.");

  bool blHashapass = HashapassCompatCheck->Checked;
  SecureMem<word8> tempKey(AESCtrPRNG::KEY_SIZE);

  if (blHashapass) {
    m_key.New(sPassw.StrLen());
    unicodeToLSByte(sPassw, reinterpret_cast<char*>(m_key.Data()), m_key.Size());
  }

  sha256_hmac(sPassw.Bytes(), sPassw.StrLenBytes(),
    reinterpret_cast<const word8*>(MPPG_KEYGEN_SALTSTR),
    sizeof(MPPG_KEYGEN_SALTSTR) - 1, tempKey.Data(), 0);

  if (!blHashapass)
    m_key = tempKey;

  // encrypt the key before storing it in memory
  g_fastRandGen.GetData(memcryptKey, sizeof(memcryptKey));
  memcrypt(m_key, m_key, m_key.Size(), memcryptKey, sizeof(memcryptKey));

  if (ShowPasswHashCheck->Checked) {
    SecureMem<word8> hash(32);
    sha256(tempKey, tempKey.Size(), hash, 0);

    //word32 lHashValue = *reinterpret_cast<word32*>(hash.Data());
    int nHashType = PasswHashList->ItemIndex;
    word32 lHashValue = 0;
    for (int i = 0; i < (nHashType == PASSW_HASH_HEX16BIT ? 2 : 4); i++)
      lHashValue = (lHashValue << 8) | hash[i];

    WString sHashFmt;
    switch (nHashType) {
    case PASSW_HASH_DEC99:
      lHashValue %= 100;
      sHashFmt = "%.2d";
      break;
    case PASSW_HASH_DEC9999:
      lHashValue %= 10000;
      sHashFmt = "%.4d";
      break;
    case PASSW_HASH_HEX16BIT:
      lHashValue &= 0xffff;
      sHashFmt = "%.4x";
      break;
    case PASSW_HASH_HEX32BIT:
      sHashFmt = "%.8x";
      break;
    }
    sPasswInfo += " Hash: " + Format(sHashFmt,
      ARRAYOFCONST((lHashValue))).LowerCase();
  }

  PasswStatusBox->Text = sPasswInfo;

  ClearKeyBtn->Enabled = true;
  GenerateBtn->Enabled = true;
  UseAsDefaultRNGBtn->Enabled = true;
  HashapassCompatCheck->Enabled = false;
  UseAsDefaultRNGBtn->Enabled = !blHashapass;

  SetKeyExpiry(true);

  ParameterBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::KeyExpiryTimerTimer(TObject *Sender)
{
  m_nExpiryCountdown--;
  KeyExpiryCountdownLbl->Caption = IntToStr(m_nExpiryCountdown);

  if (m_nExpiryCountdown == 0)
    ClearKey(true);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ClearKeyBtnClick(TObject *Sender)
{
  ClearKey();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::GenerateBtnClick(TObject *Sender)
{
  if (m_key.IsEmpty())
    return;

  double dPasswBits;

  SecureMem<word8> plainKey(m_key.Size());
  memcrypt(m_key, plainKey, plainKey.Size(), memcryptKey, sizeof(memcryptKey));

  WString sParam = ParameterBox->Text;
  int nParamLen = sParam.Length();

  if (HashapassCompatCheck->Checked) {
    AnsiString asParam;

    if (nParamLen != 0) {
      asParam.SetLength(nParamLen);
      unicodeToLSByte(sParam.c_str(), asParam.c_str(), nParamLen);
    }

    SecureMem<word8> passwBytes(20);

    sha1_hmac(plainKey.Data(), plainKey.Size(),
      reinterpret_cast<const word8*>(asParam.c_str()), asParam.Length(),
      passwBytes.Data());

    SecureAnsiString asPassw(9);
	  size_t outputLen = 9;

	  base64_encode(reinterpret_cast<word8*>(asPassw.Data()),
	    &outputLen, passwBytes.Data(), 6, 0);

    SecureWString sPassw(9);
    asciiToUnicode(asPassw, sPassw, 9);

    SetEditBoxTextBuf(PasswBox, sPassw);

    dPasswBits = 48;
  }
  else {
    bool addLength = AddPasswLenToParamCheck->Checked;
    word32 lParamBytes = sParam.Length() * sizeof(wchar_t);

    SecureMem<word8> paramData(lParamBytes + (addLength ? 3 : 0));
    memcpy(paramData, reinterpret_cast<word8*>(sParam.c_str()), lParamBytes);

    int nPasswLen = PasswLengthSpinBtn->Position;

    if (addLength) {
      paramData[lParamBytes] = 0;
      paramData[lParamBytes + 1] = nPasswLen & 0xff;
      paramData[lParamBytes + 2] = (nPasswLen >> 8) & 0xff;
    }

    AESCtrPRNG randGen;
    randGen.SeedWithKey(plainKey, plainKey.Size(), paramData, paramData.Size());

    const char* pszCharSet = MPPG_CHARSETS[CharSetList->ItemIndex];
    int nCharSetSize = strlen(pszCharSet);

    SecureWString sPassw(nPasswLen + 1);

    for (int i = 0; i < nPasswLen; i++)
      sPassw[i] = pszCharSet[randGen.GetNumRange(nCharSetSize)];

    sPassw[nPasswLen] = '\0';

    SetEditBoxTextBuf(PasswBox, sPassw);

    dPasswBits = Log2(static_cast<double>(nCharSetSize)) * nPasswLen;
  }

  PasswSecurityBarPanel->Visible = true;
  PasswInfoLbl->Visible = true;
  PasswSecurityBarPanel->Width = std::max<int>((std::min(dPasswBits / 128.0, 1.0) *
        PasswSecurityBar->Width), 4);
  PasswInfoLbl->Caption = TRLFormat("%1 bits",
    { FormatFloat("0.0", std::min<double>(dPasswBits, AESCtrPRNG::KEY_SIZE*8)) });

  SetKeyExpiry();

  PasswBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::KeyExpiryCheckClick(TObject *Sender)
{
  KeyExpiryTimeBox->Enabled = KeyExpiryCheck->Checked;
  KeyExpiryTimeSpinBtn->Enabled = KeyExpiryCheck->Checked;
  if (Visible && KeyExpiryTimeBox->Enabled)
    KeyExpiryTimeBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ClearParameterBtnClick(TObject *Sender)
{
  ParameterBox->Clear();
  ParameterBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::TogglePasswBtnClick(TObject *Sender)
{
  PasswBox->PasswordChar = (TogglePasswBtn->Down) ? PASSWORD_CHAR : '\0';
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::FormClose(TObject *Sender,
  TCloseAction &Action)
{
  if (!m_key.IsEmpty())
    ClearKey();
  if (Visible) {
    TopMostManager::GetInstance().OnFormClose(this);
    if (g_nAppState & APPSTATE_HIDDEN)
      ShowWindow(Application->Handle, SW_HIDE);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::CloseBtnClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::UseAsDefaultRNGBtnClick(TObject *Sender)
{
  if (m_key.IsEmpty())
    return;

  SecureMem<word8> plainKey(m_key.Size());
  memcrypt(m_key, plainKey, plainKey.Size(), memcryptKey, sizeof(memcryptKey));

  if (!g_pKeySeededPRNG)
    g_pKeySeededPRNG.reset(new AESCtrPRNG);

  WString sParam = ParameterBox->Text;
  g_pKeySeededPRNG->SeedWithKey(plainKey, plainKey.Size(),
    reinterpret_cast<word8*>(sParam.c_str()), sParam.Length() * sizeof(wchar_t));

  MainForm->UseKeySeededPRNG();
  SetKeyExpiry();
//  WindowState = wsMinimized;
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::HashapassCompatCheckClick(TObject *Sender)
{
  bool blEnabled = !HashapassCompatCheck->Checked;
  CharSetLbl->Enabled = blEnabled;
  CharSetList->Enabled = blEnabled;
  CharSetInfoLbl->Enabled = blEnabled;
  LengthLbl->Enabled = blEnabled;
  PasswLengthBox->Enabled = blEnabled;
  PasswLengthSpinBtn->Enabled = blEnabled;
  AddPasswLenToParamCheck->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ParameterBoxKeyPress(TObject *Sender,
  char &Key)
{
  if (Key == VK_RETURN)
    GenerateBtnClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::FormActivate(TObject *Sender)
{
  if (m_key.IsEmpty())
    EnterPasswBtn->SetFocus();
  else
    ParameterBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::CharSetListChange(TObject *Sender)
{
  int nLen = strlen(MPPG_CHARSETS[CharSetList->ItemIndex]);
  double dEntropy = Log2(static_cast<double>(nLen));

  CharSetInfoLbl->Caption = TRLFormat("%1 ch. / %2 bits per ch.",
    { IntToStr(nLen), FormatFloat("0.0", dEntropy) });
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenuPopup(TObject *Sender)
{
  bool blSelected = PasswBox->SelLength != 0;
  bool blHasText = PasswBox->GetTextLen() != 0;

  PasswBoxMenu_Undo->Enabled = PasswBox->CanUndo;
  PasswBoxMenu_Cut->Enabled = blSelected;
  PasswBoxMenu_Copy->Enabled = blSelected;
  PasswBoxMenu_EncryptCopy->Enabled = blSelected;
  PasswBoxMenu_Paste->Enabled = Clipboard()->HasFormat(CF_TEXT);
  PasswBoxMenu_PerformAutotype->Enabled = blHasText;
  PasswBoxMenu_AddToDatabase->Enabled = blHasText && PasswMngForm->IsDbOpen();
  PasswBoxMenu_Delete->Enabled = blSelected;
  PasswBoxMenu_SelectAll->Enabled = blHasText;
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_UndoClick(TObject *Sender)
{
  PasswBox->Undo();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_CutClick(TObject *Sender)
{
  if (g_config.AutoClearClip) {
    SecureWString sCut = GetEditBoxSelTextBuf(PasswBox);
    SecureClipboard::GetInstance().SetData(sCut.c_str());
    PasswBox->SetSelTextBuf(const_cast<wchar_t*>(L""));
  }
  else
    PasswBox->CutToClipboard();
  //MainForm->CopiedSensitiveDataToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_CopyClick(TObject *Sender)
{
  if (g_config.AutoClearClip) {
    SecureWString sCopy = GetEditBoxSelTextBuf(PasswBox);
    SecureClipboard::GetInstance().SetData(sCopy.c_str());
  }
  else
    PasswBox->CopyToClipboard();
  //MainForm->CopiedSensitiveDataToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_EncryptCopyClick(
  TObject *Sender)
{
  SecureWString sText = GetEditBoxSelTextBuf(PasswBox);
  MainForm->CryptText(true, &sText);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_PasteClick(TObject *Sender)
{
  PasswBox->PasteFromClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_DeleteClick(TObject *Sender)
{
  PasswBox->SelText = "";
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_SelectAllClick(
  TObject *Sender)
{
  PasswBox->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswSecurityBarMouseMove(TObject *Sender,
  TShiftState Shift, int X, int Y)
{
  if (Shift.Contains(ssLeft))
    StartEditBoxDragDrop(PasswBox);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ParameterLblMouseMove(TObject *Sender,
  TShiftState Shift, int X, int Y)
{
  if (Shift.Contains(ssLeft))
    StartEditBoxDragDrop(ParameterBox);
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_AddToDatabaseClick(TObject *Sender)
{
  WString sParam = ParameterBox->Text;
  SecureWString sPassw = GetEditBoxTextBuf(PasswBox);
  PasswMngForm->AddPassw(sPassw, false, sParam.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::ShowPasswHashCheckClick(TObject *Sender)
{
  PasswHashList->Enabled = ShowPasswHashCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::PasswBoxMenu_PerformAutotypeClick(TObject *Sender)

{
  SecureWString sPassw = GetEditBoxTextBuf(PasswBox),
    sParam = GetEditBoxTextBuf(ParameterBox);
  if (g_config.MinimizeAutotype) {
    g_nAppState |= APPSTATE_AUTOTYPE;
    Application->Minimize();
    SendKeys sk(g_config.AutotypeDelay);
    sk.SendComplexString(AutotypeBox->Text, nullptr, nullptr, sParam.c_str(),
      sPassw.c_str());
    g_nAppState &= ~APPSTATE_AUTOTYPE;
  }
  else {
    TSendKeysThread::TerminateAndWait();
    if (!TSendKeysThread::ThreadRunning())
      new TSendKeysThread(Handle, g_config.AutotypeDelay, AutotypeBox->Text,
        nullptr, nullptr, sParam.c_str(), sPassw.c_str());
  }
}
//---------------------------------------------------------------------------
void __fastcall TMPPasswGenForm::OnEndSession(void)
{
  if (!m_key.IsEmpty()) {
    m_key.Clear();
    memzero(memcryptKey, sizeof(memcryptKey));
  }
}
//---------------------------------------------------------------------------

