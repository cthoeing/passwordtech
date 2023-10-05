// TopMostManager.h
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
#ifndef TopMostManagerH
#define TopMostManagerH
//---------------------------------------------------------------------------
#include <map>
#include <Forms.hpp>

class TopMostManager
{
private:
  std::map<TForm*,bool> m_formStates;
  bool m_blAlwaysOnTop;
  //bool m_blMainFormOnTopNorm;

  // set form/window top-most or non-top most
  // -> form
  // -> 'true': set window top-most
  // -> 'true': activate window at the same time
  // <- 'true': success
  bool SetWindowTopMost(TForm* pForm,
    bool blAlwaysOnTop,
    bool blActivate = false);

  // change application-wide "Always on top" setting
  // and apply to all forms that are currently visible
  void SetAlwaysOnTop(bool blSetting);

public:

  TopMostManager()
    : m_blAlwaysOnTop(false)//, m_blMainFormOnTopNorm(false)
  {};

  virtual ~TopMostManager()
  {};

  // get singleton access
  static TopMostManager& GetInstance(void);

  // set form top-most or non-top most, depending on AlwaysOnTop parameter
  void SetForm(TForm* pForm);

  // to be called whenever a non-modal window is closed
  // (no need to call it for modal windows)
  void OnFormClose(TForm* pForm);

  // to be called when the application gets out of focus
  void OnAppDeactivate(void);

  // set specified form and main form non-top most to display
  // open/save dialogs etc.
  void NormalizeTopMosts(TForm* pForm);

  // restore top-most status of specified form and main form after
  // displaying open/save dialogs etc.
  void RestoreTopMosts(TForm* pForm);

  // get access to AlwaysOnTop parameter
  __property bool AlwaysOnTop =
  { read=m_blAlwaysOnTop, write=SetAlwaysOnTop };
};

#endif
