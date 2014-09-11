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
#include "Error.hh"
#include "ExpressionConstants.hh"
#include "ExpressionFactory.hh"
#include "ExternalInterface.hh"
#include "Function.hh"
#include "Update.hh"

namespace PLEXIL
{
  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  UpdateNode::UpdateNode(PlexilNode const *node, 
                         Node *parent)
    : Node(node, parent)
  {
    checkError(node->nodeType() == NodeType_Update,
               "Invalid node type \"" << nodeTypeString(node->nodeType())
               << "\" for an UpdateNode");
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  UpdateNode::UpdateNode(const std::string& type,
                         const std::string& name, 
                         NodeState state,
                         Node *parent)
    : Node(type, name, state, parent)
  {
    checkError(type == UPDATE,
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
    if (m_update) {
      debugMsg("UpdateNode:cleanUpNodeBody", " removing update for " << m_nodeId);
      delete m_update;
      m_update = NULL;
    }
  }

  // Specific behaviors for derived classes
  void UpdateNode::specializedPostInit(PlexilNode const *node)
  {
    debugMsg("Node:postInit", "Creating update for node '" << m_nodeId << "'");
    // XML parser should have checked for this
    assertTrue_2(dynamic_cast<PlexilUpdateBody const *>(node->body()),
                 "Node is an update node but doesn't have an update body.");
    createUpdate((PlexilUpdateBody const *) node->body());

    // Create action-complete condition
    Expression *actionComplete = m_update->getAck();
    m_conditions[actionCompleteIdx] = actionComplete;
    m_garbageConditions[actionCompleteIdx] = false;
  }

  void UpdateNode::createUpdate(PlexilUpdateBody const *body) 
  {
    m_update = new Update(this, body->update());
  }

  void UpdateNode::createConditionWrappers()
  {
    Expression *ack = m_update->getAck();
    if (!(m_conditions[endIdx]) || m_conditions[endIdx] == TRUE_EXP()) {
      // Default - don't wrap, replace - (True && anything) == anything
      m_conditions[endIdx] = ack;
      m_garbageConditions[endIdx] = false;
    }
    else {
      // Wrap user-provided condition
      m_conditions[endIdx] =
        new Function(BooleanAnd::instance(),
                     ack,
                     m_conditions[endIdx],
                     false,
                     m_garbageConditions[endIdx]);
      m_garbageConditions[endIdx] = true;
    }
  }

  // Unit test variant
  void UpdateNode::createDummyUpdate() 
  {
    m_update = new Update(this);
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
    assertTrue_1(m_update);
    m_update->activate();
    m_update->execute();
  }

  bool UpdateNode::getDestStateFromExecuting()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
      checkError(cond->isActive(),
                 "Ancestor exit for " << m_nodeId << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Update node and ancestor exit true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
      checkError(cond->isActive(),
                 "Exit for " << m_nodeId << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Update node and exit true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
      checkError(cond->isActive(),
                 "Ancestor invariant for " << m_nodeId << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Update node and ancestor invariant false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getInvariantCondition())) {
      checkError(cond->isActive(),
                 "Invariant for " << m_nodeId << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FAILING. Update node and invariant false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = INVARIANT_CONDITION_FAILED;
        return true;
      }
    }

    if ((cond = getEndCondition()) && (!cond->getValue(temp) || !temp)) {
      checkError(cond->isActive(),
                 "End for " << m_nodeId << " is inactive.");
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination from EXECUTING: no state.");
      m_nextState = NO_NODE_STATE;
      return false;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId << 
             "' destination: ITERATION_ENDED.  Update node and end condition true.");
    m_nextState = ITERATION_ENDED_STATE;
    if ((cond = getPostCondition()) && (!cond->getValue(temp) || !temp)) { 
      checkError(cond->isActive(),
                 "Node::getDestState: Post for " << m_nodeId << " is inactive.");
      m_nextOutcome = FAILURE_OUTCOME;
      m_nextFailureType = POST_CONDITION_FAILED;
    }
    else
      m_nextOutcome = SUCCESS_OUTCOME;
    return true;
  }

  void UpdateNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FAILING_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Update node from EXECUTING to invalid state '"
               << nodeStateName(destState) << "'");

    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivateEndCondition();
    deactivatePostCondition();

    if (destState == FAILING_STATE) {
      // N.B. FAILING waits on ActionComplete, *not* AbortComplete!
      deactivateAncestorExitInvariantConditions();
      activateActionCompleteCondition();
    }
    else { // ITERATION_ENDED
      deactivateExecutable();
      activateAncestorEndCondition();
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

  bool UpdateNode::getDestStateFromFailing()
  {
    Expression *cond = getActionCompleteCondition();
    bool temp;
    if (cond->getValue(temp) && temp) {
      if (getFailureType() == PARENT_FAILED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED. Update node, action complete true, and parent failed.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      if (getFailureType() == PARENT_EXITED) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << 
                 "' destination: FINISHED. Update node, action complete true, and parent exited.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: ITERATION_ENDED. Update node, action complete true, and exited or invariant failed.");
      m_nextState = ITERATION_ENDED_STATE;
      return true;
    }

    checkError(cond->isActive(),
               "Action complete for " << m_nodeId << " is inactive.");
    debugMsg("Node:getDestState",
             " '" << m_nodeId << 
             "' destination: no state. Update node and action complete false or unknown.");
    m_nextState = NO_NODE_STATE;
    return false;
  }

  void UpdateNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE || destState == ITERATION_ENDED_STATE,
               "Attempting to transition Update node from FAILING to invalid state '"
               << nodeStateName(destState) << "'");

    deactivateActionCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
    }

    deactivateExecutable();
  }


  void UpdateNode::specializedDeactivateExecutable()
  {
    assertTrue_1(m_update);
    m_update->deactivate();
  }

  void UpdateNode::specializedReset()
  {
    assertTrue_1(m_update);
    m_update->reset();
  }

  void UpdateNode::abort()
  {
  }

}
