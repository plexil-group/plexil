/* -*- Mode: c++ -*-
   File: plexilsrc/interfaces/UdpAdapter/UdpAdapter.cc
*/

#include "UdpAdapter.hh"
#include "Debug.hh"             // debugMsg
#include "CoreExpressions.hh"   // BooleanVariable, etc.
#include "Node.hh"              // struct PLEXIL::Node
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"    // initUdpAdapter

//#include "Error.hh"
//#include "PlexilXmlParser.h"
//#include "ExecListenerFactory.hh"

// for UdpAdapter constructor
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace PLEXIL
{
  // Constructor
  //UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface) : InterfaceAdapter(execInterface)
  //{
  //  debugMsg("UdpAdapter::UdpAdapter(execInterface)", " called");
  //}

  // Constructor
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml)
    : InterfaceAdapter(execInterface, xml)
  {
    assertTrue(xml != NULL, "XML config file not found in UdpAdapter::UdpAdapter constructor");
    //debugMsg("UdpAdapter::UdpAdapter", " Using " << xml->Attribute("AdapterType"));
    debugMsg("UdpAdapter::UdpAdapter", " Using " << xml->Document());
  }

  // Destructor
  UdpAdapter::~UdpAdapter()
  {
    debugMsg("UdpAdapter::~UdpAdapter", " called");
  }

  // Initialize
  bool UdpAdapter::initialize()
  {
    debugMsg("UdpAdapter::initialize", " called");

    // Parse the message definitions in the XML configuration
    const TiXmlElement* xml = this->getXml();
    debugMsg("UdpAdapter::initialize", " xml = " << *xml);

    //this->registerAdapter();
    //m_execInterface.defaultRegisterAdapter(getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    debugMsg("UdpAdapter::initialize", " done");
    return true;
  }

  // Start method
  bool UdpAdapter::start()
  {
    debugMsg("UdpAdapter::start()", " called");
    return true;
  }

  // Stop method
  bool UdpAdapter::stop()
  {
    debugMsg("UdpAdapter::stop()", " called");
    return true;
  }

  // Reset method
  bool UdpAdapter::reset()
  {
    debugMsg("UdpAdapter::reset()", " called");
    return true;
  }

  // Shutdown method
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

  // Execute a Plexil Command
  void UdpAdapter::executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    debugMsg("UdpAdapter::executeCommand", " called for " << name.c_str());
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

  // SEND_MESSAGE_COMMAND
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

// Register the UdpAdapter
extern "C"
{
  void initUdpAdapter()
  {
    REGISTER_ADAPTER(PLEXIL::UdpAdapter, "UdpAdapter");
  }
}
