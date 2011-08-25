/* -*- Mode: c++ -*-
   File: plexilsrc/interfaces/UdpAdapter/UdpAdapter.cc
*/

#include "UdpAdapter.hh"
#include "Debug.hh"             // debugMsg
#include "CoreExpressions.hh"   // BooleanVariable, etc.
#include "Node.hh"              // struct PLEXIL::Node
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"    // initUdpAdapter

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
      m_debug(false)
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
    // parse the XML message definitions
    parseXmlMessageDefinitions(xml); // also calls registerCommandInterface for each message
    if (m_debug) printMessageDefinitions();
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    m_execInterface.registerCommandInterface(LabelStr(SEND_UDP_MESSAGE_COMMAND()), getId());
    //m_execInterface.registerCommandInterface(LabelStr(RECEIVE_UDP_MESSAGE_COMMAND()), getId());
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
    debugMsg("UdpAdapter::stop", " called");
    // Stop the UDP listener thread
    return true;
  }

  // Reset method
  bool UdpAdapter::reset()
  {
    debugMsg("UdpAdapter::reset", " called");
    return true;
  }

  // Shutdown method
  bool UdpAdapter::shutdown()
  {
    debugMsg("UdpAdapter::shutdown", " called");
    return true;
  }

  void UdpAdapter::registerChangeLookup(const LookupKey& /* uniqueId */,
                                        const StateKey& /* stateKey */,
                                        const std::vector<double>& /* tolerances */)
  {
    debugMsg("UdpAdapter::registerChangeLookup", " called");
    debugMsg("ExternalInterface:udp", " registerChangeLookup called");
  }

  void UdpAdapter::unregisterChangeLookup(const LookupKey& /* uniqueId */)
  {
    debugMsg("UdpAdapter::unregisterChangeLookup", " called");
    debugMsg("ExternalInterface:udp", " unregisterChangeLookup called");
  }

  void UdpAdapter::lookupNow(const StateKey& /* key */, std::vector<double>& dest)
  {
    debugMsg("UdpAdapter::lookupNow", " called");
    debugMsg("ExternalInterface:udp", " lookupNow called; returning UNKNOWN");
    dest[0] = Expression::UNKNOWN();
  }

  void UdpAdapter::sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack)
  {
    debugMsg("UdpAdapter::sendPlannerUpdate", " called");
    debugMsg("ExternalInterface:udp", " sendPlannerUpdate called");
    // acknowledge updates
    debugMsg("ExternalInterface:udp", " faking acknowledgment of update node '" << node->getNodeId().toString() << "'");
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
    //else if (name == RECEIVE_UDP_MESSAGE_COMMAND()) // SendUdpCommand("cmd_name", arg1, ...); XXXX
    //  executeReceiveUdpCommand(args, dest, ack);
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

  // Abort the given command with the given arguments.  Store the abort-complete into dest
  void UdpAdapter::invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId dest, ExpressionId cmdAck)
  {
    assertTrueMsg(cmdName == RECEIVE_COMMAND_COMMAND(), "UdpAdapter: Only ReceiveCommand commands can be aborted");
    assertTrueMsg(cmdArgs.size() == 1, "UdpAdapter: Aborting ReceiveCommand requires exactly one argument");
    assertTrueMsg(LabelStr::isString(cmdArgs.front()), "UdpAdapter: The argument to the ReceiveMessage abort, "
                  << Expression::valueToString(cmdArgs.front()) << ", is not a string");
    LabelStr msgName(cmdArgs.front());
    debugMsg("UdpAdapter::invokeAbort", " called for " << msgName.c_str() << ", " << dest << ", " << cmdAck);
    // Find the active thread for this message, cancel and erase it
    ThreadMap::iterator thread;
    thread=m_activeThreads.find(msgName.c_str()); // recorded by startUdpMessageReceiver
    assertTrueMsg(thread != m_activeThreads.end(), "UdpAdapter::invokeAbort: no thread found for " << msgName);
    int status = pthread_cancel(thread->second);
    assertTrueMsg(status == 0, "UdpAdapter::invokeAbort: pthread_join(" << thread->second << ") returned " << status);
    debugMsg("UdpAdapter::invokeAbort", " " << msgName.c_str() << " listener thread (" << thread->second << ") cancelled");
    m_activeThreads.erase(thread);
    // Let the exec know that we believe things are cleaned up
    m_messageQueues.removeRecipient(formatMessageName(msgName, RECEIVE_COMMAND_COMMAND()), cmdAck);
    m_execInterface.handleValueChange(dest, BooleanVariable::TRUE());
    m_execInterface.notifyOfExternalEvent();
  }

  //
  // Implementation methods
  //

  // Default UDP command handler
  void UdpAdapter::executeDefaultCommand(const LabelStr& msgName,
                                         const std::list<double>& args,
                                         ExpressionId dest,
                                         ExpressionId ack)
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
    LabelStr msgName(args.front());
    debugMsg("UdpAdapter::executeReceiveCommandCommand", " called for " << msgName.c_str());
    LabelStr command(formatMessageName(args.front(), RECEIVE_COMMAND_COMMAND()));
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    // Set up the thread on which the message may/will eventually be received
    int status = -1;
    status = startUdpMessageReceiver(LabelStr(args.front()), dest, ack);
    debugMsg("UdpAdapter::executeReceiveCommandCommand", " message handler for \"" << command.c_str() << "\" registered.");
  }

//   // RECEIVE_UDP_MESSAGE_COMMAND
//   void UdpAdapter::executeReceiveUdpCommand(const std::list<double>& args, ExpressionId dest, ExpressionId ack)
//   {
//     // Called when node _starts_ executing, so, record the message and args so that they can be filled in
//     // if and when a UDP message comes in the fulfill this expectation.
//     // First arg is message name (which better match one of the defined messages...)
//     assertTrueMsg(LabelStr::isString(args.front()),
//                   "UdpAdapter: the first paramater to ReceiveUdpMessage command, "
//                   << Expression::valueToString(args.front()) << ", is not a string");
//     LabelStr command(args.front());
//     debugMsg("UdpAdapter::executeReceiveUdpCommand", " " << command.c_str() << ", dest==" << dest
//              << ", ack==" << ack << ", args.size()==" << args.size());
//     m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
//     m_execInterface.notifyOfExternalEvent();
//     debugMsg("UdpAdapter::executeReceiveUdpCommand", " handler for \"" << command.c_str() << "\" registered.");
//   }

  // SEND_UDP_MESSAGE_COMMAND
  void UdpAdapter::executeSendUdpMessageCommand(const std::list<double>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // First arg is message name (which better match one of the defined messages...)
    assertTrueMsg(LabelStr::isString(args.front()), "UdpAdapter: the first paramater to SendUdpMessage command, "
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
    buildUdpBuffer(udp_buffer, msg->second, args, true, m_debug);
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
                  << Expression::valueToString(args.front()) << ", is not a string");
    debugMsg("UdpAdapter::executeGetParameterCommand",
             " " << LabelStr(args.front()).c_str() << ", dest==" << dest << ", ack==" << ack);
    // Extract the message name and try to verify the number of parameters defined vs the number of args used
    std::string msgName = LabelStr(args.front()).toString();
    size_t pos;
    pos = msgName.find(":");
    msgName = msgName.substr(0, pos);
    MessageMap::iterator msg;
    msg=m_messages.find(msgName);
    assertTrueMsg(msg != m_messages.end(), "UdpAdapter::executeGetParameterCommand: no message found for " << msgName);
    int params = msg->second.parameters.size();
    debugMsg("UdpAdapter::executeGetParameterCommand", " msgName==" << msgName);
    debugMsg("UdpAdapter::executeGetParameterCommand", " params==" << params);
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
                      "UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not an integer");
        assertTrueMsg(id >= 0,
                      "UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not a valid index");
        // Brute strength error check for the plan using a message/command with to many arguments.
        // The intent is that this might be discovered during development.
        assertTrueMsg(id < params,
                      "UdpAdapter: the message \"" << msgName << "\" is defined to have " << params
                      << " parameters in the configuration file, but is being used in the plan with "
                      << id+1 << " arguments");
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
    // First, set up the internal debugging output
    const char* debug = NULL;
    debug = xml->Attribute("debug");
    assertTrueMsg((debug == NULL || strcasecmp(debug, "true") == 0 || strcasecmp(debug, "false") == 0),
                  "parseXmlMessageDefinitions: debug must be a boolean, not " << debug);
    if (debug && strcasecmp(debug, "true") == 0) m_debug = true;
    // Now, do the real work of parsing the XML UDP Configuration
    const char* default_local_port = NULL; // char* NULL allows for a boolean test below...
    const char* default_peer_port = NULL;
    const char* default_peer = NULL;
    default_local_port = xml->Attribute("default_local_port");
    default_peer_port = xml->Attribute("default_peer_port");
    default_peer = xml->Attribute("default_peer");
    if (default_local_port) m_default_local_port = atoi(default_local_port);
    if (default_peer_port) m_default_peer_port = atoi(default_peer_port);
    if (default_peer) m_default_peer = default_peer;
    // Walk the messages
    for (const TiXmlElement* child = xml->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
      {
        UdpMessage msg;
        const char* name = NULL;       // the Plexil command name
        const char* peer = NULL;       // needed for bool test below (i.e., it is optional)
        const char* local_port = NULL;
        const char* peer_port = NULL;
        const char* len = NULL;
        name = child->Attribute("name"); // name is required, hence...
        assertTrueMsg(name, "parseXmlMessageDefinitions: no name given in <Message/>");
        msg.name = name;
        peer = child->Attribute("peer");
        local_port = child->Attribute("local_port");
        peer_port = child->Attribute("peer_port");
        // Use either the given peer, the default_peer, or "localhost"
        msg.peer = peer ? peer : (default_peer ? m_default_peer : "localhost");
        // Warn about possible run time errors (planners may simply not use a message I suppose)
        if (!(default_local_port || local_port))
          std::cout << "Warning: no default or message specific local port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if "
                    << name << "it is called to send an outgoing command/message\n";
        if (!(default_peer_port || peer_port))
          std::cout << "Warning: no default or message specific peer port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if "
                    << name << "it is called to receive an incoming command/message\n";
        msg.local_port = local_port ? atoi(local_port) : m_default_local_port;
        msg.peer_port = peer_port ? atoi(peer_port) : m_default_peer_port;
        // Walk the <Parameter/> elements of this <Message/>
        for (const TiXmlElement* param = child->FirstChildElement(); param != NULL; param = param->NextSiblingElement())
          {
            Parameter arg;
            const char* param_desc = NULL;
            const char* param_type = NULL;
            // const char* param_text = NULL;
            // param_text = param->GetText();
            // if (param_text) printf("\n\nparam_text: %s\n\n", param_text);
            // Get the description (if any)
            param_desc = param->Attribute("desc");
            if (param_desc) arg.desc = param_desc; // only assign it if it exists
            // Get the (required) type
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
    std::string indent = "             ";
    int i = 0;
    for (msg=m_messages.begin(); msg != m_messages.end(); msg++, i++)
      {
        std::cout << "UDP Message: " << msg->first;
        std::list<Parameter>::iterator param;
        for (param=msg->second.parameters.begin(); param != msg->second.parameters.end(); param++)
          {
            std::string temp = param->desc.empty() ? " (no description)" : " (" + param->desc + ")";
            std::cout << "\n" << indent << param->len << " byte " << param->type << temp;
          }
        std::cout << std::endl << indent << "length: " << msg->second.len << " (bytes)";
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
    assertTrueMsg(thread_handle != 0, "UdpAdapter::startUdpMessageReceiver: threadSpawn return NULL");
    debugMsg("UdpAdapter::startUdpMessageReceiver",
             " " << name.toString() << " listener thread (" << thread_handle << ") spawned");
    // Record the thread in case it has to be cancelled later (in invokeAbort)
    m_activeThreads[name.c_str()]=thread_handle;
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
    int status = wait_for_input_on_thread(&params);
    assertTrueMsg(status==0, "waitForUdpMessage call to wait_for_input_on_thread returned " << status);
    // When the message has been received, tell the UdpAdapter about it and its contents
    status = udpAdapter->handleUdpMessage(msg, params.buffer, params.debug);
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
    unique_id << msgDef->name << ":msg_parameter:" << counter++;
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
        if (debug) std::cout << "  handleUdpMessage: decoding " << len << " byte " << type
                             << " starting at " << offset << std::endl;
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
    int param_count = msg.parameters.size();
    if (skip_arg) param_count++;
    assertTrueMsg((args.size() == param_count),
                  "the " << param_count << " paramaters defined in the XML configuration file do not match the "
                  << args.size() << " paramaters used in the plan for <Message name=\"" << msg.name << "\"/>");
    // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
    for (param=msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(); param++, it++)
      {
        if (skip_arg) { it++; skip_arg = false; } // only skip the first arg
        int len = param->len;
        std::string type = param->type;
        double plexil_val = *it;
        // The parameter passed will be one of these two
        if (debug) std::cout << "  buildUdpBuffer: encoding ";
        // Encode only 32 bit entities (i.e., no 64 bit reals/ints)
        if (type.compare("int") == 0)
          {
            assertTrueMsg((len==2 || len==4), "buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            // Large ints are caught in Expression.cc:337
            if (debug) std::cout << len << " byte int starting at " << start_index;
            if (len==2)
              encode_short_int((int)plexil_val, buffer, start_index);
            else
              encode_long_int((int)plexil_val, buffer, start_index);
          }
        else if (type.compare("float") == 0)
          {
            float temp = plexil_val;
            assertTrueMsg(len==4, "buildUdpBuffer: Reals must be 4 bytes, not " << len);
            // Catch really big floats
            assertTrueMsg((FLT_MIN <= plexil_val) && (FLT_MAX >= plexil_val),
                          "buildUdpBuffer: Reals (floats) must be between " << FLT_MIN << " and " << FLT_MAX <<
                          ", not " << plexil_val);
            if (debug) std::cout << len << " byte float starting at " << start_index;
            encode_float(temp, buffer, start_index);
          }
        else if (type.compare("bool") == 0) // these are 64 bits in Plexil
          {
            assertTrueMsg((len==1 || len==2 || len==4), "buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            assertTrueMsg((plexil_val == false || plexil_val == true), "buildUdpBuffer: Booleans must be either true ("
                          << true << ") or false (" << false << ")" << ", not " << plexil_val);
            if (debug) std::cout << len << " byte bool starting at " << start_index;
            if (len==1)
              number_to_network_bytes((int)plexil_val, buffer, start_index, 8);
            else if (len==2)
              encode_short_int((int)plexil_val, buffer, start_index);
            else
              encode_long_int((int)plexil_val, buffer, start_index);
          }
        else
          {
            std::string str = LabelStr(plexil_val).c_str();
            assertTrueMsg(str.length()==len, "buildUdpBuffer: Declared string length (" << len <<
                          ") and actual length (" << str.length() << " do not match");
            if (debug) std::cout << len << " byte string starting at " << start_index;
            encode_string(str, buffer, start_index);
          }
        if (debug) std::cout << std::endl;
        start_index += len;
      }
    if (debug)
      {
        std::cout << "  buildUdpBuffer: buffer: ";
        print_buffer(buffer, msg.len);
      }
    return start_index;
  }

  void UdpAdapter::printMessageContent(const LabelStr& name, const std::list<double>& args)
  {
    // Print the content of a message
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
