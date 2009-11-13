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

#include "ActionNodeStateManager.hh"
#include "NodeStateManager.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "CommandHandle.hh"

namespace PLEXIL {

  class ActionExecutingStateComputer : public StateComputer {
  public:
    ActionExecutingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());
      checkError(node->getState() == StateVariable::EXECUTING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not EXECUTING.");
      checkError(node->pairActive(Node::ANCESTOR_INVARIANT_CONDITION()),
		 "Ancestor invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::INVARIANT_CONDITION()),
		 "Invariant for " << node->getNodeId().toString() << " is inactive.");
      checkError(node->pairActive(Node::END_CONDITION()),
		 "End for " << node->getNodeId().toString() << " is inactive.");


      if (node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() == 
          BooleanVariable::FALSE())
      {
         if (node->getCondition(Node::END_CONDITION())->getValue() == BooleanVariable::TRUE()) 
         {
            debugMsg("Node:getDestState",
                     "Destination: FINISHED.  Ancestor invariant condition false and end " <<
                     "condition true.. ");
            return StateVariable::FINISHED();
         }
         else 
         {
            debugMsg("Node:getDestState",
                     "Destination: FAILING.  Ancestor invariant condition false and end " <<
                     "condition false or unknown.");
            return StateVariable::FAILING();
	}
      }
      if (node->getCondition(Node::INVARIANT_CONDITION())->getValue() == 
          BooleanVariable::FALSE())
      {
         if (node->getCondition(Node::END_CONDITION())->getValue() == BooleanVariable::TRUE()) {
            debugMsg("Node:getDestState",
                     "Destination: ITERATION_ENDED.  Invariant condition false and end " <<
                     "condition true.. ");
            return StateVariable::ITERATION_ENDED();
         }
         else 
         {
            debugMsg("Node:getDestState",
                     "Destination: FAILING.  Invariant condition false and end condition " <<
                     "false or unknown.");
            return StateVariable::FAILING();
         }
      }

      if ((node->getType() == Node::COMMAND()) && 
          (node ->getCondition(Node::COMMAND_HANDLE_RECEIVED_CONDITION())->getValue() ==
           BooleanVariable::TRUE()))
        {
          node->getCommandHandleVariable()->setValue(node->getAcknowledgementValue());
        }

      if(node->getCondition(Node::END_CONDITION())->getValue() == BooleanVariable::TRUE()) 
        {
          return StateVariable::ITERATION_ENDED();
        }
      
      debugMsg("Node:getDestState",
               "Destination from EXECUTING: no state." << std::endl <<
               "  Ancestor invariant: " 
               << node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->toString() 
               << std::endl <<
               "  Invariant: " << node->getCondition(Node::INVARIANT_CONDITION())->toString() 
               << std::endl <<
               "  End: " << node->getCondition(Node::END_CONDITION())->toString());
      return StateVariable::NO_STATE();
    }
  };

  class ActionExecutingTransitionHandler : public TransitionHandler {
  public:
    ActionExecutingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());

      checkError(node->getState() == StateVariable::EXECUTING(),
		 "In state '" << node->getState().toString() << "', not EXECUTING.");
      checkError(destState == StateVariable::FINISHED() ||
		 destState == StateVariable::FAILING() ||
		 destState == StateVariable::ITERATION_ENDED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      if (node->getCondition(Node::ANCESTOR_INVARIANT_CONDITION())->getValue() ==
          BooleanVariable::FALSE())
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
         if(node->getCondition(Node::END_CONDITION())->getValue() != BooleanVariable::TRUE())
	  handleAbort(node);
      }
      else if (node->getCondition(Node::INVARIANT_CONDITION())->getValue() ==
               BooleanVariable::FALSE()) 
      {
         node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
         node->getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
         if(node->getCondition(Node::END_CONDITION())->getValue() != BooleanVariable::TRUE())
            handleAbort(node);
      }
      else if(node->getCondition(Node::END_CONDITION())->getValue() ==
	      BooleanVariable::TRUE()) {
	if(node->getCondition(Node::POST_CONDITION())->getValue() != BooleanVariable::TRUE()) {
          node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  node->getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	}
	else
	  node->getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
      }
      else {
	checkError(ALWAYS_FAIL, "Should never get here.");
      }

      deactivatePair(node, Node::END_CONDITION());
      deactivatePair(node, Node::INVARIANT_CONDITION());
      deactivatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());
      deactivatePair(node, Node::POST_CONDITION());
      deactivatePair(node, Node::COMMAND_HANDLE_RECEIVED_CONDITION());
      deactivateExecutable(node);
    }

    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());

      checkError(destState == StateVariable::EXECUTING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      activatePair(node, Node::ANCESTOR_INVARIANT_CONDITION());
      activatePair(node, Node::INVARIANT_CONDITION());
      activatePair(node, Node::END_CONDITION());
      activatePair(node, Node::POST_CONDITION());
      activatePair(node, Node::COMMAND_HANDLE_RECEIVED_CONDITION());

      node->getStateVariable()->setValue(destState);
      handleExecution(node);
    }
  };

  class ActionFailingStateComputer : public StateComputer {
  public:
    ActionFailingStateComputer() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());
      checkError(node->getState() == StateVariable::FAILING(),
		 "Node " << node->getNodeId().toString() << " in state " <<
		 node->getState().toString() << " not FAILING.");
      checkError(node->pairActive(Node::ABORT_COMPLETE()),
		 "Abort complete for " << node->getNodeId().toString() << " is inactive.");


      if(node->getCondition(Node::ABORT_COMPLETE())->getValue() == BooleanVariable::TRUE()) {
	if(node->findVariable(Node::FAILURE_TYPE())->getValue() ==
	   FailureVariable::PARENT_FAILED()) {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  Command/Update/Request node abort complete, " <<
		   "and parent failed.");
	  return StateVariable::FINISHED();
	}
	else {
	  debugMsg("Node:getDestState",
		   "Destination: FINISHED.  Command/Update/Request node abort complete.");
	  return StateVariable::ITERATION_ENDED();
	}
      }

      debugMsg("Node:getDestState",
	       "Destination: no state.");
      return StateVariable::NO_STATE();
    }
  };

  class ActionFailingTransitionHandler : public TransitionHandler {
  public:
    ActionFailingTransitionHandler() : TransitionHandler() {}
    void transitionFrom(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());

      checkError(node->getState() == StateVariable::FAILING(),
		 "In state '" << node->getState().toString() << "', not FAILING.");
      checkError(destState == StateVariable::FINISHED() ||
		 destState == StateVariable::ITERATION_ENDED(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      deactivatePair(node, Node::ABORT_COMPLETE());

    }
    void transitionTo(NodeId& node, const LabelStr& destState) {
      checkError(node->getType() == Node::COMMAND() ||
		 node->getType() == Node::UPDATE() ||
		 node->getType() == Node::REQUEST(),
		 "Expected command, update, or request node, got " <<
		 node->getType().toString());

      checkError(destState == StateVariable::FAILING(),
		 "Attempting to transition to invalid state '" << destState.toString() << "'");

      activatePair(node, Node::ABORT_COMPLETE());

      node->findVariable(Node::STATE())->setValue(destState);
    }
  };

  ActionNodeStateManager::ActionNodeStateManager() : DefaultStateManager() {
    addStateComputer(StateVariable::EXECUTING(), (new ActionExecutingStateComputer())->getId());
    addTransitionHandler(StateVariable::EXECUTING(), (new ActionExecutingTransitionHandler())->getId());
    addStateComputer(StateVariable::FAILING(), (new ActionFailingStateComputer())->getId());
    addTransitionHandler(StateVariable::FAILING(), (new ActionFailingTransitionHandler())->getId());
  }
}
