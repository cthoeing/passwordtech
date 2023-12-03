// PasswManager.h
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

#ifndef PasswManagerH
#define PasswManagerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <System.ImageList.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
//#include <Vcl.Grids.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.ToolWin.hpp>
#include <Vcl.Imaging.jpeg.hpp>
#include <Vcl.BaseImageCollection.hpp>
#include <Vcl.ImageCollection.hpp>
#include <Vcl.VirtualImageList.hpp>
#include <map>
#include <list>
//---------------------------------------------------------------------------
#include <ComCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <ToolWin.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.ValEdit.hpp>
#include "PasswDatabase.h"
#include "PasswMngDbSettings.h"

class TSelectItemThread : public TThread
{
public:
  __fastcall TSelectItemThread(std::function<void(void)>);

  __fastcall ~TSelectItemThread()
  {
    delete m_pEvent;
  }

  void __fastcall Trigger(void)
  {
    if (++m_nTriggers > 1)
      m_pEvent->SetEvent();
  }

  void __fastcall ApplyNow(void);

  void __fastcall TerminateAndWait(void);

private:
  void __fastcall Execute(void) override;

  TSimpleEvent* m_pEvent;
  std::atomic<int> m_nTriggers;
  //std::atomic<bool> m_blStop;
  std::function<void(void)> m_applyFunc;
};


class TPasswMngForm : public TForm
{
__published:	// IDE-managed Components
  TListView *DbView;
  TMainMenu *MainMenu;
  TMenuItem *MainMenu_File;
  TMenuItem *MainMenu_File_Open;
  TMenuItem *MainMenu_File_SaveAs;
  TMenuItem *MainMenu_File_New;
  TMenuItem *MainMenu_File_N1;
  TMenuItem *MainMenu_File_Save;
  TOpenDialog *OpenDlg;
  TSaveDialog *SaveDlg;
  TMenuItem *MainMenu_File_Close;
  TPanel *EditPanel;
  TLabel *TitleLbl;
  TLabel *UserNameLbl;
  TLabel *PasswLbl;
  TLabel *UrlLbl;
  TLabel *KeywordLbl;
  TLabel *NotesLbl;
  TSpeedButton *AddModifyBtn;
  TSpeedButton *PrevBtn;
  TSpeedButton *NextBtn;
  TSpeedButton *CancelBtn;
  TEdit *TitleBox;
  TEdit *UserNameBox;
  TEdit *PasswBox;
  TEdit *UrlBox;
  TEdit *KeywordBox;
  TMemo *NotesBox;
    TSplitter *Splitter2;
  TLabel *CreationTimeLbl;
  TLabel *CreationTimeInfo;
  TMenuItem *MainMenu_View;
  TMenuItem *MainMenu_View_ShowColumns;
  TMenuItem *MainMenu_View_ShowPasswInList;
  TSpeedButton *DeleteBtn;
  TMenuItem *MainMenu_View_SortBy;
  TMenuItem *MainMenu_View_SortBy_Unsorted;
  TMenuItem *MainMenu_View_SortBy_N1;
    TMenuItem *MainMenu_View_N1;
  TLabel *LastModificationLbl;
  TLabel *LastModificationInfo;
  TSpeedButton *TogglePasswBtn;
  TMenuItem *MainMenu_File_N2;
  TMenuItem *MainMenu_File_ChangeMasterPassword;
  TPopupMenu *DbViewMenu;
  TMenuItem *DbViewMenu_CopyUserName;
  TMenuItem *DbViewMenu_CopyPassw;
  TMenuItem *DbViewMenu_OpenUrl;
  TToolBar *ToolBar;
  TComboBox *SearchBox;
  TMenuItem *MainMenu_Edit;
  TMenuItem *MainMenu_Edit_CopyUserName;
  TMenuItem *MainMenu_Edit_CopyPassw;
  TMenuItem *MainMenu_Edit_OpenUrl;
    TMenuItem *MainMenu_Edit_N1;
  TMenuItem *MainMenu_Edit_DuplicateEntry;
  TMenuItem *MainMenu_Edit_DeleteEntry;
    TMenuItem *MainMenu_Edit_N3;
  TMenuItem *MainMenu_Edit_SelectAll;
  TMenuItem *MainMenu_File_DbSettings;
  TToolButton *NewBtn;
  TToolButton *OpenBtn;
  TMenuItem *MainMenu_File_Lock;
  TToolButton *SaveBtn;
  TSpeedButton *ClearSearchBtn;
  TMenuItem *MainMenu_File_N3;
  TMenuItem *MainMenu_File_Exit;
  TToolButton *LockBtn;
  TTimer *IdleTimer;
  TMenuItem *MainMenu_File_Export;
  TMenuItem *MainMenu_File_N4;
  TMenuItem *MainMenu_File_Export_CsvFile;
  TMenuItem *DbViewMenu_N1;
  TMenuItem *DbViewMenu_DuplicateEntry;
  TMenuItem *DbViewMenu_DeleteEntry;
  TMenuItem *MainMenu_View_N2;
  TMenuItem *MainMenu_View_ChangeListFont;
  TFontDialog *FontDlg;
    TMenuItem *MainMenu_Edit_PerformAutotype;
    TMenuItem *DbViewMenu_PerformAutotype;
    TLabel *KeyValueListLbl;
  TMenuItem *MainMenu_Edit_Run;
  TMenuItem *DbViewMenu_Run;
  TMenuItem *MainMenu_Edit_AddEntry;
  TPanel *SearchResultPanel;
    TMenuItem *MainMenu_Edit_N2;
    TMenuItem *MainMenu_Edit_Rearrange;
    TMenuItem *MainMenu_Edit_Rearrange_Top;
    TMenuItem *MainMenu_Edit_Rearrange_Up;
    TMenuItem *MainMenu_Edit_Rearrange_Down;
    TMenuItem *MainMenu_Edit_Rearrange_Bottom;
    TMenuItem *DbViewMenu_N2;
    TMenuItem *DbViewMenu_Rearrange;
    TMenuItem *DbViewMenu_Rearrange_Top;
    TMenuItem *DbViewMenu_Rearrange_Up;
    TMenuItem *DbViewMenu_Rearrange_Down;
    TMenuItem *DbViewMenu_Rearrange_Bottom;
    TMenuItem *MainMenu_View_SortBy_N2;
    TMenuItem *MainMenu_View_SortBy_Ascending;
    TMenuItem *MainMenu_View_SortBy_Descending;
    TMenuItem *MainMenu_View_ChangePasswFont;
    TSplitter *Splitter1;
    TListView *TagView;
    TLabel *TagsLbl;
    TEdit *TagsBox;
    TSpeedButton *AddTagBtn;
    TPopupMenu *TagMenu;
    TMenuItem *MainMenu_View_SortTagsBy;
    TMenuItem *MainMenu_View_SortTagsBy_Name;
    TMenuItem *MainMenu_View_SortTagsBy_Freq;
    TMenuItem *MainMenu_View_SortTagsBy_N1;
    TMenuItem *MainMenu_View_SortTagsBy_Ascending;
    TMenuItem *MainMenu_View_SortTagsBy_Descending;
  TMenuItem *DbViewMenu_N3;
  TMenuItem *DbViewMenu_SelectAll;
  TEdit *KeyValueListBox;
  TSpeedButton *EditKeyValBtn;
  TCheckBox *ExpiryCheck;
  TDateTimePicker *ExpiryDatePicker;
  TSpeedButton *ExpiryBtn;
  TPopupMenu *ExpiryMenu;
  TMenuItem *MainMenu_View_Filter_Expired;
  TMenuItem *MainMenu_View_N3;
    TToolButton *AddEntryBtn;
    TMenuItem *MainMenu_File_Properties;
    TMenuItem *MainMenu_File_N5;
  TMenuItem *MainMenu_View_Filter_ExpireSoon;
    TMenuItem *MainMenu_File_OpenReadOnly;
    TMenuItem *MainMenu_File_SetRecoveryPassword;
    TToolButton *SearchBtn;
    TPopupMenu *SearchMenu;
    TMenuItem *SearchMenu_CaseSensitive;
    TMenuItem *SearchMenu_FuzzySearch;
    TMenuItem *SearchMenu_SelectFields;
    TMenuItem *SearchMenu_N1;
    TSpeedButton *PasswQualityBtn;
    TPanel *PasswSecurityBarPanel;
    TImage *PasswSecurityBar;
    TLabel *PasswSecurityLbl;
    TSpeedButton *UrlBtn;
    TImageCollection *ImageCollection32;
    TVirtualImageList *ImageList32;
    TImageCollection *ImageCollection16;
    TVirtualImageList *ImageList16;
    TLabel *PasswChangeLbl;
    TLabel *PasswChangeInfo;
    TSpeedButton *PasswHistoryBtn;
    TMenuItem *MainMenu_View_ResetListFont;
  TMenuItem *MainMenu_View_Filter;
  TMenuItem *MainMenu_View_Filter_WeakPassw;
  TPanel *FilterInfoPanel;
  TSpeedButton *ClearFilterBtn;
  void __fastcall MainMenu_File_NewClick(TObject *Sender);
  void __fastcall DbViewSelectItem(TObject *Sender,
    TListItem *Item, bool Selected);
  void __fastcall AddModifyBtnClick(TObject *Sender);
  void __fastcall PrevBtnClick(TObject *Sender);
  void __fastcall NextBtnClick(TObject *Sender);
  void __fastcall MainMenu_File_SaveAsClick(TObject *Sender);
  void __fastcall MainMenu_File_OpenClick(TObject *Sender);
  void __fastcall MainMenu_File_CloseClick(TObject *Sender);
  void __fastcall CancelBtnClick(TObject *Sender);
  void __fastcall MainMenu_View_ShowColClick(TObject *Sender);
  void __fastcall MainMenu_View_ShowPasswInListClick(
    TObject *Sender);
  void __fastcall TitleBoxChange(TObject *Sender);
  void __fastcall DeleteBtnClick(TObject *Sender);
  void __fastcall MainMenu_View_SortByClick(TObject *Sender);
  void __fastcall DbViewCompare(TObject *Sender, TListItem *Item1,
    TListItem *Item2, int Data, int &Compare);
  void __fastcall MainMenu_View_SortOrderClick(
    TObject *Sender);
  void __fastcall TogglePasswBtnClick(TObject *Sender);
  void __fastcall MainMenu_File_ChangeMasterPasswordClick(
    TObject *Sender);
  void __fastcall MainMenu_File_SaveClick(TObject *Sender);
  void __fastcall TogglePasswBtnMouseUp(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall DbViewMenuPopup(TObject *Sender);
  void __fastcall SearchBoxKeyPress(TObject *Sender, char &Key);
  void __fastcall MainMenu_Edit_CopyUserNameClick(TObject *Sender);
  void __fastcall MainMenu_Edit_CopyPasswClick(TObject *Sender);
  void __fastcall MainMenu_Edit_OpenUrlClick(TObject *Sender);
  void __fastcall MainMenu_Edit_DuplicateEntryClick(TObject *Sender);
  void __fastcall MainMenu_Edit_SelectAllClick(TObject *Sender);
  void __fastcall MainMenu_File_DbSettingsClick(
    TObject *Sender);
  void __fastcall ClearSearchBtnClick(TObject *Sender);
  void __fastcall MainMenu_File_LockClick(TObject *Sender);
  void __fastcall MainMenu_File_ExitClick(TObject *Sender);
  void __fastcall IdleTimerTimer(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall SearchBtnClick(TObject *Sender);
  void __fastcall DbViewColumnClick(TObject *Sender, TListColumn *Column);
  void __fastcall SearchBoxSelect(TObject *Sender);
  void __fastcall MainMenu_File_Export_CsvFileClick(TObject *Sender);
  void __fastcall MainMenu_EditClick(TObject *Sender);
  void __fastcall UserNameLblMouseMove(TObject *Sender, TShiftState Shift, int X,
    int Y);
  void __fastcall PasswLblMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
  void __fastcall MainMenu_View_ChangeListFontClick(TObject *Sender);
  void __fastcall TitleLblMouseMove(TObject *Sender, TShiftState Shift, int X,
    int Y);
  void __fastcall UrlLblMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
  void __fastcall NotesLblMouseMove(TObject *Sender, TShiftState Shift, int X,
    int Y);
  void __fastcall KeywordLblMouseMove(TObject *Sender, TShiftState Shift, int X,
    int Y);
    void __fastcall MainMenu_Edit_PerformAutotypeClick(TObject *Sender);
    void __fastcall DbViewDragOver(TObject *Sender, TObject *Source, int X, int Y,
          TDragState State, bool &Accept);
    void __fastcall DbViewDragDrop(TObject *Sender, TObject *Source, int X, int Y);
    void __fastcall DbViewMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
  void __fastcall MainMenu_Edit_RunClick(TObject *Sender);
  void __fastcall MainMenu_Edit_AddEntryClick(TObject *Sender);
    void __fastcall SearchBoxEnter(TObject *Sender);
    void __fastcall MainMenu_Edit_Rearrange_TopClick(TObject *Sender);
    void __fastcall MainMenu_Edit_Rearrange_UpClick(TObject *Sender);
    void __fastcall MainMenu_Edit_Rearrange_DownClick(TObject *Sender);
    void __fastcall MainMenu_Edit_Rearrange_BottomClick(TObject *Sender);
    void __fastcall MainMenu_View_ChangePasswFontClick(TObject *Sender);
    void __fastcall TagViewSelectItem(TObject *Sender, TListItem *Item, bool Selected);
    void __fastcall TagViewCompare(TObject *Sender, TListItem *Item1, TListItem *Item2,
          int Data, int &Compare);
    void __fastcall AddTagBtnClick(TObject *Sender);
    void __fastcall MainMenu_View_SortTagsByClick(TObject *Sender);
    void __fastcall MainMenu_View_SortTagsOrderClick(TObject *Sender);
  void __fastcall EditKeyValBtnClick(TObject *Sender);
  void __fastcall ExpiryCheckClick(TObject *Sender);
  void __fastcall ExpiryDatePickerChange(TObject *Sender);
  void __fastcall ExpiryBtnClick(TObject *Sender);
  void __fastcall DbViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
    void __fastcall MainMenu_File_PropertiesClick(TObject *Sender);
    void __fastcall MainMenu_File_OpenReadOnlyClick(TObject *Sender);
    void __fastcall MainMenu_File_SetRecoveryPasswordClick(TObject *Sender);
    void __fastcall SearchMenu_SelectFieldsClick(TObject *Sender);
    void __fastcall TitleBoxKeyPress(TObject *Sender, System::WideChar &Key);
    void __fastcall PasswBoxChange(TObject *Sender);
    void __fastcall PasswQualityBtnClick(TObject *Sender);
    void __fastcall EditPanelResize(TObject *Sender);
    void __fastcall PasswSecurityBarPanelMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
    void __fastcall UrlBtnClick(TObject *Sender);
    void __fastcall PasswHistoryBtnClick(TObject *Sender);
    void __fastcall MainMenu_View_ResetListFontClick(TObject *Sender);
    void __fastcall NotesBoxKeyPress(TObject *Sender, System::WideChar &Key);
  void __fastcall MainMenu_View_Filter_ExpiredClick(TObject *Sender);
  void __fastcall ClearFilterBtnClick(TObject *Sender);


private:	// User declarations
  std::shared_ptr<PasswDatabase> m_passwDb;
  std::unique_ptr<TSelectItemThread> m_dbViewSelItemThread;
  std::unique_ptr<TSelectItemThread> m_tagViewSelItemThread;
  WString m_sDbFileName;
  bool m_blDbReadOnly;
  TListItem* m_pSelectedItem;
  int m_nShowColMask;
  int m_nSortByIdx;
  int m_nTagsSortByIdx;
  int m_nSortOrderFactor;
  int m_nTagsSortOrderFactor;
  int m_nSearchMode;
  int m_nLockSelItemIndex;
  int m_nPasswEntropyBits;
  std::vector<int> m_lockSelTags;
  bool m_blItemChanged;
  bool m_blItemPasswChangeConfirm;
  bool m_blDbChanged;
  bool m_blLocked;
  bool m_blUnlockTried;
  //word32 m_lLastUserActionTime;
  TDateTime m_lastUserActionTime;
  int m_nSearchFlags;
  //int m_nNumSearchResults;
  TColor m_defaultListColor;
  std::vector<int> m_listColWidths;
  WString m_uiFieldNames[PasswDbEntry::NUM_FIELDS];
  std::map<std::wstring, std::wstring> m_keyValNames;
  //std::map<SecureWString,word32> m_globalTags, m_searchResultTags;
  std::list<std::pair<SecureWString,word32>> m_tags;
  std::map<SecureWString,SecureWString> m_globalCaseiTags;
  std::set<SecureWString> m_tagFilter;
  std::unique_ptr<PasswDbEntry::KeyValueList> m_tempKeyVal;
  std::unique_ptr<PasswDbEntry::PasswHistory> m_tempPasswHistory;
  IDropTarget* m_pPasswBoxDropTarget;

  void __fastcall LoadConfig(void);
  bool __fastcall OpenDatabase(int nOpenFlags = 0, WString sFileName = WString());
  int __fastcall AskSaveChanges(int nLock = 0);
  bool __fastcall CloseDatabase(bool blForce = false, int nLock = 0);
  void __fastcall LockOrUnlockDatabase(bool blAuto);
  void __fastcall ClearEditPanel(void);
  void __fastcall ClearListView(void);
  bool __fastcall SaveDatabase(const WString& sFileName);
  void __fastcall ResetDbOpenControls(void);
  void __fastcall SetItemChanged(bool blChanged);
  void __fastcall ResetNavControls(void);
  void __fastcall AddModifyListViewEntry(TListItem* pItem = NULL,
    PasswDbEntry* pEntry = NULL);
  void __fastcall ScrollToItem(int nIndex);
  void __fastcall ResetListView(int nFlags = 0);
//  void __fastcall ResetTagView(void);
  void __fastcall SetShowColMask(void);
  void __fastcall ChangeCaption(void);
  void __fastcall SetDbChanged(bool blChanged = true, bool blEntryChanged = false);
  void __fastcall SearchDatabase(WString sStr, int nFlags,
    bool blCaseSensitive, bool blFuzzy);
  const wchar_t* __fastcall DbKeyValNameToKey(const wchar_t* pwszName);
  void __fastcall MoveDbEntries(int nDir);
  void __fastcall OnTagMenuItemClick(TObject* Sender);
  void __fastcall OnExpiryMenuItemClick(TObject* Sender);
  void __fastcall WMSize(TWMSize& msg);
  void __fastcall OnWindowPosChanging(TWMWindowPosChanging& msg);
  SecureMem<word8> __fastcall RequestPasswAndCheck(
    const WString& sRequestMsg,
    const WString& sInvalidMsg,
    std::function<bool(const SecureMem<word8>&)> checkFunc);
  void __fastcall SetRecoveryKeyDependencies(void);
  void __fastcall SetPasswQualityBarWidth(void);
  void __fastcall EstimatePasswQuality(const wchar_t* pwszPassw = nullptr);
  void __fastcall ApplyDbViewItemSelection(TListItem* pItem = nullptr);
  void __fastcall ApplyTagViewItemSelection(void);
  void __fastcall SetListViewSortFlag(void);
  void __fastcall OnQueryEndSession(TWMQueryEndSession& msg);
  void __fastcall ToggleShutdownBlocker(const WString& sMsg = WString());
public:		// User declarations
  __fastcall TPasswMngForm(TComponent* Owner);
  __fastcall ~TPasswMngForm();
  bool __fastcall IsDbOpen(void)
  {
    return m_passwDb && m_passwDb->IsOpen();
  }
  bool __fastcall IsDbOpenOrLocked(void)
  {
    return IsDbOpen() || m_blLocked;
  }
  int __fastcall AddPassw(const wchar_t* pwszPassw, bool blShowNumPassw,
    const wchar_t* pwszParam = NULL);
  void __fastcall SearchDbForKeyword(bool blAutotype);
  void __fastcall NotifyUserAction(void);
  void __fastcall SaveConfig(void);
  bool __fastcall ApplyDbSettings(const PasswDbSettings& settings);
  SecureWString __fastcall BuildTranslKeyValString(
    const PasswDbEntry::KeyValueList& keyValList);
  void __fastcall OnEndSession(TWMEndSession& msg);
  bool __fastcall SaveDbOnShutdown(void);

  BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_SIZE, TWMSize, WMSize);
    MESSAGE_HANDLER(WM_WINDOWPOSCHANGING, TWMWindowPosChanging, OnWindowPosChanging);
    MESSAGE_HANDLER(WM_QUERYENDSESSION, TWMQueryEndSession, OnQueryEndSession);
    MESSAGE_HANDLER(WM_ENDSESSION, TWMEndSession, OnEndSession)
  END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswMngForm *PasswMngForm;
//---------------------------------------------------------------------------
#endif
