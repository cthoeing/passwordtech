// Util.h
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
#ifndef UtilH
#define UtilH
//---------------------------------------------------------------------------
#include "SecureMem.h"
#include "UnicodeUtil.h"

extern WString g_msgBoxCaptionList[4];

// displays a Windows message box
// -> text to display
// -> message box properties
// <- message box return value
int MsgBox(const WString& sText, int nFlags);

// throws an "out of disk space" error
void OutOfDiskSpaceError(void);

// returns length of the text in an edit box
// -> pointer to edit control
// <- text length
int GetEditBoxTextLen(TCustomEdit* pEdit);

// fills buffer with text in edit box
// -> pointer to edit control
// -> SecureMem object to store the text contents,
//    buffer is resized automatically
void GetEditBoxTextBuf(TCustomEdit* pEdit,
  SecureWString& sDest);

void GetEditBoxSelTextBuf(TCustomEdit* pEdit,
  SecureWString& sDest);

void GetRichEditSelTextBuf(TCustomRichEdit* pEdit,
  SecureWString& sDest);

// sets text contents of edit box
// -> pointer to edit control
// -> pointer to wide string containing text to be copied to the control
void SetEditBoxTextBuf(TCustomEdit* pEdit,
  const wchar_t* pwszSrc);

// clears the text buffer of Windows controls
// -> TCustomEdit control
// -> length of the text to clear (can be 0)
void ClearEditBoxTextBuf(TCustomEdit* pControl,
  int nTextLen = 0);

// gets text contents of clipboard
// -> pointer to secure wide string object, must be specified
//    if Unicode contents are to be retrieved
//    should be NULL otherwise
// -> pointer to secure string (8-bit) object, must be specified
//    if ANSI contents are to be retrieved
//    should be NULL otherwise
// <- 'false' if clipboard doesn't contain any text
// NOTE: function throws an error if clipboard cannot be opened!
bool GetClipboardTextBuf(SecureWString* psDestW,
  SecureAnsiString* psDestA);

// sets text contents of clipboard
// -> pointer to secure wide string object, must be specified
//    if Unicode contents are to be copied
//    should be NULL otherwise
// -> pointer to secure string (8-bit) object, must be specified
//    if ANSI contents are to be copied
//    should be NULL otherwise
// NOTE: function throws an error if clipboard cannot be opened!
void SetClipboardTextBuf(const wchar_t* pwszSrcW,
  const char* pszSrcA);

bool StartEditBoxDragDrop(TCustomEdit* pEdit);

// executes the specified operation using ShellExecute()
// -> file name
// -> show message in case of error?
bool ExecuteShellOp(const WString& sOperation,
  bool blShowErrorMsg = true);

bool ExecuteCommand(const WString& sCommand,
  bool blShowErrorMsg = true);

// compare two version strings (format x.y.z, e.g. 1.23.456)
// -> string S1
// -> string S2
// <- -1 : S1 < S2
//     0 : S1 == S2
//     1 : S1 > S2
int CompareVersions(AnsiString asVer1,
  AnsiString asVer2);

// returns %APPDATA% path by calling the SHGetFolderPath() function
// from the Windows API
WString GetAppDataPath(void);

// converts font object into string
WString FontToString(TFont* pFont);

// converts string into font object
// <- number of attributes which could be successfully parsed (max. 7)
int StringToFont(WString sFont,
  TFont* pFont);

// converts certain types of std::exception into string
WString CppStdExceptionToString(std::exception* e);

// replaces "\n" with "\r\n"
WString ConvertCr2Crlf(const WString& sSrc);

// converts file time into string (short/long date + time)
WString FileTimeToString(FILETIME ft, bool blLongDate);

enum {
  DONOR_KEY_VALID = 0,
  DONOR_KEY_EXPIRED = 1,
  DONOR_KEY_INVALID = 2,

  DONOR_TYPE_STD = 0,
  DONOR_TYPE_PRO = 1,

  DONOR_STD_NUM_UPDATES = 3
};

int CheckDonorKey(const AnsiString& asInput,
  AnsiString* pasId = NULL,
  int* pnType = NULL);

#endif
