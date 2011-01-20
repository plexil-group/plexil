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

#ifndef _H_ExecDefs
#define _H_ExecDefs

#include "Id.hh"

#include <vector>
#include <list>
#include <utility>

namespace PLEXIL {

  // NOTE: this used to be 100000000, which somehow gets printed as
  // scientific notation in XML and doesn't parse correctly.  
  const double WORST_PRIORITY = 100000;

  class Expression;
  typedef Id<Expression> ExpressionId;

  class ExpressionListener;
  typedef Id<ExpressionListener> ExpressionListenerId;

  class Node;
  typedef Id<Node> NodeId;

  class NodeConnector;
  typedef Id<NodeConnector> NodeConnectorId;

  class PlexilExec;
  typedef Id<PlexilExec> PlexilExecId;
  
  class StateCache;
  typedef Id<StateCache> StateCacheId;

  class ExecListener;
  typedef Id<ExecListener> ExecListenerId;

  class ExecConnector;
  typedef Id<ExecConnector> ExecConnectorId;

  class ExternalInterface;
  typedef Id<ExternalInterface> ExternalInterfaceId;

  class Command;
  typedef Id<Command> CommandId;

  class Assignment;
  typedef Id<Assignment> AssignmentId;

  class Update;
  typedef Id<Update> UpdateId;

  typedef std::pair<double, std::vector<double> > State;
  typedef std::list<ExpressionId> ExpressionList;
  typedef std::list<ExpressionId>::iterator ExpressionListItr;
  typedef std::list<ExpressionId>::const_iterator ExpressionListConstItr;
  typedef std::vector<ExpressionId> Expressions;
  typedef double StateKey;
  typedef double LookupKey;

    /**
     * @brief Variable type enumerator.  An enumeration of plexil variable types.
     */

    enum PlexilType
    {
      UNKNOWN_TYPE, INTEGER, REAL, BOOLEAN, BLOB, ARRAY, STRING, TIME, 
      NODE_STATE, NODE_OUTCOME, FAILURE_TYPE, COMMAND_HANDLE,
    };

  /**
   * @brief Node state enumeration.
   * @note Must be in same order as StateVariable::ALL_STATES(). See CoreExpressions.cc.
   */
  enum NodeState {
    INACTIVE_STATE = 0,
    WAITING_STATE,
    EXECUTING_STATE,
    FINISHING_STATE,
    FINISHED_STATE,
    FAILING_STATE,
    ITERATION_ENDED_STATE,
    NODE_STATE_MAX
  };
}

#endif
