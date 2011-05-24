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

#include "VarBindingStateManager.hh"
#include "Debug.hh"
#include "Node.hh"
#include "Expression.hh"
#include "CoreExpressions.hh"

namespace PLEXIL {

  class BindingExecutingStateComputer : public StateComputer {
  public:
    BindingExecutingStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
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

      debugMsg("Node:getDestState",
	       "VarBinding:EXECUTING" << std::endl <<
	       node->getAncestorInvariantCondition()->toString() << std::endl <<
	       node->getInvariantCondition()->toString() << std::endl <<
	       node->getEndCondition()->toString());

      if(node->getAncestorInvariantCondition()->getValue() ==
	 BooleanVariable::FALSE())
      {
	debugMsg("Node:getDestState", "Destination: FINISHED. Ancestor invariant false.");
	return FINISHED_STATE;
      }
      else if(node->getInvariantCondition()->getValue() ==
	      BooleanVariable::FALSE()) 
      {
	debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  Invariant false.");
	return ITERATION_ENDED_STATE;
      }
      else if(node->getEndCondition()->getValue() ==
	      BooleanVariable::TRUE()) 
      {
	debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  End condition true.");
	return ITERATION_ENDED_STATE;
      }
      return NO_NODE_STATE;
    }
  };

  class BindingExecutingTransitionHandler : public TransitionHandler {
  public:
    BindingExecutingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, NodeState destState) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
		 node->getType().toString());
      checkError(node->getState() == EXECUTING_STATE,
		 "In state '" << node->getStateName().toString() << "', not EXECUTING.");
      checkError(destState == ITERATION_ENDED_STATE ||
		 destState == FINISHED_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      bool abort = false;
      if (node->getAncestorInvariantCondition()->getValue() ==
	 BooleanVariable::FALSE()) 
      {
	node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
	abort = true;
      }
      else if (node->getInvariantCondition()->getValue() ==
	      BooleanVariable::FALSE()) 
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
         abort = true;
      }
      else if(node->getPostCondition()->getValue() ==
	      BooleanVariable::TRUE()) {
	node->getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
      }
      else {
	node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	node->getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	abort = true;
      }

      if(abort)
		node->abort();

      node->deactivateAncestorInvariantCondition();
      node->deactivateInvariantCondition();
      node->deactivateEndCondition();
      node->deactivatePostCondition();
      node->deactivateExecutable();
    }

    void transitionTo(NodeId& node, NodeState destState) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
		 node->getType().toString());
      checkError(destState == EXECUTING_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'.");

      node->activateAncestorInvariantCondition();
      node->activateInvariantCondition();
      node->activateEndCondition();
      node->activatePostCondition();

      node->setState(destState);
      node->execute();
    }
  };

  VarBindingStateManager::VarBindingStateManager() : DefaultStateManager() {
    addStateComputer(EXECUTING_STATE, (new BindingExecutingStateComputer())->getId());
    addTransitionHandler(EXECUTING_STATE,
			 (new BindingExecutingTransitionHandler())->getId());
  }
}
