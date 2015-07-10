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
  // Forward declaration
  template <typename T> class LinkedQueue; 

  template <typename T> class QueueItem
  {
    friend class LinkedQueue<T>;
    
  public:
    QueueItem()
      : m_next(NULL)
    {
    }
    
    virtual ~QueueItem()
    {
    }

    T *next() const
    {
      return static_cast<T *>(m_next);
    }

  private:
    QueueItem<T> *m_next;
  };

  template <typename T> class LinkedQueue :
    public QueueItem<T>
  {
  private:
    QueueItem<T> *m_tail;
    size_t m_count;

  public:
    LinkedQueue()
      : QueueItem<T>(),
        m_tail(NULL),
        m_count(0)
    {
    }
    
    ~LinkedQueue()
    {
    }

    T *front() const
    {
      return static_cast<T *>(this->m_next); // may be null
    }

    size_t size() const
    {
      return m_count;
    }

    bool empty() const
    {
      return (this->m_next == NULL);
    }

    void pop()
    {
      if (!this->m_next)
        return; // empty, nothing to do

      if (this->m_next == m_tail)
        // Exactly one item was in queue, is now empty
        this->m_next = m_tail = NULL;
      else
        this->m_next = this->m_next->m_next;
      --m_count; // better be 0 if empty!
    }

    void push(T *item_as_T)
    {
      QueueItem<T> *item = static_cast<QueueItem<T> *>(item_as_T);
      item->m_next = NULL; // mark as end of queue
      if (!this->m_next)
        // Was empty
        this->m_next = item;
      else
        m_tail->m_next = item;
      m_tail = item;
      ++m_count;
    }

    void remove(T *item_as_T)
    {
      if (!item_as_T || !this->m_next)
        return;
      QueueItem<T> *item = static_cast<QueueItem<T> *>(item_as_T);
      QueueItem<T> *ptr =  static_cast<QueueItem<T> *>(this);
      while (ptr->m_next) {
        QueueItem<T> *nxt = ptr->m_next;
        if (item == nxt) {
          // unlink it
          ptr->m_next = nxt->m_next;
          if (this->m_next == NULL)
            // Deleted only item in queue
            m_tail = NULL;
          --m_count;
          return;
        }
        ptr = ptr->m_next;
      }
      // Can fall through to here without finding item
    }

    // TODO: null out links of items?
    void clear()
    {
      this->m_next = m_tail = NULL;
      m_count = 0;
    }

  };

}

#endif // LINKED_QUEUE_HH
