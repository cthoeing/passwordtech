// CreateTrigramFile.cpp
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

#include "CreateTrigramFile.h"
#include "Language.h"
#include "Main.h"
#include "PasswGen.h"
#include "Util.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCreateTrigramFileDlg *CreateTrigramFileDlg;

static const WString
CONFIG_ID = "CreateTrigramFile";

//---------------------------------------------------------------------------
__fastcall TCreateTrigramFileDlg::TCreateTrigramFileDlg(TComponent* Owner)
  : TForm(Owner)
{
  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp != NULL) {
    TRLCaption(this);
    TRLCaption(SourceFileLbl);
    TRLCaption(DestFileLbl);
    TRLCaption(CreateFileBtn);
    TRLCaption(CloseBtn);
    TRLHint(BrowseBtn);
    TRLHint(BrowseBtn2);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::LoadConfig(void)
{
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::FormActivate(TObject *Sender)
{
  SourceFileBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::BrowseBtnClick(TObject *Sender)
{
  MainForm->OpenDlg->FilterIndex = 1;

  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = MainForm->OpenDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (!blSuccess)
    return;

  SourceFileBox->Text = MainForm->OpenDlg->FileName;
  SourceFileBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::BrowseBtn2Click(TObject *Sender)
{
  MainForm->SaveDlg->FilterIndex = 3;

  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = MainForm->SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (!blSuccess)
    return;

  WString sFileName = MainForm->SaveDlg->FileName;
  if (ExtractFileExt(sFileName).IsEmpty())
    sFileName += ".tgm";

  DestFileBox->Text = sFileName;
  DestFileBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::CreateFileBtnClick(TObject *Sender)
{
  WString sSrcFileName = SourceFileBox->Text;
  WString sDestFileName = DestFileBox->Text;

  if (ExtractFilePath(sSrcFileName).IsEmpty())
    sSrcFileName = g_sExePath + sSrcFileName;

  if (ExtractFilePath(sDestFileName).IsEmpty())
    sDestFileName = g_sExePath + sDestFileName;

  Screen->Cursor = crHourGlass;
  bool blSuccess = false;
  WString sMsg;

  try {
    word32 lNumOfTris;
    double dEntropy;
    PasswordGenerator::CreateTrigramFile(sSrcFileName, sDestFileName,
      &lNumOfTris, &dEntropy);

    blSuccess = true;
    sMsg = TRLFormat("Trigram file \"%s\" successfully created.\n\n%d trigrams "
        "evaluated.\n%1.2f bits of entropy per letter.",
        ExtractFileName(sDestFileName).c_str(),
        lNumOfTris, dEntropy);
  }
  catch (Exception& e) {
    sMsg = TRLFormat("Error while creating file\n\"%s\":\n%s.", sDestFileName.c_str(),
        e.Message.c_str());
  }

  Screen->Cursor = crDefault;
  MsgBox(sMsg, blSuccess ? MB_ICONINFORMATION : MB_ICONERROR);
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::SourceFileBoxChange(TObject *Sender)
{
  CreateFileBtn->Enabled = !SourceFileBox->Text.IsEmpty() &&
    !DestFileBox->Text.IsEmpty();
}
//---------------------------------------------------------------------------
void __fastcall TCreateTrigramFileDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------

