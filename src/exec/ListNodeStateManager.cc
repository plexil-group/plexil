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
#include "NodeStateManager.hh"
#include "CoreExpressions.hh"
#include "Node.hh"
#include "Debug.hh"

namespace PLEXIL {

  class ListExecutingStateComputer : public StateComputer {
  public:
    ListExecutingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::EXECUTING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not EXECUTING.");
      checkError(node->isAncestorInvariantConditionActive(),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isInvariantConditionActive(),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isEndConditionActive(),
		 "End for " << node->getNodeId().toString() << " is inactive.");
      
      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE() ||
          node->getInvariantCondition()->getValue() == BooleanVariable::FALSE())
      {
         debugMsg("Node:getDestState", "Destination: FAILING.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState",
                      "List node and ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState",
                      "List node and INVARIANT_CONDITION false.");
         return StateVariable::FAILING();
      }
      if(node->getEndCondition()->getValue() == BooleanVariable::TRUE()) {
	debugMsg("Node:getDestState",
		 "Destination: FINISHING.  List node and END_CONDITION true.");
	return StateVariable::FINISHING();
      }
      debugMsg("Node:getDestState",
	       "Destination: no state.");
      return StateVariable::NO_STATE();
    }
  };

  class ListFailingStateComputer : public StateComputer {
  public:
    ListFailingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::FAILING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not FAILING.");
      checkError(node->isChildrenWaitingOrFinishedConditionActive(),
		 "Children waiting or finished for " << node->getNodeId().toString() <<
		 " is inactive.");

      if(node->getChildrenWaitingOrFinishedCondition()->getValue() ==
	 BooleanVariable::TRUE()) {
	if(node->findVariable(Node::FAILURE_TYPE())->getValue() ==
	   FailureVariable::PARENT_FAILED()) {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  List node and ALL_CHILDREN_WAITING_OR_FINISHED" <<
		   " true and parent failed.");
	  return StateVariable::FINISHED();
	}
	else {
	  debugMsg("Node:getDestState",
		   "Destination: ITERATION_ENDED.  List node and self-failure.");
	  return StateVariable::ITERATION_ENDED();
	}
      }
      debugMsg("Node:getDestState", "Destination: no state.");
      return StateVariable::NO_STATE();
    }
  };


  class ListFinishingStateComputer : public StateComputer {
  public:
    ListFinishingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::FINISHING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not FINISHING.");
      checkError(node->isAncestorInvariantConditionActive(),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isInvariantConditionActive(),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->isChildrenWaitingOrFinishedConditionActive(),
		 "Children waiting or finished for " << node->getNodeId().toString() <<
		 " is inactive.");

      if (node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE() ||
          node->getInvariantCondition()->getValue() == BooleanVariable::FALSE()) 
      {
         debugMsg("Node:getDestState",
                  "Destination: FAILING.");
         condDebugMsg(node->getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState",
                      "List node and ANCESTOR_INVARIANT_CONDITION false.");
         condDebugMsg(node->getInvariantCondition()->getValue() == BooleanVariable::FALSE(),
                      "Node:getDestState",
                      "List node and INVARIANT_CONDITION false.");
         return StateVariable::FAILING();
      }
      if(node->getChildrenWaitingOrFinishedCondition()->getValue() ==
	 BooleanVariable::TRUE()) {
	if(!node->getPostCondition()->isActive())
	  node->getPostCondition()->activate();

	if(BooleanVariable::falseOrUnknown(node->getPostCondition()->getValue())) {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  List node, ALL_CHILDREN_WAITING_OR_FINISHED " <<
		   "true and POST_CONDITION false or unknown.");
	  return StateVariable::ITERATION_ENDED();
	}

	debugMsg("Node:getDestState",
		 "Destination: ITERATION_ENDED.  List node and " <<
		 "ALL_CHILDREN_WAITING_OR_FINISHED and POST_CONDITION true.");
	return StateVariable::ITERATION_ENDED();
      }
      debugMsg("Node:getDestState",
	       "Destination: no state. ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
      return StateVariable::NO_STATE();
    }
  };

  class ListExecutingTransitionHandler : public TransitionHandler {
  public:
    ListExecutingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::EXECUTING(),
		 "In state '" << node->getState().toString() << "', not EXECUTING.");
      checkError(destState == StateVariable::FINISHING() ||
		 destState == StateVariable::FAILING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      if (node->getAncestorInvariantCondition()->getValue() ==
	 BooleanVariable::FALSE())
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }
      else if (node->getInvariantCondition()->getValue() ==
	 BooleanVariable::FALSE()) 
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
      }
      

      node->deactivateAncestorInvariantCondition();
      if(destState != StateVariable::FINISHING())
		node->deactivateInvariantCondition();
      node->deactivateEndCondition();
      // Any variables declared in this node also need to be deactivated.
      // Chucko 17 Dec 2009
      deactivateExecutable(node);
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(destState == StateVariable::EXECUTING(),
		 "Attempting to transition to inavlid state '" << destState.toString() << "'");

      node->activateAncestorInvariantCondition();
      node->activateInvariantCondition();
      node->activateEndCondition();

      node->setState(destState);
      handleExecution(node);
    }

  };

  class ListFailingTransitionHandler : public TransitionHandler {
  public:
    ListFailingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::FAILING(),
		 "In state '" << node->getState().toString() << "', not FAILING.");
      checkError(destState == StateVariable::ITERATION_ENDED() ||
		 destState == StateVariable::FINISHED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      node->deactivateChildrenWaitingOrFinishedCondition();
      
    }
    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(destState == StateVariable::FAILING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");
      
      node->activateChildrenWaitingOrFinishedCondition();

      node->setState(destState);

    }
  };

  class ListFinishingTransitionHandler : public TransitionHandler {
  public:
    ListFinishingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(node->getState() == StateVariable::FINISHING(),
		 "In state '" << node->getState().toString() << "', not FINISHING.");
      checkError(destState == StateVariable::ITERATION_ENDED() ||
		 destState == StateVariable::FAILING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      if (node->getAncestorInvariantCondition()->getValue() ==
          BooleanVariable::FALSE())
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }
      else if (node->getInvariantCondition()->getValue() ==
               BooleanVariable::FALSE())
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
      }
      else if(node->getPostCondition()->getValue() ==
	      BooleanVariable::TRUE())
         node->getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
      else {
	node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	node->getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
      }

      node->deactivateAncestorInvariantCondition();
      node->deactivateInvariantCondition();
      node->deactivateChildrenWaitingOrFinishedCondition();
      node->deactivatePostCondition();
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::LIST(),
		 "Expected node list, got " << node->getType().toString());
      checkError(destState == StateVariable::FINISHING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'.");

      node->activateAncestorInvariantCondition();
      node->activateChildrenWaitingOrFinishedCondition();
      node->activatePostCondition();

      node->setState(destState);
    }
  };

  ListNodeStateManager::ListNodeStateManager() : DefaultStateManager() {
    addStateComputer(StateVariable::EXECUTING(), (new ListExecutingStateComputer())->getId());
    addTransitionHandler(StateVariable::EXECUTING(),
			 (new ListExecutingTransitionHandler())->getId());

    addStateComputer(StateVariable::FAILING(), (new ListFailingStateComputer())->getId());
    addTransitionHandler(StateVariable::FAILING(),
			 (new ListFailingTransitionHandler())->getId());

    addStateComputer(StateVariable::FINISHING(), (new ListFinishingStateComputer())->getId());
    addTransitionHandler(StateVariable::FINISHING(),
			 (new ListFinishingTransitionHandler())->getId());
  }

}
