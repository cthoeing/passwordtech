// UpdateCheck.h
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
#ifndef UpdateCheckH
#define UpdateCheckH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include "UnicodeUtil.h"

class TUpdateCheckThread : public TThread
{
public:

  enum class CheckResult {
    NotAvailable,
    Positive,
    Negative,
    Error,
    Timeout
  };

  __fastcall TUpdateCheckThread(
    TNetHTTPClient* pClient,
    std::function<void(TObject*)> terminateFunc,
    float fTimeout = 3)
    : TThread(false), m_pClient(pClient),
      m_result(CheckResult::NotAvailable), m_terminateFunc(terminateFunc),
      m_fTimeout(fTimeout)
  {
    FreeOnTerminate = true;
    Priority = tpIdle;
    s_blThreadRunning = true;
    OnTerminate = ThreadTerminate;
  }

  __property CheckResult Result =
  { read=m_result };

  static CheckResult __fastcall CheckForUpdates(
    TNetHTTPClient* pClient,
    bool blShowError,
    float fTimeout = 0);

  static bool __fastcall ThreadRunning(void)
  {
    return s_blThreadRunning;
  }

private:
  TNetHTTPClient* m_pClient;
  CheckResult m_result;
  std::function<void(TObject*)> m_terminateFunc;
  float m_fTimeout;
  static std::atomic<bool> s_blThreadRunning;

  virtual void __fastcall Execute(void) override
  {
    m_result = CheckForUpdates(m_pClient, false, m_fTimeout);
    ReturnValue = static_cast<int>(m_result);
  }

  void __fastcall ThreadTerminate(TObject* Sender)
  {
    m_terminateFunc(Sender);
    s_blThreadRunning = false;
  }
};

#endif
