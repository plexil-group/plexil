/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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

#include "InterfaceAdapter.hh"
#include "LabelStr.hh"
#include "ThreadSemaphore.hh"
#include "MessageQueueMap.hh"
#include "IpcFacade.hh"
#include <ipc.h>
#include <vector>
#include <list>

// Forward declarations outside of namespace
struct PlexilMsgBase;
struct PlexilStringValueMsg;

#define TRANSACTION_ID_SEPARATOR_CHAR ':'

namespace PLEXIL {
class IpcAdapter: public InterfaceAdapter {
public:

  //
  // Static class constants
  //

  DECLARE_STATIC_CLASS_CONST(std::string, COMMAND_PREFIX, "__COMMAND__")
  DECLARE_STATIC_CLASS_CONST(std::string, MESSAGE_PREFIX, "__MESSAGE__")
  DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_PREFIX, "__LOOKUP__")
  DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_ON_CHANGE_PREFIX, "__LOOKUP_ON_CHANGE__")
  DECLARE_STATIC_CLASS_CONST(std::string, PARAM_PREFIX, "__PARAMETER__")
  DECLARE_STATIC_CLASS_CONST(std::string, SERIAL_UID_SEPERATOR, ":")

  DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_MESSAGE_COMMAND, "SendMessage")
  DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_MESSAGE_COMMAND, "ReceiveMessage")
  DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_COMMAND_COMMAND, "ReceiveCommand")
  DECLARE_STATIC_CLASS_CONST(LabelStr, GET_PARAMETER_COMMAND, "GetParameter")
  DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_RETURN_VALUE_COMMAND, "SendReturnValue")
  DECLARE_STATIC_CLASS_CONST(LabelStr, UPDATE_LOOKUP_COMMAND, "UpdateLookup")


  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  IpcAdapter(AdapterExecInterface& execInterface);

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const reference to the XML element describing this adapter
   * @note The instance maintains a shared pointer to the XML.
   */
  IpcAdapter(AdapterExecInterface& execInterface, const pugi::xml_node& xml);

  /**
   * @brief Destructor.
   */
  virtual ~IpcAdapter();

  //
  // API to ExecApplication
  //

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  virtual bool initialize();

  /**
   * @brief Starts the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  virtual bool start();

  /**
   * @brief Stops the adapter.
   * @return true if successful, false otherwise.
   */
  virtual bool stop();

  /**
   * @brief Resets the adapter.
   * @return true if successful, false otherwise.
   * @note Adapters should provide their own methods.  The default method simply returns true.
   */
  virtual bool reset();

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   * @note Adapters should provide their own methods.  The default method simply returns true.
   */
  virtual bool shutdown();

  /**
   * @brief Perform an immediate lookup on an existing state.
   * @param state The state.
   * @return The current value for the state.
   */
  virtual double lookupNow(const State& state);

  /**
   * @brief Inform the interface that it should report changes in value of this state.
   * @param state The state.
   */
  virtual void subscribe(const State& state);

  /**
   * @brief Inform the interface that a lookup should no longer receive updates.
   * @param state The state.
   */
  virtual void unsubscribe(const State& state);

  /**
   * @brief Advise the interface of the current thresholds to use when reporting this state.
   * @param state The state.
   * @param hi The upper threshold, at or above which to report changes.
   * @param lo The lower threshold, at or below which to report changes.
   */
  virtual void setThresholds(const State& state, double hi, double lo);

  /**
   * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
   * @param node The Node requesting the update.
   * @param valuePairs A map of <LabelStr key, value> pairs.
   * @param ack The expression in which to store an acknowledgement of completion.
   */

  virtual void sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack);

  /**
   * @brief Execute a command with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the command.
   * @param ack The expression in which to store an acknowledgement of command transmission.
   */

  virtual void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param cmdName The LabelString representing the command name.
   * @param cmdArgs The command arguments expressed as doubles.
   * @param cmdAck The acknowledgment of the pending command
   * @param abrtAck The expression in which to store an acknowledgment of command abort.
   */

  virtual void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck);

private:

  // Deliberately unimplemented
  IpcAdapter();
  IpcAdapter(const IpcAdapter &);
  IpcAdapter & operator=(const IpcAdapter &);

  //
  // Implementation methods
  //

  /**
   * @brief handles SEND_MESSAGE_COMMAND commands from the exec
   */
  void executeSendMessageCommand(const std::list<double>& args,
								 ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
   */
  void executeSendReturnValueCommand(const std::list<double>& args,
									 ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
   */
  void executeReceiveMessageCommand(const std::list<double>& args,
									ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
   */
  void executeReceiveCommandCommand(const std::list<double>& args,
									ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles GET_PARAMETER_COMMAND commands from the exec
   */
  void executeGetParameterCommand(const std::list<double>& args,
								  ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles UPDATE_LOOKUP_COMMAND commands from the exec
   */
  void executeUpdateLookupCommand(const std::list<double>& args,
								  ExpressionId dest, ExpressionId ack);

  /**
   * @brief handles all other commands from the exec
   */
  void executeDefaultCommand(const LabelStr& name, const std::list<double>& args,
							 ExpressionId dest, ExpressionId ack);

  /**
   * @brief Parses external lookups from xml and puts them in the lookup map.
   * If external is NULL, does nothing.
   */
  void parseExternalLookups(const pugi::xml_node& external);

  /**
   * @brief Handler function as seen by adapter.
   */

  void handleIpcMessage(const std::vector<PlexilMsgBase *> msgData);

  //
  // Helper methods
  //
  /**
   * @brief Send a single message to the Exec's queue and free the message
   */
  void enqueueMessage(const PlexilMsgBase* msgData);

  /**
   * @brief Process a PlexilMsgType_Message packet and free the message
   */
  void handleMessageMessage(const PlexilStringValueMsg* msgData);

  /**
   * @brief Queues the command in the message queue
   */
  void handleCommandSequence(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Process a TelemetryValues message sequence
   */
  void handleTelemetryValuesSequence(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Process a ReturnValues message sequence
   */
  void handleReturnValuesSequence(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Process a LookupNow. Ignores any lookups that are not defined in config
   */
  void handleLookupNow(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Helper function for converting message names into the proper format given the command type and a user-defined id.
   */
  double formatMessageName(const LabelStr& name, const LabelStr& command, int id);

  /**
   * @brief Helper function for converting message names into the propper format given the command type.
   */
  double formatMessageName(const LabelStr& name, const LabelStr& command);

  /**
   * @brief Helper function for converting message names into the propper format given the command type.
   */
  double formatMessageName(const char* name, const LabelStr& command);

  //
  // Static member functions
  //

  /**
   * @brief Returns true if the string starts with the prefix, false otherwise.
   */
  static bool hasPrefix(const std::string& s, const std::string& prefix);

  /**
   * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
   */
  static double parseReturnValue(const std::vector<const PlexilMsgBase*>& msgs);

  //
  // Private data types
  //

  //* brief Associates serial numbers with active LookupOnChange instances
  typedef std::map<uint32_t, State> IpcChangeLookupMap;

  //* brief Cache of message/command/lookup names we're actively listening for
  typedef std::map<std::string, State> ActiveListenerMap;

  //* brief Cache of command serials and their corresponding ack and return value variables
  typedef std::map<uint32_t, std::pair<ExpressionId, ExpressionId> > PendingCommandsMap;

  typedef std::map<double, double> ExternalLookupMap;

  //* brief Class to receive messages from Ipc
  class MessageListener : public IpcMessageListener {
  public:
    MessageListener(IpcAdapter&);
    ~MessageListener();
    void ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs);
  private:
    IpcAdapter& m_adapter;
  };

  //
  // Member variables
  //

  //* @brief Interface with IPC
  IpcFacade m_ipcFacade;

  //* @brief Map of queues for holding complete messages and message handlers while they wait to be paired
  MessageQueueMap m_messageQueues;

  //* @brief Cache of active outgoing LookupOnChange instances
  IpcChangeLookupMap m_changeLookups;

  //* @brief Cache of open LookupOnChange instances for messages
  ActiveListenerMap m_activeMessageListeners;

  //* @brief Cache of open LookupOnChange instances for commands
  ActiveListenerMap m_activeCommandListeners;

  //* @brief Cache of open LookupOnChange instances for LookupNow
  ActiveListenerMap m_activeLookupListeners;

  //* @brief Cache of open LookupOnChange instances for LookupOnChange
  ActiveListenerMap m_activeChangeLookupListeners;

  //* @brief Cache of ack and return value variables for commands we sent
  PendingCommandsMap m_pendingCommands;

  //* @brief Semaphore for return values from LookupNow
  ThreadSemaphore m_lookupSem;

  //* @brief Listener instance to receive messages.
  MessageListener m_listener;

  /**
   * @brief Mutex used to hold the processing of incoming return values while commands
   * are being sent and recorded.
   */
  ThreadMutex m_cmdMutex;

  //* @brief Serial # of current pending LookupNow request, or 0
  uint32_t m_pendingLookupSerial;

  //* @brief Place to store result of current pending LookupNow request
  double m_pendingLookupResult;

  //* @brief Map of external lookup values.
  ExternalLookupMap m_externalLookups;
};
}
