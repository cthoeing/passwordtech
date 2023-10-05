// hrtimer.cpp
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
#include <windows.h>
#pragma hdrstop

#include "hrtimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


enum HighResTimer g_highResTimer = HighResTimer::None;

inline void rdtsc(word64* pCounter)
{
  //word64 x;
  //__asm volatile (".byte 0x0f, 0x31" : "=A" (x));
  *pCounter = __rdtsc();
}

//---------------------------------------------------------------------------
enum HighResTimer HighResTimerCheck(void)
{
  // first, check if the RDTSC instruction is available
  // calling IsProcessorFeaturePresent() with parameter 8 will probably
  // only succeed on Windows version 5 or higher
  if (IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE))
    g_highResTimer = HighResTimer::RDTSC;
  else {
    // check if we can use Windows' QPC instead
    LARGE_INTEGER dummy;
    if (QueryPerformanceFrequency(&dummy))
      g_highResTimer = HighResTimer::QPC;
  }

  if (g_highResTimer != HighResTimer::None) {
    // roughly check if the high-resolution timer is REALLY available
    // (this may not be the case in emulated systems...)

    word64 qBefore, qAfter;

    HighResTimer(&qBefore);

    // do something
    word32 lVal = 0xdeadbeef;
    for (int i = 0; i < 10; i++)
      lVal = (lVal << 5) + lVal + i;

    HighResTimer(&qAfter);

    // if the time difference is zero, the high-resolution counter
    // is definitely not available, so we should better use a low-resolution
    // timer instead, which is likely to be implemented correctly...
    // We expect the low-resolution timer to have millisecond resolution
    // (at least), which is okay for measuring the delay between mouse events
    // and keystrokes.
    if (qAfter == qBefore)
      g_highResTimer = HighResTimer::None;
  }

  return g_highResTimer;
}
//---------------------------------------------------------------------------
void HighResTimer(void* pTimer)
{
  switch (g_highResTimer) {
  case HighResTimer::RDTSC:
    rdtsc(reinterpret_cast<word64*>(pTimer));
    break;
  case HighResTimer::QPC:
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(pTimer));
    break;
  default:
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(pTimer));
    // Although the FILETIME structure officially supports a time resolution
    // of 100 nanoseconds, the actual accuracy of this timer doesn't seem
    // to be any better than that of GetTickCount() (which has a maximum
    // accuracy of 1 millisecond). So for entropy estimations, the value
    // retrieved by GetSystemTimeAsFileTime() should be derated by a factor
    // of 10,000 (10,000 * 100 nanoseconds = 1 millisecond).
  }
}
//---------------------------------------------------------------------------
