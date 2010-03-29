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
#include <sstream>

#include "tinyxml.h"

namespace PLEXIL {

/**
 * @brief Constructor.
 * @param execInterface Reference to the parent AdapterExecInterface object.
 */
IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface) :
  InterfaceAdapter(execInterface), m_ipcFacade(), m_lookupSem(), m_cmdMutex(), m_listener(*this), m_pendingLookupSerial(0), m_pendingLookupDestination(NULL),
      m_messageQueues(execInterface), m_externalLookups() {
}

/**
 * @brief Constructor from configuration XML.
 * @param execInterface Reference to the parent AdapterExecInterface object.
 * @param xml A const pointer to the TiXmlElement describing this adapter
 */
IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface, const TiXmlElement * xml) :
  InterfaceAdapter(execInterface, xml), m_ipcFacade(), m_lookupSem(), m_cmdMutex(), m_listener(*this), m_pendingLookupSerial(0), m_pendingLookupDestination(
      NULL), m_messageQueues(execInterface), m_externalLookups() {

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
    parseExternalLookups(xml->FirstChildElement("ExternalLookups"));
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

  if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX())) {
    // Set up to receive this lookup
    m_activeLookupListeners[nameLabel.toString().substr(LOOKUP_PREFIX().size())] = stateKey;
  } else if (hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX())) {
    // Set up to receive this lookup
    m_activeChangeLookupListeners[nameLabel.toString().substr(LOOKUP_ON_CHANGE_PREFIX().size())] = stateKey;
  }
  // *** TODO: implement receiving planner update
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

  if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX())) {
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
  ExternalLookupMap::iterator it = m_externalLookups.find(state.first);
  if (it != m_externalLookups.end()) {
    debugMsg("IpcAdapter:lookupNow", " returning external lookup " << nameLabel.toString() << " with internal value " << it->second);
    dest.assign(1, it->second);
  } else {
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
  if (name == SEND_MESSAGE_COMMAND())
    executeSendMessageCommand(name, args, dest, ack);
  // Check for SendReturnValue command
  else if (name == SEND_RETURN_VALUE_COMMAND())
    executeSendReturnValueCommand(name, args, dest, ack);
  // Check for ReceiveMessage command
  else if (name == RECEIVE_MESSAGE_COMMAND())
    executeReceiveMessageCommand(name, args, dest, ack);
  // Check for ReceiveCommand command
  else if (name == RECEIVE_COMMAND_COMMAND())
    executeReceiveCommandCommand(name, args, dest, ack);
  else if (name == GET_PARAMETER_COMMAND())
    executeGetParameterCommand(name, args, dest, ack);
  // Check for UpdateLookup command
  else if (name == UPDATE_LOOKUP_COMMAND())
    executeUpdateLookupCommand(name, args, dest, ack);
  else
    // general case
    executeDefaultCommand(name, args, dest, ack);
}
/**
 * @brief Abort the pending command with the supplied name and arguments.
 * @param cmdName The LabelString representing the command name.
 * @param cmdArgs The command arguments expressed as doubles.
 * @param cmdAck The acknowledgment of the pending command
 * @param abrtAck The expression in which to store an acknowledgment of command abort.
 * @note Derived classes may implement this method.  The default method causes an assertion to fail.
 */

void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck) {
  //TODO: implement unique command IDs for referencing command instances
  assertTrueMsg(cmdName == RECEIVE_MESSAGE_COMMAND() || cmdName == RECEIVE_COMMAND_COMMAND(),
      "IpcAdapter: Only ReceiveMessage and ReceiveCommand commands can be aborted");
  assertTrueMsg(cmdArgs.size() == 1,
      "IpcAdapter: Aborting ReceiveMessage requires exactly one argument");
  assertTrueMsg(LabelStr::isString(cmdArgs.front()),
      "IpcAdapter: The argument to the ReceiveMessage abort, " << cmdArgs.front()
      << ", is not a string");
  LabelStr theMessage(cmdArgs.front());

  debugMsg("IpcAdapter:invokeAbort", "Aborting command listener " << theMessage.c_str() << " with ack " << (double) cmdAck << std::endl);
  m_messageQueues.removeRecipient(theMessage, cmdAck);
  m_execInterface.handleValueChange(abrtAck, BooleanVariable::TRUE());
  m_execInterface.notifyOfExternalEvent();
}

//
// Implementation methods
//


/**
 * @brief Helper function for converting message names into the proper format given the command type and a user-defined id.
 */
double IpcAdapter::formatMessageName(const LabelStr& name, const LabelStr& command, int id) {
  std::stringstream ss;
  if (command == RECEIVE_COMMAND_COMMAND()) {
    ss << COMMAND_PREFIX() << name.toString();
  } else if (command == GET_PARAMETER_COMMAND()) {
    ss << PARAM_PREFIX() << name.toString();
  } else {
    ss << name.getKey();
  }
  ss << '_' << id;
  return LabelStr(ss.str()).getKey();
}
/**
 * @brief Helper function for converting message names into the proper format given the command type.
 */
double IpcAdapter::formatMessageName(const LabelStr& name, const LabelStr& command) {
  return formatMessageName(name, command, 0);
}

/**
 * @brief Helper function for converting message names into the proper format given the command type.
 */
double IpcAdapter::formatMessageName(const char* name, const LabelStr& command) {
  return formatMessageName(LabelStr(name), command, 0);
}

/**
 * @brief handles SEND_MESSAGE_COMMAND commands from the exec
 */
void IpcAdapter::executeSendMessageCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
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

/**
 * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
 */
void IpcAdapter::executeSendReturnValueCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
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
  serial = m_ipcFacade.publishReturnValues(serial, LabelStr(front_string), argsToDeliver);
  assertTrue(serial != IpcFacade::ERROR_SERIAL(), "Return values failed to be sent");

  // store ack
  m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
  m_execInterface.notifyOfExternalEvent();
  debugMsg("IpcAdapter:executeCommand", " return value sent.");
}

/**
 * @brief handles SEND_RETURN_VALUE_COMMAND commands from the exec
 */
void IpcAdapter::executeReceiveMessageCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
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

/**
 * @brief handles RECEIVE_COMMAND_COMMAND commands from the exec
 */
void IpcAdapter::executeReceiveCommandCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
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
}

/**
 * @brief handles GET_PARAMETER_COMMAND commands from the exec
 */
void IpcAdapter::executeGetParameterCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
  // Check for one argument, the message
  assertTrueMsg(args.size() == 1 || args.size() == 2,
      "IpcAdapter: The " << name.c_str() << " command requires either one or two arguments");
  assertTrueMsg(LabelStr::isString(args.front()),
      "IpcAdapter: The first argument to the " << name.c_str() << " command, " << args.front()
      << ", is not a string");
  std::list<double>::const_iterator it = ++args.begin();
  int id;
  if (it == args.end()) {
    id = 0;
  } else {
    id = static_cast<int> (*it);
    assertTrueMsg(id == *it,
        "IpcAdapter: The second argument to the " << name.c_str() << " command, " << *it
        << ", is not an integer");
    assertTrueMsg(id >= 0,
        "IpcAdapter: The second argument to the " << name.c_str() << " command, " << *it
        << ", is not a valid index");
  }
  LabelStr command(formatMessageName(args.front(), name, id));
  m_messageQueues.addRecipient(command, ack, dest);
  m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
  m_execInterface.notifyOfExternalEvent();
  debugMsg("IpcAdapter:executeCommand", " message handler for \"" << command.c_str() << "\" registered.");
}

void IpcAdapter::executeUpdateLookupCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
  assertTrueMsg(args.size() == 2,
      "IpcAdapter: The " << name.c_str() << " command requires exactly two arguments");
  assertTrueMsg(LabelStr::isString(args.front()),
      "IpcAdapter: The argument to the SendMessage command, " << args.front()
      << ", is not a string");
  ThreadMutexGuard guard(m_cmdMutex);
  ExternalLookupMap::iterator it = m_externalLookups.find(args.front());
  LabelStr lookup_name(args.front());
  assertTrueMsg(it != m_externalLookups.end(),
      "IpcAdapter: The external lookup " << lookup_name.c_str() << " is not defined");
  //Set value internally
  it->second = *(++args.begin());
  //send telemetry
  assertTrueMsg(m_ipcFacade.publishTelemetry(lookup_name.toString(), std::list<double>(1, it->second)) != IpcFacade::ERROR_SERIAL(),
      "IpcAdapter: publishTelemetry returned status \"" << m_ipcFacade.getError() << "\"");
  m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
  m_execInterface.notifyOfExternalEvent();
}

/**
 * @brief handles all other commands from the exec
 */
void IpcAdapter::executeDefaultCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack) {
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

void IpcAdapter::parseExternalLookups(const TiXmlElement* external) {
  if (external != NULL) {

    //process external lookups
    const TiXmlElement* lookup = external->FirstChildElement("Lookup");
    const char* name = NULL;
    const char* type = NULL;
    const char* def = NULL;
    while (lookup != 0) {
      name = lookup->Attribute("name");
      type = lookup->Attribute("type");
      def = lookup->Attribute("value");
      debugMsg("IpcAdapter:parseExternalLookups",
          "External Lookup: name=\"" << name << "\" type=\"" <<type
          << "\" default=\"" << def << "\"");
      assertTrueMsg(name != NULL && type != NULL && def != NULL,
          "IpcAdapter:parseExternalLookups: Lookup element attribute 'name', 'type', or 'value' missing");
      m_execInterface.registerLookupInterface(LabelStr(name), getId());
      if (strcmp(type, "String") == 0) {
        m_externalLookups.insert(std::pair<double, double>(LabelStr(name).getKey(), LabelStr(def).getKey()));
      } else {
        char* end = 0;
        double d = strtod(def, &end);
        assertTrueMsg(*end == 0, "IpcAdapter:parseExternalLookups: \"" << def
            << "\" is not valid for type " << type);
        m_externalLookups.insert(std::pair<double, double>(LabelStr(name).getKey(), d));
      }
      lookup = lookup->NextSiblingElement("Lookup");
    }
  }
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
  m_messageQueues.addMessage(msg, uid_lbl.getKey());
  int i = 0;
  for (std::vector<const PlexilMsgBase*>::const_iterator it = ++msgs.begin(); it != msgs.end(); it++, i++) {
    const LabelStr& paramLbl(formatMessageName(uid_lbl, GET_PARAMETER_COMMAND(), i));
    switch ((int) (*it)->msgType) {
    case (int) PlexilMsgType_NumericValue: {
      const PlexilNumericValueMsg* param = reinterpret_cast<const PlexilNumericValueMsg*> (*it);
      debugMsg("IpcAdapter:handleCommandSequence",
          " Sending numeric parameter \"" << param->doubleValue << "\" to the command queue");
      m_messageQueues.addMessage(paramLbl, param->doubleValue);
      break;
    }
    case (int) PlexilMsgType_StringArray: {
      const PlexilStringArrayMsg* param = reinterpret_cast<const PlexilStringArrayMsg*> (*it);
      StoredArray array(param->arraySize);
      for (int j = 0; j < param->arraySize; j++) {
        array[j] = LabelStr(param->stringArray[j]).getKey();
      }
      m_messageQueues.addMessage(paramLbl, array.getKey());
      break;
    }
    case (int) PlexilMsgType_NumericArray: {
      const PlexilNumericArrayMsg* param = reinterpret_cast<const PlexilNumericArrayMsg*> (*it);
      StoredArray array(param->arraySize);
      for (int j = 0; j < param->arraySize; j++) {
        array[j] = param->doubleArray[j];
      }
      m_messageQueues.addMessage(paramLbl, array.getKey());
    }
    default: { //assume string
      const PlexilStringValueMsg* param = reinterpret_cast<const PlexilStringValueMsg*> (*it);
      assertTrueMsg(param != NULL, "Non-numeric non-string parameter sent with command \""
          << header->stringValue << "\"");
      debugMsg("IpcAdapter:handleCommandSequence",
          " Sending parameter \"" << param->stringValue << "\" to the command queue");
      m_messageQueues.addMessage(paramLbl, LabelStr(param->stringValue).getKey());
    }
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
 * @brief Process a LookupNow. Ignores any lookups that are not defined in config
 */
void IpcAdapter::handleLookupNow(const std::vector<const PlexilMsgBase*>& msgs) {

  debugMsg("IpcAdapter:handleLookupNow", " received message. processing as LookupNow");
  const PlexilStringValueMsg* msg = reinterpret_cast<const PlexilStringValueMsg*> (msgs.front());
  LabelStr lookup(msg->stringValue);
  ThreadMutexGuard guard(m_cmdMutex);
  ExternalLookupMap::iterator it = m_externalLookups.find(lookup.getKey());
  if (it == m_externalLookups.end()) {
    debugMsg("IpcAdapter:handleLookupNow", " undefined external lookup" << msg->stringValue
        << ", returning UNKNOWN");
  } else {
    debugMsg("IpcAdapter:handleLookupNow", " Publishing value of external lookup \"" << msg->stringValue
        << "\" with internal value '" << (*it).second << "'");
    m_ipcFacade.publishReturnValues(msg->header.serial, msg->header.senderUID, std::list<double>(1, (*it).second));
  }
}

/**
 * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
 */
double IpcAdapter::parseReturnValues(const std::vector<const PlexilMsgBase*>& msgs) {
  size_t nValues = msgs[0]->count;
  checkError(nValues == 1, "PlexilMsgType_ReturnValue may only have one parameter");
  double value;
  switch (msgs[1]->msgType) {
  case PlexilMsgType_NumericValue:
    value = ((PlexilNumericValueMsg*) msgs[1])->doubleValue;
    break;
  case PlexilMsgType_StringValue:
    value = LabelStr(((PlexilStringValueMsg*) msgs[1])->stringValue).getKey();
    break;
  case PlexilMsgType_StringArray: {
    const PlexilStringArrayMsg* msg = reinterpret_cast<const PlexilStringArrayMsg*> (msgs[1]);
    std::vector<double> str_array(msg->arraySize);
    for (int i = 0; i < msg->arraySize; i++) {
      str_array[i] = LabelStr(msg->stringArray[i]).getKey();
    }
    value = StoredArray(str_array).getKey();
    break;
  }
  case PlexilMsgType_NumericArray: {
    const PlexilNumericArrayMsg* msg = reinterpret_cast<const PlexilNumericArrayMsg*> (msgs[1]);
    std::vector<double> num_array(msg->arraySize);
    for (int i = 0; i < msg->arraySize; i++) {
      num_array[i] = msg->doubleArray[i];
      debugMsg("IpcAdapter:parseReturnValues",
          " processing array numeric value: " << msg->doubleArray[i]);
    }
    value = StoredArray(num_array).getKey();
    break;
  }
  default:
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter:parseReturnValues: Unknown parameter type "
        << msgs[1]->msgType)
    ;
  }
  return value;
}

IpcAdapter::MessageListener::MessageListener(IpcAdapter& adapter) :
  m_adapter(adapter) {
}
IpcAdapter::MessageListener::~MessageListener() {
}
void IpcAdapter::MessageListener::ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs) {
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
    debugMsg("IpcAdapter:handleIpcMessage", " processing as return value")
    ;
    m_adapter.handleReturnValuesSequence(msgs);
    break;

  case PlexilMsgType_LookupNow:
    m_adapter.handleLookupNow(msgs);
    break;

    //unhandled so far
  default:
    debugMsg("IpcAdapter::enqueueMessageSequence",
        "Unhandled leader message type " << msgs.front()->msgType << ". Disregarding")
    ;
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
