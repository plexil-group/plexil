/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#ifndef DEFINE_IPC_FACADE_H
#define DEFINE_IPC_FACADE_H

#include "ipc.h"
#include "ipc-data-formats.h"
#include "ThreadSpawn.hh"
#include "RecursiveThreadMutex.hh"
#include "CommonDefs.hh"
#include "LabelStr.hh"

#include <vector>
#include <list>
#include <string>
#include <map>

namespace PLEXIL {

class IpcMessageListener;

/**
 * @brief Manages connection with IPC. This class is not thread-safe.
 */
//TODO: Integrate all plexil type converting into this class.
class IpcFacade {
public:
  DECLARE_STATIC_CLASS_CONST(int, ERROR_SERIAL, -1)

  IpcFacade();
  ~IpcFacade();

  /**
   * @brief Returns the unique id of the IPC connection. This method will always return
   * a valid string, and it will never change, regardless of the state changes of the
   * connection.
   */
  static const std::string& getUID();
  /**
   * Returns a formatted message type string given the basic message type and destination ID.
   * @param msgName The name of the message type
   * @param destId The destination ID for the message
   */
  static std::string formatMsgName(const std::string& msgName, const std::string& destId);
  /**
   * @breif Connects to the Ipc server. This should be called before calling start().
   * If it is not, this method is called by start. If already initilized, this method
   * does nothing and returns IPC_OK.
   */
  IPC_RETURN_TYPE initilize(const char* taskName, const char* serverName);
  /**
   * @brief Starts the Ipc message handling thread. If not initilized, initilization occurs.
   * If Ipc is already started, this method does nothing and returns IPC_OK.
   */
  IPC_RETURN_TYPE start();
  /**
   * @brief Stops the Ipc message handling thread and removes all subscriptions. If
   * Ipc is not running, this method does nothing and returns IPC_OK.
   */
  void stop();
  /**
   * @brief Disconnects from the Ipc server. This puts Ipc back in its initial state before
   * being initilized. If not initilized, this method does nothing.
   */
  void shutdown();

  /**
   * @brief Adds the IpcMessageListener as a subscriber to all plexil
   * message types
   * @param handler The handler function to call for each message
   * @param clientData The object to associate as a subscriber
   */
  void subscribeAll(IpcMessageListener* listener);

  /**
   * @brief Adds the given IpcMessageListener as a subscriber to the given
   * PLEXIL message type
   * @param handler The handler function to call for each message
   * @param clientData The object to associate as a subscriber
   */
  void subscribe(IpcMessageListener* listener, PlexilMsgType type);

  /**
   * @brief Removes all registered IpcMessageListeners to all plexil message subscriptions
   */
  void unsubscribeAll();

  /**
   * @brief Removes the given IpcMessageListener to all plexil message subscriptions
   * @param handler The handler function to be unsubscribed
   */
  void unsubscribeAll(IpcMessageListener* handler);

  /**
   * @brief publishes the given message via IPC
   * @param command The command string to send
   */
  uint32_t publishMessage(LabelStr command);

  /**
   * @brief publishes the given command via IPC. This is equivalent to calling
   * sendCommand with an empty destination string.
   * Note: The response to this command may be received before
   * this method returns.
   * @param command The command string to send
   */
  uint32_t publishCommand(LabelStr command, const std::list<double>& argsToDeliver);

  /**
   * @brief Sends the given command to the given client ID via IPC. If the client ID is
   * an empty string, the command is published to all clients.
   * Note: The response to this command may be received before
   * this method returns.
   * @param command The command string to send
   * @param dest The destination ID for this command
   */
  uint32_t sendCommand(LabelStr command, LabelStr dest, const std::list<double>& argsToDeliver);

  /**
   * @brief publishes the given LookupNow via IPC
   * @param command The command string to send
   */
  uint32_t publishLookupNow(LabelStr lookup, const std::list<double>& argsToDeliver);


  /**
   * @brief Sends the given LookupNow to the given client ID via IPC. If the client ID is
   * an empty string, the LookupNow is published to all clients.
   * @param lookup The lookup string to send
   * @param dest The destination ID for this LookupNow
   */
  uint32_t sendLookupNow(LabelStr lookup, LabelStr dest, const std::list<double>& argsToDeliver);

  /**
   * @brief publishes the given return values via IPC
   * @param command The command string to send
   */
  uint32_t publishReturnValues(uint32_t request_serial, LabelStr command, double arg);

  /**
   * @brief publishes the given telemetry value via IPC
   * @param command The command string to send
   */
  uint32_t publishTelemetry(const std::string& destName, const std::list<double>& values);
  /**
   * @brief Returns the error code of the last publish method call. If the last publish call returned
   * -1, this will return the appropriate error. Otherwise, it will return IPC_OK.
   */
  IPC_RETURN_TYPE getError();
private:
  //* brief Structure for holding references to listeners registered through local instances
  typedef std::pair<int, IpcMessageListener*> LocalListenerRef;
  //* brief List of listeners registered through local instances
  typedef std::list<LocalListenerRef> LocalListenerList;
  //* brief List of listeners registered globally - for ListenerMap
  typedef std::list<IpcMessageListener*> ListenerList;
  //* brief Map of message types to lists of registered listeners
  typedef std::map<int, ListenerList> ListenerMap;

  //* brief Unique identifier of a message sequence
  typedef std::pair<std::string, uint32_t> IpcMessageId;
  //* brief Cache of not-yet-complete message sequences
  typedef std::map<IpcMessageId, std::vector<const PlexilMsgBase*> > IncompleteMessageMap;

  //* brief basic types of items to send
  enum BasicType { UNKNOWN, STRING, NUMERIC };

  /**
   * @brief Initialize unique ID string
   */
  static std::string& generateUID();

  /**
   * @brief Get next serial number
   */
  static uint32_t getSerialNumber();

  /**
   * @brief Handler function as seen by IPC.
   */
  static void messageHandler(MSG_INSTANCE rawMsg, void * unmarshalledMsg, void * this_as_void_ptr);

  /**
   * @brief Cache start message of a multi-message sequence
   */

  static void cacheMessageLeader(const PlexilMsgBase* msgData);

  /**
   * @brief Cache following message of a multi-message sequence
   */

  static void cacheMessageTrailer(const PlexilMsgBase* msgData);

  /**
   * @brief Deliver the given message to all listeners registered for it
   */
  static void deliverMessage(const std::vector<const PlexilMsgBase*>& msgs);

  /**
   * @brief Helper function for sending a vector of parameters via IPC.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   */
  IPC_RETURN_TYPE sendParameters(const std::list<double>& args, uint32_t serial);

  /**
   * @brief Helper function for sending a vector of parameters via IPC to a specific executive.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   * @param dest The destination executive name
   */
  IPC_RETURN_TYPE sendParameters(const std::list<double>& args, uint32_t serial, const LabelStr& dest);

  /**
   * @brief Define all PLEXIL message types with Central. Also defines each PLEXIL message type with
   * the UID as a prefix for directed communication. Has no effect for any previously defined message types.
   * @return true if successful, false otherwise
   * @note Caller should ensure IPC_initialize() has been called first
  */
  bool definePlexilIPCMessageTypes();

  /**
   * @brief Set the error code of the last called IPC method.
   * @param error The error code of the last called IPC method.
   */
  void setError(IPC_RETURN_TYPE error);
  /**
   * @brief Registers the subscription of the given msg type/listener
   */
  void subscribeGlobal(const LocalListenerRef& listener);
  /**
   * @brief Unsubscribe the given listener from the static listener map.
   * @return True if found and unsubscribed. False if not found.
   */
  bool unsubscribeGlobal(const LocalListenerRef& listener);

  /**
   * @brief Given a transaction ID string, return the UID and the serial
   */
  static void parseTransactionId(const std::string& transId, std::string& uidOut, uint32_t& serialOut);

  /**
   * @brief Determines the type of the given StoredArray by iterating over the elements, determining the type of each one until
   * an item is not UNKNOWN.
   *
   * @return STRING if LabelStr::isString() returns true for the first non-unknown value, NUMERIC if LabelStr::isString()
   * returns false for the first non-unknown value, and UNKNOWN if all items are UNKNOWN.
   */
  static BasicType determineType(double array_id);

  /**
   * Unsubscribes from the given message on central. Wrapper for IPC_unsubscribe
   * @param msgName the name of the message to unsubscribe from
   * @param handler The handler to unsubscribe.
   */
  static IPC_RETURN_TYPE unsubscribeCentral (const char *msgName, HANDLER_TYPE handler);

  /**
   * Subscribes to the given message on central. Wrapper for IPC_subscribeData
   * @param msgName the name of the message to unsubscribe from
   * @param handler The handler to unsubscribe.
   */
  static IPC_RETURN_TYPE subscribeDataCentral (const char *msgName,
                     HANDLER_DATA_TYPE handler,
                     void *clientData);

  //* @brief Is the facade initilized?
  bool m_isInitilized;
  //* @brief Is the facade started?
  bool m_isStarted;
  //* @brief The error code of the last called IPC method.
  IPC_RETURN_TYPE m_error;
  //* @brief The listeners registered under this facade
  LocalListenerList m_localRegisteredHandlers;

  //* @brief The key for listeners on all messages
  static const int ALL_MSG_TYPE;
  //* @brief The handle for the message thread
  static pthread_t threadHandle;
  //* @brief The map of message type to list of listeners for that type
  static ListenerMap registeredListeners;
  //* @brief The number of initilized instances. Used for managing the connection
  static int numInitilized;
  //* @brief The number of started instances. Used for managing the message thread
  static int numStarted;
  //* @brief Cache of incomplete received message data
  static IncompleteMessageMap incompletes;
  //* @brief The mutex used for synchronizing initilization/shutdown methods
  static RecursiveThreadMutex mutex;
  //* @brief Unique ID of this adapter instance
  static std::string& MY_UID;

  //* @brief Count of # of outgoing commands and requests, starting with 1
  //  @note Should only ever be 0 at initialization
  static uint32_t nextSerial;
};

/**
 * @brief Base class for receiving messages through Ipc. To use, create an instance of
 * IpcFacade, initilize and start it, and register an instance of the listener as a
 * recipient for the plexil message type you wish to handle.
 */
class IpcMessageListener {
public:
  virtual ~IpcMessageListener() {};
  virtual void ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs) = 0;
};

/**
 * @brief Bounds check the supplied message type.
 * @param mtyp The message type value to check.
 * @return true if valid, false if not.
 */
inline bool msgTypeIsValid(const PlexilMsgType mtyp)
{
  return (mtyp > PlexilMsgType_uninited) && (mtyp < PlexilMsgType_limit);
}

/**
 * @brief Return the message format string corresponding to the message type.
 * @param typ The message type.
 * @return Const char pointer to the message format name.
 */
inline const char* msgFormatForType(const PlexilMsgType typ)
{
  switch (typ)
    {
    case PlexilMsgType_NotifyExec:
    case PlexilMsgType_TerminateChangeLookup:

      return MSG_BASE;
      break;

    case PlexilMsgType_AddPlan:
    case PlexilMsgType_AddPlanFile:
    case PlexilMsgType_AddLibrary:
    case PlexilMsgType_AddLibraryFile:
    case PlexilMsgType_Command:
    case PlexilMsgType_Message:
    case PlexilMsgType_LookupNow:
    case PlexilMsgType_LookupOnChange:
    case PlexilMsgType_PlannerUpdate:
    case PlexilMsgType_StringValue:
    case PlexilMsgType_TelemetryValues:

      return STRING_VALUE_MSG;
      break;

    case PlexilMsgType_ReturnValues:

      return RETURN_VALUE_MSG;
      break;

    case PlexilMsgType_NumericValue:

      return NUMERIC_VALUE_MSG;
      break;

    case PlexilMsgType_PairNumeric:
      
      return NUMERIC_PAIR_MSG;
      break;

    case PlexilMsgType_PairString:

      return STRING_PAIR_MSG;
      break;
			  
    case PlexilMsgType_NumericArray:
      return NUMERIC_ARRAY_MSG;
      break;

    case PlexilMsgType_StringArray:
      return STRING_ARRAY_MSG;
      break;

    default:

      return NULL;
      break;
    }
}
}

#endif /* DEFINE_IPC_FACADE_H */
