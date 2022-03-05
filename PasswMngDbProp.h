// PasswMngDbProp.h
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

#ifndef PasswMngDbPropH
#define PasswMngDbPropH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ComCtrls.hpp>
#include "UnicodeUtil.h"

enum class DbProperty : int {
  FileName,
  FilePath,
  FileSize,
  CreationTime,
  ModificationTime,
  FormatVersion,
  NumEntries,
  NumExpiredEntries,
  NumTags
};

//---------------------------------------------------------------------------
class TPasswMngDbPropDlg : public TForm
{
__published:	// IDE-managed Components
    TListView *PropView;
    TButton *CloseBtn;
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall FormShow(TObject *Sender);
private:	// User declarations
    void __fastcall LoadConfig(void);
public:		// User declarations
    __fastcall TPasswMngDbPropDlg(TComponent* Owner);
    void __fastcall SetProperty(DbProperty prop, const WString& sVal);
    void __fastcall SaveConfig(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TPasswMngDbPropDlg *PasswMngDbPropDlg;
//---------------------------------------------------------------------------
#endif
