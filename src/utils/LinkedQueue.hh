/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#ifndef LINKED_QUEUE_HH
#define LINKED_QUEUE_HH

namespace PLEXIL
{
  //*
  // @class LinkedQueue
  // @brief Simple unidirectional linked-list queue implementation.
  //        Participant classes must only provide two member functions:
  //        T *next() const and T **nextPtr()
  //

  template <typename T> class LinkedQueue
  {
  private:
    T *m_head;
    T *m_tail;
    size_t m_count;

  public:
    LinkedQueue()
      : m_head(nullptr),
        m_tail(nullptr),
        m_count(0)
    {
    }
    
    ~LinkedQueue()
    {
    }

    T *front() const
    {
      return m_head; // may be null
    }

    size_t size() const
    {
      return m_count;
    }

    bool empty() const
    {
      return (m_head == nullptr);
    }

    void pop()
    {
      if (empty())
        return; // empty, nothing to do

      if (m_head == m_tail)
        // Exactly one item was in queue, is now empty
        m_head = m_tail = nullptr;
      else
        m_head = m_head->next();

      --m_count; // better be 0 if empty!
    }

    void push(T *item)
    {
      *(item->nextPtr()) = nullptr; // mark as end of queue
      if (empty())
        // Was empty
        m_head = item;
      else
        *(m_tail->nextPtr()) = item;
      m_tail = item;
      ++m_count;
    }

    void remove(T *item)
    {
      if (empty() || item == nullptr)
        return;
      T **ptr  = &m_head;
      T *nxt  = m_head;
      while (nullptr != nxt) {
        if (item == nxt) {
          // unlink it
          *ptr = item->next();
          *(item->nextPtr()) = nullptr;
          if (empty())
            // Removed only item in queue
            m_tail = nullptr;
          --m_count;
          return;
        }
        ptr = nxt->nextPtr();
        nxt = *ptr;
      }
      // Can fall through to here without finding item
    }

    void clear()
    {
      m_head = m_tail = nullptr;
      m_count = 0;
    }

  };

}

#endif // LINKED_QUEUE_HH
