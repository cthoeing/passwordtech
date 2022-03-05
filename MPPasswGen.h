// MPPasswGen.h
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
#ifndef MPPasswGenH
#define MPPasswGenH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
#include <Buttons.hpp>
#include <jpeg.hpp>
#include <Menus.hpp>
#include "SecureMem.h"

const char MPPG_KEYGEN_SALTSTR[] = "PWGen MP Password Generator";

class TMPPasswGenForm : public TForm
{
__published:	// IDE-managed Components
  TGroupBox *MasterPasswGroup;
  TButton *EnterPasswBtn;
  TCheckBox *ConfirmPasswCheck;
  TCheckBox *ShowPasswHashCheck;
  TCheckBox *KeyExpiryCheck;
  TEdit *KeyExpiryTimeBox;
  TUpDown *KeyExpiryTimeSpinBtn;
  TLabel *PasswStatusLbl;
  TEdit *PasswStatusBox;
  TLabel *PasswExpiryCountdownLbl;
  TButton *ClearKeyBtn;
  TGroupBox *PasswGeneratorGroup;
  TLabel *ParameterLbl;
  TEdit *ParameterBox;
  TLabel *CharSetLbl;
  TComboBox *CharSetList;
  TLabel *LengthLbl;
  TEdit *PasswLengthBox;
  TUpDown *PasswLengthSpinBtn;
  TButton *ClearParameterBtn;
  TLabel *ResultingPasswLbl;
  TEdit *PasswBox;
  TButton *GenerateBtn;
  TLabel *KeyExpiryInfoLbl;
  TButton *UseAsDefaultRNGBtn;
  TButton *CloseBtn;
  TTimer *KeyExpiryTimer;
  TLabel *KeyExpiryCountdownLbl;
  TSpeedButton *TogglePasswBtn;
  TLabel *PasswInfoLbl;
  TPanel *PasswSecurityBarPanel;
  TImage *PasswSecurityBar;
  TCheckBox *HashapassCompatCheck;
  TLabel *CharSetInfoLbl;
  TPopupMenu *PasswBoxMenu;
  TMenuItem *PasswBoxMenu_Undo;
  TMenuItem *PasswBoxMenu_N1;
  TMenuItem *PasswBoxMenu_Cut;
  TMenuItem *PasswBoxMenu_Copy;
  TMenuItem *PasswBoxMenu_EncryptCopy;
  TMenuItem *PasswBoxMenu_Paste;
  TMenuItem *PasswBoxMenu_Delete;
  TMenuItem *PasswBoxMenu_N2;
  TMenuItem *PasswBoxMenu_SelectAll;
  TCheckBox *AddPasswLenToParamCheck;
  TMenuItem *PasswBoxMenu_AddToDatabase;
    TComboBox *PasswHashList;
  TLabel *AutotypeLbl;
  TEdit *AutotypeBox;
  TMenuItem *PasswBoxMenu_PerformAutotype;
  void __fastcall EnterPasswBtnClick(TObject *Sender);
  void __fastcall KeyExpiryTimerTimer(TObject *Sender);
  void __fastcall ClearKeyBtnClick(TObject *Sender);
  void __fastcall GenerateBtnClick(TObject *Sender);
  void __fastcall KeyExpiryCheckClick(TObject *Sender);
  void __fastcall ClearParameterBtnClick(TObject *Sender);
  void __fastcall TogglePasswBtnClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender,
    TCloseAction &Action);
  void __fastcall CloseBtnClick(TObject *Sender);
  void __fastcall UseAsDefaultRNGBtnClick(TObject *Sender);
  void __fastcall HashapassCompatCheckClick(TObject *Sender);
  void __fastcall ParameterBoxKeyPress(TObject *Sender, char &Key);
  void __fastcall FormActivate(TObject *Sender);
  void __fastcall CharSetListChange(TObject *Sender);
  void __fastcall PasswBoxMenuPopup(TObject *Sender);
  void __fastcall PasswBoxMenu_UndoClick(TObject *Sender);
  void __fastcall PasswBoxMenu_CutClick(TObject *Sender);
  void __fastcall PasswBoxMenu_CopyClick(TObject *Sender);
  void __fastcall PasswBoxMenu_EncryptCopyClick(TObject *Sender);
  void __fastcall PasswBoxMenu_PasteClick(TObject *Sender);
  void __fastcall PasswBoxMenu_DeleteClick(TObject *Sender);
  void __fastcall PasswBoxMenu_SelectAllClick(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall PasswSecurityBarMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y);
  void __fastcall ParameterLblMouseMove(TObject *Sender,
    TShiftState Shift, int X, int Y);
  void __fastcall PasswBoxMenu_AddToDatabaseClick(TObject *Sender);
    void __fastcall ShowPasswHashCheckClick(TObject *Sender);
  void __fastcall PasswBoxMenu_PerformAutotypeClick(TObject *Sender);
private:	// User declarations
  SecureMem<word8> m_key;
  int m_nExpiryCountdown;
  IDropTarget* m_pPasswBoxDropTarget;
  IDropTarget* m_pParamBoxDropTarget;

  void __fastcall LoadConfig(void);
  void __fastcall ClearKey(bool blExpired = false,
    bool blClearKeyOnly = false);
  void __fastcall SetKeyExpiry(bool blSetup = false);
public:		// User declarations
  __fastcall TMPPasswGenForm(TComponent* Owner);
  __fastcall ~TMPPasswGenForm();
  void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TMPPasswGenForm *MPPasswGenForm;
//---------------------------------------------------------------------------
#endif
