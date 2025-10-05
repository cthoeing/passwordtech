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

std::atomic<bool> TUpdateCheckThread::s_blThreadRunning(false);

//---------------------------------------------------------------------------
TUpdateCheckThread::CheckResult __fastcall TUpdateCheckThread::CheckForUpdates(
  TNetHTTPClient* pClient,
  bool blShowError,
  float fTimeout)
{
  try {
    Stopwatch sw;

    auto stream = pClient->Get(PROGRAM_URL_VERSION);

    if (fTimeout > 0 && sw.ElapsedSeconds() > fTimeout) {
      return CheckResult::Timeout;
    }

    if (stream->StatusCode < 200 || stream->StatusCode > 299) {
      throw Exception(TRL("Could not download version file") +
        Format(" (%d %s)", ARRAYOFCONST((stream->StatusCode, stream->StatusText))));
    }

    auto strList = std::make_unique<TStringList>();
    strList->LoadFromStream(stream->ContentStream);

    WString sVersion, sUrl, sNote;
    if (strList->Count >= 2) {
      sVersion = strList->Strings[0];

      auto version = ParseVersionNumber(sVersion);
      if (version.size() == 3) {
        sUrl = strList->Strings[1];
        if (strList->Count >= 3)
          sNote = strList->Strings[2];
      }
      else {
        sVersion = WString();
      }
    }

    if (sVersion.IsEmpty() || sUrl.IsEmpty()) {
      throw Exception(TRL("Version or URL not specified in version file, or unknown "
        "file format"));
    }

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
    return e.Message.Pos("timed out") > 0 ? CheckResult::Timeout :
      CheckResult::Error;
  }

  return CheckResult::Negative;
}
