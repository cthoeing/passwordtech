// SecureClipboard.cpp
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
  word32 lLen = wcslen(pwszData);
  if (lLen != 0) {
    if (m_blSet)
      ClearData();
    SetClipboardTextBuf(pwszData);
    if (m_blAutoClear) {
      sha256(reinterpret_cast<const word8*>(pwszData),
        std::min(DIGEST_MAX_SRC_LEN, lLen * sizeof(wchar_t)), m_digest, 0);
      m_blSet = true;
    }
    for (auto fun : m_onSetDataFuns)
      fun();
  }
}

void SecureClipboard::ClearData(bool blForce)
{
  if (!m_blSet && !blForce)
    return;

  TClipboard* pClipboard = Clipboard();
  try {
    pClipboard->Open();
    if (pClipboard->HasFormat(CF_UNICODETEXT)) {
      HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_UNICODETEXT);
      if (hText != nullptr) {
        wchar_t* pwszText = reinterpret_cast<wchar_t*>(GlobalLock(hText));
        if (pwszText != nullptr && *pwszText != '\0') {
          word32 lLen = wcslen(pwszText);
          bool blClear = true;
          if (!blForce) {
            SecureMem<word8> checkDigest(32);
            sha256(reinterpret_cast<const word8*>(pwszText),
              std::min(DIGEST_MAX_SRC_LEN, lLen * sizeof(wchar_t)),
              checkDigest, 0);
            blClear = checkDigest == m_digest;
          }
          if (blClear) {
            memzero(pwszText, lLen * sizeof(wchar_t));
            GlobalUnlock(hText);
            pClipboard->Clear();
            if (m_blSet) {
              m_digest.Zeroize();
              m_blSet = false;
            }
          }
          else
            GlobalUnlock(hText);
        }
      }
    }
  }
  catch (...) {}

  pClipboard->Close();
}
