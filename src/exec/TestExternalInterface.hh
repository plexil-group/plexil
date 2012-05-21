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

#ifndef _H_TestExternalInterface
#define _H_TestExternalInterface

#include "ExecDefs.hh"
#include "ExternalInterface.hh"
#include "ResourceArbiterInterface.hh"
#include <iostream>
#include <map>
#include <set>

// Forward reference
namespace pugi
{
  class xml_node;
}

namespace PLEXIL {

   
  //typedef std::pair<double, std::list<double> > UniqueState;
  //typedef std::pair<double, std::list<double> > UniqueCommand;
  //bite me, g++

   typedef std::pair<double, std::vector<double> >     UniqueThing;
   typedef std::map<UniqueThing, ExpressionId>         ExpressionUtMap;
   typedef std::map<UniqueThing, double >              StateMap;

  class TestExternalInterface : public ExternalInterface {
  public:
    TestExternalInterface();
    void run(const pugi::xml_node& input)
      throw(ParserException);

    double lookupNow(const State& state);

	// LookupOnChange
	void subscribe(const State& state);
	void unsubscribe(const State& state);
	void setThresholds(const State& state, double hi, double lo);

    void batchActions(std::list<CommandId>& commands);
    void updatePlanner(std::list<UpdateId>& updates);

    void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmd The command.
     */
    void invokeAbort(const CommandId& cmd);

    double currentTime();
  protected:
  private:
    std::string getText(const UniqueThing& c);
    std::string getText(const UniqueThing& c, double v);
    std::string getText(const UniqueThing& c,
                        const std::vector<double>& vals);
    void handleInitialState(const pugi::xml_node& input);

	void setVariableValue(const std::string& source,
						  ExpressionId expr,
						  double& value);
        
	void parseState(const pugi::xml_node& state, 
					LabelStr& name, 
					std::vector<double>& args, 
					double& value);

	void parseCommand(const pugi::xml_node& cmd, 
					  LabelStr& name, 
					  std::vector<double>& args, 
					  double& value);

    void parseParams(const pugi::xml_node& root, std::vector<double>& dest);
    double parseValues(const std::string& type, pugi::xml_node valXml);
    double parseValue(const std::string& type, const std::string& valStr);

    std::map<double, UpdateId> m_waitingUpdates;
    ExpressionUtMap m_executingCommands; //map from commands to the destination variables
    ExpressionUtMap m_commandAcks; //map from command to the acknowledgement variables
    ExpressionUtMap m_abortingCommands;
    StateMap m_states; //uniquely identified states and their values
    static UniqueThing& timeState();
    ResourceArbiterInterface raInterface;
    std::map<ExpressionId, CommandId> m_destToCmdMap;
  };
}

#endif
