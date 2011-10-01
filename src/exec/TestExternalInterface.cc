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

#include "TestExternalInterface.hh"

#include "BooleanVariable.hh"
#include "Command.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include "PlexilXmlParser.hh"
#include "StateCache.hh"
#include "StoredArray.hh"
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

  UniqueThing& TestExternalInterface::timeState()
  {
    static UniqueThing* sl_state = NULL;
    if (sl_state == NULL)
      sl_state = new UniqueThing(std::make_pair((double)LabelStr("time"), std::vector<double>()));
    return *sl_state;
  }

  TestExternalInterface::TestExternalInterface() : ExternalInterface()
  {
	// Ensure there's a "time" state
    m_states.insert(std::make_pair(timeState(), 0));
  }

  void TestExternalInterface::run(const pugi::xml_node& input)
    throw(ParserException)
  {
    checkError(m_exec.isValid(), "Attempted to run a script without an executive.");
    handleInitialState(input);
	pugi::xml_node script = input.child("Script");
    checkError(!script.empty(), "No Script element in Plexilscript.");
	pugi::xml_node scriptElement = script.first_child();
    while (!scriptElement.empty()) {
	  LabelStr name;
	  double value;
	  std::vector<double> args;

	  // ignore text element (e.g. from <Script> </Script>)
	  if (scriptElement.type() == pugi::node_pcdata) {
	  }

	  // parse state

	  else if (strcmp(scriptElement.name(), "State") == 0) {
		  parseState(scriptElement, name, args, value);
		  State st(name, args);
		  m_states[st] = value;
		  debugMsg("Test:testOutput", "Processing event: " <<
				   StateCache::toString(st)
				   << " = " << Expression::valueToString(value));
		  m_exec->getStateCache()->updateState(st, value);
		}

	  // parse command

	  else if (strcmp(scriptElement.name(), "Command") == 0) {
		parseCommand(scriptElement, name, args, value);
		UniqueThing command(name, args);
		debugMsg("Test:testOutput", "Sending command result " << 
				 getText(command, value));
		ExpressionUtMap::iterator it = 
		  m_executingCommands.find(command);
		checkError(it != m_executingCommands.end(),
				   "No currently executing command " << getText(command));

		setVariableValue(getText(command), it->second, value);
		m_executingCommands.erase(it);
		raInterface.releaseResourcesForCommand(name);
	  }

	  // parse command ack

	  else if (strcmp(scriptElement.name(), "CommandAck") == 0) {
		parseCommand(scriptElement, name, args, value);
		UniqueThing command(name, args);
		debugMsg("Test:testOutput", "Sending command ACK " << 
				 getText(command, value));

		ExpressionUtMap::iterator it = 
		  m_commandAcks.find(command);

		checkError(it != m_commandAcks.end(), 
				   "No command waiting for acknowledgement " <<
				   getText(command));
		it->second->setValue(value);
            
		// Release resources if the command does not have a return value
		if (m_executingCommands.find(command) == m_executingCommands.end())
		  raInterface.releaseResourcesForCommand(name);
	  }
         
	  // parse command abort

	  else if (strcmp(scriptElement.name(), "CommandAbort") == 0) {
		parseCommand(scriptElement, name, args, value);
		UniqueThing command(name, args);
		debugMsg("Test:testOutput", "Sending abort ACK " << 
				 getText(command, value));
		ExpressionUtMap::iterator it = 
		  m_abortingCommands.find(command);
		checkError(it != m_abortingCommands.end(), 
				   "No abort waiting for acknowledgement " << 
				   getText(command));
		debugMsg("Test:testOutput", "Acknowledging abort into " << 
				 it->second->toString());
		it->second->setValue(BooleanVariable::TRUE_VALUE());
		m_abortingCommands.erase(it);
	  }

	  // parse update ack

	  else if (strcmp(scriptElement.name(), "UpdateAck") == 0) {
		LabelStr name(scriptElement.attribute("name").value());
		debugMsg("Test:testOutput", "Sending update ACK " << name.toString());
		std::map<double, UpdateId>::iterator it = m_waitingUpdates.find(name);
		checkError(it != m_waitingUpdates.end(),
				   "No update from node " << name.toString() << 
				   " waiting for acknowledgement.");
		it->second->getAck()->setValue(BooleanVariable::TRUE_VALUE());
		m_waitingUpdates.erase(it);
	  }

	  // parse send plan

	  else if (strcmp(scriptElement.name(), "SendPlan") == 0) {
		pugi::xml_document* doc = new pugi::xml_document();
		pugi::xml_parse_result parseResult = 
		  doc->load_file(scriptElement.attribute("file").value());

		LabelStr parent;
		if (!scriptElement.attribute("parent").empty())
		  parent = LabelStr(scriptElement.attribute("parent").value());
		debugMsg("Test:testOutput", "Sending plan from file " << 
				 scriptElement.attribute("file").value());
		condDebugMsg(parent != EMPTY_LABEL(), "Test:testOutput", 
					 "To be child of parent " << parent.toString());
		PlexilNodeId root =
		  PlexilXmlParser::parse(doc->document_element().child("PlexilPlan").child("Node"));
		checkError(m_exec->addPlan(root, parent),
				   "Adding plan " << scriptElement.attribute("file").value() << " failed");
	  }

	  // parse simultaneous

	  else if (strcmp(scriptElement.name(), "Simultaneous") == 0) {
		pugi::xml_node stateUpdates = scriptElement.child("State");
		while (!stateUpdates.empty()) {
		  parseState(stateUpdates, name, args, value);
		  State st(name, args);
		  m_states[st] = value;
		  debugMsg("Test:testOutput", "Processing simultaneous event: " <<
				   StateCache::toString(st) << " = " << value);
		  m_exec->getStateCache()->updateState(st, value);
		  args.clear();
		  stateUpdates = stateUpdates.next_sibling("State");
		}
	  }
	  else if (strcmp(scriptElement.name(), "Delay") == 0) {
		; // No-op
	  }

	  // report unknown script element

	  else {
		checkError(ALWAYS_FAIL, "Unknown script element '" << scriptElement.name() << "'");
		return;
	  }
         
	  // step the exec forward
	  m_exec->step();

	  scriptElement = scriptElement.next_sibling();
	}
	// Continue stepping the Exec til quiescent
	while (m_exec->needsStep()) {
	  m_exec->step();
	}
  }

  // map values from script into a variable expression

  void TestExternalInterface::setVariableValue(const std::string& source,
                                               ExpressionId expr,
                                               double& value)
  {
    if (expr != ExpressionId::noId())
      {
        checkError(Id<VariableImpl>::convertable(expr),
                   "Expected string or atomic variable in \'" <<
                   source << "\'");
        expr->setValue(value);
        std::map<ExpressionId, CommandId>::iterator iter;
        if ((iter = m_destToCmdMap.find (expr)) != m_destToCmdMap.end()) {
          CommandId cmdId = iter->second;
          std::string destName = cmdId->getDestName();
          m_destToCmdMap.erase(iter);
        }
        else std::cerr << "Error in TestExternalInterface: "
                       << "Could not find destination for command " << source
                       << ".  Should never happen!" << std::endl;
      }
  }

  void TestExternalInterface::handleInitialState(const pugi::xml_node& input)
  {
	pugi::xml_node initialState = input.child("InitialState");
    if (!initialState.empty()) {
	  LabelStr name;
	  double value;
	  std::vector<double> args;
	  pugi::xml_node state = initialState.child("State");
	  while (!state.empty()) {
		args.clear();
		parseState(state, name, args, value);
		UniqueThing st(name, args);
		debugMsg("Test:testOutput", "Creating initial state " 
				 << getText(st, value));
		m_states.insert(std::pair<UniqueThing, double>(st, value));
		state = state.next_sibling("State");
	  }
	}
	m_exec->step();
  }

  void TestExternalInterface::parseState(const pugi::xml_node& state,
                                         LabelStr& name,
                                         std::vector<double>& args,
                                         double& value)

  {
    checkError(strcmp(state.name(), "State") == 0,
               "Expected <State> element.  Found '" << state.name() << "'");

    checkError(!state.attribute("name").empty(),
               "No name attribute in <State> element.");
    name = LabelStr(state.attribute("name").value());
    checkError(!state.attribute("type").empty(),
               "No type attribute in <State> element.");
    std::string type(state.attribute("type").value());
	pugi::xml_node valXml = state.child("Value");
    checkError(!valXml.empty(), "No Value child in State element.");
    checkError(!valXml.first_child().empty(),
               "Empty Value child in State element.");

    // read in the initiial values and parameters

    value = parseValues(type, valXml);
    parseParams(state, args);
  }

  void TestExternalInterface::parseCommand(const pugi::xml_node& cmd,
                                           LabelStr& name, 
                                           std::vector<double>& args, 
                                           double& value)
  {
    checkError(strcmp(cmd.name(), "Command") == 0 ||
               strcmp(cmd.name(), "CommandAck") == 0 ||
               strcmp(cmd.name(), "CommandAbort") == 0, "Expected <Command> element.  Found '" << cmd.name() << "'");

    checkError(!cmd.attribute("name").empty(), "No name attribute in <Command> element.");
    name = LabelStr(cmd.attribute("name").value());
    checkError(!cmd.attribute("type").empty(), "No type attribute in <Command> element.");
    std::string type(cmd.attribute("type").value());

	pugi::xml_node resXml = cmd.child("Result");
    checkError(!resXml.empty(), "No Result child in Command element.");
    checkError(!resXml.first_child().empty(), "Empty Result child in Command element.");

    // read in the initiial values and parameters

    value = parseValues(type, resXml);
    parseParams(cmd, args);
  }

  void TestExternalInterface::parseParams(const pugi::xml_node& root, 
                                          std::vector<double>& dest)
  {
	pugi::xml_node param = root.child("Param");

    while (!param.empty()) {
	  checkError(!param.first_child().empty()
				 || strcmp(param.attribute("type").value(), "string") == 0,
				 "Empty Param child in " << root.name() << " element.");
	  double value;
	  if (!param.attribute("type").empty() &&
		  (strcmp(param.attribute("type").value(), "int") == 0 ||
		   strcmp(param.attribute("type").value(), "real") == 0 ||
		   strcmp(param.attribute("type").value(), "bool") == 0)) {
		std::istringstream str(param.first_child().value());
		str >> value;
	  }
	  // string case
	  else if (param.first_child().empty()) {
		value = LabelStr();
	  }
	  else if (0 == strcmp(param.first_child().value(), "UNKNOWN")) {
		value = Expression::UNKNOWN();
	  }
	  else {
		value = LabelStr(param.first_child().value());
	  }

	  dest.push_back(value);
	  param = param.next_sibling("Param");
	}
  }

  double TestExternalInterface::parseValues(const std::string& type, 
                                            pugi::xml_node valXml)
  {
    // read in values
    std::vector<double> values;
    while (!valXml.empty()) {
	  values.push_back(parseValue(type, valXml.first_child().value()));
	  valXml = valXml.next_sibling();
	}

    // if atomic use 1st value, else its an array, create array
    double value = (type.find("array", 0) == std::string::npos)
      ? values[0]
      : StoredArray(values.size(), values).getKey();
    return value;
  }

  // parse in value

  double TestExternalInterface::parseValue(const std::string& type, 
                                           const std::string& valStr)
  {
    double value;

    if (type == "string" || type == "string-array") {
	  value = (double) LabelStr(valStr);
	}
    else if (type == "int" || type == "real" ||
             type == "int-array" || type == "real-array") {
	  std::istringstream ss(valStr);
	  ss >> value;
	}
    else if (type == "bool" || type == "bool-array") {
	  if (0 == stricmp(valStr.c_str(), "true"))
		value = 1;
	  else if (0 == stricmp(valStr.c_str(), "false"))
		value = 0;
	  else {
		std::istringstream ss(valStr);
		ss >> value;
	  }
	}
    else {
	  checkError(ALWAYS_FAIL,
				 "Unknown type '" << type << "' in State element.");
	  return 0;
	}
    return value;
  }

  double TestExternalInterface::lookupNow(const State& state)
  {
    debugMsg("Test:testOutput", "Looking up immediately "
             << StateCache::toString(state));
    StateMap::const_iterator it = m_states.find(state);
    if (it == m_states.end()) {
	  debugMsg("Test:testOutput", "No state found.  Setting UNKNOWN.");
	  it = m_states.insert(std::make_pair(state, Expression::UNKNOWN())).first;
	}
    double value = it->second;
    debugMsg("Test:testOutput", "Returning value "
			 << Expression::valueToString(value));
	return value;
  }

  void TestExternalInterface::subscribe(const State& state)
  {
    debugMsg("Test:testOutput",
			 "Registering change lookup for " << StateCache::toString(state));

    //ignore source, because we don't care about bandwidth here
    StateMap::iterator it = m_states.find(state);
    if (it == m_states.end()) {
	  std::pair<UniqueThing, double> p = 
		std::make_pair(state, Expression::UNKNOWN());
	  it = m_states.insert(p).first;
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
    raInterface.arbitrateCommands(commands, acceptCmds);
    for (std::list<CommandId>::iterator it = commands.begin(); it != commands.end(); ++it)
      {
        CommandId cmd = *it;
        check_error(cmd.isValid());
         
        if (acceptCmds.find(cmd) != acceptCmds.end()) {
          m_destToCmdMap[cmd->getDest()] = cmd;
          executeCommand(cmd->getName(), cmd->getArgValues(), cmd->getDest(), cmd->getAck());
        }
        else
          {
            debugMsg("Test:testOutput", 
                     "Permission to execute " << cmd->getName().toString()
                     << " has been denied by the resource arbiter.");
            cmd->getAck()->setValue(CommandHandleVariable::COMMAND_DENIED());
          }
      }
  }

  void TestExternalInterface::executeCommand (const LabelStr& name,
                                              const std::list<double>& args,
                                              ExpressionId dest, ExpressionId ack)
  {
    std::vector<double> realArgs(args.begin(), args.end());
    UniqueThing cmd((double)name, realArgs);
    debugMsg("Test:testOutput", "Executing " << getText(cmd) <<
             " into " <<
             (dest.isNoId() ? std::string("noId") : dest->toString()) <<
             " with ack " << ack->toString());
    if (dest.isId())
	  m_executingCommands[cmd] = dest;

    // Special handling of the utility commands (a bit of a hack!):
    const std::string& cname = name.toString();
    if (cname == "print") {
      print (args);
	  ack->setValue(CommandHandleVariable::COMMAND_SUCCESS()); // LabelStr ("COMMAND_SUCCESS"));
      raInterface.releaseResourcesForCommand(name);
    }
    else if (cname == "pprint") {
	  pprint (args);
	  ack->setValue(CommandHandleVariable::COMMAND_SUCCESS()); // LabelStr ("COMMAND_SUCCESS"));
      raInterface.releaseResourcesForCommand(name);
    }
	else {
	  // Usual case - set up for scripted ack value
	  m_commandAcks[cmd] = ack;
	}
  }


  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param cmdName The LabelString representing the command name.
   * @param cmdArgs The command arguments expressed as doubles.
   * @param cmdAck The acknowledgment of the pending command
   * @param abrtAck The expression in which to store an acknowledgment of command abort.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void TestExternalInterface::invokeAbort(const LabelStr& cmdName,
                                          const std::list<double>& cmdArgs,
                                          ExpressionId abrtAck,
                                          ExpressionId /* cmdAck */)
  {
    //checkError(ALWAYS_FAIL, "Don't do that.");
    std::vector<double> realArgs(cmdArgs.begin(), cmdArgs.end());
    UniqueThing cmd((double)cmdName, realArgs);
    debugMsg("Test:testOutput", "Aborting " << getText(cmd));
    m_abortingCommands[cmd] = abrtAck;
  }

  void TestExternalInterface::updatePlanner(std::list<UpdateId>& updates)
  {
    for (std::list<UpdateId>::const_iterator it = updates.begin(); it != updates.end(); ++it)
      {
        debugMsg("Test:testOutput", "Received update: ");
        const std::map<double, double>& pairs = (*it)->getPairs();
        for (std::map<double, double>::const_iterator pairIt = pairs.begin(); pairIt != pairs.end(); ++pairIt)
          debugMsg("Test:testOutput", " " << LabelStr(pairIt->first).toString() << " => " << pairIt->second);
        m_waitingUpdates.insert(std::make_pair((*it)->getSource()->getNodeId(), *it));
      }
  }

  std::string TestExternalInterface::getText(const UniqueThing& c)
  {
    std::ostringstream retval;
    retval << LabelStr(c.first).toString() << "(";
    std::vector<double>::const_iterator it = c.second.begin();
    if (it != c.second.end())
      {
        if (!LabelStr::isString(*it))
          retval << *it;
        else
          retval << LabelStr(*it).toString();
        for (++it; it != c.second.end(); ++it)
          {
            retval << ", ";
            if (!LabelStr::isString(*it))
              retval << *it;
            else
              retval << LabelStr(*it).toString();
          }
      }
    retval << ")";
    return retval.str();
  }

  std::string TestExternalInterface::getText(const UniqueThing& c, 
                                             const double val)
  {
    std::ostringstream retval;
    retval << getText(c);
    retval << " = ";
    if (LabelStr::isString(val))
      retval << "(string)" << LabelStr(val).toString();
    else
      retval << val;
    return retval.str();
  }

  std::string TestExternalInterface::getText(const UniqueThing& c, 
                                             const std::vector<double>& vals)
  {
    std::ostringstream retval;
    retval << getText(c);
    retval << " = ";
    for (std::vector<double>::const_iterator it = vals.begin();
         it != vals.end(); ++it)
      {
        double val = *it;
        if (LabelStr::isString(val))
          retval << "(string)" << LabelStr(val).toString();
        else
          retval << val;
      }
    return retval.str();
  }

  double TestExternalInterface::currentTime()
  {
    return m_states[timeState()];
  }
}
