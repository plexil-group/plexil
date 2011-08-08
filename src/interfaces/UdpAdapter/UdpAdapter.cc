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
      m_messageQueues(execInterface),
      m_debug(true)
  {
    debugMsg("UdpAdapter::UdpAdapter(execInterface)", " called");
  }

  // Constructor
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface, const TiXmlElement* xml)
    : InterfaceAdapter(execInterface, xml),
      m_messageQueues(execInterface),
      m_debug(true)
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
    // debugMsg("UdpAdapter::initialize", " xml = " << *xml);
    // parse the XML message definitions
    parseXmlMessageDefinitions(xml); // also calls registerCommandInterface for each message
    //this->registerAdapter();
    //m_execInterface.defaultRegisterAdapter(getId());
    if (m_debug) printMessageDefinitions();
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_UDP_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(RECEIVE_UDP_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(RECEIVE_COMMAND_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(GET_PARAMETER_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_RETURN_VALUE_COMMAND()), getId());
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
    else if (name == RECEIVE_UDP_MESSAGE_COMMAND()) // SendUdpCommand("cmd_name", arg1, ...); XXXX
      executeReceiveUdpCommand(args, dest, ack);
    else if (name == RECEIVE_COMMAND_COMMAND())
      executeReceiveCommandCommand(args, dest, ack); // OnCommand cmd_name (arg1, ...); XXXX
    else if (name == GET_PARAMETER_COMMAND())
      executeGetParameterCommand(args, dest, ack);
    else if (name == SEND_RETURN_VALUE_COMMAND())
      executeSendReturnValueCommand(args, dest, ack);
    else
      executeDefaultCommand(name, args, dest, ack); // Command cmd_name (arg1, ...); XXXX
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeCommand", " " << name.c_str() << " done.");
  }

  //abort the given command with the given arguments.  store the abort-complete into dest
  void UdpAdapter::invokeAbort(const LabelStr&, const std::list<double>&, ExpressionId, ExpressionId ack)
  {
    debugMsg("UdpAdapter::invokeAbort(name, args, dest, ack)", " called");
    debugMsg("ExternalInterface:udp", " invokeAbort called");
    m_execInterface.handleValueChange(ack, BooleanVariable::TRUE());
    m_execInterface.notifyOfExternalEvent();
  }

  //
  // Implementation methods
  //

  // Default UDP command handler
  void UdpAdapter::executeDefaultCommand(const LabelStr& msgName, const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    debugMsg("UdpAdapter::executeDefaultCommand", " called for \"" << msgName.c_str() << "\" with " << args.size() << " args");
    ThreadMutexGuard guard(m_cmdMutex);
    MessageMap::iterator msg;
    msg=m_messages.find(msgName.c_str());
    // Check for an obviously bogus port
    assertTrueMsg(msg->second.peer_port != 0,
                  "executeDefaultCommand: bad peer port (0) given for " << msgName.c_str() << " message");
    // Set up the outgoing UDP buffer to be sent
    int length = msg->second.len;
    unsigned char* udp_buffer = new unsigned char[length]; // fixed length to start with
    memset((char*)udp_buffer, 0, length); // zero out the buffer
    // Walk the parameters and encode them in the buffer to be sent out
    buildUdpBuffer(udp_buffer, msg->second, args, false, m_debug);
    // Send the buffer to the given host:port
    int status = -1;
    status = sendUdpMessage(udp_buffer, msg->second, m_debug);
    debugMsg("UdpAdapter::executeSendUdpMessageCommand", " sendUdpMessage returned " << status << " (bytes sent)");
    // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    // Clean up some (one hopes)
    delete udp_buffer;
  }

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
    debugMsg("UdpAdapter::executeReceiveCommandCommand", " message handler for \"" << command.c_str() << "\" registered.");
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
  }

  // SEND_UDP_MESSAGE_COMMAND
  void UdpAdapter::executeSendUdpMessageCommand(const std::list<double>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // First arg is message name (which better match one of the defined messages...)
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: the first paramater to SendUdpMessage command, "
                  << Expression::valueToString(args.front()) << ", is not a string");
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
    buildUdpBuffer(udp_buffer, msg->second, args, m_debug);
    // Send the buffer to the given host:port
    int status = -1;
    status = sendUdpMessage(udp_buffer, msg->second, m_debug);
    debugMsg("UdpAdapter::executeSendUdpMessageCommand", " sendUdpMessage returned " << status << " (bytes sent)");
    // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    // Clean up some (one hopes)
    delete udp_buffer;
  }

  // GET_PARAMETER_COMMAND
  void UdpAdapter::executeGetParameterCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    assertTrueMsg(args.size() == 1 || args.size() == 2,
                  "UdpAdapter: The " << GET_PARAMETER_COMMAND().c_str() << " command requires either one or two arguments");
    assertTrueMsg(LabelStr::isString(args.front()),
                  "UdpAdapter: The first argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, "
                  << Expression::valueToString(args.front())
                  << ", is not a string");
    //LabelStr command(args.front());
    debugMsg("UdpAdapter::executeGetParameterCommand",
             " " << LabelStr(args.front()).c_str() << ", dest==" << dest << ", ack==" << ack);
    //debugMsg("UdpAdapter::executeGetParameterCommand", " message handler for \"" << command.c_str() << "\" registered.");
    std::list<double>::const_iterator it = ++args.begin();
    int id;
    if (it == args.end())
      {
        id = 0;
      }
    else
      {
        id = static_cast<int> (*it);
        assertTrueMsg(id == *it,
                      "IpcAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not an integer");
        assertTrueMsg(id >= 0,
                      "IpcAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not a valid index");
      }
    LabelStr command(formatMessageName(args.front(), GET_PARAMETER_COMMAND(), id));
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeGetParameterCommand", " message handler for \"" << command.c_str() << "\" registered.");
  }

  // SEND_RETURN_VALUE_COMMAND
  void UdpAdapter::executeSendReturnValueCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
  {
    // Open loop communications only.  Perhaps this is being called by the expanded nodes?
    //debugMsg("UdpAdapter::executeSendReturnValueCommand", " called for " << LabelStr(args.front()).c_str());
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

  void UdpAdapter::parseXmlMessageDefinitions(const TiXmlElement* xml)
  // Parse and verify the given Adapter configuration
  {
    m_messages.clear();         // clear the old messages (if any)
    const char* default_local_port = NULL;
    const char* default_peer_port = NULL;
    default_local_port = xml->Attribute("default_local_port");
    default_peer_port = xml->Attribute("default_peer_port");
    if (default_local_port) m_default_local_port = atoi(default_local_port);
    if (default_peer_port) m_default_peer_port = atoi(default_peer_port);
    for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
      {
        UdpMessage msg;
        //std::string name;        // the Plexil command name
        const char* name = NULL;       // the Plexil command name
        const char* peer = NULL;       // needed for bool test below
        const char* local_port = NULL; // needed for bool test below
        const char* peer_port = NULL;
        const char* len = NULL;
        name = child->Attribute("name"); // name is required
        assertTrueMsg(name, "parseXmlMessageDefinitions: no name given in <Message/>");
        //msg.name = name = child->Attribute("name"); // for debugging message below
        msg.name = name;
        //msg.type = type = child->Attribute("type"); // for debugging message below
        peer = child->Attribute("peer");
        local_port = child->Attribute("local_port");
        peer_port = child->Attribute("peer_port");
        msg.peer = peer ? peer : "localhost";   // record the host given or a default
        // Check for either the given port or the (appropriate) default port, or singal an error
        //assertTrueMsg((default_local_port || local_port || default_peer_port || peer_port),
        //              "parseXmlMessageDefinitions: no local or peer ports given for <Message name=\""
        //              << name.c_str() << "\"/>");
        // Warn about possible run time errors (planners may simply not use a message I suppose)
        if (!(default_local_port || local_port))
          std::cout << "Warning: no default or message specific local port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if " 
                    << name << "is called to send an outgoing command/message\n";
        if (!(default_peer_port || peer_port))
          std::cout << "Warning: no default or message specific peer port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if " 
                    << name << "is called to receive an incoming command/message\n";
        msg.local_port = local_port ? atoi(local_port) : m_default_local_port;
        msg.peer_port = peer_port ? atoi(peer_port) : m_default_peer_port;
        // Walk the <Parameter/> elements of this <Message/>, if any
        for (const TiXmlElement* param = child->FirstChildElement(); param != NULL; param = param->NextSiblingElement())
          {
            Parameter arg;
            const char* param_desc = NULL;
            const char* param_type = NULL;
            // const char* param_text = NULL;
            // param_text = param->GetText();
            // if (param_text) printf("\n\nparam_text: %s\n\n", param_text);
            // Get the description (it is exists)
            param_desc = param->Attribute("desc");
            if (param_desc) arg.desc = param_desc; // only assign it if it exists
            // Get the type, which is required
            param_type = param->Attribute("type");
            assertTrueMsg(param_type, "parseXmlMessageDefinitions: no type for parameter given in <Message name=\""
                          << name << "\"/>");
            arg.type = param_type;
            // Get the length, which is required
            len = param->Attribute("bytes");
            assertTrueMsg(len, "parseXmlMessageDefinitions: no parameter length (in bytes) given in <Message name=\""
                          << name << "\"/>");
            arg.len = atoi(len);
            assertTrueMsg((arg.len > 0),
                          "parseXmlMessageDefinitions: zero length (in bytes) parameter given in <Message name=\""
                          << name << "\"/>");
            // Do some error checking for reasonble/usable encoding/decoding byte lengths
            if (arg.type.compare("int") == 0)
              {
                assertTrueMsg((arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: integers must be 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            else if (arg.type.compare("float") == 0)
              {
                assertTrueMsg((arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: floats must be 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            else if (arg.type.compare("bool") == 0)
              {
                assertTrueMsg((arg.len==1 || arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: bools must be 1, 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            // what about strings? -- fixed length to start with I suppose...
            else if (arg.type.compare("string") == 0)
              {
                assertTrueMsg(arg.len>=1,
                              "parseXmlMessageDefinitions: floats must be 1 byte or longer (in <Message name=\""
                              << name << "\"/>)");
              }
            else
              {
                assertTrueMsg(0, "parseXmlMessageDefinitions: unknown parameter type \"" << arg.type
                              << "\" (in <Message name=\"" << name << "\"/>)");
              }
            msg.len += arg.len;
            msg.parameters.push_back(arg);
          }
        m_messages[child->Attribute("name")]=msg; // record the message with the name as the key
        m_execInterface.registerCommandInterface(LabelStr(name), getId()); // register name with executeCommand
      }
  }

  void UdpAdapter::printMessageDefinitions()
  {
    // print all of the stuff in m_message for debugging
    MessageMap::iterator msg;
    int i = 0;
    for (msg=m_messages.begin(); msg != m_messages.end(); msg++, i++)
      {
        std::cout << "Message: " << msg->first;
        std::list<Parameter>::iterator param;
        for (param=msg->second.parameters.begin(); param != msg->second.parameters.end(); param++)
          {
            std::string temp = param->desc.empty() ? "(no description)" :  param->desc;
            std::cout << "\n         ";
            std::cout << param->len << " byte " << param->type << " " << temp;
          }
        std::cout << std::endl << "         length: " << msg->second.len << " (bytes)";
        std::cout << ", peer: " << msg->second.peer << ", peer_port: " << msg->second.peer_port;
        std::cout << ", local_port: " << msg->second.local_port;
        std::cout << std::endl;
      }
  }

  // Start a UDP Message Handler for a node waiting on a UDP message
  int UdpAdapter::startUdpMessageReceiver(const LabelStr& name, ExpressionId dest, ExpressionId ack)
  {
    debugMsg("UdpAdapter::startUdpMessageReceiver",
             " entered for " << name.toString() << ", dest==" << dest << ", ack==" << ack);
    // Find the message definition to get the message port and size
    MessageMap::iterator msg;
    msg=m_messages.find(name.c_str());
    assertTrueMsg(msg != m_messages.end(),
                  "UdpAdapter::startUdpMessageReceiver: no message found for " << name.c_str());
    // Check for a bogus local port
    assertTrueMsg(msg->second.local_port != 0,
                  "startUdpMessageReceiver: bad local port (0) given for " << name.c_str() << " message");
    msg->second.name = name.c_str();
    msg->second.self = (void*) this; // pass a reference to "this" UdpAdapter for later use
    pthread_t thread_handle;
    // Set up the receiver.  This needs to be wrapped in a further layer I think.
    threadSpawn((THREAD_FUNC_PTR) waitForUdpMessage, &msg->second, thread_handle);
    // Check to see if the thread got started
    assertTrueMsg(thread_handle != NULL, "UdpAdapter::startUdpMessageReceiver: threadSpawn return NULL");
    //msg->second.thread = thread_handle; // record the thread
    debugMsg("UdpAdapter::startUdpMessageReceiver", " for " << name.toString() << " done");
    return 0;
  }

  void* UdpAdapter::waitForUdpMessage(const UdpMessage* msg)
  {
    debugMsg("UdpAdapter::waitForUdpMessage", " called for " << msg->name);
    // A pointer to the adapter
    UdpAdapter* udpAdapter = reinterpret_cast<UdpAdapter*>(msg->self);
    int local_port = msg->local_port;
    size_t size = msg->len;
    udp_thread_params params;
    params.local_port = local_port;
    params.buffer = new unsigned char[size];
    params.size = size;
    params.debug = udpAdapter->m_debug; // see if debugging is enabled
    udp_thread_params* param_ptr = &params;
    //debugMsg("UdpAdapter::waitForUdpMessage", " params: " << params.local_port << ", " << params.size);
    wait_for_input_on_thread(&params);
    // When the message has been received, tell the UdpAdapter about it and its contents
    int status = udpAdapter->handleUdpMessage(msg, params.buffer, params.debug);
    assertTrueMsg(status==0, "waitForUdpMessage call to handleUdpMessage returned " << status);
    delete[] params.buffer;     // release the input buffer
  }

  int UdpAdapter::handleUdpMessage(const UdpMessage* msgDef, const unsigned char* buffer, bool debug)
  {
    // Handle a UDP message once it has indeed arrived.
    // msgDef is passed in, therefore, we will assume it is good.
    debugMsg("UdpAdapter::handleUdpMessage", " called for " << msgDef->name);
    if (debug) {std::cout << "  handleUdpMessage: buffer: "; print_buffer(buffer, msgDef->len);}
    // (1) addMessage for expected message
    static int counter = 1;     // gensym counter
    std::ostringstream unique_id;
    unique_id << msgDef->name << "_msg_parameter:" << counter++;
    LabelStr msg_label(unique_id.str());
    debugMsg("UdpAdapter::handleUdpMessage", " adding \"" << msgDef->name << "\" to the command queue");
    const LabelStr& msg_name(formatMessageName(msgDef->name, RECEIVE_COMMAND_COMMAND()));
    m_messageQueues.addMessage(msg_name, msg_label.getKey());
    // (2) walk the parameters, and for each, call addMessage(label, <value-or-key>), which
    //     (somehow) arranges for executeCommand(GetParameter) to be called, and which in turn
    //     calls addRecipient and updateQueue
    int i = 0;
    int offset = 0;
    std::list<Parameter>::const_iterator param;
    for (param=msgDef->parameters.begin() ; param != msgDef->parameters.end() ; param++, i++)
      {
        const LabelStr& param_label(formatMessageName(msg_label, GET_PARAMETER_COMMAND(), i));
        int len = param->len;   // number of bytes to read
        std::string type = param->type; // type to decode
        if (debug) std::cout << "  handleUdpMessage: decoding " << len << " byte " << type << " starting at " << offset << std::endl;
        if (type.compare("int") == 0)
          {
            assertTrueMsg((len==2 || len==4), "handleUdpMessage: Integers must be 2 or 4 bytes, not " << len);
            int num;
            if (len == 2)
              num = decode_short_int(buffer, offset);
            else if (len == 4)
              num = decode_long_int(buffer, offset);
            debugMsg("UdpAdapter::handleUdpMessage", " sending numeric (integer) parameter " << num);
            m_messageQueues.addMessage(param_label, (double) num);
            offset += len;
          }
        else if (type.compare("float") == 0)
          {
            assertTrueMsg(len==4, "handleUdpMessage: Reals must be 4 bytes, not " << len);
            float num;
            num = decode_float(buffer, offset);
            debugMsg("UdpAdapter::handleUdpMessage", " sending numeric (real) parameter " << num);
            m_messageQueues.addMessage(param_label, (double) num);
            offset += len;
          }
        else if (type.compare("bool") == 0)
          {
            assertTrueMsg((len==1 || len==2 || len==4), "handleUdpMessage: Booleans must be 1, 2 or 4 bytes, not " << len);
            int num;
            if (len==1)
              num = network_bytes_to_number(buffer, offset, 8, false);
            else if (len==2)
              num = decode_short_int(buffer, offset);
            else if (len==4)
              num == decode_long_int(buffer, offset);
            debugMsg("UdpAdapter::handleUdpMessage", " sending numeric (boolean) parameter " << num);
            m_messageQueues.addMessage(param_label, (double) num);
            offset += len;
          }
        else //string
          {
            assertTrueMsg(!type.compare("string"), "handleUdpMessage: unknown parameter type " << type.c_str());
            std::string str = decode_string(buffer, offset, len);
            debugMsg("UdpAdapter::handleUdpMessage", " sending string parameter " << str);
            m_messageQueues.addMessage(param_label, LabelStr(str).getKey());
            offset += len;
          }
      }
    return 0;
  }

  int UdpAdapter::sendUdpMessage(const unsigned char* buffer, const UdpMessage& msg, bool debug)
  {
    int status = 0; // return status
    debugMsg("UdpAdapter::sendUdpMessage", " sending " << msg.len << " bytes to " << msg.peer << ":" << msg.peer_port);
    status = send_message_connect(msg.peer.c_str(), msg.peer_port, (const char*) buffer, msg.len, debug);
    return status;
  }

  int UdpAdapter::buildUdpBuffer(unsigned char* buffer, const UdpMessage& msg, const std::list<double>& args,
                                 bool skip_arg, bool debug)
  {
    std::list<double>::const_iterator it;
    std::list<Parameter>::const_iterator param;
    int start_index = 0; // where in the buffer to write
    // Do what error checking we can, since we absolutely know that planners foul this up.
    debugMsg("UdpAdapter::buildUdpBuffer", " args.size()==" << args.size() << ", parameters.size()==" << msg.parameters.size());
    assertTrueMsg((args.size() == skip_arg ? 0 : 1 + msg.parameters.size()),
                  "the message definition given in the XML configuration file does not match the command used"
                  << " in the plan for <Message name=\"" << LabelStr(args.front()).c_str() << "\"/>");
    // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
    //for (param=msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(), it != args.end(); param++, it++)
    for (param=msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(); param++, it++)
      {
        //it++; // Plexil adds the "command name" to the args of commands, so we must carefully skip it here XXXX
        if (skip_arg) { it++; skip_arg = false; } // only skip the first arg
        int len = param->len;
        std::string type = param->type;
        double plexil_val = *it;
        // if (debug) std::cout << "plexil_val: " << Expression::valueToString(plexil_val) << std::endl;
        // The parameter passed will be one of these two
        if (debug) std::cout << "  start_index: " << start_index << ", ";
        // Encode only 32 bit entities (i.e., no 64 bit reals/ints)
        // XXXX need to check min/max floats/ints for out of 32 bit range
        if (type.compare("int") == 0)
          {
            assertTrueMsg((len==2 || len==4), "buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            if (debug) std::cout << "int: " << plexil_val;
            if (len==2)
              encode_short_int(plexil_val, buffer, start_index);
            else
              encode_long_int(plexil_val, buffer, start_index);
          }
        else if (type.compare("float") == 0)
          {
            float temp = plexil_val;
            assertTrueMsg(len==4, "buildUdpBuffer: Reals must be 4 bytes, not " << len);
            if (debug) std::cout << "float: " << temp;
            encode_float(temp, buffer, start_index);
          }
        else if (type.compare("bool") == 0) // these are 64 bits in Plexil
          {
            assertTrueMsg((len==1 || len==2 || len==4), "buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            if (debug) std::cout << "bool: " << plexil_val;
            if (len==1)
              number_to_network_bytes(plexil_val, buffer, start_index, 8);
            else if (len==2)
              encode_short_int(plexil_val, buffer, start_index);
            else
              encode_long_int(plexil_val, buffer, start_index);
          }
        else
          {
            std::string str = LabelStr(plexil_val).c_str();
            assertTrueMsg(str.length()==len, "buildUdpBuffer: Declared string length (" << len <<
                          ") and actual length (" << str.length() << " do not match");
            if (debug) std::cout << "string: \"" << LabelStr(plexil_val).c_str() << "\"";
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
