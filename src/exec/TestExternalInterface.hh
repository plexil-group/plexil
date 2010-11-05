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
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"
#include <iostream>
#include <map>
#include <set>

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
    void run(const TiXmlElement& input)
      throw(ParserException);

    //void registerChangeLookup(ExpressionId dest, const LabelStr& name, double tolerance, const std::list<double>& params);
    void registerChangeLookup(const LookupKey& source, const State& state, const StateKey& key, const std::vector<double>& tolerances, 
				      std::vector<double>& dest);
    void registerChangeLookup(const LookupKey& source, const StateKey& key, const std::vector<double>& tolerances);


    //void lookupNow(ExpressionId dest, const LabelStr& name, const std::list<double>& params);
    void lookupNow(const State& state, const StateKey& key, std::vector<double>& dest);
    void lookupNow(const StateKey& key, std::vector<double>& dest);


    //void unregisterChangeLookup(ExpressionId dest);
    void unregisterChangeLookup(const LookupKey& dest);

    void batchActions(std::list<CommandId>& commands);
    void updatePlanner(std::list<UpdateId>& updates);

    void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmdName The LabelString representing the command name.
     * @param cmdArgs The command arguments expressed as doubles.
     * @param cmdAck The acknowledgment of the pending command
     * @param abrtAck The expression in which to store an acknowledgment of command abort.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck);
    void addPlan(const TiXmlElement& plan, const LabelStr& parent)
      throw(ParserException);
    
    double currentTime();
  protected:
  private:
    std::string getText(const UniqueThing& c);
    std::string getText(const UniqueThing& c, double v);
    std::string getText(const UniqueThing& c,
                        const std::vector<double>& vals);
    void handleInitialState(const TiXmlElement& input);

        void setVariableValue(std::string source,
                              ExpressionId expr,
                              double& value);
        
        void parseState(const TiXmlElement& state, 
                        LabelStr& name, 
                        std::vector<double>& args, 
                        double& value);

        void parseCommand(const TiXmlElement& cmd, 
                          LabelStr& name, 
                          std::vector<double>& args, 
                          double& value);

    void parseParams(const TiXmlElement& root, std::vector<double>& dest);
    double parseValues(std::string type, const TiXmlElement* valXml);
    double parseValue(std::string type, std::string valStr);
    //bool handleStateChange(const UniqueThing& state, double value);
    //bool updateState(const LabelStr& name, const std::list<double>& args, const double value);

    std::map<double, UpdateId> m_waitingUpdates;
    ExpressionUtMap m_executingCommands; //map from commands to the destination variables
    ExpressionUtMap m_commandAcks; //map from command to the acknowledgement variables
    ExpressionUtMap m_abortingCommands;
    StateMap m_states; //uniquely identified states and their values
    //std::multimap<UniqueThing, ExpressionId> m_stateListeners; //map from states to lookup expressions
    //std::map<ExpressionId, double> m_tolerances; //map from lookups to tolerances
    //std::map<ExpressionId, std::pair<double, double> > m_frequencies;
    //std::map<ExpressionId, double> m_cachedValues; //map from lookups to previously returned values or the time at which the last value was updated
    std::map<StateKey, State> m_statesByKey;
    static UniqueThing& timeState();
    ResourceArbiterInterface raInterface;
    std::map<ExpressionId, CommandId> m_destToCmdMap;
  };
}

#endif
