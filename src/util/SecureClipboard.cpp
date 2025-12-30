// SecureClipboard.cpp
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
#include <clipbrd.hpp>
#pragma hdrstop

#include "SecureClipboard.h"
#include "Util.h"
#include "sha256.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

const size_t DIGEST_MAX_SRC_LEN = 1'000'000;

void SecureClipboard::SetData(const wchar_t* pwszData)
{
  size_t len = wcslen(pwszData);
  if (len != 0) {
    if (!m_digest.IsEmpty())
      ClearData();
    SetClipboardTextBuf(pwszData);
    if (m_blAutoClear) {
      m_digest.New(32);
      sha256(reinterpret_cast<const word8*>(pwszData),
        std::min(DIGEST_MAX_SRC_LEN, len * sizeof(wchar_t)), m_digest, 0);
    }
    for (auto fun : m_onSetDataFuns)
      fun();
  }
}

bool SecureClipboard::ClearData(bool blForce, UnicodeString* psErrorMsg,
  int nAttempts, int nWaitTime)
{
  if (m_digest.IsEmpty() && !blForce)
    return true;

  bool blSuccess = false;
  TClipboard* pClipboard = Clipboard();
  for (int a = 0; a < std::max(1, nAttempts); a++) {
    if (a > 0) {
      TThread::Sleep(std::max(1, nWaitTime));
    }
    try {
      pClipboard->Open();
      if (pClipboard->HasFormat(CF_UNICODETEXT)) {
        HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_UNICODETEXT);
        if (hText != nullptr) {
          wchar_t* pwszText = reinterpret_cast<wchar_t*>(GlobalLock(hText));
          if (pwszText != nullptr && *pwszText != '\0') {
            size_t len = wcslen(pwszText);
            bool blClear = true;
            if (!blForce) {
              SecureMem<word8> checkDigest(32);
              sha256(reinterpret_cast<const word8*>(pwszText),
                std::min(DIGEST_MAX_SRC_LEN, len * sizeof(wchar_t)),
                checkDigest, 0);
              blClear = checkDigest == m_digest;
            }
            if (blClear) {
              memzero(pwszText, len * sizeof(wchar_t));
              GlobalUnlock(hText);
              pClipboard->Clear();
            }
            else {
              GlobalUnlock(hText);
            }
          }
        }
      }
      m_digest.Clear();
      blSuccess = true;
    }
    catch (const EClipboardException& e) {
      if (psErrorMsg) {
        *psErrorMsg = e.Message;
      }
      // might be an error due to invalid access, wait a bit and
      // try again, if desired
      continue;
    }
    // for exceptions unrelated to clipboard access, it doesn't make sense
    // to try the same procedure again
    catch (const Exception& e) {
      if (psErrorMsg) {
        *psErrorMsg = e.Message;
      }
    }
    catch (const std::exception& e) {
      if (psErrorMsg) {
        *psErrorMsg = e.what();
      }
    }
    catch (...) {
      if (psErrorMsg) {
        *psErrorMsg = "Unknown error";
      }
    }
    break;
  }

  pClipboard->Close();

  return blSuccess;
}
