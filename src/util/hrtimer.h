// hrtimer.h
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
#ifndef hrtimerH
#define hrtimerH
//---------------------------------------------------------------------------
#include <chrono>
#include "types.h"

const int HRT_NONE_ACCURACY_DERATING = 10000; // 100 nanoseconds => 1 millisecond

enum class HighResTimer {
  None,   // uses GetSystemTimeAsFileTime()
  RDTSC,  // uses RDTSC Pentium instruction
  QPC     // uses Windows API function QueryPerformanceCounter()
};

extern HighResTimer g_highResTimer;

// check which high resolution timer is available
HighResTimer HighResTimerCheck(void);

// get the current value of the high resolution timer
// -> buffer for the timer value (size must be at least 8 bytes)
void HighResTimer(void* pTimer);

class Stopwatch
{
    using Clock = std::chrono::steady_clock;
    Clock::time_point m_last;

public:
    Stopwatch()
    {
        Reset();
    }

    void Reset()
    {
        m_last = Clock::now();
    }

    typename Clock::duration Elapsed() const
    {
        return Clock::now() - m_last;
    }

    double ElapsedSeconds() const
    {
        return std::chrono::duration<double>(Elapsed()).count();
    }

    Clock::duration Tick()
    {
        auto now = Clock::now();
        auto elapsed = now - m_last;
        m_last = now;
        return elapsed;
    }
};

#endif
