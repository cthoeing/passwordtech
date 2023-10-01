// ProfileEditor.cpp
//
// PASSWORD TECH
// Copyright (c) 2002-2023 by Christian Thoeing <c.thoeing@web.de>
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

#include "ProfileEditor.h"
#include "Main.h"
#include "Util.h"
#include "Language.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProfileEditDlg *ProfileEditDlg;

static const WString
CONFIG_ID = "ProfileEditor";

//---------------------------------------------------------------------------
__fastcall TProfileEditDlg::TProfileEditDlg(TComponent* Owner)
  : TForm(Owner)
{
  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(LoadBtn);
    TRLCaption(DeleteBtn);
    TRLCaption(ConfirmCheck);
    TRLCaption(ProfileNameLbl);
    TRLCaption(AddBtn);
    TRLCaption(SaveAdvancedOptionsCheck);
    TRLCaption(CloseBtn);
    TRLHint(MoveUpBtn);
    TRLHint(MoveDownBtn);
  }

  LoadConfig();

  for (const auto& pProfile : g_profileList) {
    WString sProfileName = pProfile->ProfileName;
    if (pProfile->AdvancedOptionsUsed)
      sProfileName += WString(" [+]");

    ProfileList->Items->Add(sProfileName);
  }
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
  ConfirmCheck->Checked = g_pIni->ReadBool(CONFIG_ID, "Confirm", true);
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteBool(CONFIG_ID, "Confirm", ConfirmCheck->Checked);
}
//---------------------------------------------------------------------------
int __fastcall TProfileEditDlg::Execute(bool blExitAfterAdd)
{
  /*
    for (int nI = 0; nI < ProfileList->Count; nI++)
      ProfileList->Selected[nI] = false;

    LoadBtn->Enabled = false;
    DeleteBtn->Enabled = false;
  */

  m_blModified = m_blAdded = false;
  m_blExitAfterAdd = blExitAfterAdd;

  ProfileList->ClearSelection();
  ProfileNameBox->Text = WString();

  ShowModal();

  int nFlags = 0;
  if (m_blModified)
    nFlags |= MODIFIED;
  if (m_blAdded)
    nFlags |= ADDED;

  return nFlags;
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::ProfileListClick(TObject *Sender)
{
  if (ProfileList->SelCount == 0) {
    LoadBtn->Enabled = false;
    DeleteBtn->Enabled = false;
    MoveUpBtn->Enabled = false;
    MoveDownBtn->Enabled = false;
  }
  else if (ProfileList->SelCount == 1) {
    auto pProfile = g_profileList[ProfileList->ItemIndex].get();

    ProfileNameBox->Text = pProfile->ProfileName;
    SaveAdvancedOptionsCheck->Checked = pProfile->AdvancedOptionsUsed;

    LoadBtn->Enabled = true;
    DeleteBtn->Enabled = true;

    int nCount = ProfileList->Count;
    MoveUpBtn->Enabled = nCount > 1 && ProfileList->ItemIndex > 0;
    MoveDownBtn->Enabled = nCount > 1 && ProfileList->ItemIndex < nCount - 1;
  }
  else {
    LoadBtn->Enabled = false;
    DeleteBtn->Enabled = true;
    MoveUpBtn->Enabled = false;
    MoveDownBtn->Enabled = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::ProfileNameBoxChange(TObject *Sender)
{
  AddBtn->Enabled = GetEditBoxTextLen(ProfileNameBox) != 0;
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::LoadBtnClick(TObject *Sender)
{
  if (ProfileList->ItemIndex >= 0)
    MainForm->LoadProfile(ProfileList->ItemIndex);
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::DeleteBtnClick(TObject *Sender)
{
  if (ProfileList->SelCount == 0)
    return;

  if (ConfirmCheck->Checked) {
    WString sMsg;
    if (ProfileList->SelCount == 1)
      sMsg = TRLFormat("Delete profile \"%s\"?",
        ProfileList->Items->Strings[ProfileList->ItemIndex].c_str());
    else
      sMsg = TRLFormat("Delete %d profiles?", ProfileList->SelCount);
    if (MsgBox(sMsg, MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDNO)
      return;
  }

  int nI = 0;
  int nLast = 0;

  while (nI < ProfileList->Count) {
    if (ProfileList->Selected[nI]) {
      MainForm->DeleteProfile(nI);
      ProfileList->Items->Delete(nI);
      nI = nLast;
    }
    else
      nLast = ++nI;
  }

  ProfileListClick(this);
  m_blModified = true;
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::AddBtnClick(TObject *Sender)
{
  if (ProfileNameBox->GetTextLen() == 0)
    return;

  WString sProfileName = ProfileNameBox->Text;
  bool blSaveAdvancedOptions = SaveAdvancedOptionsCheck->Checked;
  int nFoundIdx = FindPWGenProfile(sProfileName);

  if (nFoundIdx < 0 && g_profileList.size() == PROFILES_MAX_NUM) {
    MsgBox(TRLFormat("Maximum number of profiles (%d) reached.\nPlease delete "
        "or overwrite profiles.", PROFILES_MAX_NUM),
      MB_ICONWARNING);
    return;
  }

  if (nFoundIdx >= 0) {
    if (ConfirmCheck->Checked &&
      MsgBox(TRLFormat("A profile with the name \"%s\" already exists.\n"
          "Do you want to overwrite it?",
          g_profileList[nFoundIdx]->ProfileName.c_str()),
        MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDNO) {
      ProfileNameBox->SetFocus();
      return;
    }
  }

  MainForm->CreateProfile(sProfileName, blSaveAdvancedOptions, nFoundIdx);

  if (blSaveAdvancedOptions)
    sProfileName += WString(" [+]");

  if (nFoundIdx < 0)
    ProfileList->Items->Add(sProfileName);
  else
    ProfileList->Items->Strings[nFoundIdx] = sProfileName;

  ProfileNameBox->Clear();
  ProfileNameBox->SetFocus();
  m_blModified = true;
  m_blAdded = true;

  if (m_blExitAfterAdd)
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::ProfileNameBoxKeyPress(TObject *Sender,
  char &Key)
{
  if (Key == VK_RETURN) {
    AddBtnClick(this);
    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::FormActivate(TObject *Sender)
{
  ProfileNameBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::ProfileListDblClick(TObject *Sender)
{
  LoadBtn->Click();
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::MoveUpBtnClick(TObject *Sender)
{
  int nIdx = ProfileList->ItemIndex;
  if (nIdx > 0) {
    ProfileList->Items->Move(nIdx, nIdx - 1);
    std::swap(g_profileList[nIdx], g_profileList[nIdx - 1]);
    ProfileList->ItemIndex = --nIdx;
    MoveUpBtn->Enabled = nIdx > 0;
    MoveDownBtn->Enabled = true;
  }
  m_blModified = true;
}
//---------------------------------------------------------------------------
void __fastcall TProfileEditDlg::MoveDownBtnClick(TObject *Sender)
{
  int nIdx = ProfileList->ItemIndex;
  if (nIdx >= 0 && nIdx < ProfileList->Count - 1) {
    ProfileList->Items->Move(nIdx, nIdx + 1);
    std::swap(g_profileList[nIdx], g_profileList[nIdx + 1]);
    ProfileList->ItemIndex = ++nIdx;
    MoveUpBtn->Enabled = true;
    MoveDownBtn->Enabled = nIdx < ProfileList->Count - 1;
  }
  m_blModified = true;
}
//---------------------------------------------------------------------------

