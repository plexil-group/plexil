/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"    // initUdpAdapter
#include "BooleanVariable.hh"
#include "CoreExpressions.hh"   // BooleanVariable, etc.
#include "Command.hh"
#include "Debug.hh"             // debugMsg
#include "Node.hh"              // struct PLEXIL::Node
#include "pugixml.hpp"
#include "stricmp.h"

#include <cfloat>
#include <climits>
#include <cstring>

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
  UdpAdapter::UdpAdapter(AdapterExecInterface& execInterface, const pugi::xml_node& xml)
    : InterfaceAdapter(execInterface, xml),
      m_messageQueues(execInterface),
      m_debug(false)
  {
    assertTrue(!xml.empty(), "XML config file not found in UdpAdapter::UdpAdapter constructor");
    debugMsg("UdpAdapter::UdpAdapter", " Using " << xml.attribute("AdapterType").value());
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
    const pugi::xml_node& xml = this->getXml();
    // parse the XML message definitions
    parseXmlMessageDefinitions(xml); // also calls registerCommandInterface for each message
    if (m_debug) printMessageDefinitions();
    m_execInterface.registerCommandInterface(LabelStr(SEND_MESSAGE_COMMAND()), getId());
    //m_execInterface.registerCommandInterface(LabelStr(SEND_UDP_MESSAGE_COMMAND()), getId());
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

  void UdpAdapter::subscribe(const State& /* state */)
  {
    debugMsg("UdpAdapter::subscribe", " called");
    debugMsg("ExternalInterface:udp", " subscribe called");
  }

  void UdpAdapter::unsubscribe(const State& /* state */)
  {
    debugMsg("UdpAdapter::unsubscribe", " called");
    debugMsg("ExternalInterface:udp", " unsubscribe called");
  }

  Value UdpAdapter::lookupNow(const State& /* state */)
  {
    debugMsg("UdpAdapter::lookupNow", " called");
    debugMsg("ExternalInterface:udp", " lookupNow called; returning UNKNOWN");
    return Value();
  }

  void UdpAdapter::sendPlannerUpdate(const NodeId& node, const std::map<LabelStr, Value>& valuePairs, ExpressionId ack)
  {
    debugMsg("UdpAdapter::sendPlannerUpdate", " called");
    debugMsg("ExternalInterface:udp", " sendPlannerUpdate called");
    // acknowledge updates
    debugMsg("ExternalInterface:udp", " faking acknowledgment of update node '" << node->getNodeId().toString() << "'");
    m_execInterface.handleValueChange(ack, BooleanVariable::TRUE_VALUE());
    m_execInterface.notifyOfExternalEvent();
  }

  // Execute a Plexil Command
  // void UdpAdapter::executeCommand(const LabelStr& name, const std::vector<Value>& args, ExpressionId dest, ExpressionId ack)
  void UdpAdapter::executeCommand(const CommandId& cmd)
  {
    Value name = cmd->getName();
    VariableId dest = cmd->getDest();
    VariableId ack = cmd->getAck();
    const std::vector<Value>& args = cmd->getArgValues();
    debugMsg("UdpAdapter::executeCommand", " " << name << " (dest==" << dest << ", ack==" << ack << ")");
    if (name == SEND_MESSAGE_COMMAND())
      executeSendMessageCommand(args, dest, ack);
    //else if (name == SEND_UDP_MESSAGE_COMMAND())
    //  executeSendUdpMessageCommand(args, dest, ack);
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
    //debugMsg("UdpAdapter::executeCommand", " " << name.c_str() << " done.");
  }

  // Abort the given command with the given arguments.  Store the abort-complete into dest
  void UdpAdapter::invokeAbort(const CommandId& cmd) 
  {
    Value cmdName = cmd->getName();
    if (cmdName != RECEIVE_COMMAND_COMMAND()) {
      m_execInterface.handleValueChange(cmd->getAbortComplete(), BooleanVariable::TRUE_VALUE());
      m_execInterface.notifyOfExternalEvent();
      return;
    }

    const std::vector<Value>& cmdArgs = cmd->getArgValues();
    assertTrueMsg(cmdArgs.size() == 1, "UdpAdapter: Aborting ReceiveCommand requires exactly one argument");
    assertTrueMsg(cmdArgs.front().isString(),
                  "UdpAdapter: The argument to the ReceiveMessage abort, "
                  << cmdArgs.front() << ", is not a string");
    const char* msgName = cmdArgs.front().c_str(); // The defined message name, needed for looking up the thread and socket
    VariableId dest = cmd->getDest();
    VariableId cmdAck = cmd->getAck();
    debugMsg("UdpAdapter::invokeAbort", " called for " << cmdName.c_str() << " (" << msgName <<
             "), " << dest << ", " << cmdAck);
    int status;                        // The return status of the calls to pthread_cancel() and close()
    // First, find the active thread for this message, cancel and erase it
    ThreadMap::iterator thread;
    thread=m_activeThreads.find(msgName); // recorded by startUdpMessageReceiver
    assertTrueMsg(thread != m_activeThreads.end(), "UdpAdapter::invokeAbort: no thread found for " << msgName);
    status = pthread_cancel(thread->second);
    assertTrueMsg(status == 0, "UdpAdapter::invokeAbort: pthread_cancel(" << thread->second << ") returned " << status);
    debugMsg("UdpAdapter::invokeAbort", " " << msgName << " listener thread (" << thread->second << ") cancelled");
    m_activeThreads.erase(thread); // erase the cancelled thread
    // Second, find the open socket for this message and close it
    SocketMap::iterator socket;
    socket=m_activeSockets.find(msgName); // recorded by startUdpMessageReceiver
    assertTrueMsg(socket != m_activeSockets.end(), "UdpAdapter::invokeAbort: no socket found for " << msgName);
    status = close(socket->second);
    assertTrueMsg(status == 0, "UdpAdapter::invokeAbort: close(" << socket->second << ") returned " << status);
    debugMsg("UdpAdapter::invokeAbort", " " << msgName << " socket (" << socket->second << ") closed");
    m_activeSockets.erase(socket); // erase the closed socket
    // Let the exec know that we believe things are cleaned up
    m_messageQueues.removeRecipient(formatMessageName(msgName, RECEIVE_COMMAND_COMMAND()), cmdAck);
    m_execInterface.handleValueChange(cmd->getAbortComplete(), BooleanVariable::TRUE_VALUE());
    m_execInterface.notifyOfExternalEvent();
  }

  //
  // Implementation methods
  //

  // Default UDP command handler
  void UdpAdapter::executeDefaultCommand(const LabelStr& msgName,
                                         const std::vector<Value>& args,
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
    debugMsg("UdpAdapter::executeDefaultCommand", " sendUdpMessage returned " << status << " (bytes sent)");
    // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS());
    m_execInterface.notifyOfExternalEvent();
    // Clean up some (one hopes)
    delete udp_buffer;
  }

  // RECEIVE_COMMAND_COMMAND
  void UdpAdapter::executeReceiveCommandCommand(const std::vector<Value>& args, ExpressionId dest, ExpressionId ack)
  {
    assertTrueMsg(args.size() == 1,
                  "UdpAdapter: The " << RECEIVE_COMMAND_COMMAND().c_str() << " command requires exactly one argument");
    assertTrueMsg(args.front().isString(),
                  "UdpAdapter: The argument to the " << RECEIVE_COMMAND_COMMAND().c_str()
                  << " command, " << args.front() << ", is not a string");
    const char* msgName = args.front().c_str();
    debugMsg("UdpAdapter::executeReceiveCommandCommand", " called for " << msgName);
    LabelStr command = formatMessageName(msgName, RECEIVE_COMMAND_COMMAND());
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    m_execInterface.notifyOfExternalEvent();
    // Set up the thread on which the message may/will eventually be received
    int status = -1;
    status = startUdpMessageReceiver(msgName, dest, ack);
    debugMsg("UdpAdapter::executeReceiveCommandCommand", " message handler for \"" << command.c_str() << "\" registered");
  }

//   // RECEIVE_UDP_MESSAGE_COMMAND
//   void UdpAdapter::executeReceiveUdpCommand(const std::vector<Value>& args, ExpressionId dest, ExpressionId ack)
//   {
//     // Called when node _starts_ executing, so, record the message and args so that they can be filled in
//     // if and when a UDP message comes in the fulfill this expectation.
//     // First arg is message name (which better match one of the defined messages...)
//     assertTrueMsg(LabelStr::isString(args.front()),
//                   "UdpAdapter: the first parameter to ReceiveUdpMessage command, "
//                   << args.front() << ", is not a string");
//     LabelStr command(args.front());
//     debugMsg("UdpAdapter::executeReceiveUdpCommand", " " << command.c_str() << ", dest==" << dest
//              << ", ack==" << ack << ", args.size()==" << args.size());
//     m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
//     m_execInterface.notifyOfExternalEvent();
//     debugMsg("UdpAdapter::executeReceiveUdpCommand", " handler for \"" << command.c_str() << "\" registered");
//   }

//  // SEND_UDP_MESSAGE_COMMAND
//   void UdpAdapter::executeSendUdpMessageCommand(const std::vector<Value>& args, ExpressionId /* dest */, ExpressionId ack)
//   {
//     // First arg is message name (which better match one of the defined messages...)
//     assertTrueMsg(LabelStr::isString(args.front()), "UdpAdapter: the first parameter to SendUdpMessage command, "
//                   << args.front() << ", is not a string");
//     // Lookup the appropriate message in the message definitions in m_messages
//     LabelStr msgName(args.front());
//     debugMsg("UdpAdapter::executeSendUdpMessageCommand", " called for " << msgName.c_str());
//     //printMessageContent(msgName, args);
//     MessageMap::iterator msg;
//     msg=m_messages.find(msgName.c_str());
//     // Set up the outgoing UDP buffer to be sent
//     int length = msg->second.len;
//     unsigned char* udp_buffer = new unsigned char[length]; // fixed length to start with
//     memset((char*)udp_buffer, 0, length); // zero out the buffer
//     // Walk the parameters and encode them in the buffer to be sent out
//     buildUdpBuffer(udp_buffer, msg->second, args, true, m_debug);
//     // Send the buffer to the given host:port
//     int status = -1;
//     status = sendUdpMessage(udp_buffer, msg->second, m_debug);
//     debugMsg("UdpAdapter::executeSendUdpMessageCommand", " sendUdpMessage returned " << status << " (bytes sent)");
//     // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
//     m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS());
//     m_execInterface.notifyOfExternalEvent();
//     // Clean up some (one hopes)
//     delete udp_buffer;
//  }

  // GET_PARAMETER_COMMAND
  void UdpAdapter::executeGetParameterCommand(const std::vector<Value>& args, ExpressionId dest, ExpressionId ack)
  {
    assertTrueMsg(args.size() == 1 || args.size() == 2,
                  "UdpAdapter: The " << GET_PARAMETER_COMMAND().c_str() << " command requires either one or two arguments");
    assertTrueMsg(args.front().isString(),
                  "UdpAdapter: The first argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, "
                  << args.front() << ", is not a string");
    // Extract the message name and try to verify the number of parameters defined vs the number of args used in the plan
    std::string msgName = args.front().getStringValue();
    debugMsg("UdpAdapter::executeGetParameterCommand",
             " " << msgName << ", dest==" << dest << ", ack==" << ack);
    size_t pos;
    pos = msgName.find(":");
    msgName = msgName.substr(0, pos);
    MessageMap::iterator msg;
    msg=m_messages.find(msgName);
    assertTrueMsg(msg != m_messages.end(), "UdpAdapter::executeGetParameterCommand: no message definition found for " << msgName);
    int params = msg->second.parameters.size();
    //debugMsg("UdpAdapter::executeGetParameterCommand", " msgName==" << msgName << ", params==" << params);
    std::vector<Value>::const_iterator it = ++args.begin();
    int id;
    if (it == args.end())
      {
        id = 0;
      }
    else
      {
        assertTrueMsg(it->isInteger(),
                      "UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not an integer");
        id = it->getIntValue();
        assertTrueMsg(id >= 0,
                      "UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND().c_str() << " command, " << *it
                      << ", is not a valid index");
        // Brute strength error check for the plan using a message/command with to many arguments.
        // The intent is that this might be discovered during development.
        assertTrueMsg(id < params,
                      "UdpAdapter: the message \"" << msgName << "\" is defined to have " << params
                      << " parameters in the XML configuration file, but is being used in the plan with "
                      << id+1 << " arguments");
      }
    LabelStr command = formatMessageName(args.front().c_str(), GET_PARAMETER_COMMAND(), id);
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeGetParameterCommand", " message handler for \"" << command.c_str() << "\" registered");
  }

  // SEND_RETURN_VALUE_COMMAND
  void UdpAdapter::executeSendReturnValueCommand(const std::vector<Value>& args, ExpressionId dest, ExpressionId ack)
  {
    // Open loop communications only.  Perhaps this is being called by the expanded nodes?
    //debugMsg("UdpAdapter::executeSendReturnValueCommand", " called for " << LabelStr::c_str(args.front()));
  }

  // SEND_MESSAGE_COMMAND
  void UdpAdapter::executeSendMessageCommand(const std::vector<Value>& args, ExpressionId /* dest */, ExpressionId ack)
  {
    // Check for one argument, the message
    assertTrueMsg(args.size() == 1, "UdpAdapter: The SendMessage command requires exactly one argument");
    assertTrueMsg(args.front().isString(),
                  "UdpAdapter: The argument to the SendMessage command, "
                  << args.front()
                  << ", is not a string");
    const char* theMessage = args.front().c_str();
    debugMsg("UdpAdapter::executeSendMessageCommand", " SendMessage(\"" << theMessage << "\")");
    // store ack
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("UdpAdapter::executeSendMessageCommand", " message \"" << theMessage << "\" sent.");
  }

  //
  // XML Support
  //

  void UdpAdapter::parseXmlMessageDefinitions(const pugi::xml_node& xml)
  // Parse and verify the given Adapter configuration
  {
    m_messages.clear();         // clear the old messages (if any)
    // First, set up the internal debugging output
    const char* debug = xml.attribute("debug").value();
    assertTrueMsg((*debug == '\0' || stricmp(debug, "true") == 0 || stricmp(debug, "false") == 0),
                  "parseXmlMessageDefinitions: debug must be a boolean, not " << debug);
    if (*debug && stricmp(debug, "true") == 0) m_debug = true;
    // Now, do the real work of parsing the XML UDP Configuration
    const pugi::xml_attribute default_local_port = xml.attribute("default_local_port");
    const pugi::xml_attribute default_peer_port = xml.attribute("default_peer_port");
    const char* default_peer = xml.attribute("default_peer").value();
    if (default_local_port) m_default_local_port = default_local_port.as_uint();
    if (default_peer_port) m_default_peer_port = default_peer_port.as_uint();
    if (*default_peer) m_default_peer = default_peer;
    // Walk the messages
    for (pugi::xml_node child = xml.first_child(); !child.empty(); child = child.next_sibling())
      {
        UdpMessage msg;
        const char* name = child.attribute("name").value(); // name is required, hence...
        assertTrueMsg(*name, "parseXmlMessageDefinitions: no name given in <Message/>");
        msg.name = name;
        const char* peer = child.attribute("peer").value(); // needed for bool test below (i.e., it is optional)

        pugi::xml_attribute local_port = child.attribute("local_port");
        pugi::xml_attribute peer_port = child.attribute("peer_port");
        // Use either the given peer, the default_peer, or "localhost"
        msg.peer = *peer ? peer : (*default_peer ? m_default_peer : "localhost");
        // Warn about possible run time errors (planners may simply not use a message I suppose)
        if (!(default_local_port || local_port))
          std::cout << "Warning: no default or message specific local port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if "
                    << name << "it is called to send an outgoing command/message\n";
        if (!(default_peer_port || peer_port))
          std::cout << "Warning: no default or message specific peer port given for <Message name=\""
                    << name << "\"/>\n         this will cause a run time error if "
                    << name << "it is called to receive an incoming command/message\n";
        msg.local_port = local_port ? local_port.as_uint() : m_default_local_port;
        msg.peer_port = peer_port ? peer_port.as_uint() : m_default_peer_port;
        // Walk the <Parameter/> elements of this <Message/>
        for (pugi::xml_node param = child.first_child(); !param.empty(); param = param.next_sibling())
          {
            Parameter arg;
            // Get the description (if any)
            pugi::xml_attribute param_desc = param.attribute("desc");
            if (param_desc)
              arg.desc = param_desc.value(); // only assign it if it exists
            // Get the (required) type
            const char* param_type = param.attribute("type").value();
            assertTrueMsg(*param_type, "parseXmlMessageDefinitions: no type for parameter given in <Message name=\""
                          << name << "\"/>");
            arg.type = param_type;
            // Get the length, which is required
            pugi::xml_attribute len = param.attribute("bytes");
            assertTrueMsg(len, "parseXmlMessageDefinitions: no parameter length (in bytes) given in <Message name=\""
                          << name << "\"/>");
            arg.len = len.as_uint();
            assertTrueMsg((arg.len > 0),
                          "parseXmlMessageDefinitions: zero length (in bytes) parameter given in <Message name=\""
                          << name << "\"/>");
            // Get the number of elements for the array types
            int size = 1;
            pugi::xml_attribute param_elements = param.attribute("elements");
            if (param_elements) size = param_elements.as_uint();
            arg.elements = size;
            // Do some error checking for reasonable/usable encoding/decoding byte lengths
            if (arg.type.find("array") != std::string::npos)
              {
                //printf("\nsize==%d, find==%d\n", size, arg.type.find("array"));
                assertTrueMsg(param_elements,
                              "parseXmlMessageDefinitions: arrays must have a size element, "
                              << "<Message=\"" << name << "\"/> does not");
              }
            if ((arg.type.compare("int") == 0) || (arg.type.compare("int-array") == 0))
              {
                assertTrueMsg((arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: integers must be 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            else if ((arg.type.compare("float") == 0) || (arg.type.compare("float-array") == 0))
              {
                assertTrueMsg((arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: floats must be 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            else if ((arg.type.compare("bool") == 0) || (arg.type.compare("bool-array") == 0))
              {
                assertTrueMsg((arg.len==1 || arg.len==2 || arg.len==4),
                              "parseXmlMessageDefinitions: booleans must be 1, 2 or 4 bytes, not " << arg.len
                              << " (in <Message name=\"" << name << "\"/>)");
              }
            // what about strings? -- fixed length to start with I suppose...
            else if ((arg.type.compare("string") == 0) || (arg.type.compare("string-array") == 0))
              {
                assertTrueMsg(arg.len>=1,
                             "parseXmlMessageDefinitions: strings must be 1 byte or longer (in <Message name=\""
                              << name << "\"/>)");
              }
            else
              {
                assertTrueMsg(0, "parseXmlMessageDefinitions: unknown parameter type \"" << arg.type
                              << "\" (in <Message name=\"" << name << "\"/>)");
              }
            msg.len += arg.len * size; // only arrays are not of size 1
            msg.parameters.push_back(arg);
          }
        m_messages[child.attribute("name").value()]=msg; // record the message with the name as the key
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
        std::vector<Parameter>::iterator param;
        for (param=msg->second.parameters.begin(); param != msg->second.parameters.end(); param++)
          {
            std::string temp = param->desc.empty() ? " (no description)" : " (" + param->desc + ")";
            if (param->elements == 1)
              {
                std::cout << "\n" << indent << param->len << " byte " << param->type << temp;
              }
            else
              {
                size_t pos = param->type.find("-");
                std::cout << "\n" << indent << param->elements << " element array of " << param->len << " byte "
                          << param->type.substr(0, pos) << "s" << temp;
              }
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
    //ThreadMutexGuard guard(m_cmdMutex);
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
    // Try to set up the socket so that we can close it later if the thread is cancelled
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    assertTrueMsg(sock > 0, "UdpAdapter::startUdpMessageReceiver: call to socket() failed");
    debugMsg("UdpAdapter::startUdpMessageReceiver", " " << name.toString() << " socket (" << sock << ") opened");
    msg->second.sock = sock; // pass the socket descriptor to waitForUdpMessage, which will then reset it
    pthread_t thread_handle;
    // Spawn the listener thread
    threadSpawn((THREAD_FUNC_PTR) waitForUdpMessage, &msg->second, thread_handle);
    // Check to see if the thread got started correctly
    assertTrueMsg(thread_handle != 0, "UdpAdapter::startUdpMessageReceiver: threadSpawn return NULL");
    debugMsg("UdpAdapter::startUdpMessageReceiver",
             " " << name.toString() << " listener thread (" << thread_handle << ") spawned");
    // Record the thread and socket in case they have to be cancelled and closed later (in invokeAbort)
    m_activeThreads[name.c_str()]=thread_handle;
    m_activeSockets[name.c_str()]=sock;
    return 0;
  }

  void* UdpAdapter::waitForUdpMessage(UdpMessage* msg)
  {
    debugMsg("UdpAdapter::waitForUdpMessage", " called for " << msg->name);
    // A pointer to the adapter
    UdpAdapter* udpAdapter = reinterpret_cast<UdpAdapter*>(msg->self);
    //int local_port = msg->local_port;
    size_t size = msg->len;
    udp_thread_params params;
    params.local_port = msg->local_port;
    params.buffer = new unsigned char[size];
    params.size = size;
    params.debug = udpAdapter->m_debug; // see if debugging is enabled
    params.sock = msg->sock; // The socket opened in startUdpMessageReceiver
    msg->sock = 0;           // Reset the temporary socket descriptor in the UdpMessage
    int status = wait_for_input_on_thread(&params);
    assertTrueMsg(status==0, "waitForUdpMessage call to wait_for_input_on_thread returned " << status);
    // When the message has been received, tell the UdpAdapter about it and its contents
    status = udpAdapter->handleUdpMessage(msg, params.buffer, params.debug);
    assertTrueMsg(status==0, "waitForUdpMessage call to handleUdpMessage returned " << status);
    delete[] params.buffer;     // release the input buffer
    return (void*) 0;
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
    const LabelStr msg_name = formatMessageName(msgDef->name.c_str(), RECEIVE_COMMAND_COMMAND());
    m_messageQueues.addMessage(msg_name, msg_label);
    // (2) walk the parameters, and for each, call addMessage(label, <value-or-key>), which
    //     (somehow) arranges for executeCommand(GetParameter) to be called, and which in turn
    //     calls addRecipient and updateQueue
    int i = 0;
    int offset = 0;
    std::vector<Parameter>::const_iterator param;
    for (param=msgDef->parameters.begin() ; param != msgDef->parameters.end() ; param++, i++)
      {
        const LabelStr param_label = formatMessageName(msg_label, GET_PARAMETER_COMMAND(), i);
        int len = param->len;   // number of bytes to read
        int size = param->elements; // size of the array, or 1 for scalars
        std::string type = param->type; // type to decode
        if (debug) {
          if (size==1)
            {
              std::cout << "  handleUdpMessage: decoding " << len << " byte " << type
                        << " starting at buffer[" << offset << "]: ";
            }
          else
            {
              size_t pos = type.find("-"); // remove the "-array" from the type
              std::cout << "  handleUdpMessage: decoding " << size << " element array of " << len
                        << " byte " << type.substr(0, pos) << "s starting at buffer[" << offset
                        << "]: ";
            }
        }
        if (type.compare("int") == 0)
          {
            assertTrueMsg((len==2 || len==4), "handleUdpMessage: Integers must be 2 or 4 bytes, not " << len);
            int num;
            num = (len == 2) ? decode_short_int(buffer, offset) : decode_long_int(buffer, offset);
            if (debug) std::cout << num << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing numeric (integer) parameter " << num);
            m_messageQueues.addMessage(param_label, Value(num));
            offset += len;
          }
        else if (type.compare("int-array") == 0)
          {
            assertTrueMsg((len==2 || len==4), "handleUdpMessage: Integers must be 2 or 4 bytes, not " << len);
            StoredArray array(size, Value(0));
            for (int i = 0 ; i < size ; i++)
              {
                array[i] = Value((int32_t) ((len == 2) ? decode_short_int(buffer, offset) : decode_long_int(buffer, offset)));
                offset += len;
              }
            if (debug) std::cout << array.toString() << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing numeric (integer) array " << array.toString());
            m_messageQueues.addMessage(param_label, array);
          }
        else if (type.compare("float") == 0)
          {
            assertTrueMsg(len==4, "handleUdpMessage: Reals must be 4 bytes, not " << len);
            float num;
            num = decode_float(buffer, offset);
            if (debug) std::cout << num << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing numeric (real) parameter " << num);
            m_messageQueues.addMessage(param_label, Value((double) num));
            offset += len;
          }
        else if (type.compare("float-array") == 0)
          {
            assertTrueMsg(len==4, "handleUdpMessage: Reals must be 4 bytes, not " << len);
            StoredArray array(size, Value(0.0));
            for (int i = 0 ; i < size ; i++)
              {
                array[i] = Value(decode_float(buffer, offset));
                offset += len;
              }
            if (debug) std::cout << array.toString() << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing numeric (real) array " << array.toString());
            m_messageQueues.addMessage(param_label, Value(array));
          }
        else if (type.compare("bool") == 0)
          {
            assertTrueMsg((len==1 || len==2 || len==4), "handleUdpMessage: Booleans must be 1, 2 or 4 bytes, not " << len);
            int num;
            switch (len)
              {
              case 1: num = network_bytes_to_number(buffer, offset, 8, false); break;
              case 2: num = decode_short_int(buffer, offset); break;
              default: num = decode_long_int(buffer, offset);
              }
            if (debug) std::cout << num << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing numeric (boolean) parameter " << num);
            m_messageQueues.addMessage(param_label, Value((double) num));
            offset += len;
          }
        else if (type.compare("bool-array") == 0)
          {
            assertTrueMsg((len==1 || len==2 || len==4), "handleUdpMessage: Booleans must be 1, 2 or 4 bytes, not " << len);
            StoredArray array(size, Value(false));
            for (int i = 0 ; i < size ; i++)
              {
                switch (len)
                  {
                  case 1: array[i] = Value((double) network_bytes_to_number(buffer, offset, 8, false)); break;
                  case 2: array[i] = Value((double) decode_short_int(buffer, offset)); break;
                  default: array[i] = Value((double) decode_long_int(buffer, offset)); break;
                  }
                offset += len;
              }
            if (debug) std::cout << array.toString() << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queueing boolean array " << array.toString());
            m_messageQueues.addMessage(param_label, Value(array));
          }
        else if (type.compare("string-array") == 0)
          {
            // XXXX For unknown reasons, OnCommand(... String arg); is unable to receive this (inlike int and float arrays)
            StoredArray array(size, LabelStr());
            for (int i = 0 ; i < size ; i++)
              {
                std::string str = decode_string(buffer, offset, len);
                array[i] = Value(str);
                offset += len;
              }
            if (debug) std::cout << array.toString() << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queuing string array " << array.toString());
            m_messageQueues.addMessage(param_label, Value(array));
          }
        else // string or die
          {
            assertTrueMsg(!type.compare("string"), "handleUdpMessage: unknown parameter type " << type.c_str());
            std::string str = decode_string(buffer, offset, len);
            if (debug) std::cout << str << std::endl;
            debugMsg("UdpAdapter::handleUdpMessage", " queuing string parameter \"" << str << "\"");
            m_messageQueues.addMessage(param_label, Value(str));
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

  int UdpAdapter::buildUdpBuffer(unsigned char* buffer, const UdpMessage& msg, const std::vector<Value>& args,
                                 bool skip_arg, bool debug)
  {
    std::vector<Value>::const_iterator it;
    std::vector<Parameter>::const_iterator param;
    int start_index = 0; // where in the buffer to write
    // Do what error checking we can, since we absolutely know that planners foul this up.
    debugMsg("UdpAdapter::buildUdpBuffer", " args.size()==" << args.size() << ", parameters.size()==" << msg.parameters.size());
    size_t param_count = msg.parameters.size();
    if (skip_arg) param_count++;
    assertTrueMsg((args.size() == param_count),
                  "the " << param_count << " parameters defined in the XML configuration file do not match the "
                  << args.size() << " para-maters used in the plan for <Message name=\"" << msg.name << "\"/>");
    // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
    for (param=msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(); param++, it++)
      {
        if (skip_arg) { it++; skip_arg = false; } // only skip the first arg
        unsigned int len = param->len;
        std::string type = param->type;
        const Value& plexil_val = *it;
        // The parameter passed will be one of these two
        if (debug) std::cout << "  buildUdpBuffer: encoding ";
        // Encode only 32 bit entities (i.e., no 64 bit reals/ints)
        if (type.compare("int") == 0)
          {
            assertTrueMsg((len==2 || len==4), "buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            // Large ints (> 32 bits) are caught in Expression.cc:337
            if (debug) std::cout << len << " byte int starting at buffer[" << start_index << "]: " << plexil_val.getIntValue();
            if (len==2)
              {
                int temp = plexil_val.getIntValue();
                assertTrueMsg(SHRT_MIN <= temp && temp <= SHRT_MAX,
                              "buildUdpBuffer: 2 bytes integers must be between " << SHRT_MIN << " and " << SHRT_MAX
                              << ", " << temp << " is not");
                encode_short_int(temp, buffer, start_index);
              }
            else
              encode_long_int(plexil_val.getIntValue(), buffer, start_index);
            start_index += len;
          }
        else if (type.compare("int-array") == 0)
          {
            assertTrueMsg((len==2 || len==4), "buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            unsigned int size = param->elements; // XXXX
            StoredArray array = plexil_val;
            if (debug) std::cout << size << " element array of " << len << " byte ints starting at ["
                                 << start_index << "]: " << array.toString();
            assertTrueMsg(size==array.size(), "buildUdpBuffer: declared and actual array sizes differ: "
                          << size << " was delcared, but " << array.size() << " is being used in the plan");
            for (unsigned int i = 0 ; i < size ; i++)
              {
                int temp = array[i].getIntValue();
                if (len==2)
                  {
                    assertTrueMsg(SHRT_MIN <= temp && temp <= SHRT_MAX,
                                  "buildUdpBuffer: 2 bytes integers must be between " << SHRT_MIN << " and " << SHRT_MAX
                                  << ", " << temp << " is not");
                    encode_short_int(temp, buffer, start_index);
                  }
                else
                  encode_long_int(temp, buffer, start_index);
                start_index += len;
              }
          }
        else if (type.compare("float-array") == 0)
          {
            assertTrueMsg(len==4, "buildUdpBuffer: Reals must be 4 bytes, not " << len);
            unsigned int size = param->elements;
            assertTrueMsg(size >= 1, "buildUdpBuffer: all scalars and arrays must be of at least size 1, not " << size);
            StoredArray array = plexil_val;
            if (debug) std::cout << size << " element array of " << len << " byte floats starting at buffer["
                                 << start_index << "]: " << array.toString();
            assertTrueMsg(size==array.size(), "buildUdpBuffer: declared and actual (float) array sizes differ: "
                          << size << " was delcared, but " << array.size() << " is being used in the plan");
            for (unsigned int i = 0 ; i < size ; i++)
              {
                double temp = array[i].getDoubleValue();
                assertTrueMsg((-FLT_MAX) <= temp && temp <= FLT_MAX,
                              "buildUdpBuffer: Reals (floats) must be between " << (-FLT_MAX) << " and " << FLT_MAX <<
                              ", " << temp << " is not");
                encode_float((float) temp, buffer, start_index);
                start_index += len;
              }
          }
        else if (type.compare("float") == 0)
          {
            double temp = plexil_val.getDoubleValue();
            assertTrueMsg(len==4, "buildUdpBuffer: Reals must be 4 bytes, not " << len);
            // Catch really big floats
            assertTrueMsg((-FLT_MAX) <= temp && temp <= FLT_MAX,
                          "buildUdpBuffer: Reals (floats) must be between " << (-FLT_MAX) << " and " << FLT_MAX <<
                          ", not " << plexil_val);
            if (debug) std::cout << len << " byte float starting at buffer[" << start_index << "]: " << temp;
            encode_float((float) temp, buffer, start_index);
            start_index += len;
          }
        else if (type.compare("bool-array") == 0)
          {
            assertTrueMsg((len==1 || len==2 || len==4), "buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            unsigned int size = param->elements;
            StoredArray array = plexil_val;
            if (debug) std::cout << size << " element array of " << len << " byte booleans starting at buffer["
                                 << start_index << "]: " << array.toString();
            assertTrueMsg(size==array.size(), "buildUdpBuffer: declared and actual (boolean) array sizes differ: "
                          << size << " was delcared, but " << array.size() << " is being used in the plan");
            for (unsigned int i = 0 ; i < size ; i++)
              {
                assertTrueMsg(!array[i].isUnknown(), "buildUdpBuffer: Booleans must be either true ("
                              << true << ") or false (" << false << ")" << ", not UNKNOWN");
                bool temp = array[i].getBoolValue();
                switch (len)
                  {
                  case 1: number_to_network_bytes(temp, buffer, start_index, 8, false); break;
                  case 2: encode_short_int(temp, buffer, start_index); break;
                  default: encode_long_int(temp, buffer, start_index);
                  }
                start_index += len;
              }
          }
        else if (type.compare("bool") == 0) // these are 64 bits in Plexil
          {
            assertTrueMsg((len==1 || len==2 || len==4), "buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            assertTrueMsg(plexil_val.isBoolean(), "buildUdpBuffer: Booleans must be either true ("
                          << true << ") or false (" << false << ")" << ", not " << plexil_val);
            if (debug)
              std::cout << len << " byte bool starting at buffer[" << start_index << "]: " << plexil_val.getBoolValue();
            switch (len)
              {
              case 1: number_to_network_bytes(plexil_val.getBoolValue(), buffer, start_index, 8, false); break;
              case 2: encode_short_int(plexil_val.getBoolValue(), buffer, start_index); break;
              default: encode_long_int(plexil_val.getBoolValue(), buffer, start_index);
              }
            start_index += len;
          }
        else if (type.compare("string-array") == 0)
          {
            unsigned int size = param->elements;
            StoredArray array = plexil_val;
            if (debug) std::cout << size << " element array of " << len << " byte strings starting at buffer["
                                 << start_index << "]: " << array.toString();
            assertTrueMsg(size==array.size(), "buildUdpBuffer: declared and actual (string) array sizes differ: "
                          << size << " was delcared, but " << array.size() << " is being used in the plan");
            for (unsigned int i = 0 ; i < size ; i++)
              {
                const std::string& str = array[i].getStringValue();
                assertTrueMsg(str.length()<=len, "buildUdpBuffer: declared string length (" << len <<
                              ") and actual length (" << str.length() << ", " << str.c_str() <<
                              ") used in the plan are not compatible");
                encode_string(str, buffer, start_index);
                start_index += len;
              }
          }
        else
          {
            assertTrueMsg(!type.compare("string"), "buildUdpBuffer: unknown parameter type " << type.c_str());
            const std::string& str = plexil_val.getStringValue();
            assertTrueMsg(str.length()<=len, "buildUdpBuffer: declared string length (" << len <<
                          ") and actual length (" << str.length() << ", " << str.c_str() <<
                          ") used in the plan are not compatible");
            if (debug) std::cout << len << " byte string starting at buffer[" << start_index << "]: " << str;
            encode_string(str, buffer, start_index);
            start_index += len;
          }
        if (debug) std::cout << std::endl;
      }
    if (debug)
      {
        std::cout << "  buildUdpBuffer: buffer: ";
        print_buffer(buffer, msg.len);
      }
    return start_index;
  }

  void UdpAdapter::printMessageContent(const LabelStr& name, const std::vector<Value>& args)
  {
    // Print the content of a message
    std::vector<Value>::const_iterator it;
    std::cout << "Message: " << name.c_str() << ", Params:";
    for (it=args.begin(); it != args.end(); it++) {
      // Real, Integer, Boolean, String (and Array, maybe...)
      // Integers and Booleans are represented as Real (oops...)
      std::cout << " ";
      if (it->isString()) // Extract strings
        std::cout << "\"" << it->getStringValue() << "\"";
      else { // Extract numbers (bool, float, int)
        std::cout << *it;
      }
    }
    std::cout << std::endl;
  }

  LabelStr UdpAdapter::formatMessageName(const LabelStr& name, const LabelStr& command, int id)
  {
    return formatMessageName(name.c_str(), command, 0);
  }

  LabelStr UdpAdapter::formatMessageName(const LabelStr& name, const LabelStr& command)
  {
    return formatMessageName(name.c_str(), command, 0);
  }

  LabelStr UdpAdapter::formatMessageName(const char* name, const LabelStr& command, int id)
  {
    std::ostringstream ss;
    if (command == RECEIVE_COMMAND_COMMAND())
      ss << COMMAND_PREFIX();
    else if (command == GET_PARAMETER_COMMAND())
      ss << PARAM_PREFIX();
    ss << name << '_' << id;
    return LabelStr(ss.str());
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
