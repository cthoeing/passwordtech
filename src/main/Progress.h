// Callback.h
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
#ifndef ProgressH
#define ProgressH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
#include "UnicodeUtil.h"
#include <Vcl.ExtCtrls.hpp>

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
private:	// User declarations
  TForm* m_pCaller;
  WString m_sProgressInfo;
  word64 m_qLastValue;
  word64 m_qMaxValue;
  std::atomic<bool>* m_pCancelFlag;
  const std::atomic<word64>* m_pCurrentProgress;

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
    word64 qMaxValue,
    std::atomic<bool>* pCancelFlag = nullptr,
    const std::atomic<word64>* pCurProgress = nullptr);
  void __fastcall MainThreadCallback(word64 qValue);
  void __fastcall MainThreadCallback(const WString& sMsg);
  void __fastcall Terminate(void);
  bool __fastcall IsRunning(void)
  {
    return m_state != STATE_IDLE;
  }
};
//---------------------------------------------------------------------------
extern PACKAGE TProgressForm *ProgressForm;
//---------------------------------------------------------------------------
#endif
