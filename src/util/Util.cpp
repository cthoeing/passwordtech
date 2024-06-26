// Util.cpp
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
#include <vcl.h>
//#include <stdio.h>
#include <clipbrd.hpp>
#include <vector>
#include <exception>
#include <stdexcept>
#include <shlobj.h>
#include <StrUtils.hpp>
#pragma hdrstop

#include "Util.h"
#include "MemUtil.h"
#include "hrtimer.h"
#include "Language.h"
#include "Main.h"
#include "sha256.h"
#include "FastPRNG.h"
#include "EntropyManager.h"
#include "dragdrop.h"
#include "TopMostManager.h"
#include "base64.h"
#include "ProgramDef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

WString g_msgBoxCaptionList[4] =
{ "Info", "Warning", "Question", "Error" };

//---------------------------------------------------------------------------
SecureWString strCr2Crlf(const SecureWString& sSrc)
{
  const wchar_t* pwszBuf = sSrc.c_str();

  int nNumCR = 0;
  while ((pwszBuf = wcschr(pwszBuf, '\r')) != nullptr) {
    if (*++pwszBuf == '\n')
      return sSrc;
    nNumCR++;
  }

  if (nNumCR == 0)
    return sSrc;

  int nSize = sSrc.Size();
  SecureWString sDest(nSize + nNumCR);
  for (int nI = 0, nJ = 0; nI < nSize; nI++) {
    sDest[nJ++] = sSrc[nI];
    if (sSrc[nI] == '\r')
      sDest[nJ++] = '\n';
  }

  return sDest;
}
//---------------------------------------------------------------------------
int MsgBox(const WString& sText,
  int nFlags)
{
  int nIcon = nFlags & 0xFF;
  int nListIndex = 0;

  if (nIcon >= MB_ICONINFORMATION)
    nListIndex = 0;
  else if (nIcon >= MB_ICONWARNING)
    nListIndex = 1;
  else if (nIcon >= MB_ICONQUESTION)
    nListIndex = 2;
  else if (nIcon >= MB_ICONERROR)
    nListIndex = 3;

  if (TopMostManager::GetInstance().AlwaysOnTop)
    nFlags |= MB_TOPMOST;

  BeforeDisplayDlg();
  int nResult = Application->MessageBox(sText.c_str(),
      g_msgBoxCaptionList[nListIndex].c_str(), nFlags);
  AfterDisplayDlg();

  // the handle of the message box doesn't belong to our application,
  // so TApplication::OnMessage is not invoked => we have to incorporate
  // the entropy manually here
  // we simply assume a mouse event and assign 1 bit of entropy to it
  TMessage msg;
  msg.Msg = 0;
  msg.WParam = 0;
  msg.LParam = 0;
  msg.Result = nResult;
  EntropyManager::GetInstance().AddEvent(msg, entOther, 1);

  return nResult;
}
//---------------------------------------------------------------------------
void OutOfDiskSpaceError(void)
{
  throw EStreamError(TRL("Out of disk space"));
}
//---------------------------------------------------------------------------
int GetEditBoxTextLen(TCustomEdit* pEdit)
{
  return pEdit->GetTextLen(); //GetWindowTextLengthW(pEdit->Handle);
}
//---------------------------------------------------------------------------
SecureWString GetEditBoxTextBuf(TCustomEdit* pEdit)
{
  int nLen = pEdit->GetTextLen();
  if (nLen == 0)
    return SecureWString();

  SecureWString sDest(++nLen);
  pEdit->GetTextBuf(sDest, nLen);
  return sDest;
}
//---------------------------------------------------------------------------
SecureWString GetEditBoxSelTextBuf(TCustomEdit* pEdit)
{
  int nLen = pEdit->SelLength;
  if (nLen == 0)
    return SecureWString();

  SecureWString sDest(++nLen);
  pEdit->GetSelTextBuf(sDest, nLen);
  return sDest;
}
//---------------------------------------------------------------------------
SecureWString GetRichEditSelTextBuf(TCustomRichEdit* pEdit)
{
  int nLen = pEdit->SelLength;
  if (nLen == 0)
    return SecureWString();

  SecureWString sDest(++nLen);
  pEdit->GetSelTextBuf(sDest, nLen);
  return strCr2Crlf(sDest);
}
//---------------------------------------------------------------------------
void SetEditBoxTextBuf(TCustomEdit* pEdit,
  const wchar_t* pwszSrc)
{
  const wchar_t* pwszBuf = (pwszSrc != nullptr) ? pwszSrc : L"";

  SetWindowText(pEdit->Handle, pwszBuf);
  pEdit->Perform(CM_TEXTCHANGED, 0, static_cast<int>(0));
}
//---------------------------------------------------------------------------
void ClearEditBoxTextBuf(TCustomEdit* pEdit,
  int nTextLen)
{
  // OK, this is an important function: it overwrites and eventually 'clears'
  // the text buffer of the Windows control. Yes, this solution is
  // somewhat lousy, but unfortunately we cannot access the buffer directly
  // -- no way via VCL.

  // this character table contains 34 space characters to accelerate the
  // formatting done by Windows; if the text doesn't contain any spaces,
  // the formatting becomes quite slow...
  // the probability of a space to occur is ~27%.
  static const char CHARTABLE128[129] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
    "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~                                  ";

  nTextLen = std::min(1'000'000, std::max(GetEditBoxTextLen(pEdit), nTextLen));
  if (nTextLen == 0)
    return;
  //if (nTextLen < 0)
  //  nTextLen = INT_MAX;

  try {
    //std::vector<wchar_t> randStrBuf(nTextLen + 1);
    auto randStrBuf = std::make_unique<wchar_t[]>(nTextLen + 1);
    randStrBuf[nTextLen] = '\0';

    const int BUF_SIZE = 64;
    word8 randBuf[BUF_SIZE];
    int nRandPos = BUF_SIZE;
    for (int i = 0; i < nTextLen; i++) {
      if (nRandPos == BUF_SIZE) {
        g_fastRandGen.GetData(randBuf, BUF_SIZE);
        nRandPos = 0;
      }
      randStrBuf[i] = CHARTABLE128[randBuf[nRandPos++] & 127];
    }

    SetEditBoxTextBuf(pEdit, randStrBuf.get());
  }
  catch (...)
  {
  }

  pEdit->Clear();
}
//---------------------------------------------------------------------------
SecureWString GetClipboardTextBuf(void)
{
  TClipboard* pClipboard = Clipboard();
  SecureWString sText;

  try {
    pClipboard->Open();
    if (pClipboard->HasFormat(CF_UNICODETEXT)) {
      HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_UNICODETEXT);
      if (hText != nullptr) {
        const wchar_t* pwszText = reinterpret_cast<const wchar_t*>(
          GlobalLock(hText));
        if (pwszText != nullptr) {
          //word32 lTextLen = wcslen(pwszText);
          sText.AssignStr(pwszText);

          GlobalUnlock(hText);
        }
      }
    }
  }
  __finally {
    pClipboard->Close();
  }

  return sText;
}
//---------------------------------------------------------------------------
SecureAnsiString GetClipboardTextBufAnsi(void)
{
  TClipboard* pClipboard = Clipboard();
  SecureAnsiString asText;

  try {
    if (pClipboard->HasFormat(CF_TEXT)) {
      HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_TEXT);
      if (hText != nullptr) {
        const char* pszText = reinterpret_cast<const char*>(GlobalLock(hText));
        if (pszText != nullptr) {
          //word32 lTextLen = strlen(pszText);
          asText.AssignStr(pszText);

          GlobalUnlock(hText);
        }
      }
    }
  }
  __finally {
    pClipboard->Close();
  }

  return asText;
}
//---------------------------------------------------------------------------
void SetClipboardTextBuf(const wchar_t* pwszSrc)
{
  TClipboard* pClipboard = Clipboard();

  try {
    pClipboard->Open();
    pClipboard->SetTextBuf(const_cast<wchar_t*>(pwszSrc));
  }
  __finally {
    pClipboard->Close();
  }
}
//---------------------------------------------------------------------------
void SetClipboardTextBufAnsi(const char* pszSrc)
{
  TClipboard* pClipboard = Clipboard();

  try {
    pClipboard->Open();

    word32 lLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, nullptr, 0);
    if (lLen > 0) {
      SecureWString wideSrc(lLen);
      MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, wideSrc, lLen);
      pClipboard->SetTextBuf(wideSrc);
    }
  }
  __finally {
    pClipboard->Close();
  }
}
//---------------------------------------------------------------------------
bool StartEditBoxDragDrop(TCustomEdit* pEdit)
{
  SecureWString sText;

  if (pEdit->SelLength == 0)
    sText = GetEditBoxTextBuf(pEdit);
  else
    sText = GetEditBoxSelTextBuf(pEdit);

  if (sText.IsStrEmpty())
    return false;

  // allocate system memory for storing the text
  HGLOBAL hMem = GlobalAlloc(GHND, sText.SizeBytes());
  wchar_t* pwszMem = reinterpret_cast<wchar_t*>(GlobalLock(hMem));

  wcscpy(pwszMem, sText.c_str());

  GlobalUnlock(hMem);

  FORMATETC fmtetc{ CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stgmed{ TYMED_HGLOBAL, { 0 }, 0 };

  // transfer the current selection into the IDataObject
  stgmed.hGlobal = hMem;

  IDataObject *pDataObject;
  IDropSource *pDropSource;

  // Create IDataObject and IDropSource COM objects
  CreateDropSource(&pDropSource);
  CreateDataObject(&fmtetc, &stgmed, 1, &pDataObject);

  //
  //	** ** ** The drag-drop operation starts here! ** ** **
  //
  DWORD dwEffect;
  DWORD dwResult;
  dwResult = DoDragDrop(pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

  pDataObject->Release();
  pDropSource->Release();

  return dwResult == DRAGDROP_S_DROP;
}
//---------------------------------------------------------------------------
void SetFormComponentsAnchors(TForm* pForm)
{
  for (int i = 0; i < pForm->ComponentCount; i++) {
    auto pControl = dynamic_cast<TControl*>(pForm->Components[i]);
    if (pControl) {
      int nTag = pControl->Tag;
      if (nTag > 0 && nTag < 16) {
        TAnchors anc;
        if (nTag & 1) anc << akLeft;
        if (nTag & 2) anc << akTop;
        if (nTag & 4) anc << akRight;
        if (nTag & 8) anc << akBottom;
        pControl->Anchors = anc;
      }
    }
  }
}
//---------------------------------------------------------------------------
bool ExecuteShellOp(const WString& sOperation, bool blShowErrorMsg)
{
  if (sOperation.IsEmpty())
    return false;

  bool blSuccess = false;
  try {
    blSuccess = reinterpret_cast<int>(ShellExecute(nullptr, L"open",
          sOperation.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32;
  }
  catch (std::exception& e) {
    throw Exception(CppStdExceptionToString(e));
  }
  catch (...)
  {
  }

  if (!blSuccess && blShowErrorMsg)
    MsgBox(TRLFormat("Could not open file/execute operation\n\"%1\".",
      { sOperation }),  MB_ICONERROR);

  return blSuccess;
}
//---------------------------------------------------------------------------
bool ExecuteCommand(const WString& sCommand, bool blShowErrorMsg)
{
  if (sCommand.IsEmpty())
    return false;

  bool blSuccess = false;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory( &si, sizeof(si) );
  si.cb = sizeof(si);
  ZeroMemory( &pi, sizeof(pi) );

  try {
    // Start the child process.
    blSuccess = CreateProcess( nullptr,   // No module name (use command line)
        sCommand.c_str(),        // Command line
        nullptr,           // Process handle not inheritable
        nullptr,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        nullptr,           // Use parent's environment block
        nullptr,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi );           // Pointer to PROCESS_INFORMATION structure
  }
  catch (...)
  {
  }

  if (!blSuccess && blShowErrorMsg)
    MsgBox(TRLFormat("Could not execute command\n\"%1\"", { sCommand }),
      MB_ICONERROR);

  return blSuccess;
}
//---------------------------------------------------------------------------
std::vector<int> ParseVersionNumber(const WString& sVersion)
{
  std::vector<int> version;
  auto subStr = SplitString(sVersion, ".");
  for (const auto& sNum : subStr) {
    int nNum = StrToIntDef(sNum, -1);
    if (nNum >= 0) {
      version.push_back(nNum);
      if (version.size() == 5)
        break;
    }
    else break;
  }
  return version;
}
//---------------------------------------------------------------------------
int CompareVersionNumbers(const WString& sVer1, const WString& sVer2)
{
  auto ver1 = ParseVersionNumber(sVer1);
  auto ver2 = ParseVersionNumber(sVer2);

  auto it1 = ver1.begin();
  auto it2 = ver2.begin();

  while (it1 != ver1.end() || it2 != ver2.end()) {
    int n1 = 0, n2 = 0;
    if (it1 != ver1.end())
      n1 = *it1++;
    if (it2 != ver2.end())
      n2 = *it2++;
    if (n1 > n2)
      return 1;
    if (n1 < n2)
      return -1;
  }

  return 0;
}
//---------------------------------------------------------------------------
WString GetAppDataPath(void)
{
  WString sPath;

  wchar_t wszPath[MAX_PATH];

  if (SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, wszPath) == S_OK)
    sPath = WString(wszPath) + WString("\\");

  return sPath;
}
//---------------------------------------------------------------------------
WString FontToString(TFont* pFont)
{
  return Format("%s;%d;%d;%s", ARRAYOFCONST((pFont->Name, pFont->Size,
    static_cast<int>(pFont->Style.Contains(fsBold) |
    (pFont->Style.Contains(fsItalic) << 1) |
    (pFont->Style.Contains(fsUnderline) << 2) |
    (pFont->Style.Contains(fsStrikeOut) << 3)),
    ColorToString(pFont->Color))));
}
//---------------------------------------------------------------------------
int StringToFont(const WString& sFont,
  TFont* pFont)
{
  if (sFont.IsEmpty())
    return 0;

  int i = 0;

  try {
    auto items = SplitString(sFont, ";");
    for (const auto& sItem : items) {
      switch (i) {
      case 0:
        if (pFont) pFont->Name = sItem;
        break;
      case 1:
        {
          int nSize = StrToInt(sItem);
          if (nSize == 0)
            return i;
          if (pFont) pFont->Size = nSize;
        }
        break;
      case 2:
        {
          int nFlags = StrToInt(sItem);
          if (pFont) {
            TFontStyles style;
            if (nFlags & 1)
              style << fsBold;
            if (nFlags & 2)
              style << fsItalic;
            if (nFlags & 4)
              style << fsUnderline;
            if (nFlags & 8)
              style << fsStrikeOut;
            pFont->Style = style;
          }
        }
        break;
      case 3:
        if (pFont) pFont->Color = StringToColor(sItem);
        break;
      }
      if (++i == 4)
        break;
    }
  }
  catch (...)
  {}

  return i;
}
//---------------------------------------------------------------------------
WString CppStdExceptionToString(const std::exception& e)
{
  WString sMsg;

  if (dynamic_cast<const std::bad_alloc*>(&e))
    sMsg = "std::bad_alloc: ";
  else if (dynamic_cast<const std::bad_exception*>(&e))
    sMsg = "std::bad_exception: ";
  else if (dynamic_cast<const std::logic_error*>(&e))
    sMsg = "std::logic_error: ";
  else if (dynamic_cast<const std::runtime_error*>(&e))
    sMsg = "std::runtime_error: ";
  else
    sMsg = "std::exception: ";

  sMsg += e.what();

  return sMsg;
}
//---------------------------------------------------------------------------
WString ConvertCr2Crlf(const WString& sSrc)
{
  return ReplaceStr(sSrc, "\n", CRLF);

  /*if (sSrc.IsEmpty())
  return WString();

  const wchar_t* pwszStr = sSrc.c_str();
  int nLen = sSrc.Length();
  std::wstring sDest;
  sDest.reserve(nLen);

  while (nLen--) {
  if (*pwszStr == '\n')
    sDest.push_back('\r');
  sDest.push_back(*pwszStr++);
  }

  return WString(sDest.c_str());*/
}
//---------------------------------------------------------------------------
WString FileTimeToString(FILETIME ft, bool blLongDate, bool blConvertToLocalTime)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);
  if (blConvertToLocalTime && !SystemTimeToTzSpecificLocalTimeEx(nullptr, &st, &st))
    return WString();

  const int BUFSIZE = 256;
  wchar_t wszDateTime[BUFSIZE];

  int nDateLen = GetDateFormatEx(LOCALE_NAME_USER_DEFAULT,
    blLongDate ? DATE_LONGDATE : DATE_SHORTDATE, &st, nullptr, wszDateTime,
    BUFSIZE, nullptr);

  if (nDateLen == 0)
    return WString();

  wszDateTime[nDateLen - 1] = ' ';

  int nTimeLen = GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, 0, &st, nullptr,
      &wszDateTime[nDateLen], BUFSIZE - nDateLen);

  if (nTimeLen == 0)
    return WString();

  return WString(wszDateTime);
}
//---------------------------------------------------------------------------
int CompareFileTime(FILETIME ft1, FILETIME ft2)
{
  word64 qTime1 = (static_cast<word64>(
    ft1.dwHighDateTime) << 32) | ft1.dwLowDateTime;
  word64 qTime2 = (static_cast<word64>(
    ft2.dwHighDateTime) << 32) | ft2.dwLowDateTime;
  if (qTime1 > qTime2)
    return 1;
  if (qTime1 < qTime2)
    return -1;
  return 0;
}
//---------------------------------------------------------------------------
/*WString EnableInt64FormatSpec(const WString& sFormatStr)
{
  return ReplaceStr(sFormatStr, "%d", "%llu");
}*/
//---------------------------------------------------------------------------
std::vector<SecureWString> SplitStringBuf(const wchar_t* pwszSrc,
  const wchar_t* pwszSep)
{
  std::vector<SecureWString> result;
  while (*pwszSrc != '\0') {
    word32 lLen = wcscspn(pwszSrc, pwszSep);
    if (lLen > 0) {
      SecureWString sItem(pwszSrc, lLen + 1);
      sItem[lLen] = '\0';
      result.push_back(std::move(sItem));
    }
    pwszSrc += lLen;
    if (*pwszSrc != '\0')
      pwszSrc++; // move pointer beyond separator character
  }
  return result;
}
//---------------------------------------------------------------------------
WString RemoveAccessKeysFromStr(const WString& sCaption)
{
  std::wstring sConv;
  sConv.reserve(sCaption.Length());
  for (auto it = sCaption.begin(); it != sCaption.end(); ) {
    if (*it == '&') {
      it++;
      if (it != sCaption.end() && *it == '&') {
        sConv.push_back('&');
        sConv.push_back('&');
        it++;
      }
    }
    else
      sConv.push_back(*it++);
  }
  return WString(sConv.c_str(), sConv.length());
}
//---------------------------------------------------------------------------
int FloorEntropyBits(double val)
{
  double intpart;
  double frac = modf(val, &intpart);
  if (frac > 0.99)
    intpart += 1;
  return static_cast<int>(intpart);
}
//---------------------------------------------------------------------------
#define MX (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z))
#define DELTA 0x9e3779b9
#define SUB(s, v) \
  i++; j = j + s[i]; \
  std::swap(s[i], s[j]); \
  v=((((v>>16)*sk[u&7])&0xffff)<<16)|(((v&0xffff)*sk[(u+1)&7])&0xffff); \
  v=(s[(v>>16)&0xff]<<24)|(s[v&0xff]<<16)|(s[(v>>24)&0xff]<<8)|s[(v>>8)&0xff]; \
  u+=2

static bool decode_96bit(word32 data[3], const word32 key[4])
{
  word32 z, y = data[0], p, sum = 0x36fbef9f, e, u = 0;
  word8 i = 0, j = 0;
  const word16* sk = reinterpret_cast<const word16*>(key);

  word8 sbox[256] = { 74, 173, 14, 155, 255, 101, 149, 208, 147,
  47, 206, 83, 67, 219, 201, 161, 134, 166, 107, 185, 80, 109, 79,
  21, 103, 180, 178, 223, 94, 194, 202, 36, 151, 152, 241, 144, 45,
  114, 158, 254, 198, 214, 251, 177, 46, 22, 81, 187, 230, 29, 92,
  86, 220, 150, 30, 78, 165, 115, 33, 238, 90, 171, 167, 175, 193,
  100, 226, 64, 186, 172, 50, 59, 125, 16, 70, 53, 87, 93, 143, 27,
  117, 142, 247, 58, 31, 164, 240, 221, 248, 148, 124, 73, 35, 137,
  15, 154, 200, 140, 215, 49, 189, 130, 89, 7, 229, 52, 56, 211, 242,
  75, 123, 43, 12, 199, 204, 3, 246, 105, 236, 232, 128, 71, 234, 239,
  0, 72, 218, 2, 160, 18, 40, 54, 181, 60, 138, 163, 84, 146, 37, 135,
  237, 20, 132, 129, 245, 65, 96, 97, 39, 131, 191, 224, 192, 19, 233,
  112, 26, 225, 119, 28, 82, 244, 174, 98, 169, 42, 212, 231, 68, 62,
  66, 102, 170, 85, 188, 179, 57, 17, 162, 252, 168, 116, 61, 203, 205,
  127, 88, 159, 9, 110, 51, 228, 44, 77, 153, 197, 32, 41, 122, 222, 76,
  249, 250, 121, 91, 25, 139, 106, 235, 184, 182, 113, 10, 120, 133, 217,
  157, 24, 95, 253, 196, 63, 141, 38, 108, 111, 55, 4, 104, 69, 195, 213,
  118, 243, 6, 207, 1, 11, 126, 190, 156, 227, 34, 136, 8, 13, 176, 183,
  99, 209, 23, 48, 210, 5, 216, 145 };

  while (sum) {
    e = sum >> 2 & 3;

    for (p = 2; p > 0; p--) {
      z = data[p - 1];
      SUB(sbox, data[p]);
      y = data[p] -= MX;
    }

    z = data[2];
    SUB(sbox, data[0]);
    y = data[0] -= MX;
    sum -= DELTA;
  }

  const word8* buf = reinterpret_cast<word8*>(data);
  for (i = 0; i < 2; i++) {
    word8 h = sbox[(buf[0] + i)&0xff]; // avoid buffer overflow
    for (j = 1; j < 10; j++)
      h = sbox[buf[j] ^ h];
    if (h != buf[10 + i])
      return false;
  }

  return true;
}

const word8 CUSTOM_BASE64_DEC_MAP[128] =
{
  127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
  127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
  127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
  127, 127, 127,  63, 127,  62, 127, 127, 127, 127,
  127, 127, 127, 127, 127, 127, 127, 127,  52,  53,
  54,  55,  56,  57,  58,  59,  60,  61, 127, 127,
  127, 127, 127, 127, 127,   0,   1,   2,   3,   4,
  5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
  25, 127, 127, 127, 127, 127, 127,  26,  27,  28,
  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
  49,  50,  51, 127, 127, 127, 127, 127
};

std::tuple<int, int, AnsiString> CheckDonorKey(const AnsiString& asInput)
{
  int nType = DONOR_TYPE_STD;
  AnsiString asDonorId;

  if (asInput.Length() < 16)
    return { DONOR_KEY_INVALID, nType, asDonorId };

  AnsiString asKey = asInput.Trim();

  if (asKey.Length() != 16)
    return { DONOR_KEY_INVALID, nType, asDonorId };

  word8 buf[13];
  buf[12] = '\0';
  size_t destLen = 12;

  base64_decode(buf, &destLen, reinterpret_cast<const word8*>(asKey.c_str()), 16);

  if (destLen != 12)
    return { DONOR_KEY_INVALID, nType, asDonorId };

  const word32 param[4] =  { 0x77adb64b, 0x959561b7, 0x8799de93, 0x22ef89dd };

  if (!decode_96bit(reinterpret_cast<word32*>(buf), param))
    return { DONOR_KEY_INVALID, nType, asDonorId };

  if (buf[0] != 'P' || buf[1] != '3')
    return { DONOR_KEY_INVALID, nType, asDonorId };

  for (int nI = 2; nI < 10; nI++) {
    if (buf[nI] < ' ' || buf[nI] > '~')
      return { DONOR_KEY_INVALID, nType, asDonorId };
  }

  int n1 = CUSTOM_BASE64_DEC_MAP[buf[5]];
  int n2 = CUSTOM_BASE64_DEC_MAP[buf[6]];

  if (n1 == 127 || n2 == 127)
    return { DONOR_KEY_INVALID, nType, asDonorId };

  int nVersion = ((n2 & 15) << 6) | n1;
  nType = n2 >> 4;

  if (nType != DONOR_TYPE_PRO &&
      PROGRAM_MAINVER_UPDATE_NUM - nVersion > DONOR_STD_NUM_UPDATES)
    return { DONOR_KEY_EXPIRED, nType, asDonorId };

  buf[10] = '\0';
  asDonorId = AnsiString(reinterpret_cast<char*>(buf) + 2);

  return { DONOR_KEY_VALID, nType, asDonorId };
}
//---------------------------------------------------------------------------
