// Main.h
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
#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <CheckLst.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <jpeg.hpp>
#include <Graphics.hpp>
#include <IniFiles.hpp>
//---------------------------------------------------------------------------
#include <windows.h>
#include <System.ImageList.hpp>
#include <Vcl.BaseImageCollection.hpp>
#include <Vcl.ImageCollection.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.VirtualImageList.hpp>
#include <unordered_set>
#include <atomic>
#include <optional>
#include "RandomPool.h"
#include "PasswGen.h"
#include "PasswOptions.h"
#include "RandomGenerator.h"
#include "Configuration.h"
#include "UpdateCheck.h"
#include "Scripting.h"
#include "EntropyManager.h"

const wchar_t
CMDLINE_INI[]      = L"ini",
CMDLINE_READONLY[] = L"readonly",
CMDLINE_PROFILE[]  = L"profile",
CMDLINE_GENERATE[] = L"gen",
CMDLINE_SILENT[]   = L"silent",
CMDLINE_OPENDB[]   = L"opendb";

struct CmdLineOptions {
  WString IniFileName;
  WString ProfileName;
  WString UnknownSwitches;
  WString PasswDbFileName;
  int GenNumPassw = 0;
  bool ConfigReadOnly = false;
};

struct PWGenProfile {
  WString ProfileName;
  bool IncludeChars;
  int CharsLength;
  WString CharSet;
  bool IncludeWords;
  int WordsNum;
  WString WordListFileName;
  bool CombineWordsChars;
  bool SpecifyLength;
  WString SpecifyLengthString;
  bool FormatPassw;
  WString FormatString;
  bool RunScript;
  WString ScriptFileName;
  WString NumPassw;
  //bool AdvancedOptionsUsed;
  std::optional<PasswOptions> AdvancedPasswOptions;
};

int FindPWGenProfile(const WString& sName);

const int
  PASSW_MAX_BYTES  = 65536,
  PROFILES_MAX_NUM = 50;

const int
  APPSTATE_MINIMIZED  = 0x01,
  APPSTATE_HIDDEN     = 0x02,
  APPSTATE_AUTOTYPE   = 0x04;

extern CmdLineOptions g_cmdLineOptions;
extern std::unique_ptr<TMemIniFile> g_pIni;
extern bool g_blFakeIniFile;
extern std::vector<std::unique_ptr<PWGenProfile>> g_profileList;
extern RandomGenerator* g_pRandSrc;
extern std::unique_ptr<RandomGenerator> g_pKeySeededPRNG;
extern WString g_sExePath;
extern WString g_sAppDataPath;
extern bool g_blConsole;
extern int g_nAppState;
extern int g_nDisplayDlg;
extern Configuration g_config;
extern AnsiString g_asDonorInfo;
extern WString g_sNewline;

enum class TerminateAction {
  None, RestartProgram, SystemShutdown
};
extern TerminateAction g_terminateAction;


bool IsRandomPoolActive(void);

void BeforeDisplayDlg(void);

void AfterDisplayDlg(void);

bool IsDisplayDlg(void);

enum GeneratePasswDest
{ gpdGuiSingle,  // single password in edit control
  gpdGuiList,    // password list in multiline edit control
  gpdClipboardList,  // password list in clipboard
  gpdFileList,   // password list in file
  gpdClipboard,  // copy password to clipboard
  gpdMsgBox,     // show password in message box
  gpdConsole,    // output on console
  gpdAutotype    // autotype in external window
};

class TMainForm : public TForm
{
__published:	// IDE-managed Components
  TPanel *ToolBar;
  TLabel *LogoLbl;
  TGroupBox *PasswGroup;
  TEdit *PasswBox;
  TButton *GenerateBtn;
  TLabel *PasswInfoLbl;
  TGroupBox *SettingsGroup;
  TCheckBox *IncludeCharsCheck;
  TCheckBox *IncludeWordsCheck;
  TLabel *CharsLengthLbl;
  TLabel *WordsNumLbl;
  TLabel *WordListFileLbl;
  TLabel *CharSetLbl;
  TLabel *CharSetInfoLbl;
  TLabel *WordListInfoLbl;
  TSpeedButton *MPPasswGenBtn;
  TOpenDialog *OpenDlg;
  TSpeedButton *ClearClipBtn;
  TSpeedButton *ConfigBtn;
  TSaveDialog *SaveDlg;
  TSpeedButton *CryptTextBtn;
  TComboBox *CharSetList;
  TCheckBox *CombineWordsCharsCheck;
  TPopupMenu *TrayMenu;
  TMenuItem *TrayMenu_Restore;
  TMenuItem *TrayMenu_N1;
  TMenuItem *TrayMenu_GenPassw;
  TMenuItem *TrayMenu_GenAndShowPassw;
  TMenuItem *TrayMenu_N2;
  TMenuItem *TrayMenu_ClearClip;
  TMenuItem *TrayMenu_EncryptClip;
  TMenuItem *TrayMenu_DecryptClip;
  TMenuItem *TrayMenu_CreateRandDataFile;
  TMenuItem *TrayMenu_N3;
  TMenuItem *TrayMenu_OpenManual;
  TMenuItem *TrayMenu_About;
  TMenuItem *TrayMenu_N4;
  TMenuItem *TrayMenu_Exit;
  TTimer *Timer;
  TSpeedButton *HelpBtn;
  TPopupMenu *ListMenu;
  TMenuItem *ListMenu_Cut;
  TMenuItem *ListMenu_Copy;
  TMenuItem *ListMenu_Paste;
  TMenuItem *ListMenu_Delete;
  TMenuItem *ListMenu_N3;
  TMenuItem *ListMenu_ClearList;
  TMenuItem *ListMenu_Undo;
  TMenuItem *ListMenu_N1;
  TMenuItem *ListMenu_SelectAll;
  TMenuItem *ListMenu_N2;
  TMainMenu *MainMenu;
  TMenuItem *MainMenu_File;
  TMenuItem *MainMenu_Tools;
  TMenuItem *MainMenu_Options;
  TMenuItem *MainMenu_Help;
  TMenuItem *MainMenu_File_Exit;
  TMenuItem *MainMenu_Tools_ClearClipboard;
  TMenuItem *MainMenu_Tools_EncryptClip;
  TMenuItem *MainMenu_Tools_DecryptClip;
  TMenuItem *MainMenu_Tools_N2;
  TMenuItem *MainMenu_Tools_CreateRandDataFile;
  TMenuItem *MainMenu_Options_N2;
  TMenuItem *MainMenu_Options_SaveSettingsOnExit;
  TMenuItem *MainMenu_Options_SaveSettingsNow;
  TMenuItem *MainMenu_Help_OpenManual;
  TMenuItem *MainMenu_Help_N1;
    TComboBox *WLFNList;
  TEdit *CharsLengthBox;
  TUpDown *CharsLengthSpinBtn;
  TEdit *WordsNumBox;
  TUpDown *WordsNumSpinBtn;
  TPopupMenu *PasswBoxMenu;
  TMenuItem *PasswBoxMenu_Copy;
  TMenuItem *PasswBoxMenu_SelectAll;
  TMenuItem *PasswBoxMenu_N3;
  TMenuItem *PasswBoxMenu_ChangeFont;
  TMenuItem *PasswBoxMenu_Undo;
  TMenuItem *PasswBoxMenu_N1;
  TMenuItem *PasswBoxMenu_Cut;
  TMenuItem *PasswBoxMenu_Paste;
  TMenuItem *PasswBoxMenu_Delete;
  TMenuItem *PasswBoxMenu_N2;
  TMenuItem *PasswBoxMenu_Editable;
  TFontDialog *FontDlg;
  TSpeedButton *TogglePasswBtn;
  TCheckBox *FormatPasswCheck;
  TComboBox *FormatList;
  TLabel *MultiplePasswLbl;
    TEdit *NumPasswBox;
  TButton *GenerateBtn2;
  TButton *AdvancedBtn;
  TSpeedButton *CharSetInfoBtn;
  TSpeedButton *BrowseBtn;
  TMenuItem *MainMenu_Help_VisitWebsite;
  TMenuItem *MainMenu_Help_Donate;
  TMenuItem *MainMenu_Help_N2;
  TLabel *FormatPasswInfoLbl;
  TSpeedButton *WordListInfoBtn;
  TSpeedButton *CharSetHelpBtn;
  TSpeedButton *FormatPasswHelpBtn;
  TPanel *PasswSecurityBarPanel;
  TImage *PasswSecurityBar;
  TMenuItem *MainMenu_Help_TimerInfo;
  TMenuItem *MainMenu_File_Profile;
  TMenuItem *MainMenu_File_Profile_N1;
  TMenuItem *MainMenu_File_Profile_ProfileEditor;
  TSpeedButton *ProfileEditorBtn;
  TMenuItem *TrayMenu_Profile;
  TMenuItem *TrayMenu_Profile_N1;
  TMenuItem *TrayMenu_Profile_ProfileEditor;
  TMenuItem *MainMenu_File_N1;
  TMenuItem *MainMenu_Tools_N3;
  TMenuItem *MainMenu_Tools_CreateTrigramFile;
  TMenuItem *MainMenu_Help_CheckForUpdates;
  TMenuItem *MainMenu_Help_N3;
  TMenuItem *MainMenu_Help_About;
  TMenuItem *MainMenu_Tools_N1;
  TMenuItem *MainMenu_Tools_MPPasswGen;
  TMenuItem *MainMenu_Tools_DetermRandGen;
  TMenuItem *MainMenu_Tools_DetermRandGen_Reset;
  TMenuItem *MainMenu_Tools_DetermRandGen_Deactivate;
  TMenuItem *TrayMenu_MPPasswGen;
  TMenuItem *PasswBoxMenu_EnablePasswTest;
  TMenuItem *PasswBoxMenu_N5;
  TMenuItem *PasswBoxMenu_EncryptCopy;
  TMenuItem *MainMenu_Options_Config;
  TMenuItem *MainMenu_Tools_N4;
  TMenuItem *MainMenu_Tools_ProvideAddEntropy;
  TBevel *Separator;
  TMenuItem *MainMenu_Options_AlwaysOnTop;
  TMenuItem *MainMenu_Options_N1;
  TMenuItem *MainMenu_Tools_ProvideAddEntropy_AsText;
  TMenuItem *MainMenu_Tools_ProvideAddEntropy_FromFile;
  TEdit *SpecifyLengthBox;
  TCheckBox *SpecifyLengthCheck;
  TMenuItem *PasswBoxMenu_AddToDb;
    TSpeedButton *PasswMngBtn;
  TMenuItem *PasswBoxMenu_SaveAsFile;
  TMenuItem *PasswBoxMenu_N4;
  TMenuItem *ListMenu_RemoveEntry;
  TMenuItem *MainMenu_Options_ClearPasswCache;
  TMenuItem *MainMenu_Help_EnterDonorKey;
  TStatusBar *StatusBar;
  TMenuItem *MainMenu_Options_HideEntProgress;
  TMenuItem *MainMenu_Help_TotalEntBits;
  TMenuItem *MainMenu_Help_N4;
  TCheckBox *RunScriptCheck;
  TComboBox *ScriptList;
  TMenuItem *MainMenu_Tools_DetermRandGen_Setup;
  TSpeedButton *BrowseBtn2;
    TSpeedButton *ReloadScriptBtn;
    TTrayIcon *TrayIcon;
    TLabel *ProfileLbl;
    TComboBox *ProfileList;
    TSpeedButton *ReloadProfileBtn;
    TSpeedButton *AddProfileBtn;
    TMenuItem *PasswBoxMenu_PerformAutotype;
    TMenuItem *TrayMenu_GenAndAutoTypePassw;
  TMenuItem *MainMenu_File_N2;
  TMenuItem *MainMenu_File_PasswManager;
  TMenuItem *TrayMenu_PasswManager;
  TMenuItem *MainMenu_Help_GetTranslations;
    TMenuItem *TrayMenu_ResetWindowPos;
    TMenuItem *TrayMenu_N5;
    TImageCollection *ImageCollection32;
    TVirtualImageList *ImageList32;
    TImageCollection *ImageCollection16;
    TVirtualImageList *ImageList16;
    TPopupMenu *AdvancedOptionsMenu;
    TMenuItem *AdvancedOptionsMenu_DeactivateAll;
    TMenuItem *AdvancedOptionsMenu_DeactivateAllStarred;
  TPopupMenu *GenerateMenu;
  TMenuItem *GenerateMenu_Clipboard;
  TMenuItem *GenerateMenu_File;
  void __fastcall GenerateBtnClick(TObject *Sender);
  void __fastcall IncludeCharsCheckClick(TObject *Sender);
  void __fastcall CharSetInfoBtnClick(TObject *Sender);
  void __fastcall BrowseBtnClick(TObject *Sender);
  void __fastcall ClearClipBtnClick(TObject *Sender);
  void __fastcall MPPasswGenBtnClick(TObject *Sender);
  void __fastcall CryptTextBtnMouseUp(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y);
  void __fastcall CryptTextBtnClick(TObject *Sender);
  void __fastcall EntropyProgressMenu_ResetCountersClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall MainMenu_Options_SaveSettingsNowClick(TObject *Sender);
  void __fastcall MainMenu_Help_AboutClick(TObject *Sender);
  void __fastcall TrayMenu_RestoreClick(TObject *Sender);
  void __fastcall TrayMenu_EncryptClipClick(TObject *Sender);
  void __fastcall TrayMenu_DecryptClipClick(TObject *Sender);
  void __fastcall TrayMenu_ExitClick(TObject *Sender);
  void __fastcall TimerTick(TObject *Sender);
  void __fastcall FormPaint(TObject *Sender);
  void __fastcall HelpBtnClick(TObject *Sender);
  void __fastcall AdvancedBtnClick(TObject *Sender);
  void __fastcall ListMenuPopup(TObject *Sender);
  void __fastcall ListMenu_UndoClick(TObject *Sender);
  void __fastcall ListMenu_CutClick(TObject *Sender);
  void __fastcall ListMenu_CopyClick(TObject *Sender);
  void __fastcall ListMenu_PasteClick(TObject *Sender);
  void __fastcall ListMenu_DeleteClick(TObject *Sender);
  void __fastcall ListMenu_SelectAllClick(TObject *Sender);
  void __fastcall ListMenu_ClearListClick(TObject *Sender);
  void __fastcall CharSetListEnter(TObject *Sender);
  void __fastcall WLFNListEnter(TObject *Sender);
  void __fastcall CharSetListKeyPress(TObject *Sender, char &Key);
  void __fastcall NumPasswBoxKeyPress(TObject *Sender, char &Key);
  void __fastcall PasswBoxKeyPress(TObject *Sender, char &Key);
  void __fastcall CharSetListExit(TObject *Sender);
  void __fastcall PasswBoxMenu_CopyClick(TObject *Sender);
  void __fastcall PasswBoxMenu_SelectAllClick(TObject *Sender);
  void __fastcall PasswBoxMenu_ChangeFontClick(TObject *Sender);
  void __fastcall TogglePasswBtnClick(TObject *Sender);
  void __fastcall CharsLengthBoxExit(TObject *Sender);
  void __fastcall WordsNumBoxExit(TObject *Sender);
  void __fastcall MainMenu_Help_VisitWebsiteClick(TObject *Sender);
  void __fastcall MainMenu_Help_DonateClick(TObject *Sender);
  void __fastcall FormatListExit(TObject *Sender);
  void __fastcall WLFNListExit(TObject *Sender);
  void __fastcall WordListInfoBtnClick(TObject *Sender);
  void __fastcall WLFNListKeyPress(TObject *Sender, char &Key);
  void __fastcall FormatListEnter(TObject *Sender);
  void __fastcall CharSetHelpBtnClick(TObject *Sender);
  void __fastcall FormatPasswHelpBtnClick(TObject *Sender);
  void __fastcall PasswBoxChange(TObject *Sender);
  void __fastcall PasswBoxMenuPopup(TObject *Sender);
  void __fastcall PasswBoxMenu_UndoClick(TObject *Sender);
  void __fastcall PasswBoxMenu_CutClick(TObject *Sender);
  void __fastcall PasswBoxMenu_PasteClick(TObject *Sender);
  void __fastcall PasswBoxMenu_DeleteClick(TObject *Sender);
  void __fastcall PasswBoxMenu_EditableClick(TObject *Sender);
  void __fastcall MainMenu_Help_TimerInfoClick(TObject *Sender);
  void __fastcall ProfileEditorBtnClick(TObject *Sender);
  void __fastcall MainMenu_File_ProfileClick(TObject *Sender);
  void __fastcall TrayMenuPopup(TObject *Sender);
  void __fastcall MainMenu_Tools_CreateTrigramFileClick(
    TObject *Sender);
  void __fastcall MainMenu_Help_CheckForUpdatesClick(
    TObject *Sender);
  void __fastcall MainMenu_Tools_DetermRandGen_ResetClick(
    TObject *Sender);
  void __fastcall MainMenu_Tools_DetermRandGen_DeactivateClick(
    TObject *Sender);
  void __fastcall PasswBoxMenu_EnablePasswTestClick(TObject *Sender);
  void __fastcall MainMenu_File_ExitClick(TObject *Sender);
  void __fastcall PasswBoxMenu_EncryptCopyClick(TObject *Sender);
  void __fastcall MainMenu_Options_ConfigClick(TObject *Sender);
  void __fastcall MainMenu_Tools_CreateRandDataFileClick(
    TObject *Sender);
  void __fastcall FormResize(TObject *Sender);
  void __fastcall MainMenu_Options_AlwaysOnTopClick(TObject *Sender);
  void __fastcall MainMenu_Tools_ProvideAddEntropy_AsTextClick(TObject *Sender);
  void __fastcall MainMenu_Tools_ProvideAddEntropy_FromFileClick(
    TObject *Sender);
  void __fastcall PasswGroupMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y);
  void __fastcall SpecifyLengthCheckClick(TObject *Sender);
  void __fastcall PasswBoxMenu_AddToDbClick(TObject *Sender);
  void __fastcall PasswMngBtnClick(TObject *Sender);
  void __fastcall PasswBoxMenu_SaveAsFileClick(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall ListMenu_RemoveEntryClick(TObject *Sender);
  void __fastcall MainMenu_OptionsClick(TObject *Sender);
  void __fastcall MainMenu_Options_ClearPasswCacheClick(TObject *Sender);
  void __fastcall MainMenu_Help_EnterDonorKeyClick(TObject *Sender);
  void __fastcall MainMenu_Options_HideEntProgressClick(TObject *Sender);
  void __fastcall MainMenu_HelpClick(TObject *Sender);
  void __fastcall MainMenu_Tools_DetermRandGen_SetupClick(TObject *Sender);
  void __fastcall ReloadScriptBtnClick(TObject *Sender);
  void __fastcall BrowseBtn2Click(TObject *Sender);
    void __fastcall TrayIconDblClick(TObject *Sender);
    void __fastcall ProfileListSelect(TObject *Sender);
    void __fastcall AddProfileBtnClick(TObject *Sender);
    void __fastcall PasswBoxMenu_PerformAutotypeClick(TObject *Sender);
    void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
  void __fastcall MainMenu_Help_GetTranslationsClick(TObject *Sender);
    void __fastcall TrayMenu_ResetWindowPosClick(TObject *Sender);
    void __fastcall AdvancedOptionsMenu_DeactivateAllClick(TObject *Sender);
    void __fastcall AdvancedOptionsMenu_DeactivateAllStarredClick(TObject *Sender);

private:	// User declarations
  RandomPool& m_randPool;
  EntropyManager& m_entropyMng;
  PasswordGenerator m_passwGen;
  WString m_sCharSetInput;
  WString m_sCharSetInfo;
  WString m_sWLFileName;
  WString m_sWordListInfo;
  WString m_sWLFileNameErr;
  WString m_sHelpFileName;
  WString m_sRandSeedFileName;
  WString m_sStartupErrors;
  WString m_sCharSetHelp;
  WString m_sFormatPasswHelp;
  int m_nNumStartupErrors;
  int m_nAutoClearClipCnt;
  int m_nAutoClearPasswCnt;
  bool m_blStartup;
  bool m_blCharSetError;
  bool m_blShowEntProgress;
  //bool m_blRestart;
  PasswOptions m_passwOptions;
  IDropTarget* m_pPasswBoxDropTarget;
  TUpdateCheckThread* m_pUpdCheckThread;
  //std::atomic<bool> m_blUpdCheckThreadRunning;
  std::vector<LanguageEntry> m_languages;
  std::vector<HotKeyEntry> m_hotKeys;
  std::unordered_set<std::wstring> m_commonPassw;
  double m_dCommonPasswEntropy;
  std::unique_ptr<LuaScript> m_pScript;
  TDateTime m_lastUpdateCheck;
  AnsiString m_asDonorKey;

  void __fastcall DelayStartupError(const WString& sMsg);
  void __fastcall LoadLangConfig(void);
  bool __fastcall ChangeLanguage(const WString& sLangFileName);
  void __fastcall WriteRandSeedFile(bool blShowError = true);
  void __fastcall LoadConfig(void);
  bool __fastcall SaveConfig(void);
  int  __fastcall LoadCharSet(const WString& sInput,
    bool blShowError = false);
  int  __fastcall LoadWordListFile(const WString& sInput = "",
    bool blShowError = false,
    bool blResetInfoOnly = false);
  int  __fastcall LoadTrigramFile(const WString& sInput = "");
  int  __fastcall AddEntryToList(TComboBox* pComboBox,
    const WString& sEntry,
    bool blCaseSensitive);
  void __fastcall ShowPasswInfo(int nPasswLen, double dPasswBits,
    bool blCommonPassw = false,
    bool blEstimated = false);
  void __fastcall AppMessage(MSG& msg, bool& blHandled);
  void __fastcall AppException(TObject* Sender, Sysutils::Exception* E);
  void __fastcall AppMinimize(TObject* Sender);
  void __fastcall AppRestore(TObject* Sender);
  void __fastcall AppDeactivate(TObject* Sender);
  void __fastcall UpdateProfileControls(void);
  void __fastcall OnHotKey(TMessage& msg);
  void __fastcall ChangeGUIFont(const WString& asFontStr);
  void __fastcall ShowInfoBox(const WString& sInfo);
  void __fastcall SetAdvancedBtnCaption(void);
  void __fastcall OnUpdCheckThreadTerminate(TObject* Sender);
  void __fastcall SetDonorUI(int nDonorType);
  void __fastcall RestoreAction(void);
  void __fastcall OnSetSensitiveClipboardData(void);
  void __fastcall OnQueryEndSession(TWMQueryEndSession& msg);
public:		// User declarations
  __fastcall TMainForm(TComponent* Owner);
  __fastcall ~TMainForm();
  void __fastcall StartupAction(void);
  bool __fastcall ApplyConfig(const Configuration& config);
  void __fastcall UpdateEntropyProgress(bool blForce = false);
  int __fastcall ActivateHotKeys(const HotKeyList& hotKeys);
  void __fastcall DeactivateHotKeys(void);
  void __fastcall CreateProfile(const WString& sProfileName,
    bool blSaveAdvancedOptions,
    int nCreateIdx = -1);
  void __fastcall LoadProfile(int nIndex);
  bool __fastcall LoadProfile(const WString& sName);
  void __fastcall DeleteProfile(int nIndex);
  void __fastcall UseKeySeededPRNG(void);
  void __fastcall CryptText(bool blEncrypt,
    const SecureWString* psText = nullptr,
    TForm* pParentForm = nullptr);
  void __fastcall GeneratePassw(GeneratePasswDest dest,
    TCustomEdit* pEditBox = NULL);
  void __fastcall ShowTrayInfo(const WString& sInfo,
    TBalloonFlags flags = bfNone);
  void __fastcall OnEndSession(TWMEndSession& msg);
  BEGIN_MESSAGE_MAP
    MESSAGE_HANDLER(WM_HOTKEY, TMessage, OnHotKey)
    MESSAGE_HANDLER(WM_QUERYENDSESSION, TWMQueryEndSession, OnQueryEndSession)
    MESSAGE_HANDLER(WM_ENDSESSION, TWMEndSession, OnEndSession)
  END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
