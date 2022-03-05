// Configuration.cpp
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

#include "Configuration.h"
#include "Main.h"
#include "Util.h"
#include "Language.h"
#include "TopMostManager.h"
#include "FastPRNG.h"
#include "hrtimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigurationDlg *ConfigurationDlg;

const WString
  CONFIG_ID = "ConfigWin";

const int NUM_RANDOM_POOL_CIPHERS = 3;
const wchar_t* RANDOM_POOL_CIPHER_NAMES[NUM_RANDOM_POOL_CIPHERS] =
{
  L"AES-CTR",
  L"ChaCha20",
  L"ChaCha8",
//  L"Speck128"
};

const int RANDOM_POOL_CIPHER_INFO[NUM_RANDOM_POOL_CIPHERS][2] =
{
  { 256, 128 },
  { 256, 512 },
  { 256, 512 },
//  { 256, 128 }
};

//---------------------------------------------------------------------------
__fastcall TConfigurationDlg::TConfigurationDlg(TComponent* Owner)
  : TForm(Owner)
{
  //Constraints->MaxHeight = Height;
  //Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  AutoClearClipTimeSpinBtn->Min = AUTOCLEARCLIPTIME_MIN;
  AutoClearClipTimeSpinBtn->Max = AUTOCLEARCLIPTIME_MAX;

  for (int nI = 0; nI < NUM_RANDOM_POOL_CIPHERS; nI++) {
    WString sCipher = TRLFormat("%s (%d-bit key, operates on %d-bit blocks)",
      RANDOM_POOL_CIPHER_NAMES[nI],
      RANDOM_POOL_CIPHER_INFO[nI][0], RANDOM_POOL_CIPHER_INFO[nI][1]);
    RandomPoolCipherList->Items->Add(sCipher);
  }

  if (g_pLangSupp != NULL) {
    TRLCaption(this);
    TRLCaption(GeneralSheet);
    TRLCaption(ChangeFontLbl);
    TRLCaption(SelectFontBtn);
    TRLCaption(ShowSysTrayIconConstCheck);
    TRLCaption(MinimizeToSysTrayCheck);
    TRLCaption(MinimizeAutotypeCheck);
    TRLCaption(AutotypeDelayLbl);
    TRLCaption(AskBeforeExitCheck);
    TRLCaption(LaunchSystemStartupCheck);

    TRLCaption(SecuritySheet);
    TRLCaption(RandomPoolCipherLbl);
    TRLCaption(BenchmarkBtn);
    TRLCaption(TestCommonPasswCheck);
    TRLCaption(AutoClearClipCheck);
    TRLCaption(AutoClearPasswCheck);

    TRLCaption(HotKeySheet);
    TRLCaption(HotKeyActionsGroup);
    TRLCaption(HotKeyShowMainWinCheck);
    TRLCaption(HotKeyLbl);
    TRLCaption(AddBtn);
    TRLCaption(RemoveBtn);
    TRLCaption(HotKeyView->Columns->Items[0]);
    TRLCaption(HotKeyView->Columns->Items[1]);

    int nI;
    for (nI = 0; nI < HotKeyActionsList->Items->Count; nI++)
      HotKeyActionsList->Items->Strings[nI] =
        TRL(HotKeyActionsList->Items->Strings[nI]);

    TRLCaption(FilesSheet);
    TRLCaption(FileEncodingLbl);
    TRLCaption(NewlineCharLbl);

    TRLCaption(UpdatesSheet);
    TRLCaption(AutoCheckUpdatesLbl);
    for (nI = 0; nI < AutoCheckUpdatesList->Items->Count; nI++)
      AutoCheckUpdatesList->Items->Strings[nI] =
        TRL(AutoCheckUpdatesList->Items->Strings[nI]);

    TRLCaption(LanguageSheet);
    TRLCaption(SelectLanguageLbl);

    TRLCaption(DatabaseSheet);
    TRLCaption(ClearClipMinimizeCheck);
    TRLCaption(ClearClipExitCheck);
    TRLCaption(LockMinimizeCheck);
    TRLCaption(LockIdleCheck);
    TRLCaption(LockAutoSaveCheck);
    TRLCaption(CreateBackupCheck);
    TRLCaption(NumberBackupsCheck);
    TRLCaption(OpenWindowOnStartupCheck);
    TRLCaption(OpenDbOnStartupCheck);
    TRLCaption(ClearClipMinimizeCheck);
    TRLCaption(DefaultAutotypeSeqLbl);
    TRLCaption(AutoSaveCheck);
    TRLCaption(WarnExpiredEntriesCheck);

    for (nI = 0; nI < AutoSaveList->Items->Count; nI++)
      AutoSaveList->Items->Strings[nI] =
        TRL(AutoSaveList->Items->Strings[nI]);

    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);
  }

  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::LoadConfig(void)
{
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::GetOptions(Configuration& config)
{
  config.GUIFontString = FontToString(FontDlg->Font);
  config.AutoClearClip = AutoClearClipCheck->Checked;
  config.AutoClearClipTime = AutoClearClipTimeSpinBtn->Position;
  config.AutoClearPassw = AutoClearPasswCheck->Checked;
  config.AutoClearPasswTime = AutoClearPasswTimeSpinBtn->Position;
  config.MinimizeAutotype = MinimizeAutotypeCheck->Checked;
  config.AutotypeDelay = AutotypeDelayUpDown->Position;
  config.ConfirmExit = AskBeforeExitCheck->Checked;
  config.LaunchSystemStartup = LaunchSystemStartupCheck->Checked;
  config.RandomPoolCipher = RandomPoolCipherList->ItemIndex;
  config.TestCommonPassw = TestCommonPasswCheck->Checked;
  config.ShowSysTrayIconConst = ShowSysTrayIconConstCheck->Checked;
  config.MinimizeToSysTray = MinimizeToSysTrayCheck->Checked;
  config.AutoCheckUpdates = AutoCheckUpdates(AutoCheckUpdatesList->ItemIndex);
  config.FileEncoding = CharacterEncoding(FileEncodingList->ItemIndex);
  config.FileNewlineChar = NewlineChar(NewlineCharList->ItemIndex);
  config.HotKeys = m_hotKeys;
  config.LanguageIndex = LanguageList->ItemIndex;
  config.Database.ClearClipMinimize = ClearClipMinimizeCheck->Checked;
  config.Database.ClearClipExit = ClearClipExitCheck->Checked;
  config.Database.LockMinimize = LockMinimizeCheck->Checked;
  config.Database.LockIdle = LockIdleCheck->Checked;
  config.Database.LockIdleTime = LockIdleTimeUpDown->Position;
  config.Database.LockAutoSave = LockAutoSaveCheck->Checked;
  config.Database.CreateBackup = CreateBackupCheck->Checked;
  config.Database.NumberBackups = NumberBackupsCheck->Checked;
  config.Database.MaxNumBackups = MaxNumBackupsUpDown->Position;
  config.Database.OpenWindowOnStartup = OpenWindowOnStartupCheck->Checked;
  config.Database.OpenLastDbOnStartup = OpenDbOnStartupCheck->Checked;
  config.Database.WarnExpiredEntries = WarnExpiredEntriesCheck->Checked;
  config.Database.AutoSave = AutoSaveCheck->Checked;
  config.Database.AutoSaveOption = static_cast<AutoSaveDatabase>(
    AutoSaveList->ItemIndex);
  config.Database.DefaultAutotypeSequence = DefaultAutotypeSeqBox->Text;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SetOptions(const Configuration& config)
{
  StringToFont(config.GUIFontString, FontDlg->Font);
  ShowFontSample(FontDlg->Font);
  RandomPoolCipherList->ItemIndex = config.RandomPoolCipher;
  TestCommonPasswCheck->Checked = config.TestCommonPassw;
  AutoClearClipCheck->Checked = config.AutoClearClip;
  AutoClearClipTimeSpinBtn->Position = config.AutoClearClipTime;
  AutoClearClipCheckClick(this);
  AutoClearPasswCheck->Checked = config.AutoClearPassw;
  AutoClearPasswTimeSpinBtn->Position = config.AutoClearPasswTime;
  AutoClearPasswCheckClick(this);
  MinimizeAutotypeCheck->Checked = config.MinimizeAutotype;
  AutotypeDelayUpDown->Position = config.AutotypeDelay;
  ShowSysTrayIconConstCheck->Checked = config.ShowSysTrayIconConst;
  MinimizeToSysTrayCheck->Checked = config.MinimizeToSysTray;
  AskBeforeExitCheck->Checked = config.ConfirmExit;
  LaunchSystemStartupCheck->Checked = config.LaunchSystemStartup;
  AutoCheckUpdatesList->ItemIndex = config.AutoCheckUpdates;
  FileEncodingList->ItemIndex = static_cast<int>(config.FileEncoding);
  NewlineCharList->ItemIndex = static_cast<int>(config.FileNewlineChar);
  LanguageList->ItemIndex = config.LanguageIndex;
  m_hotKeys = config.HotKeys;
  UpdateHotKeyList();
  ClearClipMinimizeCheck->Checked = config.Database.ClearClipMinimize;
  ClearClipExitCheck->Checked = config.Database.ClearClipExit;
  LockMinimizeCheck->Checked = config.Database.LockMinimize;
  LockIdleCheck->Checked = config.Database.LockIdle;
  LockIdleCheckClick(this);
  LockIdleTimeUpDown->Position = config.Database.LockIdleTime;
  LockAutoSaveCheck->Checked = config.Database.LockAutoSave;
  CreateBackupCheck->Checked = config.Database.CreateBackup;
  NumberBackupsCheck->Checked = config.Database.NumberBackups;
  CreateBackupCheckClick(this);
  //NumberBackupsCheckClick(this);
  MaxNumBackupsUpDown->Position = config.Database.MaxNumBackups;
  OpenWindowOnStartupCheck->Checked = config.Database.OpenWindowOnStartup;
  OpenDbOnStartupCheck->Checked = config.Database.OpenLastDbOnStartup;
  WarnExpiredEntriesCheck->Checked = config.Database.WarnExpiredEntries;
  AutoSaveCheck->Checked = config.Database.AutoSave;
  AutoSaveList->ItemIndex = static_cast<int>(config.Database.AutoSaveOption);
  AutoSaveCheckClick(this);
  DefaultAutotypeSeqBox->Text = config.Database.DefaultAutotypeSequence;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SetLanguageList(
  const std::vector<LanguageEntry>& languages)
{
  for (std::vector<LanguageEntry>::const_iterator it = languages.begin();
    it != languages.end(); it++)
  {
    LanguageList->Items->Add(FormatW("%s (v%s)", it->Name.c_str(),
      it->Version.c_str()));
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::ShowFontSample(TFont* pFont)
{
  FontSampleLbl->Caption = FormatW("%s, %dpt", pFont->Name.c_str(), pFont->Size);
  FontSampleLbl->Font = pFont;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::UpdateHotKeyList(void)
{
  HotKeyView->Clear();
  for (HotKeyList::iterator it = m_hotKeys.begin(); it != m_hotKeys.end(); it++)
  {
    TListItem* pItem = HotKeyView->Items->Add();
    pItem->Data = reinterpret_cast<void*>(it->first);
    pItem->Caption = ShortCutToText(it->first);

    WString sAction;
    if (it->second.ShowMainWin)
      sAction = TRL("Show");
    if (it->second.Action != hkaNone) {
      if (!sAction.IsEmpty())
        sAction += ", ";
      sAction += HotKeyActionsList->Items->Strings[static_cast<int>(it->second.Action)];
    }
    pItem->SubItems->Add(sAction);
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SelectFontBtnClick(TObject *Sender)
{
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = FontDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);

  if (blSuccess)
    ShowFontSample(FontDlg->Font);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::AutoClearClipCheckClick(
  TObject *Sender)
{
  AutoClearClipTimeBox->Enabled = AutoClearClipCheck->Checked;
  AutoClearClipTimeSpinBtn->Enabled = AutoClearClipCheck->Checked;
  if (Visible && AutoClearClipTimeBox->Enabled)
    AutoClearClipTimeBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;

  TopMostManager::GetInstance()->SetForm(this);

  ConfigPages->ActivePage = GeneralSheet;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::OKBtnClick(TObject *Sender)
{
  Configuration config;
  GetOptions(config);

  if (MainForm->ApplyConfig(config))
    ModalResult = mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::HotKeyBoxChange(TObject *Sender)
{
  AddBtn->Enabled = HotKeyBox->HotKey != 0;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::AddBtnClick(TObject *Sender)
{
  if (m_hotKeys.size() == HOTKEYS_MAX_NUM) {
    MsgBox(TRLFormat("Maximum number of hot keys (%d) reached.",
        HOTKEYS_MAX_NUM), MB_ICONWARNING);
    return;
  }

  if (!HotKeyShowMainWinCheck->Checked && HotKeyActionsList->ItemIndex <= 0) {
    MsgBox(TRL("Please select a hot key action."), MB_ICONWARNING);
    return;
  }

  HotKeyEntry hke;
  hke.Action = static_cast<HotKeyAction>(std::max(0, HotKeyActionsList->ItemIndex));
  hke.ShowMainWin = HotKeyShowMainWinCheck->Checked;

  std::pair<HotKeyList::iterator,bool> ret =
    m_hotKeys.insert(std::pair<TShortCut,HotKeyEntry>(HotKeyBox->HotKey, hke));

  if (ret.second)
    UpdateHotKeyList();
  else
    MsgBox(TRL("This hot key has already been assigned."), MB_ICONWARNING);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::HotKeyViewSelectItem(TObject *Sender,
  TListItem *Item, bool Selected)
{
  RemoveBtn->Enabled = Selected;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::RemoveBtnClick(TObject *Sender)
{
  for (int nI = HotKeyView->Items->Count - 1; nI >= 0; nI--) {
    if (HotKeyView->Items->Item[nI]->Selected) {
      word32 lKey = reinterpret_cast<word32>(HotKeyView->Items->Item[nI]->Data);
      m_hotKeys.erase(static_cast<TShortCut>(lKey));
      HotKeyView->Items->Delete(nI);
    }
  }
  UpdateHotKeyList();
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::LockIdleCheckClick(TObject *Sender)
{
  bool blEnabled = LockIdleCheck->Checked;
  LockIdleTimeBox->Enabled = blEnabled;
  LockIdleTimeUpDown->Enabled = blEnabled;
  LockAutoSaveCheck->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::CreateBackupCheckClick(TObject *Sender)
{
  NumberBackupsCheck->Enabled = CreateBackupCheck->Checked;
  NumberBackupsCheckClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::AutoClearPasswCheckClick(TObject *Sender)
{
  AutoClearPasswTimeBox->Enabled = AutoClearPasswCheck->Checked;
  AutoClearPasswTimeSpinBtn->Enabled = AutoClearPasswCheck->Checked;
  if (Visible && AutoClearPasswTimeBox->Enabled)
    AutoClearPasswTimeBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::NumberBackupsCheckClick(TObject *Sender)
{
  bool blEnabled = CreateBackupCheck->Checked && NumberBackupsCheck->Checked;
  MaxNumBackupsBox->Enabled = blEnabled;
  MaxNumBackupsUpDown->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::AutoSaveCheckClick(TObject *Sender)
{
  AutoSaveList->Enabled = AutoSaveCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::BenchmarkBtnClick(TObject *Sender)
{
  const int DATA_SIZE_MB = 50;
  SplitMix64 sm64(42);
  RandomPool rp(static_cast<RandomPool::Cipher>(0), sm64, false);
  rp.Randomize();
  std::vector<word8> data(1048576*DATA_SIZE_MB);
  WString sResult;
  Screen->Cursor = crHourGlass;
  for (int i = 0; i < NUM_RANDOM_POOL_CIPHERS; i++) {
    if (i > 0)
      rp.ChangeCipher(static_cast<RandomPool::Cipher>(i));
    Stopwatch clock;
    rp.GetData(&data[0], data.size());
    double rate = DATA_SIZE_MB / clock.ElapsedSeconds();
    sResult += "\n" + FormatW("%s: %.2f MB/s", RANDOM_POOL_CIPHER_NAMES[i], rate);
  }
  Screen->Cursor = crDefault;
  MsgBox(TRLFormat("Benchmark results (data size: %d MB):", DATA_SIZE_MB) +
    sResult, MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

