/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "ExecListenerHub.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ExecListener.hh"
#include "ExecListenerFactory.hh"
#include "InterfaceSchema.hh"
#include "NodeTransition.hh"

#include "pugixml.hpp"

namespace PLEXIL
{
  ExecListenerHub::ExecListenerHub()
    : m_listeners(),
      m_transitions(),
      m_assignments()
  {
  }

  //
  // API to Exec
  //

  /**
   * @brief Notify that some set of nodes has changed state.
   * @param transitions Const reference to vector of transition records.
   */
  void
  ExecListenerHub::notifyOfTransitions(std::vector<NodeTransition> const &transitions)
  {
    m_transitions.insert(m_transitions.end(),
                         transitions.begin(), transitions.end());
  }

  /**
   * @brief Notify that a variable assignment has been performed.
   * @param dest The Expression being assigned to.
   * @param destName A string naming the destination.
   * @param value The value (in internal Exec representation) being assigned.
   * @note This is called synchronously from the inner loop of the Exec.
   *       Listeners should not do any I/O during this call.
   */
  void ExecListenerHub::notifyOfAssignment(Expression const *dest,
                                           std::string const &destName,
                                           Value const &value)
  {
    m_assignments.push_back(AssignmentRecord(dest, destName, value));
  }

  //
  // API to ExecApplication
  //
  
  /**
   * @brief Notify that a plan has been received by the Exec.
   * @param plan The intermediate representation of the plan.
   */
  void ExecListenerHub::notifyOfAddPlan(pugi::xml_node const plan)
  {
    for (ExecListenerPtr const &listener : m_listeners)
      listener->notifyOfAddPlan(plan);
  }

  /**
   * @brief Notify that a library node has been received by the Exec.
   * @param libNode The intermediate representation of the plan.
   */
  void ExecListenerHub::notifyOfAddLibrary(pugi::xml_node const libNode)
  {
    for (ExecListenerPtr const &listener : m_listeners)
      listener->notifyOfAddLibrary(libNode);
  }

  /**
   * @brief Notify that a step is complete and the listener
   *        may publish transitions and assignments.
   */
  void ExecListenerHub::stepComplete(unsigned int cycleNum)
  {
    for (ExecListenerPtr const &listener : m_listeners) {
      listener->notifyOfTransitions(m_transitions);
      for (AssignmentRecord const &assign : m_assignments)
        listener->notifyOfAssignment(assign.dest, assign.destName, assign.value);
    }
    m_transitions.clear();
    m_assignments.clear();
  }

  //
  // API to AdapterConfiguration
  //

  bool ExecListenerHub::constructListener(pugi::xml_node const configXml)
  {
    debugMsg("ExecListenerHub:constructListener",
             " constructing listener type \""
             << configXml.attribute(InterfaceSchema::LISTENER_TYPE_ATTR).value()
             << '"');
    ExecListener *listener = 
      ExecListenerFactory::createInstance(configXml);
    if (!listener) {
      warn("constructInterfaces: failed to construct listener type \""
           << configXml.attribute(InterfaceSchema::LISTENER_TYPE_ATTR).value()
           << '"');
      return false;
    }
    m_listeners.emplace_back(ExecListenerPtr(listener));
    return true;
  }

  /**
   * @brief Adds an Exec listener for publication of plan events.
   */
  void ExecListenerHub::addListener(ExecListener *listener)
  {
    check_error_1(listener);
    m_listeners.emplace_back(ExecListenerPtr(listener));
    debugMsg("ExecListenerHub:addListener", " called");
  }

  /**
   * @brief Perform listener-specific initialization.
   * @return true if successful, false otherwise.
   */
  bool ExecListenerHub::initialize()
  {
    debugMsg("ExecListenerHub:initialize", " entered");
    for (ExecListenerPtr const &listener : m_listeners) {
      if (!listener->initialize()) {
        debugMsg("ExecListenerHub:initialize",
                 " failed to initialize all Exec listeners, returning false");
        return false;
      }
    }
    debugMsg("ExecListenerHub:initialize", " returns true");
    return true;
  }

  /**
   * @brief Perform listener-specific startup.
   * @return true if successful, false otherwise.
   */
  bool ExecListenerHub::start()
  {
    for (ExecListenerPtr const &listener : m_listeners) {
      if (!listener->start()) {
        debugMsg("ExecListenerHub:start",
                 " failed to start all Exec listeners, returning false");
        return false; // stop at first failure
      }
    }
    debugMsg("ExecListenerHub:start", " returns true");
    return true;
  }

  /**
   * @brief Perform listener-specific actions to stop.
   */
  void ExecListenerHub::stop()
  {
    for (ExecListenerPtr const &listener : m_listeners)
      listener->stop();
  }

}
