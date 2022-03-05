//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include "Main.h"
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("PasswEnter.cpp", PasswEnterDlg);
USEFORM("MPPasswGen.cpp", MPPasswGenForm);
USEFORM("PasswList.cpp", PasswListForm);
USEFORM("PasswMngKeyValEdit.cpp", PasswMngKeyValDlg);
USEFORM("PasswMngDbSettings.cpp", PasswDbSettingsDlg);
USEFORM("PasswMngDbProp.cpp", PasswMngDbPropDlg);
USEFORM("PasswMngColSelect.cpp", PasswMngColDlg);
USEFORM("PasswManager.cpp", PasswMngForm);
USEFORM("Main.cpp", MainForm);
USEFORM("QuickHelp.cpp", QuickHelpForm);
USEFORM("PasswOptions.cpp", PasswOptionsDlg);
USEFORM("ProvideEntropy.cpp", ProvideEntropyDlg);
USEFORM("Progress.cpp", ProgressForm);
USEFORM("ProfileEditor.cpp", ProfileEditDlg);
USEFORM("InfoBox.cpp", InfoBoxForm);
USEFORM("Configuration.cpp", ConfigurationDlg);
USEFORM("About.cpp", AboutForm);
USEFORM("CreateRandDataFile.cpp", CreateRandDataFileDlg);
USEFORM("CreateTrigramFile.cpp", CreateTrigramFileDlg);
//---------------------------------------------------------------------------
HANDLE g_hAppMutex;

int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
  try
  {
    g_hAppMutex = CreateMutex(NULL, true, L"Password Tech Password Generator by C.T.");
    if (g_hAppMutex == NULL)
      RaiseLastOSError();

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      MessageBox(NULL, L"An instance of Password Tech is already running.\n"
        "Please close it before starting a new instance.", L"Password Tech", MB_ICONWARNING);
      return 0;
    }

    Application->Initialize();
    Application->MainFormOnTaskBar = false;

    // Is the program running as console or GUI application
    g_blConsole = AttachConsole(ATTACH_PARENT_PROCESS);

    for (int nI = 1; nI <= ParamCount(); nI++) {
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
        if (nI < ParamCount()) {
          WString sIniFileName = ParamStr(++nI);
          if (ExtractFilePath(sIniFileName).IsEmpty())
            sIniFileName = ExtractFilePath(Application->ExeName) + sIniFileName;
          g_cmdLineOptions.IniFileName = sIniFileName;
        }
      }
      else if (SameText(sParam, CMDLINE_READONLY)) {
        g_cmdLineOptions.ConfigReadOnly = true;
      }
      else if (SameText(sParam, CMDLINE_PROFILE)) {
        if (nI < ParamCount()) {
          g_cmdLineOptions.ProfileName = ParamStr(++nI);
        }
      }
      else if (SameText(sParam, CMDLINE_GENERATE)) {
        int nNum = 1;
        if (nI < ParamCount() && (nNum = StrToIntDef(ParamStr(nI+1), -1)) >= 0)
          nI++;
        g_cmdLineOptions.GenNumPassw = std::max(1, nNum);
        if (g_blConsole)
          Application->ShowMainForm = false;
      }
      else if (SameText(sParam, CMDLINE_SILENT)) {
        Application->ShowMainForm = false;
      }
      else if (SameText(sParam, CMDLINE_OPENDB)) {
        if (nI < ParamCount())
          g_cmdLineOptions.PasswDbFileName = ParamStr(++nI);
      }
      else
        g_cmdLineOptions.UnknownSwitches += "\"" + sParam + "\"; ";
    }

    Application->CreateForm(__classid(TMainForm), &MainForm);
         Application->CreateForm(__classid(TAboutForm), &AboutForm);
         Application->CreateForm(__classid(TProgressForm), &ProgressForm);
         Application->CreateForm(__classid(TConfigurationDlg), &ConfigurationDlg);
         Application->CreateForm(__classid(TCreateRandDataFileDlg), &CreateRandDataFileDlg);
         Application->CreateForm(__classid(TCreateTrigramFileDlg), &CreateTrigramFileDlg);
         Application->CreateForm(__classid(TMPPasswGenForm), &MPPasswGenForm);
         Application->CreateForm(__classid(TPasswEnterDlg), &PasswEnterDlg);
         Application->CreateForm(__classid(TPasswListForm), &PasswListForm);
         Application->CreateForm(__classid(TPasswMngForm), &PasswMngForm);
         Application->CreateForm(__classid(TPasswOptionsDlg), &PasswOptionsDlg);
         Application->CreateForm(__classid(TProfileEditDlg), &ProfileEditDlg);
         Application->CreateForm(__classid(TProvideEntropyDlg), &ProvideEntropyDlg);
         Application->CreateForm(__classid(TQuickHelpForm), &QuickHelpForm);
         Application->CreateForm(__classid(TInfoBoxForm), &InfoBoxForm);
         Application->CreateForm(__classid(TPasswMngColDlg), &PasswMngColDlg);
         Application->CreateForm(__classid(TPasswDbSettingsDlg), &PasswDbSettingsDlg);
         Application->CreateForm(__classid(TPasswMngKeyValDlg), &PasswMngKeyValDlg);
         Application->CreateForm(__classid(TPasswMngDbPropDlg), &PasswMngDbPropDlg);
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
