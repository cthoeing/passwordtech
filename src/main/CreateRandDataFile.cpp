// CreateRandDataFile.cpp
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
#include "TaskCancel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCreateRandDataFileDlg *CreateRandDataFileDlg;

const WString
CONFIG_ID = "CreateRandDataFile";

//---------------------------------------------------------------------------
__fastcall TCreateRandDataFileDlg::TCreateRandDataFileDlg(TComponent* Owner)
  : TForm(Owner)
{
  SetFormComponentsAnchors(this);

  Constraints->MaxHeight = Height;
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp) {
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

  word8 bDriveLetter = UpCase(sFileName[1]);
  if (bDriveLetter >= 'A' && bDriveLetter <= 'Z' &&
      static_cast<word64>(DiskFree(
        static_cast<word8>(bDriveLetter - 'A' + 1))) < qFileSize) {
    MsgBox(TRL("Not enough free disk space available\nto create the file."),
      MB_ICONERROR);
    return;
  }

  std::unique_ptr<RandomPool> pRandPool;
  //std::unique_ptr<SplitMix64> pFastRandGen;
  RandomGenerator* pRandSrc;

  if (IsRandomPoolActive()) {
    //pFastRandGen.reset(new SplitMix64(g_fastRandGen.GetWord64()));
    pRandPool.reset(new RandomPool(RandomPool::GetInstance()));
    pRandPool->Randomize();
    pRandSrc = pRandPool.get();
    EntropyManager::GetInstance().ConsumeEntropyBits(RandomPool::MAX_ENTROPY);
  }
  else
    pRandSrc = g_pRandSrc;

  WString sMsg;
  word64 qTotalWritten = 0;
  //std::atomic<bool> cancelFlag(false);
  TaskCancelToken cancelToken;
  auto progressPtr = std::make_shared<std::atomic<word64>>(0);
  //std::atomic<word64> progress(0);
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
  Screen->Cursor = crHourGlass;

  auto pTask = TTask::Create([&](){
    try {
      std::unique_ptr<TFileStream> pFile(new TFileStream(sFileName, fmCreate));

      SecureMem<word8> randBuf(qFileSize >= 10485760 ? 1048576 : 65536);

      word64 qRestToWrite = qFileSize;

      while (qRestToWrite > 0 && !cancelToken) {
        int nBytesToWrite = std::min<word64>(qRestToWrite, randBuf.Size());

        pRandSrc->GetData(randBuf, nBytesToWrite);

#ifdef _DEBUG
        int nBytesWritten = nBytesToWrite;
#else
        int nBytesWritten = pFile->Write(randBuf, nBytesToWrite);
#endif
        qTotalWritten += nBytesWritten;
        qRestToWrite -= nBytesWritten;
        *progressPtr = qTotalWritten / qProgressStep;

        if (nBytesWritten < nBytesToWrite)
          OutOfDiskSpaceError();
      }
    }
    catch (Exception& e) {
      sMsg = e.Message;
    }
  });

  //Enabled = false;

  pTask->Start();

  int nTimeout = 0;
  while (!pTask->Wait(100)) {
    Application->ProcessMessages();
    nTimeout += 100;
    if (nTimeout >= 1000 && !ProgressForm->Visible) {
      WString sProgressInfo = ReplaceStr(TRL("%1 of %2 %3 written."), "%3", sProgressStep);
      ProgressForm->ExecuteModal(this,
        TRL("Creating random data file ..."),
        sProgressInfo,
        cancelToken.Get(),
        [&pTask](unsigned int timeout)
        {
          return pTask->Wait(timeout);
        },
        qFileSize / qProgressStep,
        progressPtr);
      break;
    }
  }

  if (!cancelToken || cancelToken.Reason == TaskCancelReason::UserCancel) {
    if (cancelToken || !sMsg.IsEmpty()) {
      if (cancelToken)
        sMsg = EUserCancel::UserCancelMsg;
      sMsg = TRLFormat(
        "Error while creating file\n\"%1\":\n%2.\n\n%3 bytes written.",
        { sFileName, sMsg, IntToStr(static_cast<__int64>(qTotalWritten)) });
      MsgBox(sMsg, MB_ICONERROR);
    }
    else {
      MsgBox(TRLFormat(
        "File \"%1\" successfully created.\n\n%2 bytes written.",
        { ExtractFileName(sFileName),
          IntToStr(static_cast<__int64>(qTotalWritten)) }), MB_ICONINFORMATION);
    }

    Screen->Cursor = crDefault;
    MainForm->UpdateEntropyProgress();
  }
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::BrowseBtnClick(TObject *Sender)
{
  MainForm->SaveDlg->FilterIndex = 1;

  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = MainForm->SaveDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

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
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TCreateRandDataFileDlg::FormActivate(TObject *Sender)
{
  FileNameBox->SetFocus();
}
//---------------------------------------------------------------------------

