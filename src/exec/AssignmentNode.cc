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

#include "AssignmentNode.hh"
#include "Assignment.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"

namespace PLEXIL
{

  AssignmentNode::AssignmentNode(const PlexilNodeId& nodeProto, 
                                 const ExecConnectorId& exec,
                                 const NodeId& parent)
    : Node(nodeProto, exec, parent),
      m_priority(nodeProto->priority())
  {
    checkError(nodeProto->nodeType() == NodeType_Assignment,
               "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
               << "\" for an AssignmentNode");
  }

  // Used only by module test
  AssignmentNode::AssignmentNode(const LabelStr& type,
                                 const LabelStr& name, 
                                 const NodeState state,
                                 const ExecConnectorId& exec,
                                 const NodeId& parent)
    : Node(type, name, state, exec, parent)
  {
    checkError(type == ASSIGNMENT(),
               "Invalid node type \"" << type.toString() << "\" for an AssignmentNode");

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
             "Creating assignment for node '" << m_nodeId.toString() << "'");
    // XML parser should have checked for this
    checkError(Id<PlexilAssignmentBody>::convertable(node->body()),
               "Node is an assignment node but doesn't have an assignment body.");
    createAssignment((PlexilAssignmentBody*) node->body());

    // Set action-complete condition
    ExpressionId ack = (ExpressionId) m_assignment->getAck();
    ack->addListener(makeConditionListener(actionCompleteIdx));
    m_conditions[actionCompleteIdx] = ack;
    m_garbageConditions[actionCompleteIdx] = false;

    // Set abort-complete condition
    ExpressionId abortComplete = (ExpressionId) m_assignment->getAbortComplete();
    abortComplete->addListener(makeConditionListener(abortCompleteIdx));
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
    VariableId dest;
    LabelStr destName;
    bool deleteLhs = false;
    if (Id<PlexilVarRef>::convertable(destExpr)) {
      destName = destExpr->name();
      dest = findVariable((Id<PlexilVarRef>) destExpr);
      // FIXME: push this check up into XML parser
      checkError(dest.isValid(),
                 "Dest variable '" << destName <<
                 "' not found in assignment node '" << m_nodeId.toString() << "'");
    }
    else if (Id<PlexilArrayElement>::convertable(destExpr)) {
      dest =
        (VariableId)
        ExpressionFactory::createInstance(destExpr->name(),
                                          destExpr,
                                          NodeConnector::getId());
      // *** beef this up later ***
      PlexilArrayElement* arrayElement = (PlexilArrayElement*) destExpr;
      debugMsg("ArrayElement:ArrayElement", " name = " << arrayElement->getArrayName() << ". To: " << dest->toString());
      size_t e_index = dest->toString().find(": ", dest->toString().length()-15);
      size_t b_index = dest->toString().find("u]", dest->toString().length()-40) + 2;
      int diff_index = e_index - b_index;
      std::string m_index = " ";
      if(e_index != std::string::npos)
        {

          m_index = dest->toString().substr(e_index-diff_index,diff_index);
        }
      debugMsg("ArrayElement:ArrayElement", " b_index = " << b_index << ". e_index = " << e_index << ". diff_index" << diff_index);
      const std::string m_str = std::string("").append(arrayElement->getArrayName()).append(m_index);
      destName = LabelStr(m_str);
      deleteLhs = true;
    }
    else {
      // FIXME: push this check up into XML parser 
      checkError(ALWAYS_FAIL, "Invalid left-hand side to an assignment");
    }

    bool deleteRhs = false;
    ExpressionId rhs =
      ExpressionFactory::createInstance(body->RHS()->name(), 
                                        body->RHS(),
                                        NodeConnector::getId(),
                                        deleteRhs);
    m_assignment =
      (new Assignment(dest, rhs, deleteLhs, deleteRhs, destName, m_nodeId))->getId();
  }

  // Unit test variant of above
  void AssignmentNode::createDummyAssignment() 
  {
    VariableId dest = (new BooleanVariable(BooleanVariable::FALSE_VALUE()))->getId();
    LabelStr destName("dummy");
    m_assignment =
      (new Assignment(dest, BooleanVariable::TRUE_EXP(), true, false, destName, m_nodeId))->getId();
  }

  const VariableId& AssignmentNode::getAssignmentVariable() const
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
    checkError(isActionCompleteConditionActive(),
               "Node::getDestStateFromExecuting: Assignment-complete for " << m_nodeId.toString() << " is inactive.");
    if (getActionCompleteCondition()->getValue() != BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: no state. Assignment node and assignment-complete false or unknown.");
      return NO_NODE_STATE;
    }

    checkError(isAncestorExitConditionActive(),
               "Node::getDestStateFromExecuting: Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. Assignment node and ANCESTOR_EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isExitConditionActive(),
               "Node::getDestStateFromExecuting: Exit condition for " << m_nodeId.toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. Assignment node and EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Node::getDestStateFromExecuting: Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. Assignment node and Ancestor invariant false.");
      return FAILING_STATE;
    }

    checkError(isInvariantConditionActive(),
               "Node::getDestStateFromExecuting: Invariant for " << m_nodeId.toString() << " is inactive.");
    if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. Assignment node and Invariant false.");
      return FAILING_STATE;
    }

    checkError(isEndConditionActive(),
               "Node::getDestStateFromExecuting: End for " << m_nodeId.toString() << " is inactive.");
    if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. Assignment node and End condition true.");
      return ITERATION_ENDED_STATE;
    }

    return NO_NODE_STATE;
  }

  void AssignmentNode::specializedHandleExecution()
  {
    // Perform assignment
    checkError(m_assignment.isValid(),
               "Node::handleExecution: Assignment is invalid");
    m_assignment->activate();
    m_assignment->fixValue();
    m_exec->enqueueAssignment(m_assignment);
  }

  void AssignmentNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FAILING_STATE
               || destState == ITERATION_ENDED_STATE,
               "Attempting to transition AssignmentNode from EXECUTING to invalid state '"
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
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
    }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
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

    deactivateActionCompleteCondition();
    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    if (destState == FAILING_STATE) {
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
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
    checkError(isAbortCompleteConditionActive(),
               "Abort complete for " << getNodeId().toString() << " is inactive.");
    if (getAbortCompleteCondition()->getValue() != BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString()
               << "' destination: no state. Assignment node and abort complete false or unknown.");
      return NO_NODE_STATE;
    }

    double failureValue = m_failureTypeVariable->getValue();
    if (failureValue == FailureVariable::PARENT_FAILED()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FINISHED.  Assignment node, abort complete, and parent failed.");
      return FINISHED_STATE;
    }
    else if (failureValue == FailureVariable::PARENT_EXITED()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: FINISHED.  Assignment node, abort complete, and parent exited.");
      return FINISHED_STATE;
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << 
               "' destination: ITERATION_ENDED.  Assignment node and abort complete.");
      return ITERATION_ENDED_STATE;
    }
  }

  void AssignmentNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == FINISHED_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition Assignment node from FAILING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    deactivateAbortCompleteCondition();
    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
    }

    deactivateExecutable();
  }
    
  void AssignmentNode::abort()
  {
    check_error(m_assignment.isValid());
    debugMsg("Node:abort", "Aborting node " << m_nodeId.toString());
    m_exec->enqueueAssignmentForRetraction(m_assignment);
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
      debugMsg("AssignmentNode:cleanUpNodeBody", "<" << m_nodeId.toString() << "> Removing assignment.");
      delete (Assignment*) m_assignment;
      m_assignment = AssignmentId::noId();
    }
  }

}
