// UpdateCheck.cpp
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
//#include <stdio.h>
#include <urlmon.h>
#include <wininet.h>
#pragma hdrstop

#include "UpdateCheck.h"
#include "Main.h"
#include "ProgramDef.h"
#include "StringFileStreamW.h"
#include "Language.h"
#include "Util.h"
#include "hrtimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#ifndef _WIN64
#pragma link "urlmon.lib"
#pragma link "wininet.lib"
#endif

std::atomic<bool> TUpdateCheckThread::s_blThreadRunning(false);

//---------------------------------------------------------------------------
TUpdateCheckThread::CheckResult __fastcall TUpdateCheckThread::CheckForUpdates(
  bool blShowError,
  float fTimeoutSec)
{
  try {
    Stopwatch sw;

    const WString sAltUrl = Format("%s?fakeParam=%.8x", ARRAYOFCONST((
      PROGRAM_URL_VERSION, time(nullptr))));

    //WString(PROGRAM_URL_VERSION) +
    //  WString("?fakeParam=") + WString(IntToHex(int(time(NULL)), 8));

    wchar_t wszFileName[MAX_PATH];
    wszFileName[0] = '\0';

    //const wchar_t* pwszUrl = L"http://pwgen-win.sourceforge.net/manual.pdf";
	  const wchar_t* pwszUrl = PROGRAM_URL_VERSION;

    // first try to delete a cache entry of the file before downloading it
    // to ensure that we get the latest version from the server
    if (!DeleteUrlCacheEntry(pwszUrl) && GetLastError() == ERROR_ACCESS_DENIED)
      pwszUrl = sAltUrl.c_str();

	  HRESULT hResult = URLDownloadToCacheFile(nullptr, pwszUrl, wszFileName,
        MAX_PATH, 0, NULL);

    if (fTimeoutSec > 0 && sw.ElapsedSeconds() > fTimeoutSec)
      return CheckResult::Timeout;

    WString sFileName(wszFileName);

    if (hResult != S_OK || sFileName.IsEmpty())
      throw Exception("Could not download version file");

    auto pFile = std::make_unique<TStringFileStreamW>(
      sFileName, fmOpenRead, ceAnsi, true, 1024);

    const int BUFSIZE = 256;
    wchar_t wszBuf[BUFSIZE];
    wszBuf[0] = '\0';

    WString sVersion, sUrl, sNote;
    if (pFile->ReadString(wszBuf, BUFSIZE))
      sVersion = Trim(WString(wszBuf));

    auto version = ParseVersionNumber(sVersion);
    if (version.size() == 3) {
      if (pFile->ReadString(wszBuf, BUFSIZE))
        sUrl = Trim(WString(wszBuf));
      if (pFile->ReadString(wszBuf, BUFSIZE))
        sNote = Trim(WString(wszBuf));
    }
    else
      sVersion = WString();

    if (sVersion.IsEmpty() || sUrl.IsEmpty())
      throw Exception("Version or URL not specified in version file, or unknown "
        "file format");

    if (CompareVersionNumbers(sVersion, PROGRAM_VERSION) > 0) {
      WString sMsg = TRLFormat("A new version (%1) of %2 is available!\nDo you want "
          "to visit the download page now?",
          { sVersion, PROGRAM_NAME });
      if (!sNote.IsEmpty())
        sMsg += WString("\n\n") + TRLFormat("(NOTE: %1)", { sNote });
      TThread::Synchronize(nullptr, [&sMsg,&sUrl]()
        {
          if (MsgBox(sMsg, MB_ICONINFORMATION + MB_YESNO) == IDYES)
            ExecuteShellOp(sUrl, true);
        });
      return CheckResult::Positive;
    }
  }
  catch (Exception& e) {
    if (blShowError) {
      TThread::Synchronize(nullptr, [&e]()
      {
        MsgBox(TRLFormat("Error while checking for updates:\n%1.",
          { e.Message }), MB_ICONERROR);
      });
    }
    return CheckResult::Error;
  }

  return CheckResult::Negative;
}
