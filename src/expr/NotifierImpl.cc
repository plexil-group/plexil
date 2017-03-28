/* Copyright (c) 2006-2017, Universities Space Research Association (USRA).
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

#include "NotifierImpl.hh"

// #define LISTENER_DEBUG 1

#ifdef LISTENER_DEBUG
#include "Debug.hh"
#include <typeinfo>
#endif

#include "Error.hh"

#include <algorithm> // for std::find()

namespace PLEXIL {

  //
  // NotifierImpl
  //

  // Static initialization
#ifdef RECORD_EXPRESSION_STATS
  NotifierImpl *NotifierImpl::s_instanceList = NULL;
#endif

  NotifierImpl::NotifierImpl()
    : m_activeCount(0),
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

  NotifierImpl::~NotifierImpl()
  {

#ifdef LISTENER_DEBUG
    if (!m_outgoingListeners.empty()) {
      std::cerr << "*** " << this << " HAS " << m_outgoingListeners.size() << " OUTGOING LISTENERS:";
      for (ExpressionListener const *l : m_outgoingListeners)
        std::cerr << ' ' << l << ' ';
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

  bool NotifierImpl::isActive() const
  {
    return m_activeCount > 0;
  }

  void NotifierImpl::activate()
  {
    bool changed = (!m_activeCount);
    ++m_activeCount;
    if (changed)
      this->handleActivate();
    else
      // Check for counter wrap only if active at entry
      assertTrue_2(m_activeCount,
                   "NotifierImpl::activate: Active counter overflowed.");
  }

  // No-op default method.
  void NotifierImpl::handleActivate()
  {
  }

  void NotifierImpl::deactivate()
  {
    assertTrue_2(m_activeCount != 0,
                 "Attempted to deactivate expression too many times.");
    if (--m_activeCount == 0)
      this->handleDeactivate();
  }

  // No-op default method.
  void NotifierImpl::handleDeactivate()
  {
  }

  void NotifierImpl::notifyChanged()
  {
    if (isActive())
      this->handleChange();
  }

  // Default method.
  void NotifierImpl::handleChange()
  {
    publishChange();
  }

  // helpers for below
  static void addListenerToSubexprs(ExpressionListener *l,
                                    Expression *e)
  {
#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:addListenerToSubexprs",
             ' ' << e << " adding " << l);
#endif
    std::function<void (Expression *)> innerFn
      = ([l, &innerFn](Expression *x)
         { 
#ifdef LISTENER_DEBUG
           debugMsg("NotifierImpl:addListenerToSubexprs[1]",
                    ' ' << x << " adding " << l);
#endif
           if (x->hasListeners()) {
             // Subexprs already dealt with, just listen to this one
             x->addListenerInternal(l);
           }
           else {
             if (x->isPropagationSource()) {
               // This expression can independently generate notifications
               x->addListenerInternal(l);
             }
             // Either way, do its subexpressions
             x->doSubexprs(innerFn);
           }
         });
    e->doSubexprs(innerFn);
  }

  void NotifierImpl::addListener(ExpressionListener *ptr)
  {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:addListener",
               ' ' << *this << " adding " << ptr << ' ' << typeid(*ptr).name());
#endif
    if (m_outgoingListeners.empty()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:addListener",
               ' ' << this << " previously had no listeners, adding it to subexpressions");
#endif
      addListenerToSubexprs(this, this);
    }
    addListenerInternal(ptr);
  }

  // Have to check for duplicates, sigh.
  void NotifierImpl::addListenerInternal(ExpressionListener *ptr)
  {
    std::vector<ExpressionListener *>::iterator it =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (it != m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:addListener",
               ' ' << this << " listener " << ptr << " already present");
#endif
      return;
    }
    m_outgoingListeners.push_back(ptr);
#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:addListener",
             ' ' << this << " added " << ptr);
#endif
  }

  // Helper function
  static void removeListenerFromSubexprs(ExpressionListener *l,
                                         Expression *e)
  {
    std::function<void (Expression *)> innerFn
      = ([l, &innerFn](Expression *x)
         { 
#ifdef LISTENER_DEBUG
           debugMsg("NotifierImpl:removeListenerFromSubexprs[1]",
                    ' ' << x << " removing " << l);
#endif
           x->removeListener(l);
         });
    e->doSubexprs(innerFn);
  }

  void NotifierImpl::removeListener(ExpressionListener *ptr)
  {
#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:removeListener",
             ' ' << *this << " removing " << ptr << ' ' << typeid(*ptr).name());
#endif
    // Children might have this listener, so walk the whole tree
    removeListenerFromSubexprs(ptr, this);
    if (m_outgoingListeners.empty()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:removeListener", ' ' << this << " has no listeners");
#endif
      return;
    }
    std::vector<ExpressionListener *>::iterator it =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (it == m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:removeListener",
               ' ' << this << " listener " << ptr << " not found");
#endif
        return;
    }
    m_outgoingListeners.erase(it);
#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:removeListener", ' ' << this << " removed " << ptr);
#endif
    if (m_outgoingListeners.empty())
      removeListenerFromSubexprs(this, this);
  }

  bool NotifierImpl::hasListeners() const
  {
    return !m_outgoingListeners.empty();
  }

  void NotifierImpl::publishChange()
  {
    if (isActive())
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:publishChange", ' ' << *this);
#endif
      for (ExpressionListener *l : m_outgoingListeners) {
#ifdef LISTENER_DEBUG
        debugMsg("NotifierImpl:publishChange",
                 ' ' << this << " to listener " << typeid(*l).name() << ' ' << l);
#endif
        l->notifyChanged();
      }
  }

#ifdef RECORD_EXPRESSION_STATS
  NotifierImpl const *NotifierImpl::next() const
  {
    return m_next;
  }
  
  NotifierImpl const *NotifierImpl::getInstanceList()
  {
    return s_instanceList;
  }

  size_t NotifierImpl::getListenerCount() const
  {
    return m_outgoingListeners.size();
  }
#endif

} // namespace PLEXIL
