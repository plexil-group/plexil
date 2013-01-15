/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#include "ValueQueue.hh"

namespace PLEXIL
{

  //
  // ValueQueue implementation
  //
  
  ValueQueue::ValueQueue()
    : m_head(),
      m_tail(),
      m_freeList(),
      m_mutex(new ThreadMutex()),
      m_markCount(0)
  {
  }

  ValueQueue::~ValueQueue()
  {
    // Free the queue entries, if any
    while (m_head.isId())
      delete (QueueEntry*) behead();

    // Free the free-list entries, if any
    while (m_freeList.isId()) {
      QueueEntryId entry = m_freeList;
      m_freeList = entry->next;
      delete (QueueEntry*) entry;
    }

    // Free the mutex
    delete m_mutex;
  }

  void ValueQueue::enqueue(const ExpressionId & exp,
                           const Value& newValue)
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId e = allocate();
    e->type = queueEntry_RETURN_VALUE;
    e->expression = exp;
    e->value = newValue;
    insert(e);
  }

  void ValueQueue::enqueue(const State& state, 
                           const Value& newValue)
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId e = allocate();
    e->type = queueEntry_LOOKUP_VALUES;
    e->state = state;
    e->value = newValue;
    insert(e);
  }

  void ValueQueue::enqueue(PlexilNodeId newPlan,
                           const LabelStr & parent)
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId e = allocate();
    e->type = queueEntry_PLAN;
    e->plan = newPlan;
    e->parent = parent;
    insert(e);
  }

  void ValueQueue::enqueue(PlexilNodeId newLibraryNode)
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId e = allocate();
    e->type = queueEntry_LIBRARY;
    e->plan = newLibraryNode;
    insert(e);
  }

  QueueEntryType
  ValueQueue::dequeue(Value& newValue,
                      State& state,
                      ExpressionId& exp,
                      PlexilNodeId& plan,
                      LabelStr& planParent,
                      unsigned int& sequence)
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId e = behead();
    if (e.isNoId())
      return queueEntry_EMPTY;

    QueueEntryType eType = e->type;
    switch (eType) {
    case queueEntry_MARK:
      sequence = e->sequence;
      break;

    case queueEntry_LOOKUP_VALUES:
      newValue = e->value;
      state = e->state;
      break;

    case queueEntry_RETURN_VALUE:
      newValue = e->value;
      exp = e->expression;
      break;

    case queueEntry_PLAN:
      planParent = e->parent;
      // fall thru to library case

    case queueEntry_LIBRARY:
      plan = e->plan;
      break;

    default:
      assertTrue(ALWAYS_FAIL,
                 "ValueQueue::dequeue: Invalid queue entry");
      break;
    }

    // We're done with the head, return it to free list
    free(e);

    // Return the entry type
    return eType;
  }

  void ValueQueue::pop()
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId head = behead();
    if (head.isId())
      free(head);
  }

  bool ValueQueue::isEmpty() const
  {
    ThreadMutexGuard guard(*m_mutex);
    return m_head.isNoId();
  }
    
  unsigned int ValueQueue::mark()
  {
    ThreadMutexGuard guard(*m_mutex);
    QueueEntryId theMark = allocate();
    theMark->type = queueEntry_MARK;
    theMark->sequence = ++m_markCount;
    insert(theMark);
    return m_markCount;
  }

  //
  // Queue internal methods
  // All assume calling thread is holding the mutex
  //

  // Insertion
  void ValueQueue::insert(QueueEntryId entry)
  {
    if (m_head.isId()) {
      // Queue not empty
      m_tail->next = entry;
      m_tail = entry;
    }
    else {
      // Queue is empty
      m_head = m_tail = entry;
    }
  }

  // Removal
  ValueQueue::QueueEntryId ValueQueue::behead()
  {
    QueueEntryId result = m_head;
    if (result.isNoId())
      // already empty, nothing more to do
      return result;
    m_head = result->next;
    if (m_head.isNoId()) {
      // Queue is now empty, clear tail
      m_tail = m_head;
    }
    else {
      result->next = QueueEntryId::noId();
    }
    return result;
  }

  // Free list management

  ValueQueue::QueueEntryId ValueQueue::allocate()
  {
    if (m_freeList.isNoId()) 
      return (new QueueEntry())->getId();

    // Manage free list in last-in, first-out fashion
    QueueEntryId result = m_freeList;
    m_freeList = result->next;
    result->next = QueueEntryId::noId();
    return result;
  }
  
  void ValueQueue::free(QueueEntryId entry)
  {
    entry->clear();
    entry->next = m_freeList;
    m_freeList = entry;
  }

}
