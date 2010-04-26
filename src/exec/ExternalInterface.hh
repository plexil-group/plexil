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

#ifndef _H_ExternalInterface
#define _H_ExternalInterface

#include "ExecDefs.hh"
#include "Expression.hh"
#include "ParserException.hh"
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

//facade classes have the following naming scheme:
//FooBarIntf is the interface from Foo to Bar

// class ExecFLIntf {
// public:
//   void requestStateUpdate(const State& state, const StateKey& key);
//   void requestStateUpdate(const StateKey& key);

//   void registerChangeLookup(const double source, const State& state, StateKey& state, const std::vector<double>& tolerances);
//   void registerChangeLookup(const double source, const StateKey& state, const std::vector<double>& tolerances);
// };

//implemented by us
// class FLExecIntf {
// public:
//   void updateState(const StateKey& key, const std::vector<double>& values);

//   //void enableChecks()
//   //void disableChecks()
// };

// class ExecPlannerIntf {
// };

//implemented by us
// class PlannerExecIntf {
// };

/**
   TODO:
   2) factor planner interface, fl interface, and ue interface
 */

namespace PLEXIL {
  class ExternalInterface {
  public:
    /**
     * @brief Return the ID of this instance.
     */
    inline ExternalInterfaceId& getId()
    {
      return m_id;
    }

    /**
     * @brief Register a change lookup on a new state, expecting values back.
     * @param source The unique key for this lookup.
     * @param state The state
     * @param key The key for the state to be used in future communications about the state.
     * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */


    virtual void registerChangeLookup(const LookupKey& source, const State& state, const StateKey& key, const std::vector<double>& tolerances, 
				      std::vector<double>& dest);

    /**
     * @brief Register a change lookup on an existing state.
     * @param source The unique key for this lookup.
     * @param key The key for the state.
     * @param tolerances The tolerances for the lookup.  May be used by the FL to reduce the number of updates sent to the exec.
     */
    virtual void registerChangeLookup(const LookupKey& source, const StateKey& key, const std::vector<double>& tolerances);

    /**
     * @brief Perform an immediate lookup on a new state.
     * @param state The state
     * @param key The key for the state to be used in future communications about the state.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    virtual void lookupNow(const State& state, const StateKey& key, std::vector<double>& dest);

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param key The key for the state.
     * @param dest The destination for the current values for the state.
     * @note dest is stack allocated, therefore pointers to it should not be stored!
     */
    virtual void lookupNow(const StateKey& key, std::vector<double>& dest);

    /**
     * @brief Inform the FL that a lookup should no longer receive updates.
     */
    virtual void unregisterChangeLookup(const LookupKey& dest);

    //this batches the set of actions from quiescence completion.  calls PlexilExecutive::step() at the end
    //assignments must be performed first.
    //though it only takes a list of commands as an argument at the moment, it will eventually take function calls and the like
    //so the name remains "batchActions"
    virtual void batchActions(std::list<CommandId>& commands);
    virtual void batchActions(std::list<FunctionCallId>& functionCalls);
    virtual void updatePlanner(std::list<UpdateId>& updates);

    //executes a command with the given arguments
    //this base version looks up the command name and the correct argument types
    //and passes the information to internalExecuteCommand, which is overridden in base classes
    virtual void executeCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);

    //executes a function call with the given arguments
    virtual void executeFunctionCalls(const LabelStr& name, const std::list<double>& args, ExpressionId dest, ExpressionId ack);


    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmdName The LabelString representing the command name.
     * @param cmdArgs The command arguments expressed as doubles.
     * @param cmdAck The acknowledgment of the pending command
     * @param abrtAck The expression in which to store an acknowledgment of command abort.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck);
    //XML VERSION IS DEPRECATED
    virtual void addPlan(const TiXmlElement& plan, const LabelStr& parent = EMPTY_LABEL())
      throw(ParserException);
    virtual void addPlan(PlexilNode* node, const LabelStr& parent = EMPTY_LABEL());
    void setExec(const PlexilExecId exec);

    virtual double currentTime();
    virtual ~ExternalInterface();
  protected:
    //virtual void internalExecuteCommand(const LabelStr& name, const std::list<double>& args, ExpressionId dest) = 0;
    //virtual void internalInvokeAbort(const LabelStr& name, const std::list<double>& args, ExpressionId dest) = 0;

    //this should eventually take a domain description as well
    ExternalInterface();
    PlexilExecId m_exec;

  private:
    ExternalInterfaceId m_id;
  };
}

#endif
