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

#include "NotifierImpl.hh"

// This definition should be commented out for production use.
// Uncomment it if you need to trace setup or teardown of, or propagation
// through, the notification graph.
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

  bool NotifierImpl::isActive() const
  {
    return m_activeCount > 0;
  }

  void NotifierImpl::activate()
  {
    bool changed = !m_activeCount;
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

  //
  // Expression listener graph construction and teardown
  // 

  //
  // In order to reduce memory usage and graph propagation delays, we try to
  // minimize the number of listeners added to expressions.
  //
  // There are three cases where we want to add a listener to an expression:
  //  1. Root expression, i.e. a node condition. This is the expression on which
  //     addListener() is explicitly called during plan loading.
  //  2. Interior subexpression whose value can change independently of its
  //     parameters (e.g. Lookup, random number generator).
  //  3. Leaf expression that can change, i.e. variable.
  //
  // We only add listeners to expressions that are propagation sources,
  // whether they are leaves or interior nodes of the tree.

  //
  // Internal helper for addListener() method
  //
  
  class AddListenerHelper : public ExprUnaryOperator
  {
  public:
    AddListenerHelper(ExpressionListener *listener)
      : l(listener)
    {
    }

    virtual ~AddListenerHelper()
    {
    }

    virtual void operator()(Expression *exp) const
    {
      if (exp->isPropagationSource())
        // This expression can independently generate notifications,
        // so add requested listener here
        exp->addListener(l);
      else 
        // Recurse through subexpressions
        exp->doSubexprs(*this);
    }

  private:
    ExpressionListener *l;
  };

  // Should only be called on expression root and internal nodes that are propagation sources.
  void NotifierImpl::addListener(ExpressionListener *ptr)
  {
    if (!hasListeners())
      doSubexprs(AddListenerHelper(this));
    addListenerInternal(ptr);
  }

  // Internal member function, only meant to be called from AddListenerHelper.
  void NotifierImpl::addListenerInternal(ExpressionListener *ptr)
  {
    // Have to check for duplicates, sigh.
    std::vector<ExpressionListener *>::const_iterator iter =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (iter != m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:addListener",
               ' ' << (Expression *) this << " listener " << ptr << " already present");
#endif
      return;
    }
    m_outgoingListeners.push_back(ptr);
#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:addListener",
             ' ' << (Expression *) this << " added " << ptr);
#endif
  }

  //
  // Internal helper for removeListener() method
  //

  class RemoveListenerHelper : public ExprUnaryOperator
  {
  public:
    RemoveListenerHelper(ExpressionListener *listener)
      : l(listener)
    {
    }

    virtual ~RemoveListenerHelper()
    {
    }

    virtual void operator()(Expression *exp) const
    {
      if (exp->isPropagationSource())
        exp->removeListener(l);
      else
        exp->doSubexprs(*this);
    }

  private:
    ExpressionListener *l;
  };

  void NotifierImpl::removeListener(ExpressionListener *ptr)
  {
    removeListenerInternal(ptr);
    if (!hasListeners())
      doSubexprs(RemoveListenerHelper(this));
  }
  
  void NotifierImpl::removeListenerInternal(ExpressionListener *ptr)
  {
    if (m_outgoingListeners.empty()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:removeListener",
               ' ' << (Expression *) this << " has no listeners");
#endif
      return;
    }

#ifdef LISTENER_DEBUG
    debugMsg("NotifierImpl:removeListener",
             ' ' << (Expression *) this << ' ' << *this
             << " removing " << ptr << ' ' << typeid(*ptr).name());
#endif
    std::vector<ExpressionListener *>::iterator iter =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), ptr);
    if (iter == m_outgoingListeners.end()) {
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:removeListener",
               ' ' << (Expression *) this << " listener " << ptr << " not found");
#endif
    }
    else {
      m_outgoingListeners.erase(iter);
#ifdef LISTENER_DEBUG
      debugMsg("NotifierImpl:removeListener",
               ' ' << (Expression *) this << " removed " << ptr);
#endif
    }
  }

  bool NotifierImpl::hasListeners() const
  {
    return !m_outgoingListeners.empty();
  }
  
  void NotifierImpl::publishChange()
  {
    if (isActive())
      for (std::vector<ExpressionListener *>::iterator it = m_outgoingListeners.begin();
           it != m_outgoingListeners.end();
           ++it)
        (*it)->notifyChanged();
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
