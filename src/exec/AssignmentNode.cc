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

#include "AssignmentNode.hh"
#include "Assignment.hh"
#include "UserVariable.hh"
#include "Constant.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"

namespace PLEXIL
{

  AssignmentNode::AssignmentNode(const PlexilNodeId& nodeProto, 
                                 const NodeId& parent)
    : Node(nodeProto, parent),
      m_priority(nodeProto->priority())
  {
    checkError(nodeProto->nodeType() == NodeType_Assignment,
               "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
               << "\" for an AssignmentNode");
  }

  // Used only by module test
  AssignmentNode::AssignmentNode(const std::string &type,
                                 const std::string &name, 
                                 const NodeState state,
                                 const NodeId& parent)
    : Node(type, name, state, parent)
  {
    checkError(type == ASSIGNMENT(),
               "Invalid node type \"" << type << "\" for an AssignmentNode");

    // Create Assignment object
    createDummyAssignment();

    switch (state) {
    case EXECUTING_STATE:
      m_assignment->activate();
      activateActionCompleteCondition();
      break;

    case FAILING_STATE:
      m_assignment->activate();
      activateAbortCompleteCondition();
      break;

    case FINISHING_STATE:
      checkError(ALWAYS_FAIL, "Invalid state FINISHING for an AssignmentNode");
      break;

    default:
      break;
    }
  }

  AssignmentNode::~AssignmentNode()
  {
    cleanUpConditions();
    cleanUpNodeBody();
  }

  void AssignmentNode::specializedPostInit(const PlexilNodeId& node)
  {
    debugMsg("Node:postInit",
             "Creating assignment for node '" << m_nodeId << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilAssignmentBody>::convertable(node->body()),
               "Node is an assignment node but doesn't have an assignment body.");
    createAssignment((PlexilAssignmentBody*) node->body());

    // Set action-complete condition
    ExpressionId ack = (ExpressionId) m_assignment->getAck();
    ack->addListener(m_listener.getId());
    m_conditions[actionCompleteIdx] = ack;
    m_garbageConditions[actionCompleteIdx] = false;

    // Set abort-complete condition
    ExpressionId abortComplete = (ExpressionId) m_assignment->getAbortComplete();
    abortComplete->addListener(m_listener.getId());
    m_conditions[abortCompleteIdx] = abortComplete;
    m_garbageConditions[abortCompleteIdx] = false;
  }

  void AssignmentNode::createAssignment(const PlexilAssignmentBody* body) 
  {
    //we still only support one variable on the LHS
    // FIXME: push this check up into XML parser
    checkError(body->dest().size() >= 1,
               "Need at least one destination variable in assignment.");
    const PlexilExprId& destExpr = (body->dest())[0]->getId();
    ExpressionId dest;
    bool deleteLhs = false;
    if (Id<PlexilVarRef>::convertable(destExpr)) {
      dest = findVariable((Id<PlexilVarRef>) destExpr);
      // FIXME: push this check up into XML parser?
      assertTrueMsg(dest.isId(),
                    "Dest variable '" << destExpr->name() <<
                    "' not found in assignment node '" << m_nodeId << "'");
    }
    else if (Id<PlexilArrayElement>::convertable(destExpr)) {
      // FIXME: Construct MutableArrayReference
      dest = createExpression(destExpr, NodeConnector::getId());
      // *** beef this up later ***
      PlexilArrayElement* arrayElement = (PlexilArrayElement*) destExpr;
      debugMsg("ArrayElement:ArrayElement", " name = " << arrayElement->getArrayName() << ". To: " << dest->toString());
      deleteLhs = true;
    }
    else {
      // FIXME: push this check up into XML parser 
      checkError(ALWAYS_FAIL, "Invalid left-hand side to an assignment");
    }
    
    assertTrueMsg(dest->isAssignable(),
                  "Assignment destination " << dest->toString() << " is not writable");
    bool deleteRhs = false;
    ExpressionId rhs = createExpression(body->RHS(),
                                        NodeConnector::getId(),
                                        deleteRhs);
    m_assignment =
      (new Assignment(dest->asAssignable(), rhs, deleteLhs, deleteRhs, m_nodeId))->getId();
  }

  // Unit test variant of above
  void AssignmentNode::createDummyAssignment() 
  {
    ExpressionId dest = (new BooleanVariable(false))->getId();
    m_assignment =
      (new Assignment(dest->asAssignable(), (new BooleanConstant(true))->getId(), true, true, m_nodeId))->getId();
  }

  ExpressionId AssignmentNode::getAssignmentVariable() const
  {
    return m_assignment->getDest();
  }

  //
  // Transition handlers
  //

  //
  // EXECUTING 
  // 
  // Description and methods here are for Assignment node only
  //
  // Legal predecessor states: WAITING
  // Conditions active: ActionComplete, AncestorExit, AncestorInvariant, End, Exit, Invariant, Post
  // Legal successor states: FAILING, ITERATION_ENDED

  void AssignmentNode::transitionToExecuting()
  {
    Node::transitionToExecuting();
    activateActionCompleteCondition();
  }

  NodeState AssignmentNode::getDestStateFromExecuting()
  {
    // Not eligible to transition from EXECUTING until the assignment has been executed.
    ExpressionId cond = getActionCompleteCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Assignment-complete for " << m_nodeId << " is inactive.");
    bool temp;
    if (!cond->getValue(temp) || !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: no state. Assignment node and assignment-complete false or unknown.");
      return NO_NODE_STATE;
    }

    cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Ancestor exit for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FAILING. Assignment node and ANCESTOR_EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Exit condition for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FAILING. Assignment node and EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Ancestor invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FAILING. Assignment node and Ancestor invariant false.");
      return FAILING_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FAILING. Assignment node and Invariant false.");
      return FAILING_STATE;
    }

    cond = getEndCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: End for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: ITERATION_ENDED. Assignment node and End condition true.");
      return ITERATION_ENDED_STATE;
    }

    return NO_NODE_STATE;
  }

  void AssignmentNode::specializedHandleExecution()
  {
    // Perform assignment
    checkError(m_assignment.isValid(),
               "Node::execute: Assignment is invalid");
    m_assignment->activate();
    m_assignment->fixValue();
    g_exec->enqueueAssignment(m_assignment);
  }

  void AssignmentNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FAILING_STATE
               || destState == ITERATION_ENDED_STATE,
               "Attempting to transition AssignmentNode from EXECUTING to invalid state '"
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

    deactivateActionCompleteCondition();
    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    if (destState == FAILING_STATE) {
      deactivateAncestorExitInvariantConditions();
    }
    else { // ITERATION_ENDED
      activateAncestorEndCondition();

      deactivateExecutable();
    }
  }
    
  //
  // FAILING
  //
  // Description and methods here apply only to Assignment nodes
  //
  // Legal predecessor states: EXECUTING
  // Conditions active: AbortComplete
  // Legal successor states: FINISHED, ITERATION_ENDED

  void AssignmentNode::transitionToFailing()
  {
    activateAbortCompleteCondition();
    abort();
  }

  NodeState AssignmentNode::getDestStateFromFailing()
  {
    ExpressionId cond = getAbortCompleteCondition();
    checkError(cond->isActive(),
               "Abort complete for " << getNodeId() << " is inactive.");
    bool temp;
    if (!cond->getValue(temp) || !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId
               << "' destination: no state. Assignment node and abort complete false or unknown.");
      return NO_NODE_STATE;
    }

    FailureType failureValue = getFailureType();
    if (failureValue == PARENT_FAILED) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FINISHED.  Assignment node, abort complete, and parent failed.");
      return FINISHED_STATE;
    }
    else if (failureValue == PARENT_EXITED) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: FINISHED.  Assignment node, abort complete, and parent exited.");
      return FINISHED_STATE;
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << 
               "' destination: ITERATION_ENDED.  Assignment node and abort complete.");
      return ITERATION_ENDED_STATE;
    }
  }

  void AssignmentNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Assignment node from FAILING to invalid state '"
               << nodeStateName(destState) << "'");

    deactivateAbortCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
    }

    deactivateExecutable();
  }
    
  void AssignmentNode::abort()
  {
    check_error(m_assignment.isValid());
    debugMsg("Node:abort", "Aborting node " << m_nodeId);
    g_exec->enqueueAssignmentForRetraction(m_assignment);
  }

  void AssignmentNode::specializedReset()
  {
    m_assignment->reset();
  }

  void AssignmentNode::specializedDeactivateExecutable() 
  {
    if (m_assignment.isValid())
      m_assignment->deactivate();
  }

  void AssignmentNode::cleanUpNodeBody()
  {
    if (m_assignment.isId()) {
      debugMsg("AssignmentNode:cleanUpNodeBody", "<" << m_nodeId << "> Removing assignment.");
      delete (Assignment*) m_assignment;
      m_assignment = AssignmentId::noId();
    }
  }

}
