/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "timeval-utils.hh"

#include <deque>
#include <memory>
#include <mutex>

typedef std::pair<timeval, std::unique_ptr<ResponseMessage> > AgendaEntry;
typedef std::deque<AgendaEntry> AgendaQueue;

class AgendaImpl : public Agenda
{
private:
  friend Agenda* makeAgenda();

  //
  // Member variables
  //

  AgendaQueue m_queue;
  std::unique_ptr<std::mutex> m_mutex;

  AgendaImpl()
    : m_mutex(new std::mutex())
  {
  }

public:

  virtual ~AgendaImpl()
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    // Delete all the ResponseMessage instances
    m_queue.clear();
  }

  virtual size_t size() const
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    return m_queue.size();
  }

  virtual bool empty() const
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    return m_queue.empty();
  }

  // Adds its parameter to every ResponseMessage in the queue.
  virtual void setSimulatorStartTime(timeval const &tym)
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    // If queue was sorted on entry, it'll remain sorted at exit.
    for (AgendaQueue::iterator it = m_queue.begin();
         it != m_queue.end();
         it++)
      it->first = it->first + tym;
  }
    
  // Only valid when not empty.
  virtual timeval const &nextResponseTime() const
  {
    static struct timeval sl_zero = {0, 0};
    std::lock_guard<std::mutex> g(*m_mutex);
    if (m_queue.empty())
      return sl_zero;
    return m_queue.front().first;
  }

  virtual ResponseMessage *popResponse()
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    if (m_queue.empty())
      return nullptr;

    ResponseMessage *msg = m_queue.front().second.release();
    m_queue.pop_front();
    return msg;
  }
  
  // Insert into list in earliest-first order.
  virtual void scheduleResponse(timeval tym, ResponseMessage *msg)
  {
    std::lock_guard<std::mutex> g(*m_mutex);
    AgendaQueue::iterator it = m_queue.begin();
    while (it != m_queue.end() && it->first < tym)
      ++it;
    m_queue.emplace(it, AgendaEntry(tym, msg));
  }
  
};

Agenda *makeAgenda()
{
  return new AgendaImpl();
}
