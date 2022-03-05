// CreateRandDataFile.cpp
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
#include <SysUtils.hpp>
#include <System.Threading.hpp>
#include <System.StrUtils.hpp>
#pragma hdrstop

#include "CreateRandDataFile.h"
#include "EntropyManager.h"
#include "Main.h"
#include "Progress.h"
#include "Util.h"
#include "Language.h"
#include "TopMostManager.h"
#include "hrtimer.h"
#include "FastPRNG.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCreateRandDataFileDlg *CreateRandDataFileDlg;

static const WString
CONFIG_ID = "CreateRandDataFile";

//---------------------------------------------------------------------------
__fastcall TCreateRandDataFileDlg::TCreateRandDataFileDlg(TComponent* Owner)
  : TForm(Owner)
{
  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp != NULL) {
    TRLCaption(this);
    TRLCaption(FileNameLbl);
    TRLCaption(FileSizeLbl);
    TRLCaption(CloseBtn);
    TRLCaption(CreateFileBtn);
    TRLHint(BrowseBtn);

    for (int nI = 0; nI < SizeUnitList->Items->Count; nI++)
      SizeUnitList->Items->Strings[nI] = TRL(SizeUnitList->Items->Strings[nI]);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::LoadConfig(void)
{
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
  FileSizeBox->Text = g_pIni->ReadString(CONFIG_ID, "FileSize", "");

  int nSizeUnitIdx = g_pIni->ReadInteger(CONFIG_ID, "FileSizeUnitIdx", 0);
  SizeUnitList->ItemIndex = (nSizeUnitIdx >= 0 &&
    nSizeUnitIdx < SizeUnitList->Items->Count) ? nSizeUnitIdx : 0;
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteString(CONFIG_ID, "FileSize", FileSizeBox->Text);
  g_pIni->WriteInteger(CONFIG_ID, "FileSizeUnitIdx", SizeUnitList->ItemIndex);
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::CreateFileBtnClick(TObject *Sender)
{
  WString sFileName = FileNameBox->Text;
  if (ExtractFilePath(sFileName).IsEmpty())
    sFileName = g_sExePath + sFileName;

  word64 qFileSize = 0;
  WString sText = FileSizeBox->Text;
  int nIdx = SizeUnitList->ItemIndex;
  if (sText.Pos(".") > 0) {
    TFormatSettings fs;
    fs.DecimalSeparator = '.';
    double dFileSize = StrToFloatDef(sText, 0, fs);
    while (nIdx--)
      dFileSize *= 1024;
    qFileSize = static_cast<word64>(dFileSize);
  }
  else {
    qFileSize = StrToInt64Def(FileSizeBox->Text, 0);
    while (nIdx--)
      qFileSize *= 1024;
  }

  // limit file size to 10TB
  if (qFileSize == 0 || qFileSize > 0xa0000000000ll) {
    MsgBox(TRL("Invalid file size."), MB_ICONERROR);
    FileSizeBox->SetFocus();
    return;
  }

  //size_t fileSize = static_cast<size_t>(dFileSize);

  word8 bDriveLetter = UpCase(sFileName[1]);
  if (bDriveLetter >= 'A' && bDriveLetter <= 'Z' &&
      DiskFree(static_cast<word8>(bDriveLetter - 'A' + 1)) < qFileSize) {
    MsgBox(TRL("Not enough free disk space available\nto create the file."),
      MB_ICONERROR);
    return;
  }

  TTask::Run([=](){
    std::unique_ptr<RandomPool> pRandPool;
    std::unique_ptr<SplitMix64> pFastRandGen;
    RandomGenerator* pRandSrc;
    if (IsRandomPoolActive()) {
      TThread::Synchronize(0, [&]() {
        pFastRandGen.reset(new SplitMix64(g_fastRandGen.GetWord64()));
        pRandPool.reset(new RandomPool(
          *RandomPool::GetInstance(), *pFastRandGen, false));
        pRandPool->Randomize();
        pRandSrc = pRandPool.get();
        EntropyManager::GetInstance()->ConsumeEntropyBits(RandomPool::MAX_ENTROPY);
        Enabled = false;
      });
    }
    else
      pRandSrc = g_pRandSrc;

    bool blSuccess = false;
    String sMsg;
    word64 qProgressStep;
    WString sProgressStep;
    if (qFileSize >= 104857600) {
      qProgressStep = 1048576;
      sProgressStep = "MB";
    }
    else {
      qProgressStep = 1024;
      sProgressStep = "KB";
    }
    word64 qTotalWritten = 0;
    std::atomic<bool> cancelFlag;
    std::atomic<word64> progress;

    try {
      std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmCreate));

      SecureMem<word8> randBuf(qFileSize >= 10485760 ? 1048576 : 65536);

      word64 qRestToWrite = qFileSize;
      Stopwatch clock;
      bool progFormInit = false;

      while (qRestToWrite > 0 && !cancelFlag) {
        if (!progFormInit && clock.ElapsedSeconds() > 1) {
          WString sProgressInfo = TRL("%d of %d KB written.");
          if (sProgressStep != "KB")
            sProgressInfo = ReplaceStr(sProgressInfo, "KB", sProgressStep);
          TThread::Synchronize(0, [&](){
            ProgressForm->Init(this, TRL("Creating random data file ..."),
              sProgressInfo, qFileSize / qProgressStep,
              &cancelFlag, &progress);
            Screen->Cursor = crAppStart;
          });
          progFormInit = true;
        }

        int nBytesToWrite = std::min<word64>(qRestToWrite, randBuf.Size());

        pRandSrc->GetData(randBuf, nBytesToWrite);

        int nBytesWritten = pFile->Write(randBuf, nBytesToWrite);
        qTotalWritten += nBytesWritten;
        qRestToWrite -= nBytesWritten;
        progress = qTotalWritten / qProgressStep;

        if (nBytesWritten < nBytesToWrite)
          OutOfDiskSpaceError();
      }

      if (cancelFlag)
        sMsg = EUserCancel::UserCancelMsg;
      else {
        sMsg = FormatW(ReplaceStr(TRL(
          "File \"%s\" successfully created.\n\n%d bytes written."), "%d", "%llu"),
          ExtractFileName(sFileName).c_str(), qTotalWritten);
        blSuccess = true;
      }
    }
    catch (Exception& e) {
      sMsg = e.Message;
    }

    if (!blSuccess) {
      sMsg = FormatW(ReplaceStr(
        TRL("Error while creating file\n\"%s\":\n%s.\n\n%d bytes written."),
        "%d", "%llu"), sFileName.c_str(), sMsg.c_str(), qTotalWritten);
    }

    TThread::Synchronize(0, [&](){
      Enabled = true;
      Screen->Cursor = crDefault;
      MainForm->UpdateEntropyProgress();
      MsgBox(sMsg, blSuccess ? MB_ICONINFORMATION : MB_ICONERROR);
      ProgressForm->Terminate();
    });
  });
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::BrowseBtnClick(TObject *Sender)
{
  MainForm->SaveDlg->FilterIndex = 1;

  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = MainForm->SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (!blSuccess)
    return;

  FileNameBox->Text = MainForm->SaveDlg->FileName;
  FileNameBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::FileNameBoxChange(TObject *Sender)
{
  CreateFileBtn->Enabled = (GetEditBoxTextLen(FileNameBox) != 0) &&
    (GetEditBoxTextLen(FileSizeBox) != 0);
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::FormActivate(TObject *Sender)
{
  FileNameBox->SetFocus();
}
//---------------------------------------------------------------------------

