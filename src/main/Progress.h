// Callback.h
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
#ifndef ProgressH
#define ProgressH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
#include <Vcl.ExtCtrls.hpp>
#include <functional>
#include <mutex>
#include "UnicodeUtil.h"


class EUserCancel : public Exception
{
public:
  EUserCancel(const WString& sMsg = "")
    : Exception(sMsg.IsEmpty() ? UserCancelMsg : sMsg)
  {
  }

  static WString UserCancelMsg;
};

class TProgressForm : public TForm
{
__published:	// IDE-managed Components
  TProgressBar *ProgressBar;
  TLabel *ProgressLbl;
  TButton *CancelBtn;
    TTimer *Timer;
  void __fastcall CancelBtnClick(TObject *Sender);
  void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
    TShiftState Shift);
  void __fastcall FormShow(TObject *Sender);
    void __fastcall TimerTimer(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
private:	// User declarations
  TForm* m_pCaller;
  WString m_sProgressInfo;
  word64 m_qLastValue;
  word64 m_qMaxValue;
  //std::atomic<bool>* m_pCancelFlag;
  std::weak_ptr<std::atomic<bool>> m_cancelFlag;
  std::weak_ptr<std::atomic<word64>> m_currentProgress;
  //const std::atomic<word64>* m_pCurrentProgress;
  WString m_sProgressMsg;
  std::function<bool(unsigned int)> m_waitThreadFun;
  std::mutex m_lock;

  enum {
    MODE_INACTIVE,
    MODE_MAIN_THREAD,
    MODE_ASYNC,
    MODE_ASYNC_MODAL
  } m_mode;

  enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_CANCELED
  } m_state;

  void __fastcall UpdateProgress(word64 qValue);
public:		// User declarations
  __fastcall TProgressForm(TComponent* Owner);
  void __fastcall Init(TForm* pCaller,
    const WString& sTitle,
    const WString& sProgressInfo,
    word64 qMaxValue = 0,
    std::shared_ptr<std::atomic<bool>> cancelFlag = {},
    std::shared_ptr<std::atomic<word64>> curProgress = {});
  int __fastcall ExecuteModal(TForm* pCaller,
    const WString& sTitle,
    const WString& sProgressInfo,
    std::shared_ptr<std::atomic<bool>> cancelFlag,
    std::function<bool(unsigned int)> waitThreadFun,
    word64 qMaxValue = 0,
    std::shared_ptr<std::atomic<word64>> curProgress = {});
  // this can be called from any thread
  void __fastcall SetProgressMessageAsync(const TForm* pCaller, const WString& sMsg = "");
  // this must be called from main thread only
  void __fastcall SetProgressMessage(const TForm* pCaller, const WString& sMsg)
  {
    if (pCaller == m_pCaller) {
      ProgressLbl->Caption = sMsg;
      Refresh();
    }
  }
  // these must be called from main thread
  void __fastcall MainThreadCallback(word64 qValue = -1, const WString& sMsg = "");
  void __fastcall Terminate(const TForm* pCaller);
};
//---------------------------------------------------------------------------
extern PACKAGE TProgressForm *ProgressForm;
//---------------------------------------------------------------------------
#endif
