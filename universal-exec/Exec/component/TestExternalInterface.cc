/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
#include "Expression.hh"
#include "PlexilExec.hh"
#include "Node.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "CoreExpressions.hh"
#include "XMLUtils.hh"
#include "StateCache.hh"
#include "PlexilXmlParser.hh"
#include "CommandHandle.hh"
#include <limits>
#include <sstream>
#include <cmath>

//ADD A TIME STATE WITH A VALUE OF 0 AT CONSTRUCTION TIME!

namespace PLEXIL
{

   UniqueThing& TestExternalInterface::timeState()
   {
      static UniqueThing sl_state(std::make_pair((double)LabelStr("time"), std::vector<double>()));
      return sl_state;
   }

   TestExternalInterface::TestExternalInterface() : ExternalInterface()
   {
      m_states.insert(std::make_pair(timeState(), 0));
   }

   void TestExternalInterface::run(const TiXmlElement& input)
     throw(ParserException)
   {
      checkError(m_exec.isValid(), "Attempted to run a script without an executive.");
      handleInitialState(input);
      TiXmlElement* script = input.FirstChildElement("Script");
      checkError(script != NULL, "No Script element in script...");
      TiXmlElement* scriptElement = script->FirstChildElement();
      while (scriptElement != NULL)
      {
         //bool stepExec = false;
         LabelStr name;
         double value;
         std::vector<double> args;

         // parse state

         if (strcmp(scriptElement->Value(), "State") == 0)
         {
            parseState(*scriptElement, name, args, value);
            State st(name, args);
            std::vector<double> stateValues(1, value);
            m_states[st] = value;
            debugMsg("Test:testOutput", "Processing event: " <<
                     StateCache::toString(st)
                     << " = " << StateCache::toString(stateValues));
            m_exec->getStateCache()->updateState(st, stateValues);
         }

         // parse command

         else if (strcmp(scriptElement->Value(), "Command") == 0)
         {
            parseCommand(*scriptElement, name, args, value);
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
            //stepExec = true;
         }

         // parse function call

         else if (strcmp(scriptElement->Value(), "FunctionCall") == 0)
         {
            parseFunctionCall(*scriptElement, name, args, value);
            UniqueThing functionCall(name, args);
            debugMsg("Test:testOutput", "Sending functionCall result "
                     << getText(functionCall, value));
            ExpressionUtMap::iterator it = 
               m_executingFunctionCalls.find(functionCall);
            checkError(it != m_executingFunctionCalls.end(),
                       "No currently executing functionCall " <<
                       getText(functionCall));
            setVariableValue(getText(functionCall), it->second,
                             value);
            m_executingFunctionCalls.erase(it);

            debugMsg("Test:testOutput", "Sending function call ACK " 
                     << getText(functionCall, value));
            it = m_functionCallAcks.find(functionCall);
            checkError(it != m_functionCallAcks.end(),
                       "No functionCall waiting for an acknowledgement"
                       << getText(functionCall));
            if (it->second != ExpressionId::noId())
               it->second->setValue(BooleanVariable::TRUE());
            m_functionCallAcks.erase(it);

            //stepExec = true;
         }

         // parse command ack

         else if (strcmp(scriptElement->Value(), "CommandAck") == 0)
         {
            parseCommand(*scriptElement, name, args, value);
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
            
            //            m_commandAcks.erase(it);
            //stepExec = true;
         }
         
         // parse command abort

         else if (strcmp(scriptElement->Value(), "CommandAbort") == 0)
         {
            parseCommand(*scriptElement, name, args, value);
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
            it->second->setValue(BooleanVariable::TRUE());
            m_abortingCommands.erase(it);
         }

         // parse update ack

         else if (strcmp(scriptElement->Value(), "UpdateAck") == 0)
         {
            LabelStr name(scriptElement->Attribute("name"));
            debugMsg("Test:testOutput", "Sending update ACK " << name.toString());
            std::map<double, UpdateId>::iterator it = m_waitingUpdates.find(name);
            checkError(it != m_waitingUpdates.end(),
                       "No update from node " << name.toString() << 
                       " waiting for acknowledgement.");
            it->second->getAck()->setValue(BooleanVariable::TRUE());
            m_waitingUpdates.erase(it);
         }

         // parse send plan

         else if (strcmp(scriptElement->Value(), "SendPlan") == 0)
         {
            TiXmlDocument* doc = new TiXmlDocument();
            doc->LoadFile(scriptElement->Attribute("file"));

            LabelStr parent;
            if (scriptElement->Attribute("parent") != NULL)
               parent = LabelStr(scriptElement->Attribute("parent"));
            debugMsg("Test:testOutput", "Sending plan from file " << 
                     scriptElement->Attribute("file"));
            condDebugMsg(parent != EMPTY_LABEL(), "Test:testOutput", 
                         "To be child of parent " << parent.toString());
            PlexilXmlParser parser;
            PlexilNodeId root =
               parser.parse(doc->FirstChildElement("PlexilPlan")->
                            FirstChildElement("Node"));
            m_exec->addPlan(root, parent);
         }

         // parse simultaneous

         else if (strcmp(scriptElement->Value(), "Simultaneous") == 0)
         {
            TiXmlElement* stateUpdates = scriptElement->FirstChildElement("State");
            while (stateUpdates != NULL)
            {
               
               parseState(*stateUpdates, name, args, value);
               State st(name, args);
               m_states[st] = value;
               debugMsg("Test:testOutput", "Processing simultaneous event: " <<
                        StateCache::toString(st) << " = " << value);
               m_exec->getStateCache()->updateState(
                  st, std::vector<double>(1, value));
               args.clear();
               stateUpdates = stateUpdates->NextSiblingElement("State");
            }
         }
         else if (strcmp(scriptElement->Value(), "Delay") == 0)
         {
           ; // No-op
         }

         // report unknow script element

         else
         {
            checkError(ALWAYS_FAIL, "Unknown script element '" << 
                       scriptElement->Value() << "'");
            return;
         }
         
         // step the exec forward
         
         m_exec->step();
         scriptElement = scriptElement->NextSiblingElement();
      }
   }

   // map values from script into a variable expression

   void TestExternalInterface::setVariableValue(std::string source,
                                                ExpressionId expr,
                                                double& value)
   {
      if (expr != ExpressionId::noId())
      {
         checkError(Id<Variable>::convertable(expr),
                    "Expected string or atomic variable in \'" <<
                    source << "\'");
         expr->setValue(value);
      }
   }

   void TestExternalInterface::handleInitialState(const TiXmlElement& input)
   {
      TiXmlElement* initialState = input.FirstChildElement("InitialState");
      if (initialState != NULL)
      {
         LabelStr name;
         double value;
         std::vector<double> args;
         TiXmlElement* state = initialState->FirstChildElement("State");
         while (state != NULL)
         {
            args.clear();
            parseState(*state, name, args, value);
            UniqueThing st(name, args);
            debugMsg("Test:testOutput", "Creating initial state " 
                     << getText(st, value));
            m_states.insert(std::pair<UniqueThing, double>(st, value));
            state = state->NextSiblingElement("State");
         }
      }
      m_exec->step();
   }

   void TestExternalInterface::parseState(const TiXmlElement& state,
                                          LabelStr& name,
                                          std::vector<double>& args,
                                          double& value)

   {
      checkError(strcmp(state.Value(), "State") == 0,
                 "Expected <State> element.  Found '" << state.Value() << "'");

      checkError(state.Attribute("name") != NULL,
                 "No name attribute in <State> element.");
      name = LabelStr(state.Attribute("name"));
      checkError(state.Attribute("type") != NULL,
                 "No type attribute in <State> element.");
      std::string type(state.Attribute("type"));
      TiXmlElement* valXml = state.FirstChildElement("Value");
      checkError(valXml != NULL, "No Value child in State element.");
      checkError(valXml->FirstChild() != NULL,
                 "Empty Value child in State element.");

      // read in the initiial values and parameters

      value = parseValues(type, valXml);
      parseParams(state, args);
   }

   void TestExternalInterface::parseCommand(const TiXmlElement& cmd,
                                            LabelStr& name, 
                                            std::vector<double>& args, 
                                            double& value)
   {
      checkError(strcmp(cmd.Value(), "Command") == 0 ||
                 strcmp(cmd.Value(), "CommandAck") == 0 ||
                 strcmp(cmd.Value(), "CommandAbort") == 0, "Expected <Command> element.  Found '" << cmd.Value() << "'");

      checkError(cmd.Attribute("name") != NULL, "No name attribute in <Command> element.");
      name = LabelStr(cmd.Attribute("name"));
      checkError(cmd.Attribute("type") != NULL, "No type attribute in <Command> element.");
      std::string type(cmd.Attribute("type"));

      TiXmlElement* resXml = cmd.FirstChildElement("Result");
      checkError(resXml != NULL, "No Result child in Command element.");
      checkError(resXml->FirstChild() != NULL, "Empty Result child in Command element.");

      // read in the initiial values and parameters

      value = parseValues(type, resXml);
      parseParams(cmd, args);
   }

   void TestExternalInterface::parseFunctionCall(const TiXmlElement& cmd, 
                                                 LabelStr& name, 
                                                 std::vector<double>& args,
                                                 double& value)
   {
      checkError(strcmp(cmd.Value(), "FunctionCall") == 0 ||
                 strcmp(cmd.Value(), "FunctionCallAck") == 0 ||
                 strcmp(cmd.Value(), "FunctionCallAbort") == 0, 
                 "Expected <FunctionCall> element.  Found '" << cmd.Value() << "'");

      checkError(cmd.Attribute("name") != NULL,
                 "No name attribute in <FunctionCall> element.");
      name = LabelStr(cmd.Attribute("name"));
      checkError(cmd.Attribute("type") != NULL, 
                 "No type attribute in <FunctionCall> element.");
      std::string type(cmd.Attribute("type"));

      TiXmlElement* resXml = cmd.FirstChildElement("Result");
      checkError(resXml != NULL, "No Result child in FunctionCall element.");
      checkError(resXml->FirstChild() != NULL, 
                 "Empty Result child in FunctionCall element.");
      std::string resStr(resXml->FirstChild()->Value());

      // read in the initiial values and parameters

      value = parseValues(type, resXml);
      parseParams(cmd, args);
   }

   void TestExternalInterface::parseParams(const TiXmlElement& root, 
                                           std::vector<double>& dest)
   {
      TiXmlElement* param = root.FirstChildElement("Param");

      while (param != NULL)
      {
         checkError(param->FirstChild() != NULL || strcmp(param->Attribute("type"),
                                                          "string") == 0,
                    "Empty Param child in " << root.Value() << " element.");
         double value;
         if (param->Attribute("type") != NULL &&
             (strcmp(param->Attribute("type"), "int") == 0 ||
              strcmp(param->Attribute("type"), "real") == 0 ||
              strcmp(param->Attribute("type"), "bool") == 0))
         {
            std::stringstream str;
            str << param->FirstChild()->Value();
            str >> value;
         }
         else
            value = (param->FirstChild() != NULL)
               ? LabelStr(param->FirstChild()->Value())
               : LabelStr();

         dest.push_back(value);
         param = param->NextSiblingElement("Param");
      }
   }

   double TestExternalInterface::parseValues(std::string type, 
                                             TiXmlElement* valXml)
   {
      // read in values

      std::vector<double> values;
      while (valXml != NULL)
      {
         values.push_back(parseValue(type, valXml->FirstChild()->Value()));
         valXml = valXml->NextSiblingElement();
      }

      // if atomic use 1st value, else its an array, create array

      double value = (type.find("array", 0) == std::string::npos)
         ? values[0]
         : StoredArray(values.size(), values).getKey();
      return value;
   }

   // parse in value

   double TestExternalInterface::parseValue(std::string type, 
                                            std::string valStr)
   {
      double value;

      if (type == "string" || type == "string-array")
      {
         value = (double) LabelStr(valStr);
      }
      else if (type == "int" || type == "real" ||
               type == "int-array" || type == "real-array")
      {
         std::stringstream ss;
         ss << valStr;
         ss >> value;
      }
      else if (type == "bool" || type == "bool-array")
      {
         std::stringstream ss;
         if (valStr == "true" || valStr == "TRUE" || valStr == "True")
            value = 1;
         else if (valStr == "false" || valStr == "FALSE" || valStr == "False")
            value = 0;
         else
         {
            std::stringstream ss;
            ss << valStr;
            ss >> value;
         }
      }
      else
      {
         checkError(ALWAYS_FAIL,
                    "Unknown type '" << type << "' in State element.");
         return 0;
      }
      return value;
   }

   void TestExternalInterface::registerChangeLookup(
      const LookupKey& source, const State& state, 
      const StateKey& key, const std::vector<double>& tolerances,
      std::vector<double>& dest)
   {
      debugMsg("Test:testOutput", "Registering change lookup " 
               << StateCache::toString(state) << " with tolerances " 
               << StateCache::toString(tolerances));
      m_statesByKey.insert(std::make_pair(key, state));

      //ignore source, because we don't care about bandwidth here
      StateMap::iterator it = m_states.find(state);
      if (it == m_states.end())
      {
         std::pair<UniqueThing, double> p = 
            std::make_pair(state, Expression::UNKNOWN());
         it = m_states.insert(p).first;
      }
      dest[0] = m_states[state];
   }

   void TestExternalInterface::registerChangeLookup(
      const LookupKey& source, const StateKey& key, 
      const std::vector<double>& tolerances)
   {
      std::vector<double> fakeDest(1, 0);
      registerChangeLookup(source, m_statesByKey[key], key, tolerances, fakeDest);
   }

   void TestExternalInterface::registerFrequencyLookup(const LookupKey& source, const State& state, const StateKey& key, const double& lowFreq, const double& highFreq,
                                                       std::vector<double>& dest)
   {
      debugMsg("Test:testOutput", "Registering frequency lookup " << StateCache::toString(state) << " with frequency (" << lowFreq << ", " << highFreq << ")");
      m_statesByKey.insert(std::make_pair(key, state));
      StateMap::iterator it = m_states.find(state);
      if (it == m_states.end())
      {
         it = m_states.insert(std::make_pair(state, Expression::UNKNOWN())).first;
      }
      checkError(m_states.find(state) != m_states.end(), "No entry for state " << StateCache::toString(state));
      dest[0] = 77.7; //m_states[state][0];
   }

   void TestExternalInterface::registerFrequencyLookup(const LookupKey& source, const StateKey& key, const double& lowFreq, const double& highFreq)
   {
      checkError(m_statesByKey.find(key) != m_statesByKey.end(), "No known state for key " << key);
      std::vector<double> fakeDest(1, 0);
      registerFrequencyLookup(source, m_statesByKey[key], key, lowFreq, highFreq, fakeDest);
   }

   void TestExternalInterface::lookupNow(const State& state, const StateKey& key,
                                         std::vector<double>& dest)
   {
      debugMsg("Test:testOutput", "Looking up immediately "
               << StateCache::toString(state));
      m_statesByKey.insert(std::make_pair(key, state));
      StateMap::iterator it = m_states.find(state);
      if (it == m_states.end())
      {
         debugMsg("Test:testOutput", "No state found.  Setting UNKNOWN.");
         it = m_states.insert(
            std::make_pair(state, Expression::UNKNOWN())).first;
      }
      debugMsg("Test:testOutput", "Returning value " << m_states[state]);
      double value = m_states[state];
      if (dest.size() < 1)
         dest.push_back(value);
      else
         dest[0] = value;
   }

   void TestExternalInterface::lookupNow(const StateKey& key, 
                                         std::vector<double>& dest)
   {
      checkError(m_statesByKey.find(key) != m_statesByKey.end(),
                 "No state known for key " << key);
      lookupNow(m_statesByKey[key], key, dest);
   }


   void TestExternalInterface::unregisterChangeLookup(const LookupKey& dest)
   {}

   void TestExternalInterface::unregisterFrequencyLookup(const LookupKey& dest)
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
         
         if (acceptCmds.find(cmd) != acceptCmds.end())
           executeCommand(cmd->getName(), cmd->getArgValues(), cmd->getDest(), cmd->getAck());
         else
           {
             debugMsg("Test:testOutput", 
                      "Permission to execute " << cmd->getName().toString()
                      << " has been denied by the resource arbiter.");
             cmd->getAck()->setValue(CommandHandleVariable::COMMAND_DENIED());
           }
      }
   }

   void TestExternalInterface::executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack)
   {
      std::vector<double> realArgs(args.begin(), args.end());
      UniqueThing cmd((double)name, realArgs);
      debugMsg("Test:testOutput", "Executing " << getText(cmd) <<
               " into " << (dest.isNoId() ? std::string("noId") : dest->toString()) << " with ack " << ack->toString());
      if (!dest.isNoId()) m_executingCommands[cmd] = dest;
      m_commandAcks[cmd] = ack;
      //m_executingCommands.insert(UniqueThing((double)name, args), dest);
   }

   void TestExternalInterface::batchActions(std::list<FunctionCallId>& functionCalls)
   {
      if (functionCalls.empty())
         return;
      for (std::list<FunctionCallId>::iterator it = functionCalls.begin(); it != functionCalls.end(); ++it)
      {
         FunctionCallId fc = *it;
         check_error(fc.isValid());
         executeFunctionCalls(fc->getName(), fc->getArgValues(), fc->getDest(), fc->getAck());
      }
      //m_exec->step();
   }

   void TestExternalInterface::executeFunctionCalls(const LabelStr& name, const std::list<double>& args,
                                                    ExpressionId dest, ExpressionId ack)
   {
      std::vector<double> realArgs(args.begin(), args.end());
      UniqueThing fc((double)name, realArgs);
      debugMsg("Test:testOutput", "Executing " << getText(fc) <<
               " into " << (dest.isNoId() ? std::string("noId") : dest->toString()) << " with ack " << ack->toString());
      m_executingFunctionCalls[fc] = dest;
      m_functionCallAcks[fc] = ack;
      //m_executingCommands.insert(UniqueThing((double)name, args), dest);
   }

   void TestExternalInterface::invokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest)
   {
      //checkError(ALWAYS_FAIL, "Don't do that.");
      std::vector<double> realArgs(args.begin(), args.end());
      UniqueThing cmd((double)name, realArgs);
      debugMsg("Test:testOutput", "Aborting " << getText(cmd));
      m_abortingCommands[cmd] = dest;
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
      std::stringstream retval;
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
      std::stringstream retval;
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
      std::stringstream retval;
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

   void TestExternalInterface::addPlan(const TiXmlElement& plan, const LabelStr& parent)
     throw(ParserException)
   {}

   double TestExternalInterface::currentTime()
   {
      return m_states[timeState()];
   }
}
