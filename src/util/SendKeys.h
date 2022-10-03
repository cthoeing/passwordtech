// SendKeys.h
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
#ifndef SendKeysH
#define SendKeysH
//---------------------------------------------------------------------------
#include <vector>
#include <atomic>
#include "PasswDatabase.h"

class SendKeys
{
public:
  struct KeySequence {
    std::vector< std::vector<INPUT> > keys;
    std::vector<int> delays;
    KeySequence() {}
    ~KeySequence();
  };

  SendKeys(int nDelay);

  void SendString(const wchar_t* pwszStr);

  void SendComplexString(const WString& sStr,
    const PasswDbEntry* pPasswDbEntry = NULL,
    PasswDatabase* pPasswDb = NULL,
    const wchar_t* pwszParam = NULL,
    const wchar_t* pwszPassw = NULL,
    KeySequence* pKeySequence = NULL);

  void SendKeySequence(KeySequence& input);

private:

  const wchar_t* SendUnicodeChar(const wchar_t* pwszKeyPair,
    //std::vector<word16>* pAddKeys = NULL,
    std::vector<INPUT>* pDest = NULL);
  void SendVirtualKey(word16 wKey,
    std::vector<word16>* pAddKeys = NULL,
    std::vector<INPUT>* pDest = NULL);
  void AddVirtualKey(std::vector<INPUT>& dest, word16 wKey, bool blDown);
  void AddString(const wchar_t* pwszStr, KeySequence& dest);

  int m_nDelay;
};

class TSendKeysThread : public TThread
{
public:
  __fastcall TSendKeysThread(HWND hSender,
    int nDelay,
    const wchar_t* pwszSrc,
    word32 lLen)
    : m_sendKeys(nDelay), m_hSender(hSender), m_sStr(pwszSrc, lLen + 1)
  {
    FreeOnTerminate = true;
    s_nThreadState = RUNNING;
    OnTerminate = ThreadTerminate;
  }

  __fastcall TSendKeysThread(HWND hSender,
    int nDelay,
    const WString& sStr,
    const PasswDbEntry* pPasswDbEntry = nullptr,
    PasswDatabase* pPasswDb = nullptr,
    const wchar_t* pwszParam = nullptr,
    const wchar_t* pwszPassw = nullptr)
    : m_sendKeys(nDelay), m_hSender(hSender)
  {
    FreeOnTerminate = true;
    s_nThreadState = RUNNING;
    OnTerminate = ThreadTerminate;
    m_sendKeys.SendComplexString(sStr, pPasswDbEntry, pPasswDb,
      pwszParam, pwszPassw, &m_keySeq);
  }

  void __fastcall Execute(void);

  static bool __fastcall ThreadRunning(void)
  {
    return s_nThreadState != INACTIVE;
  }

  static void __fastcall TerminateActiveThread(void)
  {
    if (s_nThreadState == RUNNING)
      s_nThreadState = ABORTED;
  }

  static void __fastcall TerminateAndWait(int nTimeout = 1000);

private:

  enum {
    INACTIVE,
    RUNNING,
    ABORTED
  };

  void __fastcall ThreadTerminate(TObject* Sender)
  {
    s_nThreadState = INACTIVE;
  }

  SecureWString m_sStr;
  SendKeys m_sendKeys;
  HWND m_hSender;
  SendKeys::KeySequence m_keySeq;
  static std::atomic<int> s_nThreadState;
};

#endif
