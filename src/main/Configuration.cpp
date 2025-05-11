// Configuration.cpp
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
  //L"Speck128"
};

const int RANDOM_POOL_CIPHER_INFO[NUM_RANDOM_POOL_CIPHERS][2] =
{
  { 256, 128 },
  { 256, 512 },
  { 256, 512 },
  //{ 256, 128 }
};

const std::vector<std::pair<WString, WString>> AppIconNames = {
  { "Default (dark)", "A" },
  { "Blue", "Icon_Blue" },
  { "Red", "Icon_Red" },
  { "Orange", "Icon_Orange" },
  { "Green", "Icon_Green" },
  { "Colors", "Icon_Colors" }
};

//---------------------------------------------------------------------------
__fastcall TConfigurationDlg::TConfigurationDlg(TComponent* Owner)
  : TForm(Owner) //, m_pLangList(nullptr)
{
  SetFormComponentsAnchors(this);

  //Constraints->MaxHeight = Height;
  //Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  AutoClearClipTimeSpinBtn->Min = AUTOCLEARCLIPTIME_MIN;
  AutoClearClipTimeSpinBtn->Max = AUTOCLEARCLIPTIME_MAX;

  for (const auto& sName : TStyleManager::StyleNames) {
    UiStylesList->Items->Add(sName);
  }

  auto icon = std::make_unique<TIcon>();
  int i = 0;
  for (const auto& p : AppIconNames) {
    icon->LoadFromResourceName(reinterpret_cast<NativeUInt>(HInstance), p.second);
    AppIconImageList->AddIcon(icon.get());
    AppIconList->ItemsEx->AddItem(p.first, i++, -1, -1, -1, nullptr);
  }

  for (i = 0; i < NUM_RANDOM_POOL_CIPHERS; i++) {
    WString sCipher = TRLFormat("%1 (%2-bit key, operates on %3-bit blocks)",
      { RANDOM_POOL_CIPHER_NAMES[i],
        IntToStr(RANDOM_POOL_CIPHER_INFO[i][0]),
        IntToStr(RANDOM_POOL_CIPHER_INFO[i][1]) });
    RandomPoolCipherList->Items->Add(sCipher);
  }

  for (i = 0; i <= 10; i++)
    BenchmarkMemList->Items->Add(IntToStr(1 << i) + " MB");
  BenchmarkMemList->ItemIndex = 6;

  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(GeneralSheet);
    TRLCaption(UiStyleLbl);
    TRLCaption(AppIconLbl);
    TRLCaption(ChangeFontLbl);
    TRLCaption(SelectFontBtn);
    TRLCaption(ShowSysTrayIconConstCheck);
    TRLCaption(MinimizeToSysTrayCheck);
    TRLCaption(MinimizeAutotypeCheck);
    TRLCaption(AutotypeDelayLbl);
    TRLCaption(AskBeforeExitCheck);
    TRLCaption(LaunchSystemStartupCheck);
    TRLCaption(LoadProfileStartupCheck);

    TRLCaption(SecuritySheet);
    TRLCaption(RandomPoolCipherLbl);
    TRLCaption(BenchmarkBtn);
    TRLHint(BenchmarkMemList);
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

    for (i = 0; i < HotKeyActionsList->Items->Count; i++)
      HotKeyActionsList->Items->Strings[i] =
        TRL(HotKeyActionsList->Items->Strings[i]);

    TRLCaption(FilesSheet);
    TRLCaption(CharEncodingGroup);
    TRLCaption(NewlineCharGroup);

    TRLCaption(UpdatesSheet);
    TRLCaption(UpdateCheckGroup);
    for (i = 0; i < UpdateCheckGroup->Items->Count; i++)
      UpdateCheckGroup->Items->Strings[i] =
        TRL(UpdateCheckGroup->Items->Strings[i]);

    TRLCaption(LanguageSheet);
    TRLCaption(SelectLanguageLbl);
    TRLCaption(InstallLanguageBtn);
    TRLCaption(RemoveLanguageBtn);

    TRLCaption(DatabaseSheet);
    TRLCaption(ClearClipCloseLockCheck);
    TRLCaption(LockMinimizeCheck);
    TRLCaption(LockIdleCheck);
    TRLCaption(LockAutoSaveCheck);
    TRLCaption(CreateBackupCheck);
    TRLCaption(NumberBackupsCheck);
    TRLCaption(OpenWindowOnStartupCheck);
    TRLCaption(OpenDbOnStartupCheck);
    TRLCaption(DefaultAutotypeSeqLbl);
    TRLCaption(AutoSaveCheck);
    TRLCaption(WarnExpiredEntriesCheck);
    TRLCaption(WarnEntriesExpireSoonCheck);
    TRLCaption(WarnExpireNumDaysLbl);
    TRLCaption(KeepRecentFilesCheck);

    for (i = 0; i < AutoSaveList->Items->Count; i++)
      AutoSaveList->Items->Strings[i] =
        TRL(AutoSaveList->Items->Strings[i]);

    TRLCaption(OKBtn);
    TRLCaption(CancelBtn);

    TRLMenu(SelectFontMenu);

    OpenDlg->Title = TRL(OpenDlg->Title);
  }

  UseAdvancedPasswEst->Caption = TRLFormat("Use advanced password strength "
    "estimation (%1)", { "zxcvbn" });

  OpenDlg->Filter = FormatW("%1 (*.po)|*.po|%2 (*.*)|*.*|",
    { TRL("Language files"),
      TRL("All files") }
  );

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
  config.UiStyleName = UiStylesList->Text;
  config.AppIconName = AppIconList->Text;
  config.GUIFontString = FontToString(FontDlg->Font);
  config.AutoClearClip = AutoClearClipCheck->Checked;
  config.AutoClearClipTime = AutoClearClipTimeSpinBtn->Position;
  config.AutoClearPassw = AutoClearPasswCheck->Checked;
  config.AutoClearPasswTime = AutoClearPasswTimeSpinBtn->Position;
  config.MinimizeAutotype = MinimizeAutotypeCheck->Checked;
  config.AutotypeDelay = AutotypeDelaySpinBtn->Position;
  config.ConfirmExit = AskBeforeExitCheck->Checked;
  config.LoadProfileStartup = LoadProfileStartupCheck->Checked;
  config.LoadProfileName = LoadProfileBox->Text;
  config.LaunchSystemStartup = LaunchSystemStartupCheck->Checked;
  config.RandomPoolCipher = RandomPoolCipherList->ItemIndex;
  config.TestCommonPassw = TestCommonPasswCheck->Checked;
  config.UseAdvancedPasswEst = UseAdvancedPasswEst->Checked;
  config.ShowSysTrayIconConst = ShowSysTrayIconConstCheck->Checked;
  config.MinimizeToSysTray = MinimizeToSysTrayCheck->Checked;
  config.AutoCheckUpdates = AutoCheckUpdates(UpdateCheckGroup->ItemIndex);
  config.FileEncoding = CharacterEncoding(CharEncodingGroup->ItemIndex);
  config.FileNewlineChar = NewlineChar(NewlineCharGroup->ItemIndex);
  config.HotKeys = m_hotKeys;
  config.Language = g_languages.at(LanguageList->ItemIndex);
  //config.Database.ClearClipMinimize = ClearClipMinimizeCheck->Checked;
  config.Database.ClearClipCloseLock = ClearClipCloseLockCheck->Checked;
  config.Database.LockMinimize = LockMinimizeCheck->Checked;
  config.Database.LockIdle = LockIdleCheck->Checked;
  config.Database.LockIdleTime = LockIdleTimeSpinBtn->Position;
  config.Database.LockAutoSave = LockAutoSaveCheck->Checked;
  config.Database.CreateBackup = CreateBackupCheck->Checked;
  config.Database.NumberBackups = NumberBackupsCheck->Checked;
  config.Database.MaxNumBackups = MaxNumBackupsSpinBtn->Position;
  config.Database.OpenWindowOnStartup = OpenWindowOnStartupCheck->Checked;
  config.Database.OpenLastDbOnStartup = OpenDbOnStartupCheck->Checked;
  config.Database.WarnExpiredEntries = WarnExpiredEntriesCheck->Checked;
  config.Database.WarnEntriesExpireSoon = WarnEntriesExpireSoonCheck->Checked;
  config.Database.WarnExpireNumDays = WarnExpireNumDaysSpinBtn->Position;
  config.Database.AutoSave = AutoSaveCheck->Checked;
  config.Database.AutoSaveOption = static_cast<AutoSaveDatabase>(
    AutoSaveList->ItemIndex);
  config.Database.DefaultAutotypeSequence = DefaultAutotypeSeqBox->Text;
  config.Database.KeepRecentFiles = KeepRecentFilesCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SetOptions(const Configuration& config)
{
  static bool blInit = true;
  if (blInit) {
    blInit = false;
    UpdateProfileList();
  }
  int nIndex = UiStylesList->Items->IndexOf(config.UiStyleName);
  if (nIndex < 0)
    nIndex = UiStylesList->Items->IndexOf("Windows");
  UiStylesList->ItemIndex = nIndex;
  AppIconList->ItemIndex = std::max(0, AppIconList->Items->IndexOf(config.AppIconName));
  StringToFont(config.GUIFontString, FontDlg->Font);
  ShowFontSample(FontDlg->Font);
  RandomPoolCipherList->ItemIndex = config.RandomPoolCipher;
  TestCommonPasswCheck->Checked = config.TestCommonPassw;
  UseAdvancedPasswEst->Checked = config.UseAdvancedPasswEst;
  AutoClearClipCheck->Checked = config.AutoClearClip;
  AutoClearClipTimeSpinBtn->Position = config.AutoClearClipTime;
  AutoClearClipCheckClick(this);
  AutoClearPasswCheck->Checked = config.AutoClearPassw;
  AutoClearPasswTimeSpinBtn->Position = config.AutoClearPasswTime;
  AutoClearPasswCheckClick(this);
  MinimizeAutotypeCheck->Checked = config.MinimizeAutotype;
  AutotypeDelaySpinBtn->Position = config.AutotypeDelay;
  ShowSysTrayIconConstCheck->Checked = config.ShowSysTrayIconConst;
  MinimizeToSysTrayCheck->Checked = config.MinimizeToSysTray;
  AskBeforeExitCheck->Checked = config.ConfirmExit;
  LoadProfileStartupCheck->Checked = config.LoadProfileStartup;
  LoadProfileBox->ItemIndex = LoadProfileBox->Items->IndexOf(config.LoadProfileName);
  LoadProfileStartupCheckClick(this);
  LaunchSystemStartupCheck->Checked = config.LaunchSystemStartup;
  UpdateCheckGroup->ItemIndex = config.AutoCheckUpdates;
  CharEncodingGroup->ItemIndex = static_cast<int>(config.FileEncoding);
  NewlineCharGroup->ItemIndex = static_cast<int>(config.FileNewlineChar);
  auto it = std::find(g_languages.begin(), g_languages.end(), config.Language.Code);
  LanguageList->ItemIndex = it != g_languages.end() ? it - g_languages.begin() : 0;
  LanguageListSelect(this);
  m_hotKeys = config.HotKeys;
  UpdateHotKeyList();
  //ClearClipMinimizeCheck->Checked = config.Database.ClearClipMinimize;
  ClearClipCloseLockCheck->Checked = config.Database.ClearClipCloseLock;
  LockMinimizeCheck->Checked = config.Database.LockMinimize;
  LockIdleCheck->Checked = config.Database.LockIdle;
  LockIdleCheckClick(this);
  LockIdleTimeSpinBtn->Position = config.Database.LockIdleTime;
  LockAutoSaveCheck->Checked = config.Database.LockAutoSave;
  CreateBackupCheck->Checked = config.Database.CreateBackup;
  NumberBackupsCheck->Checked = config.Database.NumberBackups;
  CreateBackupCheckClick(this);
  //NumberBackupsCheckClick(this);
  MaxNumBackupsSpinBtn->Position = config.Database.MaxNumBackups;
  OpenWindowOnStartupCheck->Checked = config.Database.OpenWindowOnStartup;
  OpenDbOnStartupCheck->Checked = config.Database.OpenLastDbOnStartup;
  KeepRecentFilesCheck->Checked = config.Database.KeepRecentFiles;
  WarnExpiredEntriesCheck->Checked = config.Database.WarnExpiredEntries;
  WarnEntriesExpireSoonCheck->Checked = config.Database.WarnEntriesExpireSoon;
  WarnExpireNumDaysSpinBtn->Position = config.Database.WarnExpireNumDays;
  AutoSaveCheck->Checked = config.Database.AutoSave;
  AutoSaveList->ItemIndex = static_cast<int>(config.Database.AutoSaveOption);
  AutoSaveCheckClick(this);
  DefaultAutotypeSeqBox->Text = config.Database.DefaultAutotypeSequence;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::LoadLanguages(void)
{
  LanguageList->Clear();
  for (const auto& entry : g_languages)
  {
    LanguageList->Items->Add(FormatW("%1 (v%2)", { entry.Name,
      entry.Version }));
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::ShowFontSample(TFont* pFont)
{
  FontSampleLbl->Caption = FormatW("%1, %2pt", { pFont->Name,
    IntToStr(pFont->Size) });
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
      sAction = HotKeyShowMainWinCheck->Caption;
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
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = FontDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

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
void __fastcall TConfigurationDlg::UpdateProfileList(void)
{
  WString sCurrProfile = LoadProfileBox->Text;

  LoadProfileBox->Items->Clear();

  for (const auto& profile : g_profileList) {
    LoadProfileBox->Items->Add(profile->ProfileName);
  }

  if (!sCurrProfile.IsEmpty())
    LoadProfileBox->ItemIndex = LoadProfileBox->Items->IndexOf(sCurrProfile);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::FormShow(TObject *Sender)
{
  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;

  TopMostManager::GetInstance().SetForm(this);

  ConfigPages->ActivePage = GeneralSheet;
  UpdateProfileList();
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
    MsgBox(TRLFormat("Maximum number of hot keys (%1) reached.",
      { IntToStr(HOTKEYS_MAX_NUM) }), MB_ICONWARNING);
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
  LockIdleTimeSpinBtn->Enabled = blEnabled;
  //LockAutoSaveCheck->Enabled = blEnabled;
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
  MaxNumBackupsSpinBtn->Enabled = blEnabled;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::AutoSaveCheckClick(TObject *Sender)
{
  AutoSaveList->Enabled = AutoSaveCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::BenchmarkBtnClick(TObject *Sender)
{
  try {
    RandomPool rp(static_cast<RandomPool::CipherType>(0));
    rp.Randomize();
    word32 lDataSizeMB = 1 << std::max(0, BenchmarkMemList->ItemIndex);
    word32 lBufSize = lDataSizeMB << 20;
    auto buf = std::make_unique<word8[]>(lBufSize);
    WString sResult;
    Screen->Cursor = crHourGlass;
    for (int i = 0; i < NUM_RANDOM_POOL_CIPHERS; i++) {
      if (i > 0)
        rp.SetCipher(static_cast<RandomPool::CipherType>(i));
      Stopwatch clock;
      rp.GetData(buf.get(), lBufSize);
      double rate = lDataSizeMB / clock.ElapsedSeconds();
      sResult += "\n" + Format("%s: %.2f MB/s", ARRAYOFCONST((
        RANDOM_POOL_CIPHER_NAMES[i], static_cast<TVARREC_DOUBLE>(rate))));
    }
    Screen->Cursor = crDefault;
    MsgBox(TRLFormat("Benchmark results (data size: %1 MB):",
      { IntToStr(static_cast<int>(lDataSizeMB)) }) + sResult, MB_ICONINFORMATION);
  }
  catch (const Exception& e) {
    MsgBox(e.Message, MB_ICONERROR);
  }
  catch (const std::exception& e) {
    MsgBox(e.what(), MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::ConvertLangFileBtnClick(TObject *Sender)
{
  int nIndex = LanguageList->ItemIndex;
  if (nIndex > 0 && nIndex < static_cast<int>(g_languages.size())) {
    const auto& entry = g_languages[nIndex];
    //if (SameText(ExtractFileExt(entry.FileName), ".lng")) {
    TopMostManager::GetInstance().NormalizeTopMosts(this);
    bool blSuccess = SaveDlg->Execute();
    TopMostManager::GetInstance().RestoreTopMosts(this);

    if (blSuccess) {
      try {
        LanguageSupport ls(entry.FileName, false, true);
        ls.SaveToPOFileFormat(SaveDlg->FileName,
          (g_config.FileEncoding == ceAnsi) ? ceUtf8 : g_config.FileEncoding);
        MsgBox("File successfully converted.", MB_ICONINFORMATION);
      }
      catch (Exception& e) {
        MsgBox(e.Message + ".", MB_ICONERROR);
      }
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::LanguageListSelect(TObject *Sender)
{
  bool blEnabled = false;
  int nIndex = LanguageList->ItemIndex;
  if (nIndex > 0 && nIndex < static_cast<int>(g_languages.size())) {
    const auto& entry = g_languages[nIndex];
    blEnabled = SameText(ExtractFileExt(entry.FileName), ".lng");
  }
  ConvertLangFileBtn->Enabled = blEnabled;
  RemoveLanguageBtn->Enabled = nIndex > 0;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SelectFontMenu_RestoreDefaultClick(TObject *Sender)

{
  auto pFont = std::make_unique<TFont>();
  pFont->Name = "Tahoma";
  pFont->Size = 8;
  FontDlg->Font = pFont.get();
  ShowFontSample(pFont.get());
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::LoadProfileStartupCheckClick(TObject *Sender)
{
  LoadProfileBox->Enabled = LoadProfileStartupCheck->Checked;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::InstallLanguageBtnClick(TObject *Sender)
{
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = OpenDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

  if (!blSuccess) return;

  WString sMsg;
  try {
    WString sSrc = OpenDlg->FileName;
    LanguageSupport ls(sSrc, true);
    if (std::find(g_languages.begin(), g_languages.end(), ls.LanguageCode)
          != g_languages.end())
    {
      throw Exception(TRL("Language already installed"));
    }

    const auto destinations = { g_sAppDataPath, g_sExePath };
    WString sDest;
    for (const auto& sPath : destinations) {
      if (sPath.IsEmpty()) continue;
      sDest = sPath + ExtractFileName(sSrc);
      if (FileExists(sDest)) {
        throw Exception(TRLFormat("Language file already exists:\n\"%1\"", { sDest }));
      }
      if (CopyFile(sSrc.c_str(), sDest.c_str(), true)) {
        LanguageEntry e;
        e.FileName = sSrc;
        e.Code = ls.LanguageCode;
        e.Name = ls.LanguageName;
        e.Version = ls.LanguageVersion;
        g_languages.push_back(e);
        int nIndex = LanguageList->ItemIndex;
        LoadLanguages();
        LanguageList->ItemIndex = nIndex;
        MsgBox(TRLFormat("Language \"%1\" installed successfully.", { ls.LanguageName }),
          MB_ICONINFORMATION);
        return;
      }
      break;
    }

    sMsg = TRLFormat("Could not copy file \"%1\" to\n\"%2\"", { sSrc, sDest });
  }
  catch (Exception& e) {
    sMsg = e.Message;
  }

  MsgBox(TRLFormat("Could not install language:\n%1.", { sMsg }), MB_ICONERROR);
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::RemoveLanguageBtnClick(TObject *Sender)
{
  int nIndex = LanguageList->ItemIndex;
  if (nIndex > 0 && nIndex < g_languages.size()) {
    auto e = g_languages[nIndex];
    if (MsgBox(TRLFormat("Are you sure you want to remove\n\"%1\"?", { e.Name }),
        MB_ICONQUESTION + MB_YESNO + MB_DEFBUTTON2) == IDYES)
    {
      if (DeleteFile(e.FileName.c_str())) {
        g_languages.erase(g_languages.begin() + nIndex);
        LoadLanguages();
        LanguageList->ItemIndex = 0;
        MsgBox(TRLFormat("Language \"%1\" successfully removed.", { e.Name }),
          MB_ICONINFORMATION);
      }
      else {
        MsgBox(TRLFormat("Could not delete file\n\"%1\".", { e.FileName }),
          MB_ICONERROR);
      }
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationDlg::SetDonorUI(void)
{
  AppIconLbl->Enabled = g_donorInfo.Valid == DONOR_KEY_VALID;
  AppIconList->Enabled = g_donorInfo.Valid == DONOR_KEY_VALID;
}
//---------------------------------------------------------------------------
