/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "Mutable.hh"
#include "ExpressionListener.hh"

#include <algorithm> // for std::find()

namespace PLEXIL {

  //
  // Mutable
  //

  Mutable::Mutable()
    : Expression(),
      m_outgoingListeners(),
      m_activeCount(0)
  {
  }

  Mutable::~Mutable()
  {
    assertTrueMsg(m_outgoingListeners.empty(),
                  "Error: Expression '" << getId() << "' still has outgoing listeners.");
  }

  bool Mutable::isActive() const
  {
    return m_activeCount > 0;
  }

  void Mutable::activate()
  {
    bool changed = (!m_activeCount);
    ++m_activeCount;
    if (changed)
      this->handleActivate();
    else
      // Check for counter wrap only if active at entry
      assertTrueMsg(m_activeCount,
                    "Mutable::activate: Active counter overflowed for " << getId());
  }

  // No-op default method.
  void Mutable::handleActivate()
  {
  }

  void Mutable::deactivate()
  {
    assertTrueMsg(m_activeCount != 0,
                  "Attempted to deactivate expression " << getId() << " too many times.");
    if (--m_activeCount == 0)
      this->handleDeactivate();
  }

  // No-op default method.
  void Mutable::handleDeactivate()
  {
  }

  void Mutable::notifyChanged()
  {
    if (isActive())
      this->handleChange();
  }

  // No-op default method.
  void Mutable::handleChange()
  {
  }

  // *** TODO: determine whether we really need to check for duplicates ***
  void Mutable::addListener(ExpressionListenerId l)
  {
    // Don't add duplicates
    for (std::vector<ExpressionListenerId>::const_iterator it = m_outgoingListeners.begin();
         it != m_outgoingListeners.end();
         ++it)
      if (*it == l) {
#ifdef EXPRESSION_DEBUG
        debugMsg("Mutable:addListener", " ignoring duplicate " << l);
#endif
        return;
      }
    m_outgoingListeners.push_back(l);
  }

  void Mutable::removeListener(ExpressionListenerId l)
  {
    std::vector<ExpressionListenerId>::iterator it =
      std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), l);
    if (it == m_outgoingListeners.end()) {
#ifdef EXPRESSION_DEBUG
        debugMsg("Mutable:removeListener", " listener " << l << " not found");
#endif
        return;
    }
    m_outgoingListeners.erase(it);
  }

  void Mutable::publishChange()
  {
    if (isActive())
      for (std::vector<ExpressionListenerId>::iterator it = m_outgoingListeners.begin();
           it != m_outgoingListeners.end();
           ++it)
        (*it)->notifyChanged();
  }

} // namespace PLEXIL
