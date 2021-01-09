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

#include "plexil-stdint.h" // includes plexil-config.h

#include "AdapterConfiguration.hh"
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"    // REGISTER_ADAPTER() macro
#include "ArrayImpl.hh"
#include "Command.hh"
#include "Configuration.hh"
#include "Debug.hh"
#include "Error.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceError.hh"
#include "MessageQueueMap.hh"
#include "StateCacheEntry.hh"
#include "ThreadSpawn.hh"
#include "udp-utils.hh"

#include "pugixml.hpp"

#include <mutex>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CFLOAT)
#include <cfloat>
#elif defined(HAVE_FLOAT_H)
#include <float.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h> // close()
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

namespace PLEXIL
{
  //
  // Local constants
  //

  // Command names
  static constexpr char const SEND_MESSAGE_COMMAND[] = "SendMessage";
  static constexpr char const RECEIVE_COMMAND_COMMAND[] = "ReceiveCommand";
  static constexpr char const GET_PARAMETER_COMMAND[] = "GetParameter";
  static constexpr char const SEND_RETURN_VALUE_COMMAND[] = "SendReturnValue";

  // Message formatting
  static constexpr char const COMMAND_PREFIX[] = "__COMMAND__";
  static constexpr char const PARAM_PREFIX[] = "__PARAMETER__";

  //
  // Local helper functions
  //

  static std::string formatMessageName(const std::string& name,
                                       const std::string& command,
                                       int /* id */)
  {
    return formatMessageName(name.c_str(), command, 0);
  }

  static std::string formatMessageName(const std::string& name,
                                       const std::string& command)
  {
    return formatMessageName(name.c_str(), command, 0);
  }

  static std::string formatMessageName(const char* name,
                                       const std::string& command,
                                       int id)
  {
    std::ostringstream ss;
    if (command == RECEIVE_COMMAND_COMMAND)
      ss << COMMAND_PREFIX;
    else if (command == GET_PARAMETER_COMMAND)
      ss << PARAM_PREFIX;
    ss << name << '_' << id;
    debugMsg("UdpAdapter:formatMessageName", " returning " << ss.str());
    return ss.str();
  }

  struct Parameter final
  {
    std::string desc;           // optional parameter description
    std::string type;           // int|float|bool|string|int-array|float-array|string-array|bool-array
    unsigned int len;           // number of bytes for type (or array element)
    unsigned int elements;      // number of elements in the array (non-array types are 0 or 1?)
  };

  struct UdpMessage final
  {
    std::string name;                // the Plexil Command name
    std::string peer;                // peer to which to send
    std::vector<Parameter> parameters; // message value parameters
    void *self;                      // reference to the UdpAdapter for use in message decoding
    unsigned int len;                         // the length of the message in bytes
    unsigned int local_port;                  // local port on which to receive
    unsigned int peer_port;                   // port to which to send
    int sock;                        // socket to use -- only meaningful in call to waitForUdpMessage
    UdpMessage()
      : name(),
        peer(),
        parameters(),
        self(nullptr),
        len(0),
        local_port(0),
        peer_port(0),
        sock(0)
    {}
    UdpMessage(std::string nam)
      : name(nam),
        peer(),
        parameters(),
        self(nullptr),
        len(0),
        local_port(0),
        peer_port(0),
        sock(0)
    {}
    UdpMessage(UdpMessage const &) = default;
    UdpMessage(UdpMessage &&) = default;
    UdpMessage &operator=(UdpMessage const &) = default;
    UdpMessage &operator=(UdpMessage &&) = default;
    ~UdpMessage() = default;
  };

  class UdpAdapter : public InterfaceAdapter
  {
  private:
    //
    // Local types
    //
    using MessageMap = std::map<std::string, UdpMessage>;
    using ThreadMap = std::map<std::string, pthread_t>;
    using SocketMap = std::map<std::string, int>;

  public:

    //! Constructor
    UdpAdapter(AdapterExecInterface &execInterface, AdapterConf *conf)
      : InterfaceAdapter(execInterface, conf),
        m_default_peer("localhost"),
        m_messageQueues(execInterface),
        m_default_local_port(0),
        m_default_peer_port(0),
        m_debug(false)
    {
      debugMsg("UdpAdapter", " constructor");
    }

    //! Virtual destructor
    virtual ~UdpAdapter()
    {
      debugMsg("UdpAdapter", " destructor");
    }

    // Initialize
    virtual bool initialize(AdapterConfiguration *config) override
    {
      debugMsg("UdpAdapter:initialize", " called");

      // Register the basic command handlers
      config->registerCommandHandler(new SendMessageHandler(this),
                                     std::string(SEND_MESSAGE_COMMAND));
      config->registerCommandHandler(new ReceiveCommandHandler(this),
                                     std::string(RECEIVE_COMMAND_COMMAND));
      config->registerCommandHandler(new GetParameterHandler(this),
                                     std::string(GET_PARAMETER_COMMAND));
      config->registerCommandHandler(new SendReturnValueHandler(this),
                                     std::string(SEND_RETURN_VALUE_COMMAND));

      pugi::xml_node const xml = getXml();

      // Enable debug output if requested
      m_debug = xml.attribute("debug").as_bool();

      // Parsing the UDP configuration
      m_default_local_port = xml.attribute("default_local_port").as_uint(m_default_local_port);
      // TODO: range check
      m_default_peer_port = xml.attribute("default_peer_port").as_uint(m_default_peer_port);
      // TODO: range check
      m_default_peer = xml.attribute("default_peer").as_string(m_default_peer.c_str());
      if (m_default_peer.empty()) {
        warn("UdpAdapter: empty default_peer value supplied");
      }
      
      // parse the message definitions and register the commands
      if (!parseMessageDefinitions(config)) {
        debugMsg("UdpAdapter:initialize", " message definition parsing failed");
        return false;
      }
      if (m_debug)
        printMessageDefinitions();

      debugMsg("UdpAdapter:initialize", " succeeded");
      return true;
    }

    // Start method
    virtual bool start() override
    {
      debugMsg("UdpAdapter:start()", " called");
      // Start the UDP listener thread
      return true;
    }

    // Stop method
    virtual bool stop() override
    {
      debugMsg("UdpAdapter:stop", " called");
      // Stop the UDP listener thread
      return true;
    }

  private:

    //
    // Command handlers
    //

    // Base class, handles user commands
    
    struct UdpCommandHandler : public CommandHandler
    {
      UdpCommandHandler(UdpAdapter *adapter)
        : CommandHandler(),
          m_adapter(adapter)
      {}
      virtual ~UdpCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->executeDefaultCommand(cmd);
      }
      virtual void abortCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->abortCommand(cmd, true);
      }
    protected:
      UdpAdapter *m_adapter;
    };

    //
    // Specialized command handlers
    //

    struct SendMessageHandler final : public UdpCommandHandler
    {
      SendMessageHandler(UdpAdapter *adapter)
        : UdpCommandHandler(adapter)
      {}
      virtual ~SendMessageHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->executeSendMessageCommand(cmd);
      }
    };

    struct GetParameterHandler final : public UdpCommandHandler
    {
      GetParameterHandler(UdpAdapter *adapter)
        : UdpCommandHandler(adapter)
      {}
      virtual ~GetParameterHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->executeGetParameterCommand(cmd);
      }
    };

    struct SendReturnValueHandler final : public UdpCommandHandler
    {
      SendReturnValueHandler(UdpAdapter *adapter)
        : UdpCommandHandler(adapter)
      {}
      virtual ~SendReturnValueHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->executeSendReturnValueCommand(cmd);
      }
    };

    struct ReceiveCommandHandler final : public UdpCommandHandler
    {
      ReceiveCommandHandler(UdpAdapter *adapter)
        : UdpCommandHandler(adapter)
      {}
      virtual ~ReceiveCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->executeReceiveCommandCommand(cmd);
      }
      virtual void abortCommand(Command *cmd, AdapterExecInterface * /* intf */) override
      {
        m_adapter->abortReceiveCommandCommand(cmd);
      }
    };

  public: 

    //
    // Implementation methods for the command handlers
    // Public so command handlers can see them
    //

    // Generic command
    void executeDefaultCommand(Command *cmd)
    {
      std::vector<Value> const &args = cmd->getArgValues();
      if (args.size() == 0) {
        warn("UdpAdapter:executeDefaultCommand: command requires at least one argument");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      if (args[0].valueType() != STRING_TYPE) {
        warn("UdpAdapter:executeDefaultCommand: message name must be a string");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      std::string const &msgName = cmd->getName();
      debugMsg("UdpAdapter:executeDefaultCommand",
               " called for \"" << msgName << "\" with " << args.size() << " args");
      std::lock_guard<std::mutex> guard(m_cmdMutex);
      MessageMap::const_iterator msg = m_messages.find(msgName);
      // Check for an obviously bogus port
      if (msg->second.peer_port == 0) {
        warn("executeDefaultCommand: bad peer port (0) given for " << msgName << " message");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      // Set up the outgoing UDP buffer to be sent
      int length = msg->second.len;
      unsigned char* udp_buffer = new unsigned char[length]; // fixed length to start with
      memset((char*)udp_buffer, 0, length); // zero out the buffer
      // Walk the parameters and encode them in the buffer to be sent out
      if (0 > buildUdpBuffer(udp_buffer, msg->second, args, false, m_debug)) {
        warn("executeDefaultCommand: error formatting buffer");
        delete[] udp_buffer;
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      // Send the buffer to the given host:port
      int status = sendUdpMessage(udp_buffer, msg->second, m_debug);
      debugMsg("UdpAdapter:executeDefaultCommand",
               " sendUdpMessage returned " << status << " (bytes sent)");
      // Clean up some (one hopes)
      delete[] udp_buffer;
      // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
      getInterface().handleCommandAck(cmd, COMMAND_SUCCESS);
      getInterface().notifyOfExternalEvent();
    }

    // RECEIVE_COMMAND_COMMAND
    void executeReceiveCommandCommand(Command *cmd)
    {
      std::vector<Value> const &args = cmd->getArgValues();
      if (args.size() != 1) {
        warn("UdpAdapter: The " << RECEIVE_COMMAND_COMMAND
             << " command requires exactly one argument");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      if (args.front().valueType() != STRING_TYPE) {
        warn("UdpAdapter: The argument to the " << RECEIVE_COMMAND_COMMAND
             << " command, " << args.front() << ", is not a string");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      std::string msgName;
      if (!args.front().getValue(msgName)) {
        warn("UdpAdapter:executeDefaultCommand: message name is unknown");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      debugMsg("UdpAdapter:executeReceiveCommandCommand", " called for " << msgName);
      std::string command = formatMessageName(msgName, RECEIVE_COMMAND_COMMAND);
      m_messageQueues.addRecipient(command, cmd);
      // Set up the thread on which the message may/will eventually be received
      int status = startUdpMessageReceiver(msgName, cmd);
      if (status) {
        warn("executeReceiveCommandCommand: startUdpMessageReceiver failed");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
      }
      else {
        debugMsg("UdpAdapter:executeReceiveCommandCommand",
                 " message handler for \"" << command << "\" registered");
        getInterface().handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
      }
      getInterface().notifyOfExternalEvent();
    }

    //   // RECEIVE_UDP_MESSAGE_COMMAND
    //   void executeReceiveUdpCommand(Command *cmd)
    //   {
    //     // Called when node _starts_ executing, so, record the message and args so that they can be filled in
    //     // if and when a UDP message comes in the fulfill this expectation.
    //     // First arg is message name (which better match one of the defined messages...)
    //     std::string command(args.front());
    //     debugMsg("UdpAdapter:executeReceiveUdpCommand", " " << command << ", dest==" << dest
    //              << ", ack==" << ack << ", args.size()==" << args.size());
    //     getInterface().handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
    //     getInterface().notifyOfExternalEvent();
    //     debugMsg("UdpAdapter:executeReceiveUdpCommand", " handler for \"" << command << "\" registered");
    //   }

    //  // SEND_UDP_MESSAGE_COMMAND
    //   void executeSendUdpMessageCommand(Command *cmd)
    //   {
    //     // First arg is message name (which better match one of the defined messages...)
    //     // Lookup the appropriate message in the message definitions in m_messages
    //     std::string msgName(args.front());
    //     debugMsg("UdpAdapter:executeSendUdpMessageCommand", " called for " << msgName);
    //     //printMessageContent(msgName, args);
    //     MessageMap::iterator msg;
    //     msg=m_messages.find(msgName);
    //     // Set up the outgoing UDP buffer to be sent
    //     int length = msg->second.len;
    //     unsigned char* udp_buffer = new unsigned char[length]; // fixed length to start with
    //     memset((char*)udp_buffer, 0, length); // zero out the buffer
    //     // Walk the parameters and encode them in the buffer to be sent out
    //     buildUdpBuffer(udp_buffer, msg->second, args, true, m_debug);
    //     // Send the buffer to the given host:port
    //     int status = -1;
    //     status = sendUdpMessage(udp_buffer, msg->second, m_debug);
    //     debugMsg("UdpAdapter:executeSendUdpMessageCommand", " sendUdpMessage returned " << status << " (bytes sent)");
    //     // Do the internal Plexil Boiler Plate (as per example in IpcAdapter.cc)
    //     getInterface().handleCommandAck(cmd, COMMAND_SUCCESS);
    //     getInterface().notifyOfExternalEvent();
    //     // Clean up some (one hopes)
    //     delete udp_buffer;
    //  }

    // GET_PARAMETER_COMMAND
    void executeGetParameterCommand(Command *cmd)
    {
      std::vector<Value> const &args = cmd->getArgValues();
      if (args.size() < 1 || args.size() > 2) {
        warn("UdpAdapter: The " << GET_PARAMETER_COMMAND
             << " command requires either one or two arguments");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      if (args.front().valueType() != STRING_TYPE) {
        warn("UdpAdapter: The oparameter name argument to the " << GET_PARAMETER_COMMAND
             << " command, " << args.front() << ", is not a string");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      // Extract the message name and try to verify the number of parameters defined vs the number of args used in the plan
      std::string msgName;
      if (!args.front().getValue(msgName)) {
        warn("UdpAdapter:executeGetParameterCommand: message name is unknown");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      debugMsg("UdpAdapter:executeGetParameterCommand", " " << msgName);
      size_t pos;
      pos = msgName.find(":");
      std::string const baseName = msgName.substr(0, pos);
      MessageMap::const_iterator msg = m_messages.find(baseName);
      if (msg == m_messages.end()) {
        warn("UdpAdapter:executeGetParameterCommand: no message definition found for "
             << baseName);
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      int params = msg->second.parameters.size();
      debugMsg("UdpAdapter:executeGetParameterCommand",
               " msgName==" << msgName << ", params==" << params);
      std::vector<Value>::const_iterator it = ++args.begin();
      int32_t id = 0;
      if (it != args.end()) {
        if (it->valueType() != INTEGER_TYPE) {
          warn("UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND
               << " command, " << *it << ", is not an integer");
          getInterface().handleCommandAck(cmd, COMMAND_FAILED);
          getInterface().notifyOfExternalEvent();
          return;
        }
        
        if (!it->getValue(id)) {
          warn("UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND
               << " command is unknown");
          getInterface().handleCommandAck(cmd, COMMAND_FAILED);
          getInterface().notifyOfExternalEvent();
          return;
        }

        if (id < 0) {
          warn("UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND
               << " command, " << *it << ", is not a valid index");
          warn("UdpAdapter: The second argument to the " << GET_PARAMETER_COMMAND
               << " command is unknown");
          getInterface().handleCommandAck(cmd, COMMAND_FAILED);
          getInterface().notifyOfExternalEvent();
          return;
        }

        // Brute strength error check for the plan using a message/command with to many arguments.
        // The intent is that this might be discovered during development.
        if (id >= params) {
          warn("UdpAdapter: the message \"" << msgName << "\" is defined to have " << params
               << " parameters in the XML configuration file, but is being used in the plan with "
               << id+1 << " arguments");
          getInterface().handleCommandAck(cmd, COMMAND_FAILED);
          getInterface().notifyOfExternalEvent();
          return;
        }
      }
      std::string command = formatMessageName(msgName, GET_PARAMETER_COMMAND, id);
      m_messageQueues.addRecipient(command, cmd);
      debugMsg("UdpAdapter:executeGetParameterCommand", " message handler for \"" << cmd->getName() << "\" registered");
      getInterface().handleCommandAck(cmd, COMMAND_SENT_TO_SYSTEM);
      getInterface().notifyOfExternalEvent();
    }

    // SEND_RETURN_VALUE_COMMAND
    // Required by OnCommand XML macro. No-op for UDP.
    void executeSendReturnValueCommand(Command * cmd)
    {
      getInterface().handleCommandAck(cmd, COMMAND_SUCCESS);
      getInterface().notifyOfExternalEvent();
    }

    // SEND_MESSAGE_COMMAND
    void executeSendMessageCommand(Command *cmd)
    {
      std::vector<Value> const &args = cmd->getArgValues();
      // Check for one argument, the message
      if (args.size() != 1) {
        warn("UdpAdapter: The SendMessage command requires exactly one argument");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      if (args.front().valueType() != STRING_TYPE) {
        warn("UdpAdapter: The message name argument to the SendMessage command, "
             << args.front() << ", is not a string");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }

      std::string theMessage;
      if (!args.front().getValue(theMessage)) {
        warn("UdpAdapter: The message name argument to the SendMessage command is unknown");
        getInterface().handleCommandAck(cmd, COMMAND_FAILED);
        getInterface().notifyOfExternalEvent();
        return;
      }
      
      debugMsg("UdpAdapter:executeSendMessageCommand", " SendMessage(\"" << theMessage << "\")");
      debugMsg("UdpAdapter:executeSendMessageCommand", " message \"" << theMessage << "\" sent.");
      // store ack
      getInterface().handleCommandAck(cmd, COMMAND_SUCCESS);
      getInterface().notifyOfExternalEvent();
    }

    //
    // Abort command methods
    //

    // Abort a Plexil Command
    // Also handy utility for the below
    void abortCommand(Command *cmd, bool status = true)
    {
      debugMsg("UdpAdapter:abortCommand",
               " for " << cmd->getName() << ", status = "
               << (status ? "true" : "false"));
      getInterface().handleCommandAbortAck(cmd, status);
      getInterface().notifyOfExternalEvent();
    }

    // Abort a ReceiveCommand command
    void abortReceiveCommandCommand(Command *cmd) 
    {
      std::vector<Value> const &cmdArgs = cmd->getArgValues();
      // Shouldn't be possible if the original command worked
      if (cmdArgs.size() < 1) {
        warn("UdpAdapter:abortCommand: Malformed ReceiveCommand command;\n"
             << " no command name supplied");
        abortCommand(cmd, false);
        return;
      }

      // Shouldn't be possible if the original command worked
      if (cmdArgs.front().valueType() != STRING_TYPE) {
        warn("UdpAdapter:abortCommand: Malformed ReceiveCommand command;\n"
             << "\n command name parameter value " << cmdArgs.front() << ", is not a String");
        abortCommand(cmd, false);
        return;
      }

      std::string msgName;
      // Shouldn't be possible if the original command worked
      if (!cmdArgs.front().getValue(msgName)) { // The defined message name, needed for looking up the thread and socket
        warn("UdpAdapter:abortCommand: ReceiveCommand command name argument is unknown");
        abortCommand(cmd, false);
        return;
      }
      
      debugMsg("UdpAdapter:abortCommand", " ReceiveCommand(\"" << msgName << "\")");
      int status;                        // The return status of the calls to pthread_cancel() and close()
      // First, find the active thread for this message, cancel and erase it
      ThreadMap::iterator thread = m_activeThreads.find(msgName); // recorded by startUdpMessageReceiver
      if (thread == m_activeThreads.end()) {
        warn("UdpAdapter::abortReceiveCommandCommand: no thread found for " << msgName);
        abortCommand(cmd, false);
        return;
      }

      if ((status = pthread_cancel(thread->second))) {
        warn("UdpAdapter::abortReceiveCommandCommand: pthread_cancel(" << thread->second
             << ") returned " << status << ", errno " << errno);
        abortCommand(cmd, false);
        return;
      }

      // Wait for cancelled thread to finish
      if ((status = pthread_join(thread->second, nullptr))) {
        warn("UdpAdapter::abortReceiveCommandCommand: pthread_join(" << thread->second
             << ") returned " << status << ", errno " << errno);
        abortCommand(cmd, false);
        return;
      }

      debugMsg("UdpAdapter::abortReceiveCommandCommand", " " << msgName
               << " listener thread (" << thread->second << ") cancelled");
      m_activeThreads.erase(thread); // erase the cancelled thread
      // Second, find the open socket for this message and close it
      SocketMap::iterator socket = m_activeSockets.find(msgName); // recorded by startUdpMessageReceiver
      if (socket == m_activeSockets.end()) {
        warn("UdpAdapter::abortReceiveCommandCommand: no socket found for " << msgName);
        abortCommand(cmd, false);
        return;
      }
      
      if ((status = close(socket->second))) {
        warn("UdpAdapter::abortReceiveCommandCommand: close(" << socket->second
             << ") returned " << status
             << ", errno " << errno);
        m_activeSockets.erase(socket); // erase the closed socket
        abortCommand(cmd, false);
        return;
      }

      debugMsg("UdpAdapter:abortReceiveCommandCommand", " " << msgName
               << " socket (" << socket->second << ") closed");
      m_activeSockets.erase(socket); // erase the closed socket
      // Let the exec know that we believe things are cleaned up
      abortCommand(cmd, true);
    }

    //
    // Configuration parsing
    //

    // Parse the message definitions; assign a command handler to each of them.
    bool parseMessageDefinitions(AdapterConfiguration *config)
    {
      if (!getXml().child("Message"))
        return true;  // no messages, hence nothing to do

      CommandHandler *handler = nullptr;
      for (pugi::xml_node const msgXml : getXml().children("Message")) {
        if (!parseMessageDefinition(msgXml))
          return false;
        // Only construct handler when we have parsed at least one valid message def'n
        if (!handler)
          handler = new UdpCommandHandler(this);
        config->registerCommandHandler(handler,
                                       std::string(msgXml.attribute("name").value()));
      }
      return true;
    }

    // Parse one message definition
    bool parseMessageDefinition(pugi::xml_node const msgXml)
    {
      const char* name = msgXml.attribute("name").value();
      if (!name || !*name) {
        warn("UdpAdapter: Message definition missing required 'name' attribute");
        return false;
      }

      pugi::xml_attribute local_port_attr = msgXml.attribute("local_port");
      unsigned int local_port = 0;
      if (local_port_attr) {
        local_port = local_port_attr.as_uint();
        if (local_port == 0 || local_port > 65535) {
          warn("UdpAdapter: Message " << name
               << ": local_port value " << local_port_attr.value()
               << " is out of range or invalid");
          return false;
        }
      }
      else if (m_default_local_port) {
        local_port = m_default_local_port;
        warn("UdpAdapter: Message " << name
             << ": using default local port " << m_default_local_port);
      }
      else {
        warn("UdpAdapter: Message " << name
             << ": no local_port attribute and no default local port specified");
        return false;
      }

      pugi::xml_attribute peer_port_attr = msgXml.attribute("peer_port");
      unsigned int peer_port = 0;
      if (peer_port_attr) {
        peer_port = peer_port_attr.as_uint();
        if (peer_port == 0 || peer_port > 65535) {
          warn("UdpAdapter: Message " << name
               << ": peer_port value " << peer_port_attr.value()
               << " is out of range or invalid");
          return false;
        }
      }
      else if (m_default_peer_port) {
        peer_port = m_default_peer_port;
        warn("UdpAdapter: Message " << name
             << ": using default peer port " << m_default_peer_port);
      }
      else {
        warn("UdpAdapter: Message " << name
             << ": no peer_port attribute and no default peer port specified");
        return false;
      }

      // Construct the message
      UdpMessage msg(name);
      msg.local_port = local_port;
      msg.peer_port = peer_port;
      msg.peer = msgXml.attribute("peer").as_string(m_default_peer.c_str());

      // Walk the <Parameter/> elements of this <Message/>
      for (pugi::xml_node param : msgXml.children("Parameter")) {
        Parameter arg;

        // Get the (required) type
        const char *param_type = param.attribute("type").value();
        if (!param_type || !*param_type) {
          warn("UdpAdapter: Message " << name
               << ": Parameter 'type' attribute missing or empty");
          return false;
        }
        arg.type = param_type;

        // Get the required length in bytes for one value
        pugi::xml_attribute len = param.attribute("bytes");
        if (!len) {
          warn("UdpAdapter: Message " << name << ": Parameter missing required attribute 'bytes'");
          return false;
        }
        arg.len = len.as_uint();
        if (!arg.len) {
          warn("UdpAdapter: Message " << name
               << ": Parameter 'bytes' value " << len.value() << " invalid");
          return false;
        }

        // Check type, and oh BTW bytes value for the type
        if ((arg.type.compare("int") == 0) || (arg.type.compare("int-array") == 0)) {
          if (arg.len != 2 && arg.len != 4) {
            warn("UdpAdapter: Message " << name
                 << ": Invalid 'bytes' value " << arg.len
                 << " for " << arg.type << " parameter;\n valid values are 2 or 4");
            return false;
          }
        }
        else if ((arg.type.compare("float") == 0) || (arg.type.compare("float-array") == 0)) {
          // FIXME - shouldn't these be 4 or 8??
          if (arg.len != 2 && arg.len != 4) {
            warn("UdpAdapter: Message " << name
                 << ": Invalid 'bytes' value " << arg.len
                 << " for " << arg.type << " parameter;\n valid values are 2 or 4");
            return false;
          }
        }
        else if ((arg.type.compare("bool") == 0) || (arg.type.compare("bool-array") == 0)) {
          if (arg.len != 1 && arg.len != 2 && arg.len != 4) {
            warn("UdpAdapter: Message " << name
                 << ": Invalid 'bytes' value " << arg.len
                 << " for " << arg.type << " parameter;\n valid values are 1, 2, or 4");
            return false;
          }
        }
        // what about strings? -- fixed length to start with I suppose...
        else if ((arg.type.compare("string") == 0) || (arg.type.compare("string-array") == 0)) {
          if (arg.len < 1) {
            warn("UdpAdapter: Message " << name << ": " << arg.type
                 << " parameter 'bytes' value must be greater than 0");
            return false;
          }
        }
        else {
          warn("UdpAdapter: Message " << name << ": Invalid parameter type \""
               << arg.type << '"');
          return false;
        }

        // Get the number of elements for the array types
        pugi::xml_attribute param_elements = param.attribute("elements");
        if (arg.type.find("array") != std::string::npos) {
          if (!param_elements) {
            warn("UdpAdapter: Message " << name << ": " << arg.type
                 << " parameter missing required 'elements' attribute");
            return false;
          }
          arg.elements = param_elements.as_uint(0);
          if (!arg.elements) {
            warn("UdpAdapter: Message " << name << ": " << arg.type
                 << " parameter with zero or invalid 'elements' attribute");
            return false;
          }
          msg.len += arg.len * arg.elements;
        }
        else if (param_elements) {
          warn("UdpAdapter: Message " << name << ": " << arg.type <<
               "parameter may not have an 'elements' attribute");
          return false;
        }
        else {
          arg.elements = 1;
        }

        // Get the (optional) description
        pugi::xml_attribute param_desc = param.attribute("desc");
        if (param_desc)
          arg.desc = param_desc.value();

        // Success!
        msg.parameters.push_back(arg);
      }
      m_messages[name] = msg; // record the message with the name as the key
      return true;
    }

    void printMessageDefinitions()
    {
      // print all of the stuff in m_message for debugging
      std::string indent = "             ";
      for (std::pair<std::string, UdpMessage> const &msg : m_messages) {
        std::cout << "UDP Message: " << msg.first;
        for (Parameter const &param : msg.second.parameters) {
          std::string temp = param.desc.empty() ? " (no description)" : " (" + param.desc + ")";
          if (param.elements == 1) {
            std::cout << "\n" << indent << param.len << " byte " << param.type << temp;
          }
          else {
            size_t pos = param.type.find("-");
            std::cout << "\n" << indent << param.elements << " element array of " << param.len << " byte "
                      << param.type.substr(0, pos) << "s" << temp;
          }
        }
        std::cout << std::endl << indent << "length: " << msg.second.len << " (bytes)";
        std::cout << ", peer: " << msg.second.peer << ", peer_port: " << msg.second.peer_port;
        std::cout << ", local_port: " << msg.second.local_port;
        std::cout << std::endl;
      }
    }

    // Start a UDP Message Handler for a node waiting on a UDP message
    int startUdpMessageReceiver(const std::string& name, Command * /* cmd */)
    {
      debugMsg("UdpAdapter:startUdpMessageReceiver",
               " for " << name);
      // Find the message definition to get the message port and size
      MessageMap::iterator msg = m_messages.find(name);
      if (msg == m_messages.end()) {
        warn("UdpAdapter:startUdpMessageReceiver: no message found for " << name);
        return -1;
      }
    
      // Check for a bogus local port
      if (msg->second.local_port == 0) {
        warn("startUdpMessageReceiver: bad local port (0) given for " << name << " message");
        return -1;
      }

      msg->second.name = name;
      msg->second.self = (void*) this; // pass a reference to "this" UdpAdapter for later use
      // Try to set up the socket so that we can close it later if the thread is cancelled
      int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      if (sock < 0) {
        warn("UdpAdapter:startUdpMessageReceiver: call to socket() failed");
        return -1;
      }
    
      debugMsg("UdpAdapter:startUdpMessageReceiver",
               " " << name << " socket (" << sock << ") opened");
      msg->second.sock = sock; // pass the socket descriptor to waitForUdpMessage, which will then reset it
      pthread_t thread_handle;
      // Spawn the listener thread
      threadSpawn((THREAD_FUNC_PTR) waitForUdpMessage, &msg->second, thread_handle);
      // Check to see if the thread got started correctly
      if (thread_handle == 0) {
        warn("UdpAdapter:startUdpMessageReceiver: threadSpawn returned null");
        return -1;
      }

      debugMsg("UdpAdapter:startUdpMessageReceiver",
               " " << name << " listener thread (" << thread_handle << ") spawned");
      // Record the thread and socket in case they have to be cancelled and closed later (in invokeAbort)
      m_activeThreads[name] = thread_handle;
      m_activeSockets[name] = sock;
      return 0;
    }

    static void* waitForUdpMessage(UdpMessage* msg)
    {
      debugMsg("UdpAdapter:waitForUdpMessage", " called for " << msg->name);
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
      if (status) {
        warn("waitForUdpMessage: call to wait_for_input_on_thread returned " << status);
        delete[] params.buffer;     // release the input buffer
        return (void *) 0;
      }

      // When the message has been received, tell the UdpAdapter about it and its contents
      status = udpAdapter->handleUdpMessage(msg, params.buffer, params.debug);
      delete[] params.buffer;     // release the input buffer
      if (status) {
        warn("waitForUdpMessage call to handleUdpMessage returned " << status);
      }

      return (void *) 0;
    }

    int handleUdpMessage(const UdpMessage* msgDef,
                         const unsigned char* buffer,
                         bool debug)
    {
      // Handle a UDP message once it has indeed arrived.
      // msgDef is passed in, therefore, we will assume it is good.
      debugMsg("UdpAdapter:handleUdpMessage", " called for " << msgDef->name);
      if (debug) {
        std::cout << "  handleUdpMessage: buffer: ";
        print_buffer(buffer, msgDef->len);
      }
      // (1) addMessage for expected message
      static int counter = 1;     // gensym counter
      std::ostringstream unique_id;
      unique_id << msgDef->name << ":msg_parameter:" << counter++;
      std::string msg_label(unique_id.str());
      debugMsg("UdpAdapter:handleUdpMessage", " adding \"" << msgDef->name << "\" to the command queue");
      const std::string msg_name = formatMessageName(msgDef->name, RECEIVE_COMMAND_COMMAND);
      m_messageQueues.addMessage(msg_name, msg_label);
      // (2) walk the parameters, and for each, call addMessage(label, <value-or-key>), which
      //     (somehow) arranges for executeCommand(GetParameter) to be called, and which in turn
      //     calls addRecipient and updateQueue
      int i = 0;
      int offset = 0;
      for (std::vector<Parameter>::const_iterator param = msgDef->parameters.begin();
           param != msgDef->parameters.end();
           param++, i++) {
        const std::string param_label = formatMessageName(msg_label, GET_PARAMETER_COMMAND, i);
        int len = param->len;   // number of bytes to read
        int size = param->elements; // size of the array, or 1 for scalars
        std::string type = param->type; // type to decode
        if (debug) {
          if (size == 1) {
            std::cout << "  handleUdpMessage: decoding " << len << " byte " << type
                      << " starting at buffer[" << offset << "]: ";
          }
          else {
            size_t pos = type.find("-"); // remove the "-array" from the type
            std::cout << "  handleUdpMessage: decoding " << size << " element array of " << len
                      << " byte " << type.substr(0, pos) << "s starting at buffer[" << offset
                      << "]: ";
          }
        }
        if (type.compare("int") == 0) {
          if (len != 2 && len != 4){
            warn("handleUdpMessage: Integers must be 2 or 4 bytes, not " << len);
            return -1;
          }
          int num;
          num = (len == 2) ? decode_short_int(buffer, offset) : decode_int32_t(buffer, offset);
          if (debug)
            std::cout << num << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing numeric (integer) parameter " << num);
          m_messageQueues.addMessage(param_label, Value(num));
          offset += len;
        }
        else if (type.compare("int-array") == 0) {
          if (len != 2 && len != 4) {
            warn("handleUdpMessage: Integers must be 2 or 4 bytes, not " << len);
            return -1;
          }
          IntegerArray array(size);
          for (int i = 0 ; i < size ; i++) {
            array.setElement(i, (int32_t) ((len == 2) ? decode_short_int(buffer, offset) : decode_int32_t(buffer, offset)));
            offset += len;
          }
          if (debug)
            std::cout << array.toString() << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing numeric (integer) array " << array.toString());
          m_messageQueues.addMessage(param_label, array);
        }
        else if (type.compare("float") == 0) {
          if (len != 4) {
            warn("handleUdpMessage: Reals must be 4 bytes, not " << len);
            return -1;
          }
          float num = decode_float(buffer, offset);
          if (debug)
            std::cout << num << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing numeric (real) parameter " << num);
          m_messageQueues.addMessage(param_label, Value((double) num));
          offset += len;
        }
        else if (type.compare("float-array") == 0) {
          if (len != 4) {
            warn("handleUdpMessage: Reals must be 4 bytes, not " << len);
            return -1;
          }
          RealArray array(size);
          for (int i = 0 ; i < size ; i++) {
            array.setElement(i, (double) decode_float(buffer, offset));
            offset += len;
          }
          if (debug)
            std::cout << array.toString() << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing numeric (real) array " << array.toString());
          m_messageQueues.addMessage(param_label, Value(array));
        }
        else if (type.compare("bool") == 0) {
          int num;
          switch (len) {
          case 1:
            num = buffer[offset];
            break;

          case 2:
            num = decode_short_int(buffer, offset); break;

          case 4:
            num = decode_int32_t(buffer, offset); break;

          default:
            warn("handleUdpMessage: Booleans must be 1, 2 or 4 bytes, not " << len);
            return -1;
          }
          if (debug)
            std::cout << num << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing numeric (boolean) parameter " << num);
          m_messageQueues.addMessage(param_label, Value(num != 0));
          offset += len;
        }
        else if (type.compare("bool-array") == 0) {
          BooleanArray array(size);
          for (int i = 0 ; i < size ; i++) {
            switch (len) {
            case 1: 
              array.setElement(i, 0 != buffer[offset]); break;
            case 2:
              array.setElement(i, 0 != decode_short_int(buffer, offset)); break;
            case 4:
              array.setElement(i, 0 != decode_int32_t(buffer, offset)); break;
            default:
              warn("handleUdpMessage: Booleans must be 1, 2 or 4 bytes, not " << len);
              return -1;
            }
            offset += len;
          }
          if (debug)
            std::cout << array.toString() << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queueing boolean array " << array.toString());
          m_messageQueues.addMessage(param_label, Value(array));
        }
        else if (type.compare("string-array") == 0) {
          // XXXX For unknown reasons, OnCommand(... String arg); is unable to receive this (inlike int and float arrays)
          StringArray array(size);
          for (int i = 0 ; i < size ; i++) {
            array.setElement(i, decode_string(buffer, offset, len));
            offset += len;
          }
          if (debug)
            std::cout << array.toString() << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queuing string array " << array.toString());
          m_messageQueues.addMessage(param_label, Value(array));
        }
        else { // string or die
          if (type.compare("string")) {
            warn("handleUdpMessage: unknown parameter type " << type);
            return -1;
          }
          std::string str = decode_string(buffer, offset, len);
          if (debug)
            std::cout << str << std::endl;
          debugMsg("UdpAdapter:handleUdpMessage", " queuing string parameter \"" << str << "\"");
          m_messageQueues.addMessage(param_label, Value(str));
          offset += len;
        }
      }
      debugMsg("UdpAdapter:handleUdpMessage", " for " << msgDef->name << " complete");
      return 0;
    }

    int sendUdpMessage(const unsigned char* buffer, const UdpMessage& msg, bool debug)
    {
      int status = 0; // return status
      debugMsg("UdpAdapter:sendUdpMessage", " sending " << msg.len << " bytes to " << msg.peer << ":" << msg.peer_port);
      status = send_message_connect(msg.peer.c_str(), msg.peer_port, (const char*) buffer, msg.len, debug);
      return status;
    }

    int buildUdpBuffer(unsigned char* buffer,
                       const UdpMessage& msg,
                       const std::vector<Value>& args,
                       bool skip_arg,
                       bool debug)
    {
      std::vector<Value>::const_iterator it;
      std::vector<Parameter>::const_iterator param;
      int start_index = 0; // where in the buffer to write

      // Do what error checking we can, since we absolutely know that planners foul this up.
      debugMsg("UdpAdapter:buildUdpBuffer",
               " args.size()==" << args.size()
               << ", parameters.size()==" << msg.parameters.size());
      size_t param_count = msg.parameters.size();
      if (skip_arg)
        param_count++;

      if (args.size() != param_count) {
        warn("the " << param_count
             << " parameters defined in the XML configuration file do not match the "
             << args.size() << " parameters used in the plan for <Message name=\""
             << msg.name << "\"/>");
        return -1;
      }

      // Iterate over the given args (it) and the message definition (param) in lock step to encode the outgoing buffer.
      for (param = msg.parameters.begin(), it=args.begin(); param != msg.parameters.end(); param++, it++) {
        if (skip_arg) { // only skip the first arg
          it++; 
          skip_arg = false; 
        }
        unsigned int len = param->len;
        std::string type = param->type;
        Value const &plexil_val = *it;
        if (!plexil_val.isKnown()) {
          warn("buildUdpBuffer: Value to be sent is unknown");
          return -1;
        }

        ValueType valType = plexil_val.valueType();
      
        if (debug)
          std::cout << "  buildUdpBuffer: encoding ";
        if (type.compare("bool") == 0) {
          if (valType != BOOLEAN_TYPE) {
            warn("buildUdpBuffer: Format requires Boolean, but supplied value is not");
            return -1;
          }
          bool temp;
          plexil_val.getValue(temp);
          if (debug)
            std::cout << len << " byte bool starting at buffer[" << start_index << "]: " << temp;
          switch (len) {
          case 1: 
            buffer[start_index] = (unsigned char) temp;
            break;
          case 2:
            encode_short_int(temp, buffer, start_index);
            break;
          case 4:
            encode_int32_t(temp, buffer, start_index);
            break;
          default:
            warn("buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            return -1;
          }
          start_index += len;
        }
        else if (type.compare("int") == 0) {
          if (valType != INTEGER_TYPE) {
            warn("buildUdpBuffer: Format requires Integer, but supplied value is not");
            return -1;
          }
          if (len != 2 && len != 4) {
            warn("buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            return -1;
          }
          if (debug)
            std::cout << len << " byte int starting at buffer[" << start_index << "]: " << plexil_val;
          Integer temp;
          plexil_val.getValue(temp);
          if (len == 2) {
            if (INT16_MIN > temp || temp > INT16_MAX) {
              warn("buildUdpBuffer: 2 byte integers must be between "
                   << INT16_MIN << " and " << INT16_MAX
                   << ", " << temp << " is not");
              return -1;
            }
            encode_short_int(temp, buffer, start_index);
          }
          else
            encode_int32_t(temp, buffer, start_index);
          start_index += len;
        }
        else if (type.compare("float") == 0) {
          if (valType != REAL_TYPE) {
            warn("buildUdpBuffer: Format requires Real, but supplied value is not");
            return -1;
          }
          if (len != 4) {
            warn("buildUdpBuffer: Reals must be 4 bytes, not " << len);
            return -1;
          }
          double temp;
          plexil_val.getValue(temp);
          // Catch really big floats
          if ((-FLT_MAX) > temp || temp > FLT_MAX) {
            warn("buildUdpBuffer: Reals (floats) must be between "
                 << (-FLT_MAX) << " and " << FLT_MAX <<
                 ", not " << plexil_val);
            return -1;
          }
          if (debug)
            std::cout << len << " byte float starting at buffer[" << start_index << "]: " << temp;
          encode_float((float) temp, buffer, start_index);
          start_index += len;
        }
        else if (type.compare("string") == 0) {
          if (valType != STRING_TYPE) {
            warn("buildUdpBuffer: Format requires String, but supplied value is not");
            return -1;
          }
          std::string const *str = nullptr;
          plexil_val.getValuePointer(str);
          if (str->length() > len) {
            warn("buildUdpBuffer: declared string length (" << len <<
                 ") and actual length (" << str->length() << ", " << *str <<
                 ") used in the plan are not compatible");
            return -1;
          }
          if (debug) 
            std::cout << len << " byte string starting at buffer[" << start_index << "]: " << str;
          encode_string(*str, buffer, start_index);
          start_index += len;
        }
        else if (type.compare("bool-array") == 0) {
          if (valType != BOOLEAN_ARRAY_TYPE) {
            warn("buildUdpBuffer: Format requires BooleanArray, but supplied value is not");
            return -1;
          }
          if (len != 1 && len != 2 && len != 4) {
            warn("buildUdpBuffer: Booleans must be 1, 2 or 4 bytes, not " << len);
            return -1;
          }
          unsigned int size = param->elements;
          BooleanArray const *array = nullptr;
          plexil_val.getValuePointer(array);
          if (debug)
            std::cout << size << " element array of "
                      << len << " byte booleans starting at buffer["
                      << start_index << "]: " << array->toString();
          if (size != array->size()) {
            warn("buildUdpBuffer: declared and actual (boolean) array sizes differ: "
                 << size << " was declared, but "
                 << array->size() << " is being used in the plan");
            return -1;
          }
          for (unsigned int i = 0 ; i < size ; i++) {
            bool temp;
            if (!array->getElement(i, temp)) {
              warn("buildUdpBuffer: Array element at index " << i << " is unknown");
              return -1;
            }
            switch (len) {
            case 1:
              buffer[start_index] = (unsigned char) temp;
              break;
            case 2:
              encode_short_int(temp, buffer, start_index);
              break;
            default:
              encode_int32_t(temp, buffer, start_index);
              break;
            }
            start_index += len;
          }
        }
        else if (type.compare("int-array") == 0) {
          if (plexil_val.valueType() != INTEGER_ARRAY_TYPE) {
            warn("buildUdpBuffer: Format requires IntegerArray, supplied value is a "
                 << valueTypeName(plexil_val.valueType()));
            return -1;
          }
          if (len != 2 && len != 4) {
            warn("buildUdpBuffer: Integers must be 2 or 4 bytes, not " << len);
            return -1;
          }
          unsigned int size = param->elements;
          IntegerArray const *array = nullptr;
          plexil_val.getValuePointer(array);
          if (debug)
            std::cout << size << " element array of " << len << " byte ints starting at ["
                      << start_index << "]: " << array->toString();
          if (size != array->size()) {
            warn("buildUdpBuffer: declared and actual array sizes differ: "
                 << size << " was declared, but "
                 << array->size() << " is being used in the plan");
            return -1;
          }
          for (unsigned int i = 0 ; i < size ; i++) {
            Integer temp;
            if (!array->getElement(i, temp)) {
              warn("buildUdpBuffer: Array element at index " << i << " is unknown");
              return -1;
            }
            if (len == 2) {
              if (INT16_MIN > temp || temp > INT16_MAX) {
                warn("buildUdpBuffer: 2 bytes integers must be between "
                     << INT16_MIN << " and " << INT16_MAX
                     << ", " << temp << " is not");
                return -1;
              }
              encode_short_int(temp, buffer, start_index);
            }
            else
              encode_int32_t(temp, buffer, start_index);
            start_index += len;
          }
        }
        else if (type.compare("float-array") == 0) {
          if (plexil_val.valueType() != REAL_ARRAY_TYPE) {
            warn("buildUdpBuffer: Format requires RealArray, supplied value is a "
                 << valueTypeName(plexil_val.valueType()));
            return -1;
          }
          if (len != 4) {
            warn("buildUdpBuffer: Reals must be 4 bytes, not " << len);
            return -1;
          }
          unsigned int size = param->elements;
          if (size < 1) {
            warn("buildUdpBuffer: all scalars and arrays must be of at least size 1, not " << size);
            return -1;
          }
          RealArray const *array = nullptr;
          plexil_val.getValuePointer(array);
          if (debug)
            std::cout << size << " element array of " << len << " byte floats starting at buffer["
                      << start_index << "]: " << array->toString();
          if (size != array->size()) {
            warn("buildUdpBuffer: declared and actual (float) array sizes differ: "
                 << size << " was declared, but "
                 << array->size() << " is being used in the plan");
            return -1;
          }
          for (unsigned int i = 0 ; i < size ; i++) {
            double temp;
            if (!array->getElement(i, temp)) {
              warn("buildUdpBuffer: Array element at index " << i << " is unknown");
              return -1;
            }
            if ((-FLT_MAX) > temp || temp > FLT_MAX) {
              warn("buildUdpBuffer: Reals (floats) must be between "
                   << (-FLT_MAX) << " and " << FLT_MAX <<
                   ", " << temp << " is not");
              return -1;
            }
            encode_float((float) temp, buffer, start_index);
            start_index += len;
          }
        }
        else if (type.compare("string-array") == 0) {
          if (plexil_val.valueType() != STRING_ARRAY_TYPE) {
            warn("buildUdpBuffer: Format requires StringArray, supplied value is a "
                 << valueTypeName(plexil_val.valueType()));
            return -1;
          }
          unsigned int size = param->elements;
          StringArray const *array = nullptr;
          plexil_val.getValuePointer(array);
          if (debug)
            std::cout << size << " element array of " << len << " byte strings starting at buffer["
                      << start_index << "]: " << array->toString();
          if (size != array->size()) {
            warn("buildUdpBuffer: declared and actual (string) array sizes differ: "
                 << size << " was declared, but " << array->size() << " is being used in the plan");
            return -1;
          }
          for (unsigned int i = 0 ; i < size ; i++) {
            std::string const *temp = nullptr;
            if (!array->getElementPointer(i, temp)) {
              warn("buildUdpBuffer: Array element at index " << i << " is unknown");
              return -1;
            }
            if (temp->length() > len) {
              warn("buildUdpBuffer: declared string length (" << len <<
                   ") and actual length (" << temp->length() <<
                   ") used in the plan are not compatible");
              return -1;
            }
            encode_string(*temp, buffer, start_index);
            start_index += len;
          }
        }
        else {
          warn("buildUdpBuffer: unknown parameter type " << type);
          return -1;
        }

        if (debug)
          std::cout << std::endl;
      }
      if (debug) {
        std::cout << "  buildUdpBuffer: buffer: ";
        print_buffer(buffer, msg.len);
      }
      return start_index;
    }

    void printMessageContent(const std::string& name, const std::vector<Value>& args)
    {
      // Print the content of a message
      std::vector<Value>::const_iterator it;
      std::cout << "Message: " << name << ", Params:";
      for (it=args.begin(); it != args.end(); it++) {
        // Real, Integer, Boolean, String (and Array, maybe...)
        // Integers and Booleans are represented as Real (oops...)
        std::cout << " ";
        if (it->valueType() == STRING_TYPE) { 
          // Extract strings
          std::string const *temp = nullptr;
          if (it->getValuePointer(temp))
            std::cout << "\"" << *temp << "\"";
          else
            std::cout << "UNKNOWN";
        }
        else { // Extract numbers (bool, float, int)
          std::cout << *it;
        }
      }
      std::cout << std::endl;
    }
    
    //
    // Member variables
    //

    std::mutex m_cmdMutex;

    // Somewhere to hang the messages, default ports and peers, threads and sockets
    std::string m_default_peer;
    MessageMap m_messages;
    MessageQueueMap m_messageQueues;
    ThreadMap m_activeThreads;
    SocketMap m_activeSockets;
    unsigned int m_default_local_port;
    unsigned int m_default_peer_port;
    bool m_debug; // Show debugging output

  }; // class UdpAdapter

} // namespace PLEXIL

// Register the UdpAdapter
extern "C"
void initUdpAdapter()
{
  REGISTER_ADAPTER(PLEXIL::UdpAdapter, "UdpAdapter");
}

