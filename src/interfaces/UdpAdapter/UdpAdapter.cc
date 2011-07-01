/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "UdpAdapter.hh"
#include "Debug.hh"             // debugMsg
#include "CoreExpressions.hh"   // BooleanVariable, etc.
#include "Node.hh"              // struct PLEXIL::Node
#include "AdapterExecInterface.hh"
//#include "Error.hh"
//#include "PlexilXmlParser.h"

namespace PLEXIL
{
  /**
   * @brief Base constructor.
   */
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface) : InterfaceAdapter(execInterface)
  {
    debugMsg("UdpAdapter::UdpAdapter(execInterface)", " called");
  }

  /**
   * @brief Constructor w/ configuration XML.
   */
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml)
    : InterfaceAdapter(execInterface, xml)
  {
    debugMsg("UdpAdapter::UdpAdapter(execInterface, xml)", " called");
  }

  /**
   * @brief Destructor.
   */
  UdpAdapter::~UdpAdapter()
  {
    debugMsg("UdpAdapter::~UdpAdapter", " called");
  }

  /**
   * @brief Initialize and register the udp adapter.
   */
  bool UdpAdapter::initialize()
  {
    debugMsg("UdpAdapter::initialize", " called");
    //this->registerAdapter();
    //m_execInterface.defaultRegisterAdapter(getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    debugMsg("UdpAdapter::initialize", " done");
    return true;
  }

  /**
   * @brief Starts the adaptor, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool UdpAdapter::start()
  {
    debugMsg("UdpAdapter::start()", " called");
    return true;
  }

  /**
   * @brief Stops the adaptor.  
   * @return true if successful, false otherwise.
   */
  bool UdpAdapter::stop()
  {
    debugMsg("UdpAdapter::stop()", " called");
    return true;
  }

  /**
   * @brief Resets the adaptor.  
   * @return true if successful, false otherwise.
   */
  bool UdpAdapter::reset()
  {
    debugMsg("UdpAdapter::reset()", " called");
    return true;
  }

  /**
   * @brief Shuts down the adaptor, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  bool UdpAdapter::shutdown()
  {
    debugMsg("UdpAdapter::shutdown()", " called");
    return true;
  }

  void UdpAdapter::registerChangeLookup(const LookupKey& /* uniqueId */,
                                        const StateKey& /* stateKey */,
                                        const std::vector<double>& /* tolerances */)
  {
    debugMsg("UdpAdapter::registerChangeLookup(Lookupkey, Statekey, tolerances)", " called");
    debugMsg("ExternalInterface:udp", " registerChangeLookup called");
  }

  void UdpAdapter::unregisterChangeLookup(const LookupKey& /* uniqueId */)
  {
    debugMsg("UdpAdapter::unregisterChangeLookup(LookupKey)", " called");
    debugMsg("ExternalInterface:udp", " unregisterChangeLookup called");
  }

  void UdpAdapter::lookupNow(const StateKey& /* key */,
                             std::vector<double>& dest)
  {
    debugMsg("UdpAdapter::lookupNow(StateKey, dest)", " called");
    debugMsg("ExternalInterface:udp", " lookupNow called; returning UNKNOWN");
    dest[0] = Expression::UNKNOWN();
  }

  void UdpAdapter::sendPlannerUpdate(const NodeId& node,
                                     const std::map<double, double>& /* valuePairs */,
                                     ExpressionId ack)
  {
    debugMsg("UdpAdapter::sendPlannerUpdate(node, valuePairs, ack)", " called");
    debugMsg("ExternalInterface:udp", " sendPlannerUpdate called");

    // acknowledge updates
    debugMsg("ExternalInterface:udp",
	     " faking acknowledgment of update node '"
	     << node->getNodeId().toString()
	     << "'");
    m_execInterface.handleValueChange(ack, BooleanVariable::TRUE());
    m_execInterface.notifyOfExternalEvent();
  }

  void UdpAdapter::executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    debugMsg("UdpAdapter::executeCommand", " called for " << name.c_str());
    //debugMsg("ExternalInterface:udp", " executeCommand called");
    if (name == SEND_MESSAGE_COMMAND())
      executeSendMessageCommand(args, dest, ack);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    m_execInterface.notifyOfExternalEvent();
  }

  //abort the given command with the given arguments.  store the abort-complete into dest
  void UdpAdapter::invokeAbort(const LabelStr& /* name */, 
                               const std::list<double>& /* args */, 
                               ExpressionId /* dest */,
                               ExpressionId ack)
  {
    debugMsg("UdpAdapter::invokeAbort(name, args, dest, ack)", " called");
    debugMsg("ExternalInterface:udp", " invokeAbort called");
    m_execInterface.handleValueChange(ack, BooleanVariable::TRUE());
    m_execInterface.notifyOfExternalEvent();
  }

  //
  // Implementation methods
  //

  /**
   * @brief handles SEND_MESSAGE_COMMAND commands from the exec
   */
  void UdpAdapter::executeSendMessageCommand(const std::list<double>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // Check for one argument, the message
    assertTrueMsg(args.size() == 1, "UdpAdapter: The SendMessage command requires exactly one argument");
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: The argument to the SendMessage command, "
                  << Expression::valueToString(args.front())
                  << ", is not a string");
    LabelStr theMessage(args.front());
    debugMsg("UdpAdapter::executeSendMessageCommand", " SendMessage(\"" << theMessage.c_str() << "\")");
    //assertTrue(m_ipcFacade.publishMessage(theMessage) != IpcFacade::ERROR_SERIAL(), "Message publish failed");
    // store ack
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeSendMessageCommand", " message \"" << theMessage.c_str() << "\" sent.");
  }
}
