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

#include "EmptyNodeStateManager.hh"
#include "BooleanVariable.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Node.hh"

namespace PLEXIL
{
  class EmptyNodeExecutingStateComputer : public StateComputer 
  {
  public:
    EmptyNodeExecutingStateComputer() : StateComputer()
    {}
    NodeState getDestState(NodeId& node)
    {
      checkError(node->getType() == Node::EMPTY(),
		 "Expected empty node, got " <<
		 node->getType().toString());
      checkError(node->getState() == EXECUTING_STATE,
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getStateName().toString() << " not EXECUTING.");
      checkError(node->isAncestorInvariantConditionActive(),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isInvariantConditionActive(),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isEndConditionActive(),
		 "End for " << node->getNodeId().toString() << " is inactive.");

      if (node->getAncestorInvariantCondition()->getValue() ==
          BooleanVariable::FALSE_VALUE())
      {
         debugMsg("Node:getDestState", "Destination: FINISHED. Ancestor invariant false.");
         return FINISHED_STATE;
      }
      else if(node->getInvariantCondition()->getValue() ==
	      BooleanVariable::FALSE_VALUE())
      {
         debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  Invariant false.");
         return ITERATION_ENDED_STATE;
      }
      else if(node->getEndCondition()->getValue() ==
	      BooleanVariable::TRUE_VALUE())
	{
	  debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  End condition true.");
	  return ITERATION_ENDED_STATE;
	}
      return NO_NODE_STATE;
    }
  };

  EmptyNodeStateManager::EmptyNodeStateManager() : DefaultStateManager()
  {
    addStateComputer(EXECUTING_STATE, (new EmptyNodeExecutingStateComputer())->getId());
  }
}
