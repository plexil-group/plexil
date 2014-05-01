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

#include "CoreExpressions.hh"
#include "InterfaceAdapter.hh"
#include "AdapterExecInterface.hh"
#include "Command.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LabelStr.hh"
#include "Node.hh"
#include "StateCache.hh"
#include "Variable.hh"

namespace PLEXIL
{

  //
  // Constructors
  //

  InterfaceAdapter::InterfaceAdapter(AdapterExecInterface& execInterface)
    : m_execInterface(execInterface),
      m_xml(),
      m_id(this)
  {
  }

  InterfaceAdapter::InterfaceAdapter(AdapterExecInterface& execInterface, 
                                     const pugi::xml_node& xml)
    : m_execInterface(execInterface),
      m_xml(xml),
      m_id(this)
  {
  }

  //
  // Destructor
  //

  InterfaceAdapter::~InterfaceAdapter()
  {
    m_id.remove();
  }


  //
  // Default methods for InterfaceManager API
  //

  Value InterfaceAdapter::lookupNow(const State& /* state */)
  {
	warn("lookupNow: default method called - returning UNKNOWN.");
    return UNKNOWN();
  }

  void InterfaceAdapter::subscribe(const State& /* state */)
  {
	warn("subscribe: default method called -- doing nothing.");
  }

  void InterfaceAdapter::unsubscribe(const State& /* state */)
  {
	warn("unsubscribe: default method called -- doing nothing.");
  }

  void InterfaceAdapter::setThresholds(const State& /* state */, double /* hi */, double /* lo */)
  {
    debugMsg("InterfaceAdapter:setThresholds", " default method called");
  }

  void InterfaceAdapter::sendPlannerUpdate(const NodeId& /* node */,
                                           const std::map<std::string, Value>& /* valuePairs */,
                                           ExpressionId ack)
  {
	warn("sendPlannerUpdate: default method called -- just acknowledging request.");
	ack->setValue(true);
  }


  // This default method is a wrapper for backward compatibility.
  void InterfaceAdapter::executeCommand(const CommandId& cmd)
  {
    this->executeCommand(cmd->getName(), cmd->getArgValues(), cmd->getDest(), cmd->getAck());
  }

  // executes a command with the given arguments
  void InterfaceAdapter::executeCommand(const LabelStr& /* name */,
                                        const std::vector<Value>& /* args */,
                                        ExpressionId /* dest */,
                                        ExpressionId ack)
  {
	warn("executeCommand: default method called -- acknowledging command as failed.");
ack->setValue(CommandHandleVariable::COMMAND_FAILED());
  }

  // Abort the given command. Call method below for compatibility.
  void InterfaceAdapter::invokeAbort(const CommandId& cmd)
  {
    this->invokeAbort(cmd->getName(), cmd->getArgValues(), cmd->getAbortComplete(), cmd->getAck());
  }

  //abort the given command with the given arguments.
  void InterfaceAdapter::invokeAbort(const LabelStr& name, 
                                     const std::vector<Value>& /* args */, 
                                     ExpressionId abort_ack,
                                     ExpressionId /* cmd_ack */)
  {
	warn("Default and deprecated method called. "
		 "Please use invokeAbort(CommandId)");
    debugMsg("InterfaceAdapter:invokeAbort", "Aborted command " << name);
	abort_ack->setValue(true);
  }

  /**
   * @brief Register this adapter based on its XML configuration data.
   * @note The adapter is presumed to be fully initialized and working at the time of this call.
   * @note This is a default method; adapters are free to override it.
   */
  void InterfaceAdapter::registerAdapter()
  {
    m_execInterface.defaultRegisterAdapter(m_id);
  }

}
