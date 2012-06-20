/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#include "UpdateNode.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"
#include "Update.hh"

namespace PLEXIL
{
  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  UpdateNode::UpdateNode(const PlexilNodeId& node, 
                         const ExecConnectorId& exec, 
                         const NodeId& parent)
    : Node(node, exec, parent)
  {
    checkError(node->nodeType() == NodeType_Update,
               "Invalid node type \"" << PlexilParser::nodeTypeString(node->nodeType())
               << "\" for an UpdateNode");
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  UpdateNode::UpdateNode(const LabelStr& type,
                         const LabelStr& name, 
                         const NodeState state,
                         const ExecConnectorId& exec,
                         const NodeId& parent)
    : Node(type, name, state, exec, parent)
  {
    checkError(type == UPDATE(),
               "Invalid node type \"" << type.toString() << "\" for an UpdateNode");

    // Construct stuff as required for unit test
    createDummyUpdate();

    // Activate conditions not activated by the base class constructor
    switch (m_state) {
    case EXECUTING_STATE:
      m_update->activate();
      break;

    case FINISHING_STATE:
      checkError(ALWAYS_FAIL, "Invalid state FINISHING for an UpdateNode");
      break;

    case FAILING_STATE:
      activateActionCompleteCondition();
      m_update->activate();
      break;

    default:
      break;
    }
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  UpdateNode::~UpdateNode()
  {
    // Redundant with base class destructor
    cleanUpConditions();
    cleanUpNodeBody();
  }

  void UpdateNode::cleanUpNodeBody()
  {
    if (m_update.isId()) {
      debugMsg("Node:cleanUpConds", "<" << m_nodeId.toString() << "> Removing update.");
      delete (Update*) m_update;
      m_update = UpdateId::noId();
    }
  }

  // Specific behaviors for derived classes
  void UpdateNode::specializedPostInit(const PlexilNodeId& node)
  {
    debugMsg("Node:postInit", "Creating update for node '" << m_nodeId.toString() << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilUpdateBody>::convertable(node->body()),
               "Node is an update node but doesn't have an update body.");
    createUpdate((PlexilUpdateBody*) node->body());

    // Create action-complete condition
    ExpressionId actionComplete = (ExpressionId) m_update->getAck();
    actionComplete->addListener(makeConditionListener(actionCompleteIdx));
    m_conditions[actionCompleteIdx] = actionComplete;
    m_garbageConditions[actionCompleteIdx] = false;
  }

  void UpdateNode::createUpdate(const PlexilUpdateBody* body) 
  {
    m_update = (new Update(m_id, body->update()))->getId();
  }

  void UpdateNode::createConditionWrappers()
  {
    ExpressionId ack = (ExpressionId) m_update->getAck();
    if (m_conditions[endIdx] == BooleanVariable::TRUE_EXP()) {
      // Default - don't wrap, replace - (True && anything) == anything
      m_conditions[endIdx] = ack;
      ack->addListener(makeConditionListener(endIdx));
      m_garbageConditions[endIdx] = false;
    }
    else {
      // Wrap user-provided condition
      if (m_listeners[endIdx].isId())
        m_conditions[endIdx]->removeListener(m_listeners[endIdx]);
      else
        makeConditionListener(endIdx); // for effect
      ExpressionId realEnd = (new Conjunction(ack,
                                              false,
                                              m_conditions[endIdx],
                                              m_garbageConditions[endIdx]))->getId();
      realEnd->addListener(m_listeners[endIdx]);
      m_conditions[endIdx] = realEnd;
      m_garbageConditions[endIdx] = true;
    }
  }

  // Unit test variant
  void UpdateNode::createDummyUpdate() 
  {
    m_update = (new Update(m_id))->getId();
  }

  //
  // State transition logic
  //

  //
  // EXECUTING 
  // 
  // Legal predecessor states: WAITING
  // Conditions active: AncestorExit, AncestorInvariant, End, Exit, Invariant, Post
  // Legal successor states: FAILING, ITERATION_ENDED

  void UpdateNode::specializedHandleExecution()
  {
    checkError(m_update.isValid(),
               "Node::handleExecution: Update is invalid");
    m_update->activate();
    m_update->fixValues();
    m_exec->enqueueUpdate(m_update);
  }

  NodeState UpdateNode::getDestStateFromExecuting()
  {
    checkError(isAncestorExitConditionActive(),
               "Ancestor exit for " << getNodeId().toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Update node and ancestor exit true.");
      return FAILING_STATE;
    }

    checkError(isExitConditionActive(),
               "Exit for " << getNodeId().toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Update node and exit true.");
      return FAILING_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FAILING. Update node and ancestor invariant false.");
        return FAILING_STATE;
    }

    checkError(isInvariantConditionActive(),
               "Invariant for " << getNodeId().toString() << " is inactive.");
    if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FAILING. Update node and invariant false.");
      return FAILING_STATE;
    }

    checkError(isEndConditionActive(),
               "End for " << getNodeId().toString() << " is inactive.");
    if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: ITERATION_ENDED.  Update node and end condition true.");
      return ITERATION_ENDED_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << 
             "' destination from EXECUTING: no state.\n  Ancestor invariant: " 
             << getAncestorInvariantCondition()->toString()
             << "\n  Invariant: " << getInvariantCondition()->toString() 
             << "\n  End: " << getEndCondition()->toString());
    return NO_NODE_STATE;
  }

  void UpdateNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FAILING_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Update node from EXECUTING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
    }
    else if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::EXITED());
    }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::PARENT_FAILED());
    }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      m_outcomeVariable->setValue(OutcomeVariable::FAILURE());
      m_failureTypeVariable->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
    }
    else { // End true -> ITERATION_ENDED
      checkError(isPostConditionActive(),
                 "AssignmentNode::transitionFromExecuting: Post for " << m_nodeId.toString() << " is inactive.");
      if (getPostCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
        getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
      }
      else {
        getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
        getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
      }
    }

    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    if (destState == FAILING_STATE) {
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
      // N.B. FAILING waits on ActionComplete, *not* AbortComplete!
      activateActionCompleteCondition();
    }
    else { // ITERATION_ENDED
      activateAncestorEndCondition();

      deactivateExecutable();
    }

  }

  //
  // FAILING
  //
  // Legal predecessor states: EXECUTING
  // Conditions active: ActionComplete
  // Legal successor states: FINISHED, ITERATION_ENDED

  // *** N.B. Since abort is a no-op, monitors ActionComplete instead of AbortComplete! ***

  void UpdateNode::transitionToFailing()
  {
    abort(); // no-op for now
  }

  NodeState UpdateNode::getDestStateFromFailing()
  {
    checkError(isActionCompleteConditionActive(),
               "Action complete for " << getNodeId().toString() << " is inactive.");

    if (getActionCompleteCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_FAILED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FINISHED. Update node, action complete true, and parent failed.");
        return FINISHED_STATE;
      }
      if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_EXITED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: FINISHED. Update node, action complete true, and parent exited.");
        return FINISHED_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << 
                 "' destination: ITERATION_ENDED. Update node, action complete true, and exited or invariant failed.");
        return ITERATION_ENDED_STATE;
      }
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: no state. Update node and action complete false or unknown.");
      return NO_NODE_STATE;
    }
  }

  void UpdateNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE || destState == ITERATION_ENDED_STATE,
               "Attempting to transition Update node from FAILING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    deactivateActionCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
    }

    deactivateExecutable();
  }


  void UpdateNode::specializedDeactivateExecutable()
  {
    if (m_update.isValid())
      m_update->deactivate();
  }

  void UpdateNode::specializedReset()
  {
    m_update->reset();
  }

}
