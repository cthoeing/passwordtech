// PasswMngPwHistory.h
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

#ifndef PasswMngPwHistoryH
#define PasswMngPwHistoryH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Grids.hpp>
//---------------------------------------------------------------------------
class TPasswHistoryDlg : public TForm
{
__published:	// IDE-managed Components
    TCheckBox *EnableHistoryCheck;
    TEdit *HistorySizeBox;
    TUpDown *HistorySizeSpinBtn;
    TListView *HistoryView;
    TButton *ClearBtn;
    TButton *CopyBtn;
    TButton *OKBtn;
    TButton *CancelBtn;
    void __fastcall ClearBtnClick(TObject *Sender);
    void __fastcall CopyBtnClick(TObject *Sender);
    void __fastcall EnableHistoryCheckClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall HistoryViewKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);


private:	// User declarations
    void __fastcall LoadConfig(void);
public:		// User declarations
    __fastcall TPasswHistoryDlg(TComponent* Owner);
    void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswHistoryDlg *PasswHistoryDlg;
//---------------------------------------------------------------------------
#endif
