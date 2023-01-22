// Callback.cpp
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
#include <StrUtils.hpp>
#pragma hdrstop

#include "Progress.h"
#include "Language.h"
#include "Main.h"
#include "Util.h"
#include "TopMostManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProgressForm *ProgressForm;

WString EUserCancel::UserCancelMsg;

//---------------------------------------------------------------------------
__fastcall TProgressForm::TProgressForm(TComponent* Owner)
  : TForm(Owner), m_state(STATE_IDLE)
{
  EUserCancel::UserCancelMsg = TRL("Process was cancelled by the user");

  if (g_pLangSupp)
    TRLCaption(CancelBtn);
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::Init(TForm* pCaller,
  const WString& sTitle,
  const WString& sProgressInfo,
  word64 qMaxValue,
  std::atomic<bool>* pCancelFlag,
  const std::atomic<word64>* pCurProgress)
{
  m_pCaller = pCaller;
  m_sProgressInfo = EnableInt64FormatSpec(sProgressInfo);
  m_qMaxValue = qMaxValue;
  m_qLastValue = -1;
  m_pCancelFlag = pCancelFlag;
  m_pCurrentProgress = pCurProgress;

  Caption = sTitle;

  // Max/Position might be 16-bit integer, so normalize
  // progress to a value <65535
  ProgressBar->Max = 10000;
  ProgressBar->Position = 0;

  Top = MainForm->Top + (MainForm->Height - Height) / 2;
  Left = MainForm->Left + (MainForm->Width - Width) / 2;

  if (pCancelFlag)
    Timer->Enabled = true;
  else
    m_pCaller->Enabled = false;

  m_state = STATE_RUNNING;

  Show();
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::UpdateProgress(word64 qValue)
{
  if (qValue != m_qLastValue) {
    m_qLastValue = qValue;
    ProgressBar->Position = floor(10000.0 * qValue / m_qMaxValue);
    ProgressLbl->Caption = FormatW(m_sProgressInfo, qValue, m_qMaxValue);
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::MainThreadCallback(word64 qValue)
{
  UpdateProgress(qValue);

  Application->ProcessMessages();

  if (m_state == STATE_CANCELED) {
    m_state = STATE_RUNNING;
    throw EUserCancel();
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::MainThreadCallback(const WString& sMsg)
{
  ProgressLbl->Caption = sMsg;

  Application->ProcessMessages();

  if (m_state == STATE_CANCELED) {
	m_state = STATE_RUNNING;
	throw EUserCancel();
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::Terminate(void)
{
  if (!Visible) return;

  if (m_pCancelFlag)
    Timer->Enabled = false;
  else
    m_pCaller->Enabled = true;

  m_state = STATE_IDLE;

  Close();
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::CancelBtnClick(TObject *Sender)
{
  if (m_pCancelFlag)
    *m_pCancelFlag = true;
  else
    m_state = STATE_CANCELED;
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::FormKeyDown(TObject *Sender, WORD &Key,
  TShiftState Shift)
{
  if (Key == VK_ESCAPE || (Shift.Contains(ssCtrl) && (Key == 'C' || Key == 'Q'
		|| Key == 'X')))
    CancelBtnClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance()->SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::TimerTimer(TObject *Sender)
{
  UpdateProgress(*m_pCurrentProgress);
}
//---------------------------------------------------------------------------

