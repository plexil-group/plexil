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

#ifndef _H_ExecDefs
#define _H_ExecDefs

#include "generic_hash_map.hh"
#include "Id.hh"
#include "NodeConstants.hh"

#include <vector>
#include <utility>

namespace PLEXIL 
{

  // NOTE: this used to be 100000000, which somehow gets printed as
  // scientific notation in XML and doesn't parse correctly.  
  const double WORST_PRIORITY = 100000;

  class Assignable;
  typedef Id<Assignable> AssignableId;

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

  class ExecListener;
  typedef Id<ExecListener> ExecListenerId;

  class ExecListenerHub;
  typedef Id<ExecListenerHub> ExecListenerHubId;

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

  typedef std::vector<ExpressionId> ExpressionVector;
  typedef std::vector<ExpressionId>::iterator ExpressionVectorIter;
  typedef std::vector<ExpressionId>::const_iterator ExpressionVectorConstIter;
  typedef std::vector<ExpressionId> Expressions;
  typedef PLEXIL_HASH_MAP(std::string, AssignableId) VariableMap;

  /**
   * @brief A data structure for reporting node state transitions.
   */
  struct NodeTransition {
    NodeId node;
    NodeState state;
    // default constructor
    NodeTransition() : node(), state(INACTIVE_STATE) {}
    // trivial constructor
    NodeTransition(const NodeId& nod, NodeState stat) : node(nod), state(stat) {}
  };

}

#endif
