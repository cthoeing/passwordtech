// TaskCancel.h
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
#ifndef TaskCancelH
#define TaskCancelH
//---------------------------------------------------------------------------
#include <atomic>
#include <vector>
#include <mutex>

enum class TaskCancelReason
{
  UserCancel, ProgramTermination, SystemShutdown
};

class TaskCancelToken;

class TaskCancelManager
{
public:

  static TaskCancelManager& GetInstance();

  void CancelAll(TaskCancelReason reason);

  int GetNumTasksRunning();

private:
  friend class TaskCancelToken;
  std::vector<TaskCancelToken*> m_cancelTokens;
  std::mutex m_lock;

  TaskCancelManager() {}

  void AddToken(TaskCancelToken* ct);
  void RemoveToken(TaskCancelToken* ct);
};

class TaskCancelToken
{
public:
  TaskCancelToken();
  TaskCancelToken(const TaskCancelToken& other) = delete;
  TaskCancelToken& operator= (const TaskCancelToken& other) = delete;

  ~TaskCancelToken();

  std::shared_ptr<std::atomic<bool>> Get() const
  {
    return m_token;
  }

  void Cancel(TaskCancelReason reason)
  {
    *m_token = true;
    m_reason = reason;
  }

  operator bool() const
  {
    return m_token->load();
  }

  __property TaskCancelReason Reason =
  { read=m_reason };

private:
  std::shared_ptr<std::atomic<bool>> m_token;
  TaskCancelReason m_reason;
};

#endif
