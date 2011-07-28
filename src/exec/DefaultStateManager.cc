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

#include "DefaultStateManager.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Node.hh"

namespace PLEXIL {

  class DefaultInactiveStateComputer : public StateComputer {
  public:
    DefaultInactiveStateComputer() : StateComputer() {}

	// Node::setConditionDefaults() and Node::createConditions() need to track this method.
    NodeState getDestState(NodeId& node) {
      checkError(node->getState() == INACTIVE_STATE, "In state '" << node->getStateName().toString() << "', not INACTIVE.");
      checkError(node->isParentExecutingConditionActive(), "Parent executing for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isParentFinishedConditionActive(), "Parent finished for " << node->getNodeId().toString() << " is inactive.");

      if(node->getParentFinishedCondition()->getValue() ==
	 BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: FINISHED.");
	condDebugMsg(node->getParentFinishedCondition()->getValue() ==
		     BooleanVariable::TRUE(),
		     "Node:getDestState", "PARENT_FINISHED_CONDITION true.");
	return FINISHED_STATE;
      }
      if(node->getParentExecutingCondition()->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: WAITING.  PARENT_EXECUTING_CONDITION true");
	return WAITING_STATE;
      }
      debugMsg("Node:getDestState", "Destination: no state.");
      return NO_NODE_STATE;
    }
  };

  class DefaultWaitingStateComputer : public StateComputer {
  public:
    DefaultWaitingStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getState() == WAITING_STATE, "In state '" << node->getStateName().toString() << "', not WAITING.");
      checkError(node->isAncestorInvariantConditionActive(), "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isAncestorEndConditionActive(), "Ancestor end for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isSkipConditionActive(), "Skip for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isStartConditionActive(), "Start for " << node->getNodeId().toString() << " is inactive.");

      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE() ||
          node->getAncestorEndCondition()->getValue() == BooleanVariable::TRUE() ||
          node->getSkipCondition()->getValue() == BooleanVariable::TRUE())
      {
         debugMsg("Node:getDestState", "Destination: FINISHED.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getAncestorEndCondition()->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "ANCESTOR_END_CONDITION true.");
         condDebugMsg(node->getSkipCondition()->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "SKIP_CONDITION true.");
	        return FINISHED_STATE;
      }
      if(node->getStartCondition()->getValue() == BooleanVariable::TRUE()) {
	if(node->getPreCondition()->getValue() == BooleanVariable::TRUE()) {
	  debugMsg("Node:getDestState", "Destination: EXECUTING.  START_CONDITION and PRE_CONDITION are both true.");
	  return EXECUTING_STATE;
	}
	else {
	  debugMsg("Node:getDestState", "Destination: ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
	  return ITERATION_ENDED_STATE;
	}
      }
      debugMsg("Node:getDestState", "Destination: no state.  START_CONDITION false or unknown");
      return NO_NODE_STATE;
    }
  };

  class DefaultIterationEndedStateComputer : public StateComputer {
  public:
    DefaultIterationEndedStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getState() == ITERATION_ENDED_STATE,
		 "Node " << node->getNodeId().toString() << " in state " << node->getStateName().toString() << " not ITERATION_ENDED.");
      checkError(node->isAncestorInvariantConditionActive(), "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isAncestorEndConditionActive(), "Ancestor end for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isRepeatConditionActive(), "Repeat for " << node->getNodeId().toString() << " is inactive.");

      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE() ||
          node->getAncestorEndCondition()->getValue() == BooleanVariable::TRUE() ||
          node->getRepeatCondition()->getValue() == BooleanVariable::FALSE()) 
      {
         debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: FINISHED.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "ANCESTOR_INVARIANT false.");
         condDebugMsg(node->getAncestorEndCondition()->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "ANCESTOR_END true.");
         condDebugMsg(node->getRepeatCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "REPEAT_CONDITION false.");
         return FINISHED_STATE;
      }
      if(node->getRepeatCondition()->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: WAITING.  REPEAT_UNTIL true.");
	return WAITING_STATE;
      }
      debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: no state.  ANCESTOR_END false or unknown and REPEAT unknown.");
      return NO_NODE_STATE;
    }
  };

  class DefaultFinishedStateComputer : public StateComputer {
  public:
    DefaultFinishedStateComputer() : StateComputer() {}
    NodeState getDestState(NodeId& node) {
      checkError(node->getState() == FINISHED_STATE,
		 "Node " << node->getNodeId().toString() << " in state " << node->getStateName().toString() << " not ITERATION_ENDED.");
      checkError(node->isParentWaitingConditionActive(), "Parent waiting for " << node->getNodeId().toString() << " is inactive.");

      if(node->getParentWaitingCondition()->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: INACTIVE.  PARENT_WAITING true.");
	return INACTIVE_STATE;
      }
      debugMsg("Node:getDestState", "Destination: no state.  PARENT_WAITING false or unknown.");
      return NO_NODE_STATE;
    }
  };

  class DefaultInactiveTransitionHandler : public TransitionHandler {
  public:
    DefaultInactiveTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, NodeState destState) {
      checkError(node->getState() == INACTIVE_STATE,
		 "In state '" << node->getStateName().toString() << "', not INACTIVE.");
      checkError(destState == WAITING_STATE ||
		 destState == FINISHED_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      node->deactivateParentExecutingCondition();
      node->deactivateParentFinishedCondition();
      if(destState == FINISHED_STATE)
	node->getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
    }
    void transitionTo(NodeId& node, NodeState destState) {
      checkError(destState == INACTIVE_STATE,
		 "Attempted to transition to INACTIVE with computed dest state '" <<
		 StateVariable::nodeStateName(destState).toString());
      node->activateParentExecutingCondition();
      node->activateParentFinishedCondition();

      node->setState(destState);
    }
  };

  class DefaultWaitingTransitionHandler : public TransitionHandler {
  public:
    DefaultWaitingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, NodeState destState) {
      checkError(node->getState() == WAITING_STATE,
		 "In state '" << node->getStateName().toString() << "', not WAITING.");
      checkError(destState == FINISHED_STATE ||
		 destState == EXECUTING_STATE ||
		 destState == ITERATION_ENDED_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      node->deactivateStartCondition();
      node->deactivateSkipCondition();
      node->deactivateAncestorEndCondition();
      node->deactivateAncestorInvariantCondition();
      node->deactivatePreCondition();

      if(destState == FINISHED_STATE)
	node->getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
      else if(destState == ITERATION_ENDED_STATE) {
	node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	node->getFailureTypeVariable()->setValue(FailureVariable::PRE_CONDITION_FAILED());
      }
    }

    void transitionTo(NodeId& node, NodeState destState) {
      checkError(destState == WAITING_STATE,
		 "Attempted to transition to WAITING with computed dest state '" <<
		 StateVariable::nodeStateName(destState).toString());


      node->activateStartCondition();
      node->activatePreCondition();
      node->activateSkipCondition();
      node->activateAncestorEndCondition();
      node->activateAncestorInvariantCondition();

      node->setState(destState);
    }
  };

  class DefaultIterationEndedTransitionHandler : public TransitionHandler {
  public:
    DefaultIterationEndedTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, NodeState destState) {
      checkError(node->getState() == ITERATION_ENDED_STATE,
		 "In state '" << node->getStateName().toString() << "', not ITERATION_ENDED.");
      checkError(destState == FINISHED_STATE ||
		 destState == WAITING_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      if (node->getAncestorInvariantCondition()->getValue() ==
	 BooleanVariable::FALSE()) 
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }

      node->deactivateRepeatCondition();
      node->deactivateAncestorEndCondition();
      node->deactivateAncestorInvariantCondition();

      if(destState == WAITING_STATE)
		node->reset();
    }

    void transitionTo(NodeId& node, NodeState destState) {
      checkError(destState == ITERATION_ENDED_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      node->activateRepeatCondition();
      node->activateAncestorEndCondition();
      node->activateAncestorInvariantCondition();

      node->setState(destState);
    }
  };

  class DefaultFinishedTransitionHandler : public TransitionHandler {
  public:
    DefaultFinishedTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, NodeState destState) {
      checkError(node->getState() == FINISHED_STATE,
		 "In state '" << node->getStateName().toString() << "', not FINISHED.");
      checkError(destState == INACTIVE_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      node->deactivateParentWaitingCondition();
      node->reset();
    }

    void transitionTo(NodeId& node, NodeState destState) {
      checkError(destState == FINISHED_STATE,
		 "Attempting to transition to invalid state '"
		 << StateVariable::nodeStateName(destState).toString() << "'");

      node->activateParentWaitingCondition();
      node->setState(destState);
    }
  };

  DefaultStateManager::DefaultStateManager() : NodeStateManager() {
    //have defaults for INACTIVE, WAITING,
    addStateComputer(INACTIVE_STATE, (new DefaultInactiveStateComputer())->getId());
    addTransitionHandler(INACTIVE_STATE, (new DefaultInactiveTransitionHandler())->getId());
    addStateComputer(WAITING_STATE, (new DefaultWaitingStateComputer())->getId());
    addTransitionHandler(WAITING_STATE, (new DefaultWaitingTransitionHandler())->getId());
    addStateComputer(ITERATION_ENDED_STATE, (new DefaultIterationEndedStateComputer())->getId());
    addTransitionHandler(ITERATION_ENDED_STATE, (new DefaultIterationEndedTransitionHandler())->getId());
    addStateComputer(FINISHED_STATE, (new DefaultFinishedStateComputer())->getId());
    addTransitionHandler(FINISHED_STATE, (new DefaultFinishedTransitionHandler())->getId());

    //need to specialize EXECUTING (for list, cmd/update/request, assignment, function calls), FAILING (list, cmd/update/request),
    // FINISHING (just for lists)
//     addStateComputer(EXECUTING_STATE, (new StateComputerError())->getId());
//     addStateComputer(FAILING_STATE, (new StateComputerError())->getId());
//     addStateComputer(FINISHING_STATE, (new StateComputerError())->getId());
//     addTransitionHandler(EXECUTING_STATE, (new TransitionHandlerError())->getId());
//     addTransitionHandler(FAILING_STATE, (new TransitionHandlerError())->getId());
//     addTransitionHandler(FINISHING_STATE, (new TransitionHandlerError())->getId());
  }
}
