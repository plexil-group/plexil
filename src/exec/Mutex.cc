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

#include "Mutex.hh"

#include "Debug.hh"
#include "ExpressionListener.hh"
#include "Node.hh" 
#include "PlanError.hh"

#include <algorithm> // std::find
#include <map>
#include <memory> // std::unique_ptr

namespace PLEXIL
{

  Mutex::Mutex(char const *name)
    : m_name(name),
      m_holder(nullptr),
      m_listeners()
  {
    m_listeners.reserve(2); // at least one listener will register upon creation
  }

  Mutex::~Mutex()
  {
    // To be deleted; for debugging only
    check_error_2(m_listeners.empty(),
                  "Mutex destructor: listeners still registered");
  }

  bool Mutex::tryAcquire(Node const *acquirer)
  {
    // To be deleted; for debugging only
    assertTrue_2(acquirer, "Mutex::tryAcquire: null argument!");

    if (m_holder) {
      // Check for double acquisition
      Node const *ancestor = acquirer;
      do {
        checkPlanError(m_holder != ancestor,
                       "Error: Node " << acquirer->getNodeId()
                       << " attempting to acquire mutex " << m_name
                       << " already held by node's ancestor " << ancestor->getNodeId());
      } while ((ancestor = ancestor->getParent()));

      debugMsg("Mutex:tryAcquire",
               ' ' << m_name << " node " << acquirer->getNodeId() << " failed");
      return false; // sorry, Charlie
    }

    m_holder = acquirer;
    debugMsg("Mutex:tryAcquire",
             ' ' << m_name << " node " << acquirer->getNodeId() << " succeeded");
    return true;
  }

  void Mutex::release()
  {
    assertTrue_2(m_holder, "Releasing mutex which was not held");
    if (m_holder) {
      debugMsg("Mutex:release",' ' << m_name << " by node " << m_holder->getNodeId());
      m_holder = nullptr;
      notifyAvailable();
    }
  }

  void Mutex::addListener(ExpressionListener *l)
  {
    if (std::find(std::begin(m_listeners), std::end(m_listeners), l)
        == m_listeners.end())
      m_listeners.push_back(l);
  }

  void Mutex::removeListener(ExpressionListener *l)
  {
    std::vector<ExpressionListener *>::iterator it =
      std::find(std::begin(m_listeners), std::end(m_listeners), l);
    if (it != m_listeners.end())
      m_listeners.erase(it);
  }

  void Mutex::notifyAvailable()
  {
    debugMsg("Mutex:notifyAvailable", ' ' << m_name);
    for (ExpressionListener *l : m_listeners)
      l->notifyChanged();
  }

  //
  // Mutex management
  //

  typedef std::map<std::string, std::unique_ptr<Mutex> > MutexMap;

  static MutexMap s_mutexes;

  Mutex *createMutex(char const *name)
  {
    debugMsg("Mutex:ensureMutex", " constructing new mutex " << name);
    Mutex *result = new Mutex(name);
    s_mutexes.emplace(std::make_pair(std::string(name),
                                     std::unique_ptr<Mutex>(result)));
    return result;
  }

  Mutex *ensureMutex(char const *name)
  {
    assertTrue_2(name && *name,
                 "ensureMutex: null or empty name");
    MutexMap::const_iterator it = s_mutexes.find(name);
    if (it == s_mutexes.end())
      return createMutex(name);
    debugMsg("Mutex:ensureMutex", " returning existing mutex " << name);
    return it->second.get();
  }

}
