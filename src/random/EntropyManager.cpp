// EntropyManager.cpp
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
#include <vcl.h>
#pragma hdrstop

#include "EntropyManager.h"
#include "hrtimer.h"
#include "minilzo.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#if 0
const word8 LOGTABLE256[256] =
{
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
  0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
  LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
  LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
#undef LT
};
#endif

// logBase2(): based on code from
// http://graphics.stanford.edu/~seander/bithacks.html

inline word32 logBase2(word32 v)
{
#if 1
  return v ? __builtin_clz(v) ^ 31 : 0;
#else
  word32 t, tt;

  if ((tt = v >> 16))
    return (t = tt >> 8) ? 24 + LOGTABLE256[t] : 16 + LOGTABLE256[tt];

  return (t = v >> 8) ? 8 + LOGTABLE256[t] : LOGTABLE256[v];
#endif
}

const int TIMER_ENTROPYBITS_DERATING = 4;

//---------------------------------------------------------------------------
word32 EntropyManager::AddEvent(const MSG& msg,
  EntropyEventType event,
  word32 lEntBits)
{
  static word64 qLastTimer = 0;
  word64 qTimer;

  HighResTimer(&qTimer);
  m_randPool.AddData(&qTimer, sizeof(word64));

  word32 lDelta = static_cast<word32>(
    std::min(0xffffffffull, qTimer - qLastTimer));
  qLastTimer = qTimer;

  if (event == entKeyboard) {
    if (msg.wParam == m_lastKeys[0] && msg.wParam == m_lastKeys[1])
      return 0;
    m_lastKeys[1] = m_lastKeys[0];
    m_lastKeys[0] = msg.wParam;
  }
  else if (event == entMouseClick) {
    if (msg.lParam == m_lastPos[0] && msg.lParam == m_lastPos[1])
      return 0;
    m_lastPos[1] = m_lastPos[0];
    m_lastPos[0] = msg.lParam;
  }

  m_randPool.AddData(&msg, sizeof(MSG));

  if (g_highResTimer == HighResTimer::None)
    lDelta /= HRT_NONE_ACCURACY_DERATING;

  if (lDelta >= (1 << (TIMER_ENTROPYBITS_DERATING + 1)) &&
    !(lDelta == m_lastDeltas[0] && lDelta == m_lastDeltas[1])) {
    m_lastDeltas[1] = m_lastDeltas[0];
    m_lastDeltas[0] = lDelta;

    // calculate log_2(lDelta) to _roughly_ estimate the (maximum) entropy
    // provided by lDelta
    // (inspired by the PGP source code, pgpRandomPool.c)
    word32 lTimerEntBits = logBase2(lDelta) - TIMER_ENTROPYBITS_DERATING;

    lEntBits += std::min(lTimerEntBits, m_lMaxTimerEntBits);
  }

  IncreaseEntropyBits(lEntBits);

  return lEntBits;
}
//---------------------------------------------------------------------------
word32 EntropyManager::AddEvent(const TMessage& msg,
  EntropyEventType event,
  word32 lEntBits)
{
  MSG winMsg;
  winMsg.message = msg.Msg;
  winMsg.lParam = msg.LParam;
  winMsg.wParam = msg.WParam;
  winMsg.time = GetMessageTime();
  GetCursorPos(&winMsg.pt);
  return AddEvent(winMsg, event, lEntBits);
}
//---------------------------------------------------------------------------
word32 EntropyManager::AddData(const void* pData,
  word32 lNumOfBytes,
  float fEntBitsIncompr,
  float fEntBitsCompr)
{
  if (lNumOfBytes == 0)
    return 0;

  SecureMem<word8> comprBuf(lNumOfBytes + lNumOfBytes / 16 + 64 + 3),
    workBuf(LZO1X_1_MEM_COMPRESS);

  lzo_uint comprLen;
  lzo1x_1_compress(reinterpret_cast<const word8*>(pData), lNumOfBytes, comprBuf,
    &comprLen, workBuf);

  m_randPool.AddData(comprBuf, comprLen);

  comprLen = (comprLen > 4) ? comprLen - 4 : 1;

  word32 lEntBits;
  if (comprLen >= lNumOfBytes)
	  lEntBits = std::min<double>(lNumOfBytes * fEntBitsIncompr, MAX_TOTALENTBITS);
  else
    lEntBits = std::min<double>(comprLen * fEntBitsCompr, MAX_TOTALENTBITS);

  IncreaseEntropyBits(lEntBits);
  return lEntBits;
}
//---------------------------------------------------------------------------
