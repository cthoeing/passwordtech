// CharSetBuilder.cpp
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
#include <set>
#pragma hdrstop

#include "CharSetBuilder.h"
#include "UnicodeUtil.h"
#include "Language.h"
#include "Util.h"
#include "Main.h"
#include "TopMostManager.h"
#include "PasswGen.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCharSetBuilderForm *CharSetBuilderForm;

const WString CONFIG_ID = "CharSetBuilder";

const int NUM_PARAM = 6;
TCheckBox* s_charSetUsed[NUM_PARAM];
TUpDown* s_numChars[NUM_PARAM];
TCheckBox* s_atLeast[NUM_PARAM];

//---------------------------------------------------------------------------
__fastcall TCharSetBuilderForm::TCharSetBuilderForm(TComponent* Owner)
  : TForm(Owner)
{
  SetFormComponentsAnchors(this);

  Constraints->MinHeight = Height;
  Constraints->MinWidth = Width;

  s_charSetUsed[0] = LowerCaseCheck;
  s_charSetUsed[1] = UpperCaseCheck;
  s_charSetUsed[2] = DigitsCheck;
  s_charSetUsed[3] = SpecialSymbolsCheck;
  s_charSetUsed[4] = CharactersRangeCheck;
  s_charSetUsed[5] = AdditionalCharsCheck;
  s_numChars[0] = NumSpinBtn1;
  s_numChars[1] = NumSpinBtn2;
  s_numChars[2] = NumSpinBtn3;
  s_numChars[3] = NumSpinBtn4;
  s_numChars[4] = NumSpinBtn5;
  s_numChars[5] = NumSpinBtn6;
  s_atLeast[0] = AtLeastCheck1;
  s_atLeast[1] = AtLeastCheck2;
  s_atLeast[2] = AtLeastCheck3;
  s_atLeast[3] = AtLeastCheck4;
  s_atLeast[4] = AtLeastCheck5;
  s_atLeast[5] = AtLeastCheck6;

  if (g_pLangSupp) {
    TRLCaption(this);
    TRLCaption(CharSetSelGroup);
    TRLCaption(ExcludeCharsGroup);
    TRLCaption(ResultGroup);
    TRLCaption(TagGroup);

    for (int i = 0; i < NUM_PARAM; i++) {
      TRLCaption(s_charSetUsed[i]);
      TRLCaption(s_atLeast[i]);
    }

    TRLCaption(FromLbl);
    TRLCaption(ToLbl);
    TRLCaption(ApplyBtn);
    TRLCaption(ResetBtn);
  }

  LoadConfig();
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::LoadConfig(void)
{
  int nTop = g_pIni->ReadInteger(CONFIG_ID, "WindowTop", INT_MAX);
  int nLeft = g_pIni->ReadInteger(CONFIG_ID, "WindowLeft", INT_MAX);

  if (nTop == INT_MAX || nLeft == INT_MAX)
    Position = poScreenCenter;
  else {
    Top = nTop;
    Left = nLeft;
  }

  Height = g_pIni->ReadInteger(CONFIG_ID, "WindowHeight", Height);
  Width = g_pIni->ReadInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::SaveConfig(void)
{
  g_pIni->WriteInteger(CONFIG_ID, "WindowTop", Top);
  g_pIni->WriteInteger(CONFIG_ID, "WindowLeft", Left);
  g_pIni->WriteInteger(CONFIG_ID, "WindowHeight", Height);
  g_pIni->WriteInteger(CONFIG_ID, "WindowWidth", Width);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::LowerCaseCheckClick(TObject *Sender)
{
  bool blEnabled = LowerCaseCheck->Checked;
  NumBox1->Enabled = blEnabled;
  NumSpinBtn1->Enabled = blEnabled;
  AtLeastCheck1->Enabled = blEnabled;
  CharSetParamChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::UpperCaseCheckClick(TObject *Sender)
{
  bool blEnabled = UpperCaseCheck->Checked;
  NumBox2->Enabled = blEnabled;
  NumSpinBtn2->Enabled = blEnabled;
  AtLeastCheck2->Enabled = blEnabled;
  CharSetParamChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::DigitsCheckClick(TObject *Sender)
{
  bool blEnabled = DigitsCheck->Checked;
  NumBox3->Enabled = blEnabled;
  NumSpinBtn3->Enabled = blEnabled;
  AtLeastCheck3->Enabled = blEnabled;
  CharSetParamChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::SpecialSymbolsCheckClick(TObject *Sender)
{
  bool blEnabled = SpecialSymbolsCheck->Checked;
  NumBox4->Enabled = blEnabled;
  NumSpinBtn4->Enabled = blEnabled;
  AtLeastCheck4->Enabled = blEnabled;
  CharSetParamChange(this);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::CharactersRangeCheckClick(TObject *Sender)
{
  bool blEnabled = CharactersRangeCheck->Checked;
  NumBox5->Enabled = blEnabled;
  NumSpinBtn5->Enabled = blEnabled;
  AtLeastCheck5->Enabled = blEnabled;
  FromLbl->Enabled = blEnabled;
  ToLbl->Enabled = blEnabled;
  FromBox->Enabled = blEnabled;
  ToBox->Enabled = blEnabled;
  CharSetParamChange(this);
  if (blEnabled)
    FromBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::AdditionalCharsCheckClick(TObject *Sender)
{
  bool blEnabled = AdditionalCharsCheck->Checked;
  AdditionalCharsBox->Enabled = blEnabled;
  NumBox6->Enabled = blEnabled;
  NumSpinBtn6->Enabled = blEnabled;
  AtLeastCheck6->Enabled = blEnabled;
  CharSetParamChange(this);
  if (AdditionalCharsCheck->Checked)
    AdditionalCharsBox->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::CharSetParamChange(TObject *Sender)
{
  static const WString CODES[4] = {
    "<az>",
    "<AZ>",
    "<09>",
    "<symbols>"
  };
  static std::set<word32> charsets[4];

  if (charsets[0].empty()) {
    for (int i = 0; i < 26; i++) {
      charsets[0].insert('a' + i);
      charsets[1].insert('A' + i);
      if (i < 10) charsets[2].insert('0' + i);
    }
    charsets[3].insert(std::begin(CHARSET_SYMBOLS), std::end(CHARSET_SYMBOLS)-1);
  }

  WString sResult;
  int nSize = 0;

  w32string sExcludeChars(WStringToW32String(ExcludeCharsBox->Text));
  std::set<word32> exCharset(sExcludeChars.begin(), sExcludeChars.end());
  bool blExclude = !exCharset.empty();

  for (int i = 0; i < NUM_PARAM; i++) {
    if (s_charSetUsed[i]->Checked) {
      switch (i) {
        case 4:
        {
          w32string sFrom = WStringToW32String(FromBox->Text);
          w32string sTo = WStringToW32String(ToBox->Text);
          if (sFrom.length() != 1 || sTo.length() != 1) {
            continue;
          }
          word32 first = sFrom[0];
          word32 last = sTo[0];
          if (last <= first) {
            MsgBox(TRL("Invalid character range."), MB_ICONERROR);
            return;
          }
          if (blExclude) {
            bool blMatch = false;
            for (auto ch = first; ch <= last; ch++) {
              if (exCharset.count(ch)) {
                blMatch = true;
                break;
              }
            }
            if (blMatch) continue;
          }
          sResult += "<" + W32StringToWString(sFrom + sTo) + ">";
          nSize += last - first + 1;
          break;
        }
        case 5:
        {
          w32string sAddChars = WStringToW32String(AdditionalCharsBox->Text);
          if (sAddChars.empty())
            continue;
          std::set<word32> chset(sAddChars.begin(), sAddChars.end());
          if (blExclude) {
            for (auto ch : exCharset) {
              chset.erase(ch);
            }
            if (chset.empty())
              continue;
          }
          nSize += chset.size();
          sResult += "<<" + W32StringToWString(w32string(
            chset.begin(), chset.end())) + ">>";
          break;
        }
        default:
        {
          bool blDefault = true;
          if (blExclude) {
            std::set<word32> chset(charsets[i]);
            for (auto ch : exCharset) {
              chset.erase(ch);
            }
            if (chset.empty())
              continue;
            if (chset.size() < charsets[i].size()) {
              sResult += "<<" + W32StringToWString(w32string(
                chset.begin(), chset.end())) + ">>";
              nSize += chset.size();
              blDefault = false;
            }
          }
          if (blDefault) {
            sResult += CODES[i];
            nSize += charsets[i].size();
          }
        }
      }

      int nChars = s_numChars[i]->Position;
      if (nChars > 0) {
        sResult += ":" + IntToStr(nChars);
        if (s_atLeast[i]->Checked) {
          sResult += "+";
        }
      }
    }
  }

  if (nSize >= 2) {
    WString sTag = TagBox->Text;
    if (!sTag.IsEmpty()) {
      sResult = "[" + sTag + "]" + sResult;
    }
  }
  else
    sResult = WString();

  ResultBox->Text = sResult;
  ApplyBtn->Enabled = !sResult.IsEmpty();
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::ApplyBtnClick(TObject *Sender)
{
  MainForm->CharSetList->Text = ResultBox->Text;
  MainForm->CharSetListExit(this);
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::ResetBtnClick(TObject *Sender)
{
  for (int i = 0; i < NUM_PARAM; i++) {
    s_charSetUsed[i]->Checked = false;
    s_numChars[i]->Position = 0;
    s_atLeast[i]->Checked = false;
  }
  FromBox->Text = WString();
  ToBox->Text = WString();
  AdditionalCharsBox->Text = WString();
}
//---------------------------------------------------------------------------
void __fastcall TCharSetBuilderForm::FormShow(TObject *Sender)
{
  TopMostManager::GetInstance().SetForm(this);
  TagBox->Text = WString();
  CharSetSelGroup->SetFocus();
}
//---------------------------------------------------------------------------

