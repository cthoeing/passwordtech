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

const int PROGRESSBAR_MAX_VALUE = 10000;

//---------------------------------------------------------------------------
__fastcall TProgressForm::TProgressForm(TComponent* Owner)
  : TForm(Owner), m_mode(MODE_INACTIVE), m_state(STATE_IDLE)
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

  Top = pCaller->Top + (pCaller->Height - Height) / 2;
  Left = pCaller->Left + (pCaller->Width - Width) / 2;

  if (pCancelFlag) {
    m_mode = MODE_ASYNC;
    if (!pCurProgress)
      m_qMaxValue = 0;
    if (m_qMaxValue) {
      Timer->Interval = 250;
      Timer->Enabled = true;
    }
  }
  else {
    m_mode = MODE_MAIN_THREAD;
    m_state = STATE_RUNNING;
    m_pCaller->Enabled = false;
    if (!m_qMaxValue)
      ProgressLbl->Caption = sProgressInfo;
  }

  if (m_qMaxValue) {
    // Max/Position might be 16-bit integer, so normalize
    // progress to a value <65535
    ProgressBar->Max = PROGRESSBAR_MAX_VALUE;
    ProgressBar->Position = 0;
    ProgressBar->Style = pbstNormal;
  }
  else
    ProgressBar->Style = pbstMarquee;

  Show();

  if (m_mode == MODE_ASYNC)
    UpdateProgress(pCurProgress ? pCurProgress->load() : 0);
}
//---------------------------------------------------------------------------
int __fastcall TProgressForm::ExecuteModal(TForm* pCaller,
  const WString& sTitle,
  const WString& sProgressInfo,
  std::atomic<bool>& cancelFlag,
  std::function<bool(unsigned int)> waitThreadFun,
  word64 qMaxValue,
  const std::atomic<word64>* pCurProgress)
{
  m_pCaller = pCaller;
  m_pCancelFlag = &cancelFlag;
  m_waitThreadFun = waitThreadFun;
  m_qMaxValue = pCurProgress ? qMaxValue : 0;
  m_qLastValue = -1;
  m_pCurrentProgress = pCurProgress;
  m_mode = MODE_ASYNC_MODAL;

  Caption = sTitle;
  if (m_qMaxValue) {
    ProgressBar->Max = PROGRESSBAR_MAX_VALUE;
    ProgressBar->Position = 0;
    ProgressBar->Style = pbstNormal;
    m_sProgressInfo = sProgressInfo;
  }
  else {
    ProgressLbl->Caption = sProgressInfo;
    ProgressBar->Style = pbstMarquee;
  }

  Screen->Cursor = crAppStart;

  Timer->Interval = 50;
  Timer->Enabled = true;

  Top = pCaller->Top + (pCaller->Height - Height) / 2;
  Left = pCaller->Left + (pCaller->Width - Width) / 2;

  UpdateProgress(pCurProgress ? pCurProgress->load() : 0);

  return ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::UpdateProgress(word64 qValue)
{
  if (m_mode != MODE_MAIN_THREAD) {
    std::lock_guard<std::mutex> lock(m_lock);
    if (!m_sProgressMsg.IsEmpty()) {
      if (m_sProgressMsg != ProgressLbl->Caption)
        ProgressLbl->Caption = m_sProgressMsg;
      return;
    }
  }

  if (m_qMaxValue && qValue != m_qLastValue) {
    m_qLastValue = qValue;
    ProgressBar->Position = static_cast<double>(PROGRESSBAR_MAX_VALUE) *
      qValue / m_qMaxValue;
    ProgressLbl->Caption = FormatW(m_sProgressInfo, qValue, m_qMaxValue);
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::MainThreadCallback(word64 qValue,
  const WString& sMsg)
{
  if (m_mode != MODE_MAIN_THREAD)
    throw Exception("ProgressForm::MainThreadCallback(): Incompatible mode");

  if (qValue != -1)
    UpdateProgress(qValue);
  else if (!sMsg.IsEmpty())
    ProgressLbl->Caption = sMsg;

  Application->ProcessMessages();

  if (m_state == STATE_CANCELED) {
    m_state = STATE_RUNNING;
    throw EUserCancel();
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::SetProgressMessageAsync(const WString& sMsg)
{
  if (m_mode == MODE_MAIN_THREAD)
    throw Exception("ProgressForm::SetProgressMessage(): Incompatible mode");

  std::lock_guard<std::mutex> lock(m_lock);
  m_sProgressMsg = sMsg;
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::Terminate(void)
{
  if (Visible)
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::CancelBtnClick(TObject *Sender)
{
  if (m_mode == MODE_ASYNC_MODAL) {
    *m_pCancelFlag = true;
    m_waitThreadFun(-1);
    ModalResult = mrCancel;
  }
  else if (m_pCancelFlag)
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
  TopMostManager::GetInstance().SetForm(this);
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::TimerTimer(TObject *Sender)
{
  if (m_mode == MODE_ASYNC_MODAL) {
    if (m_waitThreadFun(1))
      ModalResult = mrOk;
  }

  UpdateProgress(m_pCurrentProgress ? m_pCurrentProgress->load() : 0);
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  Timer->Enabled = false;
  m_sProgressMsg = WString();
  switch (m_mode) {
  case MODE_MAIN_THREAD:
    m_pCaller->Enabled = true;
    m_state = STATE_IDLE;
    break;
  case MODE_ASYNC_MODAL:
    Screen->Cursor = crDefault;
    break;
  default:
    break;
  }
  m_mode = MODE_INACTIVE;
}
//---------------------------------------------------------------------------
