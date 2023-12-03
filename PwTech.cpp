//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include "Main.h"
#include "ProgramDef.h"
#include "Util.h"
#include "Configuration.h"
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("src\main\PasswMngColSelect.cpp", PasswMngColDlg);
USEFORM("src\main\PasswManager.cpp", PasswMngForm);
USEFORM("src\main\PasswList.cpp", PasswListForm);
USEFORM("src\main\PasswEnter.cpp", PasswEnterDlg);
USEFORM("src\main\MPPasswGen.cpp", MPPasswGenForm);
USEFORM("src\main\PasswMngDbProp.cpp", PasswMngDbPropDlg);
USEFORM("src\main\ProfileEditor.cpp", ProfileEditDlg);
USEFORM("src\main\PasswOptions.cpp", PasswOptionsDlg);
USEFORM("src\main\PasswMngPwHistory.cpp", PasswHistoryDlg);
USEFORM("src\main\PasswMngKeyValEdit.cpp", PasswMngKeyValDlg);
USEFORM("src\main\PasswMngDbSettings.cpp", PasswDbSettingsDlg);
USEFORM("src\main\Main.cpp", MainForm);
USEFORM("src\main\About.cpp", AboutForm);
USEFORM("src\main\InfoBox.cpp", InfoBoxForm);
USEFORM("src\main\CreateTrigramFile.cpp", CreateTrigramFileDlg);
USEFORM("src\main\CreateRandDataFile.cpp", CreateRandDataFileDlg);
USEFORM("src\main\Configuration.cpp", ConfigurationDlg);
USEFORM("src\main\Progress.cpp", ProgressForm);
USEFORM("src\main\QuickHelp.cpp", QuickHelpForm);
USEFORM("src\main\ProvideEntropy.cpp", ProvideEntropyDlg);
//---------------------------------------------------------------------------
HANDLE g_hAppMutex;

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
  try
  {
    g_hAppMutex = CreateMutex(nullptr, true,
      L"Password Tech Password Generator by C.T.");
    if (g_hAppMutex == nullptr)
      RaiseLastOSError();

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      MessageBox(nullptr, L"An instance of Password Tech is already running.\n"
        "Please close it before starting a new instance.",
        PROGRAM_NAME, MB_ICONWARNING);
      return 0;
    }

    g_sExePath = ExtractFilePath(Application->ExeName);

    Application->Initialize();
    Application->MainFormOnTaskBar = false;

    // Is the program running as console or GUI application
    g_blConsole = AttachConsole(ATTACH_PARENT_PROCESS);

    int nParamCount = ParamCount();
    for (int nI = 1; nI <= nParamCount; nI++) {
      WString sParam = ParamStr(nI);
      int nParamLen = sParam.Length();
      int nPrefixLen = 1;

      if (nParamLen < 2 || (sParam[1] != '/' && sParam[1] != '-')) {
        if (g_cmdLineOptions.PasswDbFileName.IsEmpty())
          g_cmdLineOptions.PasswDbFileName = sParam;
        continue;
      }
      if (sParam[2] == '-') {
        if (nParamLen < 3)
          continue;
        nPrefixLen++;
      }

      sParam.Delete(1, nPrefixLen);

      if (SameText(sParam, CMDLINE_INI)) {
        if (nI < nParamCount) {
          WString sIniFileName = ParamStr(++nI);
          if (ExtractFilePath(sIniFileName).IsEmpty())
            sIniFileName = g_sExePath + sIniFileName;
          g_cmdLineOptions.IniFileName = sIniFileName;
        }
      }
      else if (SameText(sParam, CMDLINE_READONLY)) {
        g_cmdLineOptions.ConfigReadOnly = true;
      }
      else if (SameText(sParam, CMDLINE_PROFILE)) {
        if (nI < nParamCount) {
          g_cmdLineOptions.ProfileName = ParamStr(++nI);
        }
      }
      else if (SameText(sParam, CMDLINE_GENERATE)) {
        int nNum = 1;
        if (nI < nParamCount && (nNum = StrToIntDef(ParamStr(nI+1), -1)) >= 0)
          nI++;
        g_cmdLineOptions.GenNumPassw = std::max(1, nNum);
        if (g_blConsole)
          Application->ShowMainForm = false;
      }
      else if (SameText(sParam, CMDLINE_SILENT)) {
        Application->ShowMainForm = false;
      }
      else if (SameText(sParam, CMDLINE_OPENDB)) {
        if (nI < nParamCount)
          g_cmdLineOptions.PasswDbFileName = ParamStr(++nI);
      }
      else
        g_cmdLineOptions.UnknownSwitches += "\"" + sParam + "\"; ";
    }

    WString sIniFileName = g_cmdLineOptions.IniFileName.IsEmpty() ?
      g_sExePath + WString(PROGRAM_INIFILE) : g_cmdLineOptions.IniFileName;

    // load the configuration file
    WString sTryIniFileName = sIniFileName;
    try {
      g_pIni.reset(new TMemIniFile(sIniFileName, TEncoding::UTF8));

      if (g_cmdLineOptions.IniFileName.IsEmpty() &&
          g_pIni->ReadBool("Main", "UseAppDataPath", false) &&
          !(g_sAppDataPath = GetAppDataPath()).IsEmpty())
      {
        g_sAppDataPath += WString(PROGRAM_NAME) + WString("\\");

        if (!g_cmdLineOptions.ConfigReadOnly && !DirectoryExists(g_sAppDataPath))
          CreateDir(g_sAppDataPath);

        sTryIniFileName = g_sAppDataPath + PROGRAM_INIFILE;
        CopyFile(sIniFileName.c_str(), sTryIniFileName.c_str(), true);
        g_pIni.reset(new TMemIniFile(sTryIniFileName, TEncoding::UTF8));
      }
    }
    catch (Exception& e) {
      MsgBox(FormatW("Could not load configuration file\n\"%1\":\n%2",
        { sTryIniFileName, e.Message }), MB_ICONERROR);
      g_pIni.reset(new TMemIniFile("~pwtech~fake~ini"));
      g_blFakeIniFile = true;
    }

    if (g_sAppDataPath.IsEmpty())
      g_sAppDataPath = g_sExePath;

    const WString DEFAULT_STYLE_NAME = "Windows";

	g_config.UiStyleName = DEFAULT_STYLE_NAME;
	WString sStyleName = g_pIni->ReadString("Main", "GUIStyle",
      DEFAULT_STYLE_NAME);
    if (!sStyleName.IsEmpty() && !SameText(sStyleName, DEFAULT_STYLE_NAME))
    {
      // TODO: Remove this when C++ Builder fully supports styles for
      // Windows dialogs
	  //TStyleManager::SystemHooks >>= TStyleManager::TSystemHook::shDialogs;
      if (TStyleManager::TrySetStyle(sStyleName))
        g_config.UiStyleName = sStyleName;
    }

    Application->CreateForm(__classid(TMainForm), &MainForm);
     Application->CreateForm(__classid(TAboutForm), &AboutForm);
     Application->CreateForm(__classid(TConfigurationDlg), &ConfigurationDlg);
     Application->CreateForm(__classid(TCreateRandDataFileDlg), &CreateRandDataFileDlg);
     Application->CreateForm(__classid(TCreateTrigramFileDlg), &CreateTrigramFileDlg);
     Application->CreateForm(__classid(TInfoBoxForm), &InfoBoxForm);
     Application->CreateForm(__classid(TMPPasswGenForm), &MPPasswGenForm);
     Application->CreateForm(__classid(TPasswEnterDlg), &PasswEnterDlg);
     Application->CreateForm(__classid(TPasswListForm), &PasswListForm);
     Application->CreateForm(__classid(TPasswMngForm), &PasswMngForm);
     Application->CreateForm(__classid(TPasswMngColDlg), &PasswMngColDlg);
     Application->CreateForm(__classid(TPasswMngDbPropDlg), &PasswMngDbPropDlg);
     Application->CreateForm(__classid(TPasswDbSettingsDlg), &PasswDbSettingsDlg);
     Application->CreateForm(__classid(TPasswMngKeyValDlg), &PasswMngKeyValDlg);
     Application->CreateForm(__classid(TPasswOptionsDlg), &PasswOptionsDlg);
     Application->CreateForm(__classid(TProfileEditDlg), &ProfileEditDlg);
     Application->CreateForm(__classid(TProgressForm), &ProgressForm);
     Application->CreateForm(__classid(TProvideEntropyDlg), &ProvideEntropyDlg);
     Application->CreateForm(__classid(TQuickHelpForm), &QuickHelpForm);
     Application->CreateForm(__classid(TPasswHistoryDlg), &PasswHistoryDlg);
     MainForm->StartupAction();

         Application->Run();
  }
  catch (Exception &exception)
  {
    Application->ShowException(&exception);
  }
  catch (...)
  {
    try
    {
      throw Exception("");
    }
    catch (Exception &exception)
    {
      Application->ShowException(&exception);
    }
  }
  return 0;
}
//---------------------------------------------------------------------------
