// SendKeys.cpp
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
#include <unordered_map>
#include <vcl.h>
#pragma hdrstop

#include "SendKeys.h"
#include "MemUtil.h"
#include "Progress.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

static std::unordered_map<std::string, int> keyPlaceholders;

static const int
  VIRTUAL_KEY_DELAY = 200,
  INIT_DELAY        = 250;

SendKeys::KeySequence::~KeySequence()
{
  for (int nI = 0; nI < keys.size(); nI++) {
    eraseVector(keys[nI]);
    delays[nI] = 0;
  }
}
//---------------------------------------------------------------------------
SendKeys::SendKeys(int nDelay)
  : m_nDelay(nDelay)
{
  if (keyPlaceholders.empty()) {
    for (int nI = 0; nI < PasswDbEntry::NUM_STRING_FIELDS; nI++) {
      AnsiString sFieldName = AnsiString(PasswDbEntry::GetFieldName(
        static_cast<PasswDbEntry::FieldType>(nI))).LowerCase();
      keyPlaceholders[std::string(sFieldName.c_str())] = -nI;
    }

    keyPlaceholders["parameter"] = -PasswDbEntry::USERNAME;

    keyPlaceholders["tab"] = VK_TAB;
    keyPlaceholders["return"] = VK_RETURN;
    keyPlaceholders["enter"] = VK_RETURN;
    keyPlaceholders["ctrl"] = VK_CONTROL;
    keyPlaceholders["backspace"] = VK_BACK;
    keyPlaceholders["clear"] = VK_CLEAR;
    keyPlaceholders["shift"] = VK_SHIFT;
    keyPlaceholders["alt"] = VK_MENU;
    keyPlaceholders["pause"] = VK_PAUSE;
    keyPlaceholders["capslock"] = VK_CAPITAL;
    keyPlaceholders["escape"] = VK_ESCAPE;
    keyPlaceholders["space"] = VK_SPACE;
    keyPlaceholders["pageup"] = VK_PRIOR;
    keyPlaceholders["pagedown"] = VK_NEXT;
    keyPlaceholders["end"] = VK_END;
    keyPlaceholders["home"] = VK_HOME;
    keyPlaceholders["left"] = VK_LEFT;
    keyPlaceholders["right"] = VK_RIGHT;
    keyPlaceholders["down"] = VK_DOWN;
    keyPlaceholders["up"] = VK_UP;
    keyPlaceholders["select"] = VK_SELECT;
    keyPlaceholders["print"] = VK_PRINT;
    keyPlaceholders["execute"] = VK_EXECUTE;
    keyPlaceholders["snapshot"] = VK_SNAPSHOT;
    keyPlaceholders["insert"] = VK_INSERT;
    keyPlaceholders["delete"] = VK_DELETE;
    keyPlaceholders["help"] = VK_HELP;
  }
}
//---------------------------------------------------------------------------
const wchar_t* SendKeys::SendUnicodeChar(const wchar_t* pwszKeyPair,
  std::vector<INPUT>* pDest)
{
  std::vector<INPUT> vi;

  INPUT ip;
  ip.type = INPUT_KEYBOARD;
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;
  ip.ki.wVk = 0;

  if (*pwszKeyPair < 0xd800 || *pwszKeyPair > 0xdbff) {
    // first down, then up
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    ip.ki.wScan = *pwszKeyPair++;
    vi.push_back(ip);
    ip.ki.dwFlags |= KEYEVENTF_KEYUP;
    vi.push_back(ip);
  }
  else {
    // first both parts of the unicode char down
    ip.ki.dwFlags = KEYEVENTF_UNICODE;
    ip.ki.wScan = pwszKeyPair[0];
    vi.push_back(ip);
    ip.ki.wScan = pwszKeyPair[1];
    vi.push_back(ip);

    // then both parts up
    ip.ki.dwFlags |= KEYEVENTF_KEYUP;
    ip.ki.wScan = pwszKeyPair[0];
    vi.push_back(ip);
    ip.ki.wScan = pwszKeyPair[1];
    vi.push_back(ip);

    pwszKeyPair += 2;
  }

  if (pDest != NULL)
    *pDest = vi;
  else
    SendInput(vi.size(), &vi[0], sizeof(INPUT));

  eraseVector(vi);
  memzero(&ip, sizeof(INPUT));

  return pwszKeyPair;
}
//---------------------------------------------------------------------------
void SendKeys::SendString(const wchar_t* pwszStr)
{
  Sleep(INIT_DELAY);
  while (*pwszStr != '\0') {
    pwszStr = SendUnicodeChar(pwszStr);
    Sleep(m_nDelay);
  }
}
//---------------------------------------------------------------------------
void SendKeys::AddString(const wchar_t* pwszStr, KeySequence& dest)
{
  while (*pwszStr != '\0') {
    std::vector<INPUT> vi;
    pwszStr = SendUnicodeChar(pwszStr, &vi);
    dest.keys.push_back(vi);
    dest.delays.push_back(m_nDelay);
    eraseVector(vi);
  }
}
//---------------------------------------------------------------------------
void SendKeys::SendVirtualKey(word16 wKey,
  std::vector<word16>* pAddKeys,
  std::vector<INPUT>* pDest)
{
  std::vector<INPUT> vi;
  if (pAddKeys != NULL) {
    for (std::vector<word16>::iterator it = pAddKeys->begin();
         it != pAddKeys->end(); it++)
      AddVirtualKey(vi, *it, true);
  }
  AddVirtualKey(vi, wKey, true);
  AddVirtualKey(vi, wKey, false);
  if (pAddKeys != NULL) {
    for (std::vector<word16>::iterator it = pAddKeys->begin();
         it != pAddKeys->end(); it++)
      AddVirtualKey(vi, *it, false);
    eraseVector(*pAddKeys);
  }

  if (pDest != NULL)
    *pDest = vi;
  else
    SendInput(vi.size(), &vi[0], sizeof(INPUT));

  eraseVector(vi);
}
//---------------------------------------------------------------------------
void SendKeys::AddVirtualKey(std::vector<INPUT>& dest, word16 wKey, bool blDown)
{
  INPUT ip;
  ip.type = INPUT_KEYBOARD;
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;
  ip.ki.wScan = 0;
  ip.ki.wVk = wKey;
  ip.ki.dwFlags = blDown ? 0 : KEYEVENTF_KEYUP;
  dest.push_back(ip);
  memzero(&ip, sizeof(INPUT));
}
//---------------------------------------------------------------------------
void SendKeys::SendComplexString(const WString& sStr,
  const PasswDbEntry* pPasswDbEntry,
  PasswDatabase* pPasswDb,
  const wchar_t* pwszParam,
  const wchar_t* pwszPassw,
  KeySequence* pDest)
{
  const wchar_t* pwszStr = sStr.c_str();
  std::vector<word16> bufferedKeys;
  std::vector<INPUT> vi;

  while (*pwszStr != '\0') {
    bool blFound = false;

    if (*pwszStr == '{') {
      const wchar_t* pwszEnd = wcschr(pwszStr + 1, '}');
      if (pwszEnd != NULL && pwszEnd != pwszStr + 1) {
        std::string sPlaceholder;
        sPlaceholder.reserve(static_cast<size_t>(pwszEnd - pwszStr) + 1);
        const wchar_t* pwszStart = pwszStr + 1;
        while (*pwszStart != *pwszEnd)
          sPlaceholder.push_back(tolower(*pwszStart++));

        if (sPlaceholder.length() > 5 &&
            sPlaceholder.compare(0, 5, "wait:") == 0)
        {
          blFound = true;
          pwszStr += sPlaceholder.length() + 2;
          try {
            int nTime = std::max(10, std::min(10000, std::stoi(sPlaceholder.substr(5))));
            if (pDest) {
              pDest->keys.push_back(std::vector<INPUT>());
              pDest->delays.push_back(nTime);
            }
            else {
              Sleep(nTime);
            }
          }
          catch (...)
          {}
        }
        else {
          auto it = keyPlaceholders.find(sPlaceholder);
          if (it != keyPlaceholders.end()) {
            blFound = true;
            pwszStr += it->first.length() + 2; // including brackets

            // found placeholder related to password database or MPPG
            if (it->second <= 0) {
              int nIdx = -it->second;

              if (pPasswDbEntry != NULL && pPasswDb != NULL) {
                const SecureWString* psSrc;
                SecureWString sPassw;
                if (nIdx == PasswDbEntry::PASSWORD && !pPasswDbEntry->HasPlaintextPassw()) {
                  sPassw = pPasswDb->GetDbEntryPassw(*pPasswDbEntry);
                  psSrc = &sPassw;
                }
                else
                  psSrc = &pPasswDbEntry->Strings[nIdx];
                if (!psSrc->IsStrEmpty()) {
                  if (pDest != NULL)
                    AddString(psSrc->c_str(), *pDest);
                  else
                    SendString(psSrc->c_str());
                }
              }
              else {
                // "parameter" corresponds to "user name"
                if (nIdx == PasswDbEntry::USERNAME && pwszParam != NULL) {
                  if (pDest != NULL)
                    AddString(pwszParam, *pDest);
                  else
                    SendString(pwszParam);
                }
                else if (nIdx == PasswDbEntry::PASSWORD && pwszPassw != NULL) {
                  if (pDest != NULL)
                    AddString(pwszPassw, *pDest);
                  else
                    SendString(pwszPassw);
                }
              }
            }

            // found placeholder for virtual key
            else {
              word16 wKey = it->second;
              int nDelay = m_nDelay;

              switch (it->second) {
              case VK_CONTROL:
              case VK_MENU:
              case VK_SHIFT:
                if (bufferedKeys.size() < 3)
                  bufferedKeys.push_back(wKey);
                wKey = 0;
                break;

              case VK_TAB:
              case VK_RETURN:
                nDelay = std::max(VIRTUAL_KEY_DELAY, nDelay);
                break;
              }

              if (wKey > 0) {
                if (pDest != NULL) {
                  SendVirtualKey(wKey, &bufferedKeys, &vi);
                  pDest->keys.push_back(vi);
                  if (!pDest->delays.empty())
                    pDest->delays.back() = std::max(VIRTUAL_KEY_DELAY, pDest->delays.back());
                  pDest->delays.push_back(nDelay);
                  eraseVector(vi);
                }
                else {
                  SendVirtualKey(wKey, &bufferedKeys);
                  Sleep(nDelay);
                }
                wKey = 0;
              }
            }
          }
        }
      }
    }

    if (!blFound) {
      if (bufferedKeys.empty())
        pwszStr = SendUnicodeChar(pwszStr, pDest ? &vi : NULL);
      else
        SendVirtualKey(*pwszStr++, &bufferedKeys, pDest ? &vi : NULL);
      if (pDest != NULL) {
        pDest->keys.push_back(vi);
        pDest->delays.push_back(m_nDelay);
        eraseVector(vi);
      }
      else
        Sleep(m_nDelay);
    }
  }

  eraseVector(vi);
  eraseVector(bufferedKeys);
}
//---------------------------------------------------------------------------
void SendKeys::SendKeySequence(KeySequence& input)
{
  Sleep(INIT_DELAY);
  for (int nI = 0; nI < input.keys.size(); nI++) {
    if (!input.keys[nI].empty())
      SendInput(input.keys[nI].size(), &input.keys[nI][0], sizeof(INPUT));
    Sleep(input.delays[nI]);
  }
}
//---------------------------------------------------------------------------
std::atomic<int> TSendKeysThread::s_nThreadState(TSendKeysThread::INACTIVE);

void __fastcall TSendKeysThread::Execute(void)
{
  int nTimeout = 0;
  while (s_nThreadState != ABORTED && nTimeout < 10000) {
    HWND hWin = GetForegroundWindow();
    if (hWin != Application->Handle && hWin != m_hSender &&
        hWin != ProgressForm->Handle) {
      if (!m_sStr.IsEmpty())
        m_sendKeys.SendString(m_sStr.c_str());
      else
        m_sendKeys.SendKeySequence(m_keySeq);
      break;
    }
    Sleep(100);
    nTimeout += 100;
  }
}
//---------------------------------------------------------------------------
void __fastcall TSendKeysThread::TerminateAndWait(int nTimeout)
{
  TSendKeysThread::TerminateActiveThread();
  int nWait = 0;
  while (ThreadRunning() && nWait < nTimeout) {
    Sleep(100);
    nWait += 100;
    Application->ProcessMessages();
  }
}
