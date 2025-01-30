// Callback.cpp
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

const WString E_USAGE_CONFLICT = "ProgressForm already used by another process";

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
  std::shared_ptr<std::atomic<bool>> cancelFlag,
  std::shared_ptr<std::atomic<word64>> curProgress)
{
  {
    std::lock_guard<std::mutex> lock(m_lock);

    if (m_pCaller)
      throw Exception(E_USAGE_CONFLICT);

    m_pCaller = pCaller;
    m_sProgressInfo = sProgressInfo;
    m_qMaxValue = qMaxValue;
    m_qLastValue = -1;
    m_currentProgress = curProgress;

    Caption = sTitle;

    Top = pCaller->Top + (pCaller->Height - Height) / 2;
    Left = pCaller->Left + (pCaller->Width - Width) / 2;

    if (cancelFlag) {
      m_mode = MODE_ASYNC;
      m_cancelFlag = cancelFlag;
      if (!curProgress)
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
  }

  if (m_mode == MODE_ASYNC)
    UpdateProgress(curProgress ? curProgress->load() : 0);
}
//---------------------------------------------------------------------------
int __fastcall TProgressForm::ExecuteModal(TForm* pCaller,
  const WString& sTitle,
  const WString& sProgressInfo,
  std::shared_ptr<std::atomic<bool>> cancelFlag,
  std::function<bool(unsigned int)> waitThreadFun,
  word64 qMaxValue,
  std::shared_ptr<std::atomic<word64>> curProgress)
{
  {
    std::lock_guard<std::mutex> lock(m_lock);

    if (m_pCaller)
      throw Exception(E_USAGE_CONFLICT);

    m_pCaller = pCaller;
    //m_pCancelFlag = &cancelFlag;
    m_cancelFlag = cancelFlag;
    m_waitThreadFun = waitThreadFun;
    m_qMaxValue = curProgress ? qMaxValue : 0;
    m_qLastValue = -1;
    m_currentProgress = curProgress;
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
  }

  UpdateProgress(curProgress ? curProgress->load() : 0);

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
    ProgressLbl->Caption = FormatW(m_sProgressInfo,
      { IntToStr(static_cast<__int64>(qValue)),
        IntToStr(static_cast<__int64>(m_qMaxValue)) });
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
void __fastcall TProgressForm::SetProgressMessageAsync(const TForm* pCaller,
  const WString& sMsg)
{
  std::lock_guard<std::mutex> lock(m_lock);

  if (pCaller == m_pCaller) {
    if (m_mode == MODE_MAIN_THREAD)
      throw Exception("ProgressForm::SetProgressMessage(): Incompatible mode");

    m_sProgressMsg = sMsg;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::Terminate(const TForm* pCaller)
{
  if (Visible && m_pCaller == pCaller)
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TProgressForm::CancelBtnClick(TObject *Sender)
{
  switch (m_mode) {
  case MODE_MAIN_THREAD:
    m_state = STATE_CANCELED;
    break;
  case MODE_ASYNC:
    if (auto cf = m_cancelFlag.lock())
      *cf = true;
    else
      Close();
    break;
  case MODE_ASYNC_MODAL:
    if (auto cf = m_cancelFlag.lock())
      *cf = true;
    else
      throw Exception("Cancel token expired before task finished");
    m_waitThreadFun(-1);
    ModalResult = mrCancel;
  default:
    break;
  }
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
  switch (m_mode) {
  case MODE_ASYNC:
    if (m_cancelFlag.expired()) {
      Close();
      return;
    }
    break;
  case MODE_ASYNC_MODAL:
    if (m_waitThreadFun(1))
      ModalResult = mrOk;
    break;
  default:
    break;
  }

  auto progress = m_currentProgress.lock();
  UpdateProgress(progress ? progress->load() : m_qLastValue);
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
  m_pCaller = nullptr;
  m_cancelFlag.reset();
  m_currentProgress.reset();
}
//---------------------------------------------------------------------------
