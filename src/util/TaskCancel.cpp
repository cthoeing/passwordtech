// TaskCancel.cpp
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
#pragma hdrstop

#include "TaskCancel.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

TaskCancelManager& TaskCancelManager::GetInstance()
{
  static TaskCancelManager inst;
  return inst;
}

void TaskCancelManager::CancelAll(TaskCancelReason reason)
{
  std::lock_guard<std::mutex> lock(m_lock);
  for (auto ct : m_cancelTokens)
    ct->Cancel(reason);
}

int TaskCancelManager::GetNumTasksRunning()
{
  std::lock_guard<std::mutex> lock(m_lock);
  return m_cancelTokens.size();
}

void TaskCancelManager::AddToken(TaskCancelToken* ct)
{
  std::lock_guard<std::mutex> lock(m_lock);
  m_cancelTokens.push_back(ct);
}

void TaskCancelManager::RemoveToken(TaskCancelToken* ct)
{
  std::lock_guard<std::mutex> lock(m_lock);
  auto it = std::find(m_cancelTokens.begin(), m_cancelTokens.end(), ct);
  if (it != m_cancelTokens.end())
    m_cancelTokens.erase(it);
}


TaskCancelToken::TaskCancelToken()
  : m_token(new std::atomic<bool>(false)), m_reason(TaskCancelReason::UserCancel)
{
  TaskCancelManager::GetInstance().AddToken(this);
}

TaskCancelToken::~TaskCancelToken()
{
  TaskCancelManager::GetInstance().RemoveToken(this);
}
