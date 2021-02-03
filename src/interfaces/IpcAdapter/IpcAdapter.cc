/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "IpcAdapter.h"

#include "AdapterExecInterface.hh"
#include "AdapterConfiguration.hh"
#include "AdapterFactory.hh"
#include "ArrayImpl.hh"
#include "Command.hh"
#include "commandHandlerDefs.hh"
#include "Debug.hh"
#include "Error.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"
#include "IpcFacade.hh"
#include "lookupHandlerDefs.hh"
#include "MessageQueueMap.hh"
#include "parsePlan.hh"
#include "State.hh"
#include "StateCacheEntry.hh"
#include "ThreadSemaphore.hh"
#include "Update.hh"
#include "NodeConnector.hh"

#include <algorithm>
#include <string>

#if defined(HAVE_CSTDLIB)
#include <cstdlib>
#elif defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#define TRANSACTION_ID_SEPARATOR_CHAR ':'

namespace PLEXIL 
{

  namespace {

    //
    // Constants
    //

    std::string const COMMAND_PREFIX = "__COMMAND__";
    std::string const PARAM_PREFIX = "__PARAMETER__";
    std::string const SERIAL_UID_SEPERATOR = ":";

    std::string const SEND_MESSAGE_COMMAND = "SendMessage";
    std::string const RECEIVE_MESSAGE_COMMAND = "ReceiveMessage";
    std::string const RECEIVE_COMMAND_COMMAND = "ReceiveCommand";
    std::string const GET_PARAMETER_COMMAND = "GetParameter";
    std::string const SEND_RETURN_VALUE_COMMAND = "SendReturnValue";
    std::string const UPDATE_LOOKUP_COMMAND = "UpdateLookup";

    //
    // Helper functions
    //

    /**
     * @brief Convert a message name into the proper format,
     *        given the command type and a user-defined id.
     */
    static std::string formatMessageName(const std::string& name,
                                         const std::string& command,
                                         int id = 0)
    {
      std::ostringstream ss;
      if (command == RECEIVE_COMMAND_COMMAND)
        ss << COMMAND_PREFIX;
      if (command == GET_PARAMETER_COMMAND)
        ss << PARAM_PREFIX;
      ss << name << '_' << id;
      return ss.str();
    }

    // Helper function used in ParseExternalLookups.
    static Value parseTypedValue(char const *type, pugi::xml_attribute const value)
    {
      ValueType t = parseValueType(type);

      switch (t) {
      case BOOLEAN_TYPE:
        return Value(value.as_bool());

      case INTEGER_TYPE:
        // FIXME: pugixml attribute routines don't have out-of-band error signaling
        // FIXME: pugixml relies on compiler definition of int type
        return Value((int32_t) value.as_int());

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
     * @brief Given a sequence of messages, turn the trailers into a Value for the Exec.
     */
    static Value parseReturnValue(const std::vector<const PlexilMsgBase*>& msgs) 
    {
      size_t nValues = msgs[0]->count;
      checkError(nValues == 1, "PlexilMsgType_ReturnValue may only have one parameter");
      return getPlexilMsgValue(msgs[1]);
    }

  }

  /**
   * @class IpcAdapter
   * @brief An InterfaceAdapter specialization which uses TCA-IPC to communicate
   *        with an external agent, which may be another PLEXIL Exec.
   */

  class IpcAdapter: public InterfaceAdapter
  {
  private:

    //
    // Private data types
    //

    //* brief Cache of message/command/lookup names we're actively listening for
    typedef std::map<std::string, State> ActiveListenerMap;

    //* brief Cache of command serials and their corresponding ack and return value variables
    typedef std::map<uint32_t, Command *> PendingCommandsMap;

    typedef std::map<State, Value> ExternalLookupMap;

    //
    // Private intarnal classes
    //

    //* brief Class to receive messages from Ipc
    class MessageListener :
      public IpcMessageListener
    {
    private:
      IpcAdapter& m_adapter;
    public:
      MessageListener(IpcAdapter& adapter)
        : m_adapter(adapter)
      {
      }

      ~MessageListener()
      {
      }

      void ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs)
      {
        PlexilMsgType msgType = (PlexilMsgType) msgs.front()->msgType;
        debugMsg("IpcAdapter:handleIpcMessage", " received message type = " << msgType);
        switch (msgType) {
          // NotifyExec is a PlexilMsgBase
        case PlexilMsgType_NotifyExec:

          // AddPlan/AddPlanFile is a PlexilStringValueMsg
        case PlexilMsgType_AddPlan:
        case PlexilMsgType_AddPlanFile:

          // AddLibrary/AddLibraryFile is a PlexilStringValueMsg
        case PlexilMsgType_AddLibrary:
        case PlexilMsgType_AddLibraryFile:

          // In each case, simply send it to Exec and free the message
          m_adapter.enqueueMessage(msgs.front());
          break;

          // Command is a PlexilStringValueMsg
          // Optionally followed by parameters

        case PlexilMsgType_Command:
          // Stash this and wait for the rest
          debugMsg("IpcAdapter:handleIpcMessage", " processing as command")
            ;
          m_adapter.handleCommandSequence(msgs);
          break;

          // Message is a PlexilStringValueMsg
          // No parameters

        case PlexilMsgType_Message:
          debugMsg("IpcAdapter:handleIpcMessage", " processing as message")
            ;
          m_adapter.handleMessageMessage(reinterpret_cast<const PlexilStringValueMsg*>(msgs.front()));
          break;

          // Not implemented yet
        case PlexilMsgType_PlannerUpdate:
          break;

          // TelemetryValues is a PlexilStringValueMsg
          // Followed by 0 (?) or more values
        case PlexilMsgType_TelemetryValues:
          m_adapter.handleTelemetryValuesSequence(msgs);
          break;

          // ReturnValues is a PlexilReturnValuesMsg
          // Followed by 0 (?) or more values
        case PlexilMsgType_ReturnValues:
          // Only pay attention to our return values
          debugMsg("IpcAdapter:handleIpcMessage", " processing as return value")
            ;
          m_adapter.handleReturnValuesSequence(msgs);
          break;

        case PlexilMsgType_LookupNow:
          m_adapter.handleLookupNow(msgs);
          break;

          // unhandled so far
        default:
          debugMsg("IpcAdapter::enqueueMessageSequence",
                   "Unhandled leader message type " << msgs.front()->msgType << ". Disregarding")
            ;
        }
      }
    };

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

    //* @brief Semaphore for return values from LookupNow
    ThreadSemaphore m_lookupSem;

    /**
     * @brief Mutex used to hold the processing of incoming return values while commands
     * are being sent and recorded.
     */
    ThreadMutex m_cmdMutex;

    //* @brief Place to store result of current pending LookupNow request
    Value m_pendingLookupResult;

    //* @brief Place to store state of pending LookupNow
    State m_pendingLookupState;

    //* @brief Serial # of current pending LookupNow request, or 0
    uint32_t m_pendingLookupSerial;

    //
    // Handler classes
    //

    friend class IpcCommandHandler;
    friend class SendMessageCommandHandler;
    friend class SendReturnValueCommandHandler;
    friend class ReceiveMessageCommandHandler;
    friend class ReceiveCommandCommandHandler;
    friend class GetParameterCommandHandler;
    friend class UpdateLookupCommandHandler;

    friend class IpcLookupHandler;
    friend class IpcExternalLookupHandler;

    friend class IpcPlannerUpdateHandler;

  public:

    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    
    IpcAdapter(AdapterExecInterface& execInterface) :
      InterfaceAdapter(execInterface),
      m_ipcFacade(),
      m_messageQueues(execInterface),
      m_externalLookupNames(),
      m_externalLookups(),
      m_listener(*this),
      m_lookupSem(),
      m_cmdMutex(),
      m_pendingLookupResult(),
      m_pendingLookupState(),
      m_pendingLookupSerial(0)
    {
      debugMsg("IpcAdapter:IpcAdapter", " configuration XML not provided");
    }

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml A const reference to the XML element describing this adapter
     */
    IpcAdapter(AdapterExecInterface& execInterface, pugi::xml_node const xml) :
      InterfaceAdapter(execInterface, xml),
      m_ipcFacade(),
      m_messageQueues(execInterface),
      m_externalLookupNames(),
      m_externalLookups(),
      m_listener(*this),
      m_lookupSem(),
      m_cmdMutex(),
      m_pendingLookupResult(),
      m_pendingLookupState(),
      m_pendingLookupSerial(0)
    {
      condDebugMsg(!xml, "IpcAdapter:IpcAdapter", " configuration XML not provided");
      condDebugMsg(xml, "IpcAdapter:IpcAdapter", " configuration XML = " << xml);
    }

    /**
     * @brief Destructor.
     */
    ~IpcAdapter() {
    }


    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, and registers it with the interface registry.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize(AdapterConfiguration *config)
    {
      debugMsg("IpcAdapter:initialize", " called");

      // Get taskName, serverName from XML, if supplied
      const char* taskName = "";
      const char* serverName = "";
      pugi::xml_attribute acceptDuplicates;

      pugi::xml_node const xml = this->getXml();
      if (!xml.empty()) {
        taskName = xml.attribute("TaskName").value();
        serverName = xml.attribute("Server").value();
        acceptDuplicates = xml.attribute("AllowDuplicateMessages");
        parseExternalLookups(xml.child("ExternalLookups"));
      }

      // Use defaults if necessary
      if (*serverName == '\0') {
        serverName = "localhost";
      }
      if (*taskName == '\0') {
        taskName = m_ipcFacade.getUID().c_str();
      }
      if (!acceptDuplicates.empty()) {
        m_messageQueues.setAllowDuplicateMessages(acceptDuplicates.as_bool());
      } 
      else {
        //debugging only. set to true for release
        m_messageQueues.setAllowDuplicateMessages(false);
      }

      debugMsg("IpcAdapter:initialize",
               " Connecting module " << taskName <<
               " to central server at " << serverName);

      // init IPC
      assertTrueMsg(m_ipcFacade.initialize(taskName, serverName) == IPC_OK,
                    "IpcAdapter: Unable to connect to the central server at " << serverName);

      //
      // Register specific command handlers
      //

      config->registerCommandHandler(SEND_MESSAGE_COMMAND,
                                     new SendMessageCommandHandler(*this));
      config->registerCommandHandler(RECEIVE_MESSAGE_COMMAND,
                                     new ReceiveMessageCommandHandler(*this));
      config->registerCommandHandler(RECEIVE_COMMAND_COMMAND,
                                     new ReceiveCommandCommandHandler(*this));
      config->registerCommandHandler(GET_PARAMETER_COMMAND,
                                     new GetParameterCommandHandler(*this));
      config->registerCommandHandler(SEND_RETURN_VALUE_COMMAND,
                                     new SendReturnValueCommandHandler(*this));
      config->registerCommandHandler(UPDATE_LOOKUP_COMMAND,
                                     new UpdateLookupCommandHandler(*this));

      //
      // Register handler for external (published) lookups
      //

      if (!m_externalLookupNames.empty()) {
        config->registerCommonLookupHandler(new IpcExternalLookupHandler(*this),
                                            m_externalLookupNames);
      }

      //
      // Register for other functionality as required
      //

      if (xml.child(InterfaceSchema::DEFAULT_ADAPTER_TAG())) {
        // Is default adapter
        config->setDefaultCommandHandler(new IpcCommandHandler(*this));
        config->setDefaultLookupHandler(new IpcLookupHandler(*this));
        config->registerPlannerUpdateHandler(new IpcPlannerUpdateHandler(*this));
      }
      else {
        if (xml.child(InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG())) {
          // Is default command adapter
          config->setDefaultCommandHandler(new IpcCommandHandler(*this));
        }
        else {
          config->registerCommonCommandHandler(new IpcCommandHandler(*this), xml);
        }

        if (xml.child(InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG())) {
          // Is default lookup adapter
          config->setDefaultLookupHandler(new IpcLookupHandler(*this));
        }
        else {
          config->registerCommonLookupHandler(new IpcLookupHandler(*this), xml);
        }

        if (xml.child(InterfaceSchema::PLANNER_UPDATE_TAG())) {
          // Register planner update handler
          config->registerPlannerUpdateHandler(new IpcPlannerUpdateHandler(*this));
        }
      }

      debugMsg("IpcAdapter:initialize", " succeeded");
      return true;
    }

    /**
     * @brief Starts the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool start() {
      // Spawn listener thread
      assertTrueMsg(m_ipcFacade.start() == IPC_OK,
                    "IpcAdapter: Unable to spawn IPC dispatch thread");
      debugMsg("IpcAdapter:start", " spawned IPC dispatch thread");
      m_ipcFacade.subscribeAll(&m_listener);
      debugMsg("IpcAdapter:start", " succeeded");
      return true;
    }

    /**
     * @brief Stops the adapter.
     * @return true if successful, false otherwise.
     */
    virtual bool stop() {
      m_ipcFacade.stop();

      debugMsg("IpcAdapter:stop", " succeeded");
      return true;
    }

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown() {
      m_ipcFacade.shutdown();
      debugMsg("IpcAdapter:shutdown", " succeeded");
      return true;
    }

  private:

    //
    // Implementation methods
    //

    //
    // Perform a command abort for a handler.
    //

    // TODO
    //  Implement unique command IDs for referencing command instances
    
    // Only implemented for ReceiveMessage and ReceiveCommand.

    void performAbort(Command *command, AdapterExecInterface *intf)
    {
      std::string const &cmdName = command->getName();
      std::vector<Value> const &cmdArgs = command->getArgValues();

      // These errors shouldn't happen, if the command was executed in the first place
      // assertTrueMsg(cmdArgs.size() == 1,
      //               "IpcAdapter:invokeAbort: Command " << cmdName
      //               << " requires exactly one argument");
      // assertTrueMsg(cmdArgs.front().isKnown() && cmdArgs.front().valueType() == STRING_TYPE,
      //               "IpcAdapter: The argument to the " << cmdName
      //               << " command being aborted, " << cmdArgs.front()
      //               << ", is not a valid String value");

      std::string const *theMessage;
      cmdArgs.front().getValuePointer(theMessage);

      debugMsg("IpcAdapter:invokeAbort",
               " for " << cmdName
               << ": Aborting command listener " << *theMessage << std::endl);
      m_messageQueues.removeRecipient(*theMessage, command);
      intf->handleCommandAbortAck(command, true);
      intf->notifyOfExternalEvent();
    }


    //
    // Command handler class for IpcAdapter commands
    //

    class IpcCommandHandler :
      public CommandHandler
    {
    public:

      IpcCommandHandler(IpcAdapter &adapter)
        : m_adapter(adapter)
      {
      }

      virtual ~IpcCommandHandler()
      {
      }

      // Default method.
      // Specialized derived classes have their own methods.
      virtual void executeCommand(Command *command, AdapterExecInterface *intf) 
      {
        m_adapter.defaultExecuteCommand(command, intf);
      }

      // Default method.
      virtual void abortCommand(Command *cmd, AdapterExecInterface *intf)
      {
        defaultAbortCommandHandler(cmd, intf);
      }

    protected:

      //
      // Member variable shared with derived classes
      //

      IpcAdapter &m_adapter;
    };

    // Adapter implementation of default command handler

    void defaultExecuteCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::string const &name = command->getName();
      debugMsg("IpcAdapter:executeCommand", " (default) for \"" << name << "\"");
      std::vector<Value> const &args = command->getArgValues();
      if (!args.empty())
        debugMsg("IpcAdapter:executeCommand", " first parameter is \""
                 << args.front()
                 << "\"");

      uint32_t serial;
      size_t sep_pos = name.find_first_of(TRANSACTION_ID_SEPARATOR_CHAR);

      // lock mutex to ensure no return values are processed while the command is being
      // sent and logged
      ThreadMutexGuard guard(m_cmdMutex);

      // decide to direct or publish command
      if (sep_pos != std::string::npos) {
        serial = m_ipcFacade.sendCommand(name.substr(sep_pos + 1),
                                         name.substr(0, sep_pos),
                                         args);
      }
      else {
        serial = m_ipcFacade.publishCommand(name, args);
      }

      // log ack and return variables in case we get values for them
      // FIXME Devise a way to report this error without crashing the Exec
      assertTrueMsg(serial != IpcFacade::ERROR_SERIAL(),
                    "IpcAdapter::executeCommand: IPC Error, IPC_errno = "
                    << m_ipcFacade.getError());
      m_pendingCommands[serial] = command;

      // store ack
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand",
               " command \"" << name << "\" sent.");
    }


    /**
     * @brief handles SEND_MESSAGE_COMMAND commands from the exec
     */

    class SendMessageCommandHandler :
      public IpcCommandHandler
    {
    public:

      SendMessageCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~SendMessageCommandHandler()
      {
      }

      virtual void executeCommand(Command *command, AdapterExecInterface *intf)
      {
        m_adapter.executeSendMessageCommand(command, intf);
      }
    };

    // Adapter implementation of above command
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

      // FIXME Rport error w/o crashing the Exec
      assertTrue_2(m_ipcFacade.publishMessage(*theMessage) != IpcFacade::ERROR_SERIAL(),
                   "Message publish failed");

      // store ack
      intf->handleCommandAck(command, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " message \"" << *theMessage << "\" sent.");
    }

    /**
     * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
     */

    class SendReturnValueCommandHandler :
      public IpcCommandHandler
    {
    public:

      SendReturnValueCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~SendReturnValueCommandHandler()
      {
      }

      void executeCommand(Command *command, AdapterExecInterface *intf)
      {
        m_adapter.executeSendReturnValueCommand(command, intf);
      }
    };

    // Adapter implementation of SendReturnValue
    void executeSendReturnValueCommand(Command *command, AdapterExecInterface *intf)
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for two arguments, the message and the value
      assertTrueMsg(args.size() == 2,
                    "IpcAdapter: The SendReturnValue command requires exactly two arguments.");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The message name argument to the SendReturnValue command, "
                    << args.front() << ", is not a valid String value");
      const std::string *front;
      args.front().getValuePointer(front);
      uint32_t serial;
      //grab serial from parameter
      std::string::size_type sep_pos = front->find(SERIAL_UID_SEPERATOR, 1);
      assertTrueMsg(sep_pos != std::string::npos, "Could not find UID seperator in first parameter of return value");
      std::string const serial_string = front->substr(0, sep_pos);
      serial = atoi(serial_string.c_str());
      std::string front_string = front->substr(sep_pos + 1, front->size());
      debugMsg("IpcAdapter:executeCommand",
               " SendReturnValue(sender_serial:\"" << serial << "\" \"" << front_string << "\")");
      //publish
      serial = m_ipcFacade.publishReturnValues(serial, front_string, args[1]);
      assertTrue_2(serial != IpcFacade::ERROR_SERIAL(), "Return values failed to be sent");

      // store ack
      intf->handleCommandAck(command, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " return value sent.");
    }

    /**
     * @brief handles ReceiveMessage commands from the exec
     */

    class ReceiveMessageCommandHandler :
      public IpcCommandHandler
    {
    public:

      ReceiveMessageCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~ReceiveMessageCommandHandler()
      {
      }

      virtual void executeCommand(Command *command, AdapterExecInterface *intf)
      {
        m_adapter.executeReceiveMessageCommand(command, intf);
      }
      
      virtual void abortCommand(Command *cmd, AdapterExecInterface *intf)
      {
        m_adapter.performAbort(cmd, intf);
      }

    };

    // Adapter implementation of ReceiveMesage command
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
      debugMsg("IpcAdapter:executeCommand",
               " message handler for \"" << *theMessage << "\" registered.");
    }


    /**
     * @brief handles RECEIVE_COMMAND_COMMAND commands from the exec
     */

    class ReceiveCommandCommandHandler :
      public IpcCommandHandler
    {
    public:

      ReceiveCommandCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~ReceiveCommandCommandHandler()
      {
      }

      virtual void executeCommand(Command *command, AdapterExecInterface *intf)
      {
        m_adapter.executeReceiveCommandCommand(command, intf);
      }
      
      virtual void abortCommand(Command *cmd, AdapterExecInterface *intf)
      {
        m_adapter.performAbort(cmd, intf);
      }

    };

    void executeReceiveCommandCommand(Command *command, AdapterExecInterface *intf)
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      assertTrueMsg(args.size() == 1,
                    "IpcAdapter: The " << RECEIVE_COMMAND_COMMAND << " command requires exactly one argument");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The argument to the " << RECEIVE_COMMAND_COMMAND
                    << " command, " << args.front()
                    << ", is not a string");
      std::string const *cmdName = NULL;
      args.front().getValuePointer(cmdName);
      std::string msgName(formatMessageName(*cmdName, RECEIVE_COMMAND_COMMAND));
      m_messageQueues.addRecipient(msgName, command);
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand",
               " command handler for \"" << msgName << "\" registered.");
    }

    /**
     * @brief handles GET_PARAMETER_COMMAND commands from the exec
     */

    class GetParameterCommandHandler :
      public IpcCommandHandler
    {
    public:

      GetParameterCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~GetParameterCommandHandler()
      {
      }

      virtual void executeCommand(Command *command, AdapterExecInterface *intf) 
      {
        m_adapter.executeGetParameterCommand(command, intf);
      }
    };

    void executeGetParameterCommand(Command *command, AdapterExecInterface *intf) 
    {
      std::vector<Value> const &args = command->getArgValues();
      // Check for one argument, the message
      assertTrueMsg(args.size() == 1 || args.size() == 2,
                    "IpcAdapter: The " << GET_PARAMETER_COMMAND << " command requires either one or two arguments");
      assertTrueMsg(args.front().isKnown() && args.front().valueType() == STRING_TYPE,
                    "IpcAdapter: The first argument to the " << GET_PARAMETER_COMMAND << " command, "
                    << args.front() << ", is not a string");
      int32_t id;
      if (args.size() == 1)
        id = 0;
      else {
        assertTrueMsg(args[1].isKnown() && args[1].valueType() == INTEGER_TYPE,
                      "IpcAdapter: The second argument to the " << GET_PARAMETER_COMMAND << " command, " << args[1]
                      << ", is not an Integer");
        args[1].getValue(id);
        assertTrueMsg(id >= 0,
                      "IpcAdapter: The second argument to the " << GET_PARAMETER_COMMAND << " command, " << args[1]
                      << ", is not a valid index");
      }
      std::string const *cmdName = NULL;
      args.front().getValuePointer(cmdName);
      std::string msgName(formatMessageName(*cmdName, GET_PARAMETER_COMMAND, id));
      m_messageQueues.addRecipient(msgName, command);
      intf->handleCommandAck(command, COMMAND_SENT_TO_SYSTEM);
      intf->notifyOfExternalEvent();
      debugMsg("IpcAdapter:executeCommand", " message handler for \"" << msgName << "\" registered.");
    }


    /**
     * @brief handles UPDATE_LOOKUP_COMMAND commands from the exec
     */

    class UpdateLookupCommandHandler :
      public IpcCommandHandler
    {
    public:

      UpdateLookupCommandHandler(IpcAdapter &adapter)
        : IpcCommandHandler(adapter)
      {
      }

      virtual ~UpdateLookupCommandHandler()
      {
      }

      virtual void executeCommand(Command *command, AdapterExecInterface *intf)
      {
        m_adapter.executeUpdateLookupCommand(command, intf);
      }
    };

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
      ThreadMutexGuard guard(m_cmdMutex);

      // Extract state from command parameters
      std::string const *lookupName;
      args.front().getValuePointer(lookupName);
      // Warn, but do not crash, if not declared
      if (std::find(m_externalLookupNames.begin(),
                    m_externalLookupNames.end(),
                    *lookupName)
          == m_externalLookupNames.end()) {
        warn("IpcAdapter: The external lookup " << *lookupName << " is not declared. Ignoring UpdateLookup command.");
        intf->handleCommandAck(command, COMMAND_FAILED);
        intf->notifyOfExternalEvent();
        return;
      }
      
      State state(*lookupName, nargs - 2);
      for (size_t i = 2; i < nargs; ++i)
        state.setParameter(i - 2, args[i]);

      //Set value internally
      m_externalLookups[state] = args[1];
      // Notify internal users, if any
      intf->handleValueChange(state, args[1]);

      //send telemetry
      std::vector<Value> result_and_args(nargs - 1);
      for (size_t i = 1; i < nargs; ++i)
        result_and_args[i - 1] = args[i];
      if (m_ipcFacade.publishTelemetry(*lookupName, result_and_args)
          == IpcFacade::ERROR_SERIAL()) {
        warn("IpcAdapter: publishTelemetry returned status \"" << m_ipcFacade.getError() << "\"");
        intf->handleCommandAck(command, COMMAND_FAILED);
      }
      else {
        // Notify of success
        intf->handleCommandAck(command, COMMAND_SUCCESS);
      }
    
      intf->notifyOfExternalEvent();
    }

    //
    // Lookup handling
    //

    //
    // For lookups responded to by another agent
    //

    class IpcLookupHandler :
      public LookupHandler
    {
    public:

      IpcLookupHandler(IpcAdapter &adapter)
        : LookupHandler(),
          m_adapter(adapter)
      {
      }

      virtual ~IpcLookupHandler()
      {
      }

      virtual void lookupNow(const State &state, StateCacheEntry &entry)
      {
        m_adapter.performLookupNow(state, entry);
      }

    private:

      IpcAdapter &m_adapter;
    };

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @param entry The state cache entry in which to store the result.
     */

    void performLookupNow(const State& state, StateCacheEntry &entry) 
    {
      const std::string& stateName = state.name();
      const std::vector<Value>& params = state.parameters();
      size_t nParams = params.size();
      debugMsg("IpcAdapter:lookupNow",
               " for state " << stateName
               << " with " << nParams << " parameters");

      //send lookup message
      m_pendingLookupResult.setUnknown();
      size_t sep_pos = stateName.find_first_of(TRANSACTION_ID_SEPARATOR_CHAR);
      //decide to direct or publish lookup
      {
        ThreadMutexGuard g(m_cmdMutex);
        if (sep_pos != std::string::npos) {
          std::string const dest(stateName.substr(0, sep_pos));
          std::string const sentStateName = stateName.substr(sep_pos + 1);
          m_pendingLookupState = state;
          m_pendingLookupState.setName(sentStateName);
          m_pendingLookupSerial = m_ipcFacade.sendLookupNow(sentStateName,
                                                            dest,
                                                            params);
        }
        else {
          m_pendingLookupState = state;
          m_pendingLookupSerial = m_ipcFacade.publishLookupNow(stateName, params);
        }
      }

      // Wait for results
      // N.B. shouldn't have to worry about signals causing wait to be interrupted -
      // ExecApplication blocks most of the common ones
      int errnum = m_lookupSem.wait();
      assertTrueMsg(errnum == 0,
                    "IpcAdapter::lookupNow: semaphore wait failed, result = " << errnum);

      entry.update(m_pendingLookupResult);

      // Clean up
      {
        ThreadMutexGuard g(m_cmdMutex);
        m_pendingLookupSerial = 0;
        m_pendingLookupState = State();
      }
      m_pendingLookupResult.setUnknown();
    }

    //
    // For lookups this agent publishes to other agents
    //

    class IpcExternalLookupHandler :
      public LookupHandler
    {
    public:

      IpcExternalLookupHandler(IpcAdapter &adapter)
        : LookupHandler(),
          m_adapter(adapter)
      {
      }

      virtual ~IpcExternalLookupHandler()
      {
      }

      virtual void lookupNow(const State &state, StateCacheEntry &entry)
      {
        m_adapter.getExternalLookup(state, entry);
      }

    private:

      IpcAdapter &m_adapter;
    };

    // IpcAdapter method to get value of published lookups

    void getExternalLookup(const State &state, StateCacheEntry &entry)
    {
      ExternalLookupMap::iterator it = m_externalLookups.find(state);
      if (it != m_externalLookups.end()) {
        debugMsg("IpcAdapter:lookupNow",
                 " returning external lookup " << state
                 << " with internal value " << it->second);
        entry.update(it->second);
      }
    }

    //
    // Planner update handler
    //

    struct IpcPlannerUpdateHandler: public PlannerUpdateHandler
    {
    public:

      IpcPlannerUpdateHandler(IpcAdapter &adapter)
        : m_adapter(adapter)
      {
      }

      virtual ~IpcPlannerUpdateHandler()
      {
      }

      virtual void operator()(Update *upd, AdapterExecInterface *intf)
      {
        m_adapter.performPlannerUpdate(upd, intf);
      }

    private:

      IpcAdapter &m_adapter;
    };


    // Implementation method for the above
    void performPlannerUpdate(Update *update, AdapterExecInterface *intf)
    {
      std::string const& name = update->getSource()->getNodeId();
      debugMsg("IpcAdapter:sendPlannerUpdate", " for \"" << name << "\"");
      std::vector<std::pair<std::string, Value> > args(update->getPairs().begin(),
                                                       update->getPairs().end());
      if(args.empty()) {
        debugMsg("IpcAdapter:sendPlannerUpdate", "Emtpy update");
        return;
      }

      ThreadMutexGuard guard(m_cmdMutex);
      uint32_t serial = m_ipcFacade.publishUpdate(name, args);
      checkInterfaceError(serial != IpcFacade::ERROR_SERIAL(),
                          "IpcAdapter::sendPlannerUpdate: IPC Error, IPC_errno = " <<
                          m_ipcFacade.getError());
      intf->handleUpdateAck(update, true);
      intf->notifyOfExternalEvent();
    }

    void parseExternalLookups(pugi::xml_node const external) 
    {
      if (external) {
        // process external lookups
        pugi::xml_node lookup = external.child("Lookup");
        while (lookup) {
          const pugi::xml_attribute nameAttr = lookup.attribute("name");
          assertTrueMsg(!nameAttr.empty() && nameAttr.value(),
                        "IpcAdapter:parseExternalLookups: Lookup element attribute 'name' missing or empty");
          const pugi::xml_attribute typeAttr = lookup.attribute("type");
          assertTrueMsg(!typeAttr.empty() && typeAttr.value(),
                        "IpcAdapter:parseExternalLookups: Lookup element attribute 'type' missing or empty");
          const pugi::xml_attribute defAttr = lookup.attribute("value");
          assertTrueMsg(!defAttr.empty() && defAttr.value(),
                        "IpcAdapter:parseExternalLookups: Lookup element attribute 'value' missing or empty");

          std::string const nameString(nameAttr.value());

          debugMsg("IpcAdapter:parseExternalLookups",
                   "External Lookup: state=\"" << nameString
                   << "\" type=\"" << typeAttr.value()
                   << "\" default=\"" << defAttr.value() << "\"");
          const char *type = typeAttr.value();
          if (std::find(m_externalLookupNames.begin(),
                        m_externalLookupNames.end(),
                        nameString)
              != m_externalLookupNames.end()) {
            warn("IpcAdapter: Lookup name \"" << nameString << "\" multiply declared");
          }
          else {
            m_externalLookupNames.push_back(nameString);
            m_externalLookups[State(nameString)] = parseTypedValue(type, defAttr);
          }
          lookup = lookup.next_sibling("Lookup");
        }
      }
    }

    /**
     * @brief Send a single message to the Exec's queue and free the message
     */
    void enqueueMessage(const PlexilMsgBase* msgData) {
      assertTrue_2(msgData, "IpcAdapter::enqueueMessage: msgData is null");

      switch (msgData->msgType) {
      case PlexilMsgType_NotifyExec:
        m_execInterface.notifyOfExternalEvent();
        break;

        // AddPlan is a PlexilStringValueMsg
      case PlexilMsgType_AddPlan: {
        const PlexilStringValueMsg* stringMsg = reinterpret_cast<const PlexilStringValueMsg*>(msgData);
        assertTrueMsg(stringMsg->stringValue != NULL,
                      "IpcAdapter::enqueueMessage: AddPlan message contains null plan string");

        // parse into XML document
        try {
          pugi::xml_document doc;
          pugi::xml_parse_result result = doc.load_string(stringMsg->stringValue);
          if (result.status == pugi::status_ok) {
            m_execInterface.handleAddPlan(doc.document_element());
            // Always notify immediately when adding a plan
            m_execInterface.notifyOfExternalEvent();
          }
          else {
            std::cerr << "Error parsing plan XML:\n" << result.description() << std::endl;
          }
        } catch (const ParserException& e) {
          std::cerr << "Error parsing plan: \n" << e.what() << std::endl;
        }
      }
        break;

        // AddPlanFile is a PlexilStringValueMsg
      case PlexilMsgType_AddPlanFile: {
        const PlexilStringValueMsg* stringMsg = reinterpret_cast<const PlexilStringValueMsg*>(msgData);
        assertTrueMsg(stringMsg->stringValue != NULL,
                      "IpcAdapter::enqueueMessage: AddPlanFile message contains null file name");

        // parse into XML document
        try {
          pugi::xml_document *doc = loadXmlFile(stringMsg->stringValue);
          if (doc) {
            m_execInterface.handleAddPlan(doc->document_element());
            // Always notify immediately when adding a plan
            m_execInterface.notifyOfExternalEvent();
          }
          else {
            std::cerr << "Error parsing plan from file: file " << stringMsg->stringValue << " not found" << std::endl;
          }
        } catch (const ParserException& e) {
          std::cerr << "Error parsing plan from file: \n" << e.what() << std::endl;
        }
      }
        break;

        // AddLibrary is a PlexilStringValueMsg
      case PlexilMsgType_AddLibrary: {
        const PlexilStringValueMsg* stringMsg = reinterpret_cast<const PlexilStringValueMsg*>(msgData);
        assertTrueMsg(stringMsg->stringValue != NULL,
                      "IpcAdapter::enqueueMessage: AddLibrary message contains null library node string");

        // parse XML into node structure
        try {
          pugi::xml_document *doc = new pugi::xml_document;
          pugi::xml_parse_result result = doc->load_string(stringMsg->stringValue);
          if (result.status == pugi::status_ok)
            m_execInterface.handleAddLibrary(doc);
          else {
            delete doc;
            std::cerr << "Error parsing plan XML:\n" << result.description() << std::endl;
          }
        } catch (const ParserException& e) {
          std::cerr << "Error parsing library node: \n" << e.what() << std::endl;
        }
      }
        break;

        // AddLibraryFile is a PlexilStringValueMsg
      case PlexilMsgType_AddLibraryFile: {
        const PlexilStringValueMsg* stringMsg = reinterpret_cast<const PlexilStringValueMsg*>(msgData);
        assertTrueMsg(stringMsg->stringValue != NULL,
                      "IpcAdapter::enqueueMessage: AddLibraryFile message contains null file name");

        // parse XML into node structure
        try {
          pugi::xml_document *doc = loadXmlFile(stringMsg->stringValue);
          if (doc)
            m_execInterface.handleAddLibrary(doc);
          else {
            std::cerr << "Error parsing plan from file: file " << stringMsg->stringValue << " not found" << std::endl;
          }
        } catch (const ParserException& e) {
          std::cerr << "Error parsing library file: \n" << e.what() << std::endl;
        }
      }
        break;

      default:
        reportInterfaceError("IpcAdapter::enqueueMessage: invalid message type " << msgData->msgType);
        break;
      }
    }

    /**
     * @brief Process a PlexilMsgType_Message packet and free the message
     */
    void handleMessageMessage(const PlexilStringValueMsg* msgData) {
      assertTrueMsg(msgData != NULL,
                    "IpcAdapter::handleMessageMessage: msgData is null")
        assertTrueMsg(msgData->stringValue != NULL,
                      "IpcAdapter::handleMessageMessage: stringValue is null")
        m_messageQueues.addMessage(msgData->stringValue);
    }

    /**
     * @brief Queues the command in the message queue
     */
    void handleCommandSequence(const std::vector<const PlexilMsgBase*>& msgs) {
      //only support one parameter, the id
      //TODO: support more parameters
      const PlexilStringValueMsg* header = reinterpret_cast<const PlexilStringValueMsg*>(msgs[0]);
      std::ostringstream uid;
      uid << ((int) header->header.serial) << SERIAL_UID_SEPERATOR << header->header.senderUID;
      std::string uid_lbl(uid.str());
      debugMsg("IpcAdapter:handleCommandSequence",
               " adding \"" << header->stringValue << "\" to the command queue");
      const std::string& msg(formatMessageName(header->stringValue, RECEIVE_COMMAND_COMMAND));
      m_messageQueues.addMessage(msg, uid_lbl);
      int i = 0;
      for (std::vector<const PlexilMsgBase*>::const_iterator it = ++msgs.begin(); it != msgs.end(); ++it, ++i) {
        std::string const paramLbl(formatMessageName(uid_lbl, GET_PARAMETER_COMMAND, i));
        m_messageQueues.addMessage(paramLbl, getPlexilMsgValue(*it));
      }
    }

    /**
     * @brief Process a TelemetryValues message sequence
     */

    void handleTelemetryValuesSequence(const std::vector<const PlexilMsgBase*>& msgs) 
    {
      const PlexilStringValueMsg* tv = reinterpret_cast<const PlexilStringValueMsg*>(msgs[0]);
      char const *stateName = tv->stringValue;
      if (!stateName || !*stateName) {
        debugMsg("IpcAdapter:handleTelemetryValuesSequence",
                 " state name missing or empty, ignoring");
        return;
      }

      size_t nValues = msgs[0]->count;
      checkError(nValues > 0,
                 "Telemetry values message requires at least 1 value");
      checkError(nValues + 1 == msgs.size(),
                 "handleTelemetryValuesSequence: Expecting " << nValues + 1 << " messages, got " << msgs.size());

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
        ThreadMutexGuard g(m_cmdMutex);
        if (m_pendingLookupState == state) {
          m_pendingLookupResult = result;
          m_lookupSem.post();
          return;
        }
      }
      // Otherwise process normally
      m_execInterface.handleValueChange(state, result);
      m_execInterface.notifyOfExternalEvent();
    }

    /**
     * @brief Process a ReturnValues message sequence
     */

    void handleReturnValuesSequence(const std::vector<const PlexilMsgBase*>& msgs) 
    {
      const PlexilReturnValuesMsg* rv = (const PlexilReturnValuesMsg*) msgs[0];
      //lock mutex to ensure all sending procedures are complete.
      ThreadMutexGuard guard(m_cmdMutex);
      if (rv->requestSerial == m_pendingLookupSerial) {
        // LookupNow for which we are awaiting data
        debugMsg("IpcAdapter:handleReturnValuesSequence",
                 " processing value(s) for a pending LookupNow");
        m_pendingLookupResult = parseReturnValue(msgs);
        // *** TODO: check for error
        m_lookupSem.post();
        return;
      }

      PendingCommandsMap::iterator cit = m_pendingCommands.find(rv->requestSerial);
      if (cit != m_pendingCommands.end()) {
        Command *cmd = cit->second;
        assertTrue_1(cmd);
        size_t nValues = msgs[0]->count;
        if (msgs[1]->count == MSG_COUNT_CMD_ACK) {
          assertTrueMsg(nValues == 1,
                        "IpcAdapter::handleReturnValuesSequence: command ack requires 1 value, received "
                        << nValues);
          if (!cmd->isActive()) {
            debugMsg("IpcAdapter:handleReturnValuesSequence",
                     " ignoring command handle value for inactive command");
            m_pendingCommands.erase(rv->requestSerial);
            return;
          }
          debugMsg("IpcAdapter:handleReturnValuesSequence",
                   " processing command acknowledgment for command " << cmd->getName());
          Value ack = parseReturnValue(msgs);
          assertTrue_2(ack.isKnown() && ack.valueType() == COMMAND_HANDLE_TYPE,
                       "IpcAdapter:handleReturnValuesSequence received a command acknowledgment which is not a CommandHandle value");
          uint16_t handle;
          ack.getValue(handle);
          m_execInterface.handleCommandAck(cmd, (CommandHandleValue) handle);
          m_execInterface.notifyOfExternalEvent();
        }
        else {
          if (!cmd->isActive()) {
            debugMsg("IpcAdapter:handleReturnValuesSequence",
                     " ignoring return value for inactive command");
            m_pendingCommands.erase(rv->requestSerial);
            return;
          }
          debugMsg("IpcAdapter:handleReturnValuesSequence",
                   " processing command return value for command " << cmd->getName());
          m_execInterface.handleCommandReturn(cmd, parseReturnValue(msgs));
          m_execInterface.notifyOfExternalEvent();
        }
      }
      else {
        debugMsg("IpcAdapter:handleReturnValuesSequence",
                 " no lookup or command found for sequence");
      }
    }

    /**
     * @brief Process a LookupNow message.
     * @note Ignores any lookups that are not defined in config or published.
     */
    void handleLookupNow(const std::vector<const PlexilMsgBase*>& msgs) 
    {
      debugMsg("IpcAdapter:handleLookupNow", " received LookupNow");
      const PlexilStringValueMsg* msg = reinterpret_cast<const PlexilStringValueMsg*> (msgs.front());
      std::string name(msg->stringValue);
      size_t nParms = msgs.front()->count;
      State lookup(name, nParms);
      for (size_t i = 1 ; i <= nParms ; ++i)
        lookup.setParameter(i - 1, getPlexilMsgValue(msgs[i]));

      ThreadMutexGuard guard(m_cmdMutex);
      
      ExternalLookupMap::iterator it = m_externalLookups.find(lookup);
      if (it != m_externalLookups.end()) {
        debugMsg("IpcAdapter:handleLookupNow", " Publishing value of external lookup \"" << lookup
                 << "\" with internal value '" << (*it).second << "'");
        m_ipcFacade.publishReturnValues(msg->header.serial, msg->header.senderUID, (*it).second);
        return;
      }

      if (nParms > 0) {
        // See if name is declared, and return default value
        lookup.setParameterCount(0);
        it = m_externalLookups.find(lookup);
        if (it != m_externalLookups.end()) {
          debugMsg("IpcAdapter:handleLookupNow", " Publishing default value of external lookup \"" << name
                   << "\" = '" << (*it).second << "'");
          m_ipcFacade.publishReturnValues(msg->header.serial, msg->header.senderUID, (*it).second);
          return;
        }      
      }

      // Ignore silently if we're not declared to handle it
      // Tracing here for debugging purposes
      debugMsg("IpcAdapter:handleLookupNow",
               " undeclared external lookup \"" << name << "\", ignoring");
    }

  };

}

extern "C"
void initIpcAdapter()
{
  REGISTER_ADAPTER(PLEXIL::IpcAdapter, "IpcAdapter")
}
