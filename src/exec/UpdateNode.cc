/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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
#include "Function.hh"
#include "PlexilExec.hh"
#include "Update.hh"

namespace PLEXIL
{

  UpdateNode::UpdateNode(char const *nodeId, NodeImpl *parent)
    : NodeImpl(nodeId, parent),
      m_update(nullptr)
  {
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  UpdateNode::UpdateNode(const std::string& type,
                         const std::string& name, 
                         NodeState state,
                         NodeImpl *parent)
    : NodeImpl(type, name, state, parent),
      m_update(new Update(this))
  {
    checkError(type == UPDATE,
               "Invalid node type " << type << " for an UpdateNode");

    // Activate conditions not activated by the base class constructor
    switch (m_state) {
    case EXECUTING_STATE:
      m_update->activate();
      break;

    case FINISHING_STATE:
      errorMsg("Invalid state FINISHING for an UpdateNode");
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

  void UpdateNode::cleanUpNodeBody()
  {
    if (m_cleanedBody)
      return;

    debugMsg("UpdateNode:cleanUpNodeBody", '<' << m_nodeId << '>');
    if (m_update)
      m_update->cleanUp();
    m_cleanedBody = true;
  }

  void UpdateNode::setUpdate(Update *upd)
  {
    m_update.reset(upd);

    // Get action-complete condition
    m_conditions[actionCompleteIdx] = m_update->getAck();
    m_garbageConditions[actionCompleteIdx] = false;
  }

  void UpdateNode::specializedCreateConditionWrappers()
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
        makeFunction(BooleanAnd::instance(),
                     ack,
                     m_conditions[endIdx],
                     false,
                     m_garbageConditions[endIdx]);
      m_garbageConditions[endIdx] = true;
    }
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

  void UpdateNode::specializedHandleExecution(PlexilExec *exec)
  {
    assertTrue_1(m_update);
    m_update->activate();
    m_update->fixValues();
    exec->enqueueUpdate(m_update.get());
  }

  bool UpdateNode::getDestStateFromExecuting()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor exit for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. Update node and ancestor exit true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Exit for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. Update node and exit true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor invariant for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. Update node and ancestor invariant false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Invariant for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. Update node and invariant false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = INVARIANT_CONDITION_FAILED;
        return true;
      }
    }

    if ((cond = getEndCondition()) && (!cond->getValue(temp) || !temp)) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "End for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> no change.");
      return false;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> ITERATION_ENDED. Update node and end condition true.");
    m_nextState = ITERATION_ENDED_STATE;
    if ((cond = getPostCondition()) && (!cond->getValue(temp) || !temp)) { 
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Node::getDestState: Post for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      m_nextOutcome = FAILURE_OUTCOME;
      m_nextFailureType = POST_CONDITION_FAILED;
    }
    else
      m_nextOutcome = SUCCESS_OUTCOME;
    return true;
  }

  void UpdateNode::transitionFromExecuting(PlexilExec *exec)
  {
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivateEndCondition();
    deactivatePostCondition();

    switch (m_nextState) {

    case FAILING_STATE:
      // N.B. FAILING waits on ActionComplete, *not* AbortComplete!
      deactivateAncestorExitInvariantConditions();
      activateActionCompleteCondition();
      break;

    case ITERATION_ENDED_STATE:
      deactivateExecutable(exec);
      activateAncestorEndCondition();
      break;

    default:
      errorMsg("Attempting to transition Update node from EXECUTING to invalid state "
               << nodeStateName(m_nextState));
      break;
    }
  }

  //
  // FAILING
  //
  // Legal predecessor states: EXECUTING
  // Conditions active: ActionComplete
  // Legal successor states: FINISHED, ITERATION_ENDED

  // *** N.B. Since abort is a no-op, monitors ActionComplete instead of AbortComplete! ***
  void UpdateNode::transitionToFailing(PlexilExec * /* exec */)
  {
  }

  bool UpdateNode::getDestStateFromFailing()
  {
    Expression *cond = getActionCompleteCondition();
    bool temp;
    if (cond->getValue(temp) && temp) {
      if (getFailureType() == PARENT_FAILED) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. Update node, action complete true, and parent failed.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      if (getFailureType() == PARENT_EXITED) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. Update node, action complete true, and parent exited.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> ITERATION_ENDED. Update node, action complete true, and exited or invariant failed.");
      m_nextState = ITERATION_ENDED_STATE;
      return true;
    }

#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
    checkError(cond->isActive(),
               "Action complete for " << m_nodeId << ' ' << this << " is inactive.");
#endif
    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> no change. Update node and action complete false or unknown.");
    return false;
  }

  void UpdateNode::transitionFromFailing(PlexilExec *exec)
  {
    deactivateActionCompleteCondition();
    deactivateExecutable(exec);

    switch (m_nextState) {

    case ITERATION_ENDED_STATE:
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
      break;

    case FINISHED_STATE:
      // all done
      break;
      
    default:
      errorMsg("Attempting to transition Update node from FAILING to invalid state "
               << nodeStateName(m_nextState));
      break;
    }
  }


  void UpdateNode::specializedDeactivateExecutable(PlexilExec * /* exec */)
  {
    m_update->deactivate();
  }

}
