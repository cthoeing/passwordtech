// UpdateCheck.h
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
#ifndef UpdateCheckH
#define UpdateCheckH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UnicodeUtil.h"

class TUpdateCheckThread : public TThread
{
public:

  enum class CheckResult {
    NotAvailable,
    Positive,
    Negative,
    Error
  };

  __fastcall TUpdateCheckThread(std::function<void(TObject*)> terminateFunc)
    : TThread(false), m_nResult(CheckResult::NotAvailable),
      m_terminateFunc(terminateFunc)
  {
    FreeOnTerminate = true;
    Priority = tpIdle;
    s_blThreadRunning = true;
    OnTerminate = ThreadTerminate;
  }

  __property CheckResult Result =
  { read=m_nResult };

  static CheckResult __fastcall CheckForUpdates(bool blShowError);

  static bool __fastcall ThreadRunning(void)
  {
    return s_blThreadRunning;
  }

private:
  CheckResult m_nResult;
  std::function<void(TObject*)> m_terminateFunc;
  static std::atomic<bool> s_blThreadRunning;

  virtual void __fastcall Execute(void) override
  {
    m_nResult = CheckForUpdates(false);
    ReturnValue = static_cast<int>(m_nResult);
  }

  void __fastcall ThreadTerminate(TObject* Sender)
  {
    m_terminateFunc(Sender);
    s_blThreadRunning = false;
  }
};

#endif
