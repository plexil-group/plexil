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
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getState() == StateVariable::INACTIVE(), "In state '" << node->getState().toString() << "', not INACTIVE.");
      checkError(node->pairActive(Node::PARENT_EXECUTING_CONDITION()), "Parent executing for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::PARENT_FINISHED_CONDITION()), "Parent finished for " << node->getNodeId().toString() << " is inactive.");

      if(node->getCondition(Node::PARENT_FINISHED_CONDITION())->getValue() ==
	 BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: FINISHED.");
	condDebugMsg(node->getCondition(Node::PARENT_FINISHED_CONDITION())->getValue() ==
		     BooleanVariable::TRUE(),
		     "Node:getDestState", "PARENT_FINISHED_CONDITION true.");
	return StateVariable::FINISHED();
      }
      if(node->getCondition(Node::PARENT_EXECUTING_CONDITION())->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: WAITING.  PARENT_EXECUTING_CONDITION true");
	return StateVariable::WAITING();
      }
      debugMsg("Node:getDestState", "Destination: no state.");
      return StateVariable::NO_STATE();
    }
  };

  class DefaultWaitingStateComputer : public StateComputer {
  public:
    DefaultWaitingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getState() == StateVariable::WAITING(), "In state '" << node->getState().toString() << "', not WAITING.");
      checkError(node->pairActive(Node::ANCESTOR_INVARIANT_CONDITION()), "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::ANCESTOR_END_CONDITION()), "Ancestor end for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::SKIP_CONDITION()), "Skip for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::START_CONDITION()), "Start for " << node->getNodeId().toString() << " is inactive.");

      if (node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() == BooleanVariable::FALSE() ||
          node->getCondition(Node::ANCESTOR_END_CONDITION())->getValue() == BooleanVariable::TRUE() ||
          node->getCondition(Node::SKIP_CONDITION())->getValue() == BooleanVariable::TRUE())
      {
         debugMsg("Node:getDestState", "Destination: FINISHED.");
         condDebugMsg(node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getCondition(Node::ANCESTOR_END_CONDITION())->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "ANCESTOR_END_CONDITION true.");
         condDebugMsg(node->getCondition(Node::SKIP_CONDITION())->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "SKIP_CONDITION true.");
	        return StateVariable::FINISHED();
      }
      if(node->getCondition(Node::START_CONDITION())->getValue() == BooleanVariable::TRUE()) {
	if(node->getCondition(Node::PRE_CONDITION())->getValue() == BooleanVariable::TRUE()) {
	  debugMsg("Node:getDestState", "Destination: EXECUTING.  START_CONDITION and PRE_CONDITION are both true.");
	  return StateVariable::EXECUTING();
	}
	else {
	  debugMsg("Node:getDestState", "Destination: ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
	  return StateVariable::ITERATION_ENDED();
	}
      }
      debugMsg("Node:getDestState", "Destination: no state.  START_CONDITION false or unknown");
      return StateVariable::NO_STATE();
    }
  };

  class DefaultIterationEndedStateComputer : public StateComputer {
  public:
    DefaultIterationEndedStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getState() == StateVariable::ITERATION_ENDED(),
		 "Node " << node->getNodeId().toString() << " in state " << node->getState().toString() << " not ITERATION_ENDED.");
      checkError(node->pairActive(Node::ANCESTOR_INVARIANT_CONDITION()), "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::ANCESTOR_END_CONDITION()), "Ancestor end for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::REPEAT_CONDITION()), "Repeat for " << node->getNodeId().toString() << " is inactive.");

      if (node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() == BooleanVariable::FALSE() ||
          node->getCondition(Node::ANCESTOR_END_CONDITION())->getValue() == BooleanVariable::TRUE() ||
          node->getCondition(Node::REPEAT_CONDITION())->getValue() == BooleanVariable::FALSE()) 
      {
         debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: FINISHED.");
         condDebugMsg(node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "ANCESTOR_INVARIANT false.");
         condDebugMsg(node->getCondition(Node::ANCESTOR_END_CONDITION())->getValue() == BooleanVariable::TRUE(),
                      "Node:getDestState", "ANCESTOR_END true.");
         condDebugMsg(node->getCondition(Node::REPEAT_CONDITION())->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState", "REPEAT_CONDITION false.");
         return StateVariable::FINISHED();
      }
      if(node->getCondition(Node::REPEAT_CONDITION())->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: WAITING.  REPEAT_UNTIL true.");
	return StateVariable::WAITING();
      }
      debugMsg("Node:getDestState", "'" << node->getNodeId().toString() << "' destination: no state.  ANCESTOR_END false or unknown and REPEAT unknown.");
      return StateVariable::NO_STATE();
    }
  };

  class DefaultFinishedStateComputer : public StateComputer {
  public:
    DefaultFinishedStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getState() == StateVariable::FINISHED(),
		 "Node " << node->getNodeId().toString() << " in state " << node->getState().toString() << " not ITERATION_ENDED.");
      checkError(node->pairActive(Node::PARENT_WAITING_CONDITION()), "Parent waiting for " << node->getNodeId().toString() << " is inactive.");

      if(node->getCondition(Node::PARENT_WAITING_CONDITION())->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState", "Destination: INACTIVE.  PARENT_WAITING true.");
	return StateVariable::INACTIVE();
      }
      debugMsg("Node:getDestState", "Destination: no state.  PARENT_WAITING false or unknown.");
      return StateVariable::NO_STATE();
    }
  };

  class DefaultInactiveTransitionHandler : public TransitionHandler {
  public:
    DefaultInactiveTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getState() == StateVariable::INACTIVE(),
		 "In state '" << node->getState().toString() << "', not INACTIVE.");
      checkError(destState == StateVariable::WAITING() ||
		 destState == StateVariable::FINISHED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      deactivatePair(node, Node::PARENT_EXECUTING_CONDITION());
      deactivatePair(node, Node::PARENT_FINISHED_CONDITION());
      if(destState == StateVariable::FINISHED())
	node->getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
    }
    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(destState == StateVariable::INACTIVE(),
		 "Attempted to transition to INACTIVE with computed dest state '" <<
		 destState.toString());
      activatePair(node, Node::PARENT_EXECUTING_CONDITION());
      activatePair(node, Node::PARENT_FINISHED_CONDITION());

      node->getStateVariable()->setValue(destState);
    }
  };

  class DefaultWaitingTransitionHandler : public TransitionHandler {
  public:
    DefaultWaitingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getState() == StateVariable::WAITING(),
		 "In state '" << node->getState().toString() << "', not WAITING.");
      checkError(destState == StateVariable::FINISHED() ||
		 destState == StateVariable::EXECUTING() ||
		 destState == StateVariable::ITERATION_ENDED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      deactivatePair(node, Node::START_CONDITION());
      deactivatePair(node, Node::SKIP_CONDITION());
      deactivatePair(node, Node::ANCESTOR_END_CONDITION());
      deactivatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());
      deactivatePair(node, Node::PRE_CONDITION());

      if(destState == StateVariable::FINISHED())
	node->getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
      else if(destState == StateVariable::ITERATION_ENDED()) {
	node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	node->getFailureTypeVariable()->setValue(FailureVariable::PRE_CONDITION_FAILED());
      }
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(destState == StateVariable::WAITING(),
		 "Attempted to transition to WAITING with computed dest state '" <<
		 destState.toString());


      activatePair(node, Node::START_CONDITION());
      activatePair(node, Node::PRE_CONDITION());
      activatePair(node, Node::SKIP_CONDITION());
      activatePair(node, Node::ANCESTOR_END_CONDITION());
      activatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());

      node->findVariable(Node::STATE())->setValue(destState);
    }
  };

  class DefaultIterationEndedTransitionHandler : public TransitionHandler {
  public:
    DefaultIterationEndedTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getState() == StateVariable::ITERATION_ENDED(),
		 "In state '" << node->getState().toString() << "', not ITERATION_ENDED.");
      checkError(destState == StateVariable::FINISHED() ||
		 destState == StateVariable::WAITING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      if (node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() ==
	 BooleanVariable::FALSE()) 
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }

      deactivatePair(node, Node::REPEAT_CONDITION());
      deactivatePair(node, Node::ANCESTOR_END_CONDITION());
      deactivatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());

      if(destState == StateVariable::WAITING())
	handleReset(node);
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(destState == StateVariable::ITERATION_ENDED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      activatePair(node, Node::REPEAT_CONDITION());
      activatePair(node, Node::ANCESTOR_END_CONDITION());
      activatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());

      node->findVariable(Node::STATE())->setValue(destState);
    }
  };

  class DefaultFinishedTransitionHandler : public TransitionHandler {
  public:
    DefaultFinishedTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getState() == StateVariable::FINISHED(),
		 "In state '" << node->getState().toString() << "', not FINISHED.");
      checkError(destState == StateVariable::INACTIVE(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      deactivatePair(node, Node::PARENT_WAITING_CONDITION());
      handleReset(node);
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(destState == StateVariable::FINISHED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      activatePair(node, Node::PARENT_WAITING_CONDITION());
      node->findVariable(Node::STATE())->setValue(destState);
    }
  };

  DefaultStateManager::DefaultStateManager() : NodeStateManager() {
    //have defaults for INACTIVE, WAITING,
    addStateComputer(StateVariable::INACTIVE(), (new DefaultInactiveStateComputer())->getId());
    addTransitionHandler(StateVariable::INACTIVE(), (new DefaultInactiveTransitionHandler())->getId());
    addStateComputer(StateVariable::WAITING(), (new DefaultWaitingStateComputer())->getId());
    addTransitionHandler(StateVariable::WAITING(), (new DefaultWaitingTransitionHandler())->getId());
    addStateComputer(StateVariable::ITERATION_ENDED(), (new DefaultIterationEndedStateComputer())->getId());
    addTransitionHandler(StateVariable::ITERATION_ENDED(), (new DefaultIterationEndedTransitionHandler())->getId());
    addStateComputer(StateVariable::FINISHED(), (new DefaultFinishedStateComputer())->getId());
    addTransitionHandler(StateVariable::FINISHED(), (new DefaultFinishedTransitionHandler())->getId());

    //need to specialize EXECUTING (for list, cmd/update/request, assignment, function calls), FAILING (list, cmd/update/request),
    // FINISHING (just for lists)
//     addStateComputer(StateVariable::EXECUTING(), (new StateComputerError())->getId());
//     addStateComputer(StateVariable::FAILING(), (new StateComputerError())->getId());
//     addStateComputer(StateVariable::FINISHING(), (new StateComputerError())->getId());
//     addTransitionHandler(StateVariable::EXECUTING(), (new TransitionHandlerError())->getId());
//     addTransitionHandler(StateVariable::FAILING(), (new TransitionHandlerError())->getId());
//     addTransitionHandler(StateVariable::FINISHING(), (new TransitionHandlerError())->getId());
  }
}
