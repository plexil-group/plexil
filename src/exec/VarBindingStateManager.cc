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
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
		 node->getType().toString());
      checkError(node->getState() == StateVariable::EXECUTING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not EXECUTING.");
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
	return StateVariable::FINISHED();
      }
      else if(node->getInvariantCondition()->getValue() ==
	      BooleanVariable::FALSE()) 
      {
	debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  Invariant false.");
	return StateVariable::ITERATION_ENDED();
      }
      else if(node->getEndCondition()->getValue() ==
	      BooleanVariable::TRUE()) 
      {
	debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  End condition true.");
	return StateVariable::ITERATION_ENDED();
      }
      return StateVariable::NO_STATE();
    }
  };

  class BindingExecutingTransitionHandler : public TransitionHandler {
  public:
    BindingExecutingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
		 node->getType().toString());
      checkError(node->getState() == StateVariable::EXECUTING(),
		 "In state '" << node->getState().toString() << "', not EXECUTING.");
      checkError(destState == StateVariable::ITERATION_ENDED() ||
		 destState == StateVariable::FINISHED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

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
	handleAbort(node);

      node->deactivateAncestorInvariantCondition();
      node->deactivateInvariantCondition();
      node->deactivateEndCondition();
      node->deactivatePostCondition();
      deactivateExecutable(node);
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::ASSIGNMENT(),
		 "Expected assignment node, got " <<
		 node->getType().toString());
      checkError(destState == StateVariable::EXECUTING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'.");

      node->activateAncestorInvariantCondition();
      node->activateInvariantCondition();
      node->activateEndCondition();
      node->activatePostCondition();

      node->setState(destState);
      handleExecution(node);
    }
  };

  VarBindingStateManager::VarBindingStateManager() : DefaultStateManager() {
    addStateComputer(StateVariable::EXECUTING(), (new BindingExecutingStateComputer())->getId());
    addTransitionHandler(StateVariable::EXECUTING(),
			 (new BindingExecutingTransitionHandler())->getId());
  }
}
