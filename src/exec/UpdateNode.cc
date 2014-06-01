/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "BooleanOperators.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionConstants.hh"
#include "ExpressionFactory.hh"
#include "Function.hh"
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
  UpdateNode::UpdateNode(const std::string& type,
                         const std::string& name, 
                         const NodeState state,
                         const ExecConnectorId& exec,
                         const NodeId& parent)
    : Node(type, name, state, exec, parent)
  {
    checkError(type == UPDATE(),
               "Invalid node type \"" << type << "\" for an UpdateNode");

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
    debugMsg("UpdateNode:~UpdateNode", " destructor for " << m_nodeId);

    // MUST be called first, here. Yes, it's redundant with base class.
    cleanUpConditions();

    cleanUpNodeBody();
  }

  // Not useful if called from base class destructor!
  // Can be called redundantly, e.g. from ListNode::cleanUpChildConditions().
  void UpdateNode::cleanUpNodeBody()
  {
    if (m_update.isId()) {
      debugMsg("UpdateNode:cleanUpNodeBody", " removing update for " << m_nodeId);
      delete (Update*) m_update;
      m_update = UpdateId::noId();
    }
  }

  // Specific behaviors for derived classes
  void UpdateNode::specializedPostInit(const PlexilNodeId& node)
  {
    debugMsg("Node:postInit", "Creating update for node '" << m_nodeId << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilUpdateBody>::convertable(node->body()),
               "Node is an update node but doesn't have an update body.");
    createUpdate((PlexilUpdateBody*) node->body());

    // Create action-complete condition
    ExpressionId actionComplete = (ExpressionId) m_update->getAck();
    actionComplete->addListener(m_listener.getId());
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
    if (m_conditions[endIdx] == TRUE_EXP()) {
      // Default - don't wrap, replace - (True && anything) == anything
      m_conditions[endIdx] = ack;
      ack->addListener(m_listener.getId());
      m_garbageConditions[endIdx] = false;
    }
    else {
      // TODO: optimize to not create wrapper if end condition is constant and true
      // Wrap user-provided condition
      removeConditionListener(endIdx);
      ExpressionId realEnd =
        (new BinaryFunction<bool>(BooleanAnd::instance(),
                                  ack,
                                  m_conditions[endIdx],
                                  false,
                                  m_garbageConditions[endIdx]))->getId();
      realEnd->addListener(m_listener.getId());
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
               "Node::execute: Update is invalid");
    m_update->activate();
    m_update->fixValues();
    m_exec->enqueueUpdate(m_update);
  }

  NodeState UpdateNode::getDestStateFromExecuting()
  {
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Ancestor exit for " << m_nodeId << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Update node and ancestor exit true.");
      return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Exit for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Update node and exit true.");
      return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Ancestor invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Update node and ancestor invariant false.");
        return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FAILING. Update node and invariant false.");
      return FAILING_STATE;
    }

    cond = getEndCondition();
    checkError(cond->isActive(),
               "End for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: ITERATION_ENDED.  Update node and end condition true.");
      return ITERATION_ENDED_STATE;
    }
      
    debugMsg("Node:getDestState",
             " '" << m_nodeId << 
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
               << nodeStateName(destState) << "'");

    bool temp;
    if (getAncestorExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(PARENT_EXITED);
    }
    else if (getExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(EXITED);
    }
    else if (getAncestorInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(PARENT_FAILED);
    }
    else if (getInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(INVARIANT_CONDITION_FAILED);
    }
    else { // End true -> ITERATION_ENDED
      checkError(isPostConditionActive(),
                 "AssignmentNode::transitionFromExecuting: Post for " << m_nodeId << " is inactive.");
      if (getPostCondition()->getValue(temp) && temp) {
        setNodeOutcome(SUCCESS_OUTCOME);
      }
      else {
        setNodeOutcome(FAILURE_OUTCOME);
        setNodeFailureType(POST_CONDITION_FAILED);
      }
    }

    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    if (destState == FAILING_STATE) {
      deactivateAncestorExitInvariantConditions();
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
    ExpressionId cond = getActionCompleteCondition();
    checkError(cond->isActive(),
               "Action complete for " << m_nodeId << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      if (getFailureType() == PARENT_FAILED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED. Update node, action complete true, and parent failed.");
        return FINISHED_STATE;
      }
      if (getFailureType() == PARENT_EXITED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED. Update node, action complete true, and parent exited.");
        return FINISHED_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: ITERATION_ENDED. Update node, action complete true, and exited or invariant failed.");
        return ITERATION_ENDED_STATE;
      }
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: no state. Update node and action complete false or unknown.");
      return NO_NODE_STATE;
    }
  }

  void UpdateNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE || destState == ITERATION_ENDED_STATE,
               "Attempting to transition Update node from FAILING to invalid state '"
               << nodeStateName(destState) << "'");

    deactivateActionCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
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

  void UpdateNode::abort()
  {
  }

}
