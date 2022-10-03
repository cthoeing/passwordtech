// About.cpp
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

#include "About.h"
#include "Main.h"
#include "Language.h"
#include "ProgramDef.h"
#include "Util.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAboutForm *AboutForm;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
  : TForm(Owner)
{
  const WString HTML_LINK = "<a href=\"%s\">%s</a>";
  Caption = TRLFormat("About %s", PROGRAM_NAME);
#ifdef _DEBUG
  ProgramLbl->Caption = PROGRAM_NAME + WString(" (debug)");
#else
  ProgramLbl->Caption = PROGRAM_NAME;
#endif
  VersionLbl->Caption = WString("Version ") + WString(PROGRAM_VERSION) +
#ifdef _WIN64
    " (64-bit)";
#else
    " (32-bit)";
#endif
  WString sRef = WString("mailto:") + WString(PROGRAM_AUTHOR_EMAIL);
  AuthorLink->Caption = WString(PROGRAM_COPYRIGHT) + " " +
    FormatW(HTML_LINK, sRef.c_str(), WString(PROGRAM_AUTHOR).c_str());
  //AuthorLink->Hint = sRef;
  sRef = PROGRAM_URL_WEBSITE;
  WWWLink->Caption = FormatW(HTML_LINK, sRef.c_str(), sRef.c_str());
  sRef = PROGRAM_LICENSEFILE;
  LicenseLink->Caption = FormatW(HTML_LINK, (g_sExePath + sRef).c_str(),
    TRL("View license").c_str());
  LicenseLink->Hint = sRef;
  if (g_pLangSupp) {
    TRLCaption(LicenseLbl);
    TRLCaption(OKBtn);
  }

  WString sLangName = TRLDEF(LANGUAGE_NAME, "English");
  WString sLangVer = TRLDEF(LANGUAGE_VERSION, "");
  if (!sLangVer.IsEmpty())
    sLangName += WString(" (v") + sLangVer + WString(")");
  WString sTransl = TRLDEF(LANGUAGE_AUTHOR, "(original language)");
  LanguageInfoLbl->Caption = TRLFormat("Current language: %s\n"
      "Translator: %s", sLangName.c_str(), sTransl.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormShow(TObject *Sender)
{
  //if (!g_config.DonorId.IsEmpty())
  //  DonorLbl->Caption = WString("DONOR ID: ") + WString(g_config.DonorId);
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::LinkClick(TObject *Sender, const UnicodeString Link,
          TSysLinkType LinkType)
{
  ExecuteShellOp(Link);
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::SetDonorUI(void)
{
  if (g_asDonorInfo.IsEmpty())
    AboutForm->DonorLbl->Caption = "Community";
  else
    AboutForm->DonorLbl->Caption = TRL("Donor ID:") + " " + g_asDonorInfo;
}
//---------------------------------------------------------------------------
