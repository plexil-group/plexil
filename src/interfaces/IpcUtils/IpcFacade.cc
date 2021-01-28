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

#include "IpcFacade.hh"

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "Debug.hh"
#include "Error.hh"

#include <algorithm>
#include <fstream>
#include <map>
#include <thread>

#if defined(HAVE_CSTDIO)
#include <cstdio>
#elif defined(HAVE_STDIO_H)
#include <stdio.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL 
{
  // forward reference
  static bool definePlexilIPCMessageTypes(std::string uid);
  static void myIpcDispatch(bool *flag_ptr);
  static void ipcMessageHandler(MSG_INSTANCE /* rawMsg */,
                                void * unmarshalledMsg,
                                void * this_as_void_ptr);

  /**
   * Returns a constant character string pointer for the formatted message type,
   *  given the basic message type and destination ID.
   * @param msgName The name of the message type
   * @param destId The destination ID for the message
   * @return Character string pointer.
   */
  static char const *formatMsgName(const char *msgName, const std::string &destId)
  {
    using MessageFormatKey = std::pair<std::string, std::string>;
    using MessageFormatMap = std::map<MessageFormatKey, std::string>;

    static MessageFormatMap sl_formatMap;

    MessageFormatKey const key(msgName, destId);
    std::string &resultString = sl_formatMap[key];
    if (!resultString.empty())
      return resultString.c_str();
    sl_formatMap[key] = destId + msgName;
    return sl_formatMap[key].c_str();
  }

  /**
   * @brief Return the message format string corresponding to the message type.
   * @param typ The message type.
   * @return Const char pointer to the message format name.
   */
  static inline const char* msgFormatForType(const PlexilMsgType typ)
  {
    switch (typ) {
    case PlexilMsgType_NotifyExec:
    case PlexilMsgType_UnknownValue:
      return MSG_BASE;
      break;

    case PlexilMsgType_AddPlan:
    case PlexilMsgType_AddPlanFile:
    case PlexilMsgType_AddLibrary:
    case PlexilMsgType_AddLibraryFile:
    case PlexilMsgType_Command:
    case PlexilMsgType_Message:
    case PlexilMsgType_LookupNow:
    case PlexilMsgType_PlannerUpdate:
    case PlexilMsgType_StringValue:
    case PlexilMsgType_TelemetryValues:
      return STRING_VALUE_MSG;
      break;

    case PlexilMsgType_ReturnValues:
      return RETURN_VALUE_MSG;
      break;

    case PlexilMsgType_BooleanValue:
      return BOOLEAN_VALUE_MSG;
      break;

    case PlexilMsgType_IntegerValue:
      return INTEGER_VALUE_MSG;
      break;

    case PlexilMsgType_RealValue:
      return REAL_VALUE_MSG;
      break;
              
    case PlexilMsgType_BooleanArray:
      return BOOLEAN_ARRAY_MSG;
      break;
              
    case PlexilMsgType_IntegerArray:
      return INTEGER_ARRAY_MSG;
      break;
              
    case PlexilMsgType_RealArray:
      return REAL_ARRAY_MSG;
      break;

    case PlexilMsgType_StringArray:
      return STRING_ARRAY_MSG;
      break;

    case PlexilMsgType_PairBoolean:
      return BOOLEAN_PAIR_MSG;
      break;

    case PlexilMsgType_PairInteger:
      return INTEGER_PAIR_MSG;
      break;

    case PlexilMsgType_PairReal:
      return REAL_PAIR_MSG;
      break;

    case PlexilMsgType_PairString:
      return STRING_PAIR_MSG;
      break;

    default:
      return nullptr;
      break;
    }
  }

  /**
   * @brief Utility function to create a value message from a PLEXIL Value.
   * @param val The Value to encode in the message.
   * @return Pointer to newly allocated IPC message.
   * @note Returns nullptr for unimplemented/invalid Values.
   */
  struct PlexilMsgBase *constructPlexilValueMsg(Value const &val)
  {
    if (val.isKnown()) {
      switch (val.valueType()) {
      case BOOLEAN_TYPE: {
        bool b;
        val.getValue(b);
        struct PlexilBooleanValueMsg *boolMsg = new struct PlexilBooleanValueMsg;
        boolMsg->header.msgType = PlexilMsgType_BooleanValue;
        boolMsg->boolValue = (unsigned char) b;
        debugMsg("constructPlexilValueMsg", " Boolean value is " << b);
        return (struct PlexilMsgBase *) boolMsg;
      }

      case INTEGER_TYPE: {
        struct PlexilIntegerValueMsg *intMsg = new struct PlexilIntegerValueMsg;
        intMsg->header.msgType = PlexilMsgType_IntegerValue;
        val.getValue(intMsg->intValue);
        debugMsg("constructPlexilValueMsg", " Integer value is " << intMsg->intValue);
        return (struct PlexilMsgBase *) intMsg;
      }

      case REAL_TYPE: {
        struct PlexilRealValueMsg *realMsg = new struct PlexilRealValueMsg;
        realMsg->header.msgType = PlexilMsgType_RealValue;
        val.getValue(realMsg->doubleValue);
        debugMsg("constructPlexilValueMsg", " Real value is " << realMsg->doubleValue);
        return (struct PlexilMsgBase *) realMsg;
      }

      case STRING_TYPE: {
        std::string const *sp;
        val.getValuePointer(sp);
        struct PlexilStringValueMsg *stringMsg = new struct PlexilStringValueMsg;
        stringMsg->header.msgType = PlexilMsgType_StringValue;
        stringMsg->stringValue = sp->c_str();
        debugMsg("constructPlexilValueMsg", " String value is \"" << stringMsg->stringValue << "\"");
        return (struct PlexilMsgBase *) stringMsg;
      }

      case COMMAND_HANDLE_TYPE: {
        CommandHandleValue handle;
        val.getValue(handle);
        struct PlexilCommandHandleValueMsg *handleMsg = new struct PlexilCommandHandleValueMsg;
        handleMsg->header.msgType = PlexilMsgType_CommandHandleValue;
        handleMsg->commandHandleValue = handle;
        return (struct PlexilMsgBase *) handleMsg;
      }

      case BOOLEAN_ARRAY_TYPE: {
        BooleanArray const *ba = nullptr;
        val.getValuePointer(ba);
        assertTrue_1(ba);
        size_t size = ba->size();
        unsigned char *bools = new unsigned char[size];
        for (size_t i = 0; i < size; i++) {
          bool b;
          assertTrue_2(ba->getElement(i, b), "Boolean array element is UNKNOWN");
          bools[i] = (unsigned char) b;
        }
        struct PlexilBooleanArrayMsg* boolArrayMsg = new PlexilBooleanArrayMsg();
        boolArrayMsg->header.msgType = PlexilMsgType_BooleanArray;
        boolArrayMsg->arraySize = size;
        boolArrayMsg->boolArray = bools;
        debugMsg("constructPlexilValueMsg",
                 " Boolean array size " << boolArrayMsg->arraySize);
        debugStmt("constructPlexilValueMsg",
                  for (size_t i = 0; i < size; ++i) 
                    std::cout << "elt " << i << ": " << (bool) boolArrayMsg->boolArray[i] << '\n');
        debugMsg("constructPlexilValueMsg",
                 " First parameter of Boolean array is " << (bool) boolArrayMsg->boolArray[0]);
        return (struct PlexilMsgBase *) boolArrayMsg;
      }

      case INTEGER_ARRAY_TYPE: {
        IntegerArray const *ia = nullptr;
        val.getValuePointer(ia);
        assertTrue_1(ia);
        size_t size = ia->size();
        int32_t *nums = new int32_t[size];
        for (size_t i = 0; i < size; i++) 
          assertTrue_2(ia->getElement(i, nums[i]), "Integer array element is UNKNOWN");
        struct PlexilIntegerArrayMsg* intArrayMsg = new PlexilIntegerArrayMsg();
        intArrayMsg->header.msgType = PlexilMsgType_IntegerArray;
        intArrayMsg->arraySize = size;
        intArrayMsg->intArray = nums;
        debugMsg("constructPlexilValueMsg",
                 " Integer array size " << intArrayMsg->arraySize);
        debugStmt("constructPlexilValueMsg",
                  for (size_t i = 0; i < size; ++i) 
                    std::cout << "elt " << i << ": " << intArrayMsg->intArray[i] << '\n');
        return (struct PlexilMsgBase*) intArrayMsg;
      }

      case REAL_ARRAY_TYPE: {
        RealArray const *ra = nullptr;
        val.getValuePointer(ra);
        assertTrue_1(ra);
        size_t size = ra->size();
        double *nums = new double[size];
        for (size_t i = 0; i < size; i++) 
          assertTrue_1(ra->getElement(i, nums[i]));
        struct PlexilRealArrayMsg* realArrayMsg = new PlexilRealArrayMsg();
        realArrayMsg->header.msgType = PlexilMsgType_RealArray;
        realArrayMsg->arraySize = size;
        realArrayMsg->doubleArray = nums;
        debugMsg("constructPlexilValueMsg",
                 " Real array size " << realArrayMsg->arraySize);
        debugStmt("constructPlexilValueMsg",
                  for (size_t i = 0; i < size; ++i) 
                    std::cout << "elt " << i << ": " << realArrayMsg->doubleArray[i] << '\n');
        return (struct PlexilMsgBase *) realArrayMsg;
      }

      case STRING_ARRAY_TYPE: {
        StringArray const *sa = nullptr;
        val.getValuePointer(sa);
        assertTrue_1(sa);
        size_t size = sa->size();
        const char **strings = new const char*[size];
        for (size_t i = 0; i < size; i++) {
          std::string const *temp = nullptr;
          assertTrue_1(sa->getElementPointer(i, temp));
          strings[i] = temp->c_str();
        }
        struct PlexilStringArrayMsg* strArrayMsg = new PlexilStringArrayMsg();
        strArrayMsg->header.msgType = PlexilMsgType_StringArray;
        strArrayMsg->arraySize = size;
        strArrayMsg->stringArray = strings;
        debugMsg("constructPlexilValueMsg",
                 " String array size " << strArrayMsg->arraySize);
        debugStmt("constructPlexilValueMsg",
                  for (size_t i = 0; i < size; ++i) 
                    std::cout << "elt " << i << ": " << strArrayMsg->stringArray[i] << '\n');
        return (struct PlexilMsgBase *) strArrayMsg;
      }

      default:
        errorMsg("constructPlexilValueMsg: Invalid or unimplemented PLEXIL data type "
                 << val.valueType());
        return nullptr;
      }
    }
    else {
      // Unknown
      struct PlexilUnknownValueMsg *unkMsg = new struct PlexilUnknownValueMsg;
      unkMsg->header.msgType = PlexilMsgType_UnknownValue;
      debugMsg("constructPlexilValueMsg", " Unknown value");
      return (struct PlexilMsgBase *) unkMsg;
    }
  }

  struct PlexilMsgBase* constructPlexilPairMsg(std::string const& name,
                                               Value const val)
  {
    PlexilMsgBase* result = nullptr;
    if (val.isKnown()) {
      switch(val.valueType()) {
      case BOOLEAN_TYPE: {
        bool b;
        val.getValue(b);
        struct BooleanPair* boolMsg = new BooleanPair;
        boolMsg->pairBoolValue = b;
        debugMsg("constructPlexilPairMsg",
                 "(" << name << ", " << boolMsg->pairBoolValue << ")");
        result = reinterpret_cast<PlexilMsgBase*>(boolMsg);
        result->msgType = PlexilMsgType_PairBoolean;
        break;
      }
      case INTEGER_TYPE: {
        struct IntegerPair* intMsg = new IntegerPair;
        val.getValue(intMsg->pairIntValue);
        debugMsg("constructPlexilPairMsg",
                 "(" << name << ", " << intMsg->pairIntValue << ")");
        result = reinterpret_cast<PlexilMsgBase*>(intMsg);
        result->msgType = PlexilMsgType_PairInteger;
        break;
      }
      case REAL_TYPE: {
        struct RealPair* realMsg = new RealPair;
        val.getValue(realMsg->pairDoubleValue);
        debugMsg("constructPlexilPairMsg",
                 "(" << name << ", " << realMsg->pairDoubleValue << ")");
        result = reinterpret_cast<PlexilMsgBase*>(realMsg);
        result->msgType = PlexilMsgType_PairReal;
        break;
      }
      case STRING_TYPE: {
        std::string const* sp;
        val.getValuePointer(sp);
        struct StringPair* strMsg = new StringPair;
        strMsg->pairStringValue = sp->c_str();
        debugMsg("constructPlexilPairMsg",
                 "(" << name << ", " << strMsg->pairStringValue << ")");
        result = reinterpret_cast<PlexilMsgBase*>(strMsg);
        result->msgType = PlexilMsgType_PairString;
        break;
      }
      default:
        break;
      }
      if(result)
        reinterpret_cast<PairHeader*>(result)->pairName = name.c_str();
    }
    else {
      debugMsg("constructPlexilPairMsg", " Unknown value.");
    }
    return result;
  }

  /**
   * @brief Utility function to extract the value from a value message.
   * @param msg Pointer to const IPC message.
   * @return The Value represented by the message.
   * @note The returned value will be unknown if the message is not a value message.
   */
  Value getPlexilMsgValue(struct PlexilMsgBase const *msg)
  {
    assertTrue_2(msg, "getPlexilMsgValue: null pointer");

    debugMsg("getPlexilMsgValue", " message type = " << msg->msgType);
    switch ((PlexilMsgType) msg->msgType) {
    case PlexilMsgType_CommandHandleValue: {
      PlexilCommandHandleValueMsg const *param = reinterpret_cast<const struct PlexilCommandHandleValueMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received CommandHandle " << commandHandleValueName((CommandHandleValue) param->commandHandleValue));
      return Value((CommandHandleValue) param->commandHandleValue);
    }

    case PlexilMsgType_BooleanValue: {
      const struct PlexilBooleanValueMsg* param = reinterpret_cast<const struct PlexilBooleanValueMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Boolean " << param->boolValue);
      return Value((bool) param->boolValue);
    }

    case PlexilMsgType_IntegerValue: {
      const struct PlexilIntegerValueMsg* param = reinterpret_cast<const struct PlexilIntegerValueMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Integer " << param->intValue);
      return Value(param->intValue);
    }

    case PlexilMsgType_RealValue: {
      const struct PlexilRealValueMsg* param = reinterpret_cast<const struct PlexilRealValueMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Real " << param->doubleValue);
      return Value(param->doubleValue);
    }

    case PlexilMsgType_StringValue: {
      const struct PlexilStringValueMsg* param = reinterpret_cast<const struct PlexilStringValueMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received String " << param->stringValue);
      return Value(param->stringValue);
    }

    case PlexilMsgType_BooleanArray: {
      const struct PlexilBooleanArrayMsg* param = reinterpret_cast<const struct PlexilBooleanArrayMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Boolean array of size " << param->arraySize);
      debugStmt("getPlexilMsgValue",
                for (size_t j = 0; j < param->arraySize; j++)
                  std::cout << " elt " << j << ": " << (bool) param->boolArray[j] << '\n');
      BooleanArray array(param->arraySize);
      for (size_t j = 0; j < param->arraySize; j++)
        array.setElement(j, (bool) param->boolArray[j]);
      return Value(array);
    }

    case PlexilMsgType_IntegerArray: {
      const struct PlexilIntegerArrayMsg* param = reinterpret_cast<const struct PlexilIntegerArrayMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Integer array of size " << param->arraySize);
      debugStmt("getPlexilMsgValue",
                for (size_t j = 0; j < param->arraySize; j++)
                  std::cout << " elt " << j << ": " << param->intArray[j] << '\n');
      IntegerArray array(param->arraySize);
      for (size_t j = 0; j < param->arraySize; j++)
        array.setElement(j, param->intArray[j]);
      return Value(array);
    }

    case PlexilMsgType_RealArray: {
      const struct PlexilRealArrayMsg* param = reinterpret_cast<const struct PlexilRealArrayMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received Real array of size " << param->arraySize);
      debugStmt("getPlexilMsgValue",
                for (size_t j = 0; j < param->arraySize; j++)
                  std::cout << " elt " << j << ": " << param->doubleArray[j] << '\n');
      RealArray array(param->arraySize);
      for (size_t j = 0; j < param->arraySize; j++)
        array.setElement(j, param->doubleArray[j]);
      return Value(array);
    }

    case PlexilMsgType_StringArray: {
      const struct PlexilStringArrayMsg* param = reinterpret_cast<const struct PlexilStringArrayMsg*> (msg);
      debugMsg("getPlexilMsgValue",
               " received String array of size " << param->arraySize);
      debugStmt("getPlexilMsgValue",
                for (size_t j = 0; j < param->arraySize; j++)
                  std::cout << " elt " << j << ": " << param->stringArray[j] << '\n');
      StringArray array(param->arraySize);
      for (size_t j = 0; j < param->arraySize; j++)
        array.setElement(j, std::string(param->stringArray[j]));
      return Value(array);
    }

    default:
      // TODO: handle error more gracefully
      errorMsg("getPlexilMsgValue: invalid or unimplemented message type "
               << msg->msgType);
      // fall thru...

    case PlexilMsgType_UnknownValue:
      return Value();
    }
  }

  IpcFacade::IpcFacade() :
    m_myUID(generateUID()),
    m_listenersMutex(),
    m_nextSerial(1),
    m_isInitialized(false),
    m_isStarted(false),
    m_stopDispatchThread(false)
  {
    debugMsg("IpcFacade", " constructor");
  }

  IpcFacade::~IpcFacade() {
    debugMsg("IpcFacade", " destructor");
    if (m_isStarted) {
      stop();
    }
    if (m_isInitialized) { // but not started
      unsubscribeAllListeners();
      IPC_disconnect();
      m_isInitialized = false;
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
  IPC_RETURN_TYPE IpcFacade::initialize(const char* taskName, const char* serverName)
  {
    if (m_isInitialized) {
      debugMsg("IpcFacade:initialize", " already done, returning");
      return IPC_OK;
    }

    if (taskName && *taskName && taskName != m_myUID)
      m_myUID = taskName;

    debugMsg("IpcFacade:initialize",
             " UID " << m_myUID <<  " server name " << serverName);

    IPC_RETURN_TYPE result = IPC_OK;

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
    if (result != IPC_OK) {
      debugMsg("IpcFacade:initialize", " IPC_connectModule() failed, IPC_errno = " << IPC_errno);
      return result;
    }

    // Define messages
    debugMsg("IpcFacade:initialize", " defining message types");
    if (definePlexilIPCMessageTypes(m_myUID)) {
      result = IPC_OK;
    }
    else {
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
  IPC_RETURN_TYPE IpcFacade::start()
  {
    IPC_RETURN_TYPE result = IPC_OK;
    if (!m_isInitialized || !IPC_isConnected())
      result = IPC_Error;

    //perform only when this instance is the only started instance of the class
    if (result == IPC_OK && !m_isStarted) {
      // Subscribe to messages
      debugMsg("IpcFacade:start",
               ' ' << m_myUID << " subscribing to messages");
      subscribeToMsgs();

      // Spawn message thread AFTER all subscribes complete
      // Running thread in parallel with subscriptions resulted in deadlocks
      debugMsg("IpcFacade:start", ' ' << m_myUID << " spawning IPC dispatch thread");
      m_thread = std::thread(myIpcDispatch, &m_stopDispatchThread);
      m_isStarted = true;
    }
    return result;
  }

  /**
   * @brief Removes all subscriptions registered by this IpcFacade. If
   * this is the only running instance of IpcFacade, stops the Ipc message
   * handling thread. If Ipc is not running, this method does nothing and returns IPC_OK.
   */
  void IpcFacade::stop()
  {
    if (!m_isStarted) {
      return;
    }

    // Cancel IPC dispatch thread first to prevent deadlocks
    debugMsg("IpcFacade:stop", ' ' << m_myUID << " cancelling dispatch thread");
    m_stopDispatchThread = true;
    m_thread.join();

    debugMsg("IpcFacade:stop", ' ' << m_myUID << " unsubscribing all");
    unsubscribeAllListeners();

    debugMsg("IpcFacade:stop", ' ' << m_myUID << " unsubscribing from messages");
    unsubscribeFromMsgs();
    m_isStarted = false;

    // Disconnect from central
    debugMsg("IpcFacade:stop", ' ' << m_myUID << " disconnecting");
    IPC_disconnect();
    m_isInitialized = false;

    debugMsg("IpcFacade:stop", ' ' << m_myUID << " complete");
  }

  void IpcFacade::subscribeAll(IpcMessageListener* listener)
  {
    debugMsg("IpcFacade:subscribeAll", " locking listeners mutex");
    {
      std::lock_guard<std::mutex> guard(m_listenersMutex);
      m_listenersToAll.push_back(listener);
    }
    debugMsg("IpcFacade:subscribeAll", " unlocked listeners mutex");
  }

  void IpcFacade::subscribe(IpcMessageListener* listener, PlexilMsgType type)
  {
    debugMsg("IpcFacade:subscribe", " locking listeners mutex");
    {
      std::lock_guard<std::mutex> guard(m_listenersMutex);
      //creates a new entry if one does not already exist
      m_registeredListeners[type].push_back(listener);
    }
    debugMsg("IpcFacade:subscribe", " unlocked listeners mutex");
  }

  // Unsubscribe this one listener from all messages to which it listens.
  void IpcFacade::unsubscribe(IpcMessageListener *listener)
  {
    debugMsg("IpcFacade:unsubscribe", " locking listeners mutex");
    {
      std::lock_guard<std::mutex> guard(m_listenersMutex);
      std::remove(m_listenersToAll.begin(), m_listenersToAll.end(), listener);
      for (ListenerMap::value_type pair : m_registeredListeners)
        std::remove(pair.second.begin(), pair.second.end(), listener);
    }
    debugMsg("IpcFacade:unsubscribe", " unlocking listeners mutex");
  }

  // Ignore the "unused variable" warning, it's used; see below.
  static constexpr const char *ALL_MSG_TYPE_NAMES[] =
    {MSG_BASE,
     RETURN_VALUE_MSG,
     BOOLEAN_VALUE_MSG,
     INTEGER_VALUE_MSG,
     REAL_VALUE_MSG,
     STRING_VALUE_MSG,
     BOOLEAN_ARRAY_MSG,
     INTEGER_ARRAY_MSG,
     REAL_ARRAY_MSG,
     STRING_ARRAY_MSG,
     BOOLEAN_PAIR_MSG,
     INTEGER_PAIR_MSG,
     REAL_PAIR_MSG,
     STRING_PAIR_MSG,
     nullptr};

  IPC_RETURN_TYPE IpcFacade::subscribeToMsgs()
  {
    IPC_RETURN_TYPE status;
    for (const char * const *name = &ALL_MSG_TYPE_NAMES[0] ; *name ; ++name) {
      status = subscribeDataCentral(*name, ipcMessageHandler);
      assertTrueMsg(status == IPC_OK,
                    "IpcFacade " << m_myUID << ": Subscribing to " << *name
                    << " messages failed; IPC_errno = " << IPC_errno);
    }      
    return status;
  }

  void IpcFacade::unsubscribeAllListeners()
  {
    // prevent modification and access while removing
    debugMsg("IpcFacade:unsubscribeAll", " locking listeners mutex");
    {
      std::lock_guard<std::mutex> guard(m_listenersMutex);

      m_listenersToAll.clear();
      m_registeredListeners.clear();
    }
    debugMsg("IpcFacade:unsubscribeAll", " unlocked listeners mutex");
  }

  IPC_RETURN_TYPE IpcFacade::unsubscribeFromMsgs()
  {
    IPC_RETURN_TYPE status;
    for (const char * const *name = &ALL_MSG_TYPE_NAMES[0] ; *name ; ++name) {
      status = IPC_unsubscribe(*name, ipcMessageHandler);
      assertTrueMsg(status == IPC_OK,
                    "IpcFacade " << m_myUID << ": Unsubscribing from " << *name
                    << " messages failed; IPC_errno = " << IPC_errno);
    }      
    return status;
  }

  /**
   * @brief publishes the given message via IPC
   * @param command The command string to send
   */
  uint32_t IpcFacade::publishMessage(std::string const &command)
  {
    assertTrue_2(m_isStarted, "publishMessage called before started");
    struct PlexilStringValueMsg packet =
      { { PlexilMsgType_Message,
          0,
          getSerialNumber(),
          m_myUID.c_str() },
        command.c_str() };
    return IPC_publishData(STRING_VALUE_MSG, (void *) &packet);
  }

  uint32_t IpcFacade::publishCommand(std::string const &command,
                                     std::vector<Value> const &argsToDeliver)
  {
    return sendCommand(command, "", argsToDeliver);
  }

  uint32_t IpcFacade::sendCommand(std::string const &command,
                                  std::string const &dest,
                                  std::vector<Value> const &argsToDeliver)
  {
    assertTrue_2(m_isStarted, "publishCommand called before started");
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg cmdPacket =
      { { PlexilMsgType_Command,
          (uint16_t) argsToDeliver.size(),
          serial,
          m_myUID.c_str() },
        command.c_str() };

    IPC_RETURN_TYPE result =
      IPC_publishData(formatMsgName(STRING_VALUE_MSG, dest), (void *) &cmdPacket);

    if (result == IPC_OK) {
      result = sendParameters(argsToDeliver, serial);
    }

    setError(result);

    if (result == IPC_OK) {
      debugMsg("IpcFacade:publishCommand", ' ' << m_myUID
               << " Command \"" << command << "\" published, serial " << serial);
      return serial;
    }
    else {
      return ERROR_SERIAL;
    }
  }

  uint32_t IpcFacade::publishLookupNow(std::string const &lookup, std::vector<Value> const &argsToDeliver) {
    return sendLookupNow(lookup, "", argsToDeliver);
  }

  uint32_t IpcFacade::sendLookupNow(std::string const &lookup,
                                    std::string const &dest,
                                    std::vector<Value> const &argsToDeliver)
  {
    // Construct the messages
    // Leader
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg leader =
      { { PlexilMsgType_LookupNow,
          (uint16_t) argsToDeliver.size(),
          serial,
          m_myUID.c_str() },
        lookup.c_str() };

    IPC_RETURN_TYPE result =
      IPC_publishData(formatMsgName(STRING_VALUE_MSG, dest), (void *) &leader);

    if (result == IPC_OK && !argsToDeliver.empty())
      // Send trailers, if any 
      result = sendParameters(argsToDeliver, serial);

    setError(result);
    return result == IPC_OK ? serial : ERROR_SERIAL;
  }

  uint32_t IpcFacade::publishReturnValues(uint32_t request_serial,
                                          std::string const &request_uid,
                                          Value const &arg)
  {
    assertTrue_2(m_isStarted, "publishReturnValues called before started");
    uint32_t serial = getSerialNumber();
    struct PlexilReturnValuesMsg packet =
      { { PlexilMsgType_ReturnValues,
          1, // trailing msgs
          serial,
          m_myUID.c_str() },
        request_serial,
        request_uid.c_str() };
    IPC_RETURN_TYPE result =
      IPC_publishData(formatMsgName(RETURN_VALUE_MSG, request_uid), (void *) &packet);
    if (result == IPC_OK) {
      result = sendParameters(std::vector<Value>(1, arg), serial, request_uid);
    }
    setError(result);
    return result == IPC_OK ? serial : ERROR_SERIAL;
  }

  IPC_RETURN_TYPE IpcFacade::getError() {
    return m_error;
  }

  void IpcFacade::setError(IPC_RETURN_TYPE error) {
    m_error = error;
  }

  uint32_t IpcFacade::publishTelemetry(const std::string& destName,
                                       std::vector<Value> const &values)
  {
    // Telemetry values message
    debugMsg("IpcFacade:publishTelemetry",
             ' ' << m_myUID << " sending telemetry message for \"" << destName << "\"");
    uint32_t leaderSerial = getSerialNumber();
    PlexilStringValueMsg tvMsg =
      { { (uint16_t) PlexilMsgType_TelemetryValues,
          (uint16_t) values.size(),
          leaderSerial,
          m_myUID.c_str()},
        destName.c_str()};
    IPC_RETURN_TYPE status = IPC_publishData(STRING_VALUE_MSG, (void *) &tvMsg);
    if (status == IPC_OK && !values.empty()) {
      status = sendParameters(values, leaderSerial);
    }
    setError(status);
    return status == IPC_OK ? leaderSerial : ERROR_SERIAL;
  }
  
  uint32_t IpcFacade::publishUpdate(const std::string& nodeName,
                                    std::vector<std::pair<std::string, Value> > const& update) {
    debugMsg("IpcFacade:publishUpdate",
             ' ' << m_myUID << " sending planner update for \"" << nodeName << "\"");
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg updatePacket =
      {{PlexilMsgType_PlannerUpdate,
        static_cast<uint16_t>(update.size()),
        serial,
        m_myUID.c_str()},
       nodeName.c_str()};
    IPC_RETURN_TYPE status = IPC_publishData(STRING_VALUE_MSG, (void*) &updatePacket);
    if (status == IPC_OK) {
      status = sendPairs(update, serial);
    }
    setError(status);
    return status == IPC_OK ? serial : ERROR_SERIAL;
  }

  /**
   * @brief Helper function for sending a vector of parameters via IPC.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   */
  IPC_RETURN_TYPE IpcFacade::sendParameters(std::vector<Value> const &args, uint32_t serial) {
    return sendParameters(args, serial, "");
  }

  /**
   * @brief Helper function for sending a vector of parameters via IPC to a specific executive.
   * @param args The arguments to convert into messages and send
   * @param serial The serial to send along with each parameter. This should be the same serial as the header
   * @param dest The destination executive name. If dest is an empty string, parameters are broadcast to
   * all executives
   */
  IPC_RETURN_TYPE IpcFacade::sendParameters(std::vector<Value> const &args, uint32_t serial, std::string const &dest) {
    size_t nParams = args.size();
    // Construct parameter messages
    PlexilMsgBase* paramMsgs[nParams];
    for (size_t i = 0; i < nParams; ++i) {
      PlexilMsgBase* paramMsg = constructPlexilValueMsg(args[i]);
      // Fill in common fields
      paramMsg->count = i;
      paramMsg->serial = serial;
      paramMsg->senderUID = m_myUID.c_str();
      paramMsgs[i] = paramMsg;
    }
    
    // Send the messages
    IPC_RETURN_TYPE result = IPC_OK;
    for (size_t i = 0; i < nParams && result == IPC_OK; i++) {
      char const *msgFormat = msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType);
      if (dest.empty()) {
        debugMsg("IpcFacade:sendParameters",
                 " using format " << msgFormat << " for parameter " << i);
        result = IPC_publishData(msgFormat, paramMsgs[i]);
      }
      else {
        char const *msgName = formatMsgName(msgFormat, dest);
        debugMsg("IpcFacade:sendParameters",
                 " using format " << msgName << " for parameter " << i);
        result = IPC_publishData(msgName, paramMsgs[i]);
      }
    }

    // free the parameter packets
    for (size_t i = 0; i < nParams; i++) {
      PlexilMsgBase* m = paramMsgs[i];
      paramMsgs[i] = nullptr;
      switch (m->msgType) {
      case PlexilMsgType_UnknownValue:
        delete (PlexilUnknownValueMsg*) m;
        break;

      case PlexilMsgType_CommandHandleValue:
        delete (PlexilCommandHandleValueMsg*) m;
        break;

      case PlexilMsgType_BooleanValue:
        delete (PlexilBooleanValueMsg*) m;
        break;

      case PlexilMsgType_IntegerValue:
        delete (PlexilIntegerValueMsg*) m;
        break;

      case PlexilMsgType_RealValue:
        delete (PlexilRealValueMsg*) m;
        break;

      case PlexilMsgType_StringValue:
        delete (PlexilStringValueMsg*) m;
        break;

        // *** DON'T FREE ARRAY DATA! IPC does this. ***
      case PlexilMsgType_BooleanArray: {
        PlexilBooleanArrayMsg *bam = (PlexilBooleanArrayMsg*) m;
        delete bam;
        break;
      }

      case PlexilMsgType_IntegerArray: {
        PlexilIntegerArrayMsg *iam = (PlexilIntegerArrayMsg*) m;
        delete iam;
        break;
      }

      case PlexilMsgType_RealArray: {
        PlexilRealArrayMsg *ram = (PlexilRealArrayMsg*) m;
        delete ram;
        break;
      }

      case PlexilMsgType_StringArray: {
        PlexilStringArrayMsg *sam = (PlexilStringArrayMsg*) m;
        delete sam;
        break;
      }

      default:
        delete m;
        break;
      }
    }

    return result;
  }

  /** 
   * @brief Helper function for sending a vector of pairs via IPC.
   * @param pairs The pairs to convert into messages and send
   * @param serial The serial to send along with each parameter.  This should be the same serial s the header.
   * 
   * @return The IPC error status.
   */
  IPC_RETURN_TYPE IpcFacade::sendPairs(std::vector<std::pair<std::string, Value> > const& pairs,
                                       uint32_t serial) {
    IPC_RETURN_TYPE result = IPC_OK;
    for(std::vector<std::pair<std::string, Value> >::const_iterator it = pairs.begin();
        it != pairs.end() && result == IPC_OK; ++it) {
      PlexilMsgBase* pairMsg = constructPlexilPairMsg(it->first, it->second);
      pairMsg->count = std::distance(pairs.begin(), it);
      pairMsg->serial = serial;
      pairMsg->senderUID = m_myUID.c_str();
      std::string msgName =
        formatMsgName(msgFormatForType((PlexilMsgType) pairMsg->msgType),
                      "");
      result = IPC_publishData(msgName.c_str(), pairMsg);

      switch(pairMsg->msgType) {
      case PlexilMsgType_PairBoolean:
        delete reinterpret_cast<BooleanPair*>(pairMsg);
        break;
      case PlexilMsgType_PairInteger:
        delete reinterpret_cast<IntegerPair*>(pairMsg);
        break;
      case PlexilMsgType_PairReal:
        delete reinterpret_cast<RealPair*>(pairMsg);
        break;
      case PlexilMsgType_PairString:
        delete reinterpret_cast<StringPair*>(pairMsg);
        break;
      default:
        delete pairMsg;
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

  //! Define all PLEXIL message types with Central. Also defines each
  //! PLEXIL message type with the UID as a prefix for directed
  //! communication. Has no effect for any previously defined message
  //! types.
  //! @param uid The UID of the calling IpcFacade instance.
  //! @return true if successful, false otherwise
  //! @note Caller should ensure IPC_initialize() has been called first
  bool definePlexilIPCMessageTypes(std::string uid)
  {
    debugMsg("IpcFacade:definePlexilIPCMessageTypes", " entered");
    IPC_RETURN_TYPE status;
    status = IPC_defineMsg(MSG_BASE, IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(MSG_BASE, uid), IPC_VARIABLE_LENGTH, MSG_BASE_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(RETURN_VALUE_MSG, IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(RETURN_VALUE_MSG, uid), IPC_VARIABLE_LENGTH, RETURN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(BOOLEAN_VALUE_MSG, IPC_VARIABLE_LENGTH, BOOLEAN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(BOOLEAN_VALUE_MSG, uid), IPC_VARIABLE_LENGTH, BOOLEAN_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(INTEGER_VALUE_MSG, IPC_VARIABLE_LENGTH, INTEGER_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(INTEGER_VALUE_MSG, uid), IPC_VARIABLE_LENGTH, INTEGER_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(REAL_VALUE_MSG, IPC_VARIABLE_LENGTH, REAL_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(REAL_VALUE_MSG, uid), IPC_VARIABLE_LENGTH, REAL_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_VALUE_MSG, IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(STRING_VALUE_MSG, uid), IPC_VARIABLE_LENGTH, STRING_VALUE_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(BOOLEAN_ARRAY_MSG, IPC_VARIABLE_LENGTH, BOOLEAN_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(BOOLEAN_ARRAY_MSG, uid), IPC_VARIABLE_LENGTH, BOOLEAN_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(INTEGER_ARRAY_MSG, IPC_VARIABLE_LENGTH, INTEGER_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(INTEGER_ARRAY_MSG, uid), IPC_VARIABLE_LENGTH, INTEGER_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(REAL_ARRAY_MSG, IPC_VARIABLE_LENGTH, REAL_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(REAL_ARRAY_MSG, uid), IPC_VARIABLE_LENGTH, REAL_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_ARRAY_MSG, IPC_VARIABLE_LENGTH, STRING_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(STRING_ARRAY_MSG, uid), IPC_VARIABLE_LENGTH, STRING_ARRAY_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(BOOLEAN_PAIR_MSG, IPC_VARIABLE_LENGTH, BOOLEAN_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(BOOLEAN_PAIR_MSG, uid), IPC_VARIABLE_LENGTH, BOOLEAN_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(INTEGER_PAIR_MSG, IPC_VARIABLE_LENGTH, INTEGER_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(INTEGER_PAIR_MSG, uid), IPC_VARIABLE_LENGTH, INTEGER_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(REAL_PAIR_MSG, IPC_VARIABLE_LENGTH, REAL_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(REAL_PAIR_MSG, uid), IPC_VARIABLE_LENGTH, REAL_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(STRING_PAIR_MSG, IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
    if (status != IPC_OK)
      return false;
    status = IPC_defineMsg(formatMsgName(STRING_PAIR_MSG, uid), IPC_VARIABLE_LENGTH, STRING_PAIR_MSG_FORMAT);
    condDebugMsg(status == IPC_OK, "IpcFacade:definePlexilIPCMessageTypes", " succeeded");
    return status == IPC_OK;
  }

  //! IPC listener thread top level.
  //! Exits when the stop flag is set.
  void myIpcDispatch(bool *flag_ptr)
  {
    assertTrue_2(flag_ptr,
                 "IpcFacade dispatch thread: pointer to stop flag is null!");

    debugMsg("IpcFacade:myIpcDispatch", " started");
    IPC_RETURN_TYPE ipcStatus = IPC_OK;

    while (!*flag_ptr && ipcStatus != IPC_Error)
      ipcStatus = IPC_listenClear(1000); 

    assertTrueMsg(ipcStatus != IPC_Error,
                  "IpcFacade dispatch thread: IPC error, IPC_errno = " << IPC_errno);
      
    *flag_ptr = false;
    debugMsg("IpcFacade:myIpcDispatch", " terminated");
  }

  /**
   * @brief Handler function as seen by IPC.
   * @note Called from dispatch thread.
   */
  void ipcMessageHandler(MSG_INSTANCE /* rawMsg */,
                         void *unmarshalledMsg,
                         void *IpcFacade_as_void_ptr)
  {
    assertTrue_2(unmarshalledMsg,
                 "ipcMessageHandler: pointer to unmarshalled message is null!");
    assertTrue_2(IpcFacade_as_void_ptr,
                 "ipcMessageHandler: pointer to IpcFacade instance is null!");

    PlexilMsgBase* msgData = reinterpret_cast<PlexilMsgBase *>(unmarshalledMsg);
    IpcFacade* facade = reinterpret_cast<IpcFacade* >(IpcFacade_as_void_ptr);
    facade->handleMessage(msgData);
  }

  // Handle a message received from IPC dispatch thread
  void IpcFacade::handleMessage(PlexilMsgBase *msgData)
  {
    PlexilMsgType msgType = (PlexilMsgType) msgData->msgType;
    debugMsg("IpcFacade:handleMessage",
             ' ' << m_myUID << " received message type = " << msgType);
    switch (msgType) {

      // These can have 0 or more parameters,
      // and frequently have 0.
    case PlexilMsgType_Command:
    case PlexilMsgType_LookupNow:
      if (msgData->count) {
        debugMsg("IpcFacade:handleMessage",
                 ' ' << m_myUID << " received "
                 << (msgType == PlexilMsgType_Command ? "Command" : "LookupNow")
                 << " message with parameters, processing as multi-part");
        cacheMessageLeader(msgData);
      }
      else {
        debugMsg("IpcFacade:handleMessage",
                 ' ' << m_myUID << " delivering "
                 << (msgType == PlexilMsgType_Command ? "Command" : "LookupNow")
                 << " message with no parameters immediately");
        deliverMessages(std::vector<PlexilMsgBase *>(1, msgData));
      }
      break;

      // PlannerUpdate can have 0 pairs, but it's unlikely.
    case PlexilMsgType_PlannerUpdate:
      // TelemetryValues can have 0 values, but it's unlikely.
    case PlexilMsgType_TelemetryValues:
      debugMsg("IpcFacade:handleMessage", "processing as multi-part message");
      cacheMessageLeader(msgData);
      break;

      // ReturnValues is a PlexilReturnValuesMsg
      // Followed by 1 value
      // Only pay attention to return values directed at us
    case PlexilMsgType_ReturnValues: {
      debugMsg("IpcFacade:handleMessage", " processing as return value");
      const PlexilReturnValuesMsg* returnLeader = (const PlexilReturnValuesMsg*) msgData;
      if (!strcmp(returnLeader->requesterUID, getUID().c_str()))
        cacheMessageLeader(msgData);
      break;
    }
      // Values - could be parameters or return values
    case PlexilMsgType_UnknownValue:
    case PlexilMsgType_BooleanValue:
    case PlexilMsgType_IntegerValue:
    case PlexilMsgType_RealValue:
    case PlexilMsgType_StringValue:

      // Array values
    case PlexilMsgType_BooleanArray:
    case PlexilMsgType_IntegerArray:
    case PlexilMsgType_RealArray:
    case PlexilMsgType_StringArray:

      // PlannerUpdate pairs
    case PlexilMsgType_PairBoolean:
    case PlexilMsgType_PairInteger:
    case PlexilMsgType_PairReal:
    case PlexilMsgType_PairString:

      // CommandHandle return
    case PlexilMsgType_CommandHandleValue:

      // Log with corresponding leader message
      cacheMessageTrailer(msgData);
      break;

      // Standalone messages
    case PlexilMsgType_Message:
      // Not currently sent by IpcFacade,
      // but handled by IpcAdapter
    case PlexilMsgType_NotifyExec:
    case PlexilMsgType_AddPlan:
    case PlexilMsgType_AddPlanFile:
    case PlexilMsgType_AddLibrary:
    case PlexilMsgType_AddLibraryFile:

      debugMsg("IpcFacade:handleMessage", "Received single-message type, delivering to listeners");
      deliverMessages(std::vector<PlexilMsgBase *>(1, msgData));
      break;

    default:
      errorMsg("IpcFacade::handleMessage: Received unimplemented or invalid message type "
               << msgType);
      break;
    }
  }
  
  /**
   * @brief Cache start message of a multi-message sequence
   * @note Called from IPC dispatch thread.
   */

  // N.B. Presumes that messages are received in order.
  // Also presumes that any required filtering (e.g. on command name) has been done by the caller

  void IpcFacade::cacheMessageLeader(PlexilMsgBase* msgData)
  {
    IpcMessageId const msgId(msgData->senderUID, msgData->serial);

    // Check that this isn't a duplicate leader
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    assertTrueMsg(it == m_incompletes.end(),
                  "IpcFacade " << m_myUID
                  << ": internal error: received duplicate leader for sender "
                  << msgData->senderUID << ", serial " << msgData->serial);

    if (msgData->count == 0) {
      debugMsg("IpcFacade:cacheMessageLeader", " count == 0, processing immediately");
      deliverMessages(std::vector<PlexilMsgBase *>(1, msgData));
    }
    else {
      debugMsg("IpcFacade:cacheMessageLeader",
               " storing leader from sender " << msgData->senderUID
               << " serial " << msgData->serial
               << ", expecting " << msgData->count << " values");
      std::vector<PlexilMsgBase *> msgVec(msgData->count + 1);
      msgVec.clear(); // reset length to 0
      msgVec.push_back(msgData);
      m_incompletes[msgId] = msgVec;
    }
  }

  /**
   * @brief Cache following message of a multi-message sequence
   * @note Called from IPC dispatch thread.
   */
  // N.B. Presumes that messages are received in order.
  void IpcFacade::cacheMessageTrailer(PlexilMsgBase* msgData)
  {
    IpcMessageId msgId(msgData->senderUID, msgData->serial);
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    if (it == m_incompletes.end()) {
      debugMsg("IpcFacade:cacheMessageTrailer",
               ' ' << m_myUID << " no existing sequence for sender "
               << msgData->senderUID << ", serial " << msgData->serial << ", ignoring");
      return;
    }
    std::vector<PlexilMsgBase*> &msgs = it->second;
    msgs.push_back(msgData);
    // Have we got them all?
    assertTrueMsg(msgs[0],
                  "IpcFacade::cacheMessageTrailer " << m_myUID
                  << ": pointer to leader is null!");

    if (msgs.size() > msgs[0]->count) {
      debugMsg("IpcFacade:cacheMessageTrailer",
               ' ' << m_myUID << " delivering " << msgs.size() << " messages");
      deliverMessages(msgs);
      m_incompletes.erase(it);
    }
  }

  //! Deliver the given messages to all listeners registered for the leader,
  //! then free the message data.
  //! @param msgs (Const reference to) Vector of message pointers
  //! @note Called from dispatch thread.
  void IpcFacade::deliverMessages(const std::vector<PlexilMsgBase *>& msgs)
  {
    assertTrue_2(!msgs.empty(),
                 "IpcFacade::deliverMessages: empty message vector");

    {
      debugMsg("IpcFacade:deliverMessage", " locking listeners mutex");
      std::lock_guard<std::mutex> guard(m_listenersMutex);

      // send to listeners for all
      for (IpcMessageListener * l : m_listenersToAll)
        l->ReceiveMessage(msgs);

      // send to listeners for msg type
      ListenerMap::const_iterator map_it =
        m_registeredListeners.find(msgs.front()->msgType);
      if (map_it != m_registeredListeners.end()) {
        for (IpcMessageListener * l : map_it->second)
          l->ReceiveMessage(msgs);
      }
    }
    debugMsg("IpcFacade:deliverMessage", " unlocked listeners mutex");

    // clean up
    for (size_t i = 0; i < msgs.size(); i++) {
      PlexilMsgBase* msg = msgs[i];
      IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msg->msgType)), (void *) msg);
    }
  }

// UUID generation constants
#define UUID_SIZE_BITS 128

// 8-4-4-4-12 format
#define UUID_STRING_SIZE (8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12)

  /**
   * @brief Initialize unique ID string
   */
  std::string IpcFacade::generateUID()
  {
    debugMsg("IpcFacade:generateUID", " entered");
    uint16_t randomBits[UUID_SIZE_BITS/16];
    {
      std::ifstream randumb("/dev/urandom", std::ios::in | std::ios::binary);
      if (!randumb) {
        debugMsg("IpcFacade:generateUID", " unable to open /dev/urandom for reading");
        return std::string();
      }

      if (!randumb.read(reinterpret_cast<char *>(randomBits), UUID_SIZE_BITS/8)) {
        debugMsg("IpcFacade:generateUID",
                 " read of " << UUID_SIZE_BITS/8 << " bytes from /dev/urandom failed");
        return std::string();
      }
    }
    char resultbuf[UUID_STRING_SIZE + 1];
    snprintf(resultbuf, UUID_STRING_SIZE + 1,
             "%04X%04X-%04X-%04X-%04X-%04X%04X%04X",
             randomBits[0],
             randomBits[1],
             randomBits[2],
             (randomBits[3] & 0xfff) | 0x4000,  // version 4 - random
             (randomBits[4] & 0x3fff) | 0x8000, // variant 1 - big-endian
             randomBits[5],
             randomBits[6],
             randomBits[7]);

    debugMsg("IpcFacade:generateUID", " returns " << resultbuf);
    return std::string(resultbuf);
  }

  /**
   * Unsubscribes from the given message and the UID-specific version of the given message on central. Wrapper for IPC_unsubscribe.
   * If an error occurs in unsubscribing from the given message, the UID-specific version is not processed
   * @param msgName the name of the message to unsubscribe from
   * @param handler The handler to unsubscribe.
   */
  IPC_RETURN_TYPE IpcFacade::unsubscribeCentral (const char *msgName, HANDLER_TYPE handler) {
    IPC_RETURN_TYPE result = IPC_unsubscribe(msgName, handler);
    if (result != IPC_OK)
      return result;
    return IPC_unsubscribe(formatMsgName(msgName, m_myUID), handler);
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
    if (result == IPC_OK)
      result = IPC_subscribeData(formatMsgName(msgName, m_myUID), handler, clientData);
    condDebugMsg(result != IPC_OK,
                 "IpcFacade:subscribeDataCentral", " for message name \"" << msgName << "\" failed, IPC_errno = " << IPC_errno);
    return result;
  }

}
