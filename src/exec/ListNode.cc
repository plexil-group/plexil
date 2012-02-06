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

#include "ListNode.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecConnector.hh"
#include "NodeFactory.hh"

#include <algorithm> // for find_if

namespace PLEXIL
{

  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  ListNode::ListNode(const PlexilNodeId& node, 
					 const ExecConnectorId& exec, 
					 const NodeId& parent)
	: Node(node, exec, parent)
  {
	checkError(node->nodeType() == NodeType_NodeList || node->nodeType() == NodeType_LibraryNodeCall,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(node->nodeType())
			   << "\" for a ListNode");

	// Initialize parent-state conditions for childrens' use
	m_conditions[parentExecutingIdx] = 
	  (new Equality(m_stateVariable, StateVariable::EXECUTING_EXP()))->getId();
	m_conditions[parentFinishedIdx] =
	  (new Equality(m_stateVariable, StateVariable::FINISHED_EXP()))->getId();
	m_conditions[parentWaitingIdx] = 
	  (new Equality(m_stateVariable, StateVariable::WAITING_EXP()))->getId();
	m_garbageConditions[parentExecutingIdx] = true;
	m_garbageConditions[parentFinishedIdx] = true;
	m_garbageConditions[parentWaitingIdx] = true;

	// Instantiate child nodes, if any
	if (node->nodeType() == NodeType_NodeList) {
	  debugMsg("Node:node", "Creating child nodes.");
	  // XML parser should have checked for this
	  checkError(Id<PlexilListBody>::convertable(node->body()),
				 "Node " << m_nodeId.toString() << " is a list node but doesn't have a " <<
				 "list body.");
	  createChildNodes((PlexilListBody*) node->body()); // constructs default end condition
	}
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  ListNode::ListNode(const LabelStr& type, const LabelStr& name, const NodeState state,
					 const bool skip, const bool start, const bool pre,
					 const bool invariant, const bool post, const bool end, const bool repeat,
					 const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
					 const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
					 const bool parentFinished, const bool cmdHdlRcvdCondition,
					 const ExecConnectorId& exec,
					 const NodeId& parent)
	: Node(type, name, state, 
		   skip, start, pre, invariant, post, end, repeat,
		   ancestorInvariant, ancestorEnd, parentExecuting, childrenFinished,
		   commandAbort, parentWaiting, parentFinished, cmdHdlRcvdCondition,
		   exec, parent)
  {
	checkError(type == LIST() || type == LIBRARYNODECALL(),
			   "Invalid node type \"" << type.toString() << "\" for a ListNode");

	// Activate parent-state conditions as required
	m_conditions[parentExecutingIdx]->activate();
	m_conditions[parentFinishedIdx]->activate();
	m_conditions[parentWaitingIdx]->activate();

	if (state == EXECUTING_STATE || state == FINISHING_STATE || state == FAILING_STATE) {
	  m_conditions[ancestorEndIdx]->activate();
	  m_conditions[ancestorInvariantIdx]->activate();
	}
  }

  void ListNode::createChildNodes(const PlexilListBody* body) 
  {
	try {
	  for (std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
		   it != body->children().end(); 
		   ++it)
		m_children.push_back(NodeFactory::createNode(*it, m_exec, m_id));
	}
	catch (const Error& e) {
	  debugMsg("Node:node", " Error creating child nodes: " << e);
	  // Clean up 
	  while (!m_children.empty()) {
		delete (Node*) m_children.back();
		m_children.pop_back();
	  }
	  // Rethrow so that outer error handler can deal with this as well
	  throw;
	}
  }

  // N.B. The end condition constructed below can be overridden by the user
  void ListNode::createSpecializedConditions()
  {
    ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
    cond->addListener(makeConditionListener(childrenWaitingOrFinishedIdx));
    m_conditions[childrenWaitingOrFinishedIdx] = cond;
    m_garbageConditions[childrenWaitingOrFinishedIdx] = true;

    ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
    endCond->addListener(makeConditionListener(endIdx));
    m_conditions[endIdx] = endCond;
    m_garbageConditions[endIdx] = true;
  }

  // Create the ancestor end and ancestor invariant conditions required by children
  void ListNode::createConditionWrappers()
  {
	if (m_parent) {
	  m_conditions[ancestorEndIdx] =
		(new Disjunction(getAncestorEndCondition(), // from parent
						 false,
						 getEndCondition(),
						 false))->getId();
	  m_conditions[ancestorInvariantIdx] =
		(new Conjunction(getAncestorInvariantCondition(), // from parent
						 false,
						 getInvariantCondition(),
						 false))->getId();
	  m_garbageConditions[ancestorEndIdx] = true;
	  m_garbageConditions[ancestorInvariantIdx] = true;
	}
	else {
	  // Simply reuse existing conditions
	  m_conditions[ancestorEndIdx] = m_conditions[endIdx];
	  m_conditions[ancestorInvariantIdx] = m_conditions[invariantIdx];
	}
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  ListNode::~ListNode()
  {
	debugMsg("ListNode:~ListNode", " destructor for " << m_nodeId.toString());

	cleanUpConditions();
	cleanUpNodeBody();
	// cleanUpVars(); // base destructor can handle this
  }

  void ListNode::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

	debugMsg("ListNode:cleanUpConditions", " for " << m_nodeId.toString());

	cleanUpChildConditions();

	// Defer to base class
	Node::cleanUpConditions();
  }

  void ListNode::cleanUpNodeBody()
  {
	debugMsg("ListNode:cleanUpNodeBody", " for " << m_nodeId.toString());
	// Delete children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }
	m_children.clear();
  }

  void ListNode::cleanUpChildConditions()
  {
	debugMsg("ListNode:cleanUpChildConditions", " for " << m_nodeId.toString());
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();
  }

  class NodeIdEq {
  public:
    NodeIdEq(const double name) : m_name(name) {}
    bool operator()(const NodeId& node) {return node->getNodeId() == m_name;}
  private:
    double m_name;
  };

  NodeId ListNode::findChild(const LabelStr& childName) const
  {
	std::vector<NodeId>::const_iterator it =
	  std::find_if(m_children.begin(), m_children.end(), NodeIdEq(childName));
	if (it == m_children.end())
	  return NodeId::noId();
	return *it;
  }

  void ListNode::specializedPostInit(const PlexilNodeId& node)
  {
    //call postInit on all children
	const PlexilListBody* body = (const PlexilListBody*) node->body();
	check_error(body != NULL);
	std::vector<NodeId>::iterator it = m_children.begin();
	std::vector<PlexilNodeId>::const_iterator pit = body->children().begin();	
	while (it != m_children.end() && pit != body->children().end()) {
	  (*it++)->postInit(*pit++);
	}
	checkError(it == m_children.end() && pit == body->children().end(),
			   "Node:postInit: mismatch between PlexilNode and list node children");
  }

  //
  // Next-state logic
  //

  NodeState ListNode::getDestStateFromExecuting()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  List node and ANCESTOR_INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}

	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  List node and INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}

	checkError(isEndConditionActive(),
			   "End for " << getNodeId().toString() << " is inactive.");
	if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FINISHING.  List node and END_CONDITION true.");
	  return FINISHING_STATE;
	}
	debugMsg("Node:getDestState",
			 " '" << m_nodeId.toString() << "' destination: no state.");
	return NO_NODE_STATE;
  }

  NodeState ListNode::getDestStateFromFailing()
  {
	checkError(isChildrenWaitingOrFinishedConditionActive(),
			   "Children waiting or finished for " << getNodeId().toString() <<
			   " is inactive.");

	if (getChildrenWaitingOrFinishedCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_FAILED()) {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() << "' destination: FINISHED.  List node and ALL_CHILDREN_WAITING_OR_FINISHED" <<
				 " true and parent failed.");
		return FINISHED_STATE;
	  }
	  else {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED.  List node and self-failure.");
		return ITERATION_ENDED_STATE;
	  }
	}
	debugMsg("Node:getDestState",
			 " '" << m_nodeId.toString() << "' destination: no state.");
	return NO_NODE_STATE;
  }

  NodeState ListNode::getDestStateFromFinishing()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  List node and ANCESTOR_INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}

	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  List node and INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}

	checkError(isChildrenWaitingOrFinishedConditionActive(),
			   "Children waiting or finished for " << getNodeId().toString() <<
			   " is inactive.");
	if (getChildrenWaitingOrFinishedCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (!getPostCondition()->isActive())
		getPostCondition()->activate();

	  if (getPostCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		  debugMsg("Node:getDestState",
				   " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED.  List node, " <<
				   "ALL_CHILDREN_WAITING_OR_FINISHED true and POST_CONDITION true.");
	  }
	  else {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() << "' destination: FINISHED.  List node, ALL_CHILDREN_WAITING_OR_FINISHED " <<
				 "true and POST_CONDITION false or unknown.");
	  }

	  return ITERATION_ENDED_STATE;
	}
	debugMsg("Node:getDestState",
			 " '" << m_nodeId.toString() << "' destination: no state. ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
	return NO_NODE_STATE;
  }

  //
  // Transition handlers
  //

  void ListNode::transitionFromExecuting(NodeState destState)
  {
	checkError(destState == FINISHING_STATE ||
			   destState == FAILING_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	if (getAncestorInvariantCondition()->getValue() ==
		BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }
	else if (getInvariantCondition()->getValue() ==
			 BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
      }
      
	deactivateAncestorInvariantCondition();
	if (destState != FINISHING_STATE)
	  deactivateInvariantCondition();
	deactivateEndCondition();
	// Any variables declared in this node also need to be deactivated.
	deactivateExecutable();
  }

  void ListNode::transitionFromFailing(NodeState destState)
  {
	checkError(destState == ITERATION_ENDED_STATE ||
			   destState == FINISHED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	deactivateChildrenWaitingOrFinishedCondition();

	m_conditions[ancestorEndIdx]->deactivate();
	m_conditions[ancestorInvariantIdx]->deactivate();
  }

  void ListNode::transitionFromFinishing(NodeState destState)
  {
	checkError(destState == ITERATION_ENDED_STATE ||
			   destState == FAILING_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	if (getAncestorInvariantCondition()->getValue() ==
		BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }
	else if (getInvariantCondition()->getValue() ==
			 BooleanVariable::FALSE_VALUE()) {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
      }
	else if (getPostCondition()->getValue() ==
			BooleanVariable::TRUE_VALUE())
	  getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
	else {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	}

	deactivateAncestorInvariantCondition();
	deactivateInvariantCondition();
	deactivateChildrenWaitingOrFinishedCondition();
	deactivatePostCondition();

	if (destState == ITERATION_ENDED_STATE) {
	  m_conditions[ancestorEndIdx]->deactivate();
	  m_conditions[ancestorInvariantIdx]->deactivate();
	}
  }

  void ListNode::transitionToExecuting()
  {
	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();

	m_conditions[ancestorEndIdx]->activate();
	m_conditions[ancestorInvariantIdx]->activate();

	setState(EXECUTING_STATE);
	execute();
  }

  void ListNode::transitionToFailing()
  {
	activateChildrenWaitingOrFinishedCondition();
  }

  void ListNode::transitionToFinishing()
  {
	activateAncestorInvariantCondition();
	activateChildrenWaitingOrFinishedCondition();
	activatePostCondition();
  }

  void ListNode::specializedActivate()
  {
	// Activate parent-state conditions
	m_conditions[parentExecutingIdx]->activate();
	m_conditions[parentFinishedIdx]->activate();
	m_conditions[parentWaitingIdx]->activate();

    // Activate all children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->activate();
  }

}
