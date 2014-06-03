/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "TestExternalInterface.hh"

#include "Command.hh"
#include "Debug.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"
#include "Update.hh"
#include "XMLUtils.hh"
#include "plan-utils.hh"
#include "pugixml.hpp"
#include "stricmp.h"

#include <cmath>
#include <cstring>
#include <limits>
#include <sstream>

namespace PLEXIL
{

  // Forward declarations for local functions
  static std::string getText(const State& c);
  static std::string getText(const State& c, const Value& v);
  static State parseCommand(const pugi::xml_node& cmd);
  static Value parseOneValue(const std::string& type,
                             const std::string& valStr);
  static Value parseParam(const pugi::xml_node& param);
  static void parseParams(const pugi::xml_node& root,
                          std::vector<Value>& dest);
  static Value parseResult(const pugi::xml_node& valXml);
  static State parseState(const pugi::xml_node& elt);
  static Value parseStateValue(const pugi::xml_node& stateXml);


  TestExternalInterface::TestExternalInterface()
    : ExternalInterface()
  {
  }

  TestExternalInterface::~TestExternalInterface()
  {
  }

  void TestExternalInterface::run(const pugi::xml_node& input)
    throw(ParserException)
  {
    checkError(g_exec.isValid(), "Attempted to run a script without an executive.");
    handleInitialState(input);
    pugi::xml_node script = input.child("Script");
    checkError(!script.empty(), "No Script element in Plexilscript.");
    pugi::xml_node scriptElement = script.first_child();
    while (!scriptElement.empty()) {
      // ignore text element (e.g. from <Script> </Script>)
      if (scriptElement.type() == pugi::node_pcdata) {
        // debugMsg("Test:verboseTestOutput", " Ignoring XML PCDATA");
      }

      // state
      else if (strcmp(scriptElement.name(), "State") == 0) {
        handleState(scriptElement);
      }

      // command
      else if (strcmp(scriptElement.name(), "Command") == 0) {
        handleCommand(scriptElement);
      }

      // command ack
      else if (strcmp(scriptElement.name(), "CommandAck") == 0) {
        handleCommandAck(scriptElement);
      }
         
      // command abort
      else if (strcmp(scriptElement.name(), "CommandAbort") == 0) {
        handleCommandAbort(scriptElement);
      }

      // update ack
      else if (strcmp(scriptElement.name(), "UpdateAck") == 0) {
        handleUpdateAck(scriptElement);
      }

      // send plan
      else if (strcmp(scriptElement.name(), "SendPlan") == 0) {
        handleSendPlan(scriptElement);
      }

      // simultaneous
      else if (strcmp(scriptElement.name(), "Simultaneous") == 0) {
        handleSimultaneous(scriptElement);
      }

      // delay
      else if (strcmp(scriptElement.name(), "Delay") == 0) {
        ; // No-op
      }

      // report unknown script element
      else {
        checkError(ALWAYS_FAIL, "Unknown script element '" << scriptElement.name() << "'");
        return;
      }
         
      // step the exec forward
      g_exec->step(currentTime());

      scriptElement = scriptElement.next_sibling();
    }
    // Continue stepping the Exec til quiescent
    while (g_exec->needsStep()) {
      g_exec->step(currentTime());
    }
  }

  // map values from script into a variable expression

  void TestExternalInterface::setVariableValue(const std::string& source,
                                               ExpressionId expr,
                                               const Value& value)
  {
    if (expr != ExpressionId::noId()) {
      checkError(expr->isAssignable(),
                 "Expected variable in \'" << source << "\'");
      ((Assignable *) expr)->setValue(value);
      std::map<ExpressionId, CommandId>::iterator iter;
      if ((iter = m_destToCmdMap.find(expr)) != m_destToCmdMap.end()) {
        m_destToCmdMap.erase(iter);
      }
      else
        std::cerr << "Error in TestExternalInterface: "
                  << "Could not find destination for command " << source
                  << ".  Should never happen!" << std::endl;
    }
  }

  void TestExternalInterface::handleInitialState(const pugi::xml_node& input)
  {
    pugi::xml_node initialState = input.child("InitialState");
    if (initialState) {
      pugi::xml_node state = initialState.first_child();
      // Deal with <InitialState>  </InitialState>
      if (state.type() != pugi::node_pcdata) {
        while (state) {
          State st = parseState(state);
          Value value = parseStateValue(state);
          debugMsg("Test:testOutput",
                   "Creating initial state " << getText(st, value));
          m_states.insert(std::pair<State, Value>(st, value));
          state = state.next_sibling();
        }
      }
    }
    g_exec->step(currentTime());
  }

  void TestExternalInterface::handleState(const pugi::xml_node& elt)
  {
    State st = parseState(elt);
    Value value = parseStateValue(elt);
    m_states[st] = value;
    debugMsg("Test:testOutput",
             "Processing event: " << st << " = " << value);
    StateCacheMap::instance().ensureStateCacheEntry(st, value.valueType())
      ->update(value);
  }

  void TestExternalInterface::handleCommand(const pugi::xml_node& elt)
  {
    State command = parseCommand(elt);
    Value value = parseResult(elt);
    debugMsg("Test:testOutput",
             "Sending command result " << getText(command, value));
    ExpressionUtMap::iterator it = 
      m_executingCommands.find(command);
    checkError(it != m_executingCommands.end(),
               "No currently executing command " << getText(command));
    setVariableValue(getText(command), it->second, value); // erases from m_destToCmdMap
    m_executingCommands.erase(it);
    m_raInterface.releaseResourcesForCommand(command.name());
  }

  void TestExternalInterface::handleCommandAck(const pugi::xml_node& elt)
  {
    State command = parseCommand(elt);
    Value value = parseResult(elt);
    debugMsg("Test:testOutput",
             "Sending command ACK " << getText(command, value));
    ExpressionUtMap::iterator it = m_commandAcks.find(command);
    assertTrueMsg(it != m_commandAcks.end(), 
                  "No command waiting for acknowledgement " << getText(command));
    ((Assignable *) it->second)->setValue(value);
    // Release resources if the command does not have a return value
    if (m_executingCommands.find(command) == m_executingCommands.end())
      m_raInterface.releaseResourcesForCommand(command.name());
  }

  void TestExternalInterface::handleCommandAbort(const pugi::xml_node& elt)
  {
    State command = parseCommand(elt);
    Value value = parseResult(elt);
    debugMsg("Test:testOutput",
             "Sending abort ACK " << getText(command, value));
    ExpressionUtMap::iterator it = 
      m_abortingCommands.find(command);
    assertTrueMsg(it != m_abortingCommands.end(), 
                  "No abort waiting for acknowledgement " << getText(command));
    debugMsg("Test:testOutput",
             "Acknowledging abort into " << it->second);
    ((Assignable *) it->second)->setValue(true);
    m_abortingCommands.erase(it);
  }

  void TestExternalInterface::handleUpdateAck(const pugi::xml_node& elt)
  {
    std::string name(elt.attribute("name").value());
    debugMsg("Test:testOutput", "Sending update ACK " << name);
    std::map<std::string, UpdateId>::iterator it = m_waitingUpdates.find(name);
    checkError(it != m_waitingUpdates.end(),
               "No update from node " << name << " waiting for acknowledgement.");
    ((Assignable *) it->second->getAck())->setValue(true);
    m_waitingUpdates.erase(it);
  }

  void TestExternalInterface::handleSendPlan(const pugi::xml_node& elt)
  {
    const char* filename = elt.attribute("file").value();
    checkError(strlen(filename) > 0,
               "SendPlan element has no file attribute");

    pugi::xml_document* doc = new pugi::xml_document();
    pugi::xml_parse_result parseResult = doc->load_file(filename);
    checkError(parseResult.status == pugi::status_ok, 
               "Error parsing plan file " << elt.attribute("file").value()
               << ": " << parseResult.description());

    debugMsg("Test:testOutput",
             "Sending plan from file " << elt.attribute("file").value());
    PlexilNodeId root =
      PlexilXmlParser::parse(doc->document_element().child("PlexilPlan").child("Node"));
    checkError(g_exec->addPlan(root),
               "Adding plan " << elt.attribute("file").value() << " failed");
  }

  void TestExternalInterface::handleSimultaneous(const pugi::xml_node& elt)
  {
    debugMsg("Test:testOutput", "Processing simultaneous event(s)");
    pugi::xml_node item = elt.first_child();
    while (!item.empty()) {
      // ignore text element (e.g. from <Script> </Script>)
      if (item.type() == pugi::node_pcdata) {
        //debugMsg("Test:verboseTestOutput", " Ignoring XML PCDATA");
      }
      // state
      else if (strcmp(item.name(), "State") == 0) {
        handleState(item);
      }
      // command
      else if (strcmp(item.name(), "Command") == 0) {
        handleCommand(item);
      }
      // command ack
      else if (strcmp(item.name(), "CommandAck") == 0) {
        handleCommandAck(item);
      }
      // command abort
      else if (strcmp(item.name(), "CommandAbort") == 0) {
        handleCommandAbort(item);
      }
      // update ack
      else if (strcmp(item.name(), "UpdateAck") == 0) {
        handleUpdateAck(item);
      }
      // report unknown script element
      else {
        checkError(ALWAYS_FAIL,
                   "Unknown script element '" << item.name() << "' inside <Simultaneous>");
        return;
      }
      item = item.next_sibling();
    }
    debugMsg("Test:testOutput", "End simultaneous event(s)");
  }

  //
  // Script parsing utilities
  //

  static State parseStateInternal(const pugi::xml_node& elt)
  {
    checkError(!elt.attribute("name").empty(),
               "No name attribute in " << elt.name() << " element.");
    std::string name(elt.attribute("name").value());
    std::vector<Value> parms;
    parseParams(elt, parms);
    return State(name, parms);
  }

  static State parseState(const pugi::xml_node& elt)
  {
    checkError(strcmp(elt.name(), "State") == 0,
               "Expected <State> element. Found '" << elt.name() << "'");
    return parseStateInternal(elt);
  }

  // Parses all command-like elements: Command, CommandAck, CommandAbort.
  static State parseCommand(const pugi::xml_node& cmd)
  {
    checkError(strcmp(cmd.name(), "Command") == 0 ||
               strcmp(cmd.name(), "CommandAck") == 0 ||
               strcmp(cmd.name(), "CommandAbort") == 0,
               "Expected <Command> element.  Found '" << cmd.name() << "'");
    return parseStateInternal(cmd);
  }

  static Value parseResult(const pugi::xml_node& cmd)
  {
    pugi::xml_node resXml = cmd.child("Result");
    checkError(!resXml.empty(), "No Result child in <" << cmd.name() << "> element.");
    checkError(!resXml.first_child().empty(), "Empty Result child in <" << cmd.name() << "> element.");
    checkError(!cmd.attribute("type").empty(),
               "No type attribute in <" << cmd.name() << "> element.");
    std::string type(cmd.attribute("type").value());

    // read in the initiial values and parameters
    if (type.rfind("array") == std::string::npos) {
      // Not an array
      return parseOneValue(type, resXml.first_child().value());
    }
    else {
      std::vector<Value> values;
      while (!resXml.empty()) {
        values.push_back(parseOneValue(type, resXml.first_child().value()));
        resXml = resXml.next_sibling();
      }
      return Value(values);
    }
  }

  static void parseParams(const pugi::xml_node& root, 
                          std::vector<Value>& dest)
  {
    pugi::xml_node param = root.child("Param");
    while (!param.empty()) {
      dest.push_back(parseParam(param));
      param = param.next_sibling("Param");
    }
  }

  static Value parseParam(const pugi::xml_node& param)
  {
    checkError(!param.first_child().empty()
               || strcmp(param.attribute("type").value(), "string") == 0,
               "Empty Param child in <" << param.parent().name() << "> element.");
    std::string type(param.attribute("type").value());
    if (!type.empty() &&
        (type == "int" || type == "real" || type == "bool")) {
      double value;
      std::istringstream str(param.first_child().value());
      str >> value;
      return Value(value);
    }
    // string case
    else if (param.first_child().empty()) {
      return Value("");
    }
    else if (0 == strcmp(param.first_child().value(), "UNKNOWN")) {
      return Value();
    }
    else {
      return Value(param.first_child().value());
    }
  }

  static Value parseStateValue(const pugi::xml_node& stateXml)
  {
    // read in values
    std::string type(stateXml.attribute("type").value());
    checkError(!type.empty(),
               "No type attribute in <" << stateXml.name() << "> element");

    pugi::xml_node valXml = stateXml.child("Value");
    checkError(valXml,
               "No <Value> element in <"  << stateXml.name() << "> element");
    if (type.rfind("array") == std::string::npos) {
      // Not an array
      return parseOneValue(type, valXml.first_child().value());
    }
    else {
      std::vector<Value> values;
      while (!valXml.empty()) {
        values.push_back(parseOneValue(type, valXml.first_child().value()));
        valXml = valXml.next_sibling();
      }
      return Value(values);
    }
  }

  // parse in value
  static Value parseOneValue(const std::string& type, 
                             const std::string& valStr)
  {
    // string or string-array
    if (type.find("string") == 0) {
      return Value(valStr);
    }
    // int, int-array, real, real-array
    else if (type.find("int") == 0
             || type.find("real") == 0) {
      double value;
      std::istringstream ss(valStr);
      ss >> value;
      return Value(value);
    }
    // bool or bool-array
    else if (type.find("bool") == 0) {
      if (0 == stricmp(valStr.c_str(), "true"))
        return Value(true);
      else if (0 == stricmp(valStr.c_str(), "false"))
        return Value(false);
      else {
        double value;
        std::istringstream ss(valStr);
        ss >> value;
        return Value(value);
      }
    }
    else {
      checkError(ALWAYS_FAIL, "Unknown type attribute \"" << type << "\"");
      return Value();
    }
  }

  Value TestExternalInterface::lookupNow(const State& state)
  {
    debugMsg("Test:testOutput", "Looking up immediately " << state);
    StateMap::const_iterator it = m_states.find(state);
    if (it == m_states.end()) {
      debugMsg("Test:testOutput", "No state found.  Setting UNKNOWN.");
      it = m_states.insert(std::make_pair(state, Value())).first;
    }
    const Value& value = it->second;
    debugMsg("Test:testOutput", "Returning value " << value);
    return value;
  }

  void TestExternalInterface::subscribe(const State& state)
  {
    debugMsg("Test:testOutput",
             "Registering change lookup for " << state);

    //ignore source, because we don't care about bandwidth here
    StateMap::iterator it = m_states.find(state);
    if (it == m_states.end()) {
      std::pair<State, Value> p = 
        std::make_pair(state, Value());
      m_states.insert(p);
    }
  }

  void TestExternalInterface::unsubscribe(const State& /* state */)
  {}

  void TestExternalInterface::setThresholds(const State& /* state */,
                                            double /* highThreshold */,
                                            double /* lowThreshold */)
  {}

  void TestExternalInterface::batchActions(std::list<CommandId>& commands)
  {
    if (commands.empty())
      return;
      
    std::set<CommandId> acceptCmds;
    m_raInterface.arbitrateCommands(commands, acceptCmds);
    for (std::list<CommandId>::iterator it = commands.begin(); it != commands.end(); ++it) {
      CommandId cmd = *it;
      check_error(cmd.isValid());
         
      if (acceptCmds.find(cmd) != acceptCmds.end()) {
        m_destToCmdMap[cmd->getDest()] = cmd;
        executeCommand(cmd->getName(), cmd->getArgValues(), cmd->getDest(), cmd->getAck());
      }
      else {
        debugMsg("Test:testOutput", 
                 "Permission to execute " << cmd->getName()
                 << " has been denied by the resource arbiter.");
        ((Assignable *) cmd->getAck())->setValue(COMMAND_DENIED);
      }
    }
  }

  void TestExternalInterface::executeCommand(const std::string& name,
                                             const std::vector<Value>& args,
                                             ExpressionId dest,
                                             ExpressionId ack)
  {
    std::vector<Value> realArgs(args.begin(), args.end());
    State cmd(name, realArgs);
    debugMsg("Test:testOutput", "Executing " << getText(cmd) <<
             " into " <<
             (dest.isNoId() ? std::string("noId") : dest->toString()) <<
             " with ack " << ack->toString());
    if (dest.isId())
      m_executingCommands[cmd] = dest;

    // Special handling of the utility commands (a bit of a hack!):
    if (name == "print") {
      print(args);
      ((Assignable *) ack)->setValue(COMMAND_SUCCESS);
      m_raInterface.releaseResourcesForCommand(name);
    }
    else if (name == "pprint") {
      pprint(args);
      ((Assignable *) ack)->setValue(COMMAND_SUCCESS);
      m_raInterface.releaseResourcesForCommand(name);
    }
    else {
      // Usual case - set up for scripted ack value
      m_commandAcks[cmd] = ack;
    }
  }


  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param cmd The command.
   */

  void TestExternalInterface::invokeAbort(const CommandId& command)
  {
    const std::vector<Value>& cmdArgs = command->getArgValues();
    std::vector<Value> realArgs(cmdArgs.begin(), cmdArgs.end());
    State cmd(command->getName(), realArgs);
    debugMsg("Test:testOutput", "Aborting " << getText(cmd));
    m_abortingCommands[cmd] = command->getAbortComplete();
  }

  void TestExternalInterface::updatePlanner(std::list<UpdateId>& updates)
  {
    for (std::list<UpdateId>::const_iterator it = updates.begin(); it != updates.end(); ++it) {
      debugMsg("Test:testOutput", "Received update: ");
      const std::map<std::string, Value>& pairs = (*it)->getPairs();
      for (std::map<std::string, Value>::const_iterator pairIt = pairs.begin(); pairIt != pairs.end(); ++pairIt)
        debugMsg("Test:testOutput", " " << pairIt->first << " => " << pairIt->second);
      m_waitingUpdates.insert(std::make_pair((*it)->getSource()->getNodeId(), *it));
    }
  }

  static std::string getText(const State& c)
  {
    std::ostringstream retval;
    retval << c.name() << "(";
    std::vector<Value>::const_iterator it = c.parameters().begin();
    if (it != c.parameters().end()) {
      retval << *it;
      for (++it; it != c.parameters().end(); ++it)
        retval << ", " << *it;
    }
    retval << ")";
    return retval.str();
  }

  static std::string getText(const State& c, const Value& val)
  {
    std::ostringstream retval;
    retval << getText(c);
    retval << " = ";
    if (val.valueType() == STRING_TYPE)
      retval << "(string)" << val;
    else
      retval << val;
    return retval.str();
  }

  double TestExternalInterface::currentTime()
  {
    double result = 0; // default if unknown
    m_states[State::timeState()].getValue(result);
    return result;
  }
}
