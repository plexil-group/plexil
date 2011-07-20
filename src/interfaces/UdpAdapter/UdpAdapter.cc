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
    debugMsg("UdpAdapter::UdpAdapter", " Using " << xml->Attribute("AdapterType"));
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
    // parse the XML message definitions
    parseMessageDefinitions(xml);
    //this->registerAdapter();
    //m_execInterface.defaultRegisterAdapter(getId());
    printMessageDefinitions();
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_UDP_MESSAGE_COMMAND()), getId());
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
    if (name == SEND_UDP_MESSAGE_COMMAND())
      executeSendUdpMessageCommand(args, dest, ack);
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

  // SEND_UDP_MESSAGE_COMMAND
  void UdpAdapter::executeSendUdpMessageCommand(const std::list<double>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // First arg is message name (which better match one of the defined messages...)
    assertTrueMsg(LabelStr::isString(args.front()), "UdpAdapter: the first paramater to SendUdpMessage command, "
                  << Expression::valueToString(args.front()) << ", is not a string");
    // Lookup the appropriate message in m_messages
    LabelStr msgName(args.front());
    MessageMap::iterator msg;
    msg=m_messages.find(msgName.c_str());
    // Set up the outgoing UDP buffer to be sent
    int length = msg->second.len;
    unsigned char* udp_buffer = new unsigned char[length];
    memset((char*)udp_buffer, 0, length); // zero out the buffer
    // Walk the parameters and encode them in the buffer to be sent out
    buildUdpBuffer(udp_buffer, msg->second, args, true);
    //printMessageContent(msgName, args);
    // Send the buffer to the given host:port
    // publishUdpMessage(buffer, msg->second);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeSendUdpMessageCommand", " message \"" << msgName.c_str() << "\" sent.");
    // Clean up some just (one hopes)
    delete udp_buffer;
  }

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

  //
  // XML Support
  //

  void UdpAdapter::parseMessageDefinitions(const TiXmlElement* xml)
  { // Walk this <Message/> element
    m_messages.clear();         // clear the old messages (if any)
    const char* default_incoming_port = NULL;
    const char* default_outgoing_port = NULL;
    default_incoming_port = xml->Attribute("default_incoming_port");
    default_outgoing_port = xml->Attribute("default_outgoing_port");
    if ( default_incoming_port ) m_default_incoming_port = atoi(default_incoming_port);
    if ( default_outgoing_port ) m_default_outgoing_port = atoi(default_outgoing_port);
    for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
      {
        UdpMessage msg;
        std::string name;
        std::string type;        // was: const char* type = NULL; std::string works with "+" below, c strings don't.
        const char* host = NULL; // needed for bool test below
        const char* port = NULL; // needed for bool test below
        const char* len = NULL;
        msg.name = name = child->Attribute("name"); // for debugging message below
        msg.type = type = child->Attribute("type"); // for debugging message below
        host = child->Attribute("host");
        port = child->Attribute("port");
        // must be either incoming or outgoing
        assertTrue((type=="outgoing" || type=="incoming"),
                   "type '" + type + "' is not one of 'incoming' or 'outgoing' in <Message name=\"" + name + "\"/>");
        msg.host = host ? host : "localhost";   // record the host given or a default
        // use either the given port, the (appropriate) default port, or singal an error
        if (type=="incoming")
          {
            assertTrue((default_incoming_port || port),
                       "No port given, and no default port for incoming <Message name=\"" + name + "\"/>");
          }
        else
          {
            assertTrue((default_outgoing_port || port),
                       "No port given, and no default port for outgoing <Message name=\"" + name + "\"/>");
          }
        msg.port = port ? atoi(port) : (type=="incoming") ? m_default_incoming_port : m_default_outgoing_port;
        //assertTrue(msg.port != NULL, "foo");
        //atoi(NULL);
        if ( port ) msg.port = atoi(port); // record the port if there is one
        // Walk the <Parameter/> elements of this <Message/>, if any
        for (const TiXmlElement* param = child->FirstChildElement(); param != NULL; param = param->NextSiblingElement())
          {
            Parameter arg;
            const char* param_name = NULL;
            param_name = param->Attribute("name");
            // what about strings? -- fixed length to start with I guess
            len = param->Attribute("length");
            assertTrue((len), "No parameter length given for <Message name=\"" + name + "\"/>");
            arg.len = atoi(len);
            //std::cout << "here" << std::endl;
            if (param_name) arg.name = param_name; // only assign it if it exists
            arg.type = param->Attribute("type");
            msg.len += arg.len;
            msg.parameters.push_back(arg);
            //msg.parameters.push_back(param->Attribute("type"));   // record the type
            //msg.parameters.push_back(param->Attribute("length")); // and length
          }
        m_messages[child->Attribute("name")]=msg; // record the message with the name as the key
      }
  }
  
  void UdpAdapter::printMessageDefinitions()
  { // print all of the stuff in m_message for debugging
    MessageMap::iterator msg;
    for (msg=m_messages.begin(); msg != m_messages.end(); msg++)
      {
        std::cout << "Message: " << msg->first;
        std::cout << " (" << msg->second.type << ")";
        std::list<Parameter>::iterator param;
        std::cout << ", Parameters:";
        for (param=msg->second.parameters.begin(); param != msg->second.parameters.end(); param++)
          {
            //std::cout << " " << (*param); // alternating type and length values
            std::cout << " \"" << param->name << "\"";
            std::cout << " " << param->type;
            std::cout << " " << param->len << ",";
          }
        std::cout << " length: " << msg->second.len;
        std::cout << ", host: " << msg->second.host << ", port: " << msg->second.port << std::endl;
      }
  }

  void UdpAdapter::buildUdpBuffer(unsigned char* buffer,
                                  const UdpMessage& msg,
                                  const std::list<double>& args,
                                  bool debug)
  {
    //print_buffer(buffer, msg.len);
    std::list<double>::const_iterator it;
    std::list<Parameter>::const_iterator param;
    int start_index = 0;        // where in the buffer to write
    // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
    for (param=msg.parameters.begin(), it=args.begin() ; param != msg.parameters.end() ; param++, it++)
      {
        int len = param->len;
        std::string type = param->type;
        double plexil_val = *it;
        // The parameter passed will be one of these two
        std::string str_val;
        double num_val;
        if (debug) std::cout << "start_index: " << start_index << ", ";
        // Only encode 32 bit entities
        if (type.compare("int") == 0)
          {
            if (debug) std::cout << "int: " << plexil_val;
            encode_long_int(plexil_val, buffer, start_index);
          }
        else if (type.compare("float") == 0)
          {
            if (debug) std::cout << "float: " << plexil_val;
            encode_float(plexil_val, buffer, start_index);
          }
        else if (type.compare("bool") == 0) // these are 64 bits in Plexil
          {
            if (debug) std::cout << "bool: " << plexil_val;
            encode_long_int(plexil_val, buffer, start_index);
          }
        else
          {
            if (debug) std::cout << "string: \"" << LabelStr(plexil_val).c_str() << "\"";
            std::string str = LabelStr(plexil_val).c_str();
            //str.copy((char*)&buffer[start_index], str.length(), 0);
            encode_string(str, buffer, start_index);
          }
        if (debug) std::cout << std::endl;
        start_index += len;
      }
    if (debug) print_buffer(buffer, msg.len);
  }

  void UdpAdapter::printMessageContent(const LabelStr& name, const std::list<double>& args)
  {
    //debugMsg("UdpAdapter::printMessageContent", " args.size() == " << args.size());
    std::list<double>::const_iterator it;
    std::cout << "Message: " << name.c_str() << ", Params:";
    for (it=args.begin(); it != args.end(); it++)
      {
        // Real, Integer, Boolean, String (and Array, maybe...)
        // Integers and Booleans are represented as Real (oops...)
        double param = *it;
        std::string str;
        double num;
        std::cout << " ";
        if (LabelStr::isString(param)) // Extract strings
          {
            str = LabelStr(param).c_str();
            std::cout << "\"" << str << "\"";
          }
        else // Extract numbers (bool, float, int)
          {
            num = param;
            std::cout << num;
          }
      }
    std::cout << std::endl;
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
