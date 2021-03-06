// Util.cpp
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
#include <vcl.h>
#include <stdio.h>
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
{ L"Info", L"Warning", L"Question", L"Error" };

//---------------------------------------------------------------------------
static void strCr2Crlf(SecureWString& sSrc)
{
  const wchar_t* pwszBuf = sSrc.c_str();

  int nNumCR = 0;
  while ((pwszBuf = wcschr(pwszBuf, '\r')) != NULL) {
    if (*++pwszBuf == '\n')
      return;
    nNumCR++;
  }

  if (nNumCR == 0)
    return;

  int nSize = sSrc.Size();
  SecureWString sTemp(nSize + nNumCR);
  for (int nI = 0, nJ = 0; nI < nSize; nI++) {
    sTemp[nJ++] = sSrc[nI];
    if (sSrc[nI] == '\r')
      sTemp[nJ++] = '\n';
  }

  sSrc = sTemp;
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

  if (TopMostManager::GetInstance()->AlwaysOnTop)
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
  EntropyManager::GetInstance()->AddEvent(msg, entOther, 1);

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
void GetEditBoxTextBuf(TCustomEdit* pEdit,
  SecureWString& sDest)
{
  int nLen = pEdit->GetTextLen();
  if (nLen > 0) {
    nLen++; // including terminating '\0'
    sDest.New(nLen);
    pEdit->GetTextBuf(sDest, nLen);
  }
}
//---------------------------------------------------------------------------
void GetEditBoxSelTextBuf(TCustomEdit* pEdit,
  SecureWString& sDest)
{
  int nLen = pEdit->SelLength;
  if (nLen > 0) {
    nLen++;
    sDest.New(nLen);
    pEdit->GetSelTextBuf(sDest, nLen);
    /*SecureWString sTemp;
    GetEditBoxTextBuf(pEdit, sTemp);

    sDest.New(nLen + 1);
    wcsncpy(sDest, sTemp + pEdit->SelStart, nLen);
    sDest[nLen] = '\0';*/
  }
}
//---------------------------------------------------------------------------
void GetRichEditSelTextBuf(TCustomRichEdit* pEdit,
  SecureWString& sDest)
{
  int nLen = pEdit->SelLength;
  if (nLen > 0) {
    nLen++;
    sDest.New(nLen);
    pEdit->GetSelTextBuf(sDest, nLen);
    strCr2Crlf(sDest);
  }
  /*CHARRANGE range;
  SendMessage(pEdit->Handle, EM_EXGETSEL, 0, reinterpret_cast<word32>(&range));

  int nLen = range.cpMax - range.cpMin;
  if (nLen > 0) {
  sDest.New(nLen + 1);
  SendMessage(pEdit->Handle, EM_GETSELTEXT, 0, reinterpret_cast<word32>(sDest.c_str()));

  strCr2Crlf(sDest);
  }*/
}
//---------------------------------------------------------------------------
void SetEditBoxTextBuf(TCustomEdit* pEdit,
  const wchar_t* pwszSrc)
{
  //pEdit->SetTextBuf((pwszSrc != NULL) ? (wchar_t*) pwszSrc : L"");
  const wchar_t* pwszBuf = (pwszSrc != NULL) ? pwszSrc : L"";

  SetWindowText(pEdit->Handle, pwszBuf);
  pEdit->Perform(CM_TEXTCHANGED, 0, static_cast<NativeInt>(0));
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

  nTextLen = std::max(GetEditBoxTextLen(pEdit), nTextLen);
  if (nTextLen == 0)
    return;

  try {
    std::wstring randStr(nTextLen, 0);
    //randStr[nTextLen] = '\0';

    const int BUF_SIZE = 64;
    word8 rand[BUF_SIZE];
    int nRandPos = BUF_SIZE;
    for (auto& ch : randStr) {
      if (nRandPos == BUF_SIZE) {
        g_fastRandGen.GetData(rand, BUF_SIZE);
        nRandPos = 0;
      }
      ch = CHARTABLE128[rand[nRandPos++] & 127];
    }

    SetEditBoxTextBuf(pEdit, randStr.c_str());
  }
  catch (...)
  {
  }

  pEdit->Clear();
}
//---------------------------------------------------------------------------
bool GetClipboardTextBuf(SecureWString* psDestW,
  SecureAnsiString* psDestA)
{
  TClipboard* pClipboard = Clipboard();

  try {
    pClipboard->Open();
    if (psDestW != NULL && pClipboard->HasFormat(CF_UNICODETEXT))
    {
      HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_UNICODETEXT);
      if (hText != NULL)
      {
        wchar_t* pwszText = (wchar_t*) GlobalLock(hText);
        if (pwszText != NULL)
        {
          word32 lTextLen = wcslen(pwszText);
          psDestW->Assign(pwszText, lTextLen + 1);

          GlobalUnlock(hText);
        }
      }
    }
    else if (pClipboard->HasFormat(CF_TEXT))
    {
      HGLOBAL hText = (HGLOBAL) pClipboard->GetAsHandle(CF_TEXT);
      if (hText != NULL)
      {
        char* pszText = (char*) GlobalLock(hText);
        if (pszText != NULL)
        {
          if (psDestW != NULL) {
            word32 lBufLen = MultiByteToWideChar(CP_ACP, 0, pszText, -1, NULL, 0);
            psDestW->New(lBufLen);
            MultiByteToWideChar(CP_ACP, 0, pszText, -1, *psDestW, lBufLen);
            //lTextLen = lBufLen - 1; // excluding '\0'
          }
          else {
            word32 lTextLen = strlen(pszText);
            psDestA->Assign(pszText, lTextLen + 1);
          }

          GlobalUnlock(hText);
        }
      }
    }
  }
  catch (...) {
    pClipboard->Close();
    return false;
  }

  pClipboard->Close();
  return true;
}
//---------------------------------------------------------------------------
void SetClipboardTextBuf(const wchar_t* pwszSrcW,
  const char* pszSrcA)
{
  TClipboard* pClipboard = Clipboard();

  try {
    pClipboard->Open();

    if (pwszSrcW != NULL)
      pClipboard->SetTextBuf((wchar_t*) pwszSrcW);
    else {
      word32 lLen = MultiByteToWideChar(CP_ACP, 0, pszSrcA, -1, NULL, 0);
      if (lLen > 0) {
        SecureWString wideSrc(lLen);
        MultiByteToWideChar(CP_ACP, 0, pszSrcA, -1, wideSrc, lLen);
        pClipboard->SetTextBuf(wideSrc);
      }
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
    GetEditBoxTextBuf(pEdit, sText);
  else
    GetEditBoxSelTextBuf(pEdit, sText);

  if (sText.Size() < 2)
    return false;

  // allocate system memory for storing the text
  HGLOBAL hMem = GlobalAlloc(GHND, sText.SizeBytes());
  wchar_t* pwszMem = (wchar_t*) GlobalLock(hMem);

  wcscpy(pwszMem, sText.c_str());

  GlobalUnlock(hMem);

  FORMATETC fmtetc = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  STGMEDIUM stgmed = { TYMED_HGLOBAL, { 0 }, 0 };

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
bool ExecuteShellOp(const WString& sOperation, bool blShowErrorMsg)
{
  if (sOperation.IsEmpty())
    return false;

  bool blSuccess = false;
  try {
    blSuccess = reinterpret_cast<int>(ShellExecute(NULL, L"open",
          sOperation.c_str(), NULL, NULL, SW_SHOWNORMAL)) > 32;
  }
  catch (std::exception& e) {
    throw Exception(CppStdExceptionToString(&e));
  }
  catch (...)
  {
  }

  if (!blSuccess && blShowErrorMsg)
    MsgBox(TRLFormat("Could not open file/execute operation\n\"%s\".",
      sOperation.c_str()),  MB_ICONERROR);

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
    blSuccess = CreateProcess( NULL,   // No module name (use command line)
        sCommand.c_str(),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi );           // Pointer to PROCESS_INFORMATION structure
  }
  catch (...)
  {
  }

  if (!blSuccess && blShowErrorMsg)
    MsgBox(TRLFormat("Could not execute command\n\"%s\"", sCommand.c_str()),
      MB_ICONERROR);

  return blSuccess;
}
//---------------------------------------------------------------------------
int CompareVersions(AnsiString asVer1,
  AnsiString asVer2)
{
  static const char VERSION_FORMAT[] = "%d.%d.%d";
  int ver1[3] = {-1,-1,-1}, ver2[3] = {-1,-1,-1};

  sscanf(asVer1.c_str(), VERSION_FORMAT, &ver1[0], &ver1[1], &ver1[2]);
  sscanf(asVer2.c_str(), VERSION_FORMAT, &ver2[0], &ver2[1], &ver2[2]);

  // handle obsolete version numbering "x.yy":
  // convert it to "x.0.yy"
  if (ver1[2] < 0 && ver1[1] >= 0) {
    ver1[2] = ver1[1];
    ver1[1] = 0;
  }
  if (ver2[2] < 0 && ver2[1] >= 0) {
    ver2[2] = ver2[1];
    ver2[1] = 0;
  }

  if (ver1[0] != ver2[0])
    return (ver1[0] > ver2[0]) ? 1 : -1;
  if (ver1[1] != ver2[1])
    return (ver1[1] > ver2[1]) ? 1 : -1;
  if (ver1[2] != ver2[2])
    return (ver1[2] > ver2[2]) ? 1 : -1;

  return 0;
}
//---------------------------------------------------------------------------
WString GetAppDataPath(void)
{
  WString sPath;

  wchar_t wszPath[MAX_PATH];

  if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, wszPath) == S_OK)
    sPath = WString(wszPath) + WString("\\");

  return sPath;
}
//---------------------------------------------------------------------------
WString FontToString(TFont* pFont)
{
  return Format("%s;%d,%d,%s", ARRAYOFCONST((pFont->Name, pFont->Size,
    static_cast<int>(pFont->Style.Contains(fsBold) |
    (pFont->Style.Contains(fsItalic) << 1) |
    (pFont->Style.Contains(fsUnderline) << 2) |
    (pFont->Style.Contains(fsStrikeOut) << 3)),
    ColorToString(pFont->Color))));
}
//---------------------------------------------------------------------------
int StringToFont(WString sFont,
  TFont* pFont)
{
  if (sFont.IsEmpty())
    return 0;

  int nPos = sFont.Pos(";");
  if (nPos < 2)
    return 0;

  pFont->Name = sFont.SubString(1, nPos - 1);
  int nNumParsed = 1;

  sFont.Delete(1, nPos);

  nPos = sFont.Pos(",");
  if (nPos < 2)
    return nNumParsed;

  int nSize = StrToIntDef(sFont.SubString(1, nPos - 1), 0);
  if (nSize == 0)
    return nNumParsed;

  pFont->Size = nSize;
  nNumParsed++;

  sFont.Delete(1, nPos);

  nPos = sFont.Pos(",");
  if (nPos == 1 || nPos > 3)
    return nNumParsed;

  int nFlags = StrToIntDef(sFont.SubString(1, nPos - 1), 0);
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
  nNumParsed++;

  sFont.Delete(1, nPos);
  if (sFont.Length() == 0)
    return nNumParsed;

  pFont->Color = StringToColor(sFont);
  nNumParsed++;

  return nNumParsed;
}
//---------------------------------------------------------------------------
WString CppStdExceptionToString(std::exception* e)
{
  WString sMsg;

  if (dynamic_cast<std::bad_alloc*>(e) != NULL)
    sMsg = "std::bad_alloc: ";
  else if (dynamic_cast<std::bad_exception*>(e) != NULL)
    sMsg = "std::bad_exception: ";
  else if (dynamic_cast<std::logic_error*>(e) != NULL)
    sMsg = "std::logic_error: ";
  else if (dynamic_cast<std::runtime_error*>(e) != NULL)
    sMsg = "std::runtime_error: ";
  else
    sMsg = "std::exception: ";

  sMsg += e->what();

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
WString FileTimeToString(FILETIME ft, bool blLongDate)
{
  SYSTEMTIME st;
  FileTimeToSystemTime(&ft, &st);

  const int BUFSIZE = 256;
  wchar_t wszDateTime[BUFSIZE];

  int nDateLen = GetDateFormat(LOCALE_USER_DEFAULT,
    blLongDate ? DATE_LONGDATE : DATE_SHORTDATE, &st, NULL, wszDateTime, BUFSIZE);

  if (nDateLen == 0)
    return WString();

  wszDateTime[nDateLen - 1] = ' ';

  int nTimeLen = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL,
      &wszDateTime[nDateLen], BUFSIZE - nDateLen);

  if (nTimeLen == 0)
    return WString();

  return WString(wszDateTime);
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
    word8 h = sbox[buf[0] + i];
    for (j = 1; j < 10; j++)
      h = sbox[buf[j] ^ h];
    if (h != buf[10 + i])
      return false;
  }

  return true;
}

static const word8 CBASE64_DEC_MAP[128] =
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

int CheckDonorKey(const AnsiString& asInput,
  AnsiString* pasId,
  int* pnType)
{
  if (asInput.Length() < 16)
    return DONOR_KEY_INVALID;

  AnsiString asKey = asInput.Trim();

  if (asKey.Length() != 16)
    return DONOR_KEY_INVALID;

  word8 buf[13];
  buf[12] = '\0';
  size_t destLen = 12;

  base64_decode(buf, &destLen, reinterpret_cast<const word8*>(asKey.c_str()), 16);

  if (destLen != 12)
    return DONOR_KEY_INVALID;

  const word32 param[4] =  { 0x77adb64b, 0x959561b7, 0x8799de93, 0x22ef89dd };

  if (!decode_96bit(reinterpret_cast<word32*>(buf), param))
    return DONOR_KEY_INVALID;

  if (buf[0] != 'P' || buf[1] != '3')
    return DONOR_KEY_INVALID;

  for (int nI = 2; nI < 10; nI++) {
    if (buf[nI] < ' ' || buf[nI] > '~')
      return DONOR_KEY_INVALID;
  }

  int n1 = CBASE64_DEC_MAP[buf[5]];
  int n2 = CBASE64_DEC_MAP[buf[6]];

  if (n1 == 127 || n2 == 127)
    return DONOR_KEY_INVALID;

  int nVersion = ((n2 & 15) << 6) | n1;
  int nType = n2 >> 4;

  if (nType != DONOR_TYPE_PRO &&
      PROGRAM_MAINVER_UPDATE_NUM - nVersion > DONOR_STD_NUM_UPDATES)
    return DONOR_KEY_EXPIRED;

  if (pasId != NULL) {
    buf[10] = '\0';
    *pasId = AnsiString(reinterpret_cast<char*>(buf) + 2);
  }

  if (pnType != NULL)
    *pnType = nType;

  return DONOR_KEY_VALID;
}
//---------------------------------------------------------------------------
