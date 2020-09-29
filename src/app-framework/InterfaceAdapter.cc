/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "InterfaceAdapter.hh"

#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "State.hh" // for operator<<() used in debugMsg

namespace PLEXIL
{

  //
  // Constructors
  //

  InterfaceAdapter::InterfaceAdapter(AdapterExecInterface& execInterface)
    : m_execInterface(execInterface),
      m_xml()
  {
  }

  InterfaceAdapter::InterfaceAdapter(AdapterExecInterface& execInterface, 
                                     pugi::xml_node const xml)
    : m_execInterface(execInterface),
      m_xml(xml)
  {
  }

  //
  // Destructor
  //

  InterfaceAdapter::~InterfaceAdapter()
  {
  }

  //
  // Default methods for InterfaceManager API
  // All are no-ops
  //

  // For backwards compatibility with older API
  bool InterfaceAdapter::initialize()
  {
    return true;
  }

  bool InterfaceAdapter::initialize(AdapterConfiguration * /* ignored */)
  {
    return this->initialize();
  }

  bool InterfaceAdapter::start()
  {
    return true;
  }

  bool InterfaceAdapter::stop()
  {
    return true;
  }

  bool InterfaceAdapter::reset()
  {
    return true;
  }

  bool InterfaceAdapter::shutdown()
  {
    return true;
  }

  void InterfaceAdapter::lookupNow(State const & state, StateCacheEntry & /* cacheEntry */)
  {
    debugMsg("InterfaceAdapter:lookupNow", " default method called for state " << state);
  }

  void InterfaceAdapter::subscribe(const State& state)
  {
    debugMsg("InterfaceAdapter:subscribe", " default method called for state " << state);
  }

  void InterfaceAdapter::unsubscribe(const State& state)
  {
    debugMsg("InterfaceAdapter:unsubscribe", " default method called for state " << state);
  }

  void InterfaceAdapter::setThresholds(const State& state, double /* hi */, double /* lo */)
  {
    debugMsg("InterfaceAdapter:setThresholds", " default method called for state " << state);
  }

  void InterfaceAdapter::setThresholds(const State& state, int32_t /* hi */, int32_t /* lo */)
  {
    debugMsg("InterfaceAdapter:setThresholds", " default method called");
  }

  void InterfaceAdapter::sendPlannerUpdate(Update * /* upd */)
  {
    debugMsg("InterfaceAdapter:sendPlannerUpdate", " default method called");
  }

  // Send a valid command handle value so the node can finish

  void InterfaceAdapter::executeCommand(Command *cmd)
  {
    debugMsg("InterfaceAdapter:executeCommand", " default method called");
    g_execInterface->handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    g_execInterface->notifyOfExternalEvent();
  }

  // Send a valid abort ack so the node can finish

  void InterfaceAdapter::invokeAbort(Command *cmd)
  {
    debugMsg("InterfaceAdapter:invokeAbort", " default method called");
    g_execInterface->handleCommandAbortAck(cmd, true);
    g_execInterface->notifyOfExternalEvent();
  }

}
