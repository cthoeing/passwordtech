// TopMostManager.cpp
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
#pragma hdrstop

#include "TopMostManager.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


//---------------------------------------------------------------------------
TopMostManager& TopMostManager::GetInstance(void)
{
  static TopMostManager inst;
  return inst;
}
//---------------------------------------------------------------------------
bool TopMostManager::SetWindowTopMost(TForm* pForm,
  bool blAlwaysOnTop,
  bool blActivate)
{
  if (!blAlwaysOnTop && !m_formStates[pForm])
    return true;

  int nFlags = SWP_NOMOVE | SWP_NOSIZE;
  if (pForm->Visible)
    nFlags |= SWP_SHOWWINDOW;
  if (!blActivate || !pForm->Visible)
    nFlags |= SWP_NOACTIVATE;

  bool blSuccess = SetWindowPos(pForm->Handle,
      blAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST,
      0, 0, 0, 0, nFlags) != 0;

  if (blSuccess)
    m_formStates[pForm] = blAlwaysOnTop;

  return blSuccess;
}
//---------------------------------------------------------------------------
void TopMostManager::SetAlwaysOnTop(bool blSetting)
{
  if (blSetting != m_blAlwaysOnTop) {
    m_blAlwaysOnTop = blSetting;
    for (int nI = 0; nI < Screen->FormCount; nI++) {
      TForm* pForm = Screen->Forms[nI];
      if (pForm->Visible && pForm->FormStyle != fsStayOnTop)
        SetWindowTopMost(pForm, blSetting);
    }
  }
}
//---------------------------------------------------------------------------
void TopMostManager::SetForm(TForm* pForm)
{
  //if (m_blAlwaysOnTop && pForm != MainForm && !pForm->FormState.Contains(fsModal))
  //  SetWindowTopMost(MainForm, false);
  SetWindowTopMost(pForm, m_blAlwaysOnTop);
  //pForm->FormStyle = m_blAlwaysOnTop ? fsStayOnTop : fsNormal;
}
//---------------------------------------------------------------------------
void TopMostManager::OnFormClose(TForm* pForm)
{
//  if (pForm == MainForm || !m_blAlwaysOnTop || !pForm->Visible)
//    return;
//
//  int nActiveForms = 0;
//  for (int nI = 0; nI < Screen->FormCount; nI++) {
//    if (Screen->Forms[nI] != MainForm && Screen->Forms[nI]->Visible &&
//      ++nActiveForms == 2)
//      break;
//  }
//
//  if (nActiveForms < 2)
//    SetWindowTopMost(MainForm, true);
}
//---------------------------------------------------------------------------
void TopMostManager::OnAppDeactivate(void)
{
//  if (!m_blAlwaysOnTop || IsDisplayDlg())
//    return;
//
//  SetWindowTopMost(MainForm, true, true);
//  if (Screen->ActiveForm != NULL && Screen->ActiveForm->FormState.Contains(fsModal))
//    SetWindowTopMost(Screen->ActiveForm, true, true);
//  else {
//    for (int nI = 0; nI < Screen->FormCount; nI++) {
//      if (Screen->Forms[nI] != MainForm && Screen->Forms[nI]->Visible)
//        SetWindowTopMost(Screen->Forms[nI], true, true);
//    }
//  }
}
//---------------------------------------------------------------------------
void TopMostManager::NormalizeTopMosts(TForm* pForm)
{
  if (!m_blAlwaysOnTop)
    return;

  //if ((m_blMainFormOnTopNorm = m_formStates[MainForm]))
  //  SetWindowTopMost(MainForm, false);
  SetWindowTopMost(pForm, false);
}
//---------------------------------------------------------------------------
void TopMostManager::RestoreTopMosts(TForm* pForm)
{
  if (!m_blAlwaysOnTop)
    return;

  //if (m_blMainFormOnTopNorm)
  //  SetWindowTopMost(MainForm, true);
  SetWindowTopMost(MainForm, true);
  if (pForm != MainForm)
    SetWindowTopMost(pForm, true, true);
}
//---------------------------------------------------------------------------
