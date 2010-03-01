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
#include "MessageQueueMap.h"
#include <ipc.h>

// Forward declarations outside of namespace
struct PlexilMsgBase;
struct PlexilStringValueMsg;
class TiXmlElement;

#define TRANSACTION_ID_SEPARATOR_CHAR ':'

namespace PLEXIL {
class IpcAdapter: public InterfaceAdapter {
public:

  //
  // Static class constants
  //

  DECLARE_STATIC_CLASS_CONST(std::string, COMMAND_PREFIX, "COMMAND__")
  DECLARE_STATIC_CLASS_CONST(std::string, MESSAGE_PREFIX, "MESSAGE__")
  DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_PREFIX, "LOOKUP__")
  DECLARE_STATIC_CLASS_CONST(std::string, LOOKUP_ON_CHANGE_PREFIX, "LOOKUP_ON_CHANGE__")
  DECLARE_STATIC_CLASS_CONST(std::string, SERIAL_UID_SEPERATOR, ":")

  DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_MESSAGE_COMMAND, "SendMessage")
  DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_MESSAGE_COMMAND, "ReceiveMessage")
  DECLARE_STATIC_CLASS_CONST(LabelStr, RECEIVE_COMMAND_COMMAND, "ReceiveCommand")
  DECLARE_STATIC_CLASS_CONST(LabelStr, SEND_RETURN_VALUE_COMMAND, "SendReturnValue")

  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  IpcAdapter(AdapterExecInterface& execInterface);

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const pointer to the TiXmlElement describing this adapter
   * @note The instance maintains a shared pointer to the TiXmlElement.
   */
  IpcAdapter(AdapterExecInterface& execInterface, const TiXmlElement * xml);

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
   * @brief Register one LookupOnChange.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param tolerances A vector of tolerances for the LookupOnChange.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void registerChangeLookup(const LookupKey& uniqueId, const StateKey& stateKey, const std::vector<double>& tolerances);

  /**
   * @brief Terminate one LookupOnChange.
   * @param uniqueId The unique ID of the lookup to be terminated.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void unregisterChangeLookup(const LookupKey& uniqueId);

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param stateKey The state key for this lookup.
   * @param dest A (reference to a) vector of doubles where the result is to be stored.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void lookupNow(const StateKey& stateKey, std::vector<double>& dest);

  /**
   * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
   * @param node The Node requesting the update.
   * @param valuePairs A map of <LabelStr key, value> pairs.
   * @param ack The expression in which to store an acknowledgement of completion.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack);

  /**
   * @brief Execute a command with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the command.
   * @param ack The expression in which to store an acknowledgement of command transmission.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The destination for the command to abort
   * @param ack The expression in which to store an acknowledgement of command abort.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  virtual void invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

private:

  // Deliberately unimplemented
  IpcAdapter();
  IpcAdapter(const IpcAdapter &);
  IpcAdapter & operator=(const IpcAdapter &);

  //
  // Implementation methods
  //

  /**
   * @brief Initialize unique ID string
   */
  void initializeUID();

  /**
   * @brief Handler function as seen by IPC.
   */

  static void messageHandler(MSG_INSTANCE rawMsg, void * unmarshalledMsg, void * this_as_void_ptr);

  /**
   * @brief Handler function as seen by adapter.
   */

  void handleIpcMessage(const PlexilMsgBase * msgData);

  //
  // Helper methods
  //

  /**
   * @brief Cache start message of a multi-message sequence
   */

  void cacheMessageLeader(const PlexilMsgBase* msgData);

  /**
   * @brief Cache following message of a multi-message sequence
   */

  void cacheMessageTrailer(const PlexilMsgBase* msgData);

  /**
   * @brief Send a single message to the Exec's queue and free the message
   */
  void enqueueMessage(const PlexilMsgBase* msgData);

  /**
   * @brief Process a PlexilMsgType_Message packet and free the message
   */
  void handleMessageMessage(const PlexilStringValueMsg* msgData);

  /**
   * @brief Send a message sequence to the Exec's queue and free the messages
   */
  void enqueueMessageSequence(std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Queues the command in the message queue
   */
  void handleCommandSequence(std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Process a TelemetryValues message sequence
   */
  void handleTelemetryValuesSequence(std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Process a ReturnValues message sequence
   */
  void handleReturnValuesSequence(std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Helper function for sending a vector of parameters via IPC.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   */
  void sendParameters(const std::list<double>& args, uint32_t serial);

  /**
   * @brief Helper function for converting message names into the propper format given the command type.
   */
  double formatMessageName(const LabelStr& name, const LabelStr& command);

  /**
   * @brief Helper function for converting message names into the propper format given the command type.
   */
  double formatMessageName(const char* name, const LabelStr& command);

  /**
   * @brief Get next serial number
   */
  uint32_t getSerialNumber();

  //
  // Static member functions
  //

  /**
   * @brief Returns true if the string starts with the prefix, false otherwise.
   */
  static bool hasPrefix(const std::string& s, const std::string& prefix);

  /**
   * @brief Generate a string combining the given UID and serial
   */
  static std::string makeTransactionID(const std::string& uid, uint32_t serial);

  /**
   * @brief Given a transaction ID string, return the UID and the serial
   */
  static void parseTransactionId(const std::string& transId, std::string& uidOut, uint32_t& serialOut);

  /**
   * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
   */
  static double parseReturnValues(std::vector<const PlexilMsgBase*>& msgs);

  //
  // Private data types
  //

  //* brief Unique identifier of a message sequence
  typedef std::pair<std::string, uint32_t> IpcMessageId;

  //* brief Associates serial numbers with active LookupOnChange instances
  typedef std::map<uint32_t, StateKey> IpcChangeLookupMap;

  //* brief Cache of not-yet-complete message sequences
  typedef std::map<IpcMessageId, std::vector<const PlexilMsgBase*> > IncompleteMessageMap;

  //* brief Cache of message/command/lookup names we're actively listening for
  typedef std::map<std::string, StateKey> ActiveListenerMap;

  //* brief Cache of command serials and their corresponding ack and return value variables
  typedef std::map<uint32_t, std::pair<ExpressionId, ExpressionId> > PendingCommandsMap;

  //
  // Member variables
  //

  //* @brief Map of queues for holding complete messages and message handlers while they wait to be paired
  MessageQueueMap m_messageQueues;

  //* @brief Cache of active outgoing LookupOnChange instances
  IpcChangeLookupMap m_changeLookups;

  //* @brief Cache of incomplete received message data
  IncompleteMessageMap m_incompletes;

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

  //* @brief Unique ID of this adapter instance
  std::string m_myUID;

  //* @brief Thread ID of IPC dispatch thread
  pthread_t m_thread;

  //* @brief Semaphore for return values from LookupNow
  ThreadSemaphore m_lookupSem;

  //* @brief Count of # of outgoing commands and requests, starting with 1
  //  @note Should only ever be 0 at initialization
  uint32_t m_serial;

  //* @brief Serial # of current pending LookupNow request, or 0
  uint32_t m_pendingLookupSerial;

  //* @brief Pointer to destination of current pending LookupNow request, or NULL
  std::vector<double>* m_pendingLookupDestination;
};
}
