// PasswList.cpp
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

#include "PasswList.h"
#include "Main.h"
#include "Util.h"
#include "Language.h"
#include "StringFileStreamW.h"
#include "TopMostManager.h"
#include "PasswManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswListForm *PasswListForm;

static const WString
CONFIG_ID = "PasswList";

//---------------------------------------------------------------------------
__fastcall TPasswListForm::TPasswListForm(TComponent* Owner)
  : TForm(Owner)
{
  if (g_pLangSupp) {
    TRLCaption(this);
    TRLMenu(PasswListMenu);
  }
  LoadConfig();
}
//---------------------------------------------------------------------------
__fastcall TPasswListForm::~TPasswListForm()
{
  ClearEditBoxTextBuf(PasswList);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::LoadConfig(void)
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

  StringToFont(g_pIni->ReadString(CONFIG_ID, "Font", ""), PasswList->Font);
  FontDlg->Font = PasswList->Font;
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteString(CONFIG_ID, "Font", FontToString(PasswList->Font));
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::Execute(void)
{
  PasswList->SelStart = 0;
  Show();
  if (WindowState == wsMinimized)
    WindowState = wsNormal;
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_CopyClick(TObject *Sender)
{
  PasswList->CopyToClipboard();
  MainForm->CopiedSensitiveDataToClipboard();
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_SelectAllClick(TObject *Sender)
{
  PasswList->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_SaveAsFileClick(TObject *Sender)
{
  MainForm->SaveDlg->Title = TRL("Save password list");
  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = MainForm->SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();
  if (!blSuccess)
    return;

  WString sFileName = MainForm->SaveDlg->FileName;
  //TStringFileStreamW* pFile = NULL;

  Screen->Cursor = crHourGlass;
  blSuccess = false;
  WString sMsg;

  try {
    SecureWString sPasswList = GetEditBoxTextBuf(PasswList);

    if (g_sNewline == "\n") {
      SecureWString sNewList(sPasswList.Size());
      const wchar_t* pwszSrc = sPasswList.c_str();
      wchar_t* pwszDest = sNewList.Data();
      while (*pwszSrc != '\0') {
        if (pwszSrc[0] == '\r' && pwszSrc[1] == '\n') {
          *pwszDest++ = '\n';
          pwszSrc += 2;
        }
        else
          *pwszDest++ = *pwszSrc++;
      }
      *pwszDest = '\0';
      //memset(pwszDest, 0, (sNewList.Size() - (pwszDest - sNewList.Data()) + 1) *
      //  sizeof(wchar_t));
      sPasswList = sNewList;
    }

    std::unique_ptr<TStringFileStreamW> pFile(new TStringFileStreamW(
        sFileName, fmCreate, g_config.FileEncoding, true, PASSW_MAX_BYTES));

    int nBytesWritten;
    if (!pFile->WriteString(sPasswList, sPasswList.StrLen(), nBytesWritten))
      OutOfDiskSpaceError();

    blSuccess = true;
    sMsg = TRLFormat("File \"%s\" successfully created.",
      ExtractFileName(sFileName).c_str());
  }
  catch (Exception& e) {
    sMsg = TRLFormat("Error while creating file\n\"%s\":\n%s.", sFileName.c_str(),
        e.Message.c_str());
  }

  Screen->Cursor = crDefault;

  MsgBox(sMsg, (blSuccess) ? MB_ICONINFORMATION : MB_ICONERROR);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenuPopup(TObject *Sender)
{
  bool blSelected = PasswList->SelLength != 0;

  PasswListMenu_Copy->Enabled = blSelected;
  PasswListMenu_EncryptCopy->Enabled = blSelected;
  PasswListMenu_AddToDb->Enabled = blSelected && PasswMngForm->IsDbOpen();
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_ChangeFontClick(TObject *Sender)
{
  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  if (FontDlg->Execute())
    PasswList->Font = FontDlg->Font;
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  ClearEditBoxTextBuf(PasswList);
  TopMostManager::GetInstance()->OnFormClose(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::FormKeyPress(TObject *Sender, char &Key)
{
  if (Key == VK_ESCAPE)
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_EncryptCopyClick(
  TObject *Sender)
{
  SecureWString sText = GetRichEditSelTextBuf(PasswList);
  MainForm->CryptText(true, &sText, this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListStartDrag(TObject *Sender,
  TDragObject *&DragObject)
{
  StartEditBoxDragDrop(PasswList);
}
//---------------------------------------------------------------------------
void __fastcall TPasswListForm::PasswListMenu_AddToDbClick(TObject *Sender)
{
  SecureWString sPasswList = GetRichEditSelTextBuf(PasswList);
  PasswMngForm->AddPassw(sPasswList, true);
}
//---------------------------------------------------------------------------

