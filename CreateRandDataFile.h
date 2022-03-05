// CreateRandDataFile.h
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
#ifndef CreateRandDataFileH
#define CreateRandDataFileH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TCreateRandDataFileDlg : public TForm
{
__published:	// IDE-managed Components
  TComboBox *SizeUnitList;
  TButton *BrowseBtn;
  TButton *CreateFileBtn;
  TButton *CloseBtn;
  TEdit *FileNameBox;
  TEdit *FileSizeBox;
  TLabel *FileNameLbl;
  TLabel *FileSizeLbl;
  void __fastcall CreateFileBtnClick(TObject *Sender);
  void __fastcall BrowseBtnClick(TObject *Sender);
  void __fastcall FileNameBoxChange(TObject *Sender);
  void __fastcall FormShow(TObject *Sender);
  void __fastcall FormActivate(TObject *Sender);
private:	// User declarations
public:		// User declarations
  __fastcall TCreateRandDataFileDlg(TComponent* Owner);
  void __fastcall LoadConfig(void);
  void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TCreateRandDataFileDlg *CreateRandDataFileDlg;
//---------------------------------------------------------------------------
#endif
