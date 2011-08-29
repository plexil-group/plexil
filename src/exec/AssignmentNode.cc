/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
	  m_ack((new BooleanVariable(BooleanVariable::UNKNOWN()))->getId()),
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
								 const bool skip,
								 const bool start,
								 const bool pre,
								 const bool invariant,
								 const bool post,
								 const bool end,
								 const bool repeat,
								 const bool ancestorInvariant,
								 const bool ancestorEnd,
								 const bool parentExecuting,
								 const bool childrenFinished,
								 const bool commandAbort,
								 const bool parentWaiting, 
								 const bool parentFinished,
								 const bool cmdHdlRcvdCondition,
								 const ExecConnectorId& exec)
	: Node(type, name, state, 
		   skip, start, pre, invariant, post, end, repeat,
		   ancestorInvariant, ancestorEnd, parentExecuting, childrenFinished,
		   commandAbort, parentWaiting, parentFinished, cmdHdlRcvdCondition,
		   exec),
	  m_ack((new BooleanVariable(BooleanVariable::UNKNOWN()))->getId())
  {
	checkError(type == ASSIGNMENT(),
			   "Invalid node type \"" << type.toString() << "\" for an AssignmentNode");
	createDummyAssignment();
	if (state == EXECUTING_STATE)
	  m_assignment->activate();
  }

  AssignmentNode::~AssignmentNode()
  {
	cleanUpConditions();
	cleanUpNodeBody();
	delete (Variable*) m_ack;
	m_ack = VariableId::noId();
  }

  void AssignmentNode::specializedPostInit(const PlexilNodeId& node)
  {
	debugMsg("Node:postInit",
			 "Creating assignment for node '" << m_nodeId.toString() << "'");
	// XML parser should have checked for this
	checkError(Id<PlexilAssignmentBody>::convertable(node->body()),
			   "Node is an assignment node but doesn't have an assignment body.");
	createAssignment((PlexilAssignmentBody*) node->body());
  }

  void AssignmentNode::createSpecializedConditions()
  {
	// Construct real end condition
	m_conditions[endIdx]->removeListener(m_listeners[endIdx]);
	ExpressionId realEndCondition =
	  (new Conjunction(m_ack,
					   false, 
					   m_conditions[endIdx],
					   m_garbageConditions[endIdx]))->getId();
	realEndCondition->addListener(m_listeners[endIdx]);
	m_conditions[endIdx] = realEndCondition;
	m_garbageConditions[endIdx] = true;
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
										  m_connector);
      // *** beef this up later ***
	  PlexilArrayElement* arrayElement = (PlexilArrayElement*) destExpr;
	  debugMsg("ArrayElement:ArrayElement", " name = " << arrayElement->getArrayName() << ". To: " << dest->toString());
	  int e_index = dest->toString().find(": ", dest->toString().length()-15);
	  int b_index = dest->toString().find("u]", dest->toString().length()-40) + 2;
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
										m_connector,
										deleteRhs);
    m_assignment =
      (new Assignment(dest, rhs, m_ack, destName, deleteLhs, deleteRhs))->getId();
  }

  // Unit test variant of above
  void AssignmentNode::createDummyAssignment() 
  {
    VariableId dest = (new BooleanVariable(BooleanVariable::FALSE_VALUE()))->getId();
	LabelStr destName("dummy");
    m_assignment =
      (new Assignment(dest, BooleanVariable::TRUE_EXP(), m_ack, destName, true, false))->getId();
  }

  const VariableId& AssignmentNode::getAssignmentVariable() const
  {
	return m_assignment->getDest();
  }

  //
  // Next-state logic
  //

  NodeState AssignmentNode::getDestStateFromExecuting()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isEndConditionActive(),
			   "End for " << getNodeId().toString() << " is inactive.");

	debugMsg("Node:getDestState",
			 "VarBinding:EXECUTING" << std::endl <<
			 getAncestorInvariantCondition()->toString() << std::endl <<
			 getInvariantCondition()->toString() << std::endl <<
			 getEndCondition()->toString());

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: FINISHED. Ancestor invariant false.");
	  return FINISHED_STATE;
	}
	else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  Invariant false.");
	  return ITERATION_ENDED_STATE;
	}
	else if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: ITERATION_ENDED.  End condition true.");
	  return ITERATION_ENDED_STATE;
	}
	return NO_NODE_STATE;
  }

  //
  // Transition handlers
  //

  void AssignmentNode::transitionFromExecuting(NodeState destState)
  {
	checkError(destState == ITERATION_ENDED_STATE ||
			   destState == FINISHED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	bool aborting = false;
	if (getAncestorInvariantCondition()->getValue() ==
		BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
		aborting = true;
      }
	else if (getInvariantCondition()->getValue() ==
			 BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
		aborting = true;
      }
	else if (getPostCondition()->getValue() ==
			 BooleanVariable::TRUE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
	}
	else {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	  aborting = true;
	}

	if (aborting)
	  abort();

	deactivateAncestorInvariantCondition();
	deactivateInvariantCondition();
	deactivateEndCondition();
	deactivatePostCondition();
	deactivateExecutable();
  }

  void AssignmentNode::transitionToExecuting()
  {
	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();
	activatePostCondition();

	setState(EXECUTING_STATE);
	execute();
  }

  void AssignmentNode::specializedHandleExecution()
  {
	checkError(m_assignment.isValid(),
			   "Node::handleExecution: Assignment is invalid");
	m_assignment->activate();
	m_assignment->fixValue();
	m_exec->enqueueAssignment(m_assignment);
  }

  void AssignmentNode::abort()
  {
    debugMsg("Node:abort", "Aborting node " << m_nodeId.toString());
	if (m_assignment.isValid())
      m_assignment->getDest()->setValue(Expression::UNKNOWN());
	else 
	  debugMsg("Warning", "Invalid assignment id in " << m_nodeId.toString());
  }

  void AssignmentNode::specializedReset()
  {
	m_ack->reset();
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
