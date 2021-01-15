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

#ifndef DEFINE_IPC_FACADE_H
#define DEFINE_IPC_FACADE_H

#include "ipc.h"
#include "ipc-data-formats.h"

#include "ConstantMacros.hh"
#include "Value.hh"

#include <limits>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <vector>


namespace PLEXIL {

  //! Return type from many of the IpcFacade member functions
  using IpcSerialNumber = uint32_t;

  /**
   * @brief Base class for receiving messages through Ipc. To use, create an instance of
   * IpcFacade, initialize and start it, and register an instance of the listener as a
   * recipient for the plexil message type you wish to handle.
   */
  class IpcMessageListener {
  public:
    virtual ~IpcMessageListener() = default;
    virtual void ReceiveMessage(const std::vector<PlexilMsgBase*>& msgs) = 0;
  };

  /**
   * @brief Manages connection with IPC. This class is not thread-safe.
   */
  //TODO: Integrate all plexil type converting into this class.
  class IpcFacade
  {
  public:

    //
    // Class constants
    //

    static constexpr IpcSerialNumber const ERROR_SERIAL =
      std::numeric_limits<IpcSerialNumber>::max();

    //! Default constructor.
    IpcFacade();

    //! Destructor.
    ~IpcFacade();

    //! Get the identifier of the IPC connection.
    //! @return The identifier.
    //! @note Can be set via the taskName parameter to initialize().
    const std::string &getUID();

    //! Connects to the IPC server.
    //! @param taskName Name to be used as an identifer for this
    //!                 instance. If null, defaults to a randomly
    //!                 generated UID.
    //! @param serverName The server host to connect to.
    //! @note If already initialized, this method does nothing and
    //!       returns IPC_OK.
    IPC_RETURN_TYPE initialize(const char* taskName, const char* serverName);

    /**
     * @brief Starts the Ipc message handling thread. If not initialized, initialization occurs.
     * If Ipc is already started, this method does nothing and returns IPC_OK.
     */
    IPC_RETURN_TYPE start();

    /**
     * @brief Stops the Ipc message handling thread, removes all
     * subscriptions, and disconnects from central. If Ipc is not
     * running, this method does nothing.
     */
    void stop();

    //! Subscribe this listener for all PLEXIL message types.
    //! @param listener The listener.
    void subscribeAll(IpcMessageListener* listener);

    //! Register the listener for the specific message type
    //! @param listener The listener to call for each message of the type.
    //! @param msgType The message type for which the listener should be called.
    void subscribe(IpcMessageListener* listener, PlexilMsgType msgType);

    //! Unsubscribe the given listener from all messages to which it is subscribed.
    //! @param listener The listener.
    void unsubscribe(IpcMessageListener* listener);

    //! Publishes the given command.
    //! @param command The command string to send
    //! @return Serial number generated for the command message.
    IpcSerialNumber publishMessage(std::string const &command);

    //! Publishes the given command with the given parameters.
    //! command The command name.
    //! @param argsToDeliver The parameters for the command.
    //! @return Serial number generated for the command message.
    //! @note The response to this command may be received before
    //! this method returns.
    IpcSerialNumber publishCommand(std::string const &command,
                                   std::vector<Value> const &argsToDeliver);

    //! Sends the command to the specific client ID.
    //! @param command The command name.
    //! @param dest The destination ID for this command.
    //! @param argsToDeliver The parameters for the command.
    //! @return Serial number generated for the command message.
    //! @note If the client ID is an empty string, the command is
    //! published to all clients.
    //! @note The response to this command may be received before this
    //! method returns.
    IpcSerialNumber sendCommand(std::string const &command,
                                std::string const &dest, 
                                std::vector<Value> const &argsToDeliver);

    /**
     * @brief publishes the given LookupNow call via IPC
     * @param lookup The state name
     * @param argsToDeliver Vector of state parameters.
     * @return IPC status
     */
    IpcSerialNumber publishLookupNow(std::string const &lookup, 
                                     std::vector<Value> const &argsToDeliver);

    /**
     * @brief Sends the given LookupNow to the given client ID via IPC. If the client ID is
     * an empty string, the LookupNow is published to all clients.
     * @param lookup The state name.
     * @param dest The destination ID for this LookupNow
     * @param argsToDeliver Vector of state parameters.
     * @return IPC status
     */
    IpcSerialNumber sendLookupNow(std::string const &lookup,
                                  std::string const &dest,
                                  std::vector<Value> const &argsToDeliver);

    /**
     * @brief publishes the given return values via IPC
     * @param request_serial The serial of the request to which this is a response.
     * @param command The command name being responded to.
     * @param arg The value being returned.
     * @return IPC status
     */
    IpcSerialNumber publishReturnValues(IpcSerialNumber request_serial,
                                        std::string const &command,
                                        Value const &arg);

    /**
     * @brief publishes the given telemetry values via IPC
     * @param destName The destination ID for this message.
     * @param values Vector of PLEXIL Values to be published.
     * @return IPC status
     */
    IpcSerialNumber publishTelemetry(std::string const &destName, std::vector<Value> const &values);

    /**
     * @brief publishes the given telemetry value via IPC
     * @param nodeName The name of the node publishing the update.
     * @param update Vector of name, value pairs to publish.
     * @return IPC status
     */
    IpcSerialNumber publishUpdate(const std::string& nodeName,
                                  std::vector<std::pair<std::string, Value> > const& update);

    /**
     * @brief Returns the error code of the last publish method call. If the last publish call returned
     * -1, this will return the appropriate error. Otherwise, it will return IPC_OK.
     */
    IPC_RETURN_TYPE getError();

    //! Receive the message from IPC and handle it as required.
    //! @param msg The message to be handled.
    //! @note Called from dispatch thread.
    void handleMessage(PlexilMsgBase *msg);

  private:

    // Disallow copy, assignment, move
    IpcFacade(IpcFacade const &) = delete;
    IpcFacade(IpcFacade &&) = delete;
    IpcFacade& operator=(IpcFacade const &) = delete;
    IpcFacade& operator=(IpcFacade &&) = delete;

    //
    // Implementation functions
    //

    /**
     * @brief Cache start message of a multi-message sequence
     */
    void cacheMessageLeader(PlexilMsgBase* msgData);

    /**
     * @brief Cache following message of a multi-message sequence
     */
    void cacheMessageTrailer(PlexilMsgBase* msgData);

    //! Deliver the vector of messages to all listeners registered for the leader,
    //! then free the message data.
    //! @param msgs (Const reference to) Vector of message pointers
    //! @note Called from dispatch thread.
    void deliverMessages(const std::vector<PlexilMsgBase *> &msgs);

    /**
     * @brief Helper function for sending a vector of parameters via IPC.
     * @param args The arguments to convert into messages and send
     * @param serial The serial to send along with each parameter. This should be the same serial as the header
     */
    IPC_RETURN_TYPE sendParameters(std::vector<Value> const &args, IpcSerialNumber serial);

    /**
     * @brief Helper function for sending a vector of parameters via IPC to a specific executive.
     * @param args The arguments to convert into messages and send
     * @param serial The serial to send along with each parameter. This should be the same serial as the header
     * @param dest The destination executive name
     */
    IPC_RETURN_TYPE sendParameters(std::vector<Value> const &args, IpcSerialNumber serial, std::string const &dest);

    /** 
     * @brief Helper function for sending a vector of pairs via IPC.
     * @param pairs The pairs to convert into messages and send
     * @param serial The serial to send along with each parameter.  This should be the same serial s the header.
     * 
     * @return The IPC error status.
     */
    IPC_RETURN_TYPE sendPairs(std::vector<std::pair<std::string, Value> > const& pairs,
                              IpcSerialNumber serial);

    /**
     * @brief Set the error code of the last called IPC method.
     * @param error The error code of the last called IPC method.
     */
    void setError(IPC_RETURN_TYPE error);

    /**
     * @brief Get next serial number
     */
    IpcSerialNumber getSerialNumber();

    /**
     * @brief Subscribes to all PLEXIL messages.
     * @return IPC_OK if all succeeded.
     */
    IPC_RETURN_TYPE subscribeToMsgs();

    /**
     * @brief Unsubscribes from all PLEXIL messages.
     * @return IPC_OK if all succeeded.
     */
    IPC_RETURN_TYPE unsubscribeFromMsgs();

    /**
     * @brief Removes all registered IpcMessageListeners to all plexil message subscriptions
     */
    void unsubscribeAllListeners();

    /**
     * @brief Removes the given IpcMessageListener from all PLEXIL message subscriptions
     * @param handler The handler function to be unsubscribed
     */
    void unsubscribeAll(IpcMessageListener* handler);

    /**
     * Unsubscribes from the given message on central. Wrapper for IPC_unsubscribe
     * @param msgName the name of the message to unsubscribe from
     * @param handler The handler to unsubscribe.
     */
    IPC_RETURN_TYPE unsubscribeCentral(const char *msgName, HANDLER_TYPE handler);

    /**
     * Subscribes to the given message on central. Wrapper for IPC_subscribeData
     * @param msgName the name of the message to unsubscribe from
     * @param handler The handler to unsubscribe.
     */
    IPC_RETURN_TYPE subscribeDataCentral(const char *msgName,
                                         HANDLER_DATA_TYPE handler);

    //
    // Static utility methods
    //

    /**
     * @brief Handler function as seen by IPC.
     */
    /**
     * @brief Initialize unique ID string
     */
    static std::string generateUID();

    //
    // Private types
    //

    //* brief List of listeners registered globally - for ListenerMap
    typedef std::vector<IpcMessageListener*> ListenerList;

    //* brief Map of message types to lists of registered listeners
    typedef std::map<uint16_t, ListenerList> ListenerMap;

    //* brief Unique identifier of a message sequence
    typedef std::pair<std::string, IpcSerialNumber> IpcMessageId;

    //* brief Cache of not-yet-complete message sequences
    typedef std::map<IpcMessageId, std::vector<PlexilMsgBase *> > IncompleteMessageMap;

    //
    // Class constants
    //

    static constexpr uint16_t const ALL_MSG_TYPE = std::numeric_limits<uint16_t>::max();

    //
    // Member data
    //

    //! Handlers registered for all message types.
    //* @note Shared between threads.
    ListenerList m_listenersToAll;

    //* Map of message type to list of listeners for that type
    //* @note Shared between threads.
    ListenerMap m_registeredListeners;

    //* Cache of incomplete received message data
    //* @note Only accessed from the dispatch thread, therefore no locking required.
    IncompleteMessageMap m_incompletes;

    //* @brief Unique ID of this adapter instance
    std::string m_myUID;

    //* @brief Mutex for registered listener tables.
    std::mutex m_listenersMutex;

    //* @brief The handle for the message thread
    pthread_t m_threadHandle;

    //* @brief Count of # of outgoing commands and requests, starting with 1
    //  @note Should only ever be 0 at initialization
    IpcSerialNumber m_nextSerial;

    //* @brief The error code of the last called IPC method.
    IPC_RETURN_TYPE m_error;

    //* @brief Is the facade initialized?
    bool m_isInitialized;

    //* @brief Is the facade started?
    bool m_isStarted;

    //* @brief True if the dispatch thread should stop.
    bool m_stopDispatchThread;
  };

  /**
   * @brief Utility function to extract the value from a value message.
   * @param msg Pointer to const IPC message.
   * @return The Value represented by the message.
   * @note The returned value will be unknown if the message is not a value message.
   */
  // Called by IpcAdapter, sigh.
  extern Value getPlexilMsgValue(struct PlexilMsgBase const *msg);

}

#endif /* DEFINE_IPC_FACADE_H */
