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
#include "CommonDefs.hh"

// ooid classes
#include "uuid.h"
#include "devrand.h"

namespace PLEXIL {

  IpcFacade::IpcFacade() :
    m_isInitialized(false),
    m_isStarted(false),
    m_nextSerial(1),
    m_myUID(generateUID())
  {
    debugMsg("IpcFacade", " constructor");
  }

  IpcFacade::~IpcFacade() {
    debugMsg("IpcFacade", " destructor");
    if (m_isStarted) {
      stop();
    }
    if (m_isInitialized) {
      shutdown();
    }
    // *** why is this necessary?? ***
    if (m_mutex.isLockedByCurrentThread()) {
      debugMsg("IpcFacade", " destructor: unlocking mutex");
      m_mutex.unlock();
    }
  }

  const std::string& IpcFacade::getUID() {
    return m_myUID;
  }
  /**
   * @brief Connects to the Ipc server. This should be called before calling start().
   * If it is not, this method is called by start. If already initialized, this method
   * does nothing and returns IPC_OK.
   * @param taskName If null, the current UID of the IpcFacade is used as the task name.
   */
  IPC_RETURN_TYPE IpcFacade::initialize(const char* taskName, const char* serverName) {
    if (m_isInitialized) {
      debugMsg("IpcFacade:initialize", " already done, returning");
      return IPC_OK;
    }

    if (taskName != NULL && taskName != m_myUID)
      m_myUID = taskName;

    IPC_RETURN_TYPE result = IPC_OK;
    debugMsg("IpcFacade:initialize", " locking mutex");
    RTMutexGuard guard(m_mutex);

    // perform global initialization
    // Initialize IPC
    // possibly redundant, but always safe
    debugMsg("IpcFacade:initialize", " calling IPC_initialize()");
    result = IPC_initialize();
    if (result != IPC_OK) {
      debugMsg("IpcFacade:initialize", " IPC_initialize() failed, IPC_errno = " << IPC_errno);
      return result;
    }

    // Connect to central
    debugMsg("IpcFacade:initialize", " calling IPC_connectModule()");
    result = IPC_connectModule(m_myUID.c_str(), serverName);
    if (result != IPC_OK)
      {
	debugMsg("IpcFacade:initialize", " IPC_connectModule() failed, IPC_errno = " << IPC_errno);
	return result;
      }

    // Define messages
    debugMsg("IpcFacade:initialize", " defining message types");
    if (definePlexilIPCMessageTypes())
      {
	result = IPC_OK;
      }
    else
      {
	condDebugMsg(result == IPC_OK, "IpcFacade:initialize", " defining message types failed");
	result = IPC_Error;
      }
    if (result == IPC_OK) {
      m_isInitialized = true;
      debugMsg("IpcFacade:initialize", " succeeded");
    }
    return result;
  }

  /**
   * @brief Initializes and starts the Ipc message handling thread. If Ipc is already
   * started, this method does nothing and returns IPC_OK.
   * @return IPC_Error if the dispatch thread is not started correctly, IPC_OK otherwise
   */
  IPC_RETURN_TYPE IpcFacade::start() {
    IPC_RETURN_TYPE result = IPC_OK;
    if (!m_isInitialized)
      result = IPC_Error;
    debugMsg("IpcFacade:start", " locking mutex");
    RTMutexGuard guard(m_mutex);
    //perform only when this instance is the only started instance of the class
    if (result == IPC_OK && !m_isStarted) {
      //spawn message thread - separated for readability
      debugMsg("IpcFacade:start", " spawning IPC dispatch thread");
      if (threadSpawn((THREAD_FUNC_PTR) IPC_dispatch, NULL, m_threadHandle)) {
	// Subscribe to messages
	debugMsg("IpcFacade:start", " subscribing to messages");
	IPC_RETURN_TYPE status;
	status = subscribeDataCentral(MSG_BASE, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << MSG_BASE << " messages, IPC_errno = " << IPC_errno);
	status = subscribeDataCentral(RETURN_VALUE_MSG, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << RETURN_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
	status = subscribeDataCentral(NUMERIC_VALUE_MSG, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
	status = subscribeDataCentral(STRING_VALUE_MSG, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
	status = subscribeDataCentral(STRING_ARRAY_MSG, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << STRING_ARRAY_MSG << " messages, IPC_errno = " << IPC_errno);
	status = subscribeDataCentral(NUMERIC_ARRAY_MSG, messageHandler);
	assertTrueMsg(status == IPC_OK, "IpcFacade::start: Error subscribing to " << NUMERIC_ARRAY_MSG << " messages, IPC_errno = " << IPC_errno);
	// *** TODO: implement receiving planner update
	//    status = subscribeDataCentral(NUMERIC_PAIR_MSG, messageHandler);
	//    assertTrueMsg(status == IPC_OK,
	//          "IpcFacade: Error subscribing to " << NUMERIC_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
	//    status = subscribeDataCentral(STRING_PAIR_MSG, messageHandler);
	//    assertTrueMsg(status == IPC_OK,
	//          "IpcFacade: Error subscribing to " << STRING_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
      } else {
	result = IPC_Error;
      }
    }
    if (result == IPC_OK) {
      debugMsg("IpcFacade:start", " succeeded");
      m_isStarted = true;
    }
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
    debugMsg("IpcFacade:stop", " locking mutex");
    RTMutexGuard guard(m_mutex);
    m_isStarted = false;
    unsubscribeAll();

    // Unsubscribe from messages
    debugMsg("IpcFacade:stop", " unsubscribing from messages");
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
    debugMsg("IpcFacade:stop", " cancelling dispatch thread");
    int myErrno;
    myErrno = pthread_cancel(m_threadHandle);
    if (myErrno == 0) {
      myErrno = pthread_join(m_threadHandle, NULL);
      if (myErrno != 0) {
	debugMsg("IpcUtil:stop", "Error in pthread_join with errno " << myErrno);
      }
    } else {
      debugMsg("IpcUtil:stop", "Error in pthread_cancel with errno " << myErrno);
    }
  }

  /**
   * @brief Disconnects from the Ipc server. This puts Ipc back in its initial state before
   * being initialized.
   */
  void IpcFacade::shutdown() {
    debugMsg("IpcFacade::shutdown", "locking mutex");
    RTMutexGuard guard(m_mutex);
    if (m_isInitialized) {
      if (m_isStarted) {
	stop();
      }

      // Disconnect from central
      IPC_disconnect();
    }
    m_isInitialized = false;
  }

  void IpcFacade::subscribeAll(IpcMessageListener* listener) {
    m_localRegisteredHandlers.push_back(LocalListenerRef(ALL_MSG_TYPE(), listener));
    subscribeGlobal(LocalListenerRef(ALL_MSG_TYPE(), listener));
  }

  void IpcFacade::subscribe(IpcMessageListener* listener, PlexilMsgType type) {
    m_localRegisteredHandlers.push_back(LocalListenerRef((uint16_t) type, listener));
    subscribeGlobal(LocalListenerRef((uint16_t) type, listener));
  }

  void IpcFacade::unsubscribeAll() {
    debugMsg("IpcFacade:unsubscribeAll", " entered");
    while (!m_localRegisteredHandlers.empty())
      unsubscribeAll(m_localRegisteredHandlers.front().second);
    debugMsg("IpcFacade:unsubscribeAll", " succeeded");
  }

  void IpcFacade::unsubscribeAll(IpcMessageListener* listener) {
    //prevent modification and access while removing
    RTMutexGuard guard(m_mutex);
    bool removed = false;
    for (LocalListenerList::iterator it = m_localRegisteredHandlers.begin();
	 !removed && it != m_localRegisteredHandlers.end();
	 it++) {
      if ((*it).second == listener) {
	unsubscribeGlobal(*it);
	m_localRegisteredHandlers.erase(it);
	removed = true;
      }
    }
  }

  /**
   * @brief publishes the given message via IPC
   * @param command The command string to send
   */
  uint32_t IpcFacade::publishMessage(LabelStr command) {
    assertTrue(m_isStarted, "publishMessage called before started");
    struct PlexilStringValueMsg packet = { { PlexilMsgType_Message, 0, getSerialNumber(), m_myUID.c_str() }, command.c_str() };
    return IPC_publishData(STRING_VALUE_MSG, (void *) &packet);
  }

  uint32_t IpcFacade::publishCommand(LabelStr command, const std::list<double>& argsToDeliver) {
    return sendCommand(command, LabelStr(), argsToDeliver);
  }
  uint32_t IpcFacade::sendCommand(LabelStr command, LabelStr dest, const std::list<double>& argsToDeliver) {
    assertTrue(m_isStarted, "publishCommand called before started");
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg cmdPacket = { { PlexilMsgType_Command, argsToDeliver.size(), serial, m_myUID.c_str() }, command.c_str() };
    IPC_RETURN_TYPE result = IPC_publishData(formatMsgName(STRING_VALUE_MSG, dest.toString()).c_str(), (void *) &cmdPacket);
    if (result == IPC_OK) {
      result = sendParameters(argsToDeliver, serial);
      debugMsg("IpcFacade:publishCommand", "Command " << command.toString() << " published with serial " << serial);
    }
    setError(result);
    return result == IPC_OK ? serial : ERROR_SERIAL();
  }

  uint32_t IpcFacade::publishLookupNow(LabelStr lookup, const std::list<double>& argsToDeliver) {
    return sendLookupNow(lookup, LabelStr(), argsToDeliver);
  }

  uint32_t IpcFacade::sendLookupNow(LabelStr lookup, LabelStr dest, const std::list<double>& argsToDeliver) {
    // Construct the messages
    // Leader
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg leader = { { PlexilMsgType_LookupNow, argsToDeliver.size(), serial, m_myUID.c_str() }, lookup.c_str() };

    IPC_RETURN_TYPE result;
    result = IPC_publishData(formatMsgName(STRING_VALUE_MSG, dest.toString()).c_str(), (void *) &leader);
    if (result == IPC_OK) {
      result = sendParameters(argsToDeliver, serial);
    }
    setError(result);
    return result == IPC_OK ? serial : ERROR_SERIAL();
  }

  uint32_t IpcFacade::publishReturnValues(uint32_t request_serial, LabelStr request_uid, double arg) {
    assertTrue(m_isStarted, "publishReturnValues called before started");
    uint32_t serial = getSerialNumber();
    struct PlexilReturnValuesMsg packet = { { PlexilMsgType_ReturnValues, 1, serial, m_myUID.c_str() }, request_serial, request_uid.c_str() };
    IPC_RETURN_TYPE result = IPC_publishData(formatMsgName(RETURN_VALUE_MSG, request_uid.toString()).c_str(), (void *) &packet);
    if (result == IPC_OK) {
      result = sendParameters(std::list<double>(1, arg), serial, request_uid);
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
    tvMsg->header.senderUID = m_myUID.c_str();
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
    return sendParameters(args, serial, "");
  }

  /**
   * @brief Helper function for sending a vector of parameters via IPC to a specific executive.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   * @param dest The destination executive name. If dest is an empty string, parameters are broadcast to
   * all executives
   */
  IPC_RETURN_TYPE IpcFacade::sendParameters(const std::list<double>& args, uint32_t serial, const LabelStr& dest) {
    size_t nParams = args.size();
    // Construct parameter messages
    PlexilMsgBase* paramMsgs[nParams];
    unsigned int i = 0;
    for (std::list<double>::const_iterator it = args.begin(); it != args.end(); it++, i++) {
      double param = *it;
      PlexilMsgBase* paramMsg;
      if (PLEXIL::UNKNOWN() != param && StoredArray::isKey(param)) {
	StoredArray array(param);
	int size = array.size();
	BasicType type = determineType(array.getKey());
	if (type == STRING) {
	  const char* (*strings) = new const char*[size];
	  for (int i = 0; i < size; i++) {
	    strings[i] = LabelStr(array[i]).c_str();
	  }
	  struct PlexilStringArrayMsg* strArrayMsg = new PlexilStringArrayMsg();
	  strArrayMsg->stringArray = strings;
	  strArrayMsg->arraySize = size;
	  paramMsg = (PlexilMsgBase*) strArrayMsg;
	  paramMsg->msgType = PlexilMsgType_StringArray;
	} else {
	  double* nums= new double[size];
	  for (int i = 0; i < size; i++) {
	    nums[i] = array[i];
	  }
	  struct PlexilNumericArrayMsg* numArrayMsg = new PlexilNumericArrayMsg();
	  numArrayMsg->arraySize = size;
	  numArrayMsg->doubleArray = nums;
	  debugMsg("IpcFacade:sendParameters", "First parameter of array is " << numArrayMsg->doubleArray[0]);
	  paramMsg = (PlexilMsgBase*) numArrayMsg;
	  paramMsg->msgType = PlexilMsgType_NumericArray;
	}
      } else if (PLEXIL::UNKNOWN() == param || !LabelStr::isString(param)) {
	// number or Boolean
	struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
	numMsg->doubleValue = param;
	paramMsg = (PlexilMsgBase*) numMsg;
	paramMsg->msgType = PlexilMsgType_NumericValue;
	debugMsg("IpcFacade:sendParameters", "Numeric parameter: " << param);
      } else {
	// string
	struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
	strMsg->stringValue = LabelStr(param).c_str();
	paramMsg = (PlexilMsgBase*) strMsg;
	paramMsg->msgType = PlexilMsgType_StringValue;
	debugMsg("IpcFacade:sendParameters", "String parameter: " << strMsg->stringValue);
      }

      // Fill in common fields
      paramMsg->count = i;
      paramMsg->serial = serial;
      paramMsg->senderUID = m_myUID.c_str();
      paramMsgs[i] = paramMsg;
    }

    // Send the messages
    IPC_RETURN_TYPE result = IPC_OK;
    for (size_t i = 0; i < nParams && result == IPC_OK; i++) {
      result = IPC_publishData(formatMsgName(std::string(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType)), dest.toString()).c_str(), paramMsgs[i]);
    }

    // free the parameter packets
    for (size_t i = 0; i < nParams; i++) {
      PlexilMsgBase* m = paramMsgs[i];
      paramMsgs[i] = NULL;
      switch (m->msgType) {
      case (PlexilMsgType_NumericValue):
	delete (PlexilNumericValueMsg*) m;
	break;
      case (PlexilMsgType_StringValue):
	delete (PlexilStringValueMsg*) m;
	break;
      case (PlexilMsgType_NumericArray):
	delete (PlexilNumericArrayMsg*) m;
	break;
      default:
	delete (PlexilStringArrayMsg*) m;
	break;
      }
    }

    return result;
  }

  /**
   * @brief Get next serial number
   */
  uint32_t IpcFacade::getSerialNumber() {
    return m_nextSerial++;
  }

  bool IpcFacade::definePlexilIPCMessageTypes() {
    debugMsg("IpcFacade:definePlexilIPCMessageTypes", " entered");
    IPC_RETURN_TYPE status;
    status = IPC_defineMsg(MSG_BASE, IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(MSG_BASE), m_myUID).c_str(), IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(RETURN_VALUE_MSG, IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(RETURN_VALUE_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(NUMERIC_VALUE_MSG, IPC_VARIABLE_LENGTH, NUMERIC_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(NUMERIC_VALUE_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, NUMERIC_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_VALUE_MSG, IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(STRING_VALUE_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(NUMERIC_ARRAY_MSG, IPC_VARIABLE_LENGTH, NUMERIC_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(NUMERIC_ARRAY_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, NUMERIC_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_ARRAY_MSG, IPC_VARIABLE_LENGTH, STRING_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(STRING_ARRAY_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, STRING_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(NUMERIC_PAIR_MSG, IPC_VARIABLE_LENGTH, NUMERIC_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(NUMERIC_PAIR_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, NUMERIC_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_PAIR_MSG, IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(std::string(STRING_PAIR_MSG), m_myUID).c_str(), IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
    condDebugMsg(status == IPC_OK, "IpcFacade:definePlexilIPCMessageTypes", " succeeded");
    return status == IPC_OK;
  }

  /**
   * @brief Handler function as seen by IPC.
   */
  void IpcFacade::messageHandler(MSG_INSTANCE /* rawMsg */,
				 void * unmarshalledMsg,
				 void * this_as_void_ptr) {
    // Check whether the thread has been canceled before going any further
    pthread_testcancel();

    const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*> (unmarshalledMsg);
    assertTrueMsg(msgData != NULL,
		  "IpcFacade::messageHandler: pointer to message data is null!");
    IpcFacade* facade = reinterpret_cast<IpcFacade*>(this_as_void_ptr);
    assertTrueMsg(msgData != NULL,
		  "IpcFacade::messageHandler: pointer to IpcFacade instance is null!");

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
      facade->cacheMessageLeader(msgData);
      break;

      // ReturnValues is a PlexilReturnValuesMsg
      // Followed by 0 (?) or more values
      // Only pay attention to return values directed at us
    case PlexilMsgType_ReturnValues: {
      debugMsg("IpcFacade:messageHandler", " processing as return value");
      const PlexilReturnValuesMsg* returnLeader = (const PlexilReturnValuesMsg*) msgData;
      if (strcmp(returnLeader->requesterUID, facade->getUID().c_str()) == 0)
	facade->cacheMessageLeader(msgData);
      break;
    }
      // Values - could be parameters or return values
    case PlexilMsgType_NumericValue:
    case PlexilMsgType_StringValue:
      // Arrays
    case PlexilMsgType_NumericArray:
    case PlexilMsgType_StringArray:

      // PlannerUpdate pairs
    case PlexilMsgType_PairNumeric:
    case PlexilMsgType_PairString:

      // Log with corresponding leader message
      facade->cacheMessageTrailer(msgData);
      break;

    default:
      debugMsg("IpcFacade:messageHandler", "Received single-message type, delivering to listeners");
      facade->deliverMessage(std::vector<const PlexilMsgBase*>(1, msgData));
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
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    assertTrueMsg(it == m_incompletes.end(),
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
      m_incompletes[msgId] = std::vector<const PlexilMsgBase*>(1, msgData);
    }
  }

  /**
   * @brief Cache following message of a multi-message sequence
   */

  // N.B. Presumes that messages are received in order.

  void IpcFacade::cacheMessageTrailer(const PlexilMsgBase* msgData) {
    IpcMessageId msgId(msgData->senderUID, msgData->serial);
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    if (it == m_incompletes.end()) {
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
      m_incompletes.erase(it);
    }
  }

  /**
   * @brief Deliver the given message to all listeners registered for it
   */
  void IpcFacade::deliverMessage(const std::vector<const PlexilMsgBase*>& msgs) {
    if (!msgs.empty()) {
      //send to listeners for all
      ListenerMap::iterator map_it = m_registeredListeners.find(ALL_MSG_TYPE());
      if (map_it != m_registeredListeners.end()) {
	for (ListenerList::iterator it = (*map_it).second.begin(); it != (*map_it).second.end(); it++) {
	  (*it)->ReceiveMessage(msgs);
	}
      }
      //send to listeners for msg type
      map_it = m_registeredListeners.find(msgs.front()->msgType);
      if (map_it != m_registeredListeners.end()) {
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
    m_registeredListeners[listener.first].push_back(listener.second);
  }

  /**
   * @brief Unsubscribe the given listener from the listener map.
   * @return True if found and unsubscribed. False if not found.
   */
  bool IpcFacade::unsubscribeGlobal(const LocalListenerRef& listener) {
    ListenerMap::iterator map_it = m_registeredListeners.find(listener.first);
    if (map_it != m_registeredListeners.end()) {
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
  std::string IpcFacade::generateUID() {
    kashmir::system::DevRand randomStream;
    kashmir::uuid_t uuid;
    randomStream >> uuid;
    std::ostringstream s;
    s << uuid;
    debugMsg("IpcFacade:generateUID", " generated UUID " << s.str());
    return s.str();
  }

  IpcFacade::BasicType IpcFacade::determineType(double array_id) {
    BasicType type = UNKNOWN;
    StoredArray array(array_id);
    int size = array.size();
    for (int i = 0; i < size && type == UNKNOWN; i++) {
      if (PLEXIL::UNKNOWN() == array[i]) {
	continue;
      } else if (LabelStr::isString(array[i])) {
	type = STRING;
      } else {
	type = NUMERIC;
      }
    }
    return type;
  }

  /**
   * Unsubscribes from the given message and the UID-specific version of the given message on central. Wrapper for IPC_unsubscribe.
   * If an error occurs in unsubscribing from the given message, the UID-specific version is not processed
   * @param msgName the name of the message to unsubscribe from
   * @param handler The handler to unsubscribe.
   */
  IPC_RETURN_TYPE IpcFacade::unsubscribeCentral (const char *msgName, HANDLER_TYPE handler) {
    IPC_RETURN_TYPE result = IPC_unsubscribe(msgName, handler);
    if (result == IPC_OK) {
      result = IPC_unsubscribe(formatMsgName(std::string(msgName), m_myUID).c_str(), handler);
    }
    return result;
  }

  /**
   * Subscribes from the given message and the UID-specific version of the given message on central. Wrapper for IPC_unsubscribe.
   * If an error occurs in subscribing from the given message, the UID-specific version is not processed
   * @param msgName the name of the message to subscribe from
   * @param handler The handler to subscribe.
   * @param clientData Pointer to data that will be passed to handler upon message receipt.
   */
  IPC_RETURN_TYPE IpcFacade::subscribeDataCentral (const char *msgName,
						   HANDLER_DATA_TYPE handler) {
    void* clientData = reinterpret_cast<void*>(this);
    debugMsg("IpcFacade:subscribeDataCentral", " for message name \"" << msgName << "\"");
    checkError(IPC_isMsgDefined(msgName),
	       "IpcFacade::subscribeDataCentral: fatal error: message \"" << msgName << "\" not defined");
    IPC_RETURN_TYPE result = IPC_subscribeData(msgName, handler, clientData);
    if (result == IPC_OK) {
      result = IPC_subscribeData(formatMsgName(std::string(msgName), m_myUID).c_str(), handler, clientData);
    }
    condDebugMsg(result != IPC_OK,
		 "IpcFacade:subscribeDataCentral", " for message name \"" << msgName << "\" failed, IPC_errno = " << IPC_errno);
    return result;
  }

  /**
   * Returns a formatted message type string given the basic message type and destination ID.
   * The caller is responsible for
   * @param msgName The name of the message type
   * @param destId The destination ID for the message
   */
  std::string IpcFacade::formatMsgName(const std::string& msgName, const std::string& destId) {
    return destId + msgName;
  }

}
