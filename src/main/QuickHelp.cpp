// QuickHelp.cpp
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
#include <vcl.h>
#pragma hdrstop

#include "QuickHelp.h"
#include "Main.h"
#include "Language.h"
#include "Util.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TQuickHelpForm *QuickHelpForm;

static const WString
CONFIG_ID = "QuickHelp";

//---------------------------------------------------------------------------
__fastcall TQuickHelpForm::TQuickHelpForm(TComponent* Owner)
  : TForm(Owner)
{
  //m_nMinWidth = 2 * GetSystemMetrics(SM_CXSIZEFRAME);
  //m_nMinHeight = GetSystemMetrics(SM_CYCAPTION) + 2 * GetSystemMetrics(SM_CYSIZEFRAME);
  //m_nVScrollX = GetSystemMetrics(SM_CXVSCROLL);

  QuickHelpBox->Color = TColor(GetSysColor(COLOR_INFOBK));

  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(QuickHelpBoxMenu_AutoPosition);
    TRLCaption(QuickHelpBoxMenu_ChangeFont);
  }

  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TQuickHelpForm::LoadConfig(void)
{
  int nTop = g_pIni->ReadInteger(CONFIG_ID, "WindowTop", INT_MAX);
  int nLeft = g_pIni->ReadInteger(CONFIG_ID, "WindowLeft", INT_MAX);
  if (nTop == INT_MAX || nLeft == INT_MAX)
    QuickHelpBoxMenu_AutoPosition->Checked = true;
  else {
    Top = nTop;
    Left = nLeft;
  }
  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
  StringToFont(g_pIni->ReadString(CONFIG_ID, "Font", ""), QuickHelpBox->Font);
  FontDlg->Font = QuickHelpBox->Font;
}
//---------------------------------------------------------------------------
void __fastcall TQuickHelpForm::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop",
    (QuickHelpBoxMenu_AutoPosition->Checked) ? INT_MAX : Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft",
    (QuickHelpBoxMenu_AutoPosition->Checked) ? INT_MAX : Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
  g_pIni->WriteString(CONFIG_ID, "Font", FontToString(QuickHelpBox->Font));
}
//---------------------------------------------------------------------------
WString __fastcall TQuickHelpForm::FormatString(const WString& sSrc)
{
  // replace tabulators ('\t') by spaces to ensure an even appearance
  int nNumTabLines = 0;
  int nMaxColLen = 0;
  int nLen = sSrc.Length();
  int nLineStart = 0;
  const wchar_t* pwszSrc = sSrc.c_str();
  for (int nI = 0; nI < nLen; nI++) {
    switch (pwszSrc[nI]) {
    case '\t':
      nNumTabLines++;
      nMaxColLen = std::max(nMaxColLen, nI - nLineStart);
      break;
    case '\n':
      nLineStart = nI + 1;
    }
  }

  if (nNumTabLines >= 2 && nMaxColLen != 0) {
    std::wstring sText;
    sText.reserve(nLen);
    nLineStart = 0;
    for (int nI = 0; nI < nLen; nI++) {
      switch (pwszSrc[nI]) {
      case '\t':
        sText.append(nMaxColLen - (nI - nLineStart) + 4, ' ');
        break;
      case '\n':
        nLineStart = nI + 1;
      default:
        sText.push_back(pwszSrc[nI]);
      }
    }

    return ConvertCr2Crlf(WString(sText.c_str()));
  }

  return ConvertCr2Crlf(sSrc);
}
//---------------------------------------------------------------------------
void __fastcall TQuickHelpForm::Execute(const WString& sHelp, int nPosX, int nPosY)
{
  if (!Visible && QuickHelpBoxMenu_AutoPosition->Checked) {
    Left = nPosX;
    Top = nPosY;
  }

  Show();
  QuickHelpBox->Text = sHelp;
  QuickHelpBox->SelStart = 0;

  //Refresh();
}
//---------------------------------------------------------------------------
void __fastcall TQuickHelpForm::FormKeyPress(TObject *Sender, char &Key)
{
  if (Key == VK_ESCAPE)
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TQuickHelpForm::QuickHelpBoxMenu_ChangeFontClick(
  TObject *Sender)
{
  TopMostManager::GetInstance().NormalizeTopMosts(this);
  bool blSuccess = FontDlg->Execute();
  TopMostManager::GetInstance().RestoreTopMosts(this);

  if (blSuccess)
    QuickHelpBox->Font = FontDlg->Font;
}
//---------------------------------------------------------------------------

