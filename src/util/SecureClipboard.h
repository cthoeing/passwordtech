// SecureClipboard.h
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
#ifndef SecureClipboardH
#define SecureClipboardH
//---------------------------------------------------------------------------
#include <vector>
#include <functional>
#include "SecureMem.h"

class SecureClipboard
{
public:
  static SecureClipboard& GetInstance()
  {
    static SecureClipboard inst;
    return inst;
  }

  void SetData(const wchar_t* pwszData);
  bool ClearData(bool blForce = false, UnicodeString* psErrorMsg = nullptr,
    int nAttempts = 1, int nWaitTime = 500);
  void RegisterOnSetDataFun(std::function<void(void)> fun)
  {
    m_onSetDataFuns.push_back(fun);
  }

  __property bool AutoClear =
  { read=m_blAutoClear, write=SetAutoClear };

private:
  SecureClipboard() : m_blAutoClear(false) {}
  ~SecureClipboard() {}

  void SetAutoClear(bool blVal)
  {
    m_blAutoClear = blVal;
    if (!m_blAutoClear) {
      m_digest.Clear();
    }
  }

  bool m_blAutoClear;
  SecureMem<word8> m_digest;
  std::vector<std::function<void(void)>> m_onSetDataFuns;
};

#endif
