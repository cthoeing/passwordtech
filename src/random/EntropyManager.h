// EntropyManager.h
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
#ifndef EntropyManagerH
#define EntropyManagerH
//---------------------------------------------------------------------------
#include <algorithm>
#include <memory>
#include "RandomPool.h"

enum EntropyEventType {
  entKeyboard, entMouseClick, entMouseMove, entMouseWheel, entOther
};


class EntropyManager
{
private:
  RandomPool& m_randPool;
  word32 m_lEntropyBits;
  word32 m_lTotalEntBits;
  word32 m_lMaxTimerEntBits;
  word32 m_lSystemEntBits;
  WPARAM m_lastKeys[2];
  LPARAM m_lastPos[2];
  word32 m_lastDeltas[2];

  // increase entropy bits in the pool by nBits
  void IncreaseEntropyBits(word32 lBits)
  {
    m_lEntropyBits = std::min<word32>(m_lEntropyBits + lBits, MAX_ENTROPYBITS);
    m_lTotalEntBits = std::min<word32>(m_lTotalEntBits + lBits, MAX_TOTALENTBITS);
  }

public:

  enum {
    MAX_ENTROPYBITS = 10000,
    MAX_TOTALENTBITS = 1000000000
  };

  // class constructor
  // -> random pool
  // -> maximum entropy (in bits) for the timer events
  // -> 'quality' of the system entropy (see TRandomPool::Randomize())
  EntropyManager(word32 lMaxTimerEntBits = 4,
    word32 lSystemEntBits = 16)
    : m_randPool(RandomPool::GetInstance()), m_lEntropyBits(0), m_lTotalEntBits(0),
      m_lMaxTimerEntBits(lMaxTimerEntBits), m_lSystemEntBits(lSystemEntBits)
  {
    m_lastKeys[0] = m_lastKeys[1] = 0;
    m_lastPos[0] = m_lastPos[1] = 0;
    m_lastDeltas[0] = m_lastDeltas[1] = 0;
  };

  // class destructor
  ~EntropyManager()
  {
    m_lEntropyBits = m_lTotalEntBits = 0;
    m_lastKeys[0] = m_lastKeys[1] = 0;
    m_lastPos[0] = m_lastPos[1] = 0;
    m_lastDeltas[0] = m_lastDeltas[1] = 0;
  };

  static EntropyManager& GetInstance(void)
  {
    static EntropyManager inst;
    return inst;
  }

  // add a mouse or keyboard event to the random pool
  // -> Windows message structure holding information about the event
  // -> event type (keyboard/mouse)
  // -> 'quality' of this event; note that the 'main entropy' is provided
  //    by the high-performance counter which is called by this function
  // <- entropy (in bits) provided by nEntBits and the high-performance counter;
  //    if this value is 0, the event was identical to the last two events,
  //    which is suspicious (for example, the user could be pressing the
  //    same key all the time)
  word32 AddEvent(const MSG& msg,
    EntropyEventType event,
    word32 lEntBits);

  // almost the same as above, using BCPPB's 'TMessage' instead of 'MSG'
  word32 AddEvent(const TMessage& msg,
    EntropyEventType event,
    word32 lEntBits);

  word32 AddData(const void* pData,
    word32 lNumOfBytes,
    float fEntBitsIncompr,
    float fEntBitsCompr);

  // just add system entropy to the pool
  void AddSystemEntropy(void)
  {
    m_randPool.Randomize();
    IncreaseEntropyBits(m_lSystemEntBits);
  }

  // decrease entropy bits in the random pool by nBits
  word32 ConsumeEntropyBits(word32 lBits)
  {
    word32 lEntBitsMax = EntropyBitsMax;
    if (lBits < lEntBitsMax)
      m_lEntropyBits = lEntBitsMax - lBits;
    else
      m_lEntropyBits = 0;
    return m_lEntropyBits;
  }

  // get max. entropy in the pool
  word32 GetEntropyBitsMax(void)
  {
    return std::min<word32>(m_lEntropyBits, RandomPool::MAX_ENTROPY);
  }

  // reset the entropy counters
  void ResetCounters(void)
  {
    m_lEntropyBits = m_lTotalEntBits = 0;
  }


  // properties

  __property word32 EntropyBits =
  { read=m_lEntropyBits };
  __property word32 EntropyBitsMax =
  { read=GetEntropyBitsMax };
  __property word32 TotalEntropyBits =
  { read=m_lTotalEntBits };
  __property word32 MaxTimerEntropyBits =
  { read=m_lMaxTimerEntBits, write=m_lMaxTimerEntBits };
  __property word32 SystemEntropyBits =
  { read=m_lSystemEntBits, write=m_lSystemEntBits };

};

#endif
