// Copyright (c) 2006-2023, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Reservable.hh"

#include "Debug.hh"
#include "Error.hh"
#include "NodeConnector.hh"

#include <algorithm>

namespace PLEXIL
{

  NodeConnector const *Reservable::getHolder() const
  {
    return m_holder;
  }

  Reservable::WaitQueue const &Reservable::getWaiters() const
  {
    return m_waiters;
  }

  bool Reservable::acquire(NodeConnector *node)
  {
    if (m_holder) {
      debugMsg("Reservable:acquire",
               ' ' << this << " by node "
               << node->getNodeId() << ' ' << node << " failed");
      reserve(node);
      return false;
    }
    m_holder = node;
    debugMsg("Reservable:acquire",
             ' ' << this << " by node "
             << m_holder->getNodeId() << ' ' << m_holder
             << " succeeded");
    return true;
  }

  bool Reservable::release(NodeConnector *node)
  {
    if (!m_holder) {
      debugMsg("Reservable:release", 
               ' ' << this << " releasing object which was not held");
      return false;
    }
    if (m_holder != node) {
      debugMsg("Reservable:release",
               ' ' << this << " invalid attempt by node "
               << node->getNodeId() << ' ' << node
               << ", which was not the holder");
      return false;
    }
    debugMsg("Reservable:release",
             ' ' << this << " by node " << node->getNodeId() << ' ' << node);
    m_holder = nullptr;
    for (NodeConnector *n : m_waiters)
      n->notifyResourceAvailable();
    return true;
  }

  bool Reservable::reserve(NodeConnector *node)
  {
    if (std::find(m_waiters.begin(), m_waiters.end(), node) == m_waiters.end()) {
      debugMsg("Reservable:reserve",
               ' ' << this << " node " << node->getNodeId() << ' ' << node);
      m_waiters.push_back(node);
      return true;
    }
    return false;
  }

  bool Reservable::cancelReservation(NodeConnector *node)
  {
    WaitQueue::iterator it = std::find(m_waiters.begin(), m_waiters.end(), node);
    if (it != m_waiters.end()) {
      debugMsg("Reservable:cancelReservation",
               ' ' << this << " node "
               << node->getNodeId() << ' ' << node);
      m_waiters.erase(it);
      return true;
    }
    return false;
  }

} // namespace PLEXIL
