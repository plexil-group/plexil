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

#include "ListNodeStateManager.hh"
#include "BooleanVariable.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Node.hh"
#include "NodeStateManager.hh"

namespace PLEXIL {

  class ListExecutingStateComputer : public StateComputer {
  public:
    ListExecutingStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == EXECUTING_STATE,
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getStateName().toString() << " not EXECUTING.");
      checkError(node->isAncestorInvariantConditionActive(),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isInvariantConditionActive(),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isEndConditionActive(),
		 "End for " << node->getNodeId().toString() << " is inactive.");
      
      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE() ||
          node->getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE())
      {
         debugMsg("Node:getDestState", "Destination: FAILING.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
                      "Node:getDestState",
                      "List node and ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
                      "Node:getDestState",
                      "List node and INVARIANT_CONDITION false.");
         return FAILING_STATE;
      }
      if(node->getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	debugMsg("Node:getDestState",
		 "Destination: FINISHING.  List node and END_CONDITION true.");
	return FINISHING_STATE;
      }
      debugMsg("Node:getDestState",
	       "Destination: no state.");
      return NO_NODE_STATE;
    }
  };

  class ListFailingStateComputer : public StateComputer {
  public:
    ListFailingStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == FAILING_STATE,
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getStateName().toString() << " not FAILING.");
      checkError(node->isChildrenWaitingOrFinishedConditionActive(),
		 "Children waiting or finished for " << node->getNodeId().toString() <<
		 " is inactive.");

      if(node->getChildrenWaitingOrFinishedCondition()->getValue() ==
	 BooleanVariable::TRUE_VALUE()) {
	if(node->findVariable(Node::FAILURE_TYPE())->getValue() ==
	   FailureVariable::PARENT_FAILED()) {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  List node and ALL_CHILDREN_WAITING_OR_FINISHED" <<
		   " true and parent failed.");
	  return FINISHED_STATE;
	}
	else {
	  debugMsg("Node:getDestState",
		   "Destination: ITERATION_ENDED.  List node and self-failure.");
	  return ITERATION_ENDED_STATE;
	}
      }
      debugMsg("Node:getDestState", "Destination: no state.");
      return NO_NODE_STATE;
    }
  };


  class ListFinishingStateComputer : public StateComputer {
  public:
    ListFinishingStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == FINISHING_STATE,
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getStateName().toString() << " not FINISHING.");
      checkError(node->isAncestorInvariantConditionActive(),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isInvariantConditionActive(),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isChildrenWaitingOrFinishedConditionActive(),
		 "Children waiting or finished for " << node->getNodeId().toString() <<
		 " is inactive.");

      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE() ||
          node->getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) 
      {
         debugMsg("Node:getDestState",
                  "Destination: FAILING.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
                      "Node:getDestState",
                      "List node and ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
                      "Node:getDestState",
                      "List node and INVARIANT_CONDITION false.");
         return FAILING_STATE;
      }
      if(node->getChildrenWaitingOrFinishedCondition()->getValue() ==
	 BooleanVariable::TRUE_VALUE()) {
	if(!node->getPostCondition()->isActive())
	  node->getPostCondition()->activate();

	if(BooleanVariable::falseOrUnknown(node->getPostCondition()->getValue())) {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  List node, ALL_CHILDREN_WAITING_OR_FINISHED " <<
		   "true and POST_CONDITION false or unknown.");
	  return ITERATION_ENDED_STATE;
	}

	debugMsg("Node:getDestState",
		 "Destination: ITERATION_ENDED.  List node and " <<
		 "ALL_CHILDREN_WAITING_OR_FINISHED and POST_CONDITION true.");
	return ITERATION_ENDED_STATE;
      }
      debugMsg("Node:getDestState",
	       "Destination: no state. ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
      return NO_NODE_STATE;
    }
  };

  ListNodeStateManager::ListNodeStateManager() : DefaultStateManager() {
    addStateComputer(EXECUTING_STATE, (new ListExecutingStateComputer())->getId());
    addStateComputer(FAILING_STATE, (new ListFailingStateComputer())->getId());
    addStateComputer(FINISHING_STATE, (new ListFinishingStateComputer())->getId());
  }

}
