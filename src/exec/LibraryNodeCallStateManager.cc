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

#include "LibraryNodeCallStateManager.hh"
#include "NodeStateManager.hh"
#include "CoreExpressions.hh"
#include "Node.hh"
#include "Debug.hh"

namespace PLEXIL
{

   class LibNodeCallExecutingStateComputer : public StateComputer
   {
      public:
         LibNodeCallExecutingStateComputer() : StateComputer()
         {}
         NodeState getDestState(NodeId& node)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(node->getState() == EXECUTING_STATE,
                       "Node " << node->getNodeId().toString() << " in state " <<
                       node->getStateName().toString() << " not EXECUTING.");
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
               condDebugMsg(node->getAncestorInvariantCondition()->getValue() == 
                            BooleanVariable::FALSE(),
                            "Node:getDestState",
                            "Library node call and ANCESTOR_INVARIANT_CONDITION false or unknown.");
               condDebugMsg(node->getInvariantCondition()->getValue() ==
                            BooleanVariable::FALSE(),
                            "Node:getDestState",
                            "Library node call and INVARIANT_CONDITION false or unknown.");
               return FAILING_STATE;
            }
            if (node->getEndCondition()->getValue() == BooleanVariable::TRUE())
            {
               debugMsg("Node:getDestState",
                        "Destination: FINISHING.  Library node call and END_CONDITION true.");
               return FINISHING_STATE;
            }
            debugMsg("Node:getDestState",
                     "Destination: no state.");
            return NO_NODE_STATE;
         }
   };

   class LibNodeCallFailingStateComputer : public StateComputer
   {
      public:
         LibNodeCallFailingStateComputer() : StateComputer()
         {}
         NodeState getDestState(NodeId& node)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(node->getState() == FAILING_STATE,
                       "Node " << node->getNodeId().toString() << " in state " <<
                       node->getStateName().toString() << " not FAILING.");
            checkError(node->isChildrenWaitingOrFinishedConditionActive(),
                       "Children waiting or finished for " << node->getNodeId().toString() <<
                       " is inactive.");

            if (node->getChildrenWaitingOrFinishedCondition()->getValue() ==
                BooleanVariable::TRUE())
            {
               if (node->findVariable(Node::FAILURE_TYPE())->getValue() ==
                   FailureVariable::PARENT_FAILED())
               {
                  debugMsg("Node:getDestState",
                           "Destination: FINISHED.  List node and ALL_CHILDREN_WAITING_OR_FINISHED" <<
                           " true and parent failed.");
                  return FINISHED_STATE;
               }
               else
               {
                  debugMsg("Node:getDestState",
                           "Destination: ITERATION_ENDED.  List node and self-failure.");
                  return ITERATION_ENDED_STATE;
               }
            }
            debugMsg("Node:getDestState", "Destination: no state.");
            return NO_NODE_STATE;
         }
   };


   class LibNodeCallFinishingStateComputer : public StateComputer
   {
      public:
         LibNodeCallFinishingStateComputer() : StateComputer()
         {}
         NodeState getDestState(NodeId& node)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
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
               return FAILING_STATE;
            }
            if (node->getChildrenWaitingOrFinishedCondition()->getValue() ==
                BooleanVariable::TRUE())
            {
               if (!node->getPostCondition()->isActive())
                  node->getPostCondition()->activate();

               if (BooleanVariable::falseOrUnknown(node->getPostCondition()->getValue()))
               {
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

   class LibNodeCallExecutingTransitionHandler : public TransitionHandler
   {
      public:
         LibNodeCallExecutingTransitionHandler() : TransitionHandler()
         {}
         void transitionFrom(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(node->getState() == EXECUTING_STATE,
                       "In state '" << node->getStateName().toString() << "', not EXECUTING.");
            checkError(destState == FINISHING_STATE ||
                       destState == FAILING_STATE,
                       "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

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
            if (destState != FINISHING_STATE)
			  node->deactivateInvariantCondition();
            node->deactivateEndCondition();
            node->deactivateExecutable();
         }

         void transitionTo(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(destState == EXECUTING_STATE,
                       "Attempting to transition to inavlid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

            node->activateAncestorInvariantCondition();
            node->activateInvariantCondition();
            node->activateEndCondition();

            node->setState(destState);
            node->execute();
         }

   };

   class LibNodeCallFailingTransitionHandler : public TransitionHandler
   {
      public:
         LibNodeCallFailingTransitionHandler() : TransitionHandler()
         {}
         void transitionFrom(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(node->getState() == FAILING_STATE,
                       "In state '" << node->getStateName().toString() << "', not FAILING.");
            checkError(destState == ITERATION_ENDED_STATE ||
                       destState == FINISHED_STATE,
                       "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

            node->deactivateChildrenWaitingOrFinishedCondition();

         }
         void transitionTo(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(destState == FAILING_STATE,
                       "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

            node->activateChildrenWaitingOrFinishedCondition();

            node->setState(destState);

         }
   };

   class LibNodeCallFinishingTransitionHandler : public TransitionHandler
   {
      public:
         LibNodeCallFinishingTransitionHandler() : TransitionHandler()
         {}
         void transitionFrom(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected library node call, got " << node->getType().toString());
            checkError(node->getState() == FINISHING_STATE,
                       "In state '" << node->getStateName().toString() << "', not FINISHING.");
            checkError(destState == ITERATION_ENDED_STATE ||
                       destState == FAILING_STATE,
                       "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

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
            else if (node->getPostCondition()->getValue() ==
                     BooleanVariable::TRUE())
               node->getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
            else
            {
               node->getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
               node->getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
            }

            node->deactivateAncestorInvariantCondition();
            node->deactivateInvariantCondition();
            node->deactivateChildrenWaitingOrFinishedCondition();
            node->deactivatePostCondition();
         }

         void transitionTo(NodeId& node, NodeState destState)
         {
            checkError(node->getType() == Node::LIBRARYNODECALL(),
                       "Expected libray node call, got " << node->getType().toString());
            checkError(destState == FINISHING_STATE,
                       "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'.");

            node->activateAncestorInvariantCondition();
            node->activateChildrenWaitingOrFinishedCondition();
            node->activatePostCondition();

            node->setState(destState);
         }
   };

   LibraryNodeCallStateManager::LibraryNodeCallStateManager() : DefaultStateManager()
   {
      addStateComputer(EXECUTING_STATE, (new LibNodeCallExecutingStateComputer())->getId());
      addTransitionHandler(EXECUTING_STATE,
                           (new LibNodeCallExecutingTransitionHandler())->getId());

      addStateComputer(FAILING_STATE, (new LibNodeCallFailingStateComputer())->getId());
      addTransitionHandler(FAILING_STATE,
                           (new LibNodeCallFailingTransitionHandler())->getId());

      addStateComputer(FINISHING_STATE, (new LibNodeCallFinishingStateComputer())->getId());
      addTransitionHandler(FINISHING_STATE,
                           (new LibNodeCallFinishingTransitionHandler())->getId());
   }

}
