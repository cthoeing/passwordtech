// Scripting.h
//
// PASSWORD TECH
// Copyright (c) 2002-2024 by Christian Thoeing <c.thoeing@web.de>
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
#ifndef ScriptingH
#define ScriptingH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SyncObjs.hpp>
#include "lua.hpp"
#include "UnicodeUtil.h"
#include "PasswGen.h"

class ELuaError : public Exception
{
public:
  __fastcall ELuaError(const WString& sMsg)
    : Exception(sMsg)
  {}
};

class LuaScript
{
private:
  lua_State* m_L;
  WString m_sFileName;
  int m_nScriptFlags;

  void ThrowLuaError(int);

public:
  enum {
    FLAG_STANDALONE_PASSW_GEN = 1
  };

  LuaScript();
  ~LuaScript();

  void LoadFile(const WString&);

  bool InitGenerate(word64 qNumPassw,
	int nDestType,
	int nGenFlags,
	int nAdvancedFlags,
	int nNumChars,
	int nNumWords,
	const wchar_t* pwszFormatPassw);

  void CallGenerate(word64 qPasswNum,
	const wchar_t* pwszSrcPassw,
	double dSrcPasswSec,
	SecureWString& sDestPassw,
	double& dDestPasswSec);

  static void SetRandomGenerator(RandomGenerator* pSrc);
  static void SetPasswGenerator(PasswordGenerator* pSrc);

  __property WString FileName =
  { read=m_sFileName };

  __property int Flags =
  { read=m_nScriptFlags };
};


class TScriptingThread : public TThread
{
private:
  LuaScript* m_pScript;
  TSimpleEvent* m_pCallEvent;
  TSimpleEvent* m_pResultEvent;
  bool m_blInit;
  word64 m_qInitNumPassw;
  int m_nInitGenFlags;
  int m_nInitDestType;
  int m_nInitNumChars;
  int m_nInitNumWords;
  int m_nInitAdvancedFlags;
  word64 m_qPasswNum;
  //const wchar_t* m_pwszInitFormatPassw;
  WString m_sInitFormatPassw;
  const wchar_t* m_pwszSrcPassw;
  double m_dSrcPasswSec;
  SecureWString m_sResultPassw;
  double m_dResultPasswSec;
  WString m_sErrorMsg;

  void __fastcall Execute(void) override;

public:

  __fastcall TScriptingThread(LuaScript* pScript)
    : TThread(true), m_pScript(pScript), m_blInit(false)
  {
    FreeOnTerminate = false;
    Priority = tpNormal;

    m_pCallEvent = new TSimpleEvent(nullptr, false, false, "", false);
    m_pResultEvent = new TSimpleEvent(nullptr, false, false, "", false);
  }

  __fastcall ~TScriptingThread()
  {
    delete m_pCallEvent;
    delete m_pResultEvent;
    m_dResultPasswSec = 0;
  }

  void __fastcall SetInitParam(word64 qNumPassw,
    int nDestType,
    int nGenFlags,
    int nAdvancedFlags,
    int nNumChars,
    int nNumWords,
    const WString& sInitFormatPassw)
  {
    m_blInit = true;
    m_qInitNumPassw = qNumPassw;
    m_nInitDestType = nDestType;
    m_nInitGenFlags = nGenFlags;
    m_nInitAdvancedFlags = nAdvancedFlags;
    m_nInitNumChars = nNumChars;
    m_nInitNumWords = nNumWords;
    //m_pwszInitFormatPassw = pwszFormatPassw;
    m_sInitFormatPassw = sInitFormatPassw;
  }

  void __fastcall CallGenerate(word64 qPasswNum,
    const wchar_t* pwszPassw, double dPasswSec);

  const SecureWString& __fastcall GetResultPassw(void)
  {
    return m_sResultPassw;
  }

  double __fastcall GetResultPasswSec(void)
  {
    return m_dResultPasswSec;
  }

  __property TEvent* ResultEvent =
  { read=m_pResultEvent };

  __property WString ErrorMessage =
  { read=m_sErrorMsg };
};

#endif
