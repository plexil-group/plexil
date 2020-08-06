/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//
// Agenda implementation
//

#include "Agenda.hh"

#include "ResponseMessage.hh"

#include "ThreadMutex.hh"
#include "timeval-utils.hh"

#include <map>

using PLEXIL::ThreadMutex;
using PLEXIL::ThreadMutexGuard;

class AgendaImpl : public Agenda
{
private:

  friend Agenda* makeAgenda();

  AgendaImpl()
    : m_mutex(new ThreadMutex())
  {
  }

  //
  // Member variables
  //
  typedef std::multimap<timeval, ResponseMessage *> AgendaMap;
  typedef std::pair<timeval, ResponseMessage *> AgendaMapEntry;

  AgendaMap m_map;
  ThreadMutex *m_mutex;

public:

  virtual ~AgendaImpl()
  {
    {
      ThreadMutexGuard g(*m_mutex);
      // Delete all the ResponseMessage instances
      while (!m_map.empty()) {
        delete m_map.begin()->second;
        m_map.erase(m_map.begin());
      }
    }
    delete m_mutex;
  }

  size_t size() const
  {
    ThreadMutexGuard g(*m_mutex);
    return m_map.size();
  }

  bool empty() const
  {
    ThreadMutexGuard g(*m_mutex);
    return m_map.empty();
  }
    
  // Only valid when not empty.
  timeval nextResponseTime() const
  {
    ThreadMutexGuard g(*m_mutex);
    if (m_map.empty())
      return timeval{0,0};
    return m_map.begin()->first;
  }

  ResponseMessage *getNextResponse(timeval &tym)
  {
    ThreadMutexGuard g(*m_mutex);
    if (m_map.empty()) {
      tym.tv_sec = 0;
      tym.tv_usec = 0;
      return NULL;
    }

    tym = m_map.begin()->first;
    return m_map.begin()->second;
  }

  void pop()
  {
    ThreadMutexGuard g(*m_mutex);
    if (!m_map.empty())
      m_map.erase(m_map.begin());
  }
  
  void scheduleResponse(timeval tym, ResponseMessage *msg)
  {
    ThreadMutexGuard g(*m_mutex);
    m_map.insert(AgendaMapEntry(tym, msg));
  }
  
};

Agenda *makeAgenda()
{
  return new AgendaImpl();
}
