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

#include "ThreadMutex.hh"
#include "IpcFacade.hh"
#include "Debug.hh"
#include "StoredArray.hh"

// ooid classes
#include "uuid.h"
#include "devrand.h"

namespace PLEXIL {

const int IpcFacade::ALL_MSG_TYPE = ((int) PlexilMsgType_uninited) - 1;
const std::string& IpcFacade::MY_UID = IpcFacade::generateUID();
RecursiveThreadMutex IpcFacade::mutex;
pthread_t IpcFacade::threadHandle;
IpcFacade::ListenerMap IpcFacade::registeredListeners;
IpcFacade::IncompleteMessageMap IpcFacade::incompletes;
int IpcFacade::numInitilized = 0;
int IpcFacade::numStarted = 0;
uint32_t IpcFacade::nextSerial = 1;

IpcFacade::IpcFacade() :
  m_isInitilized(false), m_isStarted(false) {

}

IpcFacade::~IpcFacade() {
  if (m_isStarted) {
    stop();
  }
  if (m_isInitilized) {
    shutdown();
  }
}

const std::string& IpcFacade::getUID() {
  return MY_UID;
}
/**
 * @breif Connects to the Ipc server. This should be called before calling start().
 * If it is not, this method is called by start. If already initilized, this method
 * does nothing and returns IPC_OK.
 */
IPC_RETURN_TYPE IpcFacade::initilize(const std::string& taskName, const std::string& serverName) {
  if (m_isInitilized) {
    return IPC_OK;
  }
  debugMsg("IpcFacade::initilize", "locking mutex");
  mutex.lock();
  IPC_RETURN_TYPE result = IPC_OK;
  //if this is the first instance to initilize, perform global initilization
  if (numInitilized == 0) {
    // Initialize IPC
    // possibly redundant, but always safe
    result = IPC_initialize();

    // Connect to central
    if (result == IPC_OK)
      result = IPC_connectModule(taskName.c_str(), serverName.c_str());

    // Define messages
    if (result == IPC_OK)
      result = definePlexilIPCMessageTypes() ? IPC_OK : IPC_Error;
  }
  if (result == IPC_OK) {
    m_isInitilized = true;
    numInitilized++;
  }
  mutex.unlock();
  return result;
}

/**
 * @brief Initilizes and starts the Ipc message handling thread. If Ipc is already
 * started, this method does nothing and returns IPC_OK.
 * @return IPC_Error if the dispatch thread is not started correctly, IPC_OK otherwise
 */
IPC_RETURN_TYPE IpcFacade::start() {
  IPC_RETURN_TYPE result = IPC_OK;
  if (!m_isInitilized)
    result = IPC_Error;
  mutex.lock();
  //perform only when this instance is the only started instance of the class
  if (result == IPC_OK && !m_isStarted && numStarted == 0) {
    //spawn message thread - seperated for readability
    if (threadSpawn((THREAD_FUNC_PTR) IPC_dispatch, NULL, threadHandle)) {
      // Subscribe to messages
      IPC_RETURN_TYPE status;
      status = IPC_subscribeData(MSG_BASE, messageHandler, NULL);
      assertTrueMsg(status == IPC_OK, "ipcUtil::ipcSubscribeAll: Error subscribing to " << MSG_BASE << " messages, IPC_errno = " << IPC_errno);
      status = IPC_subscribeData(RETURN_VALUE_MSG, messageHandler, NULL);
      assertTrueMsg(status == IPC_OK, "ipcUtil::ipcSubscribeAll: Error subscribing to " << RETURN_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
      status = IPC_subscribeData(NUMERIC_VALUE_MSG, messageHandler, NULL);
      assertTrueMsg(status == IPC_OK, "ipcUtil::ipcSubscribeAll: Error subscribing to " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
      status = IPC_subscribeData(STRING_VALUE_MSG, messageHandler, NULL);
      assertTrueMsg(status == IPC_OK, "ipcUtil::ipcSubscribeAll: Error subscribing to " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
      // *** TODO: implement receiving planner update
      //    status = IPC_subscribeData(NUMERIC_PAIR_MSG, messageHandler, this);
      //    assertTrueMsg(status == IPC_OK,
      //          "IpcFacade: Error subscribing to " << NUMERIC_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
      //    status = IPC_subscribeData(STRING_PAIR_MSG, messageHandler, this);
      //    assertTrueMsg(status == IPC_OK,
      //          "IpcFacade: Error subscribing to " << STRING_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
    } else {
      result = IPC_Error;
    }
  }
  if (result == IPC_OK) {
    m_isStarted = true;
    numStarted++;
  }
  mutex.unlock();
  return result;
}
/**
 * @brief Removes all subscriptions registered by this IpcFacade. If
 * this is the only running instance of IpcFacade, stops the Ipc message
 * handling thread. If Ipc is not running, this method does nothing and returns IPC_OK.
 */
void IpcFacade::stop() {
  if (!m_isStarted) {
    return;
  }
  debugMsg("IpcFacade::stop", "locking mutex");
  mutex.lock();
  m_isStarted = false;
  unsubscribeAll();
  numStarted--;
  //when this is the last currently running instance
  if (numStarted == 0) {
    // Unsubscribe from messages
    IPC_RETURN_TYPE status;
    status = IPC_unsubscribe(MSG_BASE, messageHandler);
    assertTrueMsg(status == IPC_OK,
        "IpcFacade: Error unsubscribing from " << MSG_BASE << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(RETURN_VALUE_MSG, messageHandler);
    assertTrueMsg(status == IPC_OK,
        "IpcFacade: Error unsubscribing from " << RETURN_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(NUMERIC_VALUE_MSG, messageHandler);
    assertTrueMsg(status == IPC_OK,
        "IpcFacade: Error unsubscribing from " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(STRING_VALUE_MSG, messageHandler);
    assertTrueMsg(status == IPC_OK,
        "IpcFacade: Error unsubscribing from " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    // *** TODO: implement receiving planner update
    //     status = IPC_unsubscribe(NUMERIC_PAIR_MSG, handler);
    //     assertTrueMsg(status == IPC_OK,
    //          "IpcFacade: Error unsubscribing from " << NUMERIC_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
    //     status = IPC_unsubscribe(STRING_PAIR_MSG, handler);
    //     assertTrueMsg(status == IPC_OK,
    //          "IpcFacade: Error unsubscribing from " << STRING_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);

    // Cancel IPC dispatch thread
    int myErrno;
    myErrno = pthread_cancel(threadHandle);
    if (myErrno == 0) {
      myErrno = pthread_join(threadHandle, NULL);
      if (myErrno != 0) {
        debugMsg("IpcUtil:stop", "Error in pthread_join with errorno " << myErrno);
      }
    } else {
      debugMsg("IpcUtil:stop", "Error in pthread_cancel with errorno " << myErrno);
    }
  }
  mutex.unlock();
}
/**
 * @brief Disconnects from the Ipc server. This puts Ipc back in its initial state before
 * being initilized.
 */
void IpcFacade::shutdown() {
  debugMsg("IpcFacade::shutdown", "locking mutex");
  mutex.lock();
  if (m_isInitilized) {
    if (m_isStarted) {
      stop();
    }
    numInitilized--;
    // Disconnect from central
    m_isInitilized = false;
    if (numInitilized == 0)
      IPC_disconnect();
  }
  mutex.unlock();
}

void IpcFacade::subscribeAll(IpcMessageListener* listener) {
  m_localRegisteredHandlers.push_back(LocalListenerRef(ALL_MSG_TYPE, listener));
  subscribeGlobal(LocalListenerRef(ALL_MSG_TYPE, listener));
}

void IpcFacade::subscribe(IpcMessageListener* listener, PlexilMsgType type) {
  m_localRegisteredHandlers.push_back(LocalListenerRef((int)type, listener));
  subscribeGlobal(LocalListenerRef((int)type, listener));
}

void IpcFacade::unsubscribeAll() {
  while (!m_localRegisteredHandlers.empty())
    unsubscribeAll(m_localRegisteredHandlers.front().second);
}

void IpcFacade::unsubscribeAll(IpcMessageListener* listener) {
  //prevent modification and access while removing
  bool removed = false;
  mutex.lock();
  for (LocalListenerList::iterator it = m_localRegisteredHandlers.begin(); !removed && it != m_localRegisteredHandlers.end(); it++) {
    if ((*it).second == listener) {
      unsubscribeGlobal(*it);
      m_localRegisteredHandlers.erase(it);
      removed = true;
    }
  }
  mutex.unlock();
}

/**
 * @brief publishes the given message via IPC
 * @param command The command string to send
 */
uint32_t IpcFacade::publishMessage(LabelStr command) {
  assertTrue(m_isStarted, "publishMessage called before started");
  struct PlexilStringValueMsg packet = { { PlexilMsgType_Message, 0, getSerialNumber(), getUID().c_str() }, command.c_str() };
  return IPC_publishData(STRING_VALUE_MSG, (void *) &packet);
}

uint32_t IpcFacade::publishCommand(LabelStr command, const std::list<double>& argsToDeliver) {
  assertTrue(m_isStarted, "publishCommand called before started");
  uint32_t serial = getSerialNumber();
  struct PlexilStringValueMsg cmdPacket = { { PlexilMsgType_Command, argsToDeliver.size(), serial, getUID().c_str() }, command.c_str() };
  IPC_RETURN_TYPE result = IPC_publishData(STRING_VALUE_MSG, (void *) &cmdPacket);
  if (result == IPC_OK) {
    result = sendParameters(argsToDeliver, serial);
    debugMsg("IpcFacade:publishCommand", "Command " << command.toString() << " published with serial " << serial);
  }
  setError(result);
  return result == IPC_OK ? serial : ERROR_SERIAL();
}

uint32_t IpcFacade::publishLookupNow(LabelStr lookup, const std::list<double>& argsToDeliver) {
  // Construct the messages
  // Leader
  uint32_t serial = getSerialNumber();
  struct PlexilStringValueMsg leader =
    { { PlexilMsgType_LookupNow, argsToDeliver.size(), serial, MY_UID.c_str() }, lookup.c_str() };

  IPC_RETURN_TYPE result;
  result = IPC_publishData(STRING_VALUE_MSG, (void *) &leader);
  if (result == IPC_OK) {
    result = sendParameters(argsToDeliver, serial);
  }
  setError(result);
  return result == IPC_OK ? serial : ERROR_SERIAL();
}

uint32_t IpcFacade::publishReturnValues(uint32_t request_serial, LabelStr request_uid, const std::list<double>& args) {
  assertTrue(m_isStarted, "publishReturnValues called before started");
  uint32_t serial = getSerialNumber();
  struct PlexilReturnValuesMsg packet = { { PlexilMsgType_ReturnValues, args.size(), serial, getUID().c_str() }, request_serial, request_uid.c_str() };
  IPC_RETURN_TYPE result = IPC_publishData(RETURN_VALUE_MSG, (void *) &packet);
  if (result == IPC_OK) {
    result = sendParameters(args, serial);
  }
  setError(result);
  return result == IPC_OK ? serial : ERROR_SERIAL();
}
IPC_RETURN_TYPE IpcFacade::getError() {
  return m_error;
}

void IpcFacade::setError(IPC_RETURN_TYPE error) {
  m_error = error;
}

uint32_t IpcFacade::publishTelemetry(const std::string& destName, const std::list<double>& values) {
  // Telemetry values message
  debugMsg("IpcFacade:publishTelemetry",
       " sending telemetry message for \"" << destName << "\"");
  PlexilStringValueMsg* tvMsg = new PlexilStringValueMsg();
  tvMsg->header.msgType = (uint16_t) PlexilMsgType_TelemetryValues;
  tvMsg->stringValue = destName.c_str();

  tvMsg->header.count = values.size();
  uint32_t leaderSerial = getSerialNumber();
  tvMsg->header.serial = leaderSerial;
  tvMsg->header.senderUID= MY_UID.c_str();
  IPC_RETURN_TYPE status = IPC_publishData(STRING_VALUE_MSG, (void *) tvMsg);
  if (status == IPC_OK) {
    status = sendParameters(values, leaderSerial);
  }
  setError(status);
  return status == IPC_OK ? leaderSerial : ERROR_SERIAL();
}

/**
 * @brief Helper function for sending a vector of parameters via IPC.
 * @param args The arguments to convert into messages and send
 * @param serial The serial to send along with each parameter. This should be the same serial as the header
 */
IPC_RETURN_TYPE IpcFacade::sendParameters(const std::list<double>& args, uint32_t serial) {
  size_t nParams = args.size();
  // Construct parameter messages
  PlexilMsgBase* paramMsgs[nParams];
  unsigned int i = 0;
  for (std::list<double>::const_iterator it = args.begin(); it != args.end(); it++, i++) {
    double param = *it;
    PlexilMsgBase* paramMsg;
    if (LabelStr::isString(param)) {
      // string
      struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
      strMsg->stringValue = LabelStr(param).c_str();
      paramMsg = (PlexilMsgBase*) strMsg;
      paramMsg->msgType = PlexilMsgType_StringValue;
      debugMsg("IpcFacade:sendParameters", "String parameter: " << strMsg->stringValue);
    } else if (StoredArray::isKey(param)) {
      // array
      assertTrueMsg(ALWAYS_FAIL,
          "IpcAdapter: Array values are not yet implemented");
    } else {
      // number or Boolean
      struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
      numMsg->doubleValue = param;
      paramMsg = (PlexilMsgBase*) numMsg;
      paramMsg->msgType = PlexilMsgType_NumericValue;
      debugMsg("IpcFacade:sendParameters", "Numeric parameter: " << param);
    }

    // Fill in common fields
    paramMsg->count = i;
    paramMsg->serial = serial;
    paramMsg->senderUID = MY_UID.c_str();
    paramMsgs[i] = paramMsg;
  }

  // Send the messages
  // *** TODO: check for IPC errors ***
  IPC_RETURN_TYPE result = IPC_OK;
  for (size_t i = 0; i < nParams && result == IPC_OK; i++) {
    result = IPC_publishData(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType), paramMsgs[i]);
  }

  // free the parameter packets
  for (size_t i = 0; i < nParams; i++) {
    PlexilMsgBase* m = paramMsgs[i];
    paramMsgs[i] = NULL;
    if (m->msgType == PlexilMsgType_NumericValue)
      delete (PlexilNumericValueMsg*) m;
    else
      delete (PlexilStringValueMsg*) m;
  }

  return result;
}

/**
 * @brief Get next serial number
 */
uint32_t IpcFacade::getSerialNumber() {
  return nextSerial++;
}

bool IpcFacade::definePlexilIPCMessageTypes() {
  IPC_RETURN_TYPE status;
  if (!IPC_isMsgDefined(MSG_BASE)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(MSG_BASE, IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  if (!IPC_isMsgDefined(RETURN_VALUE_MSG)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(RETURN_VALUE_MSG, IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  if (!IPC_isMsgDefined(NUMERIC_VALUE_MSG)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(NUMERIC_VALUE_MSG, IPC_VARIABLE_LENGTH, NUMERIC_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  if (!IPC_isMsgDefined(STRING_VALUE_MSG)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(STRING_VALUE_MSG, IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  if (!IPC_isMsgDefined(NUMERIC_PAIR_MSG)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(NUMERIC_PAIR_MSG, IPC_VARIABLE_LENGTH, NUMERIC_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  if (!IPC_isMsgDefined(STRING_PAIR_MSG)) {
    if (IPC_errno != IPC_No_Error)
      return false;
    status = IPC_defineMsg(STRING_PAIR_MSG, IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
  }
  return true;
}

/**
 * @brief Handler function as seen by IPC.
 */
void IpcFacade::messageHandler(MSG_INSTANCE rawMsg, void * unmarshalledMsg, void * this_as_void_ptr) {
  // Check whether the thread has been canceled before going any further
  pthread_testcancel();

  const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*> (unmarshalledMsg);
  assertTrueMsg(msgData != NULL,
      "IpcFacade:messageHandler: pointer to message data is null!");

  PlexilMsgType msgType = (PlexilMsgType) msgData->msgType;
  debugMsg("IpcFacade:messageHandler", " received message type = " << msgType);
  switch (msgType) {
    // LookupNow and LookupOnChange are PlexilStringValueMsg
    // Optionally followed by parameters

    // TODO: filter out commands/msgs we aren't prepared to handle
  case PlexilMsgType_Command:
  case PlexilMsgType_LookupNow:
  case PlexilMsgType_LookupOnChange:
  case PlexilMsgType_PlannerUpdate:
  case PlexilMsgType_TelemetryValues:
    debugMsg("IpcFacade:messageHandler", "processing as multi-part message");
    cacheMessageLeader(msgData);
    break;

    // ReturnValues is a PlexilReturnValuesMsg
    // Followed by 0 (?) or more values
    // Only pay attention to return values directed at us
  case PlexilMsgType_ReturnValues: {
    debugMsg("IpcFacade:messageHandler", " processing as return value");
    const PlexilReturnValuesMsg* returnLeader = (const PlexilReturnValuesMsg*) msgData;
    if (strcmp(returnLeader->requesterUID, MY_UID.c_str()) == 0)
      cacheMessageLeader(msgData);
    break;
  }
    // Values - could be parameters or return values
  case PlexilMsgType_NumericValue:
  case PlexilMsgType_StringValue:

    // PlannerUpdate pairs
  case PlexilMsgType_PairNumeric:
  case PlexilMsgType_PairString:

    // Log with corresponding leader message
    cacheMessageTrailer(msgData);
    break;

  default:
    debugMsg("IpcFacade:messageHandler", "Received single-message type, delivering to listeners");
    deliverMessage(std::vector<const PlexilMsgBase*>(1, msgData));
    break;
  }
}
/**
 * @brief Cache start message of a multi-message sequence
 */

// N.B. Presumes that messages are received in order.
// Also presumes that any required filtering (e.g. on command name) has been done by the caller

void IpcFacade::cacheMessageLeader(const PlexilMsgBase* msgData) {
  IpcMessageId msgId(msgData->senderUID, msgData->serial);

  // Check that this isn't a duplicate header
  IncompleteMessageMap::iterator it = incompletes.find(msgId);
  assertTrueMsg(it == incompletes.end(),
      "IpcFacade::cacheMessageLeader: internal error: found existing sequence for sender "
      << msgData->senderUID << ", serial " << msgData->serial);

  if (msgData->count == 0) {
    debugMsg("IpcFacade:cacheMessageLeader", " count == 0, processing immediately");
    std::vector<const PlexilMsgBase*> msgVec(1, msgData);
    deliverMessage(msgVec);
  } else {
    debugMsg("IpcFacade:cacheMessageLeader",
        " storing leader with sender " << msgData->senderUID << ", serial " << msgData->serial
        << ",\n expecting " << msgData->count << " values");
    incompletes[msgId] = std::vector<const PlexilMsgBase*>(1, msgData);
  }
}

/**
 * @brief Cache following message of a multi-message sequence
 */

// N.B. Presumes that messages are received in order.

void IpcFacade::cacheMessageTrailer(const PlexilMsgBase* msgData) {
  IpcMessageId msgId(msgData->senderUID, msgData->serial);
  IncompleteMessageMap::iterator it = incompletes.find(msgId);
  if (it == incompletes.end()) {
    debugMsg("IpcFacade::cacheMessageTrailer",
        " no existing sequence for sender "
        << msgData->senderUID << ", serial " << msgData->serial << ", ignoring");
    return;
  }
  std::vector<const PlexilMsgBase*>& msgs = it->second;
  msgs.push_back(msgData);
  // Have we got them all?
  if (msgs.size() > msgs[0]->count) {
    deliverMessage(msgs);
    incompletes.erase(it);
  }
}

/**
 * @brief Deliver the given message to all listeners registered for it
 */
void IpcFacade::deliverMessage(const std::vector<const PlexilMsgBase*>& msgs) {
  if (!msgs.empty()) {
    //send to listeners for all
    ListenerMap::iterator map_it = registeredListeners.find(ALL_MSG_TYPE);
    if (map_it != registeredListeners.end()) {
      for (ListenerList::iterator it = (*map_it).second.begin(); it != (*map_it).second.end(); it++) {
        (*it)->ReceiveMessage(msgs);
      }
    }
    //send to listeners for msg type
    map_it = registeredListeners.find(msgs.front()->msgType);
    if (map_it != registeredListeners.end()) {
      for (ListenerList::iterator it = (*map_it).second.begin(); it != (*map_it).second.end(); it++) {
        (*it)->ReceiveMessage(msgs);
      }
    }

    // clean up
    for (size_t i = 0; i < msgs.size(); i++) {
      const PlexilMsgBase* msg = msgs[i];
      IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msg->msgType)), (void *) msg);
    }
  }
}

void IpcFacade::subscribeGlobal(const LocalListenerRef& listener) {
  //creates a new entry if one does not already exist
  registeredListeners[listener.first].push_back(listener.second);
}

/**
 * @brief Unsubscribe the given listener from the static listener map.
 * @return True if found and unsubscribed. False if not found.
 */
bool IpcFacade::unsubscribeGlobal(const LocalListenerRef& listener) {
  ListenerMap::iterator map_it = registeredListeners.find(listener.first);
  if (map_it != registeredListeners.end()) {
    for (ListenerList::iterator it = (*map_it).second.begin(); it != (*map_it).second.end(); it++) {
      if (listener.second == (*it)) {
        it = (*map_it).second.erase(it);
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief Initialize unique ID string
 */
const std::string& IpcFacade::generateUID() {
  kashmir::system::DevRand randomStream;
  kashmir::uuid_t uuid;
  randomStream >> uuid;
  std::ostringstream s;
  s << uuid;
  debugMsg("IpcAdapter:initializeUID", " generated UUID " << s.str());
  return *(new std::string(s.str()));
}
}
