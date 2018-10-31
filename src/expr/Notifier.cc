/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "Notifier.hh"

#include "Error.hh"

#include <algorithm> // for std::find()

#ifdef LISTENER_DEBUG
#include "Debug.hh"
#include <iostream>
#include <typeinfo>
#endif

namespace PLEXIL
{

  // Static initialization
#ifdef RECORD_EXPRESSION_STATS
  Notifier *Notifier::s_instanceList = NULL;
#endif

  Notifier::Notifier()
    : Listenable(),
      m_activeCount(0),
      m_outgoingListeners()
  {
#ifdef RECORD_EXPRESSION_STATS
    m_prev = NULL;
    m_next = s_instanceList;
    s_instanceList = this;
    if (m_next)
      m_next->m_prev = this;
#endif
  }

  Notifier::~Notifier()
  {
#ifdef LISTENER_DEBUG
    if (!m_outgoingListeners.empty()) {
      std::cerr << "*** " << (Expression *) this
                << " HAS " << m_outgoingListeners.size() << " OUTGOING LISTENERS:";
      for (std::vector<ExpressionListener *>::const_iterator it = m_outgoingListeners.begin();
           it != m_outgoingListeners.end();
           ++it)
        std::cerr << ' ' << *it << ' ';
      std::cerr << std::endl;
    }
#endif

    assertTrue_2(m_outgoingListeners.empty(),
                 "Error: Expression still has outgoing listeners.");

#ifdef RECORD_EXPRESSION_STATS
    // Delete this from instance list
    if (m_prev)
      m_prev->m_next = m_next; // may be null
    else
      s_instanceList = m_next; // this was newest - next may be null

    if (m_next)
      m_next->m_prev = m_prev; // may be null
#endif
  }

  bool Notifier::isActive() const
  {
    return m_activeCount > 0;
  }

  bool Notifier::hasListeners() const
  {
    return !m_outgoingListeners.empty();
  }

  void Notifier::activate()
  {
    bool changed = !m_activeCount;
    ++m_activeCount;
    if (changed)
      this->handleActivate();
    else
      // Check for counter wrap only if active at entry
      assertTrue_2(m_activeCount,
                   "Notifier::activate: Active counter overflowed.");
  }

  // No-op default method.
  void Notifier::handleActivate()
  {
  }

  void Notifier::deactivate()
  {
    assertTrue_2(m_activeCount != 0,
                 "Attempted to deactivate expression too many times.");
    if (--m_activeCount == 0)
      this->handleDeactivate();
  }

  // No-op default method.
  void Notifier::handleDeactivate()
  {
  }

  void Notifier::addListener(ExpressionListener *ptr)
  {
    // Have to check for duplicates, sigh.
    std::vector<ExpressionListener *>::const_iterator iter =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (iter != m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("Notifier:addListener",
               ' ' << (Expression *) this << " listener " << ptr << " already present");
#endif
      return;
    }
    m_outgoingListeners.push_back(ptr);
#ifdef LISTENER_DEBUG
    debugMsg("Notifier:addListener",
             ' ' << (Expression *) this << " added " << ptr);
#endif
  }
  
  void Notifier::removeListener(ExpressionListener *ptr)
  {
    if (m_outgoingListeners.empty()) {
#ifdef LISTENER_DEBUG
      debugMsg("Notifier:removeListener",
               ' ' << (Expression *) this << " has no listeners");
#endif
      return;
    }

#ifdef LISTENER_DEBUG
    debugMsg("Notifier:removeListener",
             ' ' << (Expression *) this << ' ' << *this
             << " removing " << ptr << ' ' << typeid(*ptr).name());
#endif
    std::vector<ExpressionListener *>::iterator iter =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (iter == m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("Notifier:removeListener",
               ' ' << (Expression *) this << " listener " << ptr << " not found");
#endif
    }
    else {
      m_outgoingListeners.erase(iter);
#ifdef LISTENER_DEBUG
      debugMsg("Notifier:removeListener",
               ' ' << (Expression *) this << " removed " << ptr);
#endif
    }
  }
 
  void Notifier::publishChange()
  {
    if (isActive())
      for (std::vector<ExpressionListener *>::iterator it = m_outgoingListeners.begin();
           it != m_outgoingListeners.end();
           ++it)
        (*it)->notifyChanged();
  }

#ifdef RECORD_EXPRESSION_STATS
  Notifier const *Notifier::next() const
  {
    return m_next;
  }
  
  Notifier const *Notifier::getInstanceList()
  {
    return s_instanceList;
  }

  size_t Notifier::getListenerCount() const
  {
    return m_outgoingListeners.size();
  }
#endif

}
