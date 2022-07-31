// PasswMngDbSettings.cpp
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
#include <vcl.h>
#pragma hdrstop

#include "PasswMngDbSettings.h"
#include "Language.h"
#include "Main.h"
#include "Util.h"
#include "TopMostManager.h"
#include "PasswManager.h"
#include "FastPRNG.h"
#include "CryptUtil.h"
#include "hrtimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswDbSettingsDlg *PasswDbSettingsDlg;

static const int NUM_CIPHERS = 2;
static const wchar_t* CIPHER_NAMES[NUM_CIPHERS] =
{
  L"Advanced Encryption Standard (AES-CBC)",
  L"ChaCha20"
};

static const int CIPHER_KEY_SIZES[NUM_CIPHERS] =
{
  256, 256
};

static const WString CONFIG_ID = "PasswMngDbSettings";

//---------------------------------------------------------------------------
__fastcall TPasswDbSettingsDlg::TPasswDbSettingsDlg(TComponent* Owner)
  : TForm(Owner)
{
  for (int i = 0; i < NUM_CIPHERS; i++) {
    WString sCipher = TRLFormat("%s with %d-bit key", CIPHER_NAMES[i],
      CIPHER_KEY_SIZES[i]);
    EncryptionAlgoList->Items->Add(sCipher);
  }

  if (g_pLangSupp != nullptr) {
    TRLCaption(this);
    TRLCaption(GeneralSheet);
    TRLCaption(SecuritySheet);
    TRLCaption(DefUserNameLbl);
    TRLCaption(PasswFormatSeqLbl);
    TRLCaption(EncryptionAlgoLbl);
    TRLCaption(NumKdfRoundsLbl);
    TRLCaption(DefaultExpiryLbl);

    TRLHint(PasswGenTestBtn);
    TRLHint(CalcRoundsBtn);

    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }

  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::GetSettings(PasswDbSettings& s)
{
  s.DefaultUserName = GetEditBoxTextBuf(DefUserNameBox);
  s.PasswFormatSeq = GetEditBoxTextBuf(PasswFormatSeqBox);
  s.DefaultExpiryDays = DefaultExpiryUpDown->Position;
  s.CipherType = EncryptionAlgoList->ItemIndex;
  s.NumKdfRounds = StrToUInt(NumKdfRoundsBox->Text);
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::SetSettings(const PasswDbSettings& s)
{
  SetEditBoxTextBuf(DefUserNameBox, s.DefaultUserName.c_str());
  SetEditBoxTextBuf(PasswFormatSeqBox, s.PasswFormatSeq.c_str());
  DefaultExpiryUpDown->Position = s.DefaultExpiryDays;
  EncryptionAlgoList->ItemIndex = s.CipherType;
  NumKdfRoundsBox->Text = IntToStr(static_cast<__int64>(s.NumKdfRounds));
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::FormShow(TObject *Sender)
{
  Top = PasswMngForm->Top + (PasswMngForm->Height - Height) / 2;
  Left = PasswMngForm->Left + (PasswMngForm->Width - Width) / 2;

  TopMostManager::GetInstance()->SetForm(this);

  ConfigPages->ActivePage = GeneralSheet;

  PasswGenTestBox->Text = TRL("Test:");
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::OKBtnClick(TObject *Sender)
{
  word32 lKdfRounds = StrToUInt(NumKdfRoundsBox->Text);
  if (lKdfRounds == 0) {
    MsgBox(TRL("Invalid number of key derivation rounds."), MB_ICONERROR);
    return;
  }
  PasswDbSettings settings;
  GetSettings(settings);
  if (PasswMngForm->ApplyDbSettings(settings))
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::CalcRoundsBtnClick(TObject *Sender)
{
  Screen->Cursor = crHourGlass;

  try {
    word8 key[32], salt[32], result[32];
    g_fastRandGen.GetData(key, sizeof(key));
    g_fastRandGen.GetData(salt, sizeof(salt));

    const int ROUGH_EST_ROUNDS = 10000;

    Stopwatch clock;
    pbkdf2_256bit(key, sizeof(key), salt, sizeof(salt), result, ROUGH_EST_ROUNDS);
    word32 lRoughEstimate = lround(ROUGH_EST_ROUNDS / clock.ElapsedSeconds());

    clock.Reset();
    pbkdf2_256bit(key, sizeof(key), salt, sizeof(salt), result, lRoughEstimate);

    word32 lRoundsFor1s = std::max<word32>(1,
      lround(lRoughEstimate / clock.ElapsedSeconds()));

    NumKdfRoundsBox->Text = IntToStr(static_cast<__int64>(lRoundsFor1s));
  }
  __finally {
    Screen->Cursor = crDefault;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::FormActivate(TObject *Sender)
{
  DefUserNameBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::PasswGenTestBtnClick(TObject *Sender)
{
  WString sPasswTest = TRL("Test:");

  if (PasswFormatSeqBox->GetTextLen() != 0) {
    static PasswordGenerator passwGen(&g_fastRandGen);

    SecureWString sFormat = GetEditBoxTextBuf(PasswFormatSeqBox);

    w32string sFormat32 = WCharToW32String(sFormat.c_str());

    SecureW32String sDest(16001);
    if (passwGen.GetFormatPassw(sDest, sDest.Size() - 1, sFormat32, 0) != 0)
    {
      W32CharToWCharInternal(sDest);
      sPasswTest += " " + WString(reinterpret_cast<wchar_t*>(sDest.Data()));
    }

    eraseStlString(sFormat32);
  }

  PasswGenTestBox->Text = sPasswTest;
}
//---------------------------------------------------------------------------
void __fastcall TPasswDbSettingsDlg::FormClose(TObject *Sender, TCloseAction &Action)

{
  ClearEditBoxTextBuf(DefUserNameBox);
  ClearEditBoxTextBuf(PasswFormatSeqBox);
  ClearEditBoxTextBuf(PasswGenTestBox);
}
//---------------------------------------------------------------------------

