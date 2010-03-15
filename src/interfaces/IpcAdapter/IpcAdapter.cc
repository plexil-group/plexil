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

#include "IpcAdapter.hh"
#include "ipc-data-formats.h"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "CoreExpressions.hh"
#include "PlexilXmlParser.hh"
#include "StoredArray.hh"
#include "ThreadSpawn.hh"
#include <string>

#include "tinyxml.h"

namespace PLEXIL {

/**
 * @brief Constructor.
 * @param execInterface Reference to the parent AdapterExecInterface object.
 */
IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface) :
  InterfaceAdapter(execInterface),
  m_ipcFacade(),
  m_lookupSem(),
  m_cmdMutex(),
  m_listener(*this),
  m_pendingLookupSerial(0),
  m_pendingLookupDestination(NULL),
  m_messageQueues(execInterface) {
}

/**
 * @brief Constructor from configuration XML.
 * @param execInterface Reference to the parent AdapterExecInterface object.
 * @param xml A const pointer to the TiXmlElement describing this adapter
 */
IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface, const TiXmlElement * xml) :
  InterfaceAdapter(execInterface, xml),
  m_ipcFacade(),
  m_lookupSem(),
  m_cmdMutex(),
  m_listener(*this),
  m_pendingLookupSerial(0),
  m_pendingLookupDestination(NULL),
  m_messageQueues(execInterface) {

}

/**
 * @brief Destructor.
 */
IpcAdapter::~IpcAdapter() {
}

//
// API to ExecApplication
//

/**
 * @brief Initializes the adapter, possibly using its configuration data.
 * @return true if successful, false otherwise.
 */
bool IpcAdapter::initialize() {
  // Get taskName, serverName from XML, if supplied
  const char* taskName = NULL;
  const char* serverName = NULL;
  const char* acceptDuplicatesStr = NULL;

  const TiXmlElement* xml = this->getXml();
  if (xml != NULL) {
    taskName = xml->Attribute("TaskName");
    serverName = xml->Attribute("Server");
    acceptDuplicatesStr = xml->Attribute("AllowDuplicateMessages");
  }

  // Use defaults if necessary
  if (taskName == NULL) {
    taskName = IpcFacade::getUID().c_str();
  }
  if (serverName == NULL) {
    serverName = "localhost";
  }
  if (acceptDuplicatesStr != NULL) {
    if (std::strcmp(acceptDuplicatesStr, "true") == 0) {
      m_messageQueues.setAllowDuplicateMessages(true);
    } else if (std::strcmp(acceptDuplicatesStr, "false") == 0) {
      m_messageQueues.setAllowDuplicateMessages(false);
    } else {
      assertTrueMsg(ALWAYS_FAIL, "IpcAdapter: " << acceptDuplicatesStr << " invalid for \"AllowDuplicateMessages\"."
          << " Must be 'true' or 'false'.");
    }
  } else {
    //debugging only. set to true for release
    m_messageQueues.setAllowDuplicateMessages(false);
  }

  debugMsg("IpcAdapter:initialize",
      " Connecting module " << taskName <<
      " to central server at " << serverName);

  // init IPC
  assertTrueMsg(m_ipcFacade.initilize(taskName, serverName) == IPC_OK,
      "IpcAdapter: Unable to connect to the central server at " << serverName);

  // *** TODO: register lookup names for getting commands & msgs ***
  // Register with AdapterExecInterface
  m_execInterface.defaultRegisterAdapter(getId());

  debugMsg("IpcAdapter:initialize", " succeeded");
  return true;
}

/**
 * @brief Starts the adapter, possibly using its configuration data.
 * @return true if successful, false otherwise.
 */
bool IpcAdapter::start() {
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
bool IpcAdapter::stop() {
  int errorno;
  m_ipcFacade.stop();

  debugMsg("IpcAdapter:stop", " succeeded");
  return true;
}

/**
 * @brief Resets the adapter.
 * @return true if successful, false otherwise.
 */
bool IpcAdapter::reset() {
  // No-op (?)
  return true;
}

/**
 * @brief Shuts down the adapter, releasing any of its resources.
 * @return true if successful, false otherwise.
 */
bool IpcAdapter::shutdown() {
  m_ipcFacade.shutdown();
  debugMsg("IpcAdapter:shutdown", " succeeded");
  return true;
}

/**
 * @brief Register one LookupOnChange.
 * @param uniqueId The unique ID of this lookup.
 * @param stateKey The state key for this lookup.
 * @param tolerances A vector of tolerances for the LookupOnChange.
 */

void IpcAdapter::registerChangeLookup(const LookupKey& uniqueId, const StateKey& stateKey, const std::vector<double>& tolerances) {
  State state;
  assertTrueMsg(getState(stateKey, state),
      "IpcAdapter::registerChangeLookup: Internal error: state not found!");
  LabelStr nameLabel(state.first);
  const std::vector<double>& params = state.second;
  size_t nParams = params.size();
  debugMsg("IpcAdapter:registerChangeLookup",
      " for state " << nameLabel.toString()
      << " with " << nParams << " parameters");

  if (hasPrefix(nameLabel.toString(), MESSAGE_PREFIX())) {
    // Set up to receive this message
    m_activeMessageListeners[nameLabel.toString().substr(MESSAGE_PREFIX().size())] = stateKey;
  } else if (hasPrefix(nameLabel.toString(), COMMAND_PREFIX())) {
    // Set up to receive this command
    m_activeCommandListeners[nameLabel.toString().substr(COMMAND_PREFIX().size())] = stateKey;
  } else if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX())) {
    // Set up to receive this lookup
    m_activeLookupListeners[nameLabel.toString().substr(LOOKUP_PREFIX().size())] = stateKey;
  } else if (hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX())) {
    // Set up to receive this lookup
    m_activeChangeLookupListeners[nameLabel.toString().substr(LOOKUP_ON_CHANGE_PREFIX().size())] = stateKey;
  }
  // *** TODO: implement receiving planner update
  else {
//    // Send a request
//
//    // Construct the messages
//    // Leader
//    uint32_t serial = getSerialNumber();
//    struct PlexilStringValueMsg leader = { { PlexilMsgType_LookupOnChange, nParams, serial, m_myUID.c_str() }, nameLabel.c_str() };
//    // Construct parameter messages
//    PlexilMsgBase* paramMsgs[nParams];
//    for (size_t i = 0; i < nParams; i++) {
//      double param = params[i];
//      PlexilMsgBase* paramMsg;
//      if (LabelStr::isString(param)) {
//        // string
//        struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
//        strMsg->stringValue = LabelStr(param).c_str();
//        paramMsg = (PlexilMsgBase*) strMsg;
//        paramMsg->msgType = PlexilMsgType_StringValue;
//      } else if (StoredArray::isKey(param)) {
//        // array
//        assertTrueMsg(ALWAYS_FAIL,
//            "IpcAdapter: Array values are not yet implemented");
//      } else {
//        // number or Boolean
//        struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
//        numMsg->doubleValue = param;
//        paramMsg = (PlexilMsgBase*) numMsg;
//        paramMsg->msgType = PlexilMsgType_NumericValue;
//      }
//
//      // Fill in common fields
//      paramMsg->count = i;
//      paramMsg->serial = serial;
//      paramMsg->senderUID = m_myUID.c_str();
//      paramMsgs[i] = paramMsg;
//    }
//
//    // Take care of bookkeeping
//    m_changeLookups[serial] = stateKey;
//
//    // Send the messages
//    IPC_RETURN_TYPE status;
//    status = IPC_publishData(STRING_VALUE_MSG, (void *) &leader);
//    assertTrueMsg(status == IPC_OK,
//        "IpcAdapter::registerChangeLookup: IPC Error, IPC_errno = " << IPC_errno);
//    for (size_t i = 0; i < nParams; i++) {
//      status = IPC_publishData(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType), paramMsgs[i]);
//      assertTrueMsg(status == IPC_OK,
//          "IpcAdapter::registerChangeLookup: IPC Error, IPC_errno = " << IPC_errno);
//    }
//    // free the parameter packets
//    for (size_t i = 0; i < nParams; i++) {
//      PlexilMsgBase* m = paramMsgs[i];
//      paramMsgs[i] = NULL;
//      if (m->msgType == PlexilMsgType_NumericValue)
//        delete (PlexilNumericValueMsg*) m;
//      else
//        delete (PlexilStringValueMsg*) m;
//    }
  }
}

/**
 * @brief Terminate one LookupOnChange.
 * @param uniqueId The unique ID of the lookup to be terminated.
 */

void IpcAdapter::unregisterChangeLookup(const LookupKey& uniqueId) {
  debugMsg("IpcAdapter:unregisterChangeLookup", " entered");
  StateToLookupMap::const_iterator sit = this->findLookupKey(uniqueId);
  assertTrueMsg(sit != this->getAsynchLookupsEnd(),
      "IpcAdapter::unregisterChangeLookup: internal error: no state key registered for lookup!");

  debugMsg("IpcAdapter:unregisterChangeLookup", " found state key");

  // Ignore this unless we're removing the last change lookup for this state
  if (sit->second.size() > 1) {
    debugMsg("IpcAdapter:unregisterChangeLookup",
        " there are still active change lookups for this state, ignoring");

    return;
  }

  StateKey key = sit->first;
  State state;
  getState(key, state);
  const LabelStr& nameLabel = state.first;
  const std::string& name = nameLabel.toString();

  debugMsg("IpcAdapter:unregisterChangeLookup", " for " << name);

  if (hasPrefix(nameLabel.toString(), MESSAGE_PREFIX())) {
    // Stop looking for this message
    ActiveListenerMap::iterator it = m_activeMessageListeners.find(name.substr(MESSAGE_PREFIX().size()));
    assertTrueMsg(it != m_activeMessageListeners.end(),
        "IpcAdapter::unregisterChangeLookup: internal error: can't find message \""
        << name.substr(MESSAGE_PREFIX().size()) << "\"");
    m_activeMessageListeners.erase(it);
  } else if (hasPrefix(nameLabel.toString(), COMMAND_PREFIX())) {
    // Stop looking for this command
    ActiveListenerMap::iterator it = m_activeCommandListeners.find(name.substr(COMMAND_PREFIX().size()));
    assertTrueMsg(it != m_activeCommandListeners.end(),
        "IpcAdapter::unregisterChangeLookup: internal error: can't find command \""
        << name.substr(COMMAND_PREFIX().size()) << "\"");
    m_activeCommandListeners.erase(it);
  } else if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX())) {
    // Stop looking for this lookup
    ActiveListenerMap::iterator it = m_activeLookupListeners.find(name.substr(LOOKUP_PREFIX().size()));
    assertTrueMsg(it != m_activeLookupListeners.end(),
        "IpcAdapter::unregisterChangeLookup: internal error: can't find lookup \""
        << name.substr(LOOKUP_PREFIX().size()) << "\"");
    m_activeLookupListeners.erase(it);
  } else if (hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX())) {
    // Stop looking for this lookup
    ActiveListenerMap::iterator it = m_activeChangeLookupListeners.find(name.substr(LOOKUP_ON_CHANGE_PREFIX().size()));
    assertTrueMsg(it != m_activeChangeLookupListeners.end(),
        "IpcAdapter::unregisterChangeLookup: internal error: can't find change lookup \""
        << name.substr(LOOKUP_ON_CHANGE_PREFIX().size()) << "\"");
    m_activeChangeLookupListeners.erase(it);
  }
  // *** TODO: implement receiving planner update
  else {
//    // send a TerminateChangeLookup message with the serial #
//    // of the original request
//    debugMsg("IpcAdapter:unregisterChangeLookup", " sending TerminateChangeLookupMessage");
//    IpcChangeLookupMap::iterator it = m_changeLookups.begin();
//    while (it != m_changeLookups.end()) {
//      if (it->second == key)
//        break;
//      it++;
//    }
//    assertTrueMsg(it != m_changeLookups.end(),
//        "IpcAdapter::unregisterChangeLookup: internal error: lookup not found");
//    const uint32_t serial = it->first;
//    PlexilMsgBase msg = { PlexilMsgType_TerminateChangeLookup, 0, serial, m_myUID.c_str() };
//    IPC_RETURN_TYPE status;
//    status = IPC_publishData(MSG_BASE, (void*) &msg);
//    assertTrueMsg(status == IPC_OK,
//        "IpcAdapter::unregisterChangeLookup: IPC Error, IPC_errno = " << IPC_errno);
//
//    // clean up table
//    m_changeLookups.erase(it);
  }
  debugMsg("IpcAdapter:unregisterChangeLookup", " completed");
}

/**
 * @brief Perform an immediate lookup of the requested state.
 * @param stateKey The state key for this lookup.
 * @param dest A (reference to a) vector of doubles where the result is to be stored.
 * @note Derived classes may implement this method.  The default method causes an assertion to fail.
 */

void IpcAdapter::lookupNow(const StateKey& stateKey, std::vector<double>& dest) {
  State state;
  assertTrueMsg(getState(stateKey, state),
      "IpcAdapter::lookupNow: Internal error: state not found!");
  LabelStr nameLabel(state.first);
  const std::vector<double>& params = state.second;
  size_t nParams = params.size();
  debugMsg("IpcAdapter:lookupNow",
      " for state " << nameLabel.toString()
      << " with " << nParams << " parameters");

  //send lookup message
  std::list<double> paramList;
  for (std::vector<double>::const_iterator it = params.begin(); it != params.end(); it++) {
    paramList.push_back((*it));
  }
  m_pendingLookupDestination = &dest;
  m_cmdMutex.lock();
  m_pendingLookupSerial = m_ipcFacade.publishLookupNow(nameLabel, paramList);
  m_cmdMutex.unlock();

  // Wait for results
  // *** TODO: check for error
  m_lookupSem.wait();

  // Clean up
  m_pendingLookupSerial = 0;
  m_pendingLookupDestination = NULL;
}

/**
 * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
 * @param node The Node requesting the update.
 * @param valuePairs A map of <LabelStr key, value> pairs.
 * @param ack The expression in which to store an acknowledgement of completion.
 * @note Derived classes may implement this method.  The default method causes an assertion to fail.
 */

void IpcAdapter::sendPlannerUpdate(const NodeId& node, const std::map<double, double>& valuePairs, ExpressionId ack) {
  assertTrueMsg(ALWAYS_FAIL,
      "IpcAdapter: sendPlannerUpdate is not yet implemented");
}

/**
 * @brief Execute a command with the requested arguments.
 * @param name The LabelString representing the command name.
 * @param args The command arguments expressed as doubles.
 * @param dest The expression in which to store any value returned from the command.
 * @param ack The expression in which to store an acknowledgement of command transmission.
 */

void IpcAdapter::executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
  // Check for SendMessage command
  if (name == SEND_MESSAGE_COMMAND()) {
    // Check for one argument, the message
    assertTrueMsg(args.size() == 1,
        "IpcAdapter: The SendMessage command requires exactly one argument");
    assertTrueMsg(LabelStr::isString(args.front()),
        "IpcAdapter: The argument to the SendMessage command, " << args.front()
        << ", is not a string");
    LabelStr theMessage(args.front());
    debugMsg("IpcAdapter:executeCommand",
        " SendMessage(\"" << theMessage.c_str() << "\")");
    assertTrue(m_ipcFacade.publishMessage(theMessage) != IpcFacade::ERROR_SERIAL(), "Message publish failed");
    // store ack
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("IpcAdapter:executeCommand", " message \"" << theMessage.c_str() << "\" sent.");
  }
  // Check for SendReturnValue command
  else if (name == SEND_RETURN_VALUE_COMMAND()) {
    // Check for one argument, the message
    assertTrueMsg(args.size() >= 2,
        "IpcAdapter: The SendReturnValue command requires at least two arguments.");

    assertTrueMsg(LabelStr::isString(args.front()),
        "IpcAdapter: The first argument to the SendReturnValue command, " << args.front()
        << ", is not a string");
    const LabelStr& front(args.front());
    uint32_t serial;
    //grab serial from parameter
    std::string front_string = front.toString();
    std::string::size_type sep_pos = front_string.find(SERIAL_UID_SEPERATOR(), 1);
    assertTrueMsg(sep_pos != std::string::npos, "Could not find UID seperator in first parameter of return value");
    serial = atoi(front_string.substr(0, sep_pos).c_str());
    front_string = front_string.substr(sep_pos + 1, front_string.size());
    debugMsg("IpcAdapter:executeCommand",
        " SendReturnValue(sender_serial:\"" << serial << "\" \"" << front_string.c_str() << "\")");
    //erase first parameter
    std::list<double> argsToDeliver = args;
    argsToDeliver.erase(argsToDeliver.begin());
    //publish
    serial =  m_ipcFacade.publishReturnValues(serial, LabelStr(front_string), argsToDeliver);
    assertTrue(serial != IpcFacade::ERROR_SERIAL(), "Return values failed to be sent");

    // store ack
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("IpcAdapter:executeCommand", " return value sent.");
  }
  // Check for ReceiveMessage command
  else if (name == RECEIVE_MESSAGE_COMMAND()) {
    // Check for one argument, the message
    assertTrueMsg(args.size() == 1,
        "IpcAdapter: The ReceiveMessage command requires exactly one argument");
    assertTrueMsg(LabelStr::isString(args.front()),
        "IpcAdapter: The argument to the SendMessage command, " << args.front()
        << ", is not a string");
    LabelStr theMessage(args.front());
    m_messageQueues.addRecipient(theMessage, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("IpcAdapter:executeCommand", " message handler for \"" << theMessage.c_str() << "\" registered.");
  }
  // Check for ReceiveCommand command
  else if (name == RECEIVE_COMMAND_COMMAND() || name == GET_PARAMETER_COMMAND()) {
    // Check for one argument, the message
    assertTrueMsg(args.size() == 1,
        "IpcAdapter: The " << name.c_str() << " command requires exactly one argument");
    assertTrueMsg(LabelStr::isString(args.front()),
        "IpcAdapter: The argument to the SendMessage command, " << args.front()
        << ", is not a string");
    LabelStr command(formatMessageName(args.front(), name));
    m_messageQueues.addRecipient(command, ack, dest);
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("IpcAdapter:executeCommand", " message handler for \"" << command.c_str() << "\" registered.");
  } else  // general case
  {
    debugMsg("IpcAdapter:executeCommand", " for \"" << name.c_str()
        << "\", destination expression is " << dest);

    //lock mutex to ensure no return values are processed while the command is being
    //sent and logged
    ThreadMutexGuard guard(m_cmdMutex);
    if (!args.empty())
      debugMsg("IpcAdapter:executeCommand", " first parameter is \"" << args.front()
          << "\"");
    uint32_t serial = m_ipcFacade.publishCommand(name, args);
    // log ack and return variables in case we get values for them
    m_pendingCommands[serial] = std::pair<ExpressionId, ExpressionId>(dest, ack);
    assertTrueMsg(serial != IpcFacade::ERROR_SERIAL(),
        "IpcAdapter::executeCommand: IPC Error, IPC_errno = " << m_ipcFacade.getError());
    // store ack
    m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
    m_execInterface.notifyOfExternalEvent();
    debugMsg("IpcAdapter:executeCommand", " command \"" << name.c_str() << "\" sent.");
  }
}
/**
 * @brief Abort the pending command with the supplied name and arguments.
 * @param name The LabelString representing the command name.
 * @param args The command arguments expressed as doubles.
 * @param dest The destination of the pending command
 * @param ack The expression in which to store an acknowledgment of command abort.
 * @note Derived classes may implement this method.  The default method causes an assertion to fail.
 */

void IpcAdapter::invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId cmd_ack, ExpressionId ack) {
  //TODO: implement unique command IDs for referencing command instances
  assertTrueMsg(name == RECEIVE_MESSAGE_COMMAND() || name == RECEIVE_COMMAND_COMMAND(),
      "IpcAdapter: Only ReceiveMessage and ReceiveCommand commands can be aborted");
  assertTrueMsg(args.size() == 1,
      "IpcAdapter: Aborting ReceiveMessage requires exactly one argument");
  assertTrueMsg(LabelStr::isString(args.front()),
      "IpcAdapter: The argument to the ReceiveMessage abort, " << args.front()
      << ", is not a string");
  LabelStr theMessage(args.front());

  debugMsg("IpcAdapter:invokeAbort", "Aborting command listener " << theMessage.c_str() << " with ack " << (double) cmd_ack << std::endl);
  m_messageQueues.removeRecipient(theMessage, cmd_ack);
  m_execInterface.handleValueChange(ack, BooleanVariable::TRUE());
  m_execInterface.notifyOfExternalEvent();
}

//
// Implementation methods
//


/**
 * @brief Helper function for converting message names into the proper format given the command type.
 */
double IpcAdapter::formatMessageName(const LabelStr& name, const LabelStr& command) {
  if (command == RECEIVE_COMMAND_COMMAND()) {
    return LabelStr(COMMAND_PREFIX() + name.toString()).getKey();
  } else if (command == GET_PARAMETER_COMMAND()) {
    return LabelStr(PARAM_PREFIX() + name.toString()).getKey();
  } else {
    return name.getKey();
  }
}


/**
 * @brief Helper function for converting message names into the proper format given the command type.
 */
double IpcAdapter::formatMessageName(const char* name, const LabelStr& command) {
  return formatMessageName(LabelStr(name), command);
}

/**
 * @brief Send a single message to the Exec's queue and free the message
 */
void IpcAdapter::enqueueMessage(const PlexilMsgBase* msgData) {
  assertTrueMsg(msgData != NULL,
      "IpcAdapter::enqueueMessage: msgData is null");

  switch (msgData->msgType) {
  case PlexilMsgType_NotifyExec:
    m_execInterface.notifyOfExternalEvent();
    break;

    // AddPlan is a PlexilStringValueMsg
  case PlexilMsgType_AddPlan: {
    const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
    assertTrueMsg(stringMsg->stringValue != NULL,
        "IpcAdapter::enqueueMessage: AddPlan message contains null plan string");

    // parse XML into node structure
    PlexilXmlParser p;
    try {
      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), false);
      m_execInterface.handleAddPlan(root, EMPTY_LABEL());
      // Always notify immediately when adding a plan
      m_execInterface.notifyOfExternalEvent();
    } catch (const ParserException& e) {
      std::cerr << "Error parsing plan: \n" << e.what() << std::endl;
    }
  }

    // AddPlanFile is a PlexilStringValueMsg
  case PlexilMsgType_AddPlanFile: {
    const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
    assertTrueMsg(stringMsg->stringValue != NULL,
        "IpcAdapter::enqueueMessage: AddPlanFile message contains null file name");

    // parse XML into node structure
    PlexilXmlParser p;
    try {
      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), true);
      m_execInterface.handleAddPlan(root, EMPTY_LABEL());
      // Always notify immediately when adding a plan
      m_execInterface.notifyOfExternalEvent();
    } catch (const ParserException& e) {
      std::cerr << "Error parsing plan from file: \n" << e.what() << std::endl;
    }
  }
    break;

    // AddLibrary is a PlexilStringValueMsg
  case PlexilMsgType_AddLibrary: {
    const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
    assertTrueMsg(stringMsg->stringValue != NULL,
        "IpcAdapter::enqueueMessage: AddLibrary message contains null library node string");

    // parse XML into node structure
    PlexilXmlParser p;
    try {
      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), false);
      m_execInterface.handleAddLibrary(root);
    } catch (const ParserException& e) {
      std::cerr << "Error parsing library node: \n" << e.what() << std::endl;
    }
  }
    break;

    // AddLibraryFile is a PlexilStringValueMsg
  case PlexilMsgType_AddLibraryFile: {
    const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
    assertTrueMsg(stringMsg->stringValue != NULL,
        "IpcAdapter::enqueueMessage: AddLibraryFile message contains null file name");

    // parse XML into node structure
    PlexilXmlParser p;
    try {
      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), true);
      m_execInterface.handleAddLibrary(root);
    } catch (const ParserException& e) {
      std::cerr << "Error parsing library file: \n" << e.what() << std::endl;
    }
  }
    break;

  default:
    assertTrueMsg(ALWAYS_FAIL,
        "IpcAdapter::enqueueMessage: invalid message type " << msgData->msgType)
    ;
  }
}

/**
 * @brief Process a PlexilMsgType_Message packet and free the message
 */
void IpcAdapter::handleMessageMessage(const PlexilStringValueMsg* msgData) {
  assertTrueMsg(msgData != NULL,
      "IpcAdapter::handleMessageMessage: msgData is null")
  assertTrueMsg(msgData->stringValue != NULL,
      "IpcAdapter::handleMessageMessage: stringValue is null")
  m_messageQueues.addMessage(msgData->stringValue);
}

/**
 * @brief Queues the command in the message queue
 */
void IpcAdapter::handleCommandSequence(const std::vector<const PlexilMsgBase*>& msgs) {
  //only support one parameter, the id
  //TODO: support more parameters
  const PlexilStringValueMsg* header = (const PlexilStringValueMsg*) msgs[0];
  std::stringstream uid;
  uid << ((int) header->header.serial) << SERIAL_UID_SEPERATOR() << header->header.senderUID;
  LabelStr uid_lbl(uid.str());
  debugMsg("IpcAdapter:handleCommandSequence",
      " adding \"" << header->stringValue << "\" to the command queue");
  const LabelStr& msg(formatMessageName(header->stringValue, RECEIVE_COMMAND_COMMAND()));
  m_messageQueues.addMessage(msg, StoredArray(1, uid_lbl.getKey()));
  if (msgs.size() > 1) {
    const LabelStr& paramLbl(formatMessageName(uid_lbl, GET_PARAMETER_COMMAND()));
    if (msgs[1]->msgType == PlexilMsgType_NumericValue) {
      const PlexilNumericValueMsg* param = reinterpret_cast<const PlexilNumericValueMsg*>(msgs[1]);
      debugMsg("IpcAdapter:handleCommandSequence",
          " Sending numeric parameter \"" << param->doubleValue << "\" to the command queue");
      m_messageQueues.addMessage(paramLbl, StoredArray(1, param->doubleValue));
    } else { //assume string
      const PlexilStringValueMsg* param = reinterpret_cast<const PlexilStringValueMsg*>(msgs[1]);
      assertTrueMsg(param != NULL, "Non-numeric non-string parameter sent with command \""
          << header->stringValue << "\"");
      debugMsg("IpcAdapter:handleCommandSequence",
          " Sending parameter \"" << param->stringValue << "\" to the command queue");
      m_messageQueues.addMessage(paramLbl, StoredArray(1, LabelStr(param->stringValue).getKey()));
    }
  }
}

/**
 * @brief Process a TelemetryValues message sequence
 */

void IpcAdapter::handleTelemetryValuesSequence(const std::vector<const PlexilMsgBase*>& msgs) {
  const PlexilStringValueMsg* tv = (const PlexilStringValueMsg*) msgs[0];
  State state(LabelStr(tv->stringValue), std::vector<double>(0));
  StateKey key;
  if (!m_execInterface.findStateKey(state, key)) {
    debugMsg("IpcAdapter:handleTelemetryValuesSequence",
        " state \"" << tv->stringValue << "\" is unknown, ignoring");
    return;
  }

  debugMsg("IpcAdapter:handleTelemetryValuesSequence",
      " state \"" << tv->stringValue << "\" found, processing");
  size_t nValues = msgs[0]->count;
  std::vector<double> values(nValues);
  for (size_t i = 1; i <= nValues; i++) {
    if (msgs[i]->msgType == PlexilMsgType_NumericValue)
      values[i - 1] = ((const PlexilNumericValueMsg*) msgs[i])->doubleValue;
    else
      values[i - 1] = LabelStr(((const PlexilStringValueMsg*) msgs[i])->stringValue).getKey();
  }
  m_execInterface.handleValueChange(key, values);
  m_execInterface.notifyOfExternalEvent();
}

/**
 * @brief Process a ReturnValues message sequence
 */

void IpcAdapter::handleReturnValuesSequence(const std::vector<const PlexilMsgBase*>& msgs) {
  const PlexilReturnValuesMsg* rv = (const PlexilReturnValuesMsg*) msgs[0];
  size_t nValues = msgs[0]->count;
  //lock mutex to ensure all sending procedures are complete.
  ThreadMutexGuard guard(m_cmdMutex);
  if (rv->requestSerial == m_pendingLookupSerial) {
    // LookupNow for which we are awaiting data
    debugMsg("IpcAdapter:handleReturnValuesSequence",
        " processing value(s) for a pending LookupNow");
    size_t nExpected = m_pendingLookupDestination->size();
    assertTrueMsg(nExpected == 1,
        "IpcAdapter::handleReturnValuesSequence: Exec expects " << nExpected
        << " values; multiple return values for LookupNow not yet implemented");
    (*m_pendingLookupDestination)[0] = parseReturnValues(msgs);
    // *** TODO: check for error
    m_lookupSem.post();
    return;
  }

  IpcChangeLookupMap::const_iterator it = m_changeLookups.find(rv->requestSerial);
  if (it != m_changeLookups.end()) {
    // Active LookupOnChange
    debugMsg("IpcAdapter:handleReturnValuesSequence",
        " processing value(s) for an active LookupOnChange");
    const StateKey& key = it->second;
    m_execInterface.handleValueChange(key, parseReturnValues(msgs));
    m_execInterface.notifyOfExternalEvent();
    return;
  }
  PendingCommandsMap::iterator cit = m_pendingCommands.find(rv->requestSerial);
  if (cit != m_pendingCommands.end()) {
    // It's a command return value or ack
    ExpressionId& dest = cit->second.first;
    ExpressionId& ack = cit->second.second;
    size_t nValues = msgs[0]->count;
    if (msgs[1]->count == MSG_COUNT_CMD_ACK) {
      assertTrueMsg(nValues == 1,
          "IpcAdapter::handleReturnValuesSequence: command ack requires 1 value, received "
          << nValues);
      debugMsg("IpcAdapter:handleReturnValuesSequence",
          " processing command acknowledgment for expression " << dest);
      m_execInterface.handleValueChange(ack, parseReturnValues(msgs));
      m_execInterface.notifyOfExternalEvent();
    } else if (dest.isId()) {
      debugMsg("IpcAdapter:handleReturnValuesSequence",
          " processing command return value for expression " << dest);
      m_execInterface.handleValueChange(dest, parseReturnValues(msgs));
      m_execInterface.notifyOfExternalEvent();
    } else {
      debugMsg("IpcAdapter:handleReturnValuesSequence",
          " ignoring unwanted command return value");
    }
  } else {
    debugMsg("IpcAdapter:handleReturnValuesSequence",
        " no lookup or command found for sequence");
  }
}

/**
 * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
 */
double IpcAdapter::parseReturnValues(const std::vector<const PlexilMsgBase*>& msgs) {
  size_t nValues = msgs[0]->count;
  if (nValues == 1) {
    debugMsg("IpcAdapter:parseReturnValues",
        " processing single return value");
    double value;
    if (msgs[1]->msgType == PlexilMsgType_NumericValue)
      value = ((PlexilNumericValueMsg*) msgs[1])->doubleValue;
    else
      value = LabelStr(((PlexilStringValueMsg*) msgs[1])->stringValue).getKey();
    return value;
  } else {
    // Convert sequence of values into an array
    debugMsg("IpcAdapter:parseReturnValues",
        " processing array of length " << nValues);
    bool isString = false;
    bool isNumeric = false;
    for (size_t i = 1; i <= nValues; i++) {
      if (msgs[i]->msgType == PlexilMsgType_StringValue) {
        assertTrueMsg(!isNumeric, "IpcAdapter::parseReturnValues: array element types are not consistent");
        isString = true;
      } else {
        assertTrueMsg(!isString, "IpcAdapter::parseReturnValues: array element types are not consistent");
        isNumeric = true;
      }
    }
    StoredArray ary(nValues, Expression::UNKNOWN());
    if (isString) {
      for (size_t i = 0; i < nValues; i++) {
        assertTrueMsg(msgs[i+1]->msgType == PlexilMsgType_StringValue,
            "IpcAdapter:parseReturnValues: value is not a string");
        ary[i] = LabelStr(((PlexilStringValueMsg*) msgs[i + 1])->stringValue).getKey();
      }
    } else {
      for (size_t i = 0; i < nValues; i++) {
        assertTrueMsg(msgs[i+1]->msgType == PlexilMsgType_NumericValue,
            "IpcAdapter:parseReturnValues: value is not a number");
        ary[i] = ((PlexilNumericValueMsg*) msgs[i + 1])->doubleValue;
      }
    }
    return ary.getKey();
  }
}

IpcAdapter::MessageListener::MessageListener(IpcAdapter& adapter) : m_adapter(adapter) {}
IpcAdapter::MessageListener::~MessageListener() {}
void IpcAdapter::MessageListener::ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs){
  if (strcmp(msgs.front()->senderUID, IpcFacade::getUID().c_str()) == 0) {
    debugMsg("IpcAdapter:handleIpcMessage", " ignoring my own outgoing message");
    return;
  }

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
    m_adapter.handleMessageMessage((const PlexilStringValueMsg*) (msgs.front()));
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
    debugMsg("IpcAdapter:handleIpcMessage", " processing as return value");
    m_adapter.handleReturnValuesSequence(msgs);
    break;

    //unhandled so far
  default:
    debugMsg("IpcAdapter::enqueueMessageSequence",
        "Unhandled leader message type " << msgs.front()->msgType << ". Disregarding");
  }
}

//
// Static member functions
//

/**
 * @brief Returns true if the string starts with the prefix, false otherwise.
 */
bool IpcAdapter::hasPrefix(const std::string& s, const std::string& prefix) {
  if (s.size() < prefix.size())
    return false;
  return (0 == s.compare(0, prefix.size(), prefix));
}

}
