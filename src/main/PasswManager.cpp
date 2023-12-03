// PasswManager.cpp
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
#include <vector>
#include <map>
#include <algorithm>
#include <Math.hpp>
#include <IOUtils.hpp>
#include <DateUtils.hpp>
#include <Clipbrd.hpp>
#include <StrUtils.hpp>
#include <System.Threading.hpp>
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
#include "zxcvbn.h"
#include "PasswMngPwHistory.h"
#include "Progress.h"
#include "SecureClipboard.h"
#include "TaskCancel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPasswMngForm *PasswMngForm;


const char* UI_FIELD_NAMES[PasswDbEntry::NUM_FIELDS] =
{
  "Title", "User name", "Password", "URL", "Keyword", "Notes", "Key-value list",
  "Tags", "Creation time", "Last modification", "Password changed",
  "Password expiry", "Password history"
};

const WString
  CONFIG_ID          = "PasswManager",
  PASSW_MANAGER_NAME = "PassCube",
  PWDB_FILE_EXT      = ".pwdb";

const char
  PASSWORD_CHAR = '*';

const wchar_t
  HIDE_PASSW_STR[] = L"********";

const int
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
  RELOAD_TAGS = 2,

  DB_OPEN_FLAG_EXISTING = 1,
  DB_OPEN_FLAG_UNLOCK   = 2,
  DB_OPEN_FLAG_READONLY = 4,

  DB_LOCK_MANUAL  = 1,
  DB_LOCK_AUTO    = 2,

  TAGVIEW_IMAGE_INDEX_START = 6,
  DBVIEW_IMAGE_INDEX_START  = 9,

  FORM_TAG_OPEN_DB        = 1,
  FORM_TAG_UNLOCK_DB      = 2,
  FORM_TAG_ITEM_SELECTED  = 3,

  PASSWBOX_TAG_PASSW_GEN  = 1,

  WEAK_PASSW_THRESHOLD = 75;

const int
  STD_EXPIRY_DAYS[] = { 7, 14, 30, 90, 180, 365 };

const word32
  DB_FLAG_FOUND        = 1,
  DB_FLAG_EXPIRED      = 2,
  DB_FLAG_EXPIRES_SOON = 4;

const TColor DBVIEW_SEARCH_COLOR = clBlue;

const wchar_t* DB_KEYVAL_KEYS[DB_NUM_KEYVAL_KEYS] =
{
  L"Autotype", L"Run", L"Profile", L"FormatPW"
};

const wchar_t* DB_KEYVAL_UI_KEYS[DB_NUM_KEYVAL_KEYS] =
{
  L"Autotype", L"Run", L"Profile", L"Format password"
};

int s_nNumIdleTimerSuspendInst = 0;

class IdleTimerSuspender
{
public:
  IdleTimerSuspender()
  {
    if (++s_nNumIdleTimerSuspendInst == 1)
      PasswMngForm->IdleTimer->Enabled = false;
  }

  ~IdleTimerSuspender()
  {
    if (--s_nNumIdleTimerSuspendInst == 0) {
      PasswMngForm->NotifyUserAction();
      PasswMngForm->IdleTimer->Enabled = true;
    }
  }
};

#define SuspendIdleTimer IdleTimerSuspender _its

const wchar_t* getDbEntryAutotypeSeq(const PasswDbEntry* pEntry)
{
  const SecureWString* psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[
    DB_KEYVAL_AUTOTYPE]);
  return (psVal != nullptr) ? psVal->c_str() :
    g_config.Database.DefaultAutotypeSequence.c_str();
}

word32 levenshteinDist(const wchar_t* pSrc, word32 lSrcLen,
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

float strFindFuzzy(const wchar_t* pSrc, const wchar_t* pPattern)
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
    while ((pNext = wcschr(pNext, pPattern[0])) != nullptr) {
      if (i == 0 && wcsstr(pNext, pPattern) != nullptr) {
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

void getExpiryCheckDates(word32& lCurrDate, word32& lExpirySoonDate)
{
  TDate today = TDateTime::CurrentDate();
  unsigned short wYear, wMonth, wDay;
  today.DecodeDate(&wYear, &wMonth, &wDay);
  lCurrDate = PasswDbEntry::EncodeExpiryDate(wYear, wMonth, wDay);
  TDate expirySoonDate = today + std::max(1,
    std::min(100, g_config.Database.WarnExpireNumDays));
  expirySoonDate.DecodeDate(&wYear, &wMonth, &wDay);
  lExpirySoonDate = PasswDbEntry::EncodeExpiryDate(wYear, wMonth, wDay);
}

WString getTimeStampString(bool blMillisec = false)
{
  SYSTEMTIME st;
  GetLocalTime(&st);
  WString sResult = Format("%d%.2d%.2d%.2d%.2d%.2d",
    ARRAYOFCONST((st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond)));
  if (blMillisec)
    sResult += Format("_%.3d", ARRAYOFCONST((st.wMilliseconds)));
  return sResult;
}

__fastcall TSelectItemThread::TSelectItemThread(std::function<void(void)> func)
    : TThread(false), m_nTriggers(0), m_applyFunc(func)
{
  m_pEvent = new TSimpleEvent(nullptr, true, false, "", false);
}

void __fastcall TSelectItemThread::TerminateAndWait()
{
  Terminate();
  m_pEvent->SetEvent();
  WaitFor();
}

void __fastcall TSelectItemThread::Execute(void)
{
  while (!Terminated) {
    if (m_nTriggers > 0) {
      if (m_pEvent->WaitFor(50) == wrTimeout) {
        if (!Terminated && m_nTriggers > 0)
          TThread::Synchronize(m_applyFunc);
        m_nTriggers = 0;
      }
      m_pEvent->ResetEvent();
    }
    else
      TThread::Sleep(10);
  }
}

void __fastcall TSelectItemThread::ApplyNow(void)
{
  if (m_nTriggers > 0) {
    m_nTriggers = 0;
    m_pEvent->SetEvent();
  }
  m_applyFunc();
}

//---------------------------------------------------------------------------
__fastcall TPasswMngForm::TPasswMngForm(TComponent* Owner)
  : TForm(Owner), m_pSelectedItem(nullptr), m_nSortByIdx(-1),
    m_nSortOrderFactor(1), m_nTagsSortByIdx(0), m_nTagsSortOrderFactor(1),
    m_nSearchFlags(INT_MAX), m_nPasswEntropyBits(0)
{
  SetFormComponentsAnchors(this);

  ChangeCaption();

  AddModifyBtn->Left = PrevBtn->Left;
  CancelBtn->Left = NextBtn->Left;
  //PasswHistoryBtn->Left = PasswChangeInfo->Left + 4;

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
    pItem->Caption = TRLFormat("%1 days", { IntToStr(t) });
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

  FilterInfoPanel->Caption = TRL("Filtered");
  FilterInfoPanel->Font->Color = clBlue;

  if (g_pLangSupp) {
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
    TRLMenu(SearchMenu);

    TRLCaption(TitleLbl);
    TRLCaption(UserNameLbl);
    TRLCaption(PasswLbl);
    TRLCaption(UrlLbl);
    TRLCaption(KeywordLbl);
    TRLCaption(KeyValueListLbl);
    TRLCaption(NotesLbl);
    TRLCaption(CreationTimeLbl);
    TRLCaption(LastModificationLbl);
    TRLCaption(PasswChangeLbl);
    TRLCaption(TagsLbl);
    TRLCaption(ExpiryCheck);
    TRLCaption(PasswSecurityBarPanel);
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
    TRLHint(TogglePasswBtn);
    TRLHint(AddTagBtn);
    TRLHint(EditKeyValBtn);
    TRLHint(ExpiryBtn);
    TRLHint(PasswQualityBtn);
    TRLHint(PasswSecurityBarPanel);
    TRLHint(UrlBtn);
    TRLHint(PasswHistoryBtn);
    TRLHint(ClearFilterBtn);
    SearchBox->TextHint = TRL(SearchBox->TextHint);
  }

  OpenDlg->Filter = FormatW("%1 (*.pwdb)|*.pwdb|%2 (*.*)|*.*",
      { TRL("Password databases"),
        TRL("All files") });
  SaveDlg->Filter = FormatW("%1 (*.pwdb)|*.pwdb|%2 (*.csv)|*.csv|%3 (*.*)|*.*",
      { TRL("Password databases"),
        TRL("CSV files"),
        TRL("All files") });

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
  m_defaultListColor = DbView->Font->Color;

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
    auto splitList = SplitString(asColWidths, ";");
    for (const auto& sLen : splitList) {
      m_listColWidths.push_back(std::max(10, StrToIntDef(sLen, 0)));
      if (m_listColWidths.size() == PasswDbEntry::NUM_FIELDS)
        break;
    }
    /*int nLen = asColWidths.Length();
    int nStartIdx = 1;
    for (int nI = 1; nI <= nLen &&
      m_listColWidths.size() < PasswDbEntry::NUM_FIELDS; nI++)
    {
      if (asColWidths[nI] == ';' && nI > nStartIdx) {
        int nWidth = StrToIntDef(asColWidths.SubString(nStartIdx, nI - nStartIdx), 0);
        m_listColWidths.push_back(std::max(10, nWidth));
        nStartIdx = nI + 1;
      }
    }*/
  }

  //TogglePasswBtn->Down = g_pIni->ReadBool(CONFIG_ID, "HidePassw", true);
  //TogglePasswBtnClick(this);

  PasswQualityBtn->Down = g_pIni->ReadBool(CONFIG_ID, "EstimatePasswQuality", true);
  PasswQualityBtnClick(this);

  SearchMenu_CaseSensitive->Checked =
    g_pIni->ReadBool(CONFIG_ID, "FindCaseSensitive", false);
  SearchMenu_FuzzySearch->Checked = g_pIni->ReadBool(CONFIG_ID, "FindFuzzy", false);

  //g_config.Database.ClearClipMinimize = g_pIni->ReadBool(CONFIG_ID,
  //    "ClearClipMinimize", true);
  g_config.Database.ClearClipCloseLock = g_pIni->ReadBool(CONFIG_ID,
    "ClearClipCloseLock", true);
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
  g_config.Database.WarnEntriesExpireSoon = g_pIni->ReadBool(CONFIG_ID,
      "WarnEntriesExpireSoon", false);
  g_config.Database.WarnExpireNumDays = g_pIni->ReadInteger(CONFIG_ID,
      "WarnExpireNumDays", 7);
  g_config.Database.AutoSave = g_pIni->ReadBool(CONFIG_ID, "AutoSave", false);
  int nOption = g_pIni->ReadInteger(CONFIG_ID, "AutoSaveOption", 0);
  if (nOption >= asdEntryModification && nOption <= asdEveryChange)
    g_config.Database.AutoSaveOption = static_cast<AutoSaveDatabase>(nOption);
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
      DbView->Font->Style != Font->Style || DbView->Font->Color != Font->Color)
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

  //g_pIni->WriteBool(CONFIG_ID, "HidePassw", TogglePasswBtn->Down);
  g_pIni->WriteBool(CONFIG_ID, "EstimatePasswQuality", PasswQualityBtn->Down);
  g_pIni->WriteBool(CONFIG_ID, "FindCaseSensitive", SearchMenu_CaseSensitive->Checked);
  g_pIni->WriteBool(CONFIG_ID, "FindFuzzy", SearchMenu_FuzzySearch->Checked);

  g_pIni->WriteInteger(CONFIG_ID, "SortByIdx", m_nSortByIdx);
  g_pIni->WriteInteger(CONFIG_ID, "SortOrder", m_nSortOrderFactor);
  g_pIni->WriteInteger(CONFIG_ID, "TagsSortByIdx", m_nTagsSortByIdx);
  g_pIni->WriteInteger(CONFIG_ID, "TagsSortOrder", m_nTagsSortOrderFactor);
  //g_pIni->WriteBool(CONFIG_ID, "ClearClipMinimize",
  //  g_config.Database.ClearClipMinimize);
  g_pIni->WriteBool(CONFIG_ID, "ClearClipCloseLock",
    g_config.Database.ClearClipCloseLock);
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
  g_pIni->WriteBool(CONFIG_ID, "WarnEntriesExpireSoon",
    g_config.Database.WarnEntriesExpireSoon);
  g_pIni->WriteInteger(CONFIG_ID, "WarnExpireNumDays",
    g_config.Database.WarnExpireNumDays);
  g_pIni->WriteBool(CONFIG_ID, "AutoSave", g_config.Database.AutoSave);
  g_pIni->WriteInteger(CONFIG_ID, "AutoSaveOption", g_config.Database.AutoSaveOption);
  g_pIni->WriteString(CONFIG_ID, "DefaultAutotypeSeq",
    g_config.Database.DefaultAutotypeSequence);
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::OpenDatabase(int nOpenFlags,
  WString sFileName)
{
  SuspendIdleTimer;

  if (!(nOpenFlags & DB_OPEN_FLAG_UNLOCK) && IsDbOpen() &&
      AskSaveChanges() != ASK_SAVE_OK)
    return false;

  int nPasswEnterFlags = PASSWENTER_FLAG_ENABLEKEYFILE;
  WString sCaption;

  if (nOpenFlags & DB_OPEN_FLAG_EXISTING) {
    if (nOpenFlags & DB_OPEN_FLAG_UNLOCK) {
      sFileName = m_sDbFileName;
      sCaption = TRLFormat("Unlock %1", { ExtractFileName(sFileName) });
    }
    else {
      if (sFileName.IsEmpty()) {
        BeforeDisplayDlg();
        TopMostManager::GetInstance().NormalizeTopMosts(this);
        bool blSuccess = OpenDlg->Execute();
        TopMostManager::GetInstance().RestoreTopMosts(this);
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
        MsgBox(TRLFormat("Could not find database file:\n\"%1\".",
          { sFileName.c_str() }), MB_ICONERROR);
        return false;
      }
      sCaption = TRLFormat("Open %1", { ExtractFileName(sFileName) });
    }
    nPasswEnterFlags |= PASSWENTER_FLAG_DECRYPT;
  }
  else {
    sCaption = TRL("Create new database");
    nPasswEnterFlags |= PASSWENTER_FLAG_ENCRYPT | PASSWENTER_FLAG_CONFIRMPASSW |
      PASSWENTER_FLAG_ENABLEKEYFILECREATION;
  }

  auto passwDb = std::make_unique<PasswDatabase>();

  while (true) {
    try {
      if (PasswEnterDlg->Execute(
            nPasswEnterFlags,
            sCaption,
            this,
            !((nOpenFlags & DB_OPEN_FLAG_UNLOCK) && !m_blUnlockTried))
            != mrOk) {
        PasswEnterDlg->Clear();
        RandomPool::GetInstance().Flush();
        return false;
      }

      // combine password and key file to obtain database key
      SecureMem<word8> key = PasswDatabase::CombineKeySources(
        PasswEnterDlg->GetPasswBinary(), PasswEnterDlg->GetKeyFileName());

      // now clear password box
      PasswEnterDlg->Clear();
      RandomPool::GetInstance().Flush();

      if (nOpenFlags & DB_OPEN_FLAG_EXISTING) {
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
    catch (EPasswDbInvalidFormat& e) {
      Screen->Cursor = crDefault;
      MsgBox(TRLFormat("Database format error:\n%1.\nDatabase version: %2.%3\n"
        "Highest supported version: %4.%5",
        { e.Message,
          IntToStr(passwDb->Version>>8),
          IntToStr(passwDb->Version&0xff),
          IntToStr(PasswDatabase::VERSION_HIGH),
          IntToStr(PasswDatabase::VERSION_LOW) }),
        MB_ICONERROR);
      return false;
    }
    catch (EPasswDbError& e) {
      PasswEnterDlg->Clear();
      Screen->Cursor = crDefault;
      MsgBox(e.Message + ".", MB_ICONERROR);
    }
    catch (Exception& e) {
      Screen->Cursor = crDefault;
      MsgBox(TRLFormat("Error while opening database file:\n%1.", { e.Message }),
        MB_ICONERROR);
      return false;
    }
  }

  Screen->Cursor = crDefault;
  if (m_passwDb)
    CloseDatabase(true);
  //m_passwDb = std::move(passwDb);
  m_passwDb.reset(passwDb.release());
  m_pSelectedItem = nullptr;
  m_nSearchMode = SEARCH_MODE_OFF;
  m_blDbChanged = false;
  m_blItemChanged = false;
  m_blItemPasswChangeConfirm = false;
  m_blLocked = false;

  if (!(nOpenFlags & DB_OPEN_FLAG_UNLOCK)) {
    // ensure that entries will not be filtered by expiry status
    for (int i = 0; i < MainMenu_View_Filter->Count; i++) {
      MainMenu_View_Filter->Items[i]->Checked = false;
    }
  }

  if (nOpenFlags & DB_OPEN_FLAG_EXISTING) {
    ResetListView(RELOAD_TAGS);
    m_sDbFileName = sFileName;
    if (nOpenFlags & DB_OPEN_FLAG_READONLY)
      m_blDbReadOnly = true;
    else {
      int nAttr = FileGetAttr(sFileName);
      m_blDbReadOnly = nAttr != faInvalid && (nAttr & faReadOnly);
    }
  }
  else {
    AddModifyListViewEntry();
    m_sDbFileName = WString();
    m_blDbReadOnly = false;
  }

  MainMenu_File_Lock->Caption = TRL("Lock");

  //DbView->Font->Color = m_defaultListColor;

  ChangeCaption();
  ResetDbOpenControls();
  SetRecoveryKeyDependencies();

  m_dbViewSelItemThread = std::make_unique<TSelectItemThread>(
    std::bind(&TPasswMngForm::ApplyDbViewItemSelection, this, nullptr));
  m_tagViewSelItemThread = std::make_unique<TSelectItemThread>(
    ApplyTagViewItemSelection);

  if (nOpenFlags & DB_OPEN_FLAG_UNLOCK) {
    if (!m_lockSelTags.empty()) {
      TagView->Items->BeginUpdate();
      for (int i : m_lockSelTags) {
        if (i < TagView->Items->Count)
          TagView->Items->Item[i]->Selected = true;
      }
      TagView->Items->EndUpdate();
      if (m_tagViewSelItemThread)
        m_tagViewSelItemThread->ApplyNow();
    }

    if (m_nLockSelItemIndex >= 0 && m_nLockSelItemIndex < DbView->Items->Count) {
      DbView->Items->Item[m_nLockSelItemIndex]->Selected = true;
      if (m_dbViewSelItemThread)
        m_dbViewSelItemThread->ApplyNow();
      ScrollToItem(m_nLockSelItemIndex);
    }

    m_lockSelTags.clear();
    m_nLockSelItemIndex = -1;
  }
  else {
    auto checkExpiryFlag = [this](word32 lFlag)
    {
      for (const auto pEntry : *m_passwDb) {
        if (pEntry->UserFlags & lFlag)
          return true;
      }
      return false;
    };

    if (g_config.Database.WarnExpiredEntries &&
        checkExpiryFlag(DB_FLAG_EXPIRED) &&
          MsgBox(TRL("The database contains expired entries.\nDo you want to filter "
            "these entries now?"),
            MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDYES) {
      MainMenu_View_Filter_Expired->Checked = true;
      ResetListView();
    }
    if (g_config.Database.WarnEntriesExpireSoon &&
        checkExpiryFlag(DB_FLAG_EXPIRES_SOON) &&
          MsgBox(TRL("The database contains entries that will expire soon.\n"
            "Do you want to filter these entries now?"),
            MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDYES) {
      //MainMenu_View_Filter_Expired->Checked = false;
      MainMenu_View_Filter_ExpireSoon->Checked = true;
      ResetListView();
    }

    if (DbView->Items->Count == 1) {
      DbView->Items->Item[0]->Selected = true;
    }
  }

  TogglePasswBtn->Down = true;
  TogglePasswBtnClick(this);

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
    if (nLock == DB_LOCK_AUTO && g_config.Database.LockAutoSave) {
      MainMenu_File_SaveClick(this);
      if (m_blDbChanged)
        return ASK_SAVE_ERROR;
    }
    else {
      if (nLock == DB_LOCK_AUTO)
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

#ifdef _DEBUG
  if (m_passwDb.use_count() > 1) {
    ShowMessage("Database object is still in use somewhere!");
  }
#endif

  m_nLockSelItemIndex = -1;
  m_lockSelTags.clear();

  if (nLock) {
    if (m_pSelectedItem)
      m_nLockSelItemIndex = m_pSelectedItem->Index;
    for (int i = 1; i < TagView->Items->Count; i++) {
      if (TagView->Items->Item[i]->Selected)
        m_lockSelTags.push_back(i);
    }
  }

  if (m_dbViewSelItemThread) {
    m_dbViewSelItemThread->TerminateAndWait();
    m_dbViewSelItemThread.reset();
  }
  if (m_tagViewSelItemThread) {
    m_tagViewSelItemThread->TerminateAndWait();
    m_tagViewSelItemThread.reset();
  }

  m_passwDb.reset();

  //m_tempKeyVal.reset();
  //m_tempPasswHistory.reset();

  //m_pSelectedItem = nullptr;

  SetItemChanged(false);
  ToggleShutdownBlocker();

  m_nSearchMode = SEARCH_MODE_OFF;
  //m_nNumSearchResults = 0;
  m_nPasswEntropyBits = 0;

  //m_globalTags.clear();
  //m_searchResultTags.clear();
  m_tags.clear();
  m_globalCaseiTags.clear();
  m_tagFilter.clear();

  ClearListView();
  DbView->Font->Color = m_defaultListColor;
  TagView->Clear();
  ResetDbOpenControls();
  TagMenu->Items->Clear();
  PasswDbSettingsDlg->Clear();
  DisableAutoComplete(UserNameBox->Handle);

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
  FilterInfoPanel->Visible = false;

  IdleTimer->Enabled = false;

  if (g_config.Database.ClearClipCloseLock)
    Clipboard()->Clear();

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
  PasswChangeInfo->Caption = WString();
  m_nPasswEntropyBits = 0;
  if (PasswQualityBtn->Down)
    EstimatePasswQuality();
  m_tempKeyVal.reset();
  m_tempPasswHistory.reset();
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::SaveDatabase(const WString& sFileName)
{
  SuspendIdleTimer;

  if (g_config.Database.CreateBackup && FileExists(sFileName)) {
    const WString BACKUP_EXT = ".bak";
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
          WString sCheckFileName = sBackupFileName + Format("%.3d",
            ARRAYOFCONST((nBackupNum))) + BACKUP_EXT;
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
        sBackupFileName += Format("%.3d", ARRAYOFCONST((nBackupNum))) + BACKUP_EXT;
      }
      else
        sBackupFileName += BACKUP_EXT;
    }
    else {
      WString sTryFileName = sBackupFileName + "_" +
        getTimeStampString() + BACKUP_EXT;
      if (FileExists(sTryFileName))
        sBackupFileName = sBackupFileName + "_" +
          getTimeStampString(true) + BACKUP_EXT;
      else
        sBackupFileName = sTryFileName;
      //sBackupFileName += BACKUP_EXT;
    }
    // close file in case target file is equal to currently opened file
    m_passwDb->ReleaseFile();
    if (!CopyFile(sFileName.c_str(), sBackupFileName.c_str(), false))
      MsgBox(TRLFormat("Could not save backup file\n\"%1\".", { sBackupFileName }),
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
    sError = CppStdExceptionToString(e);
  }

  if (!sError.IsEmpty()) {
    MsgBox(TRLFormat("Error while saving database file:\n%1.", { sError }),
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
  MainMenu_File_SetRecoveryPassword->Enabled = blOpen;

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
  EditPanel->Enabled = blOpen && DbView->SelCount > 0;

  SaveBtn->Enabled = blOpen && !m_blDbReadOnly;
  LockBtn->Enabled = blOpen;
  AddEntryBtn->Enabled = blOpen;
  SearchBox->Enabled = blOpen;
  ClearSearchBtn->Enabled = blOpen;
  //CaseSensitiveBtn->Enabled = blOpen;
  SearchBtn->Enabled = blOpen;
  //FuzzyBtn->Enabled = blOpen;
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
    SearchBtn->Enabled = !m_blItemChanged;
  }

  //MainMenu_File->Enabled = !m_blItemChanged;
  MainMenu_Edit->Enabled = !m_blItemChanged;
  MainMenu_View->Enabled = !m_blItemChanged;

  if (m_blItemChanged) {
    PrevBtn->Visible = false;
    NextBtn->Visible = false;
    DeleteBtn->Visible = false;
  }
  else {
    m_blItemPasswChangeConfirm = false;
    ResetNavControls();
  }

  ToggleShutdownBlocker(m_blItemChanged ?
    TRL("Password database entry is being edited.") : WString());
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ResetNavControls(void)
{
  int nNumSel = DbView->SelCount;
  //bool blSingleSel = nNumSel == 1 && m_pSelectedItem != nullptr;
  int nFirstSelIdx = DbView->Selected ? DbView->Selected->Index : -1;
  PrevBtn->Visible = nFirstSelIdx > 0;
  NextBtn->Visible = nFirstSelIdx >= 0 && nFirstSelIdx < DbView->Items->Count - 1;
  DeleteBtn->Visible = nNumSel > 1 || (m_pSelectedItem && m_pSelectedItem->Data);
}
//---------------------------------------------------------------------------
int __fastcall TPasswMngForm::AddPassw(const wchar_t* pwszPassw,
  bool blShowNumPassw,
  const wchar_t* pwszParam)
{
  if (pwszPassw == nullptr || pwszPassw[0] == '\0' || !IsDbOpen())
    return 0;

  if (m_blItemChanged) {
    MsgBox(TRL("Please finish editing the current entry\nbefore adding new entries."),
      MB_ICONWARNING);
    return 0;
  }

  const wchar_t* pwszNext;
  int nNumPassw = 0;
  SecureWString sParam;
  if (pwszParam != nullptr && pwszParam[0] != '\0')
    sParam.AssignStr(pwszParam);

  do {
    pwszNext = wcsstr(pwszPassw, CRLF);

    int nLen = (pwszNext != nullptr) ? static_cast<int>(pwszNext - pwszPassw) :
      wcslen(pwszPassw);

    if (nLen > 0) {
      SecureWString sPassw(nLen + 1);
      wcsncpy(sPassw, pwszPassw, nLen);
      sPassw[nLen] = '\0';

      PasswDbEntry* pNewEntry = m_passwDb->NewDbEntry();
      m_passwDb->SetDbEntryPassw(*pNewEntry, sPassw);

      if (!sParam.IsEmpty())
        pNewEntry->Strings[PasswDbEntry::TITLE] = sParam;

      nNumPassw++;
    }

    if (pwszNext != nullptr)
      pwszPassw = pwszNext + 2;
  }
  while (pwszNext != nullptr && nNumPassw < 1000);

  if (nNumPassw > 0) {
    SetDbChanged(true, true);
    ResetListView(RELOAD_TAGS);
  }

  if (blShowNumPassw) {
    SuspendIdleTimer;
    MsgBox(TRLFormat("%1 entries with passwords have been added.",
      { IntToStr(nNumPassw) }),
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

  // "fuzzy" always performs case-insensitive search, even if the actual
  // fuzzy algorithm is not used
  if (blFuzzy)
    blCaseSensitive = false;
  if (sStr.Length() < 4)
    blFuzzy = false;

  if (!blCaseSensitive)
    sStr = AnsiLowerCase(sStr);

  m_nSearchMode = blFuzzy ? SEARCH_MODE_FUZZY : SEARCH_MODE_NORMAL;

  //ClearListView();
  //DbView->Items->BeginUpdate();

  SecureWString sBuf;

  if (!blCaseSensitive)
    sBuf.New(1024);

  int nNumFound = 0;

  for (auto *pEntry : *m_passwDb)
  {
    pEntry->UserFlags &= ~DB_FLAG_FOUND;
    for (int nI = 0; nI < PasswDbEntry::NUM_STRING_FIELDS; nI++) {
      if (nFlags & (1 << nI)) {
        const SecureWString* psSrc;
        SecureWString sPassw;
        if (nI == PasswDbEntry::PASSWORD && !pEntry->HasPlaintextPassw()) {
          sPassw = m_passwDb->GetDbEntryPassw(*pEntry);
          psSrc = &sPassw;
        }
        else
          psSrc = &pEntry->Strings[nI];
        if (!psSrc->IsStrEmpty()) {
          if (!blCaseSensitive) {
            if (psSrc->Size() > sBuf.Size())
              sBuf.New(psSrc->Size());
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
            //AddModifyListViewEntry(nullptr, pEntry);
            nNumFound++;
            break;
          }
        }
      }
    }
  }

  //m_pSelectedItem = nullptr;
  //SetItemChanged(false);
  //ResetNavControls();

  //DbView->Items->EndUpdate();

  DbView->Font->Color = DBVIEW_SEARCH_COLOR;

  //m_nNumSearchResults = nNumFound;

  //DbView->Selected = nullptr;

  ResetListView(RELOAD_TAGS);

  SearchResultPanel->Caption = nNumFound == 1 ? TRL("1 entry found.") :
    TRLFormat("%1 entries found.", { IntToStr(nNumFound) });

  //if (DbView->Items->Count > 0) {
  //  DbView->Items->Item[0]->Selected = true;
  //}
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchDbForKeyword(bool blAutotype)
{
  if (!IsDbOpen() || m_blItemChanged)
    return;

  PasswDbEntry* pFound = nullptr;
  WString sWinTitle;
  int nNumFound = 0;

  HWND hWin = GetForegroundWindow();
  if (hWin != INVALID_HANDLE_VALUE) {
    const int BUFSIZE = 256;
    wchar_t wszWinTitle[BUFSIZE];
    if (GetWindowText(hWin, wszWinTitle, BUFSIZE) != 0) {
      sWinTitle = wszWinTitle;
      CharLower(wszWinTitle);
      for (auto pEntry : *m_passwDb) {
        if (!blAutotype)
          pEntry->UserFlags &= ~DB_FLAG_FOUND;
        if (!pEntry->Strings[PasswDbEntry::KEYWORD].IsStrEmpty()) {
          SecureWString sKeyword = pEntry->Strings[PasswDbEntry::KEYWORD];
          CharLower(sKeyword.Data());
          if (wcsstr(wszWinTitle, sKeyword) != nullptr) {
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
  if (!pFound) {
    SuspendIdleTimer;
    sMsg = TRLFormat("No matching keyword found for window text\n\"%1\".",
        { sWinTitle });
  }

  if (blAutotype) {
    if (pFound) {
      SendKeys sk(g_config.AutotypeDelay);
      sk.SendComplexString(getDbEntryAutotypeSeq(pFound), pFound, m_passwDb);
    }
    else
      MainForm->ShowTrayInfo(sMsg, bfWarning);
  }
  else {
    if (pFound) {
      m_nSearchMode = SEARCH_MODE_NORMAL;
      //m_nNumSearchResults = nNumFound;
      DbView->Font->Color = DBVIEW_SEARCH_COLOR;
      //DbView->Selected = nullptr;
      ResetListView(RELOAD_TAGS);
      //DbView->Items->Item[0]->Selected = true;
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

  bool blAdd = pItem == nullptr;

  if (blAdd)
    pItem = DbView->Items->Add();

  pItem->Data = pEntry;

  if (pEntry && (pEntry->UserFlags & DB_FLAG_EXPIRED)) {
    pItem->ImageIndex = DBVIEW_IMAGE_INDEX_START;
    pItem->Indent = 0;
  }
  else if (pEntry && (pEntry->UserFlags & DB_FLAG_EXPIRES_SOON)) {
    pItem->ImageIndex = DBVIEW_IMAGE_INDEX_START + 1;
    pItem->Indent = 0;
  }
  else {
    pItem->ImageIndex = -1;
    pItem->Indent = -1;
  }

  bool blShowPassw = MainMenu_View_ShowPasswInList->Checked;
  const int MAX_NOTES_LEN = 200;

  for (int nI = 0, nColIdx = 0; nI < PasswDbEntry::NUM_FIELDS; nI++) {
    if (m_nShowColMask & (1 << nI)) {
      if (pEntry != nullptr) {
        const wchar_t* pwszSrc = nullptr;
        WString sCustomStr;
        SecureWString sParam;
        if (nI == PasswDbEntry::PASSWORD) {
          if (!blShowPassw)
            pwszSrc = HIDE_PASSW_STR;
          else if (!pEntry->HasPlaintextPassw()) {
            sParam = m_passwDb->GetDbEntryPassw(*pEntry);
            pwszSrc = sParam.c_str();
          }
        }
        if (pwszSrc == nullptr) {
          switch (nI) {
          case PasswDbEntry::NOTES:
            if (!pEntry->Strings[PasswDbEntry::NOTES].IsStrEmpty()) {
              if (pEntry->Strings[PasswDbEntry::NOTES].StrLen() > MAX_NOTES_LEN) {
                word32 lPos = 0;
                sParam.StrCat(
                  pEntry->Strings[PasswDbEntry::NOTES].c_str(),
                  MAX_NOTES_LEN,
                  lPos);

                lPos -= 3;
                sParam.StrCat(L"...", 3, lPos);
                //wcscpy(&sParam[MAX_NOTES_LEN - 3], L"...");
              }
              else
                sParam = pEntry->Strings[PasswDbEntry::NOTES];

              for (auto c = sParam.begin(); *c != '\0'; c++) {
                if (*c == '\r' || *c == '\n')
                  *c = ' ';
              }
            }
            pwszSrc = sParam.c_str();
            break;
          case PasswDbEntry::CREATIONTIME:
            pwszSrc = pEntry->CreationTimeString.c_str();
            break;
          case PasswDbEntry::MODIFICATIONTIME:
            pwszSrc = pEntry->ModificationTimeString.c_str();
            break;
          case PasswDbEntry::PASSWCHANGETIME:
            pwszSrc = pEntry->PasswChangeTimeString.c_str();
            break;
          case PasswDbEntry::PASSWEXPIRYDATE:
            pwszSrc = pEntry->PasswExpiryDateString.c_str();
            break;
          case PasswDbEntry::PASSWHISTORY:
            sCustomStr = IntToStr(static_cast<int>(
              pEntry->GetPasswHistory().GetSize()));
            pwszSrc = sCustomStr.c_str();
            break;
          default:
            if (nI < PasswDbEntry::NUM_STRING_FIELDS)
              pwszSrc = pEntry->Strings[nI].c_str();
            else
              pwszSrc = sCustomStr.c_str();
          }
        }
        if (nColIdx == 0) {
          if (m_nSearchMode == SEARCH_MODE_FUZZY)
            pItem->Caption = Format("[%d%%] ",
              ARRAYOFCONST((std::min(100, pEntry->UserTag)))) + WString(pwszSrc);
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
void __fastcall TPasswMngForm::SetListViewSortFlag(void)
{
  auto h = ListView_GetHeader(DbView->Handle);
  for (int i = 0; i < DbView->Columns->Count; i++) {
    THDItem item;
    memzero(&item, sizeof(item));
    item.mask = HDI_FORMAT;
    Header_GetItem(h, i, &item);
    item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
    if (DbView->Columns->Items[i]->Tag == m_nSortByIdx)
      item.fmt |= m_nSortOrderFactor > 0 ? HDF_SORTUP : HDF_SORTDOWN;
    Header_SetItem(h, i, &item);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ResetListView(int nFlags)
{
  void* pPrevSelData = (m_pSelectedItem != nullptr &&
    m_nSearchMode == SEARCH_MODE_OFF) ? m_pSelectedItem->Data : nullptr;

  /*SCROLLINFO scrollInfo;
  scrollInfo.cbSize = sizeof(SCROLLINFO);
  scrollInfo.fMask = SIF_POS;
  GetScrollInfo(DbView->Handle, SB_VERT, &scrollInfo);*/

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

    SetListViewSortFlag();
  }

  if (IsDbOpen()) {
    DbView->Items->BeginUpdate();

    //const PasswDbList& db = m_passwDb->GetDatabase();

    if (nFlags & RELOAD_TAGS) {
      std::map<SecureWString, word32> tags, searchResultTags;
      word32 lNumUntagged = 0;
      word32 lNumUntaggedSearch = 0;
      word32 lNumSearchResults = 0;
      for (const auto pEntry : *m_passwDb) {
        if (pEntry->GetTagList().size() == 0)
          lNumUntagged++;
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
            lNumUntaggedSearch++;
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
          TRL("All search results")) + FormatW(" (%1)",
          { UIntToStr((m_nSearchMode == SEARCH_MODE_OFF) ?
            m_passwDb->Size : lNumSearchResults) });
      pItem->ImageIndex = TAGVIEW_IMAGE_INDEX_START + 1;
      pItem->Data = nullptr;

      std::set<SecureWString> newTagFilter;

      for (const auto& kv : src) {
        const auto& sTag = kv.first;

        m_tags.emplace_back(sTag, kv.second);

        auto pItem = TagView->Items->Add();

        pItem->Caption = WString(sTag.c_str()) + Format(" (%d)",
          ARRAYOFCONST((kv.second)));
        pItem->ImageIndex = TAGVIEW_IMAGE_INDEX_START;
        pItem->Data = &m_tags.back();

        if (m_tagFilter.count(sTag) != 0) {
          newTagFilter.insert(sTag);
          pItem->Selected = true;
        }
      }

      word32 lBaseNumUntagged;
      bool blShowUntagged;
      if (m_nSearchMode == SEARCH_MODE_OFF) {
        lBaseNumUntagged = lNumUntagged;
        blShowUntagged = lBaseNumUntagged && lBaseNumUntagged < m_passwDb->Size;
      }
      else {
        lBaseNumUntagged = lNumUntaggedSearch;
        blShowUntagged = lBaseNumUntagged && lBaseNumUntagged < lNumSearchResults;
      }
      if (blShowUntagged) {
        m_tags.emplace_back(SecureWString(), lBaseNumUntagged);
        pItem = TagView->Items->Add();
        pItem->Caption = TRL("Untagged") + Format(" (%d)",
          ARRAYOFCONST((lBaseNumUntagged)));
        pItem->ImageIndex = TAGVIEW_IMAGE_INDEX_START + 2;
        pItem->Data = &m_tags.back();
        if (m_tagFilter.count(SecureWString()) != 0) {
          newTagFilter.insert(SecureWString());
          pItem->Selected = true;
        }
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

    int nIdx = 0, nPrevSelIdx = -1;
    word32 lCurrDate, lExpirySoonDate;

    getExpiryCheckDates(lCurrDate, lExpirySoonDate);

    enum class FilterType {
      None,
      Expired,
      ExpireSoon,
      WeakPassw
    } filterType = FilterType::None;
    if (MainMenu_View_Filter_Expired->Checked)
      filterType = FilterType::Expired;
    else if (MainMenu_View_Filter_ExpireSoon->Checked)
      filterType = FilterType::ExpireSoon;
    else if (MainMenu_View_Filter_WeakPassw->Checked)
      filterType = FilterType::WeakPassw;
    std::map<SecureWString, SecureWString> userNamesMap;

    for (const auto pEntry : *m_passwDb) {
      if (!pEntry->Strings[PasswDbEntry::USERNAME].IsStrEmpty()) {
        SecureWString sUserNameLC = pEntry->Strings[PasswDbEntry::USERNAME];
        CharLower(sUserNameLC.Data());
        userNamesMap.emplace(sUserNameLC, pEntry->Strings[PasswDbEntry::USERNAME]);
      }

      pEntry->UserFlags &= ~(DB_FLAG_EXPIRED | DB_FLAG_EXPIRES_SOON);
      if (pEntry->PasswExpiryDate != 0) {
        if (lCurrDate >= pEntry->PasswExpiryDate)
          pEntry->UserFlags |= DB_FLAG_EXPIRED;
        else if (lExpirySoonDate >= pEntry->PasswExpiryDate)
          pEntry->UserFlags |= DB_FLAG_EXPIRES_SOON;
      }

      if ((filterType == FilterType::Expired && !(pEntry->UserFlags & DB_FLAG_EXPIRED)) ||
          (filterType == FilterType::ExpireSoon && !(pEntry->UserFlags & DB_FLAG_EXPIRES_SOON)) ||
          (m_nSearchMode != SEARCH_MODE_OFF && !(pEntry->UserFlags & DB_FLAG_FOUND)))
        continue;

      if (filterType == FilterType::WeakPassw) {
        auto sPassw = m_passwDb->GetDbEntryPassw(*pEntry);
        if (!sPassw.IsStrEmpty()) {
          int nEntropyBits;
          if (g_config.UseAdvancedPasswEst)
            nEntropyBits = FloorEntropyBits(ZxcvbnMatch(
              WStringToUtf8(sPassw).c_str(), nullptr, nullptr));
          else
            nEntropyBits = PasswordGenerator::EstimatePasswSecurity(sPassw.c_str());
          if (nEntropyBits >= WEAK_PASSW_THRESHOLD)
            continue;
        }
        else
          continue;
      }

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

      AddModifyListViewEntry(nullptr, pEntry);
      if (pPrevSelData == pEntry)
        nPrevSelIdx = nIdx;

      nIdx++;
    }

    if (m_nSearchMode == SEARCH_MODE_OFF && filterType == FilterType::None)
      AddModifyListViewEntry();

    if (DbView->Items->Count >= 2 && m_nSearchMode == SEARCH_MODE_FUZZY &&
        m_nSortByIdx < 0)
      DbView->AlphaSort();

    DbView->Items->EndUpdate();

    if (nPrevSelIdx >= 0) {
      if (m_nSortByIdx >= 0) {
        for (int i = 0; i < DbView->Items->Count; i++) {
          if (DbView->Items->Item[i]->Data == pPrevSelData) {
            nPrevSelIdx = i;
            break;
          }
        }
      }
      DbView->Items->Item[nPrevSelIdx]->Selected = true;
      ScrollToItem(nPrevSelIdx);
    }
    else if (m_nSearchMode != SEARCH_MODE_OFF && DbView->Items->Count > 0)
      DbView->Items->Item[0]->Selected = true;
    else
      ApplyDbViewItemSelection();
      //m_pSelectedItem = nullptr;
      //DbViewSelectItem(this, nullptr, false);

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
    else
      DisableAutoComplete(UserNameBox->Handle);

    if (filterType != FilterType::None) {
      FilterInfoPanel->Visible = true;

      WString sInfo;
      switch (filterType) {
      case FilterType::Expired:
        sInfo = MainMenu_View_Filter_Expired->Caption; break;
      case FilterType::ExpireSoon:
        sInfo = MainMenu_View_Filter_ExpireSoon->Caption; break;
      case FilterType::WeakPassw:
        sInfo = MainMenu_View_Filter_WeakPassw->Caption; break;
      }

      FilterInfoPanel->Hint = RemoveAccessKeysFromStr(sInfo);
    }
    else {
      FilterInfoPanel->Visible = false;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ClearListView(void)
{
  m_pSelectedItem = nullptr;
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
      if (pEntry != nullptr) {
        word32 lPos = pEntry->GetIndex();
        word32 lNewPos = -1;
        if (nDir == MOVE_TOP)
          lNewPos = j++;
        else if (nDir == MOVE_UP && lPos > 0)
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
  OpenDatabase();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ApplyDbViewItemSelection(TListItem* pItem)
{
  NotifyUserAction();

  int nNumSel = DbView->SelCount;
  EditPanel->Enabled = nNumSel > 0;

  if (nNumSel == 1) {
    if (pItem == nullptr) {
      pItem = DbView->Selected;
      if (pItem == m_pSelectedItem)
        return;
    }
    //if (m_pSelectedItem != nullptr)
    //  ClearEditPanel();
  }
  else
    pItem = nullptr;

  Tag = FORM_TAG_ITEM_SELECTED;

  if (m_pSelectedItem)
    ClearEditPanel();
  if (pItem && pItem->Data) {
    PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(pItem->Data);
    SetEditBoxTextBuf(TitleBox, pEntry->Strings[PasswDbEntry::TITLE].c_str());
    SetEditBoxTextBuf(UserNameBox, pEntry->Strings[PasswDbEntry::USERNAME].c_str());
    SetEditBoxTextBuf(UrlBox, pEntry->Strings[PasswDbEntry::URL].c_str());
    SetEditBoxTextBuf(KeywordBox, pEntry->Strings[PasswDbEntry::KEYWORD].c_str());
    SetEditBoxTextBuf(KeyValueListBox, BuildTranslKeyValString(pEntry->GetKeyValueList()));
    SetEditBoxTextBuf(NotesBox, pEntry->Strings[PasswDbEntry::NOTES].c_str());

    if (!pEntry->GetTagList().empty()) {
      SecureWString sTags(200);
      //word32 i = 0;
      //word32 lCount = pEntry->GetTagList().size();
      word32 lPos = 0;
      /*for (const auto& sTag : pEntry->GetTagList()) {
        sTags.Grow(lPos + sTag.StrLen() + 2);
        if (lPos > 0) {
          wcscpy(sTags + lPos, L", ");
          lPos += 2;
        }
        wcscpy(sTags + lPos, sTag.c_str());
        lPos += sTag.StrLen();
      }
      sTags[lPos] = '\0';*/
      for (const auto& sTag : pEntry->GetTagList()) {
        if (lPos > 0)
          sTags.StrCat(L", ", 2, lPos);
        sTags.StrCat(sTag, lPos);
      }
      SetEditBoxTextBuf(TagsBox, sTags.c_str());
    }
    else
      SetEditBoxTextBuf(TagsBox, L"");

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
    PasswChangeInfo->Caption = pEntry->PasswChangeTimeString.IsStrEmpty() ?
      "-" : WString(pEntry->PasswChangeTimeString.c_str());

    SecureWString sPassw = m_passwDb->GetDbEntryPassw(*pEntry);

    SetEditBoxTextBuf(PasswBox, sPassw.c_str());

    if (PasswQualityBtn->Down)
      EstimatePasswQuality(sPassw);
  }
  else if (pItem) {
    SetEditBoxTextBuf(UserNameBox, m_passwDb->DefaultUserName.c_str());

    if (!m_passwDb->PasswFormatSeq.IsStrEmpty()) {
      w32string sFormat = WCharToW32String(m_passwDb->PasswFormatSeq.c_str());
      PasswordGenerator passwGen(&RandomPool::GetInstance());
      SecureW32String sDest(16001);
      if (passwGen.GetFormatPassw(sDest, sFormat, 0) != 0) {
        W32CharToWCharInternal(sDest);
        const wchar_t* pwszPassw = reinterpret_cast<wchar_t*>(sDest.Data());
        SetEditBoxTextBuf(PasswBox, pwszPassw);
        if (PasswQualityBtn->Down)
          EstimatePasswQuality(pwszPassw);
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
    else
      ExpiryCheck->Checked = false;
  }

  Tag = 0;

  m_pSelectedItem = pItem;
  PasswHistoryBtn->Left = PasswChangeInfo->Left + PasswChangeInfo->Width + 4;

  if (m_blItemChanged)
    SetItemChanged(false);
  else
    ResetNavControls();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewSelectItem(TObject *Sender,
  TListItem *Item, bool Selected)
{
  if (m_dbViewSelItemThread)
    m_dbViewSelItemThread->Trigger();
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
  if (m_pSelectedItem == nullptr)
    return;

  if (GetEditBoxTextLen(TitleBox) == 0 && GetEditBoxTextLen(PasswBox) == 0)
  {
    MsgBox(TRL("Please specify at least a title or password."), MB_ICONWARNING);
    return;
  }

  SecureWString sPassw = GetEditBoxTextBuf(PasswBox);

  if (!sPassw.IsStrEmpty() && m_blItemPasswChangeConfirm && TogglePasswBtn->Down) {
    bool blOk = false;
    if (PasswEnterDlg->Execute(0, TRL("Confirm password"), this) == mrOk) {
      if (GetEditBoxTextBuf(PasswBox) == PasswEnterDlg->GetPassw())
        blOk = true;
      else
        MsgBox(TRL("Passwords are not identical."), MB_ICONERROR);
    }
    PasswEnterDlg->Clear();
    if (!blOk)
      return;
  }

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  bool blNewEntry = false;

  if (pEntry == nullptr) {
    pEntry = m_passwDb->NewDbEntry();
    m_pSelectedItem->Data = pEntry;
    //AddModifyListViewEntry();
    blNewEntry = true;
  }

  //SecureWString sTitle, sUserName, sUrl, sKeyword, sNotes;
  pEntry->Strings[PasswDbEntry::TITLE] = GetEditBoxTextBuf(TitleBox);
  pEntry->Strings[PasswDbEntry::USERNAME] = GetEditBoxTextBuf(UserNameBox);
  pEntry->Strings[PasswDbEntry::URL] = GetEditBoxTextBuf(UrlBox);
  pEntry->Strings[PasswDbEntry::KEYWORD] = GetEditBoxTextBuf(KeywordBox);
  pEntry->Strings[PasswDbEntry::NOTES] = GetEditBoxTextBuf(NotesBox);

  if (m_tempKeyVal) {
    pEntry->SetKeyValueList(*m_tempKeyVal.get());
    pEntry->UpdateKeyValueString();
    m_tempKeyVal.reset();
  }

  if (m_tempPasswHistory) {
    pEntry->GetPasswHistory().AdoptFrom(*m_tempPasswHistory.get());
    m_tempPasswHistory.reset();
  }

  //bool blResetView = false;
  pEntry->ClearTagList();

  int nNumOverlongTags = 0;

  if (GetEditBoxTextLen(TagsBox) != 0) {
    SecureWString sTags = GetEditBoxTextBuf(TagsBox);
    const wchar_t* p = sTags.c_str();

    std::map<SecureWString,SecureWString> newTags;

    auto items = SplitStringBuf(sTags.c_str(), L",;");

    for (const auto& item : items) {
      int nStart = 0, nEnd = item.StrLen() - 1;

      // trim left side first, abort if string consists of spaces entirely
      for (; nStart <= nEnd && item[nStart] == ' '; nStart++);
      if (nStart > nEnd)
        continue;

      // limit length of tag
      if (nEnd - nStart + 1 > DB_MAX_TAG_LEN) {
        nEnd = nStart + DB_MAX_TAG_LEN - 1;
        nNumOverlongTags++;
      }

      // now trim right side
      for (; nEnd > nStart && item[nEnd] == ' '; nEnd--);

      int nTagLen = nEnd - nStart + 1;
      SecureWString sTag(&item[nStart], nTagLen + 1);
      sTag[nTagLen] = '\0';

      SecureWString sCiTag = sTag;
      CharLower(sCiTag.Data());

      auto it = m_globalCaseiTags.find(sCiTag);
      if (it != m_globalCaseiTags.end())
        newTags.emplace(it->first, it->second);
      else
        newTags.emplace(sCiTag, sTag);
    }

    for (const auto& kv : newTags) {
      pEntry->AddTag(kv.second);
    }

    pEntry->UpdateTagsString();
  }

  //SecureWString sPassw = GetEditBoxTextBuf(PasswBox);
  const SecureWString sOldPassw = m_passwDb->GetDbEntryPassw(*pEntry);
  bool blPasswChanged = sPassw != sOldPassw;

  if (blPasswChanged) {
    m_passwDb->SetDbEntryPassw(*pEntry, sPassw);
    pEntry->AddCurrentPasswToHistory(sOldPassw);
  }

  word32 lExpiryDate = 0;
  if (ExpiryCheck->Checked) {
    unsigned short wYear, wMonth, wDay;
    ExpiryDatePicker->Date.DecodeDate(&wYear, &wMonth, &wDay);
    lExpiryDate = pEntry->EncodeExpiryDate(wYear, wMonth, wDay);
  }
  pEntry->PasswExpiryDate = lExpiryDate;
  pEntry->PasswExpiryDateString = pEntry->ExpiryDateToString(lExpiryDate);

  pEntry->UpdateModificationTime(blPasswChanged);

  if (m_tags.empty() && pEntry->GetTagList().empty()) {
    pEntry->UserFlags &= ~(DB_FLAG_EXPIRED | DB_FLAG_EXPIRES_SOON);
    if (lExpiryDate != 0) {
      word32 lCurrDate, lExpirySoonDate;
      getExpiryCheckDates(lCurrDate, lExpirySoonDate);
      if (pEntry->PasswExpiryDate != 0) {
        if (lCurrDate >= pEntry->PasswExpiryDate)
          pEntry->UserFlags |= DB_FLAG_EXPIRED;
        else if (lExpirySoonDate >= pEntry->PasswExpiryDate)
          pEntry->UserFlags |= DB_FLAG_EXPIRES_SOON;
      }
    }
    AddModifyListViewEntry(m_pSelectedItem, pEntry);
    if (blNewEntry) {
      AddModifyListViewEntry();
      WString sAll = TRL("All", "tags") + Format(" (%d)",
        ARRAYOFCONST((m_passwDb->Size)));
      TListItem* pItem = (TagView->Items->Count > 0) ?
        TagView->Items->Item[0] : TagView->Items->Add();
      pItem->Caption = sAll;
      pItem->ImageIndex = TAGVIEW_IMAGE_INDEX_START + 1;
      pItem->Data = nullptr;
    }

    if (m_nSortByIdx >= 0 || m_nSearchMode == SEARCH_MODE_FUZZY)
      DbView->AlphaSort();
  }
  else
    ResetListView(RELOAD_TAGS);

  SetItemChanged(false);
  SetDbChanged(true, true);

  if (nNumOverlongTags > 0) {
    MsgBox(TRLFormat("The maximum tag length is %1 characters.\n"
      "Longer tags have been shortened.", { IntToStr(DB_MAX_TAG_LEN) }),
      MB_ICONWARNING);
  }

  NextBtnClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PrevBtnClick(TObject *Sender)
{
  NotifyUserAction();

  auto pSelItem = DbView->Selected;
  if (pSelItem == nullptr || /*DbView->SelCount != 1 ||*/ pSelItem->Index == 0)
    return;

  int nIdx = pSelItem->Index;
  //pSelItem->Selected = false;
  DbView->ClearSelection();
  DbView->Items->Item[nIdx - 1]->Selected = true;
  ScrollToItem(nIdx - 1);
  DbView->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NextBtnClick(TObject *Sender)
{
  NotifyUserAction();

  auto pSelItem = DbView->Selected;
  if (pSelItem == nullptr || /*DbView->SelCount != 1 ||*/
      pSelItem->Index == DbView->Items->Count - 1)
    return;

  int nIdx = pSelItem->Index;
  //pSelItem->Selected = false;
  DbView->ClearSelection();
  DbView->Items->Item[nIdx + 1]->Selected = true;
  ScrollToItem(nIdx + 1);
  DbView->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_SaveAsClick(TObject *Sender)
{
  SuspendIdleTimer;

  BeforeDisplayDlg();
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);
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

  OpenDatabase(DB_OPEN_FLAG_EXISTING);
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
  //DbViewSelectItem(this, m_pSelectedItem, true);
  ApplyDbViewItemSelection(m_pSelectedItem);
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
  if (m_pSelectedItem != nullptr && Tag != FORM_TAG_ITEM_SELECTED) {
    if (!m_blItemChanged) {
      NotifyUserAction();
      SetItemChanged(true);
    }
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
      if (pEntry != nullptr)
        nNumValid++;
    }
  }

  WString sMsg = nNumValid == 1 ? TRL("Delete 1 entry?") :
    TRLFormat("Delete %1 entries?", { IntToStr(nNumValid) });
  if (MsgBox(sMsg, MB_ICONWARNING + MB_YESNO + MB_DEFBUTTON2) == IDNO)
    return;

  for (int nI = 0; nI < DbView->Items->Count; nI++) {
    if (DbView->Items->Item[nI]->Selected) {
      PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>
        (DbView->Items->Item[nI]->Data);
      if (pEntry != nullptr)
        m_passwDb->DeleteDbEntry(*pEntry);
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
    SetListViewSortFlag();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::DbViewCompare(TObject *Sender,
  TListItem *Item1, TListItem *Item2, int Data, int &Compare)
{
  PasswDbEntry* pEntry1 = reinterpret_cast<PasswDbEntry*>(Item1->Data);
  PasswDbEntry* pEntry2 = reinterpret_cast<PasswDbEntry*>(Item2->Data);

  if (pEntry1 == nullptr)
    Compare = 1;
  else if (pEntry2 == nullptr)
    Compare = -1;
  else if (m_nSearchMode == SEARCH_MODE_FUZZY)
    Compare = pEntry2->UserTag - pEntry1->UserTag;
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
        SecureWString sPassw1 = m_passwDb->GetDbEntryPassw(*pEntry1),
          sPassw2 = m_passwDb->GetDbEntryPassw(*pEntry2);
        Compare = wcscmp(sPassw1.c_str(), sPassw2.c_str()) * m_nSortOrderFactor;
      }
      break;
    case PasswDbEntry::CREATIONTIME:
      Compare = CompareFileTime(pEntry1->CreationTime, pEntry2->CreationTime) *
        m_nSortOrderFactor;
      break;
    case PasswDbEntry::MODIFICATIONTIME:
      Compare = CompareFileTime(pEntry1->ModificationTime,
        pEntry2->ModificationTime) * m_nSortOrderFactor;
      break;
    case PasswDbEntry::PASSWCHANGETIME:
      Compare = CompareFileTime(pEntry1->PasswChangeTime,
        pEntry2->PasswChangeTime) * m_nSortOrderFactor;
      break;
    case PasswDbEntry::PASSWEXPIRYDATE:
      Compare = (pEntry1->PasswExpiryDate - pEntry2->PasswExpiryDate) *
        m_nSortOrderFactor;
      break;
    case PasswDbEntry::PASSWHISTORY:
      Compare = (pEntry1->GetPasswHistory().GetSize() -
        pEntry2->GetPasswHistory().GetSize()) * m_nSortOrderFactor;
      break;
    default:
      if (m_nSortByIdx < PasswDbEntry::NUM_STRING_FIELDS)
        Compare = _wcsicmp(pEntry1->Strings[m_nSortByIdx].c_str(),
            pEntry2->Strings[m_nSortByIdx].c_str()) * m_nSortOrderFactor;
      else
        Compare = 0;
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
SecureMem<word8> TPasswMngForm::RequestPasswAndCheck(const WString& sRequestMsg,
  const WString& sInvalidMsg,
  std::function<bool(const SecureMem<word8>&)> checkFunc)
{
  SecureMem<word8> key;

  while (PasswEnterDlg->Execute(PASSWENTER_FLAG_ENABLEKEYFILE,
      sRequestMsg, this) == mrOk)
  {
    try {
      auto tryKey = PasswDatabase::CombineKeySources(
        PasswEnterDlg->GetPasswBinary(), PasswEnterDlg->GetKeyFileName());
      PasswEnterDlg->Clear();
      Screen->Cursor = crHourGlass;

      bool blValid = checkFunc(tryKey);

      Screen->Cursor = crDefault;

      if (blValid) {
        key = std::move(tryKey);
        break;
      }

      MsgBox(sInvalidMsg, MB_ICONERROR);
    }
    catch (Exception& e) {
      PasswEnterDlg->Clear();
      Screen->Cursor = crDefault;
      MsgBox(e.Message + ".", MB_ICONERROR);
    }
  }

  PasswEnterDlg->Clear();
  RandomPool::GetInstance().Flush();

  return key;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_ChangeMasterPasswordClick(
  TObject *Sender)
{
  SuspendIdleTimer;

  auto key = RequestPasswAndCheck(TRL("Enter OLD master password"),
    TRL("Old master password is invalid."),
    [this](const SecureMem<word8>& key) {
      return m_passwDb->CheckMasterKey(key) || (m_passwDb->HasRecoveryKey &&
        m_passwDb->CheckRecoveryKey(key));
    });

  if (key.IsEmpty())
    return;

  if (PasswEnterDlg->Execute(PASSWENTER_FLAG_CONFIRMPASSW |
      PASSWENTER_FLAG_ENABLEKEYFILE | PASSWENTER_FLAG_ENABLEKEYFILECREATION,
      TRL("Enter NEW master password"), this) == mrOk) {
    key = PasswDatabase::CombineKeySources(
      PasswEnterDlg->GetPasswBinary(), PasswEnterDlg->GetKeyFileName());
    Screen->Cursor = crHourGlass;
    m_passwDb->ChangeMasterKey(key);
    Screen->Cursor = crDefault;
    SetDbChanged();
    MsgBox(TRL("Master password successfully changed."), MB_ICONINFORMATION);
  }

  PasswEnterDlg->Clear();
  RandomPool::GetInstance().Flush();
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
  if (blChanged && g_config.Database.AutoSave && !m_sDbFileName.IsEmpty() &&
      (g_config.Database.AutoSaveOption == asdEveryChange ||
      (g_config.Database.AutoSaveOption == asdEntryModification && blEntryChanged)))
  {
    if (SaveDatabase(m_sDbFileName))
      blChanged = false;
  }

  if (blChanged != m_blDbChanged) {
    m_blDbChanged = blChanged;
    ChangeCaption();
  }

  ToggleShutdownBlocker(blChanged ?
    TRL("Password database contains unsaved changes.") : WString());
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
    PasswBox->Tag = PASSWBOX_TAG_PASSW_GEN;
    try {
      if (m_pSelectedItem != nullptr && m_pSelectedItem->Data != nullptr) {
        const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(
          m_pSelectedItem->Data);
        const SecureWString* psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[
          DB_KEYVAL_PROFILE]);
        if (psVal != nullptr) {
          if (!MainForm->LoadProfile(psVal->c_str())) {
            MsgBox(TRLFormat("Profile \"%1\" not found.",
              { WString(psVal->c_str()) }),
              MB_ICONERROR);
            return;
          }
        }
        else {
          psVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[DB_KEYVAL_FORMATPASSW]);
          if (psVal != nullptr) {
            w32string sFormat = WCharToW32String(psVal->c_str());
            SecureW32String sDest(16001);
            PasswordGenerator passwGen(g_pRandSrc);
            if (passwGen.GetFormatPassw(sDest, sFormat, 0) != 0) {
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
    __finally {
      PasswBox->Tag = 0;
    }
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

  bool blIsDbEntry = m_pSelectedItem != nullptr && m_pSelectedItem->Data != nullptr;
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

  if (m_pSelectedItem == nullptr || m_pSelectedItem->Data == nullptr)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  if (!pEntry->Strings[PasswDbEntry::USERNAME].IsStrEmpty()) {
    SecureClipboard::GetInstance().SetData(
      pEntry->Strings[PasswDbEntry::USERNAME].c_str());
    //MainForm->CopiedSensitiveDataToClipboard();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_CopyPasswClick(
  TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == nullptr || m_pSelectedItem->Data == nullptr)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  SecureWString sPassw = m_passwDb->GetDbEntryPassw(*pEntry);
  if (!sPassw.IsStrEmpty()) {
    SecureClipboard::GetInstance().SetData(sPassw.c_str());
    //MainForm->CopiedSensitiveDataToClipboard();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_OpenUrlClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == nullptr || m_pSelectedItem->Data == nullptr)
    return;

  PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  if (!pEntry->Strings[PasswDbEntry::URL].IsStrEmpty())
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
      if (pOriginal != nullptr) {
        //PasswDbEntry* pDuplicate = m_passwDb->AddDbEntry(true, false);
        SecureWString sNewTitle = pOriginal->Strings[PasswDbEntry::TITLE];
        if (!sNewTitle.IsEmpty()) {
          sNewTitle.GrowBy(sCopyStr.Length());
          wcscat(sNewTitle, sCopyStr.c_str());
        }
        else
          sNewTitle.AssignStr(sCopyStr.c_str());
        m_passwDb->DuplicateDbEntry(*pOriginal, sNewTitle);
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
  s.DefaultMaxPasswHistorySize = m_passwDb->DefaultMaxPasswHistorySize;
  s.CipherType = m_passwDb->CipherType;
  s.NumKdfRounds = m_passwDb->KdfIterations;
  s.Compressed = m_passwDb->Compressed;
  s.CompressionLevel = m_passwDb->CompressionLevel;

  PasswDbSettingsDlg->SetSettings(s, m_passwDb->HasRecoveryKey);
  if (PasswDbSettingsDlg->ShowModal() == mrOk &&
      (m_passwDb->DefaultUserName != s.DefaultUserName ||
      m_passwDb->PasswFormatSeq != s.PasswFormatSeq ||
      m_passwDb->DefaultPasswExpiryDays != s.DefaultExpiryDays ||
      m_passwDb->DefaultMaxPasswHistorySize != s.DefaultMaxPasswHistorySize ||
      m_passwDb->CipherType != s.CipherType ||
      m_passwDb->KdfIterations != s.NumKdfRounds ||
      m_passwDb->Compressed != s.Compressed ||
      m_passwDb->CompressionLevel != s.CompressionLevel))
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
    OpenDatabase(DB_OPEN_FLAG_EXISTING | DB_OPEN_FLAG_UNLOCK);
  }
  else if (CloseDatabase(false, blAuto ? DB_LOCK_AUTO : DB_LOCK_MANUAL)) {
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
     !m_blLocked && !IsDisplayDlg() && /*!ProgressForm->Visible &&*/
     !(Screen->ActiveForm != nullptr && Screen->ActiveForm->FormState.Contains(fsModal)))
  {
    if (SecondsBetween(Now(), m_lastUserActionTime) >= g_config.Database.LockIdleTime)
      LockOrUnlockDatabase(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NotifyUserAction(void)
{
  m_lastUserActionTime = Now();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  SearchBox->Clear();
  //if (g_config.Database.ClearClipExit)
  //  Clipboard()->Clear();
  TopMostManager::GetInstance().OnFormClose(this);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance().SetForm(this);
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
    OpenDatabase(DB_OPEN_FLAG_EXISTING, (!g_cmdLineOptions.PasswDbFileName.IsEmpty() &&
      blStartup) ? g_cmdLineOptions.PasswDbFileName : m_sDbFileName);
  else if (Tag == FORM_TAG_UNLOCK_DB || (m_blLocked && !m_blUnlockTried)) {
    OpenDatabase(DB_OPEN_FLAG_EXISTING | DB_OPEN_FLAG_UNLOCK);
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
    DbView->Font->Color = m_defaultListColor;
    SearchResultPanel->Caption = WString();
  }
  else if (!sText.IsEmpty()) {
    //m_blSearchMode = true;
    SearchDatabase(SearchBox->Text, m_nSearchFlags,
      SearchMenu_CaseSensitive->Checked,
      SearchMenu_FuzzySearch->Checked);
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

  int nColumn = Column->Tag;
  int nSortOrderFactor = 1;
  if (nColumn == m_nSortByIdx)
    nSortOrderFactor = (m_nSortOrderFactor > 0) ? -1 : 1;

  m_nSortByIdx = nColumn;
  m_nSortOrderFactor = nSortOrderFactor;
  MainMenu_View_SortBy->Items[m_nSortByIdx+2]->Checked = true;
  MainMenu_View_SortBy->Items[MainMenu_View_SortBy->Count - 2 +
    (m_nSortOrderFactor>0?0:1)]->Checked = true;
  SetListViewSortFlag();
  DbView->SortType = TSortType::stData;
  DbView->AlphaSort();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchBoxSelect(TObject *Sender)
{
  if (SearchBox->Tag == 0) {
    NotifyUserAction();
    SearchDatabase(SearchBox->Text, m_nSearchFlags,
      SearchMenu_CaseSensitive->Checked,
      SearchMenu_FuzzySearch->Checked);
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

  /*case SIZE_MINIMIZED:
    if (IsDbOpen()) {
      if (g_config.Database.LockMinimize)
        MainMenu_File_LockClick(this);
    }
    m_blUnlockTried = false;*/
  }

  TForm::Dispatch(&msg);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::OnWindowPosChanging(TWMWindowPosChanging& msg)
{
  //const int SWP_STATECHANGED = 0x8000
  const int HIDE1 = SWP_NOCOPYBITS | SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOACTIVATE;
  const int HIDE2 = SWP_HIDEWINDOW | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE;
  /*if (msg.WindowPos->flags & (SWP_STATECHANGED | SWP_FRAMECHANGED) &&
      msg.WindowPos->x < 0 && msg.WindowPos->y < 0) {
    Caption = "Maximize";
  }
  else*/
  if ((msg.WindowPos->flags & HIDE1) == HIDE1 ||
      (msg.WindowPos->flags & HIDE2) == HIDE2) {
    if (IsDbOpen() && !(g_nAppState & APPSTATE_AUTOTYPE) &&
        g_config.Database.LockMinimize) {
      MainMenu_File_LockClick(this);
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
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = SaveDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);
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
    MsgBox(TRLFormat("Error while creating CSV file:\n%1.",
      { e.Message }),
      MB_ICONERROR);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_EditClick(TObject *Sender)
{
  if (!IsDbOpen())
    return;

  bool blIsDbEntry = m_pSelectedItem != nullptr && m_pSelectedItem->Data != nullptr;
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
  //AddEntryBtn->Enabled = m_nSearchMode == SEARCH_MODE_OFF;
  MainMenu_Edit_AddEntry->Enabled = m_nSearchMode == SEARCH_MODE_OFF &&
    !FilterInfoPanel->Visible;
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
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  FontDlg->Font = DbView->Font;
  if (m_nSearchMode != SEARCH_MODE_OFF)
    DbView->Font->Color = DBVIEW_SEARCH_COLOR;
  if (FontDlg->Execute())
    DbView->Font = FontDlg->Font;
  TopMostManager::GetInstance().RestoreTopMosts(this);
  AfterDisplayDlg();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_PerformAutotypeClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == nullptr || m_pSelectedItem->Data == nullptr)
    return;

  const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);

  if (g_config.MinimizeAutotype) {
    // hold local copy of database pointer to prevent object from being destroyed
    // before calling autotype function
    auto passwDb = m_passwDb;
    g_nAppState |= APPSTATE_AUTOTYPE;
    Application->Minimize();
    SendKeys sk(g_config.AutotypeDelay);
    sk.SendComplexString(getDbEntryAutotypeSeq(pEntry), pEntry, passwDb);
    g_nAppState &= ~APPSTATE_AUTOTYPE;
  }
  else {
    TSendKeysThread::TerminateAndWait();
    if (!TSendKeysThread::ThreadRunning())
      new TSendKeysThread(Handle, g_config.AutotypeDelay,
        getDbEntryAutotypeSeq(pEntry), pEntry, m_passwDb);
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
    if (pDropItem != nullptr && pDropItem != m_pSelectedItem) {
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
      m_pSelectedItem != nullptr && m_pSelectedItem->Data != nullptr)
  {
    NotifyUserAction();
    DbView->BeginDrag(false);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_RunClick(TObject *Sender)
{
  NotifyUserAction();

  if (m_pSelectedItem == nullptr || m_pSelectedItem->Data == nullptr)
    return;

  const PasswDbEntry* pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
  const SecureWString* pVal = pEntry->GetKeyValue(DB_KEYVAL_KEYS[DB_KEYVAL_RUN]);
  if (pVal != nullptr) {
    if (!ExecuteCommand(pVal->c_str(), false))
      ExecuteShellOp(pVal->c_str(), true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_Edit_AddEntryClick(TObject *Sender)
{
  NotifyUserAction();

  int nCount = DbView->Items->Count;
  if (nCount > 0 &&
      DbView->Items->Item[nCount - 1]->Data == nullptr &&
      !m_blItemChanged)
  {
    DbView->ClearSelection();
    ScrollToItem(nCount - 1);
    DbView->Items->Item[nCount - 1]->Selected = true;
    if (m_dbViewSelItemThread)
      m_dbViewSelItemThread->ApplyNow();
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
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  FontDlg->Font = PasswBox->Font;
  if (FontDlg->Execute()) {
    PasswBox->Font = FontDlg->Font;
    //PasswDbSettingsDlg->PasswGenTestBox->Font = FontDlg->Font;
  }
  TopMostManager::GetInstance().RestoreTopMosts(this);
  AfterDisplayDlg();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ApplyTagViewItemSelection(void)
{
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
void __fastcall TPasswMngForm::TagViewSelectItem(TObject *Sender, TListItem *Item,
          bool Selected)
{
  if (TagView->Tag == 0 && m_tagViewSelItemThread)
    m_tagViewSelItemThread->Trigger();
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
    if (pData1->first.IsEmpty())
      Compare = 1;
    else if (pData2->first.IsEmpty())
      Compare = -1;
    else if (m_nTagsSortByIdx == 0) {
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
    auto point = AddTagBtn->ClientToScreen(TPoint(0, AddTagBtn->Height + 2));
    TagMenu->Popup(point.X, point.Y);
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
    word32 lLen = GetEditBoxTextLen(TagsBox);
    if (lLen > 0) {
      sTags = GetEditBoxTextBuf(TagsBox);
      while (lLen > 0 && sTags[lLen - 1] == ' ')
        lLen--;
      if (lLen > 0 && sTags[lLen - 1] != ',' && sTags[lLen - 1] != ';') {
        //sTags.Grow(nLen + sNewTag.StrLen() + 3);
        //wcscpy(&sTags[nLen], L", ");
        //nLen += 2;
        sTags.StrCat(L", ", 2, lLen);
      }
      //else
      //  sTags.Grow(nLen + sNewTag.StrLen() + 1);
      //wcscpy(&sTags[nLen], sNewTag.c_str());
      sTags.StrCat(sNewTag, lLen);
    }
    else
      sTags = sNewTag;
    SetEditBoxTextBuf(TagsBox, sTags.c_str());
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
  if (!m_passwDb->HasRecoveryKey &&
      settings.NumKdfRounds != m_passwDb->KdfIterations) {
    auto key = RequestPasswAndCheck(TRL("Enter master password again"),
      TRL("Master password is invalid."), m_passwDb->CheckMasterKey);

    if (key.IsEmpty())
      return false;

    //std::atomic<bool> cancelFlag(false);
    TaskCancelToken cancelToken;
    word32 lKdfIterations = settings.NumKdfRounds;

    // no need to create a thread-safe RNG instance if database doesn't
    // use a recovery key
    //RandomPool randPool(RandomPool::GetInstance());

    auto pTask = TTask::Create([this,&key,&cancelToken,lKdfIterations]() {
      m_passwDb->ChangeMasterKey(
        key,
        lKdfIterations,
        cancelToken.Get().get());
    });

    pTask->Start();

    int nTimeout = 0;
    while (!pTask->Wait(100)) {
      Application->ProcessMessages();
      nTimeout += 100;
      if (nTimeout >= 1000 && !ProgressForm->Visible) {
        ProgressForm->ExecuteModal(
          PasswDbSettingsDlg,
          TRL("Changing number of KDF iterations"),
          TRL("Computing derived key ..."),
          cancelToken.Get(),
          [&pTask](unsigned int timeout)
          {
            return pTask->Wait(timeout);
          }
        );
        break;
      }
    }

    if (cancelToken) {
      if (cancelToken.Reason == TaskCancelReason::UserCancel)
        MsgBox(EUserCancel::UserCancelMsg, MB_ICONERROR);
      return false;
    }
  }

  m_passwDb->DefaultUserName = settings.DefaultUserName;
  m_passwDb->PasswFormatSeq = settings.PasswFormatSeq;
  m_passwDb->DefaultPasswExpiryDays = settings.DefaultExpiryDays;
  m_passwDb->DefaultMaxPasswHistorySize = settings.DefaultMaxPasswHistorySize;
  if (!m_passwDb->HasRecoveryKey)
    m_passwDb->CipherType = settings.CipherType;
  m_passwDb->Compressed = settings.Compressed;
  m_passwDb->CompressionLevel = settings.CompressionLevel;
  //m_passwDb->KdfIterations = settings.NumKdfRounds;

  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::EditKeyValBtnClick(TObject *Sender)
{
  if (!m_passwDb->IsOpen() || m_pSelectedItem == nullptr)
    return;

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
  word32 lPos = 0;
  for (const auto& p : keyValList) {
    auto it = m_keyValNames.find(std::wstring(p.first.c_str()));
    if (it != m_keyValNames.end()) {
      /*sResult.Grow(nPos + it->second.length() + p.second.StrLen() + 3);
      wcscpy(&sResult[nPos], it->second.c_str());
      nPos += it->second.length();
      sResult[nPos++] = '=';
      wcscpy(&sResult[nPos], p.second.c_str());
      nPos += p.second.StrLen();
      sResult[nPos++] = ',';
      sResult[nPos++] = ' ';*/
      if (lPos != 0)
        sResult.StrCat(L", ", 2, lPos);
      sResult.StrCat(it->second.c_str(), it->second.length(), lPos);
      sResult.StrCat(L"=", 1, lPos);
      sResult.StrCat(p.second, lPos);
    }
  }
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
  //POINT pt;
  //GetCursorPos(&pt);
  auto point = ExpiryBtn->ClientToScreen(TPoint(0, ExpiryBtn->Height + 2));
  ExpiryMenu->Popup(point.X, point.Y);
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
  SuspendIdleTimer;
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

    PasswMngDbPropDlg->SetProperty(DbProperty::RecoveryKeySet,
      TRL(m_passwDb->HasRecoveryKey ? "Yes" : "No"));

    PasswMngDbPropDlg->SetProperty(DbProperty::NumEntries, IntToStr(
      static_cast<int>(m_passwDb->Size)));

    int nExpiredEntries = 0;
    for (const auto pEntry : *m_passwDb) {
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
    MsgBox(e.Message + ".", MB_ICONERROR);
  }

  PasswMngDbPropDlg->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_OpenReadOnlyClick(TObject *Sender)
{
  NotifyUserAction();
  OpenDatabase(DB_OPEN_FLAG_EXISTING | DB_OPEN_FLAG_READONLY);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_File_SetRecoveryPasswordClick(TObject *Sender)
{
  SuspendIdleTimer;
  bool blSuccess = false;

  if (!m_passwDb->HasRecoveryKey) {
    auto key = RequestPasswAndCheck(TRL("Enter master password"),
      TRL("Master password is invalid."), m_passwDb->CheckMasterKey);

    if (key.IsEmpty())
      return;

    if (PasswEnterDlg->Execute(PASSWENTER_FLAG_CONFIRMPASSW |
        PASSWENTER_FLAG_ENABLEKEYFILE | PASSWENTER_FLAG_ENABLEKEYFILECREATION,
        TRL("Enter recovery password"), this) == mrOk) {
      try {
        auto recoveryKey = PasswDatabase::CombineKeySources(
          PasswEnterDlg->GetPasswBinary(), PasswEnterDlg->GetKeyFileName());

        Screen->Cursor = crHourGlass;
        m_passwDb->SetRecoveryKey(key, recoveryKey);
        Screen->Cursor = crDefault;

        blSuccess = true;
        MsgBox(TRL("Recovery password successfully set."), MB_ICONINFORMATION);
      }
      catch (Exception& e) {
        Screen->Cursor = crDefault;
        MsgBox(e.Message + ".", MB_ICONERROR);
      }
    }
  }
  else {
    {
      auto recoveryKey = RequestPasswAndCheck(TRL("Enter recovery password"),
        TRL("Recovery password is invalid."), m_passwDb->CheckRecoveryKey);

      if (recoveryKey.IsEmpty())
        return;
    }

    if (PasswEnterDlg->Execute(PASSWENTER_FLAG_CONFIRMPASSW |
        PASSWENTER_FLAG_ENABLEKEYFILE | PASSWENTER_FLAG_ENABLEKEYFILECREATION,
        TRL("Enter NEW master password"), this) == mrOk) {
      try {
        auto key = PasswDatabase::CombineKeySources(
          PasswEnterDlg->GetPasswBinary(), PasswEnterDlg->GetKeyFileName());

        Screen->Cursor = crHourGlass;
        m_passwDb->RemoveRecoveryKey(key);
        Screen->Cursor = crDefault;

        blSuccess = true;
        MsgBox(TRL("Recovery password successfully removed."), MB_ICONINFORMATION);
      }
      catch (Exception& e) {
        Screen->Cursor = crDefault;
        MsgBox(e.Message + ".", MB_ICONERROR);
      }
    }
  }

  PasswEnterDlg->Clear();
  RandomPool::GetInstance().Flush();

  if (blSuccess) {
    SetDbChanged(true);
    SetRecoveryKeyDependencies();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SetRecoveryKeyDependencies(void)
{
  bool blVal = m_passwDb->HasRecoveryKey;
  MainMenu_File_SetRecoveryPassword->Caption = TRL(blVal ?
    "Remove Recovery Password..." : "Set Recovery Password...");
  PasswDbSettingsDlg->EncryptionAlgoList->Enabled = !blVal;
  PasswDbSettingsDlg->NumKdfRoundsBox->Enabled = !blVal;
  PasswDbSettingsDlg->CalcRoundsBtn->Enabled = !blVal;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SearchMenu_SelectFieldsClick(TObject *Sender)
{
  SuspendIdleTimer;
  int nNewFlags = PasswMngColDlg->Execute(this, m_nSearchFlags, true);
  if (nNewFlags >= 0)
    m_nSearchFlags = nNewFlags;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::TitleBoxKeyPress(TObject *Sender, System::WideChar &Key)

{
  if (m_pSelectedItem == nullptr)
    Key = 0;
  else if (Key == VK_RETURN) {
    AddModifyBtnClick(this);
    Key = 0;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::SetPasswQualityBarWidth(void)
{
  PasswSecurityBar->Width = PasswBox->Width - 1;
  PasswSecurityBarPanel->Width = std::max<int>(std::min(
    m_nPasswEntropyBits / 128.0, 1.0) * PasswSecurityBar->Width, 4);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::EstimatePasswQuality(const wchar_t* pwszPassw)
{
  m_nPasswEntropyBits = 0;
  if (pwszPassw || PasswBox->GetTextLen() != 0) {
    SecureWString sPassw;
    if (!pwszPassw) {
      sPassw = GetEditBoxTextBuf(PasswBox);
      pwszPassw = sPassw.c_str();
    }
    m_nPasswEntropyBits = g_config.UseAdvancedPasswEst ? FloorEntropyBits(
      ZxcvbnMatch(WStringToUtf8(pwszPassw).c_str(), nullptr, nullptr)) :
      PasswordGenerator::EstimatePasswSecurity(pwszPassw);
  }

  PasswSecurityLbl->Caption = IntToStr(m_nPasswEntropyBits);
  SetPasswQualityBarWidth();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PasswBoxChange(TObject *Sender)
{
  if (m_pSelectedItem != nullptr && Tag != FORM_TAG_ITEM_SELECTED) {
    // check whether entry has been changed
    if (!m_blItemChanged) {
      NotifyUserAction();
      SetItemChanged(true);
    }

    // check whether password change needs to be confirmed
    m_blItemPasswChangeConfirm = PasswBox->Tag != PASSWBOX_TAG_PASSW_GEN;

    // estimate password quality
    if (PasswQualityBtn->Down) {
      EstimatePasswQuality();
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PasswQualityBtnClick(TObject *Sender)
{
  NotifyUserAction();
  if (PasswQualityBtn->Down) {
    PasswSecurityBar->Visible = true;
    PasswSecurityBarPanel->ShowCaption = false;
    EstimatePasswQuality();
  }
  else {
    PasswSecurityBar->Visible = false;
    PasswSecurityBarPanel->Width = PasswBox->Width;
    PasswSecurityBarPanel->ShowCaption = true;
    PasswSecurityLbl->Caption = WString();
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::EditPanelResize(TObject *Sender)
{
  if (Visible) {
    if (PasswQualityBtn->Down) {
      SetPasswQualityBarWidth();
    }
    else {
      PasswSecurityBarPanel->Width = PasswBox->Width;
    }
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PasswSecurityBarPanelMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
  NotifyUserAction();
  if (Shift.Contains(ssLeft))
    StartEditBoxDragDrop(PasswBox);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::UrlBtnClick(TObject *Sender)
{
  NotifyUserAction();
  ExecuteShellOp(UrlBox->Text);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::PasswHistoryBtnClick(TObject *Sender)
{
  if (!m_passwDb->IsOpen() || m_pSelectedItem == nullptr) {
    NotifyUserAction();
    return;
  }

  SuspendIdleTimer;

  if (!m_tempPasswHistory) {
    if (m_pSelectedItem->Data != nullptr) {
      auto pEntry = reinterpret_cast<PasswDbEntry*>(m_pSelectedItem->Data);
      m_tempPasswHistory.reset(new PasswDbEntry::PasswHistory(
        pEntry->GetPasswHistory()));
    }
    else
      m_tempPasswHistory.reset(new PasswDbEntry::PasswHistory(
        m_passwDb->DefaultMaxPasswHistorySize,
        m_passwDb->DefaultMaxPasswHistorySize > 0));
  }

  PasswHistoryDlg->EnableHistoryCheck->Checked =
    m_tempPasswHistory->GetActive();
  PasswHistoryDlg->EnableHistoryCheckClick(this);
  PasswHistoryDlg->HistorySizeSpinBtn->Position = m_tempPasswHistory->GetMaxSize();
  PasswHistoryDlg->HistorySizeBox->Text = IntToStr(static_cast<int>(
    m_tempPasswHistory->GetMaxSize()));
  PasswHistoryDlg->HistoryView->Clear();

  for (const auto& entry : *m_tempPasswHistory) {
    auto pItem = PasswHistoryDlg->HistoryView->Items->Add();
    pItem->Caption = (entry.first.dwLowDateTime == 0 &&
      entry.first.dwHighDateTime == 0) ? WString("-") :
      WString(PasswDbEntry::TimeStampToString(entry.first).c_str());
    pItem->SubItems->Add(entry.second.c_str());
  }

  if (PasswHistoryDlg->ShowModal() == mrOk) {
    m_tempPasswHistory->SetActive(PasswHistoryDlg->EnableHistoryCheck->Checked);
    m_tempPasswHistory->SetMaxSize(PasswHistoryDlg->HistorySizeSpinBtn->Position);
    if (m_tempPasswHistory->GetSize() != 0 &&
        PasswHistoryDlg->HistoryView->Items->Count == 0)
      m_tempPasswHistory->ClearHistory();
    SetItemChanged(true);
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_ResetListFontClick(TObject *Sender)
{
  NotifyUserAction();
  DbView->Font = Font;
  if (m_nSearchMode != SEARCH_MODE_OFF)
    DbView->Font->Color = DBVIEW_SEARCH_COLOR;
  m_defaultListColor = Font->Color;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::NotesBoxKeyPress(TObject *Sender, System::WideChar &Key)

{
  if (m_pSelectedItem == nullptr)
    Key = 0;
}
//---------------------------------------------------------------------------
bool __fastcall TPasswMngForm::SaveDbOnShutdown(void)
{
  if (IsDbOpen()) {
    if (m_blItemChanged)
      return false;
    if (!m_blDbChanged)
      return true;
    if (!g_config.Database.LockAutoSave)
      return false;

    WString sSaveFileName;
    if (m_sDbFileName.IsEmpty())
      sSaveFileName = "Untitled_" + getTimeStampString() + PWDB_FILE_EXT;
    else if (!m_blDbReadOnly)
      sSaveFileName = m_sDbFileName;
    else {
      WString sFilePath = ExtractFilePath(m_sDbFileName),
        sFileName = ExtractFileName(m_sDbFileName),
        sFileNameWithoutExt, sFileExt;
      int nPos = sFileName.LastDelimiter(".");
      if (nPos > 1) {
        sFileNameWithoutExt = sFileName.SubString(1, nPos - 1);
        sFileExt = sFileName.SubString(nPos, sFileName.Length() - nPos + 1);
      }
      else
        sFileNameWithoutExt = sFileName;
      sSaveFileName = sFilePath + sFileNameWithoutExt + "_session_" +
        getTimeStampString() + sFileExt;
    }

    return SaveDatabase(sSaveFileName);
  }
  return true;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::OnEndSession(TWMEndSession& msg)
{
  if (msg.EndSession) {
    m_passwDb.reset();
  }
  else {
    g_terminateAction = TerminateAction::None;
  }
  msg.Result = 0;
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::OnQueryEndSession(TWMQueryEndSession& msg)
{
  g_terminateAction = TerminateAction::SystemShutdown;
  msg.Result = SaveDbOnShutdown();
  //if (msg.Result)
  //  ToggleShutdownBlocker();
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ToggleShutdownBlocker(const WString& sMsg)
{
  static bool blActive = false;
  if (!sMsg.IsEmpty()) {
    ShutdownBlockReasonCreate(Handle, sMsg.c_str());
    blActive = true;
  }
  else if (blActive) {
    ShutdownBlockReasonDestroy(Handle);
    blActive = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::MainMenu_View_Filter_ExpiredClick(TObject *Sender)

{
  ResetListView(0);
}
//---------------------------------------------------------------------------
void __fastcall TPasswMngForm::ClearFilterBtnClick(TObject *Sender)
{
  for (int i = 0; i < MainMenu_View_Filter->Count; i++) {
    MainMenu_View_Filter->Items[i]->Checked = false;
  }
  ResetListView(0);
}
//---------------------------------------------------------------------------

