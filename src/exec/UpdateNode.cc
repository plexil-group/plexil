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
	: Node(node, exec, parent),
	  m_ack((new BooleanVariable(BooleanVariable::UNKNOWN()))->getId())
  {
	checkError(node->nodeType() == NodeType_Update,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(node->nodeType())
			   << "\" for an UpdateNode");
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  UpdateNode::UpdateNode(const LabelStr& type, const LabelStr& name, const NodeState state,
						 const bool skip, const bool start, const bool pre,
						 const bool invariant, const bool post, const bool end, const bool repeat,
						 const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
						 const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
						 const bool parentFinished, const bool cmdHdlRcvdCondition,
						 const ExecConnectorId& exec)
	: Node(type, name, state, 
		   skip, start, pre, invariant, post, end, repeat,
		   ancestorInvariant, ancestorEnd, parentExecuting, childrenFinished,
		   commandAbort, parentWaiting, parentFinished, cmdHdlRcvdCondition,
		   exec),
	  m_ack((new BooleanVariable(BooleanVariable::UNKNOWN()))->getId())
  {
	checkError(type == UPDATE(),
			   "Invalid node type \"" << type.toString() << "\" for an UpdateNode");

	// Construct stuff as required for unit test
	createDummyUpdate();
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  UpdateNode::~UpdateNode()
  {
	// Redundant with base class destructor
	cleanUpConditions();
	cleanUpNodeBody();
	delete (Variable*) m_ack;
	m_ack = VariableId::noId();
  }

  void UpdateNode::cleanUpNodeBody()
  {
    if(m_update.isId()) {
      debugMsg("Node:cleanUpConds", "<" << m_nodeId.toString() << "> Removing update.");
      delete (Update*) m_update;
	  m_update = UpdateId::noId();
    }
  }

  // Specific behaviors for derived classes
  void UpdateNode::specializedPostInit()
  {
	debugMsg("Node:postInit", "Creating update for node '" << m_nodeId.toString() << "'");
	// XML parser should have checked for this
	checkError(Id<PlexilUpdateBody>::convertable(m_node->body()),
			   "Node is an update node but doesn't have an update body.");
	createUpdate((PlexilUpdateBody*)m_node->body());
  }

  void UpdateNode::createUpdate(const PlexilUpdateBody* body) 
  {
    PlexilUpdateId update = body->update();
    ExpressionMap updatePairs;
    std::list<ExpressionId> garbage;

    if (update.isValid()) {
      for (std::vector<std::pair<std::string, PlexilExprId> >::const_iterator it =
			 update->pairs().begin();
		   it != update->pairs().end();
		   ++it) {
		LabelStr nameStr(it->first);
		debugMsg("Node:createUpdate", "Adding pair '" << nameStr.toString());
		PlexilExprId foo = it->second;
		bool wasCreated = false;
		ExpressionId valueExpr = 
		  ExpressionFactory::createInstance(foo->name(),
											foo,
											m_connector,
											wasCreated);
		check_error(valueExpr.isValid());
		if (wasCreated)
		  garbage.push_back(valueExpr);
		updatePairs.insert(std::make_pair((double) nameStr, valueExpr));
      }
    }

    m_update = (new Update(m_id, updatePairs, m_ack, garbage))->getId();
  }

  // Unit test variant
  void UpdateNode::createDummyUpdate() 
  {
    ExpressionMap updatePairs;
    std::list<ExpressionId> garbage;
    m_update = (new Update(m_id, updatePairs, m_ack, garbage))->getId();
  }

  void UpdateNode::createSpecializedConditions()
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

  //
  // Next-state logic
  //

  NodeState UpdateNode::getDestStateFromExecuting()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isEndConditionActive(),
			   "End for " << getNodeId().toString() << " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
		if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
            debugMsg("Node:getDestState",
                     "Destination: FINISHED.  Ancestor invariant condition false and end " <<
                     "condition true.. ");
            return FINISHED_STATE;
		  }
		else {
		  debugMsg("Node:getDestState",
				   "Destination: FAILING.  Ancestor invariant condition false and end " <<
				   "condition false or unknown.");
		  return FAILING_STATE;
		}
      }
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
		if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		  debugMsg("Node:getDestState",
				   "Destination: ITERATION_ENDED.  Invariant condition false and end " <<
				   "condition true.. ");
		  return ITERATION_ENDED_STATE;
		}
		else {
            debugMsg("Node:getDestState",
                     "Destination: FAILING.  Invariant condition false and end condition " <<
                     "false or unknown.");
            return FAILING_STATE;
		  }
      }

	if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		return ITERATION_ENDED_STATE;
	}
      
	debugMsg("Node:getDestState",
			 "Destination from EXECUTING: no state.\n  Ancestor invariant: " 
			 << getAncestorInvariantCondition()->toString() 
			 << "\n  Invariant: " << getInvariantCondition()->toString() 
			 << "\n  End: " << getEndCondition()->toString());
	return NO_NODE_STATE;
  }

  NodeState UpdateNode::getDestStateFromFailing()
  {
	checkError(isAbortCompleteConditionActive(),
			   "Abort complete for " << getNodeId().toString() << " is inactive.");

	if (getAbortCompleteCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (findVariable(Node::FAILURE_TYPE())->getValue() ==
		  FailureVariable::PARENT_FAILED()) {
		debugMsg("Node:getDestState",
				 "Destination: FINISHED.  Command node abort complete, " <<
				 "and parent failed.");
		return FINISHED_STATE;
	  }
	  else {
		debugMsg("Node:getDestState",
				 "Destination: ITERATION_ENDED.  Command node abort complete.");
		return ITERATION_ENDED_STATE;
	  }
	}

	debugMsg("Node:getDestState", "Destination: no state.");
	return NO_NODE_STATE;
  }

  //
  // Transition handlers
  //

  void UpdateNode::transitionFromExecuting(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == FAILING_STATE ||
			   destState == ITERATION_ENDED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
	  if (getEndCondition()->getValue() != BooleanVariable::TRUE_VALUE())
		abort();
	}
	else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
	  if (getEndCondition()->getValue() != BooleanVariable::TRUE_VALUE())
		abort();
	}
	else if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (getPostCondition()->getValue() != BooleanVariable::TRUE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	  }
	  else
		getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
	}
	else {
	  checkError(ALWAYS_FAIL, "Should never get here.");
	}

	deactivateEndCondition();
	deactivateInvariantCondition();
	deactivateAncestorInvariantCondition();
	deactivatePostCondition();
	deactivateExecutable();
  }

  void UpdateNode::transitionFromFailing(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == ITERATION_ENDED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	deactivateAbortCompleteCondition();
  }

  void UpdateNode::transitionToExecuting()
  {
	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();
	activatePostCondition();

	setState(EXECUTING_STATE);
	execute();
  }

  void UpdateNode::transitionToFailing()
  {
	activateAbortCompleteCondition();
  }


  void UpdateNode::specializedHandleExecution()
  {
	checkError(m_update.isValid(),
			   "Node::handleExecution: Update is invalid");
	m_update->activate();
	m_update->fixValues();
	m_exec->enqueueUpdate(m_update);
  }

  void UpdateNode::specializedDeactivateExecutable()
  {
    if (m_update.isValid())
      m_update->deactivate();
  }

  void UpdateNode::specializedReset()
  {
	m_ack->reset();
  }

}
