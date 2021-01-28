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

#include "plexil-config.h"

#include "AdapterExecInterface.hh"
#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh" // REGISTER_ADAPTER() macro
#include "ArrayImpl.hh"
#include "Command.hh"
#include "CommandHandler.hh"
#include "Debug.hh"
#include "Error.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"
#include "ipc-data-formats.h"
#include "IpcFacade.hh"
#include "LookupHandler.hh"
#include "LookupReceiver.hh"
#include "MessageQueueMap.hh"
#include "NodeConnector.hh"
#include "parsePlan.hh"
#include "PlanError.hh"
#include "PlannerUpdateHandler.hh"
#include "State.hh"
#include "ThreadSemaphore.hh"
#include "Update.hh"

#include "pugixml.hpp"

#include "ipc.h"

#include <algorithm>
#include <atomic>
#include <list>
#include <string>
#include <sstream>

#if defined(HAVE_CSTDLIB)
#include <cstdlib>
#elif defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL 
{

  // Helper functions
  static std::string formatMessageName(const std::string &name,
                                       const std::string &command,
                                       int id = 0);
  static std::string formatMessageName(const char *name,
                                       const std::string &command);

  static Value parseTypedValue(const char *type,
                               pugi::xml_attribute const value);

  static Value parseReturnValue(const std::vector<PlexilMsgBase*>& msgs); 
  
  //
  // String constants
  //

  // Commands implemented directly by IpcAdapter

  static constexpr char const SEND_MESSAGE_COMMAND[] = "SendMessage";
  static constexpr char const RECEIVE_MESSAGE_COMMAND[] = "ReceiveMessage";
  static constexpr char const RECEIVE_COMMAND_COMMAND[] = "ReceiveCommand";
  static constexpr char const GET_PARAMETER_COMMAND[] = "GetParameter";
  static constexpr char const SEND_RETURN_VALUE_COMMAND[] = "SendReturnValue";
  static constexpr char const UPDATE_LOOKUP_COMMAND[] = "UpdateLookup";

  // Constants used internally

  static constexpr char const COMMAND_PREFIX[] = "__COMMAND__";
  // Not used at present
  // static constexpr char const MESSAGE_PREFIX[] = "__MESSAGE__";
  // static constexpr char const LOOKUP_PREFIX[] = "__LOOKUP__";
  // static constexpr char const LOOKUP_ON_CHANGE_PREFIX[] = "__LOOKUP_ON_CHANGE__";
  static constexpr char const PARAM_PREFIX[] = "__PARAMETER__";
  static constexpr char const SERIAL_UID_SEPARATOR[] = ":";
  static constexpr char const TRANSACTION_ID_SEPARATOR_CHAR = ':';

  class IpcAdapter final : public InterfaceAdapter
  {
  public:

    //! Constructor from configuration XML.
    //! @param execInterface Reference to the parent AdapterExecInterface object.
    //! @param xml Const XML element describing this adapter; may be empty.
    IpcAdapter(AdapterExecInterface& execInterface, AdapterConf *conf)
      : InterfaceAdapter(execInterface, conf),
        m_ipcFacade(),
        m_messageQueues(execInterface),
        m_externalLookupNames(),
        m_externalLookups(),
        m_listener(*this),
        m_cmdMutex(),
        m_lookupSem(),
        m_lookupMutex(),
        m_pendingLookupResult(),
        m_pendingLookupState(),
        m_pendingLookupSerial(0)
    {
      debugMsg("IpcAdapter:IpcAdapter", " constructor");
    }

    //! Destructor.
    virtual ~IpcAdapter()
    {
    }

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize(AdapterConfiguration *config) override
    {
      debugMsg("IpcAdapter:initialize", " called");

      // Construct and register handlers for our own commands
      config->registerCommandHandler(new SendMessageCommandHandler(this),
                                     SEND_MESSAGE_COMMAND);
      config->registerCommandHandler(new ReceiveMessageCommandHandler(this),
                                     RECEIVE_MESSAGE_COMMAND);
      config->registerCommandHandler(new ReceiveCommandCommandHandler(this),
                                     RECEIVE_COMMAND_COMMAND);
      config->registerCommandHandler(new GetParameterCommandHandler(this),
                                     GET_PARAMETER_COMMAND);
      config->registerCommandHandler(new SendReturnValueCommandHandler(this),
                                     SEND_RETURN_VALUE_COMMAND);
      config->registerCommandHandler(new UpdateLookupCommandHandler(this),
                                     UPDATE_LOOKUP_COMMAND);

      // Register default command handler as necessary
      if (configuration().defaultCommandAdapter) {
        config->setDefaultCommandHandler(new IpcCommandHandler(this));
      }
      else if (!configuration().commandNames.empty()) {
        config->registerCommandHandler(new IpcCommandHandler(this),
                                       configuration().commandNames);
      }

      // Register lookup handlers as required
      parseExternalLookups(getXml().child("ExternalLookups"));
      if (configuration().defaultLookupAdapter) {
        config->setDefaultLookupHandler(new IpcLookupHandler(this));
      }
      else if (!configuration().lookupNames.empty()
               || !m_externalLookupNames.empty()) {
        LookupHandler *hndlr = new IpcLookupHandler(this);
        config->registerLookupHandler(hndlr,
                                      configuration().lookupNames);
        config->registerLookupHandler(hndlr, m_externalLookupNames);
      }

      // Planner updates
      if (configuration().plannerUpdateAdapter) {
        config->registerPlannerUpdateHandler([this](Update *u, AdapterExecInterface *i) -> void
                                             { this->sendPlannerUpdate(u, i); });
      }

      // Parse other configuration parameters
      bool acceptDuplicates = getXml().attribute("AllowDuplicateMessages").as_bool();
      m_messageQueues.setAllowDuplicateMessages(acceptDuplicates);

      debugMsg("IpcAdapter:initialize", " succeeded");
      return true;
    }

    //! Starts the adapter, possibly using its configuration data.
    //! @return true if successful, false otherwise.
    virtual bool start() override
    {
      // Get taskName, serverName from XML, if supplied
      const char* taskName = "";
      const char* serverName = "";
      pugi::xml_attribute acceptDuplicates;

      pugi::xml_node const xml = this->getXml();
      if (xml) {
        taskName = xml.attribute("TaskName").value();
        serverName = xml.attribute("Server").value();
      }

      // Use defaults if necessary
      if (!*serverName) {
        serverName = "localhost";
      }
      if (!*taskName) {
        taskName = m_ipcFacade.getUID().c_str();
      }

      debugMsg("IpcAdapter:initialize",
               " Connecting module " << taskName <<
               " to central server at " << serverName);

      // init IPC
      if (m_ipcFacade.initialize(taskName, serverName) != IPC_OK) {
        warn("IpcAdapter: Unable to connect to the central server at " << serverName);
        return false;
      }

      // Spawn listener thread
      if (m_ipcFacade.start() != IPC_OK) {
        warn("IpcAdapter: Unable to spawn IPC dispatch thread");
        return false;
      }
      debugMsg("IpcAdapter:start", " spawned IPC dispatch thread");

      m_ipcFacade.subscribeAll(&m_listener);
      debugMsg("IpcAdapter:start", " succeeded");
      return true;
    }

    /**
     * @brief Stops the adapter.
     */
    virtual void stop() override
    {
      m_ipcFacade.stop();
      debugMsg("IpcAdapter:stop", " complete");
    }

    //
    // Message handlers
    //

    //! Called from the IPC thread whenever messages are received.
    void receiveMessages(const std::vector<PlexilMsgBase*>& msgs)
    {
      PlexilMsgBase *leader = msgs.front();
      PlexilMsgType msgType = (PlexilMsgType) leader->msgType;

      debugMsg("IpcAdapter:receiveMessages",
               ' ' << m_ipcFacade.getUID() << " received " << msgs.size()
               << " messages, leader type = " << msgType);

      switch (msgType) {

        // NotifyExec is a PlexilMsgBase
      case PlexilMsgType_NotifyExec:
        // Simply notify the exec
        getInterface().notifyOfExternalEvent();
        break;

        // PlexilStringValueMsg containing the name of a plan file
      case PlexilMsgType_AddLibraryFile:
      case PlexilMsgType_AddPlanFile:
        handlePlanFileMessage(reinterpret_cast<const PlexilStringValueMsg *>(leader));
        break;

        // PlexilStringValueMsg containing the XML text of a plan
      case PlexilMsgType_AddLibrary:
      case PlexilMsgType_AddPlan:
        handlePlanTextMessage(reinterpret_cast<const PlexilStringValueMsg *>(leader));
        break;

        // Command is a PlexilStringValueMsg, optionally followed by parameters
      case PlexilMsgType_Command:
        debugMsg("IpcAdapter:receiveMessages", " processing as command");
        handleCommandSequence(msgs);
        break;

        // Message is a PlexilStringValueMsg
        // No parameters
      case PlexilMsgType_Message:
        debugMsg("IpcAdapter:receiveMessages", " processing as message");
        handleMessageMessage(reinterpret_cast<const PlexilStringValueMsg *>(leader));
        break;

        // Not implemented yet
      case PlexilMsgType_PlannerUpdate:
        break;

        // TelemetryValues is a PlexilStringValueMsg
        // Followed by 0 (?) or more values
      case PlexilMsgType_TelemetryValues:
        handleTelemetryValuesSequence(msgs);
        break;

        // ReturnValues is a PlexilReturnValuesMsg
        // Followed by 0 (?) or more values
      case PlexilMsgType_ReturnValues:
        // Only pay attention to our return values
        debugMsg("IpcAdapter:receiveMessages", " processing as return value");
        handleReturnValuesSequence(msgs);
        break;

      case PlexilMsgType_LookupNow:
        handleLookupNowMessages(msgs);
        break;

        //unhandled so far
      default:
        debugMsg("receiveMessages",
                 "Unhandled leader message type " << leader->msgType << ". Disregarding");
      }
    }

    //
    // Command handling
    //

  private:

    //
    // Handler common logic
    //
    
    struct IpcCommandHandler : public CommandHandler
    {
      IpcCommandHandler(IpcAdapter *adapter)
        : m_adapter(adapter)
      {}
      virtual ~IpcCommandHandler() = default;

      virtual void abortCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->acknowledgeCommandAbort(cmd, intf);
      }

      // Default command execution method.
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeDefaultCommand(cmd, intf);
      }

    protected:
      IpcAdapter *m_adapter;
    };

    // Specialized handlers for commands to the adapter itself
    struct SendMessageCommandHandler : public IpcCommandHandler
    {
      SendMessageCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~SendMessageCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeSendMessageCommand(cmd, intf);
      }
    };
    
    struct SendReturnValueCommandHandler : public IpcCommandHandler
    {
      SendReturnValueCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~SendReturnValueCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeSendReturnValueCommand(cmd, intf);
      }
    };
    
    struct ReceiveMessageCommandHandler : public IpcCommandHandler
    {
      ReceiveMessageCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~ReceiveMessageCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeReceiveMessageCommand(cmd, intf);
      }
    };
    
    struct ReceiveCommandCommandHandler : public IpcCommandHandler
    {
      ReceiveCommandCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~ReceiveCommandCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeReceiveCommandCommand(cmd, intf);
      }
    };
    
    struct GetParameterCommandHandler : public IpcCommandHandler
    {
      GetParameterCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~GetParameterCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeGetParameterCommand(cmd, intf);
      }
    };
    
    struct UpdateLookupCommandHandler : public IpcCommandHandler
    {
      UpdateLookupCommandHandler(IpcAdapter *adpt) : IpcCommandHandler(adpt) {}
      virtual ~UpdateLookupCommandHandler() = default;
      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        m_adapter->executeUpdateLookupCommand(cmd, intf);
      }
    };

  public:

    //
    // Command implementations
    //

    void executeSendMessageCommand(Command *command, AdapterExecInterface *intf)
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      assertTrueMsg(args.size() == 1,
                    "IpcAdapter: The SendMessage command requires exactly one argument");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The argument to the SendMessage command, "
                    << args.front() << ", is not a valid String value");
      std::string const *theMessage;
      args.front().getValuePointer(theMessage);
      debugMsg("IpcAdapter:executeCommand",
               " SendMessage(\"" << *theMessage << "\")");
      checkInterfaceError(m_ipcFacade.publishMessage(*theMessage) != IpcFacade::ERROR_SERIAL,
                          "Message publish failed");
      // store ack
      intf->handleCommandAck(command, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " message \"" << *theMessage << "\" sent.");
    }

    /**
     * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
     */
    void executeSendReturnValueCommand(Command *command, AdapterExecInterface *intf)
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for two arguments, the message and the value
      assertTrueMsg(args.size() == 2,
                    "IpcAdapter: SendReturnValue command requires exactly two arguments"
                    << "\n but was given " << args.size());
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The message name argument to the SendReturnValue command, "
                    << args.front() << ", is not a valid String value");
      const std::string *front;
      args.front().getValuePointer(front);
      uint32_t serial;
      //grab serial from parameter
      std::string::size_type sep_pos = front->find(SERIAL_UID_SEPARATOR, 1);
      assertTrueMsg(sep_pos != std::string::npos, "Could not find UID seperator in first parameter of return value");
      std::string const serial_string = front->substr(0, sep_pos);
      serial = atoi(serial_string.c_str());
      std::string front_string = front->substr(sep_pos + 1, front->size());
      debugMsg("IpcAdapter:executeCommand",
               " SendReturnValue(sender_serial:\"" << serial << "\" \"" << front_string << "\")");
      //publish
      serial = m_ipcFacade.publishReturnValues(serial, front_string, args[1]);
      checkInterfaceError(serial != IpcFacade::ERROR_SERIAL,
                          "Return values failed to be sent");

      // store ack
      intf->handleCommandAck(command, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " return value sent.");
    }

    /**
     * @brief handles RECEIVE_MESSAGE_COMMAND commands from the exec
     */
    void executeReceiveMessageCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      assertTrueMsg(args.size() == 1,
                    "IpcAdapter: The ReceiveMessage command requires exactly one argument");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The argument to the SendMessage command, "
                    << args.front()
                    << ", is not a valid String value");
      std::string const *theMessage;
      args.front().getValuePointer(theMessage);
      m_messageQueues.addRecipient(*theMessage, command);
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:receiveMessageCommand", ' ' << m_ipcFacade.getUID()
               << " message handler for \"" << *theMessage << "\" registered.");
    }

    /**
     * @brief handles RECEIVE_COMMAND_COMMAND commands from the exec
     */
    void executeReceiveCommandCommand(Command *command, AdapterExecInterface *intf)
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      checkPlanError(args.size() == 1,
                     "IpcAdapter: The " << RECEIVE_COMMAND_COMMAND
                     << " command requires exactly one argument");
      checkPlanError(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The argument to the " << RECEIVE_COMMAND_COMMAND
                    << " command, " << args.front()
                    << ", is not a string");

      std::string const *cmdName = nullptr;
      args.front().getValuePointer(cmdName);
      debugMsg("IpcAdapter:receiveCommand", ' ' << m_ipcFacade.getUID()
               << " for \"" << *cmdName << '"');

      std::string msgName(formatMessageName(*cmdName, RECEIVE_COMMAND_COMMAND));
      m_messageQueues.addRecipient(msgName, command);
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:receiveCommand", ' ' << m_ipcFacade.getUID()
               << " command handler for \"" << msgName << "\" registered.");
    }

    /**
     * @brief handles GET_PARAMETER_COMMAND commands from the exec
     */
    void executeGetParameterCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      checkPlanError(args.size() == 1 || args.size() == 2,
                    "IpcAdapter: The " << GET_PARAMETER_COMMAND
                     << " command requires either one or two arguments");
      checkPlanError(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The first argument to the " << GET_PARAMETER_COMMAND
                     << " command, " << args.front() << ", is not a string");
      int32_t id;
      if (args.size() == 1)
        // Default to parameter 0
        id = 0;
      else {
        assertTrueMsg(args[1].isKnown() && args[1].valueType() == INTEGER_TYPE,
                      "IpcAdapter: The second argument to the "
                      << GET_PARAMETER_COMMAND << " command, " << args[1]
                      << ", is not an Integer");
        args[1].getValue(id);
        assertTrueMsg(id >= 0,
                      "IpcAdapter: The second argument to the "
                      << GET_PARAMETER_COMMAND << " command, " << args[1]
                      << ", is not a valid index");
      }
      std::string const *cmdName = nullptr;
      args.front().getValuePointer(cmdName);
      debugMsg("IpcAdapter:getParameter", ' ' << m_ipcFacade.getUID()
               << " for \"" << *cmdName << "\" parameter # " << id);

      std::string msgName(formatMessageName(*cmdName, GET_PARAMETER_COMMAND, id));
      m_messageQueues.addRecipient(msgName, command);
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:getParameter",
               " message handler for \"" << msgName << "\" registered.");
    }

    void executeUpdateLookupCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::vector<Value> const &args = command->getArgValues();
      size_t nargs = args.size();
      assertTrueMsg(nargs >= 2,
                    "IpcAdapter: The " << UPDATE_LOOKUP_COMMAND
                    << " command requires at least two arguments");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The argument to the " << UPDATE_LOOKUP_COMMAND
                    << " command, " << args.front() << ", is not a string");

      // Extract state from command parameters
      std::string const *lookupName;
      args.front().getValuePointer(lookupName);

      // Warn, but do not crash, if not declared
      if (std::find(m_externalLookupNames.begin(),
                    m_externalLookupNames.end(),
                    *lookupName)
          == m_externalLookupNames.end()) {
        warn("IpcAdapter: The external lookup " << *lookupName
             << " is not declared. Ignoring UpdateLookup command.");
        intf->handleCommandAck(command, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }
      
      State state(*lookupName, nargs - 2);
      for (size_t i = 2; i < nargs; ++i)
        state.setParameter(i - 2, args[i]);

      {
        std::lock_guard<std::mutex> guard(m_cmdMutex);
        //Set value internally
        m_externalLookups[state] = args[1];
      }
      // Notify internal users, if any
      intf->handleValueChange(state, args[1]);

      //send telemetry
      std::vector<Value> result_and_args(nargs - 1);
      for (size_t i = 1; i < nargs; ++i)
        result_and_args[i - 1] = args[i];
      uint32_t publishStatus = m_ipcFacade.publishTelemetry(*lookupName, result_and_args);
      if (publishStatus == IpcFacade::ERROR_SERIAL) {
        warn("IpcAdapter: publishTelemetry returned status \"" << m_ipcFacade.getError() << "\"");
        intf->handleCommandAck(command, COMMAND_FAILED);
      }
      else {
        // Notify of success
        intf->handleCommandAck(command, COMMAND_SUCCESS);
      }
      intf->notifyOfExternalEvent();
    }

    /**
     * @brief handles all other commands from the exec
     */
    void executeDefaultCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::string const &name = command->getName();
      std::vector<Value> const &args = command->getArgValues();
      debugMsg("IpcAdapter:executeCommand", ' ' << m_ipcFacade.getUID()
               << " for \"" << name << "\", " << args.size() << " parameters");
      if (!args.empty())
        debugMsg("IpcAdapter:executeCommand", " first parameter is \""
                 << args.front()
                 << "\"");

      uint32_t serial;
      size_t sep_pos = name.find_first_of(TRANSACTION_ID_SEPARATOR_CHAR);
      // decide to direct or publish command
      if (sep_pos != std::string::npos) {
        debugMsg("IpcAdapter:executeCommand", ' ' << m_ipcFacade.getUID()
                 << " sending \"" << name.substr(sep_pos + 1)
                 << "\" to \"" << name.substr(0, sep_pos) << '"');
        serial = m_ipcFacade.sendCommand(name.substr(sep_pos + 1),
                                         name.substr(0, sep_pos),
                                         args);
      }
      else {
        debugMsg("IpcAdapter:executeCommand", ' ' << m_ipcFacade.getUID()
                 << " publishing \"" << name << '"');
        serial = m_ipcFacade.publishCommand(name, args);
      }
      assertTrueMsg(serial != IpcFacade::ERROR_SERIAL,
                    "executeCommand: IPC Error, IPC_errno = " << m_ipcFacade.getError());
      // log ack and return variables in case we get values for them
      {
        std::lock_guard<std::mutex> guard(m_cmdMutex);
        m_pendingCommands[serial] = command;
      }
      // store ack
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " command \"" << name << "\" sent.");
    }

    //! Acknowledge an abortCommand() call
    void acknowledgeCommandAbort(Command *command, AdapterExecInterface *intf)
    {
      intf->handleCommandAbortAck(command, true);
      intf->notifyOfExternalEvent();
    }

    //
    // Lookup handling
    //

    class IpcLookupHandler final : public LookupHandler
    {
    public:
      IpcLookupHandler(IpcAdapter *adapter)
        : m_adapter(adapter)
      {
      }

      virtual ~IpcLookupHandler()
      {
      }

      virtual void lookupNow(const State &state, LookupReceiver *rcvr) override
      {
        m_adapter->lookupNow(state, rcvr);
      }

      // setThresholds(), clearThresholds() not implemented

    private:

      IpcAdapter *m_adapter;
    };

    //
    // Lookup implementation 
    //

    void lookupNow(const State &state, LookupReceiver *rcvr)
    {
      // Check whether this is a state we publish to the world
      {
        std::lock_guard<std::mutex> g(m_lookupMutex);
        ExternalLookupMap::iterator it = m_externalLookups.find(state);
        if (it != m_externalLookups.end()) {
          debugMsg("IpcAdapter:lookupNow",
                   " returning external lookup " << state
                   << " with internal value " << it->second);
          rcvr->update(it->second);
          return;
        }
      }

      const std::string& stateName = state.name();
      const std::vector<Value>& params = state.parameters();
      size_t nParams = params.size();
      debugMsg("IpcAdapter:lookupNow",
               " for state " << stateName
               << " with " << nParams << " parameters");

      // Send lookup message
      size_t sep_pos = stateName.find_first_of(TRANSACTION_ID_SEPARATOR_CHAR);
      //decide to direct or publish lookup
      {
        std::lock_guard<std::mutex> g(m_lookupMutex);
        m_pendingLookupResult.setUnknown();
        m_pendingLookupState = state;
        if (sep_pos != std::string::npos) {
          // Direct query
          std::string const dest(stateName.substr(0, sep_pos));
          std::string const sentStateName = stateName.substr(sep_pos + 1);
          m_pendingLookupState.setName(sentStateName);
          m_pendingLookupSerial =
            m_ipcFacade.sendLookupNow(sentStateName, dest, params);
        }
        else {
          // Publish and see if anyone responds
          m_pendingLookupSerial = m_ipcFacade.publishLookupNow(stateName, params);
        }
      }

      // Wait for results
      // N.B. shouldn't have to worry about signals causing wait to be interrupted -
      // ExecApplication blocks most of the common ones
      int errnum = m_lookupSem.wait();
      assertTrueMsg(errnum == 0,
                    "lookupNow: semaphore wait failed, result = " << errnum);

      std::lock_guard<std::mutex> g(m_lookupMutex);
      rcvr->update(m_pendingLookupResult);

      // Clean up
      m_pendingLookupSerial = 0;
      m_pendingLookupState = State();
      m_pendingLookupResult.setUnknown();
    }

    /**
     * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
     * @param update The Update object.
     * @note Called from the Exec thread.
     */

    void sendPlannerUpdate(Update* update, AdapterExecInterface *intf)
    {
      std::string const& name = update->getSource()->getNodeId();
      debugMsg("IpcAdapter:sendPlannerUpdate", " for \"" << name << "\"");
      std::vector<std::pair<std::string, Value> > args(update->getPairs().begin(),
                                                       update->getPairs().end());
      if(args.empty()) {
        debugMsg("IpcAdapter:sendPlannerUpdate", "Emtpy update");
        return;
      }

      uint32_t serial = m_ipcFacade.publishUpdate(name, args);
      checkInterfaceError(serial != IpcFacade::ERROR_SERIAL,
                          "sendPlannerUpdate: IPC Error, IPC_errno = " <<
                          m_ipcFacade.getError());
      intf->handleUpdateAck(update, true);
      intf->notifyOfExternalEvent();
    }

  private:

    //
    // Implementation details
    //

    //! Deal with AddPlanFile, AddLibraryFile messages
    //! @param stringMsg Pointer to const message.
    void handlePlanFileMessage(const PlexilStringValueMsg *stringMsg)
    {
      assertTrue_2(stringMsg->stringValue,
                   "handlePlanFileMessage: stringValue is null");
      std::unique_ptr<pugi::xml_document> doc;
      try {
        doc.reset(loadXmlFile(stringMsg->stringValue));
      } catch (const ParserException& e) {
        std::cerr << "Error parsing plan from file: \n" << e.what() << std::endl;
        return;
      }
      if (!doc) {
        std::cerr << "Error parsing plan from file: file "
                  << stringMsg->stringValue << " not found" << std::endl;
        return;
      }
      if (stringMsg->header.msgType == PlexilMsgType_AddPlan) {
        getInterface().handleAddPlan(doc->document_element());
        // Always notify immediately when adding a plan
        getInterface().notifyOfExternalEvent();
      }
      else {
        // No notification required
        getInterface().handleAddLibrary(doc.release());
      }
    }

    //! Deal with AddPlan, AddLibrary messages
    //! @param stringMsg Pointer to const message.
    void handlePlanTextMessage(const PlexilStringValueMsg *stringMsg)
    {
      assertTrue_2(stringMsg->stringValue,
                   "handlePlanTextMessage: stringValue is null");
      std::unique_ptr<pugi::xml_document> doc(new pugi::xml_document());
      pugi::xml_parse_result result = doc->load_string(stringMsg->stringValue);
      if (result.status != pugi::status_ok) {
        std::cerr << "Error parsing plan XML:\n" << result.description() << std::endl;
        return;
      }
      if (stringMsg->header.msgType == PlexilMsgType_AddPlan) {
        try {
          getInterface().handleAddPlan(doc->document_element());
          // Always notify immediately when adding a plan
          getInterface().notifyOfExternalEvent();
        } catch (ParserException const &e) {
          std::cerr << "IpcAdapter: Error loading plan:\n " << e.what() << std::endl;
        }
      }
      else {
        // No notification required
        getInterface().handleAddLibrary(doc.release());
      }
    }

    /**
     * @brief Process a PlexilMsgType_Message message
     */
    void handleMessageMessage(const PlexilStringValueMsg* msgData)
    {
      assertTrueMsg(msgData,
                    "handleMessageMessage: msgData is null");
      assertTrueMsg(msgData->stringValue,
                    "handleMessageMessage: stringValue is null");
      m_messageQueues.addMessage(msgData->stringValue);
    }

    //! Handle a received command.
    void handleCommandSequence(const std::vector<PlexilMsgBase*> &msgs)
    {
      assertTrue_2(!msgs.empty(), "IpcAdapter::handleCommandSequence: empty message sequence");
      assertTrue_2(msgs.front(), "IpcAdapter::handleCommandSequence: leader is null");

      debugMsg("IpcAdapter:handleCommandSequence", ' ' << m_ipcFacade.getUID()
               << ' ' << msgs.size() << " messages");

      debugMsg("IpcAdapter:handleCommandSequence", ' ' << m_ipcFacade.getUID()
               << " leader: count " << msgs.front()->count
               << ", serial " << msgs.front()->serial
               << ", sender " << msgs.front()->senderUID);

      const PlexilStringValueMsg* header =
        reinterpret_cast<const PlexilStringValueMsg*>(msgs.front());
      assertTrue_2(header->stringValue,
                   "IpcAdapter::handleCommandSequence: stringValue of leader is null");
      debugMsg("IpcAdapter:handleCommandSequence",
               ' ' << m_ipcFacade.getUID() <<
               " adding \"" << header->stringValue << "\" to the command queue");

      // Construct the command identifier
      std::ostringstream uid;
      uid << ((int) header->header.serial) << SERIAL_UID_SEPARATOR << header->header.senderUID;
      std::string uid_lbl(uid.str());
      m_messageQueues.addMessage(formatMessageName(header->stringValue,
                                                   RECEIVE_COMMAND_COMMAND),
                                 uid_lbl);
      for (int i = 1; i < msgs.size(); ++i) {
        m_messageQueues.addMessage(formatMessageName(uid_lbl, GET_PARAMETER_COMMAND, i - 1),
                                   getPlexilMsgValue(msgs.at(i)));
      }
    }

    //! Process a TelemetryValues message sequence
    void handleTelemetryValuesSequence(const std::vector<PlexilMsgBase*>& msgs) 
    {
      const PlexilStringValueMsg* tv =
        reinterpret_cast<const PlexilStringValueMsg*>(msgs[0]);
      assertTrue_2(tv->stringValue,
                   "handleTelemetryValuesSequence: stringValue of leader is null");
      char const *stateName = tv->stringValue;
      if (*stateName) {
        warn("IpcAdapter: state name missing or empty, ignoring");
        return;
      }

      size_t nValues = msgs[0]->count;
      if (!nValues) {
        warn("IpcAdapter: Telemetry message for "
             << stateName << " has value count of 0, ignoring");
        return;
      }

      if (nValues + 1 != msgs.size()) {
        warn("IpcAdapter: Telemetry message for " << stateName
             << " Expected " << nValues + 1 << " messages, got " << msgs.size());
        return;
      }

      // Result is next in sequence
      Value result = getPlexilMsgValue(msgs[1]);

      // Extract state parameters from trailing messages
      State state(stateName, nValues - 1);
      for (size_t i = 2; i <= nValues; ++i)
        state.setParameter(i - 2, getPlexilMsgValue(msgs[i]));
    
      debugMsg("IpcAdapter:handleTelemetryValuesSequence",
               " state \"" << stateName << "\" found, processing");

      // Check to see if a LookupNow is waiting on this value
      {
        std::lock_guard<std::mutex> g(m_lookupMutex);
        if (m_pendingLookupState == state) {
          m_pendingLookupResult = result;
          m_lookupSem.post();
          return;
        }
      }
      // Otherwise process normally
      getInterface().handleValueChange(state, result);
      getInterface().notifyOfExternalEvent();
    }

    //! Process a ReturnValues message sequence
    //! @param msgs Const reference to vector of message pointers.
    void handleReturnValuesSequence(const std::vector<PlexilMsgBase*>& msgs) 
    {
      const PlexilReturnValuesMsg* rv = (const PlexilReturnValuesMsg*) msgs[0];
      //lock mutex to ensure all sending procedures are complete.
      if (rv->requestSerial == m_pendingLookupSerial) {
        // LookupNow for which we are awaiting data
        std::lock_guard<std::mutex> guard(m_lookupMutex);
        debugMsg("IpcAdapter:handleReturnValuesSequence",
                 " processing value(s) for a pending LookupNow");
        m_pendingLookupResult = parseReturnValue(msgs);
        // *** TODO: check for error
        m_lookupSem.post();
        return;
      }

      Command *cmd = nullptr;
      {
        std::lock_guard<std::mutex> guard(m_cmdMutex);
        PendingCommandsMap::iterator cit = m_pendingCommands.find(rv->requestSerial);
        if (cit == m_pendingCommands.end()) {
          warn("IpcAdapter: no lookup or command found for return values sequence");
          return;
        }

        cmd = cit->second;
        assertTrue_1(cmd);
      }

      size_t nValues = msgs[0]->count;
      if (msgs[1]->count == MSG_COUNT_CMD_ACK) {
        assertTrueMsg(nValues == 1,
                      "handleReturnValuesSequence: command ack requires 1 value, received "
                      << nValues);
        debugMsg("IpcAdapter:handleReturnValuesSequence",
                 " processing command acknowledgment for command " << cmd->getName());
        Value ack = parseReturnValue(msgs);
        assertTrue_2(ack.isKnown() && ack.valueType() == COMMAND_HANDLE_TYPE,
                     "IpcAdapter:handleReturnValuesSequence received a command acknowledgment which is not a CommandHandle value");
        CommandHandleValue handle;
        ack.getValue(handle);
        getInterface().handleCommandAck(cmd, handle);
        getInterface().notifyOfExternalEvent();
      }
      else {
        debugMsg("IpcAdapter:handleReturnValuesSequence",
                 " processing command return value for command " << cmd->getName());
        getInterface().handleCommandReturn(cmd, parseReturnValue(msgs));
        getInterface().notifyOfExternalEvent();
      }
    }
  
    //! Process an external LookupNow request. Ignores lookups that
    //! are not defined in config or published.
    //! @param Const reference to a vector of message pointers.
    //! @note Executed in IPC handler thread.
    void handleLookupNowMessages(const std::vector<PlexilMsgBase*>& msgs) 
    {
      debugMsg("IpcAdapter:handleLookupNowMessages", " received LookupNow");
      const PlexilStringValueMsg* msg =
        reinterpret_cast<const PlexilStringValueMsg*> (msgs.front());
      std::string name(msg->stringValue);
      size_t nParms = msgs.front()->count;
      State lookup(name, nParms);
      for (size_t i = 1 ; i <= nParms ; ++i)
        lookup.setParameter(i - 1, getPlexilMsgValue(msgs[i]));

      Value const *resultPtr = nullptr;
      ExternalLookupMap::const_iterator it;
      {
        std::lock_guard<std::mutex> guard(m_lookupMutex);      
        it = m_externalLookups.find(lookup);
        if (it != m_externalLookups.end())
          resultPtr = &it->second;
      }

      if (resultPtr) {
        debugMsg("IpcAdapter:handleLookupNowMessages",
                 " Publishing value of external lookup \"" << lookup
                 << "\" with internal value '" << *resultPtr << "'");
        m_ipcFacade.publishReturnValues(msg->header.serial, msg->header.senderUID, *resultPtr);
        return;
      }

      if (nParms > 0) {
        // See if name is declared, and return default value if so
        lookup.setParameterCount(0);
        {
          std::lock_guard<std::mutex> guard(m_lookupMutex);      
          it = m_externalLookups.find(lookup);
          if (it != m_externalLookups.end())
            resultPtr = &it->second;
        }
        if (resultPtr) {
          debugMsg("IpcAdapter:handleLookupNowMessages",
                   " Publishing default value of external lookup \"" << name
                   << "\" = '" << (*it).second << "'");
          m_ipcFacade.publishReturnValues(msg->header.serial, msg->header.senderUID, *resultPtr);
          return;
        } 
      }

      // Ignore silently if we're not declared to handle it
      // Tracing here for debugging purposes
      debugMsg("IpcAdapter:handleLookupNow",
               " undeclared external lookup \"" << name << "\", ignoring");
    }

    //
    // Helper class for use with IpcFacade
    //

    class MessageListener : public IpcMessageListener
    {
    public:
      MessageListener(IpcAdapter &adapter)
        : m_adapter(adapter)
      {
      }

      virtual ~MessageListener() = default;

      void ReceiveMessage(const std::vector<PlexilMsgBase*>& msgs)
      {
        m_adapter.receiveMessages(msgs);
      }

    private:

      IpcAdapter& m_adapter;
    };

    //
    // Configuration parsing
    //

    //! Parse the external lookup declarations for this adapter.
    void parseExternalLookups(pugi::xml_node const external) 
    {
      if (!external)
        return;

      pugi::xml_node lookup = external.child("Lookup");
      while (lookup) {
        parseExternalLookup(lookup); 
        lookup = lookup.next_sibling("Lookup");
      }
    }

    //! Parse one external lookup declaration.
    void parseExternalLookup(pugi::xml_node const lookup) 
    {
      const pugi::xml_attribute nameAttr = lookup.attribute("name");
      assertTrueMsg(!nameAttr.empty() && nameAttr.value(),
                    "IpcAdapter: Lookup required attribute 'name' missing or empty");
      const pugi::xml_attribute typeAttr = lookup.attribute("type");
      assertTrueMsg(!typeAttr.empty() && typeAttr.value(),
                    "IpcAdapter: Lookup required attribute 'type' missing or empty");
      const pugi::xml_attribute defAttr = lookup.attribute("value");
      assertTrueMsg(!defAttr.empty() && defAttr.value(),
                    "IpcAdapter: Lookup required attribute 'value' missing or empty");

      std::string const nameString(nameAttr.value());
      debugMsg("IpcAdapter:parseExternalLookups",
               "External Lookup: state=\"" << nameString
               << "\" type=\"" << typeAttr.value()
               << "\" default=\"" << defAttr.value() << "\"");

      const char *type = typeAttr.value();
      // TODO: validity check of type
      if (std::find(m_externalLookupNames.begin(),
                    m_externalLookupNames.end(),
                    nameString)
          != m_externalLookupNames.end()) {
        warn("IpcAdapter: Lookup name \"" << nameString
             << "\" multiply declared, using first declaration");
      }
      else {
        m_externalLookupNames.push_back(nameString);
        m_externalLookups[State(nameString)] = parseTypedValue(type, defAttr);
      }
    }

    //
    // Private data types
    //

    //* brief Cache of message/command/lookup names we're actively listening for
    using ActiveListenerMap = std::map<std::string, State>;

    //* brief Cache of command serials and their corresponding ack and return value variables
    using PendingCommandsMap = std::map<uint32_t, Command *>;

    using ExternalLookupMap = std::map<State, Value>;
    
    //
    // Member variables
    //

    //* @brief Interface with IPC
    IpcFacade m_ipcFacade;

    //* @brief Map of queues for holding complete messages and message handlers while they wait to be paired
    MessageQueueMap m_messageQueues;

    //* @brief Cache of ack and return value variables for commands we sent
    PendingCommandsMap m_pendingCommands;

    //* @brief Stores names of declared external lookups
    std::vector<std::string> m_externalLookupNames;

    //* @brief Map of external lookup values.
    ExternalLookupMap m_externalLookups;

    //* @brief Listener instance to receive messages.
    MessageListener m_listener;

    /**
     * @brief Mutexes used to hold the processing of incoming return values while commands
     * are being sent and recorded.
     */
    std::mutex m_cmdMutex;

    //* @brief Semaphore for return values from LookupNow
    ThreadSemaphore m_lookupSem;

    //* @brief Mutex to prevent contention for the following resources
    std::mutex m_lookupMutex;

    //* @brief Place to store result of current pending LookupNow request
    Value m_pendingLookupResult;

    //* @brief Place to store state of pending LookupNow
    State m_pendingLookupState;

    //* @brief Serial # of current pending LookupNow request, or 0
    // g++ 4.8.x doesn't support std::atomic_uint32_t
    std::atomic<uint32_t> m_pendingLookupSerial;
  };

  //
  // Helper functions
  //

  //! Helper function for converting message names into the proper
  //! format given the command type and a user-defined id.
  std::string formatMessageName(const std::string &name,
                                const std::string &command,
                                int id) 
  {
    std::ostringstream ss;
    if (command == RECEIVE_COMMAND_COMMAND) {
      ss << COMMAND_PREFIX << name;
    }
    else if (command == GET_PARAMETER_COMMAND) {
      ss << PARAM_PREFIX << name;
    }
    else {
      ss << name;
    }
    ss << '_' << id;
    return ss.str();
  }

  //! Helper function for converting message names into the proper
  //! format given the command type.
  std::string formatMessageName(const char *name, const std::string &command)
  {
    return formatMessageName(std::string(name), command, 0);
  }

  // Helper function used in ParseExternalLookups.
  Value parseTypedValue(const char *type, pugi::xml_attribute const value)
  {
    switch (parseValueType(type)) {
    case BOOLEAN_TYPE:
      return Value(value.as_bool());

    case INTEGER_TYPE:
      // FIXME: pugixml attribute routines don't have out-of-band error signaling
      // FIXME: pugixml relies on compiler definition of int type
      return Value((Integer) value.as_int());

    case REAL_TYPE:
      return Value(value.as_double());
      
    case STRING_TYPE:
      return Value(value.value()); // empty string is valid

    case BOOLEAN_ARRAY_TYPE: {
      std::vector<std::string> * args =
        InterfaceSchema::parseCommaSeparatedArgs(value.value());
      size_t n = args->size();
      BooleanArray ba(n);
      for (size_t i = 0; i < n; ++i) {
        Boolean b = false;
        if (!parseValue<Boolean>((*args)[i], b)) {
          reportInterfaceError("IpcAdapter: \"" << (*args)[i] << "\" is not a valid Boolean");
          return Value();
        }
        ba.setElement(i, b);
      }
      delete args;
      return Value(ba);
    }

    case INTEGER_ARRAY_TYPE: {
      std::vector<std::string> * args =
        InterfaceSchema::parseCommaSeparatedArgs(value.value());
      size_t n = args->size();
      IntegerArray ia(n);
      for (size_t i = 0; i < n; ++i) {
        Integer in = 0;
        if (!parseValue<Integer>((*args)[i], in)) {
          reportInterfaceError("IpcAdapter: \"" << (*args)[i] << "\" is not a valid Integer");
          return Value();
        }
        ia.setElement(i, in);
      }
      delete args;
      return Value(ia);
    }

    case REAL_ARRAY_TYPE: {
      std::vector<std::string> * args =
        InterfaceSchema::parseCommaSeparatedArgs(value.value());
      size_t n = args->size();
      RealArray ra(n);
      for (size_t i = 0; i < n; ++i) {
        Real d = 0.0;
        if (!parseValue<Real>((*args)[i], d)) {
          reportInterfaceError("IpcAdapter: \"" << (*args)[i] << "\" is not a valid Real number");
          return Value();
        }
        ra.setElement(i, d);
      }
      delete args;
      return Value(ra);
    }
      
    case STRING_ARRAY_TYPE: {
      std::vector<std::string> * args =
        InterfaceSchema::parseCommaSeparatedArgs(value.value());
      size_t n = args->size();
      StringArray sa(n);
      for (size_t i = 0; i < n; ++i) {
        String s = (*args)[i];
        // TODO: handle escapes?
        if (s[0] != '"' || s[s.size() - 1] != '"') {
          reportInterfaceError("IpcAdapter: String \"" << (*args)[i]
                               << "\" lacks leading or trailing double-quote character(s)");
          return Value();
        }
        s = s.substr(1, s.size() - 2);
        sa.setElement(i, s);
      }
      delete args;
      return Value(sa);
    }
      
    default:
      errorMsg("IpcAdapter: invalid or unimplemented lookup value type " << type);
      return Value();
    }
  }

  /**
   * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
   */
  Value parseReturnValue(const std::vector<PlexilMsgBase *>& msgs) 
  {
    size_t nValues = msgs[0]->count;
    checkError(nValues == 1, "PlexilMsgType_ReturnValue may only have one parameter");
    return getPlexilMsgValue(msgs[1]);
  }

}

extern "C"
void initIpcAdapter()
{
  REGISTER_ADAPTER(PLEXIL::IpcAdapter, "IpcAdapter")
}
