// UpdateCheck.h
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
#ifndef UpdateCheckH
#define UpdateCheckH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "UnicodeUtil.h"

class TUpdateCheckThread : public TThread
{
public:

  enum {
    CHECK_POSITIVE,
    CHECK_NEGATIVE,
    CHECK_ERROR
  };

  __fastcall TUpdateCheckThread()
    : TThread(false), m_nResult(-1)
  {
    FreeOnTerminate = true;
    Priority = tpIdle;
  }

  __property int Result =
  { read=m_nResult };

  static int __fastcall CheckForUpdates(bool blShowError);

protected:

  virtual void __fastcall Execute(void)
  {
    ReturnValue = m_nResult = CheckForUpdates(false);
  }


private:
  int m_nResult;
};

#endif
