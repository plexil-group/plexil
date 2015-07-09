/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include <stddef.h>

namespace PLEXIL
{

  template <typename T> class QueueItem
  {
  public:
    QueueItem()
      : m_next(NULL)
    {
    }
    
    virtual ~QueueItem()
    {
    }
    
    QueueItem<T> *next() const
    {
      return m_next;
    }
    
    void setNext(QueueItem<T> *item)
    {
      m_next = item;
    }

  private:
    QueueItem<T> *m_next;
  };

  template <typename T> class LinkedQueue
  {
  private:
    QueueItem<T> *m_head;
    QueueItem<T> *m_tail;
    size_t m_count;

  public:
    LinkedQueue()
      : m_head(NULL),
        m_tail(NULL),
        m_count(0)
    {
    }
    
    virtual ~LinkedQueue()
    {
    }

    T *front() const
    {
      return static_cast<T *>(m_head); // may be null
    }

    size_t size() const
    {
      return m_count;
    }

    bool empty() const
    {
      return (m_head == NULL);
    }

    void pop()
    {
      if (!m_head)
        return; // empty, nothing to do
      QueueItem<T> *oldHead = m_head;
      if (m_head == m_tail)
        // Exactly one item was in queue, is now empty
        m_head = m_tail = NULL;
      else
        m_head = oldHead->next();
      --m_count; // better be 0 if empty!
      oldHead->setNext(NULL);
    }

    void push(T *item_as_T)
    {
      QueueItem<T> *item = static_cast<QueueItem<T> *>(item_as_T);
      item->setNext(NULL); // mark as end of queue
      if (!m_head)
        // Was empty
        m_head = item;
      else
        m_tail->setNext(item);
      m_tail = item;
      ++m_count;
    }
    
  };

}



#endif // LINKED_QUEUE_HH
