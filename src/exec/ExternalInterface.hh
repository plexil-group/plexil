/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
#include "PlexilExec.hh"

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
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @return The current value for the state.
     */
    virtual double lookupNow(const State& state) = 0;

    /**
     * @brief Inform the interface that it should report changes in value of this state.
     * @param state The state.
     */
    virtual void subscribe(const State& state) = 0;

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     */
    virtual void unsubscribe(const State& state) = 0;

    /**
     * @brief Advise the interface of the current thresholds to use when reporting this state.
     * @param state The state.
     * @param hi The upper threshold, at or above which to report changes.
     * @param lo The lower threshold, at or below which to report changes.
     */
    virtual void setThresholds(const State& state, double hi, double lo) = 0;

    //@ Perform the set of actions from quiescence completion.
    virtual void batchActions(std::list<CommandId>& commands) = 0;

    // This batches planner updates.
    virtual void updatePlanner(std::list<UpdateId>& updates) = 0;

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmdName The LabelString representing the command name.
     * @param cmdArgs The command arguments expressed as doubles.
     * @param abrtAck The expression in which to store an acknowledgment of command abort.
     * @param cmdAck The acknowledgment of the pending command
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck) = 0;

    // Returns the current time.
    virtual double currentTime() = 0;

    virtual void setExec(const PlexilExecId& exec)
    {
      m_exec = exec;
      m_exec->setExternalInterface(m_id);
    }

    virtual ~ExternalInterface()   
    {
      m_id.remove();
    }


  protected:

    //this should eventually take a domain description as well
    ExternalInterface()
      : m_id(this) 
    {
    }

    PlexilExecId m_exec;

  private:
    ExternalInterfaceId m_id;
  };
}

#endif
