// About.h
//
// PASSWORD TECH
// Copyright (c) 2002-2024 by Christian Thoeing <c.thoeing@web.de>
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
#ifndef AboutH
#define AboutH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <jpeg.hpp>
#include <Vcl.Imaging.pngimage.hpp>
//---------------------------------------------------------------------------
class TAboutForm : public TForm
{
__published:	// IDE-managed Components
  TLabel *LanguageInfoLbl;
  TLabel *LicenseLbl;
  TButton *OKBtn;
  TPanel *Panel1;
  TLabel *ProgramLbl;
  TLabel *VersionLbl;
  TImage *Logo;
  TLabel *DonorLbl;
  TLinkLabel *AuthorLink;
  TLinkLabel *WWWLink;
  TLinkLabel *LicenseLink;
  void __fastcall FormShow(TObject *Sender);
  void __fastcall LinkClick(TObject *Sender, const UnicodeString Link,
          TSysLinkType LinkType);

private:	// User declarations
public:		// User declarations
  __fastcall TAboutForm(TComponent* Owner);
  void __fastcall SetDonorUI(void);
};
//---------------------------------------------------------------------------
extern PACKAGE TAboutForm *AboutForm;
//---------------------------------------------------------------------------
#endif
