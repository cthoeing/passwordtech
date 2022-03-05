// PasswManager.cpp
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
#include <vector>
#include <map>
#include <algorithm>
#include <Math.hpp>
#include <IOUtils.hpp>
#include <DateUtils.hpp>
#include <Clipbrd.hpp>
#include <StrUtils.hpp>
#pragma hdrstop

#include "PasswManager.h"
#include "Util.h"
#include "PasswEnter.h"
#include "Language.h"
#include "RandomPool.h"
#include "Main.h"
#include "PasswMngColSelect.h"
#include "Progress.h"
#include "StringFileStreamW.h"
#include "TopMostManager.h"
#include "SendKeys.h"
#include "dragdrop.h"
#include "PasswMngKeyValEdit.h"
#include "Autocomplete.h"
#include "PasswMngDbProp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswMngForm *PasswMngForm;


static const char* UI_FIELD_NAMES[PasswDbEntry::NUM_FIELDS] =
{
  "Title", "User name", "Password", "URL", "Keyword", "Notes", "Key-value list",
  "Tags", "Creation time", "Last modification", "Password expiry"
};

static const WString
  CONFIG_ID          = "PasswManager",
  PASSW_MANAGER_NAME = "PassCube";

static const char
  PASSWORD_CHAR = '*';

static const wchar_t
  HIDE_PASSW_STR[] = L"********";

static const int
  DB_NUM_KEYVAL_KEYS    = 4,

  DB_KEYVAL_AUTOTYPE    = 0,
  DB_KEYVAL_RUN         = 1,
  DB_KEYVAL_PROFILE     = 2,
  DB_KEYVAL_FORMATPASSW = 3,

  DB_MAX_TAG_LEN = 30,

  SEARCH_MODE_OFF = 0,
  SEARCH_MODE_NORMAL = 1,
  SEARCH_MODE_FUZZY = 2,

  MOVE_TOP = 0,
  MOVE_UP = 1,
  MOVE_DOWN = 2,
  MOVE_BOTTOM = 3,

  ASK_SAVE_OK = 0,
  ASK_SAVE_CANCEL = 1,
  ASK_SAVE_ERROR = 2,

  RESET_COLUMNS = 1,
  RELOAD_TAGS = 2;

static const wchar_t* DB_KEYVAL_KEYS[DB_NUM_KEYVAL_KEYS] =
{
  L"Autotype", L"Run", L"Profile", L"FormatPW"
};

static const wchar_t* DB_KEYVAL_UI_KEYS[DB_NUM_KEYVAL_KEYS] =
{
  L"Autotype", L"Run", L"Profile", L"Format password"
};


static const word32
  DB_FLAG_FOUND   = 1,
  DB_FLAG_EXPIRED = 2;

static int nNumIdleTimerSuspendInst = 0;

class IdleTimerSuspender
{
public:
  IdleTimerSuspender()
  {
    if (++nNumIdleTimerSuspendInst == 1)
      PasswMngForm->IdleTimer->Enabled = false;
  }

  ~IdleTimerSuspender()
  {
    if (--nNumIdleTimerSuspendInst == 0) {
      PasswMngForm->IdleTimer->Enabled = true;
      PasswMngForm->NotifyUserAction();
    }
  }
};

#define SuspendIdleTimer IdleTimerSuspender _its

static const int
  FORM_TAG_OPEN_DB        = 1,
  FORM_TAG_UNLOCK_DB      = 2,
  FORM_TAG_ITEM_SELECTED  = 3;

static const int STD_EXPIRY_DAYS[] = { 7, 14, 30, 90, 180, 365 };

static const wchar_t* getDbEntryAutotypeSeq(const PasswDbEntry* pEntry)
{
  const SecureWString* psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[
    DB_KEYVAL_AUTOTYPE]);
  return (psVal != NULL) ? psVal->c_str() :
    g_config.Database.DefaultAutotypeSequence.c_str();
}

static word32 levenshteinDist(const wchar_t* pSrc, word32 lSrcLen,
  const wchar_t* pTarget, word32 lTargetLen)
{
  static const int
    INSERT_COST = 2,
    DELETE_COST = 1,
    REPLACE_COST = 1;

  if (lSrcLen > lTargetLen) {
    return levenshteinDist(pTarget, lTargetLen, pSrc, lSrcLen);
  }

  const word32 lMinSize = lSrcLen;
  const word32 lMaxSize = lTargetLen;
  std::vector<word32> levDist(lMinSize + 1);
  levDist[0] = 0;
  for (word32 i = 1; i <= lMinSize; i++)
    levDist[i] = levDist[i - 1] + DELETE_COST;

  for (word32 j = 1; j <= lMaxSize; j++) {
    word32 lPrev = levDist[0], lPrevSave;
    levDist[0] += INSERT_COST;
    for (word32 i = 1; i <= lMinSize; i++) {
      lPrevSave = levDist[i];
      if (pSrc[i - 1] == pTarget[j - 1])
        levDist[i] = lPrev;
      else
        levDist[i] = std::min(std::min(levDist[i] + INSERT_COST,
          levDist[i - 1] + DELETE_COST), lPrev + REPLACE_COST);
      lPrev = lPrevSave;
    }
  }

  return levDist.back();
}

static float strFindFuzzy(const wchar_t* pSrc, const wchar_t* pPattern)
{
  const word32 lSrcLen = wcslen(pSrc);
  const word32 lPatternLen = wcslen(pPattern);
  float fBestScore = 0;

  auto levenshteinScore = [&](const wchar_t* p, word32 lLen)
  {
    word32 lDist = levenshteinDist(p, lLen, pPattern, lPatternLen);
    return (lDist < lPatternLen) ?
      (lPatternLen - lDist) / static_cast<float>(lPatternLen) : 0.0f;
  };

  if (lSrcLen <= lPatternLen) {
    if (lSrcLen == lPatternLen && wcscmp(pSrc, pPattern) == 0)
      fBestScore = 2;
    else
      fBestScore = levenshteinScore(pSrc, lSrcLen);
  }
  else {
    const wchar_t* pNext = pSrc;
    int i = 0;
    while ((pNext = wcschr(pNext, pPattern[0])) != NULL) {
      if (i == 0 && wcsstr(pNext, pPattern) != NULL) {
        fBestScore = 1;
        break;
      }
      else {
        fBestScore = std::max(fBestScore, levenshteinScore(pNext,
          std::min(lPatternLen, static_cast<word32>(pSrc + lSrcLen - pNext))));
      }
      i++;
      pNext++;
    }
  }

  return fBestScore;
}

//---------------------------------------------------------------------------
__fastcall TPasswMngForm::TPasswMngForm(TComponent* Owner)
  : TForm(Owner), m_pSelectedItem(NULL), m_nSortByIdx(-1),
    m_nSortOrderFactor(1), m_nTagsSortByIdx(0), m_nTagsSortOrderFactor(1),
    m_nSearchFlags(INT_MAX)
{
  ChangeCaption();

  for (int nI = 0; nI < DB_NUM_KEYVAL_KEYS; nI++) {
    m_keyValNames.emplace(std::wstring(DB_KEYVAL_KEYS[nI]),
      std::wstring(DB_KEYVAL_UI_KEYS[nI]));
  }

  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    m_uiFieldNames[nI] = TRL(UI_FIELD_NAMES[nI]);

    TMenuItem* pItem = new TMenuItem(MainMenu_View_ShowColumns);
    pItem->Caption = m_uiFieldNames[nI];
    pItem->AutoCheck = true;
    pItem->OnClick = MainMenu_View_ShowColClick;
    MainMenu_View_ShowColumns->Add(pItem);

    TMenuItem* pItem2 = new TMenuItem(MainMenu_View_SortBy);
    pItem2->Caption = pItem->Caption;
    pItem2->RadioItem = true;
    pItem2->AutoCheck = true;
    pItem2->OnClick = MainMenu_View_SortByClick;
    pItem2->Tag = nI;
    MainMenu_View_SortBy->Insert(2 + nI, pItem2);
  }

  for (int t : STD_EXPIRY_DAYS) {
    TMenuItem* pItem = new TMenuItem(ExpiryMenu);
    pItem->Caption = TRLFormat("%d days", t);
    pItem->Tag = t;
    pItem->OnClick = OnExpiryMenuItemClick;
    ExpiryMenu->Items->Add(pItem);
  }

  // input boxes don't seem to scale properly when
  // changing window size in constructor...
  LoadConfig();

  SetShowColMask();
  ResetListView(RESET_COLUMNS);
  SetItemChanged(false);
  ResetDbOpenControls();

  ConfigurationDlg->SetOptions(g_config);

  if (g_pLangSupp != NULL) {
    auto translKeyValNames = m_keyValNames;
    std::set<std::wstring> translNames;
    for (auto& kv : translKeyValNames) {
      kv.second = TRL(kv.second.c_str()).c_str();
      translNames.insert(kv.second);
    }

    if (translNames.size() == translKeyValNames.size())
      m_keyValNames = translKeyValNames;
    else
      MsgBox("Password manager: Translations of key names not unique!",
        MB_ICONWARNING);

    TRLMenu(MainMenu);
    TRLMenu(DbViewMenu);

    TRLCaption(TitleLbl);
    TRLCaption(UserNameLbl);
    TRLCaption(PasswLbl);
    TRLCaption(UrlLbl);
    TRLCaption(KeywordLbl);
    TRLCaption(KeyValueListLbl);
    TRLCaption(NotesLbl);
    TRLCaption(CreationTimeLbl);
    TRLCaption(LastModificationLbl);
    TRLCaption(TagsLbl);
    TRLCaption(ExpiryCheck);
    TRLHint(PrevBtn);
    TRLHint(NextBtn);
    TRLHint(DeleteBtn);
    TRLHint(AddModifyBtn);
    TRLHint(CancelBtn);
    TRLHint(NewBtn);
    TRLHint(OpenBtn);
    TRLHint(SaveBtn);
    TRLHint(LockBtn);
    TRLHint(AddEntryBtn);
    TRLHint(ClearSearchBtn);
    TRLHint(SearchBtn);
    TRLHint(CaseSensitiveBtn);
    TRLHint(FuzzyBtn);
    TRLHint(TogglePasswBtn);
    TRLHint(AddTagBtn);
    TRLHint(EditKeyValBtn);
    TRLHint(ExpiryBtn);
    SearchBox->TextHint = TRL(SearchBox->TextHint);
  }

  OpenDlg->Filter = FormatW("%s (*.pwdb)|*.pwdb|%s (*.*)|*.*",
      TRL("Password databases").c_str(),
      TRL("All files").c_str());
  SaveDlg->Filter = FormatW("%s (*.pwdb)|*.pwdb|%s (*.csv)|*.csv|%s (*.*)|*.*",
      TRL("Password databases").c_str(),
      TRL("CSV files").c_str(),
      TRL("All files").c_str());

  RegisterDropWindow(PasswBox->Handle, &m_pPasswBoxDropTarget);
}
//---------------------------------------------------------------------------
__fastcall TPasswMngForm::~TPasswMngForm()
{
  UnregisterDropWindow(PasswBox->Handle, m_pPasswBoxDropTarget);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::LoadConfig(void)
{
  int nTop = g_pIni->ReadInteger(CONFIG_ID, "WindowTop", INT_MAX);
  int nLeft = g_pIni->ReadInteger(CONFIG_ID, "WindowLeft", INT_MAX);

  if (nTop == INT_MAX || nLeft == INT_MAX)
    Position = poScreenCenter;
  else {
    Top = nTop;
    Left = nLeft;
  }

  //Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  //Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);

  int nListWidth = g_pIni->ReadInteger(CONFIG_ID, "ListWidth", -1);
  if (nListWidth > 10)
    DbView->Width = nListWidth;

  WString sListFont = g_pIni->ReadString(CONFIG_ID, "ListFont", "");
  if (!sListFont.IsEmpty())
    StringToFont(sListFont, DbView->Font);

  int nListHeight = g_pIni->ReadInteger(CONFIG_ID, "TagListHeight", -1);
  if (nListHeight > 1)
    TagView->Width = nListHeight;

  StringToFont(g_pIni->ReadString(CONFIG_ID, "PasswFont", ""), PasswBox->Font);

  m_nShowColMask = g_pIni->ReadInteger(CONFIG_ID, "ShowColMask", 7);
  for (int nI = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    if (m_nShowColMask & (1 << nI))
      MainMenu_View_ShowColumns->Items[nI]->Checked = true;
  }

  int nSortByIdx = g_pIni->ReadInteger(CONFIG_ID, "SortByIdx", -1);
  if (nSortByIdx >= -1 && nSortByIdx < PasswDbEntry::NUM_FIELDS) {
    m_nSortByIdx = nSortByIdx;
    int nItemIdx = (nSortByIdx < 0) ? 0 : 2 + nSortByIdx;
    MainMenu_View_SortBy->Items[nItemIdx]->Checked = true;
  }
  DbView->SortType = m_nSortByIdx >= 0 ? TSortType::stData : TSortType::stNone;

  int nSortOrder = g_pIni->ReadInteger(CONFIG_ID, "SortOrder", 1);
  if (nSortOrder == 1 || nSortOrder == -1) {
    m_nSortOrderFactor = nSortOrder;
    if (nSortOrder == 1)
      MainMenu_View_SortBy_Ascending->Checked = true;
    else
      MainMenu_View_SortBy_Descending->Checked = true;
    //nSortOrder = (nSortOrder == 1) ? 0 : 1;
    //MainMenu_View_SortBy->Items[MainMenu_View_SortBy->Count - 2 +
    //  nSortOrder]->Checked = true;
  }

  nSortByIdx = g_pIni->ReadInteger(CONFIG_ID, "TagsSortByIdx", 0);
  if (nSortByIdx == 0 || nSortByIdx == 1) {
    m_nTagsSortByIdx = nSortByIdx;
    MainMenu_View_SortTagsBy->Items[nSortByIdx]->Checked = true;
  }

  nSortOrder = g_pIni->ReadInteger(CONFIG_ID, "TagsSortOrder", 1);
  if (nSortOrder == 1 || nSortOrder == -1) {
    m_nTagsSortOrderFactor = nSortOrder;
    if (nSortOrder == 1)
      MainMenu_View_SortTagsBy_Ascending->Checked = true;
    else
      MainMenu_View_SortTagsBy_Descending->Checked = true;
  }

  MainMenu_View_ShowPasswInList->Checked = g_pIni->ReadBool(CONFIG_ID,
    "ShowPasswInList", false);

  AnsiString asColWidths = g_pIni->ReadString(CONFIG_ID, "ListColWidths", "");
  if (!asColWidths.IsEmpty()) {
    int nLen = asColWidths.Length();
    int nStartIdx = 1;
    for (int nI = 1; nI <= nLen &&
      m_listColWidths.size() < PasswDbEntry::NUM_FIELDS; nI++)
    {
      if (asColWidths[nI] == ';' && nI > nStartIdx) {
        int nWidth = StrToIntDef(asColWidths.SubString(nStartIdx, nI - nStartIdx), 0);
        m_listColWidths.push_back(std::max(10, nWidth));
        nStartIdx = nI + 1;
      }
    }
  }

  TogglePasswBtn->Down = g_pIni->ReadBool(CONFIG_ID, "HidePassw", true);
  TogglePasswBtnClick(this);
  CaseSensitiveBtn->Down = g_pIni->ReadBool(CONFIG_ID, "FindCaseSensitive", false);
  FuzzyBtn->Down = g_pIni->ReadBool(CONFIG_ID, "FindFuzzy", false);

  g_config.Database.ClearClipMinimize = g_pIni->ReadBool(CONFIG_ID,
      "ClearClipMinimize", true);
  g_config.Database.ClearClipExit = g_pIni->ReadBool(CONFIG_ID, "ClearClipExit", true);
  g_config.Database.LockMinimize = g_pIni->ReadBool(CONFIG_ID, "LockMinimize", true);
  g_config.Database.LockIdle = g_pIni->ReadBool(CONFIG_ID, "LockIdle", true);
  g_config.Database.LockIdleTime = std::max(10, std::min(32767,
        g_pIni->ReadInteger(CONFIG_ID, "LockIdleTime", 300)));
  g_config.Database.LockAutoSave = g_pIni->ReadBool(CONFIG_ID, "LockAutoSave", false);
  g_config.Database.CreateBackup = g_pIni->ReadBool(CONFIG_ID, "CreateBackup", true);
  g_config.Database.MaxNumBackups = std::max(1, std::min(999,
        g_pIni->ReadInteger(CONFIG_ID, "MaxNumBackups", 5)));
  g_config.Database.OpenWindowOnStartup = g_pIni->ReadBool(CONFIG_ID,
      "OpenWindowOnStartup", false);
  g_config.Database.OpenLastDbOnStartup = g_pIni->ReadBool(CONFIG_ID,
      "OpenLastDbOnStartup", true);
  if (g_config.Database.OpenLastDbOnStartup)
    m_sDbFileName = g_pIni->ReadString(CONFIG_ID, "LastDatabase", "");
  g_config.Database.WarnExpiredEntries = g_pIni->ReadBool(CONFIG_ID,
      "WarnExpiredEntries", false);
  g_config.Database.DefaultAutotypeSequence = g_pIni->ReadString(CONFIG_ID,
    "DefaultAutotypeSeq", "{username}{tab}{password}{enter}");
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteInteger(CONFIG_ID, "ListWidth", DbView->Width);
  if (DbView->Font->Name != Font->Name || DbView->Font->Size != Font->Size ||
      DbView->Font->Style != Font->Style)
    g_pIni->WriteString(CONFIG_ID, "ListFont", FontToString(DbView->Font));
  g_pIni->WriteString(CONFIG_ID, "TagListHeight", TagView->Height);
  g_pIni->WriteString(CONFIG_ID, "PasswFont", FontToString(PasswBox->Font));
  g_pIni->WriteInteger(CONFIG_ID, "ShowColMask", m_nShowColMask);
  g_pIni->WriteBool(CONFIG_ID, "ShowPasswInList",
    MainMenu_View_ShowPasswInList->Checked);

  TListColumns* pCols = DbView->Columns;
  WString sColWidths;
  for (int nI = 0; nI < pCols->Count; nI++)
    sColWidths += IntToStr(pCols->Items[nI]->Width) + ";";

  g_pIni->WriteString(CONFIG_ID, "ListColWidths", sColWidths);

  g_pIni->WriteBool(CONFIG_ID, "HidePassw", TogglePasswBtn->Down);
  g_pIni->WriteBool(CONFIG_ID, "FindCaseSensitive", CaseSensitiveBtn->Down);
  g_pIni->WriteBool(CONFIG_ID, "FindFuzzy", FuzzyBtn->Down);

  g_pIni->WriteInteger(CONFIG_ID, "SortByIdx", m_nSortByIdx);
  g_pIni->WriteInteger(CONFIG_ID, "SortOrder", m_nSortOrderFactor);
  g_pIni->WriteInteger(CONFIG_ID, "TagsSortByIdx", m_nTagsSortByIdx);
  g_pIni->WriteInteger(CONFIG_ID, "TagsSortOrder", m_nTagsSortOrderFactor);
  g_pIni->WriteBool(CONFIG_ID, "ClearClipMinimize",
    g_config.Database.ClearClipMinimize);
  g_pIni->WriteBool(CONFIG_ID, "ClearClipExit", g_config.Database.ClearClipExit);
  g_pIni->WriteBool(CONFIG_ID, "LockMinimize", g_config.Database.LockMinimize);
  g_pIni->WriteBool(CONFIG_ID, "LockIdle", g_config.Database.LockIdle);
  g_pIni->WriteInteger(CONFIG_ID, "LockIdleTime", g_config.Database.LockIdleTime);
  g_pIni->WriteBool(CONFIG_ID, "LockAutoSave", g_config.Database.LockAutoSave);
  g_pIni->WriteBool(CONFIG_ID, "CreateBackup", g_config.Database.CreateBackup);
  g_pIni->WriteInteger(CONFIG_ID, "MaxNumBackups", g_config.Database.MaxNumBackups);
  g_pIni->WriteBool(CONFIG_ID, "OpenWindowOnStartup",
    g_config.Database.OpenWindowOnStartup);
  g_pIni->WriteBool(CONFIG_ID, "OpenLastDbOnStartup",
    g_config.Database.OpenLastDbOnStartup);
  g_pIni->WriteString(CONFIG_ID, "LastDatabase", m_sDbFileName);
  g_pIni->WriteBool(CONFIG_ID, "WarnExpiredEntries",
    g_config.Database.WarnExpiredEntries);
  g_pIni->WriteString(CONFIG_ID, "DefaultAutotypeSeq",
    g_config.Database.DefaultAutotypeSequence);
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::OpenDatabase(bool blOpenExisting,
  bool blUnlock,
  WString sFileName)
{
  SuspendIdleTimer;

  if (!blUnlock && IsDbOpen() && AskSaveChanges() != ASK_SAVE_OK)
    return false;

  int nPasswEnterFlags;
  WString sCaption;

  if (blOpenExisting) {
    if (blUnlock) {
      sFileName = m_sDbFileName;
      sCaption = TRLFormat("Unlock %s", ExtractFileName(sFileName).c_str());
    }
    else {
      if (sFileName.IsEmpty()) {
        BeforeDisplayDlg();
        TopMostManager::GetInstance()->NormalizeTopMosts(this);
        bool blSuccess = OpenDlg->Execute();
        TopMostManager::GetInstance()->RestoreTopMosts(this);
        AfterDisplayDlg();
        if (!blSuccess)
          return false;
        sFileName = OpenDlg->FileName;
        if (m_passwDb && AnsiCompareText(sFileName, m_sDbFileName) == 0) {
          MsgBox(TRL("The selected file is already opened."), MB_ICONWARNING);
          return false;
        }
      }
      else if (!FileExists(sFileName)) {
        MsgBox(TRLFormat("Could not find database file:\n\"%s\".", sFileName.c_str()),
          MB_ICONERROR);
        return false;
      }
      sCaption = TRLFormat("Open %s", ExtractFileName(sFileName).c_str());
    }
    nPasswEnterFlags = PASSWENTER_FLAG_DECRYPT;
  }
  else {
    sCaption = TRL("Create new database");
    nPasswEnterFlags = PASSWENTER_FLAG_ENCRYPT | PASSWENTER_FLAG_CONFIRMPASSW;
  }

  std::unique_ptr<PasswDatabase> passwDb(new PasswDatabase());

  while (true) {
    if (PasswEnterDlg->Execute(nPasswEnterFlags, sCaption, this) != mrOk) {
      PasswEnterDlg->Clear();
      RandomPool::GetInstance()->Flush();
      return false;
    }

    // get password and convert from WideString to byte array
    SecureMem<word8> key;
    PasswEnterDlg->GetPassw(key);

    // now clear password box
    PasswEnterDlg->Clear();
    RandomPool::GetInstance()->Flush();

    try {
      if (blOpenExisting) {
        Screen->Cursor = crHourGlass;
        passwDb->Open(key, sFileName);
      }
      else
        passwDb->New(key);
      break;
    }
    catch (EPasswDbInvalidKey& e) {
      Screen->Cursor = crDefault;
      MsgBox(e.Message + ".", MB_ICONERROR);
    }
    catch (Exception& e) {
      Screen->Cursor = crDefault;
      MsgBox(TRLFormat("Error while opening database file:\n%s.", e.Message.c_str()),
        MB_ICONERROR);
      return false;
    }
  }

  Screen->Cursor = crDefault;
  CloseDatabase(true);
  m_passwDb = std::move(passwDb);
  m_pSelectedItem = NULL;
  m_nSearchMode = SEARCH_MODE_OFF;
  m_blDbChanged = false;
  m_blItemChanged = false;
  m_blLocked = false;

  if (blOpenExisting) {
    ResetListView(RELOAD_TAGS);
    m_sDbFileName = sFileName;
    int nAttr = FileGetAttr(sFileName);
    m_blDbReadOnly = nAttr != faInvalid && (nAttr & faReadOnly);
  }
  else {
    AddModifyListViewEntry();
    DbView->Items->Item[DbView->Items->Count - 1]->Selected = true;
    m_sDbFileName = WString();
    m_blDbReadOnly = false;
  }

  MainMenu_File_Lock->Caption = TRL("Lock");

  DbView->Font->Color = clBlack;

  ChangeCaption();
  ResetDbOpenControls();

  if (!blUnlock && g_config.Database.WarnExpiredEntries) {
    bool blHasExpiredEntries = false;
    for (const auto pEntry : m_passwDb->GetDatabase()) {
      if (pEntry->UserFlags & DB_FLAG_EXPIRED) {
        blHasExpiredEntries = true;
        break;
      }
    }
    if (blHasExpiredEntries &&
        MsgBox(TRL("The database contains expired entries.\nDo you want to filter "
          "these entries now?"),
          MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDYES) {
      MainMenu_View_ExpiredEntries->Checked = true;
      ResetListView();
    }
  }

  NotifyUserAction();
  IdleTimer->Enabled = true;

  return true;
}
//---------------------------------------------------------------------------
int __fastcall TPasswMngForm::AskSaveChanges(int nLock)
{
  if (m_blItemChanged) {
    switch (MsgBox(TRL("The currently selected entry has been changed.\nDo you "
          "want to save the changes?"), MB_ICONQUESTION + MB_YESNOCANCEL))
    {
    case IDYES:
      AddModifyBtnClick(this);
      break;
    case IDCANCEL:
      return ASK_SAVE_CANCEL;
    }
  }

  if (m_blDbChanged) {
    if (nLock == 2 && g_config.Database.LockAutoSave) {
      MainMenu_File_SaveClick(this);
      if (m_blDbChanged)
        return ASK_SAVE_ERROR;
    }
    else {
      if (nLock == 2)
        FlashWindow(Application->Handle, true);
      switch (MsgBox(TRL("The database has been changed.\nDo you want to save "
            "the changes?"), MB_ICONQUESTION + MB_YESNOCANCEL))
      {
      case IDYES:
        MainMenu_File_SaveClick(this);
        // check whether database could be saved successfully
        // (flag should be reset to 'false')
        if (m_blDbChanged)
          return ASK_SAVE_ERROR;
        break;
      case IDNO:
        if (nLock && m_sDbFileName.IsEmpty()) {
          MsgBox(TRL("An untitled database cannot be locked."), MB_ICONWARNING);
          return ASK_SAVE_ERROR;
        }
        break;
      case IDCANCEL:
        return ASK_SAVE_CANCEL;
      }
    }
  }

  return ASK_SAVE_OK;
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::CloseDatabase(bool blForce, int nLock)
{
  if (IsDbOpen() && !blForce) {
    // do not place in main scope of this function since timer needs to be
    // *de*activated at the end!
    SuspendIdleTimer;

    if (AskSaveChanges(nLock) != ASK_SAVE_OK)
      return false;
  }

  m_passwDb.reset();
  m_tempKeyVal.reset();

  //m_pSelectedItem = NULL;

  SetItemChanged(false);

  m_nSearchMode = SEARCH_MODE_OFF;
  //m_nNumSearchResults = 0;

  //m_globalTags.clear();
  //m_searchResultTags.clear();
  m_tags.clear();
  m_globalCaseiTags.clear();
  m_tagFilter.clear();

  ClearListView();
  TagView->Clear();
  ResetDbOpenControls();
  TagMenu->Items->Clear();
  MainMenu_View_ExpiredEntries->Checked = false;
  PasswDbSettingsDlg->Clear();

  if (nLock == 0) {
    if (!g_config.Database.OpenLastDbOnStartup)
      m_sDbFileName = WString();
    m_blLocked = false;
    MainMenu_File_Lock->Caption = TRL("Lock");
    SearchBox->Clear();
  }

  ChangeCaption();
  //ClearEditPanel();
  SearchResultPanel->Caption = WString();

  IdleTimer->Enabled = false;

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ClearEditPanel(void)
{
  ClearEditBoxTextBuf(TitleBox);
  ClearEditBoxTextBuf(UserNameBox);
  ClearEditBoxTextBuf(PasswBox);
  ClearEditBoxTextBuf(UrlBox);
  ClearEditBoxTextBuf(KeywordBox);
  ClearEditBoxTextBuf(KeyValueListBox);
  ClearEditBoxTextBuf(NotesBox);
  ClearEditBoxTextBuf(TagsBox);
  ExpiryCheck->Tag = ExpiryCheck->Checked;
  ExpiryCheck->Checked = false;
  //ExpiryCheckClick(this);
  ExpiryDatePicker->Date = TDateTime::CurrentDate();
  CreationTimeInfo->Caption = WString();
  LastModificationInfo->Caption = WString();
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::SaveDatabase(const WString& sFileName)
{
  SuspendIdleTimer;

  if (g_config.Database.CreateBackup && FileExists(sFileName)) {
    static const WString BACKUP_EXT = ".bak";
    WString sBackupFileName = ExtractFilePath(sFileName) +
      TPath::GetFileNameWithoutExtension(sFileName);
    if (g_config.Database.NumberBackups) {
      const int nMaxNumBackups = g_config.Database.MaxNumBackups;
      if (nMaxNumBackups > 1) {
        sBackupFileName += "_";
        int nBackupNum = 1;
        int nOldestNum = 0;
        TDateTime oldestAge;
        for (; nBackupNum <= nMaxNumBackups; nBackupNum++) {
          WString sCheckFileName = sBackupFileName + FormatW("%.3d", nBackupNum) +
            BACKUP_EXT;
          TDateTime fileAge;
          if (FileAge(sCheckFileName, fileAge)) {
            if (nOldestNum == 0 || fileAge < oldestAge) {
              oldestAge = fileAge;
              nOldestNum = nBackupNum;
            }
          }
          else
            break;
        }
        if (nBackupNum > nMaxNumBackups)
          nBackupNum = std::max(1, nOldestNum);
        sBackupFileName += FormatW("%.3d", nBackupNum) + BACKUP_EXT;
      }
      else
        sBackupFileName += BACKUP_EXT;
    }
    else {
      SYSTEMTIME st;
      GetLocalTime(&st);
      sBackupFileName += FormatW("_%d%.2d%.2d%.2d%.2d%.2d", st.wYear,
        st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
      if (FileExists(sBackupFileName + BACKUP_EXT))
        sBackupFileName += FormatW("_%.3d", st.wMilliseconds);
      sBackupFileName += BACKUP_EXT;
    }
    // close file in case target file is equal to currently opened file
    m_passwDb->ReleaseFile();
    if (!CopyFile(sFileName.c_str(), sBackupFileName.c_str(), false))
      MsgBox(TRLFormat("Could not save backup file\n\"%s\".", sBackupFileName.c_str()),
        MB_ICONERROR);
  }

  WString sError;
  try {
    m_passwDb->SaveToFile(sFileName);
  }
  catch (Exception& e) {
    sError = e.Message;
  }
  catch (std::exception& e) {
    sError = CppStdExceptionToString(&e);
  }

  if (!sError.IsEmpty()) {
    MsgBox(TRLFormat("Error while saving database file:\n%s.", sError.c_str()),
      MB_ICONERROR);
    return false;
  }

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ResetDbOpenControls(void)
{
  bool blOpen = IsDbOpen();

  MainMenu_File_Close->Enabled = blOpen;
  MainMenu_File_Lock->Enabled = blOpen;
  MainMenu_File_Save->Enabled = blOpen && !m_blDbReadOnly;
  MainMenu_File_SaveAs->Enabled = blOpen;
  MainMenu_File_Export->Enabled = blOpen;
  MainMenu_File_DbSettings->Enabled = blOpen;
  MainMenu_File_Properties->Enabled = blOpen;
  MainMenu_File_ChangeMasterPassword->Enabled = blOpen;

  MainMenu_Edit_CopyUserName->Enabled = blOpen;
  MainMenu_Edit_CopyPassw->Enabled = blOpen;
  MainMenu_Edit_OpenUrl->Enabled = blOpen;
  MainMenu_Edit_Run->Enabled = blOpen;
  MainMenu_Edit_PerformAutotype->Enabled = blOpen;
  MainMenu_Edit_AddEntry->Enabled = blOpen;
  MainMenu_Edit_DuplicateEntry->Enabled = blOpen;
  MainMenu_Edit_DeleteEntry->Enabled = blOpen;
  MainMenu_Edit_SelectAll->Enabled = blOpen;
  MainMenu_Edit_Rearrange->Enabled = blOpen;

  DbView->Enabled = blOpen;
  EditPanel->Enabled = false; //blOpen; // will be enabled upon item selection

  SaveBtn->Enabled = blOpen && !m_blDbReadOnly;
  LockBtn->Enabled = blOpen;
  AddEntryBtn->Enabled = blOpen;
  SearchBox->Enabled = blOpen;
  ClearSearchBtn->Enabled = blOpen;
  CaseSensitiveBtn->Enabled = blOpen;
  SearchBtn->Enabled = blOpen;
  FuzzyBtn->Enabled = blOpen;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SetItemChanged(bool blChanged)
{
  m_blItemChanged = blChanged;

  AddModifyBtn->Visible = m_blItemChanged;
  CancelBtn->Visible = m_blItemChanged;

  if (IsDbOpen()) {
    DbView->Enabled = !m_blItemChanged;
    TagView->Enabled = !m_blItemChanged;
    //ToolBar->Enabled = !m_blItemChanged;
    AddEntryBtn->Enabled = !m_blItemChanged;
    SearchBox->Enabled = !m_blItemChanged;
    ClearSearchBtn->Enabled = !m_blItemChanged;
    CaseSensitiveBtn->Enabled = !m_blItemChanged;
    SearchBtn->Enabled = !m_blItemChanged;
    FuzzyBtn->Enabled = !m_blItemChanged;
  }

  //MainMenu_File->Enabled = !m_blItemChanged;
  MainMenu_Edit->Enabled = !m_blItemChanged;
  MainMenu_View->Enabled = !m_blItemChanged;

  if (m_blItemChanged) {
    PrevBtn->Visible = false;
    NextBtn->Visible = false;
    DeleteBtn->Visible = false;
  }
  else
    ResetNavControls();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ResetNavControls(void)
{
  int nNumSel = DbView->SelCount;
  bool blSingleSel = nNumSel == 1 && m_pSelectedItem != NULL;
  PrevBtn->Visible = blSingleSel && m_pSelectedItem->Index > 0;
  NextBtn->Visible = blSingleSel && m_pSelectedItem->Index < DbView->Items->Count - 1;
  DeleteBtn->Visible = nNumSel > 1 || (blSingleSel && m_pSelectedItem->Data != NULL);
}
//---------------------------------------------------------------------------
int __fastcall TPasswMngForm::AddPassw(const wchar_t* pwszPassw,
  bool blShowNumPassw,
  const wchar_t* pwszParam)
{
  if (pwszPassw == NULL || pwszPassw[0] == '\0' || !IsDbOpen())
    return 0;

  if (m_blItemChanged) {
    MsgBox(TRL("Please finish editing the current entry\nbefore adding new entries."),
      MB_ICONWARNING);
    return 0;
  }

  const wchar_t* pwszNext;
  int nNumPassw = 0;
  do {
    pwszNext = wcsstr(pwszPassw, CRLF);

    int nLen = (pwszNext != NULL) ? static_cast<int>(pwszNext - pwszPassw) :
      wcslen(pwszPassw);

    if (nLen > 0) {
      SecureWString sPassw(nLen + 1);
      wcsncpy(sPassw, pwszPassw, nLen);
      sPassw[nLen] = '\0';

      PasswDbEntry* pNewEntry = m_passwDb->AddDbEntry();
      m_passwDb->SetDbEntryPassw(pNewEntry, sPassw);

      if (pwszParam != NULL && pwszParam[0] != '\0') {
        SecureWString& sParam = pNewEntry->Strings[PasswDbEntry::TITLE];
        sParam.Assign(pwszParam, wcslen(pwszParam) + 1);
      }

      nNumPassw++;
    }

    if (pwszNext != NULL)
      pwszPassw = pwszNext + 2;
  }
  while (pwszNext != NULL);

  if (nNumPassw > 0)
    SetDbChanged(true, true);

  ResetListView(RELOAD_TAGS);

  if (blShowNumPassw) {
    SuspendIdleTimer;
    MsgBox(TRLFormat("%d entries with passwords have been added.", nNumPassw),
      MB_ICONINFORMATION);
  }

  return nNumPassw;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchDatabase(WString sStr,
  int nFlags,
  bool blCaseSensitive,
  bool blFuzzy)
{
  if (!IsDbOpen() || sStr.IsEmpty() || nFlags == 0)
    return;

  blFuzzy = blFuzzy && sStr.Length() >= 4;
  if (blFuzzy)
    blCaseSensitive = false;

  if (!blCaseSensitive)
    sStr = AnsiLowerCase(sStr);

  m_nSearchMode = blFuzzy ? SEARCH_MODE_FUZZY : SEARCH_MODE_NORMAL;

  //ClearListView();
  //DbView->Items->BeginUpdate();

  PasswDbList& db = m_passwDb->GetDatabase();
  SecureWString sBuf;

  if (!blCaseSensitive)
    sBuf.New(16384);

  int nNumFound = 0;

  for (PasswDbList::iterator it = db.begin(); it != db.end(); it++)
  {
    PasswDbEntry* pEntry = *it;
    pEntry->UserFlags &= ~DB_FLAG_FOUND;
    //sBuf[0] = '\0';
    //word32 lBufPos = 0;
    for (int nI = 0; nI < PasswDbEntry::NUM_STRING_FIELDS; nI++) {
      if (nFlags & (1 << nI)) {
        const SecureWString* psSrc;
        SecureWString sPassw;
        if (nI == PasswDbEntry::PASSWORD && !pEntry->HasPlaintextPassw()) {
          m_passwDb->GetDbEntryPassw(pEntry, sPassw);
          psSrc = &sPassw;
        }
        else
          psSrc = &pEntry->Strings[nI];
        if (psSrc->Size() > 1) {
          //sBuf.Grow(lBufPos + psSrc->Size());
          if (!blCaseSensitive) {
            //wcscpy(&sBuf[lBufPos], psSrc->c_str());
            if (psSrc->Size() > sBuf.Size())
              sBuf.Resize(psSrc->Size(), false);
            wcscpy(sBuf, psSrc->c_str());
            CharLower(sBuf);
            psSrc = &sBuf;
          }
          float fScore;
          if (blFuzzy)
            fScore = strFindFuzzy(psSrc->c_str(), sStr.c_str());
          else
            fScore = wcsstr(psSrc->c_str(), sStr.c_str()) ? 1 : 0;
          if (fScore >= 0.5) {
            pEntry->UserFlags |= DB_FLAG_FOUND;
            pEntry->UserTag = fScore * 100;
            //AddModifyListViewEntry(NULL, pEntry);
            nNumFound++;
            break;
          }
        }
      }
    }
  }

  //m_pSelectedItem = NULL;
  //SetItemChanged(false);
  //ResetNavControls();

  //DbView->Items->EndUpdate();

  DbView->Font->Color = clBlue;

  //m_nNumSearchResults = nNumFound;

  DbView->Selected = nullptr;

  ResetListView(RELOAD_TAGS);

  SearchResultPanel->Caption = TRLFormat(nNumFound == 1 ? "1 entry found." :
    "%d entries found.", nNumFound);

  if (DbView->Items->Count > 0)
    DbView->Items->Item[0]->Selected = true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchDbForKeyword(bool blAutotype)
{
  if (!IsDbOpen() || m_blItemChanged)
    return;

  PasswDbEntry* pFound = NULL;
  WString sWinTitle;
  int nNumFound = 0;

  HWND hWin = GetForegroundWindow();
  if (hWin != INVALID_HANDLE_VALUE) {
    const int BUFSIZE = 256;
    wchar_t wszWinTitle[BUFSIZE];
    if (GetWindowText(hWin, wszWinTitle, BUFSIZE) != 0) {
      sWinTitle = wszWinTitle;
      CharLower(wszWinTitle);
      PasswDbList& db = m_passwDb->GetDatabase();
      for (auto pEntry : db) {
        if (!blAutotype)
          pEntry->UserFlags &= ~DB_FLAG_FOUND;
        if (pEntry->Strings[PasswDbEntry::KEYWORD].Size() > 1) {
          SecureWString sKeyword = pEntry->Strings[PasswDbEntry::KEYWORD];
          CharLower(sKeyword.Data());
          if (wcsstr(wszWinTitle, sKeyword) != NULL) {
            nNumFound++;
            if (pFound == nullptr)
              pFound = pEntry;

            if (blAutotype)
              break;

            // add "found" flag for the found item
            pEntry->UserFlags |= DB_FLAG_FOUND;
          }
        }
      }
    }
  }

  WString sMsg;
  if (!pFound)
    sMsg = TRLFormat("No matching keyword found for window text\n\"%s\".",
        sWinTitle.c_str());

  if (blAutotype) {
    if (pFound) {
      SendKeys sk(g_config.AutotypeDelay);
      sk.SendComplexString(getDbEntryAutotypeSeq(pFound), pFound, m_passwDb.get());
    }
    else
      MainForm->ShowTrayInfo(sMsg, bfWarning);
  }
  else {
    if (pFound) {
      m_nSearchMode = SEARCH_MODE_NORMAL;
      //m_nNumSearchResults = nNumFound;
      DbView->Font->Color = clBlue;
      DbView->Selected = nullptr;
      ResetListView(RELOAD_TAGS);
      DbView->Items->Item[0]->Selected = true;
      ResetNavControls();
      SearchBox->Text = WString(pFound->Strings[PasswDbEntry::KEYWORD].c_str());
      SearchResultPanel->Caption = TRL("Search via hot key.");
    }

    if (g_nAppState & APPSTATE_MINIMIZED)
      Application->Restore();
    Application->BringToFront();
    WindowState = wsNormal;
    SetFocus();

    if (!pFound)
      MsgBox(sMsg, MB_ICONWARNING);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::AddModifyListViewEntry(TListItem* pItem,
  PasswDbEntry* pEntry)
{
  if (m_nShowColMask == 0)
    return;

  bool blAdd = pItem == NULL;

  if (blAdd)
    pItem = DbView->Items->Add();

  pItem->Data = pEntry;
  if (pEntry && (pEntry->UserFlags & DB_FLAG_EXPIRED)) {
    pItem->ImageIndex = 0;
    pItem->Indent = 0;
  }
  else {
    pItem->ImageIndex = -1;
    pItem->Indent = -1;
  }

  bool blShowPassw = MainMenu_View_ShowPasswInList->Checked;

  for (int nI = 0, nColIdx = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    if (m_nShowColMask & (1 << nI)) {
      if (pEntry != NULL) {
        const wchar_t* pwszSrc = NULL;
        SecureWString sParam;
        if (nI == PasswDbEntry::PASSWORD) {
          if (!blShowPassw)
            pwszSrc = HIDE_PASSW_STR;
          else if (!pEntry->HasPlaintextPassw()) {
            m_passwDb->GetDbEntryPassw(pEntry, sParam);
            pwszSrc = sParam.c_str();
          }
        }
        if (pwszSrc == NULL) {
          switch (nI) {
          case PasswDbEntry::CREATIONTIME:
            pwszSrc = pEntry->CreationTimeString.c_str();
            break;
          case PasswDbEntry::MODIFICATIONTIME:
            pwszSrc = pEntry->ModificationTimeString.c_str();
            break;
          case PasswDbEntry::PASSWEXPIRYDATE:
            pwszSrc = pEntry->PasswExpiryDateString.c_str();
            break;
          default:
            pwszSrc = pEntry->Strings[nI].c_str();
          }
        }
        if (nColIdx == 0) {
          if (m_nSearchMode == SEARCH_MODE_FUZZY)
            pItem->Caption = FormatW("[%d%%] %s",
              std::min(100, pEntry->UserTag), pwszSrc);
          else
            pItem->Caption = WString(pwszSrc);
        }
        else if (blAdd)
          pItem->SubItems->Add(WString(pwszSrc));
        else
          pItem->SubItems->Strings[nColIdx - 1] = WString(pwszSrc);
      }
      else {
        if (nColIdx == 0)
          pItem->Caption = WString('<') + TRL("New Entry") + WString('>');
        else
          pItem->SubItems->Add(WString());
      }
      nColIdx++;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ScrollToItem(int nIndex)
{
  TRect rect = DbView->Items->Item[nIndex]->DisplayRect(drBounds);
  DbView->Scroll(0, rect.Top - DbView->ClientHeight / 2);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ResetListView(int nFlags)
{
  void* pPrevSelData = (m_pSelectedItem != NULL) ? m_pSelectedItem->Data : NULL;

  SCROLLINFO scrollInfo;
  scrollInfo.cbSize = sizeof(SCROLLINFO);
  scrollInfo.fMask = SIF_POS;
  GetScrollInfo(DbView->Handle, SB_VERT, &scrollInfo);

  //DbView->Clear();
  ClearListView();

  if (m_nShowColMask == 0)
    return;

  if (nFlags & RESET_COLUMNS) {
    TListColumns* pCols = DbView->Columns;

    std::vector<int> colWidths(pCols->Count);
    if (m_listColWidths.empty()) {
      for (int nI = 0; nI < pCols->Count; nI++)
        colWidths[nI] = pCols->Items[nI]->Width;
    }
    else {
      colWidths = m_listColWidths;
      m_listColWidths.clear();
    }

    pCols->Clear();
    for (int nI = 0, nJ = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
      if (m_nShowColMask & (1 << nI)) {
        TListColumn* pCol = pCols->Add();
        pCol->Caption = m_uiFieldNames[nI];
        pCol->Tag = nI;
        if (nJ < colWidths.size())
          pCol->Width = colWidths[nJ++];
      }
    }
  }

  if (IsDbOpen()) {
    DbView->Items->BeginUpdate();

    PasswDbList& db = m_passwDb->GetDatabase();

    if (nFlags & RELOAD_TAGS) {
      std::map<SecureWString, word32> tags, searchResultTags;
      word32 lNumWithoutTags = 0;
      word32 lNumWithoutTagsSearch = 0;
      word32 lNumSearchResults = 0;
      for (const auto pEntry : db) {
        if (pEntry->GetTagList().size() == 0)
          lNumWithoutTags++;
        else {
          for (const auto& sTag : pEntry->GetTagList()) {
            auto ret = tags.emplace(sTag, 0);
            ret.first->second++;
          }
        }
        if (m_nSearchMode != SEARCH_MODE_OFF &&
            (pEntry->UserFlags & DB_FLAG_FOUND)) {
          lNumSearchResults++;
          if (pEntry->GetTagList().size() == 0)
            lNumWithoutTagsSearch++;
          else {
            for (const auto& sTag : pEntry->GetTagList()) {
              auto ret = searchResultTags.emplace(sTag, 0);
              ret.first->second++;
            }
          }
        }
      }

      //m_globalTags = tags;
      //m_searchResultTags = searchResultTags;

      m_globalCaseiTags.clear();
      for (const auto& kv : tags) {
        SecureWString ciTag(kv.first);
        CharLower(ciTag.Data());
        m_globalCaseiTags.emplace(ciTag, kv.first);
      }

      m_tags.clear();

      const auto& src = (m_nSearchMode == SEARCH_MODE_OFF) ? tags :
        searchResultTags;

      TagView->Tag = 1;
      TagView->Clear();
      TagView->Items->BeginUpdate();

      auto pItem = TagView->Items->Add();
      pItem->Caption = (m_nSearchMode == SEARCH_MODE_OFF ? TRL("All", "tags") :
          TRL("All search results")) + FormatW(" (%d)",
          (m_nSearchMode == SEARCH_MODE_OFF) ?
            m_passwDb->Size : lNumSearchResults);
      pItem->ImageIndex = 1;
      pItem->Data = nullptr;

      std::set<SecureWString> newTagFilter;

      for (const auto& kv : src) {
        const auto& sTag = kv.first;

        m_tags.emplace_back(sTag, kv.second);

        auto pItem = TagView->Items->Add();

        pItem->Caption = WString(sTag.c_str()) + FormatW(" (%d)", kv.second);
        pItem->ImageIndex = 0;
        pItem->Data = &m_tags.back();

        if (m_tagFilter.count(sTag) != 0) {
          newTagFilter.insert(sTag);
          pItem->Selected = true;
        }
      }

      word32 lBaseNumWithout = (m_nSearchMode == SEARCH_MODE_OFF) ?
        lNumWithoutTags : lNumWithoutTagsSearch;
      if (lBaseNumWithout) {
        m_tags.emplace_back(SecureWString(), lBaseNumWithout);
        pItem = TagView->Items->Add();
        pItem->Caption = TRL("Untagged") + FormatW(" (%d)", lBaseNumWithout);
        pItem->ImageIndex = 2;
        pItem->Data = &m_tags.back();
      }

      m_tagFilter = newTagFilter;

      TagView->Items->EndUpdate();
      TagView->Tag = 0;
      //TagView->AlphaSort();
      SetScrollRange(TagView->Handle, SB_HORZ, 0, 0, true);

      TagMenu->Items->Clear();
      for (const auto& kv : tags) {
        auto pItem = new TMenuItem(TagMenu);
        pItem->Caption = ReplaceStr(WString(kv.first.c_str()), "&", "&&");
        pItem->OnClick = OnTagMenuItemClick;
        TagMenu->Items->Add(pItem);
      }
    }

    int nPrevSelIdx = -1, nIdx = 0;
    unsigned short wYear, wMonth, wDay;
    TDateTime::CurrentDate().DecodeDate(&wYear, &wMonth, &wDay);
    word32 lCurrDate = PasswDbEntry::EncodeExpiryDate(wYear, wMonth, wDay);

    bool blFilterExpired = MainMenu_View_ExpiredEntries->Checked;
    std::map<SecureWString, SecureWString> userNamesMap;

    for (const auto pEntry : db) {
      if (pEntry->Strings[PasswDbEntry::USERNAME].Size() >= 2) {
        SecureWString sUserNameLC = pEntry->Strings[PasswDbEntry::USERNAME];
        CharLower(sUserNameLC.Data());
        userNamesMap.emplace(sUserNameLC, pEntry->Strings[PasswDbEntry::USERNAME]);
      }

      if (pEntry->PasswExpiryDate != 0 && lCurrDate >= pEntry->PasswExpiryDate)
        pEntry->UserFlags |= DB_FLAG_EXPIRED;
      else
        pEntry->UserFlags &= ~DB_FLAG_EXPIRED;

      if ((blFilterExpired && !(pEntry->UserFlags & DB_FLAG_EXPIRED)) ||
          (m_nSearchMode != SEARCH_MODE_OFF && !(pEntry->UserFlags & DB_FLAG_FOUND)))
        continue;

      if (!m_tagFilter.empty()) {
        bool blMatch = false;
        if (pEntry->GetTagList().size() == 0) {
          if (m_tagFilter.count(SecureWString()) != 0)
            blMatch = true;
        }
        else {
          for (const auto& sFilter : m_tagFilter) {
            if (pEntry->CheckTag(sFilter)) {
              blMatch = true;
              break;
            }
          }
        }
        if (!blMatch)
          continue;
      }

      AddModifyListViewEntry(NULL, pEntry);
      if (pPrevSelData == pEntry)
        nPrevSelIdx = nIdx;

      nIdx++;
    }

    if (m_nSearchMode == SEARCH_MODE_OFF && !blFilterExpired)
      AddModifyListViewEntry();

    if (nPrevSelIdx >= 0)
      DbView->Items->Item[nPrevSelIdx]->Selected = true;
    else
      DbViewSelectItem(this, NULL, false);

    if (DbView->Items->Count > 2 && m_nSearchMode == SEARCH_MODE_FUZZY &&
        m_nSortByIdx < 0)
      DbView->AlphaSort();

    DbView->Items->EndUpdate();

    if (nPrevSelIdx >= 0)
      ScrollToItem(nPrevSelIdx);

    if (!userNamesMap.empty()) {
      std::vector<SecureWString> userNames;
      for (const auto& kv : userNamesMap) {
        userNames.push_back(kv.second);
      }

      //std::sort(userNames.begin(), userNames.end());
      //for (auto& s:userNames)
      //  ShowMessage(s.c_str());

      InitAutoComplete(UserNameBox->Handle, userNames);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ClearListView(void)
{
  m_pSelectedItem = NULL;
  DbView->Clear();
  ClearEditPanel();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SetShowColMask(void)
{
  m_nShowColMask = 0;
  for (int nI = 0; nI < MainMenu_View_ShowColumns->Count; nI++) {
    if (MainMenu_View_ShowColumns->Items[nI]->Checked)
      m_nShowColMask |= (1 << nI);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MoveDbEntries(int nDir)
{
  const word32 lDbSize = m_passwDb->Size;
  bool blChanged = false;
  for (word32 i = 0, j = 0; i < DbView->Items->Count; i++) {
    if (DbView->Items->Item[i]->Selected) {
      PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(
        DbView->Items->Item[i]->Data);
      if (pEntry != NULL) {
        word32 lPos = pEntry->GetIndex();
        word32 lNewPos = -1;
        if (nDir == MOVE_TOP)
          lNewPos = j++;
        if (nDir == MOVE_UP && lPos > 0)
          lNewPos = lPos - 1;
        else if (nDir == MOVE_DOWN && lPos < lDbSize - 1)
          lNewPos = lPos + 1;
        else if (nDir == MOVE_BOTTOM)
          lNewPos = lDbSize - 1;
        if (lNewPos != -1 && lNewPos != lPos) {
          m_passwDb->MoveDbEntry(lPos, lNewPos);
          blChanged = true;
        }
      }
    }
  }

  if (blChanged) {
    SetDbChanged();
    ResetListView();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_NewClick(TObject *Sender)
{
  NotifyUserAction();
  OpenDatabase(false);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewSelectItem(TObject *Sender,
  TListItem *Item, bool Selected)
{
  NotifyUserAction();

  //TListItem* pItem = reinterpret_cast<TListItem*>(Item);
  // return if selected item hasn't changed or is unselected
  //if (pItem == m_pSelectedItem && Selected && !m_blItemChanged)
  //  return;

  // avoid OnChange() procedures when text boxes change
  Tag = FORM_TAG_ITEM_SELECTED;

  int nNumSel = DbView->SelCount;

  EditPanel->Enabled = nNumSel > 0;

  if (nNumSel == 1 && Item->Data != NULL) {
    PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(Item->Data);
    SetEditBoxTextBuf(TitleBox, pEntry->Strings[PasswDbEntry::TITLE].c_str());
    SetEditBoxTextBuf(UserNameBox, pEntry->Strings[PasswDbEntry::USERNAME].c_str());
    SetEditBoxTextBuf(UrlBox, pEntry->Strings[PasswDbEntry::URL].c_str());
    SetEditBoxTextBuf(KeywordBox, pEntry->Strings[PasswDbEntry::KEYWORD].c_str());
    SetEditBoxTextBuf(KeyValueListBox, BuildTranslKeyValString(pEntry->GetKeyValueList()));
    SetEditBoxTextBuf(NotesBox, pEntry->Strings[PasswDbEntry::NOTES].c_str());

    if (!pEntry->GetTagList().empty()) {
      SecureWString sTags(1024);
      word32 i = 0;
      word32 lCount = pEntry->GetTagList().size();
      word32 lPos = 0;
      for (const auto& sTag : pEntry->GetTagList()) {
        sTags.Grow(lPos + sTag.StrLen() + 2);
        wcscpy(sTags + lPos, sTag.c_str());
        lPos += sTag.StrLen();
        if (i < lCount - 1) {
          wcscpy(sTags + lPos, L", ");
          lPos += 2;
        }
        else
          sTags[lPos] = '\0';
        i++;
      }

      SetEditBoxTextBuf(TagsBox, sTags.c_str());
    }

    bool blValidDate = false;
    if (pEntry->PasswExpiryDate != 0) {
      int nYear, nMonth, nDay;
      if (pEntry->DecodeExpiryDate(pEntry->PasswExpiryDate, nYear, nMonth, nDay)) {
        try {
          ExpiryDatePicker->Date = TDateTime(nYear, nMonth, nDay);
          ExpiryCheck->Tag = !ExpiryCheck->Checked;
          ExpiryCheck->Checked = true;
          ExpiryDatePicker->Enabled = true;
          blValidDate = true;
        }
        catch (...) {
        }
      }
    }

    if (!blValidDate) {
      ExpiryCheck->Tag = ExpiryCheck->Checked;
      ExpiryCheck->Checked = false;
      ExpiryDatePicker->Date = TDateTime::CurrentDate();
    }

    //ExpiryCheckClick(this);

    CreationTimeInfo->Caption = WString(pEntry->CreationTimeString.c_str());
    LastModificationInfo->Caption = WString(pEntry->ModificationTimeString.c_str());

    if (pEntry->HasPlaintextPassw())
      SetEditBoxTextBuf(PasswBox, pEntry->Strings[PasswDbEntry::PASSWORD].c_str());
    else {
      SecureWString sPassw;
      m_passwDb->GetDbEntryPassw(pEntry, sPassw);
      SetEditBoxTextBuf(PasswBox, sPassw.c_str());
    }

    m_tempKeyVal.reset();
  }
  else if (nNumSel <= 2) {
    ClearEditPanel();

    if (nNumSel == 1 && Item->Data == NULL) {
      SetEditBoxTextBuf(UserNameBox, m_passwDb->DefaultUserName.c_str());

      if (m_passwDb->PasswFormatSeq.Size() > 1) {
        w32string sFormat = WCharToW32String(m_passwDb->PasswFormatSeq.c_str());
        PasswordGenerator passwGen(RandomPool::GetInstance());
        SecureW32String sDest(16001);
        if (passwGen.GetFormatPassw(sDest, sDest.Size() - 1, sFormat, 0) != 0)
        {
          W32CharToWCharInternal(sDest);
          SetEditBoxTextBuf(PasswBox, reinterpret_cast<wchar_t*>(sDest.Data()));
        }
        eraseStlString(sFormat);
      }

      if (m_passwDb->DefaultPasswExpiryDays != 0) {
        ExpiryCheck->Tag = !ExpiryCheck->Checked;
        ExpiryCheck->Checked = true;
        auto date = TDateTime::CurrentDate() + static_cast<double>(
          m_passwDb->DefaultPasswExpiryDays);
        ExpiryDatePicker->Date = date;
        ExpiryDatePicker->Enabled = true;
      }
    }
  }

  m_pSelectedItem = (nNumSel == 1) ? Item : NULL;
  Tag = 0;

  if (m_blItemChanged)
    SetItemChanged(false);
  else
    ResetNavControls();
  //if (DbView->Enabled)
  //  DbView->SetFocus();
}
//---------------------------------------------------------------------------
const wchar_t* __fastcall TPasswMngForm::DbKeyValNameToKey(const wchar_t* pwszName)
{
  std::wstring sName(pwszName);
  for (const auto& kv : m_keyValNames) {
    if (kv.second == sName)
      return kv.first.c_str();
  }
  return nullptr;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::AddModifyBtnClick(TObject *Sender)
{
  NotifyUserAction();

  if (GetEditBoxTextLen(TitleBox) == 0 && GetEditBoxTextLen(PasswBox) == 0)
  {
    MsgBox(TRL("Please specify at least a title or password."), MB_ICONWARNING);
    return;
  }

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  bool blNewEntry = false;

  if (pEntry == NULL) {
    pEntry = m_passwDb->AddDbEntry();
    m_pSelectedItem->Data = pEntry;
    //AddModifyListViewEntry();
    blNewEntry = true;
  }

  SecureWString sTitle, sUserName, sUrl, sKeyword, sNotes;
  GetEditBoxTextBuf(TitleBox, pEntry->Strings[PasswDbEntry::TITLE]);
  GetEditBoxTextBuf(UserNameBox, pEntry->Strings[PasswDbEntry::USERNAME]);
  GetEditBoxTextBuf(UrlBox, pEntry->Strings[PasswDbEntry::URL]);
  GetEditBoxTextBuf(KeywordBox, pEntry->Strings[PasswDbEntry::KEYWORD]);
  GetEditBoxTextBuf(NotesBox, pEntry->Strings[PasswDbEntry::NOTES]);

  if (m_tempKeyVal) {
    pEntry->SetKeyValueList(*m_tempKeyVal.get());
    pEntry->UpdateKeyValueString();
  }

  //bool blResetView = false;
  pEntry->ClearTagList();

  int nNumOverlongTags = 0;

  if (GetEditBoxTextLen(TagsBox) != 0) {
    SecureWString sTags;
    GetEditBoxTextBuf(TagsBox, sTags);
    const wchar_t* p = sTags.c_str();

    std::map<SecureWString,SecureWString> newTags;

    while (*p != '\0') {
      const wchar_t* pSep = wcspbrk(p, L",;");

      if (pSep == p) {
        p++;
        continue;
      }

      if (pSep == NULL)
        pSep = wcschr(p, '\0');

      const wchar_t* pStart = p;

      // trim left side
      for (; pStart != pSep && *pStart == ' '; pStart++);

      if (pStart == pSep) {
        p = pSep + 1;
        continue;
      }

      const wchar_t* pEnd = pSep - 1;

      // trim right side
      for (; pEnd != pStart && *pEnd == ' '; pEnd--);

      word32 lTagLen = static_cast<word32>(pEnd - pStart) + 1;
      if (lTagLen > DB_MAX_TAG_LEN) {
        lTagLen = DB_MAX_TAG_LEN;
        nNumOverlongTags++;
      }
      SecureWString sTag(pStart, lTagLen + 1);
      sTag[lTagLen] = '\0';

      SecureWString sCiTag = sTag;
      CharLower(sCiTag.Data());

      auto it = m_globalCaseiTags.find(sCiTag);
      if (it != m_globalCaseiTags.end())
        newTags.emplace(it->first, it->second);
      else
        newTags.emplace(sCiTag, sTag);

      if (*pSep == '\0')
        break;

      p = pSep + 1;
    }

    /*bool blTagsDiff = false;

    for (const auto& kv : newTags) {
      if (!pEntry->CheckTag(kv.second)) {
        blTagsDiff = true;
        break;
      }
    }

    if (blTagsDiff) {
      pEntry->ClearTagList();
      for (const auto& kv : newTags) {
        pEntry->AddTag(kv.second.c_str());
      }

      blResetView = true;
    }*/

    for (const auto& kv : newTags) {
      pEntry->AddTag(kv.second);
    }

    pEntry->UpdateTagsString();
  }

  SecureWString sPassw;
  GetEditBoxTextBuf(PasswBox, sPassw);
  m_passwDb->SetDbEntryPassw(pEntry, sPassw);

  word32 lExpiryDate = 0;
  if (ExpiryCheck->Checked) {
    unsigned short wYear, wMonth, wDay;
    ExpiryDatePicker->Date.DecodeDate(&wYear, &wMonth, &wDay);
    lExpiryDate = pEntry->EncodeExpiryDate(wYear, wMonth, wDay);
  }
  pEntry->PasswExpiryDate = lExpiryDate;
  pEntry->ExpiryDateToString(lExpiryDate, pEntry->PasswExpiryDateString);

  pEntry->UpdateModificationTime();

  if (m_tags.empty() && pEntry->GetTagList().empty()) {
    AddModifyListViewEntry(m_pSelectedItem, pEntry);
    if (blNewEntry) {
      AddModifyListViewEntry();
      WString sAll = TRL("All") + FormatW(" (%d)", m_passwDb->Size);
      if (TagView->Items->Count > 0)
        TagView->Items->Item[0]->Caption = sAll;
      else
        TagView->Items->Add()->Caption = sAll;
    }

    //if (m_nSortByIdx >= 0 || m_nSearchMode == SEARCH_MODE_FUZZY)
    //  DbView->AlphaSort();
  }
  else
    ResetListView(RELOAD_TAGS);

  SetItemChanged(false);
  SetDbChanged(true, true);
  m_tempKeyVal.reset();

  if (nNumOverlongTags > 0) {
    MsgBox(TRLFormat("The maximum tag length is %d characters.\n"
      "Longer tags have been shortened.", DB_MAX_TAG_LEN), MB_ICONWARNING);
  }

  NextBtnClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PrevBtnClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Index == 0)
    return;

  int nIdx = m_pSelectedItem->Index;
  m_pSelectedItem->Selected = false;
  DbView->Items->Item[nIdx - 1]->Selected = true;
  ScrollToItem(nIdx - 1);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NextBtnClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Index == DbView->Items->Count - 1)
    return;

  int nIdx = m_pSelectedItem->Index;
  m_pSelectedItem->Selected = false;
  DbView->Items->Item[nIdx + 1]->Selected = true;
  ScrollToItem(nIdx + 1);
  DbView->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_SaveAsClick(TObject *Sender)
{
  NotifyUserAction();

  SuspendIdleTimer;

  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();

  if (blSuccess && SaveDatabase(SaveDlg->FileName)) {
    m_sDbFileName = SaveDlg->FileName;
    m_blDbReadOnly = false;
    m_blDbChanged = false;
    ChangeCaption();
    ResetDbOpenControls(); // in case read-only flag changed
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_OpenClick(TObject *Sender)
{
  NotifyUserAction();

  OpenDatabase(true);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_CloseClick(TObject *Sender)
{
  NotifyUserAction();

  CloseDatabase();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::CancelBtnClick(TObject *Sender)
{
  NotifyUserAction();

  ClearEditPanel();
  DbViewSelectItem(this, m_pSelectedItem, true);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ShowColClick(TObject *Sender)
{
  NotifyUserAction();

  SetShowColMask();
  ResetListView(RESET_COLUMNS);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ShowPasswInListClick(
  TObject *Sender)
{
  NotifyUserAction();

  ResetListView();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TitleBoxChange(TObject *Sender)
{
  if (m_pSelectedItem != NULL && Tag != FORM_TAG_ITEM_SELECTED && !m_blItemChanged) {
    NotifyUserAction();
    SetItemChanged(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DeleteBtnClick(TObject *Sender)
{
  NotifyUserAction();

  if (DbView->SelCount == 0)
    return;

  SuspendIdleTimer;

  int nNumValid = 0;
  for (int nI = 0; nI < DbView->Items->Count; nI++) {
    if (DbView->Items->Item[nI]->Selected) {
      PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>
        (DbView->Items->Item[nI]->Data);
      if (pEntry != NULL)
        nNumValid++;
    }
  }

  if (MsgBox(TRLFormat(nNumValid == 1 ? "Delete 1 entry?" : "Delete %d entries?",
        nNumValid), MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDNO)
    return;

  for (int nI = 0; nI < DbView->Items->Count; nI++) {
    if (DbView->Items->Item[nI]->Selected) {
      PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>
        (DbView->Items->Item[nI]->Data);
      if (pEntry != NULL)
        m_passwDb->DeleteDbEntry(pEntry);
    }
  }

  ResetListView(RELOAD_TAGS);
  ResetNavControls();
  SetDbChanged();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_SortByClick(TObject *Sender)
{
  NotifyUserAction();

  int nIdx = reinterpret_cast<TMenuItem*>(Sender)->Tag;
  if (nIdx != m_nSortByIdx) {
    m_nSortByIdx = nIdx;
    DbView->SortType = (nIdx >= 0) ? TSortType::stData : TSortType::stNone;
    DbView->AlphaSort();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewCompare(TObject *Sender,
  TListItem *Item1, TListItem *Item2, int Data, int &Compare)
{
  PasswDbEntry* pEntry1 = reinterpret_cast<PasswDbEntry*>(Item1->Data);
  PasswDbEntry* pEntry2 = reinterpret_cast<PasswDbEntry*>(Item2->Data);

  if (pEntry1 == NULL || pEntry2 == NULL)
    Compare = (pEntry1 == NULL) ? 1 : -1;
  else if (m_nSearchMode == SEARCH_MODE_FUZZY)
    Compare = Sign(pEntry2->UserTag - pEntry1->UserTag);
  else {
    switch (m_nSortByIdx) {
    case -1:
      Compare = pEntry1->GetIndex() - pEntry2->GetIndex();
      break;
    case PasswDbEntry::PASSWORD:
      if (pEntry1->HasPlaintextPassw() && pEntry2->HasPlaintextPassw())
        Compare = wcscmp(pEntry1->Strings[PasswDbEntry::PASSWORD].c_str(),
            pEntry2->Strings[PasswDbEntry::PASSWORD].c_str()) * m_nSortOrderFactor;
      else {
        SecureWString sPassw1, sPassw2;
        m_passwDb->GetDbEntryPassw(pEntry1, sPassw1);
        m_passwDb->GetDbEntryPassw(pEntry2, sPassw2);
        Compare = wcscmp(sPassw1.c_str(), sPassw2.c_str()) * m_nSortOrderFactor;
      }
      break;
    case PasswDbEntry::CREATIONTIME:
      ULARGE_INTEGER t1, t2;
      t1.LowPart = pEntry1->CreationTime.dwLowDateTime;
      t1.HighPart = pEntry1->CreationTime.dwHighDateTime;
      t2.LowPart = pEntry2->CreationTime.dwLowDateTime;
      t2.HighPart = pEntry2->CreationTime.dwHighDateTime;
      Compare = static_cast<int>(t1.QuadPart - t2.QuadPart) * m_nSortOrderFactor;
      break;
    case PasswDbEntry::MODIFICATIONTIME:
      //ULARGE_INTEGER t1, t2;
      t1.LowPart = pEntry1->ModificationTime.dwLowDateTime;
      t1.HighPart = pEntry1->ModificationTime.dwHighDateTime;
      t2.LowPart = pEntry2->ModificationTime.dwLowDateTime;
      t2.HighPart = pEntry2->ModificationTime.dwHighDateTime;
      Compare = static_cast<int>(t1.QuadPart - t2.QuadPart) * m_nSortOrderFactor;
      break;
    default:
      Compare = _wcsicmp(pEntry1->Strings[m_nSortByIdx].c_str(),
          pEntry2->Strings[m_nSortByIdx].c_str()) * m_nSortOrderFactor;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_SortOrderClick(TObject *Sender)
{
  NotifyUserAction();

  int nFactor = MainMenu_View_SortBy_Ascending->Checked ? 1 : -1;
  if (nFactor != m_nSortOrderFactor) {
    m_nSortOrderFactor = nFactor;
    if (m_nSortByIdx >= 0)
      DbView->AlphaSort();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TogglePasswBtnClick(TObject *Sender)
{
  NotifyUserAction();

  PasswBox->PasswordChar = TogglePasswBtn->Down ? PASSWORD_CHAR : '\0';
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_ChangeMasterPasswordClick(
  TObject *Sender)
{
  NotifyUserAction();

  SuspendIdleTimer;

  while (PasswEnterDlg->Execute(0,
      TRL("Enter OLD master password"), this) == mrOk)
  {
    SecureMem<word8> key;
    PasswEnterDlg->GetPassw(key);
    PasswEnterDlg->Clear();
    Screen->Cursor = crHourGlass;

    if (m_passwDb->CheckMasterKey(key)) {
      Screen->Cursor = crDefault;
      if (PasswEnterDlg->Execute(PASSWENTER_FLAG_CONFIRMPASSW,
          TRL("Enter NEW master password"), this) == mrOk) {
        Screen->Cursor = crHourGlass;
        PasswEnterDlg->GetPassw(key);
        m_passwDb->ChangeMasterKey(key);
        Screen->Cursor = crDefault;
        SetDbChanged();
        MsgBox(TRL("Master password successfully changed."), MB_ICONINFORMATION);
      }
      break;
    }
    else {
      Screen->Cursor = crDefault;
      MsgBox(TRL("Old master password is invalid."), MB_ICONERROR);
//      PasswEnterDlg->Clear();
//      PasswEnterDlg->ClearPasswCache();
    }
  }

  PasswEnterDlg->Clear();
  RandomPool::GetInstance()->Flush();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ChangeCaption(void)
{
  WString sCaption;
  if (IsDbOpen() || m_blLocked) {
    if (m_blDbChanged)
      sCaption = WString("*");
    sCaption += m_sDbFileName.IsEmpty() ? TRL("Untitled") :
      ExtractFileName(m_sDbFileName);
    if (m_blDbReadOnly)
      sCaption += " (R)";
    if (m_blLocked)
      sCaption += WString(" (") + TRL("Locked") + WString(")");
    sCaption += WString(" - ");
    //MainMenu_File_Save->Enabled = m_blDbChanged;
  }
  Caption = sCaption + PASSW_MANAGER_NAME;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SetDbChanged(bool blChanged, bool blEntryChanged)
{
  if (blChanged != m_blDbChanged) {
    m_blDbChanged = blChanged;
    ChangeCaption();
  }
  if (blChanged && g_config.Database.AutoSave && !m_sDbFileName.IsEmpty() &&
      (g_config.Database.AutoSaveOption == asdEveryChange ||
      (g_config.Database.AutoSaveOption == asdEntryModification && blEntryChanged)))
  {
    MainMenu_File_SaveClick(this);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_SaveClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_sDbFileName.IsEmpty() || m_blDbReadOnly)
    MainMenu_File_SaveAsClick(this);
  else if (SaveDatabase(m_sDbFileName))
    SetDbChanged(false);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TogglePasswBtnMouseUp(TObject *Sender,
  TMouseButton Button, TShiftState Shift, int X, int Y)
{
  NotifyUserAction();

  if (Button == mbRight) {
    bool blGenMain = true;
    if (m_pSelectedItem != NULL && m_pSelectedItem->Data != NULL) {
      const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(
        m_pSelectedItem->Data);
      const SecureWString* psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[
        DB_KEYVAL_PROFILE]);
      if (psVal != NULL) {
        if (!MainForm->LoadProfile(psVal->c_str())) {
          MsgBox(TRLFormat("Profile \"%s\" not found.", psVal->c_str()), MB_ICONERROR);
          return;
        }
      }
      else {
        psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[DB_KEYVAL_FORMATPASSW]);
        if (psVal != NULL) {
          w32string sFormat = WCharToW32String(psVal->c_str());
          SecureW32String sDest(16001);
          PasswordGenerator passwGen(g_pRandSrc);
          if (passwGen.GetFormatPassw(sDest, sDest.Size()-1, sFormat, 0) != 0)
          {
            W32CharToWCharInternal(sDest);
            SetEditBoxTextBuf(PasswBox, reinterpret_cast<wchar_t*>(sDest.Data()));
          }
          eraseStlString(sFormat);
          blGenMain = false;
        }
      }
    }
    if (blGenMain)
      MainForm->GeneratePassw(gpdGuiSingle, PasswBox);
    PasswBox->SetFocus();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormCloseQuery(TObject *Sender,
  bool &CanClose)
{
  NotifyUserAction();

  CanClose = CloseDatabase();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewMenuPopup(TObject *Sender)
{
  NotifyUserAction();

  bool blIsDbEntry = m_pSelectedItem != NULL && m_pSelectedItem->Data != NULL;
  DbViewMenu_CopyUserName->Enabled = blIsDbEntry;
  DbViewMenu_CopyPassw->Enabled = blIsDbEntry;
  DbViewMenu_OpenUrl->Enabled = blIsDbEntry;
  DbViewMenu_Run->Enabled = blIsDbEntry;
  DbViewMenu_PerformAutotype->Enabled = blIsDbEntry;

  bool blExtEnabled = blIsDbEntry || DbView->SelCount >= 2;
  DbViewMenu_DuplicateEntry->Enabled = blExtEnabled;
  DbViewMenu_DeleteEntry->Enabled = blExtEnabled;
  DbViewMenu_Rearrange->Enabled = blExtEnabled && m_nSortByIdx < 0 &&
    DbView->SelCount < m_passwDb->Size;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBoxKeyPress(TObject *Sender,
  char &Key)
{
  NotifyUserAction();

  SearchBox->Tag = 1;

  if (Key == VK_RETURN) {
    SearchBtnClick(this);
    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_CopyUserNameClick(
  TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Data == NULL)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  if (pEntry->Strings[PasswDbEntry::USERNAME].Size() > 1) {
    SetClipboardTextBuf(pEntry->Strings[PasswDbEntry::USERNAME].c_str(), NULL);
    MainForm->CopiedSensitiveDataToClipboard();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_CopyPasswClick(
  TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Data == NULL)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  SecureWString sPassw;
  m_passwDb->GetDbEntryPassw(pEntry, sPassw);
  if (sPassw.Size() > 1) {
    SetClipboardTextBuf(sPassw.c_str(), NULL);
    MainForm->CopiedSensitiveDataToClipboard();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_OpenUrlClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Data == NULL)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  if (pEntry->Strings[PasswDbEntry::URL].Size() > 1)
    ExecuteShellOp(WString(pEntry->Strings[PasswDbEntry::URL].c_str()), true);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_DuplicateEntryClick(
  TObject *Sender)
{
  NotifyUserAction();

  if (DbView->SelCount == 0)
    return;

  static const WString sCopyStr = WString(" - ") + TRL("Copy", "noun");

  for (int nI = 0; nI < DbView->Items->Count; nI++) {
    if (DbView->Items->Item[nI]->Selected) {
      const PasswDbEntry* pOriginal = reinterpret_cast<PasswDbEntry*>(
          DbView->Items->Item[nI]->Data);
      if (pOriginal != NULL) {
        PasswDbEntry* pDuplicate = m_passwDb->AddDbEntry(true, false);
        SecureWString sNewTitle = pOriginal->Strings[PasswDbEntry::TITLE];
        if (!sNewTitle.IsEmpty()) {
          sNewTitle.GrowBy(sCopyStr.Length());
          wcscat(sNewTitle.Data(), sCopyStr.c_str());
        }
        else
          sNewTitle.Assign(sCopyStr.c_str(), sCopyStr.Length() + 1);
        pDuplicate->Strings[PasswDbEntry::TITLE] = sNewTitle;
        pDuplicate->Strings[PasswDbEntry::USERNAME] =
          pOriginal->Strings[PasswDbEntry::USERNAME];
        pDuplicate->Strings[PasswDbEntry::URL] =
          pOriginal->Strings[PasswDbEntry::URL];
        pDuplicate->Strings[PasswDbEntry::KEYWORD] =
          pOriginal->Strings[PasswDbEntry::KEYWORD];
        pDuplicate->Strings[PasswDbEntry::NOTES] =
          pOriginal->Strings[PasswDbEntry::NOTES];

        SecureWString origPassw;
        m_passwDb->GetDbEntryPassw(pOriginal, origPassw);
        m_passwDb->SetDbEntryPassw(pDuplicate, origPassw);

        pDuplicate->SetKeyValueList(pOriginal->GetKeyValueList());
        pDuplicate->SetTagList(pOriginal->GetTagList());
        pDuplicate->PasswExpiryDate = pOriginal->PasswExpiryDate;
        pDuplicate->PasswExpiryDateString = pOriginal->PasswExpiryDateString;
      }
    }
  }

  SetDbChanged();
  ResetListView(RELOAD_TAGS);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_SelectAllClick(
  TObject *Sender)
{
  NotifyUserAction();

  DbView->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_DbSettingsClick(
  TObject *Sender)
{
  NotifyUserAction();

  SuspendIdleTimer;

  PasswDbSettings s;
  s.DefaultUserName = m_passwDb->DefaultUserName;
  s.PasswFormatSeq = m_passwDb->PasswFormatSeq;
  s.DefaultExpiryDays = m_passwDb->DefaultPasswExpiryDays;
  s.CipherType = m_passwDb->CipherType;
  s.NumKdfRounds = m_passwDb->KdfIterations;

  PasswDbSettingsDlg->SetSettings(s);
  if (PasswDbSettingsDlg->ShowModal() == mrOk &&
      (m_passwDb->DefaultUserName != s.DefaultUserName ||
      m_passwDb->PasswFormatSeq != s.PasswFormatSeq ||
      m_passwDb->DefaultPasswExpiryDays != s.DefaultExpiryDays ||
      m_passwDb->CipherType != s.CipherType ||
      m_passwDb->KdfIterations != s.NumKdfRounds))
    SetDbChanged();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ClearSearchBtnClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_nSearchMode != SEARCH_MODE_OFF || SearchBox->GetTextLen() != 0) {
    SearchBox->Text = WString();
    SearchBtnClick(this);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::LockOrUnlockDatabase(bool blAuto)
{
  if (m_blLocked) {
    OpenDatabase(true, true);
  }
  else if (CloseDatabase(false, blAuto ? 2 : 1)) {
    m_blLocked = true;
    m_blUnlockTried = false;
    MainMenu_File_Lock->Caption = TRL("Unlock");
    MainMenu_File_Lock->Enabled = true;
    MainMenu_File_Close->Enabled = true;
    LockBtn->Enabled = true;
    ChangeCaption();
    WindowState = wsMinimized;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_LockClick(TObject *Sender)
{
  NotifyUserAction();

  LockOrUnlockDatabase(false);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_ExitClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::IdleTimerTimer(TObject *Sender)
{
  if (IsDbOpen() && g_config.Database.LockIdle && !m_blItemChanged &&
     !m_blLocked && !IsDisplayDlg() &&
     (Screen->ActiveForm == NULL || !(Screen->ActiveForm->FormState.Contains(fsModal) ||
        (Screen->ActiveForm == ProgressForm && ProgressForm->IsRunning()))))
  {
    if (SecondsBetween(Now(), m_lastUserActionTime) >= g_config.Database.LockIdleTime)
      LockOrUnlockDatabase(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NotifyUserAction(void)
{
  //m_lLastUserActionTime = GetTickCount();
  m_lastUserActionTime = Now();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  SearchBox->Clear();
  if (g_config.Database.ClearClipExit)
    Clipboard()->Clear();
  TopMostManager::GetInstance()->OnFormClose(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance()->SetForm(this);
  if (!IsDbOpen() && g_config.Database.OpenLastDbOnStartup &&
      !m_sDbFileName.IsEmpty())
    Tag = FORM_TAG_OPEN_DB;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormActivate(TObject *Sender)
{
  static bool blStartup = true;
  if (blStartup) {
    // load window size to ensure proper scaling
    Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
    Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);

    // inform column selection dialog about field names
    PasswMngColDlg->SetColNames(m_uiFieldNames);

    // set font for password test box in password settings dialog
    //PasswDbSettingsDlg->PasswGenTestBox->Font = PasswBox->Font;

    // set up string grid in key-value dialog
    TStringGrid* pKeyValGrid = PasswMngKeyValDlg->KeyValueGrid;
    pKeyValGrid->RowCount = DB_NUM_KEYVAL_KEYS + 1;
    pKeyValGrid->Cells[0][0] = TRL("Key");
    pKeyValGrid->Cells[1][0] = TRL("Value");
    int nRow = 0;
    for (const auto& kv : m_keyValNames)
      pKeyValGrid->Cells[0][++nRow] = kv.second.c_str();
  }

  if (Tag == FORM_TAG_OPEN_DB)
    OpenDatabase(true, false, (!g_cmdLineOptions.PasswDbFileName.IsEmpty() &&
      blStartup) ? g_cmdLineOptions.PasswDbFileName : m_sDbFileName);
  else if (Tag == FORM_TAG_UNLOCK_DB || (m_blLocked && !m_blUnlockTried)) {
    OpenDatabase(true, true);
    m_blUnlockTried = true;
  }

  Tag = 0;
  blStartup = false;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBtnClick(TObject *Sender)
{
  NotifyUserAction();

  WString sText = SearchBox->Text;
  if (sText.IsEmpty() && m_nSearchMode != SEARCH_MODE_OFF) {
    m_nSearchMode = SEARCH_MODE_OFF;
    ResetListView(RELOAD_TAGS);
    DbView->Font->Color = clBlack;
    SearchResultPanel->Caption = WString();
  }
  else if (!sText.IsEmpty()) {
    //m_blSearchMode = true;
    SearchDatabase(SearchBox->Text, m_nSearchFlags, CaseSensitiveBtn->Down,
      FuzzyBtn->Down);
    int nPos = SearchBox->Items->IndexOf(SearchBox->Text);
    if (nPos < 0) {
      SearchBox->Items->Insert(0, sText);
    }
    else {
      SearchBox->Items->Strings[nPos] = sText;
      SearchBox->Items->Move(nPos, 0);
      SearchBox->ItemIndex = 0;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewColumnClick(TObject *Sender,
  TListColumn *Column)
{
  NotifyUserAction();

  static int nLastColumn = -1, nLastOrder = -1;
  int nColumn = Column->Tag;
  if (nColumn == nLastColumn)
    nLastOrder = (nLastOrder == 0) ? 1 : 0;
  else
    nLastOrder = 0;
  nLastColumn = nColumn;
  int nSortOrderFactor = (nLastOrder == 0) ? 1 : -1;
  if (m_nSortByIdx != nColumn || nSortOrderFactor != m_nSortOrderFactor) {
    MainMenu_View_SortBy->Items[nColumn+2]->Checked = true;
    MainMenu_View_SortBy->Items[MainMenu_View_SortBy->Count - 2 +
      nLastOrder]->Checked = true;
    m_nSortByIdx = nColumn;
    m_nSortOrderFactor = nSortOrderFactor;
    DbView->SortType = m_nSortByIdx >= 0 ? TSortType::stData : TSortType::stNone;
    DbView->AlphaSort();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBoxSelect(TObject *Sender)
{
  if (SearchBox->Tag == 0) {
    NotifyUserAction();
    SearchDatabase(SearchBox->Text, m_nSearchFlags, CaseSensitiveBtn->Down,
      FuzzyBtn->Down);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::WMSize(TWMSize& msg)
{
  if (!Visible) return;

  switch (msg.SizeType) {
  case SIZE_RESTORED:
    if (m_blLocked)
      Tag = FORM_TAG_UNLOCK_DB;
    break;

  case SIZE_MINIMIZED:
    if (IsDbOpen()) {
      if (g_config.Database.LockMinimize)
        MainMenu_File_LockClick(this);

      if (g_config.Database.ClearClipMinimize)
        Clipboard()->Clear();
    }
    m_blUnlockTried = false;
  }

  TForm::Dispatch(&msg);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_Export_CsvFileClick(TObject *Sender)

{
  SuspendIdleTimer;

  SaveDlg->FilterIndex = 2;
  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();
  if (!blSuccess)
    return;

  static int nColMask = INT_MAX;
  int nInputMask = PasswMngColDlg->Execute(this, nColMask, false);
  if (nInputMask < 0)
    return;

  nColMask = nInputMask;

  try {
    m_passwDb->ExportToCsv(SaveDlg->FileName, nColMask, m_uiFieldNames);
    MsgBox(TRL("Database successfully exported to CSV file."), MB_ICONINFORMATION);
  }
  catch (Exception& e) {
    MsgBox(TRLFormat("Error while creating CSV file:\n%s.", e.Message.c_str()),
      MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_EditClick(TObject *Sender)
{
  if (!IsDbOpen())
    return;

  bool blIsDbEntry = m_pSelectedItem != NULL && m_pSelectedItem->Data != NULL;
  MainMenu_Edit_CopyUserName->Enabled = blIsDbEntry;
  MainMenu_Edit_CopyPassw->Enabled = blIsDbEntry;
  MainMenu_Edit_OpenUrl->Enabled = blIsDbEntry;
  MainMenu_Edit_Run->Enabled = blIsDbEntry;
  MainMenu_Edit_PerformAutotype->Enabled = blIsDbEntry;

  bool blExtEnabled = blIsDbEntry || DbView->SelCount >= 2;
  MainMenu_Edit_DuplicateEntry->Enabled = blExtEnabled;
  MainMenu_Edit_DeleteEntry->Enabled = blExtEnabled;
  MainMenu_Edit_Rearrange->Enabled = blExtEnabled && m_nSortByIdx < 0 &&
    DbView->SelCount < m_passwDb->Size;
  AddEntryBtn->Enabled = m_nSearchMode == SEARCH_MODE_OFF;
  MainMenu_Edit_AddEntry->Enabled = m_nSearchMode == SEARCH_MODE_OFF;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::UserNameLblMouseMove(TObject *Sender,
  TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft)) {
    NotifyUserAction();
    StartEditBoxDragDrop(UserNameBox);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PasswLblMouseMove(TObject *Sender, TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft))
    StartEditBoxDragDrop(PasswBox);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TitleLblMouseMove(TObject *Sender, TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft)) {
    NotifyUserAction();
    StartEditBoxDragDrop(TitleBox);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::UrlLblMouseMove(TObject *Sender, TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft)) {
    NotifyUserAction();
    StartEditBoxDragDrop(UrlBox);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NotesLblMouseMove(TObject *Sender, TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft)) {
    NotifyUserAction();
    StartEditBoxDragDrop(NotesBox);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::KeywordLblMouseMove(TObject *Sender,
  TShiftState Shift,
  int X, int Y)
{
  if (Shift.Contains(ssLeft)) {
    NotifyUserAction();
    StartEditBoxDragDrop(KeywordBox);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ChangeListFontClick(TObject *Sender)

{
  SuspendIdleTimer;
  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  FontDlg->Font = DbView->Font;
  if (FontDlg->Execute())
    DbView->Font = FontDlg->Font;
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_PerformAutotypeClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Data == NULL)
    return;

  const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);

  if (g_config.MinimizeAutotype) {
    g_nAppState |= APPSTATE_AUTOTYPE;
    Application->Minimize();
    SendKeys sk(g_config.AutotypeDelay);
    sk.SendComplexString(getDbEntryAutotypeSeq(pEntry), pEntry, m_passwDb.get());
    g_nAppState &= ~APPSTATE_AUTOTYPE;
  }
  else {
    TSendKeysThread::TerminateAndWait();
    new TSendKeysThread(Handle, g_config.AutotypeDelay,
      getDbEntryAutotypeSeq(pEntry), pEntry, m_passwDb.get());
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewDragOver(TObject *Sender, TObject *Source, int X,
          int Y, TDragState State, bool &Accept)
{
  Accept = Sender == DbView;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewDragDrop(TObject *Sender, TObject *Source, int X,
          int Y)
{
  if (Sender == Source) {
    TListItem* pDropItem = DbView->GetItemAt(X, Y);
    if (pDropItem != NULL && pDropItem != m_pSelectedItem) {
      word32 lNewPos = pDropItem->Data ?
        reinterpret_cast<PasswDbEntry*>(pDropItem->Data)->GetIndex() :
        m_passwDb->Size - 1;
      m_passwDb->MoveDbEntry(
        reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data)->GetIndex(),
        lNewPos);
      ResetListView();
      SetDbChanged();
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
  // only allow drag&drop if list is unsorted, not in search mode, and
  // selected item is a real entry
  if (Shift.Contains(ssLeft) &&
      m_nSortByIdx < 0 &&
      //DbView->Items->Count > m_passwDb->GetSize() &&
      DbView->SelCount == 1 &&
      m_pSelectedItem != NULL && m_pSelectedItem->Data != NULL)
  {
    NotifyUserAction();
    DbView->BeginDrag(false);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBtnMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
  NotifyUserAction();

  if (Button == mbRight) {
    int nNewFlags = PasswMngColDlg->Execute(this, m_nSearchFlags, true);
    if (nNewFlags >= 0)
      m_nSearchFlags = nNewFlags;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_RunClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == NULL || m_pSelectedItem->Data == NULL)
    return;

  const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  const SecureWString* pVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[DB_KEYVAL_RUN]);
  if (pVal != NULL) {
    if (!ExecuteCommand(pVal->c_str(), false))
      ExecuteShellOp(pVal->c_str(), true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_AddEntryClick(TObject *Sender)
{
  NotifyUserAction();

  if (DbView->Items->Count > 0 &&
      DbView->Items->Item[DbView->Items->Count - 1]->Data == NULL &&
      !m_blItemChanged)
  {
    DbView->ClearSelection();
    DbView->Items->Item[DbView->Items->Count - 1]->Selected = true;
    ScrollToItem(DbView->Items->Count - 1);
    TitleBox->SetFocus();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBoxEnter(TObject *Sender)
{
  NotifyUserAction();
  SearchBox->Tag = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_Rearrange_TopClick(TObject *Sender)
{
  NotifyUserAction();
  MoveDbEntries(MOVE_TOP);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_Rearrange_UpClick(TObject *Sender)
{
  NotifyUserAction();
  MoveDbEntries(MOVE_UP);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_Rearrange_DownClick(TObject *Sender)

{
  NotifyUserAction();
  MoveDbEntries(MOVE_DOWN);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_Rearrange_BottomClick(TObject *Sender)
{
  NotifyUserAction();
  MoveDbEntries(MOVE_BOTTOM);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ChangePasswFontClick(TObject *Sender)
{
  SuspendIdleTimer;
  BeforeDisplayDlg();
  TopMostManager::GetInstance()->NormalizeTopMosts(this);
  FontDlg->Font = PasswBox->Font;
  if (FontDlg->Execute()) {
    PasswBox->Font = FontDlg->Font;
    //PasswDbSettingsDlg->PasswGenTestBox->Font = FontDlg->Font;
  }
  TopMostManager::GetInstance()->RestoreTopMosts(this);
  AfterDisplayDlg();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TagViewSelectItem(TObject *Sender, TListItem *Item,
          bool Selected)
{
  if (TagView->Tag)
    return;
  std::set<SecureWString> tagFilter;
  if (TagView->SelCount > 0) {
    for (int i = 0; i < TagView->Items->Count; i++) {
      const auto pItem = TagView->Items->Item[i];
      if (pItem->Selected) {
        if (pItem->Data == nullptr)
          break;
        const auto pData = reinterpret_cast<std::pair<SecureWString, word32>*>(
          pItem->Data);
        tagFilter.insert(pData->first);
      }
    }
  }
  if (tagFilter != m_tagFilter) {
    m_tagFilter = tagFilter;
    ResetListView();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TagViewCompare(TObject *Sender, TListItem *Item1, TListItem *Item2,
          int Data, int &Compare)
{
  if (Item1->Data == nullptr)
    Compare = -1;
  else if (Item2->Data == nullptr)
    Compare = 1;
  else {
    const auto pData1 = reinterpret_cast<std::pair<SecureWString, word32>*>(
      Item1->Data);
    const auto pData2 = reinterpret_cast<std::pair<SecureWString, word32>*>(
      Item2->Data);
    if (m_nTagsSortByIdx == 0) {
      if (pData1->first.IsEmpty())
        Compare = 1;
      else if (pData2->first.IsEmpty())
        Compare = -1;
      else
        Compare = _wcsicmp(pData1->first.c_str(), pData2->first.c_str()) *
          m_nTagsSortOrderFactor;
    }
    else {
      Compare = (pData1->second - pData2->second) * m_nTagsSortOrderFactor;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::AddTagBtnClick(TObject *Sender)
{
  NotifyUserAction();
  if (m_pSelectedItem == nullptr)
    return;
  if (TagMenu->Items->Count > 0) {
    POINT pt;
    GetCursorPos(&pt);
    TagMenu->Popup(pt.x, pt.y);
  }
  else
    TagsBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::OnTagMenuItemClick(TObject* Sender)
{
  NotifyUserAction();
  TMenuItem* pItem = dynamic_cast<TMenuItem*>(Sender);
  if (pItem && pItem->MenuIndex < m_tags.size()) {
    SecureWString sTags;
    //WString sNewTag = pItem->Caption;
    const SecureWString& sNewTag =
      std::next(m_tags.begin(), pItem->MenuIndex)->first;
    int nLen = GetEditBoxTextLen(TagsBox);
    if (nLen > 0) {
      GetEditBoxTextBuf(TagsBox, sTags);
      while (nLen > 0 && sTags[nLen - 1] == ' ')
        nLen--;
      if (nLen > 0 && sTags[nLen - 1] != ',' && sTags[nLen - 1] != ';') {
        sTags.Grow(nLen + sNewTag.StrLen() + 3);
        wcscpy(&sTags[nLen], L", ");
        nLen += 2;
      }
      else
        sTags.Grow(nLen + sNewTag.StrLen() + 1);
      wcscpy(&sTags[nLen], sNewTag.c_str());
    }
    else
      sTags = sNewTag;
      //sTags.Assign(sNewTag.c_str(), sNewTag.Length() + 1);
    SetEditBoxTextBuf(TagsBox, sTags.c_str());
    //eraseVclString(sNewTag);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_SortTagsByClick(TObject *Sender)
{
  NotifyUserAction();
  int nIdx = reinterpret_cast<TMenuItem*>(Sender)->Tag;
  if (nIdx != m_nTagsSortByIdx) {
    m_nTagsSortByIdx = nIdx;
    TagView->AlphaSort();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_SortTagsOrderClick(TObject *Sender)
{
  NotifyUserAction();
  int nFactor = (Sender == MainMenu_View_SortTagsBy_Ascending) ? 1 : -1;
  if (nFactor != m_nTagsSortOrderFactor) {
    m_nTagsSortOrderFactor = nFactor;
    TagView->AlphaSort();
  }
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::ApplyDbSettings(const PasswDbSettings& settings)
{
  if (settings.NumKdfRounds != m_passwDb->KdfIterations) {
    bool blSuccess = false;
    while (PasswEnterDlg->Execute(0,
           TRL("Enter master password again"), this) == mrOk)
    {
      SecureMem<word8> key;
      PasswEnterDlg->GetPassw(key);
      PasswEnterDlg->Clear();
      Screen->Cursor = crHourGlass;

      if (m_passwDb->CheckMasterKey(key)) {
        m_passwDb->KdfIterations = settings.NumKdfRounds;
        m_passwDb->ChangeMasterKey(key);
        Screen->Cursor = crDefault;
        blSuccess = true;
        break;
      }
      else {
        Screen->Cursor = crDefault;
        MsgBox(TRL("Master password is invalid."), MB_ICONERROR);
      }
    }
    if (!blSuccess) {
      PasswEnterDlg->Clear();
      return false;
    }
  }

  m_passwDb->DefaultUserName = settings.DefaultUserName;
  m_passwDb->PasswFormatSeq = settings.PasswFormatSeq;
  m_passwDb->DefaultPasswExpiryDays = settings.DefaultExpiryDays;
  m_passwDb->CipherType = settings.CipherType;
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::EditKeyValBtnClick(TObject *Sender)
{
  if (!m_passwDb->IsOpen() || m_pSelectedItem == nullptr)
    return;

  NotifyUserAction();
  SuspendIdleTimer;

  if (!m_tempKeyVal) {
    if (m_pSelectedItem->Data != nullptr)
      m_tempKeyVal.reset(new PasswDbEntry::KeyValueList(
        reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data)->GetKeyValueList()));
    else
      m_tempKeyVal.reset(new PasswDbEntry::KeyValueList());
  }

  for (const auto& kv : *m_tempKeyVal.get()) {
    auto nameIt = m_keyValNames.find(kv.first.c_str());
    if (nameIt != m_keyValNames.end()) {
      int nRow = 1 + std::distance(m_keyValNames.begin(), nameIt);
      PasswMngKeyValDlg->KeyValueGrid->Cells[1][nRow] = kv.second.c_str();
    }
  }

  if (PasswMngKeyValDlg->ShowModal() == mrOk) {
    m_tempKeyVal->clear();
    for (int i = 0; i < DB_NUM_KEYVAL_KEYS; i++) {
      WString sVal = PasswMngKeyValDlg->KeyValueGrid->Cells[1][i+1];
      if (!sVal.IsEmpty()) {
        auto it = std::next(m_keyValNames.begin(), i);
        int nIdx;
        for (nIdx = 0; nIdx < DB_NUM_KEYVAL_KEYS; nIdx++) {
          if (wcscmp(DB_KEYVAL_KEYS[nIdx], it->first.c_str()) == 0)
            break;
        }
        if (nIdx < DB_NUM_KEYVAL_KEYS)
          m_tempKeyVal->push_back({
            SecureWString(DB_KEYVAL_KEYS[nIdx], wcslen(DB_KEYVAL_KEYS[nIdx]) + 1),
            SecureWString(sVal.c_str(), sVal.Length() + 1)});
      }
      eraseVclString(sVal);
    }
    SetEditBoxTextBuf(KeyValueListBox,
      BuildTranslKeyValString(*m_tempKeyVal.get()).c_str());
  }

  PasswMngKeyValDlg->Clear();
}
//---------------------------------------------------------------------------
SecureWString __fastcall TPasswMngForm::BuildTranslKeyValString(
  const PasswDbEntry::KeyValueList& keyValList)
{
  if (keyValList.empty())
    return SecureWString();
  SecureWString sResult(256);
  int nPos = 0;
  for (const auto& p : keyValList) {
    auto it = m_keyValNames.find(std::wstring(p.first.c_str()));
    if (it != m_keyValNames.end()) {
      sResult.Grow(nPos + it->second.length() + p.second.StrLen() + 3);
      wcscpy(&sResult[nPos], it->second.c_str());
      nPos += it->second.length();
      sResult[nPos++] = '=';
      wcscpy(&sResult[nPos], p.second.c_str());
      nPos += p.second.StrLen();
      sResult[nPos++] = ',';
      sResult[nPos++] = ' ';
    }
  }
  if (nPos > 3)
    sResult[nPos - 2] = '\0';
  return sResult;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ExpiryCheckClick(TObject *Sender)
{
  if (ExpiryCheck->Tag == 0) {
    if (m_pSelectedItem == nullptr)
      return;

    NotifyUserAction();
    SetItemChanged(true);
  }

  bool blEnabled = ExpiryCheck->Checked;
  ExpiryDatePicker->Enabled = blEnabled;

  ExpiryCheck->Tag = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ExpiryDatePickerChange(TObject *Sender)
{
  NotifyUserAction();
  SetItemChanged(true);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ExpiryBtnClick(TObject *Sender)
{
  NotifyUserAction();
  if (m_pSelectedItem == nullptr || !ExpiryCheck->Checked)
    return;
  POINT pt;
  GetCursorPos(&pt);
  ExpiryMenu->Popup(pt.x, pt.y);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::OnExpiryMenuItemClick(TObject* Sender)
{
  NotifyUserAction();
  auto pItem = dynamic_cast<TMenuItem*>(Sender);
  if (pItem) {
    auto date = TDateTime::CurrentDate() + static_cast<double>(pItem->Tag);
    if (date != ExpiryDatePicker->Date) {
      ExpiryDatePicker->Date = date;
      SetItemChanged(true);
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ExpiredEntriesClick(TObject *Sender)
{
  ResetListView(0);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
  switch (Key) {
  case VK_UP:
    PrevBtnClick(this);
    break;
  case VK_DOWN:
    NextBtnClick(this);
    break;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_PropertiesClick(TObject *Sender)
{
  NotifyUserAction();
  try {
    if (!m_sDbFileName.IsEmpty()) {
      PasswMngDbPropDlg->SetProperty(DbProperty::FileName, ExtractFileName(
        m_sDbFileName));

      WString sPath = ExtractFilePath(m_sDbFileName);
      if (!sPath.IsEmpty() && sPath[sPath.Length()] == '\\')
        sPath.Delete(sPath.Length(), 1);

      PasswMngDbPropDlg->SetProperty(DbProperty::FilePath, sPath);

      WIN32_FILE_ATTRIBUTE_DATA fad;
      if (!GetFileAttributesEx(m_sDbFileName.c_str(), GetFileExInfoStandard, &fad))
        RaiseLastOSError();

      PasswMngDbPropDlg->SetProperty(DbProperty::FileSize,
        Format("%.0n %s", ARRAYOFCONST((static_cast<long double>(fad.nFileSizeLow),
        TRL("bytes")))));

      PasswMngDbPropDlg->SetProperty(DbProperty::CreationTime,
        FileTimeToString(fad.ftCreationTime, true));
      PasswMngDbPropDlg->SetProperty(DbProperty::ModificationTime,
        FileTimeToString(fad.ftLastWriteTime, true));
    }

    PasswMngDbPropDlg->SetProperty(DbProperty::FormatVersion,
      Format("%d.%d", ARRAYOFCONST((m_passwDb->Version>>8,
      m_passwDb->Version&0xff))));

    PasswMngDbPropDlg->SetProperty(DbProperty::NumEntries, IntToStr(
      static_cast<int>(m_passwDb->Size)));

    int nExpiredEntries = 0;
    for (auto pEntry : m_passwDb->GetDatabase()) {
      if (pEntry->UserFlags & DB_FLAG_EXPIRED)
        nExpiredEntries++;
    }

    PasswMngDbPropDlg->SetProperty(DbProperty::NumExpiredEntries, IntToStr(
      nExpiredEntries));

    PasswMngDbPropDlg->SetProperty(DbProperty::NumTags, IntToStr(
      static_cast<int>(m_globalCaseiTags.size())));
  }
  catch (Exception& e)
  {
    MsgBox(e.Message, MB_ICONERROR);
  }

  PasswMngDbPropDlg->ShowModal();
}
//---------------------------------------------------------------------------

