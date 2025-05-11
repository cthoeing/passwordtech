// Scripting.cpp
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
#include <vector>
#pragma hdrstop

#include "Scripting.h"
#include "StringFileStreamW.h"
#include "RandomPool.h"
#include "Language.h"
#include "Util.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

static RandomGenerator* s_pRandGen = nullptr;
static PasswordGenerator* s_pPasswGen = nullptr;

const int MAX_PASSW_CHARS = 16000;

const WString
  E_LUA_ERROR    = "Lua error: ",
  E_UNKNOWN_FUNC = "Required function \"%1\" not defined in script";

#define M_RAN_INVM32    2.32830643653869628906e-010
#define M_RAN_INVM52    2.22044604925031308085e-016

// adapted from J.A. Doornik, "Conversion of High-Period Random Numbers to
// Floating Point" (2006)
// https://www.doornik.com/research/randomdouble.pdf
inline double randbl_52new(void)
{
  int iRan1 = s_pRandGen->GetWord32();
  int iRan2 = s_pRandGen->GetWord32();
  return (iRan1 * M_RAN_INVM32 + (0.5 + M_RAN_INVM52 / 2) +
      (iRan2 & 0x000FFFFF) * M_RAN_INVM52);
}

int script_random(lua_State* L)
{
  int nArgs = lua_gettop(L);
  if (nArgs == 0) {
    lua_pushnumber(L, randbl_52new());
    return 1;
  }

  int nStart = 1;
  int nEnd = lua_tointeger(L, 1);

  if (nArgs >= 2) {
    nStart = nEnd;
    nEnd = lua_tointeger(L, 2);
  }

  if (nEnd < nStart)
    return 0;

  int nRand = s_pRandGen->GetNumRange(nEnd - nStart + 1) + nStart;

  lua_pushinteger(L, nRand);
  nRand = 0;

  return 1;
}

SecureAnsiString w32ToUtf8(word32* pSrc)
{
  W32CharToWCharInternal(pSrc);
  return WStringToUtf8_s(reinterpret_cast<wchar_t*>(pSrc));
}

int script_password(lua_State* L)
{
  if (s_pPasswGen == nullptr || lua_gettop(L) == 0)
    return 0;

  int nLength = lua_tointeger(L, 1);
  if (nLength <= 0)
    return 0;

  // returns zero if 2nd argument not specified
  int nFlags = lua_tointeger(L, 2);

  SecureW32String sPassw(std::min<int>(MAX_PASSW_CHARS, nLength) + 1);
  s_pPasswGen->GetPassword(sPassw, nLength, nFlags);

  SecureAnsiString asUtf8 = w32ToUtf8(sPassw);

  lua_pushstring(L, asUtf8);
  lua_pushnumber(L, s_pPasswGen->CustomCharSetEntropy * nLength);
  return 2;
}

int script_passphrase(lua_State* L)
{
  if (s_pPasswGen == nullptr || lua_gettop(L) == 0)
    return 0;

  int nWords = std::min<int>(100, lua_tointeger(L, 1));
  if (nWords <= 0)
    return 0;

  const char* pszChars = lua_tostring(L, 2);
  SecureW32String sInputPassw;
  int nInputPasswLen = 0;
  if (pszChars != nullptr && *pszChars != '\0') {
    SecureWString sUtf16 = Utf8ToWString_s(pszChars);
    nInputPasswLen = GetNumOfUnicodeChars(sUtf16);
    sInputPassw.NewStr(nInputPasswLen);
    WCharToW32Char(sUtf16, sInputPassw);
  }

  int nFlags = lua_tointeger(L, 3);

  SecureW32String sPassphrase(nInputPasswLen + 10 * nWords);
  s_pPasswGen->GetPassphrase(sPassphrase, nWords, sInputPassw,
    nInputPasswLen, nFlags);

  SecureAnsiString asUtf8 = w32ToUtf8(sPassphrase);

  lua_pushstring(L, asUtf8);
  lua_pushnumber(L, s_pPasswGen->WordListEntropy * nWords);
  return 2;
}

int script_phonetic(lua_State* L)
{
  if (s_pPasswGen == nullptr || lua_gettop(L) == 0)
    return 0;

  int nLength = std::min<int>(MAX_PASSW_CHARS, lua_tointeger(L, 1));
  if (nLength <= 0)
    return 0;

  int nFlags = lua_tointeger(L, 2);

  SecureW32String sPassw(nLength + 1);
  s_pPasswGen->GetPhoneticPassw(sPassw, nLength, nFlags);

  SecureAnsiString asUtf8 = w32ToUtf8(sPassw);
  lua_pushstring(L, asUtf8);

  double dPasswSec = s_pPasswGen->PhoneticEntropy;
  dPasswSec = nLength *
    ((nFlags & PASSW_FLAG_PHONETICMIXEDCASE) ? dPasswSec + 1 : dPasswSec);

  lua_pushnumber(L, dPasswSec);

  return 2;
}

int script_word(lua_State* L)
{
  if (s_pPasswGen == nullptr)
    return 0;

  int nIndex = 0;
  if (lua_gettop(L) == 0)
    nIndex = s_pRandGen->GetNumRange(s_pPasswGen->WordListSize);
  else {
    nIndex = lua_tointeger(L, 1) - 1;
    if (nIndex < 0 || nIndex >= s_pPasswGen->WordListSize)
      return 0;
  }

  WString sWord = s_pPasswGen->GetWord(nIndex);
  AnsiString sWordUtf8 = WStringToUtf8(sWord);
  lua_pushstring(L, sWordUtf8.c_str());

  return 1;
}

int script_numwords(lua_State* L)
{
  if (s_pPasswGen == nullptr)
    return 0;

  lua_pushinteger(L, s_pPasswGen->WordListSize);
  return 1;
}

int script_format(lua_State* L)
{
  if (s_pPasswGen == nullptr || lua_gettop(L) == 0)
    return 0;

  const char* pszUtf8 = lua_tostring(L, 1);
  if (pszUtf8 == nullptr || *pszUtf8 == '\0')
    return 0;

  int nFlags = lua_tointeger(L, 2);

  WString sUtf16 = Utf8ToWString(WString(pszUtf8));
  w32string sFormat = WStringToW32String(sUtf16);

  SecureW32String sPassw(MAX_PASSW_CHARS + 1);

  double dPasswSec;

  int nPasswLen = s_pPasswGen->GetFormatPassw(sPassw,
    sFormat, nFlags, nullptr, nullptr, nullptr, &dPasswSec);

  if (nPasswLen == 0)
    return 0;

  SecureAnsiString asUtf8 = w32ToUtf8(sPassw);

  lua_pushstring(L, asUtf8);
  lua_pushnumber(L, dPasswSec);
  return 2;
}

/*static WString s_sLastTraceback;

static int script_traceback(lua_State* L)
{
  lua_getglobal(L, "debug");
  lua_getfield(L, -1, "traceback");
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  s_sLastTraceback = lua_tostring(L, -1);
  lua_pop(L, 1);
  return 1;
}

static void stackDump (lua_State *L) {
  int top = lua_gettop(L);
  WString s;
  for (int i = 1; i <= top; i++) {
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:
        s += lua_tostring(L, i);
        break;

      case LUA_TBOOLEAN:
        s += lua_toboolean(L, i) ? "true" : "false";
        break;

      case LUA_TNUMBER:
        s += IntToStr(static_cast<int>(lua_tonumber(L, i)));
        break;

      default:
        s += lua_typename(L, t);
        break;

    }
    s += "\n";
  }
  s += ";";
}*/

//---------------------------------------------------------------------------
LuaScript::LuaScript()
  : m_nScriptFlags(0)
{
  m_L = luaL_newstate();
  luaL_openlibs(m_L);
  lua_pushcfunction(m_L, script_random);
  lua_setglobal(m_L, "pwtech_random");
  lua_pushcfunction(m_L, script_password);
  lua_setglobal(m_L, "pwtech_password");
  lua_pushcfunction(m_L, script_phonetic);
  lua_setglobal(m_L, "pwtech_phonetic");
  lua_pushcfunction(m_L, script_passphrase);
  lua_setglobal(m_L, "pwtech_passphrase");
  lua_pushcfunction(m_L, script_word);
  lua_setglobal(m_L, "pwtech_word");
  lua_pushcfunction(m_L, script_numwords);
  lua_setglobal(m_L, "pwtech_numwords");
  lua_pushcfunction(m_L, script_format);
  lua_setglobal(m_L, "pwtech_format");

  s_pRandGen = &RandomPool::GetInstance();
}
//---------------------------------------------------------------------------
LuaScript::~LuaScript()
{
  lua_close(m_L);
}
//---------------------------------------------------------------------------
void LuaScript::ThrowLuaError(int nError)
{
  WString sMsg;
  switch (nError)
  {
  case LUA_ERRSYNTAX:
    sMsg = "Syntax error during precompilation";
    break;
  case LUA_ERRMEM:
    OutOfMemoryError();
  case LUA_ERRRUN:
    sMsg = "Runtime error";
    break;
  case LUA_ERRERR:
    sMsg = "Error while running message handler";
    break;
  default:
    sMsg = Format("Unknown error (%d)", ARRAYOFCONST((nError)));
  }
  if (lua_gettop(m_L) >= 1) {
    WString sErr = lua_tostring(m_L, -1);
    lua_pop(m_L, 1);
    if (!sErr.IsEmpty())
      sMsg += "\n" + sErr;
  }
  //if (!s_sLastTraceback.IsEmpty())
  //  sMsg += "\n" + s_sLastTraceback;
  throw ELuaError(E_LUA_ERROR + sMsg);
}
//---------------------------------------------------------------------------
void LuaScript::LoadFile(const WString& sFileName)
{
  std::unique_ptr<TStringFileStreamW> pFile(new TStringFileStreamW(sFileName,
      fmOpenRead, ceAnsi, true, 0));

  if (pFile->Size > 1'485'760)
    throw EStreamError("Script file too large");

  int nChunkLen = pFile->Size - pFile->BOMLength;
  auto buf = std::make_unique<char[]>(nChunkLen);

  pFile->Read(buf.get(), nChunkLen);
  pFile.reset();

  int nResult = luaL_loadbuffer(m_L, buf.get(), nChunkLen,
      AnsiString(ExtractFileName(sFileName)).c_str());
  if (nResult != 0)
    ThrowLuaError(nResult);

  nResult = lua_pcall(m_L, 0, 0, 0);
  if (nResult != 0)
	  ThrowLuaError(nResult);

  if (lua_getglobal(m_L, "script_flags") == LUA_TNUMBER)
	  m_nScriptFlags = lua_tointeger(m_L, -1);
  else
	  m_nScriptFlags = 0;

  lua_pop(m_L, 1);

  m_sFileName = sFileName;
}
//---------------------------------------------------------------------------
bool LuaScript::InitGenerate(word64 qNumPassw,
  int nDestType,
  int nGenFlags,
  int nAdvancedFlags,
  int nNumChars,
  int nNumWords,
  const wchar_t* pwszPasswFormat)
{
  if (lua_getglobal(m_L, "init") != LUA_TFUNCTION) {
    lua_pop(m_L, 1);
    return false;
  }

  lua_pushinteger(m_L, qNumPassw);
  lua_pushinteger(m_L, nDestType);
  lua_pushinteger(m_L, nGenFlags);
  lua_pushinteger(m_L, nAdvancedFlags);
  lua_pushinteger(m_L, nNumChars);
  lua_pushinteger(m_L, nNumWords);

  lua_pushstring(m_L, WStringToUtf8(pwszPasswFormat).c_str());

  int nResult = lua_pcall(m_L, 7, 0, 0);
  if (nResult != 0)
    ThrowLuaError(nResult);

  return true;
}
//---------------------------------------------------------------------------
void LuaScript::CallGenerate(word64 qPasswNum,
  const wchar_t* pwszSrcPassw,
  double dSrcPasswSec,
  SecureWString& sDestPassw,
  double& dDestPasswSec)
{
  if (lua_getglobal(m_L, "generate") != LUA_TFUNCTION) {
    lua_pop(m_L, 1);
    throw ELuaError(E_LUA_ERROR + TRLFormat(E_UNKNOWN_FUNC, { "generate" }));
  }

  int nInputArgs = 0;

  lua_pushinteger(m_L, qPasswNum);
  nInputArgs++;

  if (pwszSrcPassw != nullptr && *pwszSrcPassw != '\0') {
    SecureAnsiString srcPasswUtf8 = WStringToUtf8_s(pwszSrcPassw);
    lua_pushstring(m_L, srcPasswUtf8.c_str());
    nInputArgs++;

    lua_pushnumber(m_L, dSrcPasswSec);
    nInputArgs++;
  }

  int nResult = lua_pcall(m_L, nInputArgs, 2, 0);
  if (nResult != 0)
    ThrowLuaError(nResult);

  //stackDump(m_L);

  const char* luaStr = lua_tostring(m_L, -2);
  sDestPassw = Utf8ToWString_s(luaStr);

  if (sDestPassw.Size() > MAX_PASSW_CHARS + 1 &&
      GetNumOfUnicodeChars(sDestPassw) > MAX_PASSW_CHARS)
  {
    word32 lNewStrLen = MAX_PASSW_CHARS;
    word32 lLastChar = sDestPassw[lNewStrLen - 1];
    //if (static_cast<word32>(sDestPassw[lNewStrLen - 1] - 0xDC00) <= 0x3FF)
    if (lLastChar >= 0xdc00 && lLastChar <= 0xdfff)
      lNewStrLen--;
    sDestPassw[lNewStrLen] = '\0';
    sDestPassw.Shrink(lNewStrLen + 1);
  }

  dDestPasswSec = lua_tonumber(m_L, -1);

  lua_settop(m_L, 0);

  //stackDump(m_L);
}
//---------------------------------------------------------------------------
void LuaScript::SetRandomGenerator(RandomGenerator* pSrc)
{
  s_pRandGen = pSrc ? pSrc : &RandomPool::GetInstance();
}
//---------------------------------------------------------------------------
void LuaScript::SetPasswGenerator(PasswordGenerator* pSrc)
{
  s_pPasswGen = pSrc;
}

//---------------------------------------------------------------------------
void __fastcall TScriptingThread::Execute(void)
{
  const WString sErrorPrefix = "ScriptingThread::Execute(): ";
  try {
    __try {
      while (!Terminated) {
        switch (m_pCallEvent->WaitFor(10)) {
        case wrSignaled:
          //m_pCallEvent->ResetEvent();
          if (m_blInit) {
            m_blInit = false;
            m_pScript->InitGenerate(
              m_qInitNumPassw,
              m_nInitDestType,
              m_nInitGenFlags,
              m_nInitAdvancedFlags,
              m_nInitNumChars,
              m_nInitNumWords,
              m_sInitFormatPassw.c_str());
          }
          m_pScript->CallGenerate(m_qPasswNum, m_pwszSrcPassw, m_dSrcPasswSec,
            m_sResultPassw, m_dResultPasswSec);
          m_pResultEvent->SetEvent();
          break;

        case wrError:
          throw Exception("Wait result error code " +
            IntToStr(m_pCallEvent->LastError));

        case wrAbandoned:
          throw Exception("Event object abandoned while still waiting");

        default:
          break;
        }
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
      DWORD dwError = GetExceptionCode();
      m_sErrorMsg = sErrorPrefix;
      if (dwError == 0)
        m_sErrorMsg += "Synchronization error";
      else
        m_sErrorMsg += "Exception code " + IntToStr(static_cast<int>(dwError));
      m_pResultEvent->SetEvent();
    }
  }
  catch (Exception& e) {
    m_sErrorMsg = sErrorPrefix + e.Message;
    m_pResultEvent->SetEvent();
  }
  catch (std::exception& e) {
    m_sErrorMsg = sErrorPrefix + CppStdExceptionToString(e);
    m_pResultEvent->SetEvent();
  }
  catch (...) {
    m_sErrorMsg = sErrorPrefix + "Unknown error";
    m_pResultEvent->SetEvent();
  }
}
//---------------------------------------------------------------------------
void __fastcall TScriptingThread::CallGenerate(word64 qPasswNum,
  const wchar_t* pwszPassw, double dPasswSec)
{
  m_qPasswNum = qPasswNum;
  m_pwszSrcPassw = pwszPassw;
  m_dSrcPasswSec = dPasswSec;
  //m_pResultEvent->ResetEvent();
  m_pCallEvent->SetEvent();
}
//---------------------------------------------------------------------------
