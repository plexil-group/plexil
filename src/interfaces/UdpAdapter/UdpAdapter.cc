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
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface),
      m_messageQueues(execInterface)                                                                
  {
    debugMsg("UdpAdapter::UdpAdapter(execInterface)", " called");
  }

  // Constructor
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml)
    : InterfaceAdapter(execInterface, xml),
      m_messageQueues(execInterface)
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
    m_execInterface.registerCommandInterface(LabelStr(RECEIVE_UDP_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(RECEIVE_COMMAND_COMMAND()), getId());
    debugMsg("UdpAdapter::initialize", " done");
    return true;
  }

  // Start method
  bool UdpAdapter::start()
  {
    debugMsg("UdpAdapter::start()", " called");
    // Start the UDP listener thread
    return true;
  }

  // Stop method
  bool UdpAdapter::stop()
  {
    debugMsg("UdpAdapter::stop()", " called");
    // Stop the UDP listener thread
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
    debugMsg("UdpAdapter::executeCommand", " " << name.toString() << " (dest==" << dest << ", ack==" << ack << ")");
    if (name == SEND_MESSAGE_COMMAND())
      executeSendMessageCommand(args, dest, ack);
    else if (name == SEND_UDP_MESSAGE_COMMAND())
      executeSendUdpMessageCommand(args, dest, ack);
    else if (name == RECEIVE_UDP_MESSAGE_COMMAND())
      executeReceiveUdpCommand(args, dest, ack);
    else if (name == RECEIVE_COMMAND_COMMAND())
      executeReceiveCommandCommand(args, dest, ack);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeCommand", " " << name.c_str() << " done.");
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

  // RECEIVE_COMMAND_COMMAND
  void UdpAdapter::executeReceiveCommandCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    assertTrueMsg(args.size() == 1,
                  "UdpAdapter: The " << RECEIVE_COMMAND_COMMAND().c_str() << " command requires exactly one argument");
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: The argument to the " << RECEIVE_COMMAND_COMMAND().c_str()
                  << " command, " << Expression::valueToString(args.front())
                  << ", is not a string");
    LabelStr command(formatMessageName(args.front(), RECEIVE_COMMAND_COMMAND()));
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    // Set up the thread on which the message may/will eventually be received
    int status = -1;
    status = startUdpMessageReceiver(LabelStr(args.front()), dest, ack);
    debugMsg("UdpAdapter::executeCommand", " message handler for \"" << command.c_str() << "\" registered.");
  }

  // RECEIVE_UDP_MESSAGE_COMMAND
  void UdpAdapter::executeReceiveUdpCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    // Called when node _starts_ executing, so, record the message and args so that they can be filled in
    // if and when a UDP message comes in the fulfill this expectation.
    // First arg is message name (which better match one of the defined messages...)
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: the first paramater to ReceiveUdpMessage command, "
                  << Expression::valueToString(args.front()) << ", is not a string");
    LabelStr command(args.front());
    debugMsg("UdpAdapter::executeReceiveUdpCommand", " " << command.c_str() << ", dest==" << dest
             << ", ack==" << ack << ", args.size()==" << args.size());
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeReceiveUdpCommand", " handler for \"" << command.c_str() << "\" registered.");
    // Set up the expectation (dest) for the message on which this node is waiting
    //MessageMap::iterator msg;
    //msg=m_messages.find(msgName.c_str()); // get the appropriate message definition
    //assertTrueMsg((msg->second.parameters.size()==1+args.size()), "executeReceiveUdpMessageCommand, parameters and args do not"
    //              << " match, parameters.size()==" << msg->second.parameters.size() << ", args.size()==" << args.size());
    // Record the variables that will be filled in when this message is received XXXX
    //std::list<double>::const_iterator it;
    //std::cout << "  args:";
    //for(it = args.begin() ; it != args.end() ; it++) // skip the message name
    //  {
    //    msg->second.variables.push_back(*it);
    //    std::cout << " " << Expression::valueToString(*it);
    //  }
    //std::cout << std::endl;
    // Set up the listener for this message.  Once set up, the listener will call handleValueChange
    // and notifyOfExternalEvent event if/when the expected message (and its parameters) is received.

    // dest==noId is correct -- dest is only used for a return value!
    // the args list is also correct -- I have to keep the list around and reset their values with
    // the message parameters (I think)
  }

  // SEND_UDP_MESSAGE_COMMAND
  void UdpAdapter::executeSendUdpMessageCommand(const std::list<double>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // First arg is message name (which better match one of the defined messages...)
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: the first paramater to SendUdpMessage command, "
                  << Expression::valueToString(args.front()) << ", is not a string");
    bool debug = true;
    // Lookup the appropriate message in the message definitions in m_messages
    LabelStr msgName(args.front());
    debugMsg("UdpAdapter::executeSendUdpMessageCommand", " called for " << msgName.c_str());
    //printMessageContent(msgName, args);
    MessageMap::iterator msg;
    msg=m_messages.find(msgName.c_str());
    // Set up the outgoing UDP buffer to be sent
    int length = msg->second.len;
    unsigned char* udp_buffer = new unsigned char[length]; // fixed length to start with
    memset((char*)udp_buffer, 0, length); // zero out the buffer
    // Walk the parameters and encode them in the buffer to be sent out
    buildUdpBuffer(udp_buffer, msg->second, args, debug);
    // Send the buffer to the given host:port
    int status = -1;
    status = sendUdpMessage(udp_buffer, msg->second, debug);
    debugMsg("UdpAdapter::executeSendUdpMessageCommand", " sendUdpMessage returned " << status << " (bytes sent)");
    // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    // Clean up some (one hopes)
    delete udp_buffer;
  }

  // GET_PARAMETER_COMMAND
  //void UdpAdapter::executeGetParameterCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  //{
  //  assertTrueMsg(args.size() == 1 || args.size() == 2,
  //                "UdpAdapter: The " << GET_PARAMETER_COMMAND().c_str() << " command requires either one or two arguments");
  //  LabelStr command(args.front());
  //  debugMsg("UdpAdapter::executeGetParameterCommand", " " << command.c_str() << ", dest==" << dest << ", ack==" << ack);
  //  debugMsg("UdpAdapter::executeGetParameterCommand", " message handler for \"" << command.c_str() << "\" registered.");
  //}

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
        if ( port ) msg.port = atoi(port); // record the port if there is one
        // Walk the <Parameter/> elements of this <Message/>, if any
        for (const TiXmlElement* param = child->FirstChildElement(); param != NULL; param = param->NextSiblingElement())
          {
            Parameter arg;
            const char* param_name = NULL;
            param_name = param->Attribute("name");
            // what about strings? -- fixed length to start with I guess
            len = param->Attribute("length");
            assertTrue((len), "No parameter length given in <Message name=\"" + name + "\"/>");
            arg.len = atoi(len);
            assertTrue((arg.len > 0), "Zero length parameter given in <Message name=\"" + name + "\"/>");
            if (param_name) arg.name = param_name; // only assign it if it exists
            arg.type = param->Attribute("type");
            msg.len += arg.len;
            msg.parameters.push_back(arg);
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
        std::cout << std::endl << "         length: " << msg->second.len;
        std::cout << ", host: " << msg->second.host << ", port: " << msg->second.port;
        std::cout << ", thread: " << msg->second.thread << std::endl;
      }
  }

  int UdpAdapter::startUdpMessageReceiver(const LabelStr& name, ExpressionId dest, ExpressionId ack)
  {
    debugMsg("UdpAdapter::startUdpMessageReceiver",
             " entered for " << name.toString() << ", dest==" << dest << ", ack==" << ack);
    // Find the message definition to get the message port and size
    MessageMap::iterator msg;
    msg=m_messages.find(name.c_str());
    assertTrueMsg(msg != m_messages.end(),
                  "UdpAdapter::startUdpMessageReceiver: no message found for " << name.c_str());
    int port = msg->second.port;
    size_t size = msg->second.len;
    int status = 0; // return status
    //debugMsg("UdpAdapter::startUdpMessageReceiver", " msg->second.thread==" << msg->second.thread);
    assertTrueMsg(msg->second.thread == NULL,
                  "UdpAdapter::startUdpMessageReceiver: thread is not NULL for " << name.c_str());
    udp_thread_params params;
    params.local_port = port;
    params.buffer = new unsigned char[size];
    params.size = size;
    params.debug = true;
    udp_thread_params* param_ptr = &params;
    pthread_t thread_handle;
    // debugMsg("UdpAdapter::startUdpMessageReceiver", " params: " << params.local_port << ", " << params.size);
    // Finally, set up the receiver.  This needs to be wrapped in a further layer I think.
    threadSpawn((THREAD_FUNC_PTR) wait_for_input_on_thread, param_ptr, thread_handle);
    assertTrueMsg(thread_handle != NULL, "UdpAdapter::startUdpMessageReceiver: threadSpawn return NULL");
    msg->second.thread = thread_handle;
    // XXXX, No, this isn't when to call this.  When the message is received down in the bowels is when...
    // status = handleUdpMessage(msg->second, params.buffer, size, params.debug);
    // delete[] params.buffer;
    debugMsg("UdpAdapter::startUdpMessageReceiver", " for " << name.toString() << " done");
    return status;
  }

  int UdpAdapter::handleUdpMessage()
  {
    debugMsg("UdpAdapter::handleUdpMessage", " called");
    return 0;
  }

  int UdpAdapter::sendUdpMessage(const unsigned char* buffer, const UdpMessage& msg, bool debug)
  {
    int status = 0; // return status
    debugMsg("UdpAdapter::sendUdpMessage", " sending " << msg.len << " bytes to " << msg.host << ":" << msg.port);
    status = send_message_connect(msg.host.c_str(), msg.port, (const char*) buffer, msg.len, debug);
    return status;
  }

  int UdpAdapter::buildUdpBuffer(unsigned char* buffer, const UdpMessage& msg, const std::list<double>& args, bool debug)
  {
    std::list<double>::const_iterator it;
    std::list<Parameter>::const_iterator param;
    int start_index = 0; // where in the buffer to write
    // Do what error checking we can, since we absolutely know that planners foul this up.
    assertTrueMsg((args.size() == msg.parameters.size()),
                  "the message definition given in the XML configuration file does not match the command used"
                  << " in the plan for <Message name=\"" << LabelStr(args.front()).c_str() << "\"/>");
    // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
    for (param=msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(), it != args.end(); param++, it++)
      {
        int len = param->len;
        std::string type = param->type;
        double plexil_val = *it;
        // The parameter passed will be one of these two
        if (debug) std::cout << "  start_index: " << start_index << ", ";
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
            encode_string(str, buffer, start_index);
          }
        if (debug) std::cout << std::endl;
        start_index += len;
      }
    if (debug)
      {
        std::cout << "  buffer: ";
        print_buffer(buffer, msg.len);
      }
    return start_index;
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

  double UdpAdapter::formatMessageName(const LabelStr& name, const LabelStr& command, int id)
  {
    std::ostringstream ss;
    if (command == RECEIVE_COMMAND_COMMAND())
      {
        ss << COMMAND_PREFIX() << name.toString();
      }
    else if (command == GET_PARAMETER_COMMAND())
      {
        ss << PARAM_PREFIX() << name.toString();
      }
    else
      {
        ss << name.getKey();
      }
    ss << '_' << id;
    return LabelStr(ss.str()).getKey();
  }

  double UdpAdapter::formatMessageName(const LabelStr& name, const LabelStr& command)
  {
    return formatMessageName(name, command, 0);
  }

  double UdpAdapter::formatMessageName(const char* name, const LabelStr& command)
  {
    return formatMessageName(LabelStr(name), command, 0);
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
