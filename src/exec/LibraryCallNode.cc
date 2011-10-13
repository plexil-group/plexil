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

#include "LibraryCallNode.hh"
#include "Array.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "NodeFactory.hh"
#include "Variables.hh"

namespace PLEXIL
{

  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  LibraryCallNode::LibraryCallNode(const PlexilNodeId& nodeProto, 
								   const ExecConnectorId& exec, 
								   const NodeId& parent)
	: Node(nodeProto, exec, parent),
	  m_executingExpression((new Equality(m_stateVariable,
										  StateVariable::EXECUTING_EXP()))->getId()),
	  m_finishedExpression((new Equality(m_stateVariable,
										 StateVariable::FINISHED_EXP()))->getId()),
	  m_waitingExpression((new Equality(m_stateVariable,
										StateVariable::WAITING_EXP()))->getId())
  {
	checkError(nodeProto->nodeType() == NodeType_LibraryNodeCall,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
			   << "\" for a LibraryCallNode");

	// Create library call node
	debugMsg("Node:node", "Creating library node call.");
	// XML parser should have checked for this
	const PlexilLibNodeCallBody* body = nodeProto->body();
	checkError(body != NULL,
			   "Node " << m_nodeId.toString() << " is a library node call but doesn't have a " <<
			   "library node call body.");
	createLibraryNode(body); // constructs default end condition
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  LibraryCallNode::LibraryCallNode(const LabelStr& type, const LabelStr& name, const NodeState state,
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
	  m_executingExpression((new Equality(m_stateVariable,
										  StateVariable::EXECUTING_EXP()))->getId()),
	  m_finishedExpression((new Equality(m_stateVariable,
										 StateVariable::FINISHED_EXP()))->getId()),
	  m_waitingExpression((new Equality(m_stateVariable,
										StateVariable::EXECUTING_EXP()))->getId())
  {
	checkError(type == LIBRARYNODECALL(),
			   "Invalid node type \"" << type.toString() << "\" for a LibraryCallNode");

	// Prop up stupid unit test
	if (state == EXECUTING_STATE || state == FINISHING_STATE || state == FAILING_STATE) {
	  m_ancestorEndExpression->activate();
	  m_ancestorInvariantExpression->activate();
	}
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  LibraryCallNode::~LibraryCallNode()
  {
	debugMsg("LibraryCallNode:~LibraryCallNode", " destructor for " << m_nodeId.toString());

	cleanUpConditions();
	cleanUpNodeBody();
	cleanUpVars(); // flush alias vars
  }

  void LibraryCallNode::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

	debugMsg("ListNode:cleanUpConditions", " for " << m_nodeId.toString());

	cleanUpChildConditions();

	// These reference invariant/end conditions and must be cleaned up before them
	delete (Expression*) m_ancestorEndExpression;
	delete (Expression*) m_ancestorInvariantExpression;

	// Clean up shared state expressions now that children no longer reference them
	delete (Expression*) m_executingExpression;
	delete (Expression*) m_finishedExpression;
	delete (Expression*) m_waitingExpression;

    // Clean up condition listeners
    for (unsigned int i = 0; i < conditionIndexMax; i++) {
      if (m_listeners[i].isId()) {
		debugMsg("Node:cleanUpConds",
				 "<" << m_nodeId.toString() << "> Removing condition listener for " <<
				 getConditionName(i).toString());
		m_conditions[i]->removeListener(m_listeners[i]);
		delete (ExpressionListener*) m_listeners[i];
		m_listeners[i] = ExpressionListenerId::noId();
      }
    }

    // Clean up conditions
    for (unsigned int i = 0; i < conditionIndexMax; i++) {
      if (m_garbageConditions[i]) {
		debugMsg("Node:cleanUpConds",
				 "<" << m_nodeId.toString() << "> Removing condition " << getConditionName(i).toString());
		delete (Expression*) m_conditions[i];
		m_conditions[i] = ExpressionId::noId();
	  }
	}

    m_cleanedConditions = true;
  }

  void LibraryCallNode::cleanUpChildConditions()
  {
	debugMsg("LibraryCallNode:cleanUpChildConditions", " for " << m_nodeId.toString());
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();
  }

  void LibraryCallNode::cleanUpNodeBody()
  {
	debugMsg("LibraryCallNode:cleanUpNodeBody", " for " << m_nodeId.toString());
	// Delete child
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }
	m_children.clear();
  }

  void LibraryCallNode::createLibraryNode(const PlexilLibNodeCallBody* body)
  {
	// get node body
	checkError(body != NULL,
			   "Node " << m_nodeId.toString() << ": createLibraryNode: Node has no library node call body");
      
	// get the lib node and its interface
	const PlexilNodeId& libNode = body->libNode();
	const PlexilInterfaceId& libInterface = libNode->interface();
      
	// if there is no interface, there must be no variables
	if (libInterface.isNoId()) {
	  checkError(body->aliases().size() == 0,
				 "Variable aliases in '" << m_nodeId.toString() <<
				 "' do not match interface in '" << 
				 libNode->nodeId() << "'");
	}
      
	// otherwise check variables in interface
	else {
	  // make a copy of the alias map
	  PlexilAliasMap aliasesCopy(body->aliases());

	  // Assign aliases for the In interface variables
	  createAliases(libNode, libInterface->in(), aliasesCopy, true);

	  // Assign aliases for the InOut interface variables
	  createAliases(libNode, libInterface->inOut(), aliasesCopy, false);

	  // Barf if formal parameter is not known
	  assertTrueMsg(aliasesCopy.size() == 0,
					"Interface variable \"" << LabelStr(aliasesCopy.begin()->first).toString() 
					<< "\" not found in library node \"" << libNode->nodeId()
					<< "\", called from node '" << getNodeId().toString() << "'");
	}

	// Construct the child
	m_children.push_back(NodeFactory::createNode(body->libNode(), m_exec, m_id));
  }

  // Check aliases against interfaceVars.
  // Remove all that are found from aliases.
  // If a variable exists in interfaceVars but not aliases:
  //  - and it has a default value, generate the variable with the default value;
  //  - and it doesn't have a default value, signal an error.
  // libNode is only used for error message generation.

  void LibraryCallNode::createAliases(const PlexilNodeId& libNode, 
									  const std::vector<PlexilVarRef*>& interfaceVars,
									  PlexilAliasMap& aliases,
									  bool isIn)
  {
	// check each variable in the interface to ensure it is
	// referenced in the alias list
	for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
		 var != interfaceVars.end();
		 ++var) {
	  // get var label and matching value in alias list
	  LabelStr varLabel((*var)->name());
	  PlexilExprId& aliasValue = aliases[varLabel];

	  // check that the expression is consistent with the interface variable
	  if (aliasValue.isId()) {
		VariableId actualVar;
		if (Id<PlexilVarRef>::convertable(aliasValue)) {
		  actualVar = Node::findVariable((const PlexilVarRef*) aliasValue);
		  assertTrueMsg(actualVar.isId(),
						"Can't find variable named \"" << aliasValue->name()
						<< "\" for " << (isIn ? "In" : "InOut" )
						<< "alias variable \"" << (*var)->name());

		  if (isIn) {
			// Construct const wrapper
			if (actualVar->isArray()) {
			  actualVar = 
				(new ArrayAliasVariable((*var)->name(),
										NodeConnector::getId(),
										(ExpressionId) actualVar,
										false,
										isIn))->getId();
			}
			else {
			  actualVar = 
				(new AliasVariable((*var)->name(),
								   NodeConnector::getId(),
								   (ExpressionId) actualVar,
								   false,
								   isIn))->getId();
			}
			debugMsg("LibraryCallNode:createAliases",
					 " Node \"" << m_nodeId.toString()
					 << "\": Constructed const alias wrapper for \"" << (*var)->name()
					 << "\" to variable " << *actualVar);
			m_localVariables.push_back(actualVar);
		  }
		  else {
			debugMsg("LibraryCallNode:createAliases",
					 " Node \"" << m_nodeId.toString()
					 << "\": Aliasing \"" << (*var)->name()
					 << "\" to variable " << *actualVar);
		  }
		}
		else if (Id<PlexilArrayElement>::convertable(aliasValue)) {
		  // Expression is an array reference
		  // Construct the expression
		  bool wasCreated = false;
		  ExpressionId expr =
			ExpressionFactory::createInstance(aliasValue->name(), aliasValue, NodeConnector::getId(), wasCreated);

		  // Construct a wrapper for it
		  actualVar = (new AliasVariable((*var)->name(),
										 NodeConnector::getId(),
										 expr,
										 wasCreated,
										 isIn))->getId();
		  debugMsg("LibraryCallNode:createAliases",
				   " Node \"" << m_nodeId.toString()
				   << "\": Constructed alias wrapper for \"" << (*var)->name()
					 << "\" to array element " << *expr);
		  m_localVariables.push_back(actualVar);
		}
		else {
		  // Expression is not a variable or array reference
		  // Can't do this for InOut
		  assertTrueMsg(isIn,
						"Alias value for InOut interface variable \""
						<< (*var)->name()
						<< "\" is not a variable or array reference");

		  // Construct the expression
		  bool wasCreated = false;
		  ExpressionId expr =
			ExpressionFactory::createInstance(aliasValue->name(), aliasValue, NodeConnector::getId(), wasCreated);

		  // Construct a const wrapper for it
		  actualVar = 
			(new AliasVariable((*var)->name(), NodeConnector::getId(), expr, wasCreated, isIn))->getId();
		  debugMsg("LibraryCallNode:createAliases",
				   " Node \"" << m_nodeId.toString()
				   << "\": Constructed alias wrapper for \"" << (*var)->name()
				   << "\" to expression " << *expr);
		  m_localVariables.push_back(actualVar);
		}

		// Add to alias map
		m_aliasVariables[varLabel.getKey()] = actualVar;
		
		// remove value for alias copy for later checking
		aliases.erase(varLabel);
	  }
	}
  }

  const VariableId& LibraryCallNode::findVariable(const LabelStr& name, bool recursive)
  {
	if (recursive) {
	  // Check alias map only
	  if (m_aliasVariables.find(name.getKey()) != m_aliasVariables.end())
		return m_aliasVariables[name.getKey()];
	  else
		return VariableId::noId();
	}
	else {
	  return Node::findVariable(name, false);
	}
  }

  // Specific behaviors for derived classes
  void LibraryCallNode::specializedPostInit(const PlexilNodeId& node)
  {
	// Get node body
	const PlexilLibNodeCallBody* body = (PlexilLibNodeCallBody*) node->body();
	check_error(body != NULL);
	// get the lib node 
	const PlexilNodeId& libNode = body->libNode();
    //call postInit on the child
	m_children.front()->postInit(body->libNode());
  }

  void LibraryCallNode::createSpecializedConditions()
  {
	// Construct conditions
	ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
	ExpressionListenerId listener = m_listeners[childrenWaitingOrFinishedIdx] = 
	  (new ConditionChangeListener((Node&) *this, CHILDREN_WAITING_OR_FINISHED()))->getId();
	cond->addListener(listener);
	m_conditions[childrenWaitingOrFinishedIdx] = cond;
	m_garbageConditions[childrenWaitingOrFinishedIdx] = true;

	ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
	listener = m_listeners[endIdx];
	endCond->addListener(listener);
	m_conditions[endIdx] = endCond;
	m_garbageConditions[endIdx] = true;
  }

  // Create the ancestor end and ancestor invariant conditions shared between children
  void LibraryCallNode::createConditionWrappers()
  {
	// TODO: Micro-optimization for root node possible
	// Would require 'created' flag for each of these expressions
	m_ancestorInvariantExpression =
		(new Conjunction(getAncestorInvariantCondition(),
						 false,
						 getInvariantCondition(),
						 false))->getId();
	m_ancestorEndExpression =
		(new Disjunction(getAncestorEndCondition(),
						 false,
						 getEndCondition(),
						 false))->getId();
  }

  //
  // Next-state logic
  //

  NodeState LibraryCallNode::getDestStateFromExecuting()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isEndConditionActive(),
			   "End for " << getNodeId().toString() << " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  Library node call and ANCESTOR_INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FAILING.  Library node call and INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}
	if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' destination: FINISHING.  Library node call and END_CONDITION true.");
	  return FINISHING_STATE;
	}
	debugMsg("Node:getDestState", "Destination: no state.");
	return NO_NODE_STATE;
  }

  NodeState LibraryCallNode::getDestStateFromFailing()
  {
	checkError(isChildrenWaitingOrFinishedConditionActive(),
			   "Children waiting or finished for " << getNodeId().toString() <<
			   " is inactive.");

	if (getChildrenWaitingOrFinishedCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (getFailureTypeVariable()->getValue() == FailureVariable::PARENT_FAILED()) {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() <<
				 "' destination: FINISHED.  Library node call and ALL_CHILDREN_WAITING_OR_FINISHED" <<
				 " true and parent failed.");
		return FINISHED_STATE;
	  }
	  else {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() <<
				 "' destination: ITERATION_ENDED.  Library node call and self-failure.");
		return ITERATION_ENDED_STATE;
	  }
	}
	debugMsg("Node:getDestState",
			 " '" << m_nodeId.toString() << "' destination: no state.");
	return NO_NODE_STATE;
  }

  NodeState LibraryCallNode::getDestStateFromFinishing()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << getNodeId().toString() << " is inactive.");
	checkError(isChildrenWaitingOrFinishedConditionActive(),
			   "Children waiting or finished for " << getNodeId().toString() <<
			   " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' Destination: FAILING.  Library node call and ANCESTOR_INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}
	if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << "' Destination: FAILING.  Library node call and INVARIANT_CONDITION false.");
	  return FAILING_STATE;
	}
	if (getChildrenWaitingOrFinishedCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (!getPostCondition()->isActive())
		getPostCondition()->activate();

	  if (BooleanVariable::falseOrUnknown(getPostCondition()->getValue())) {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() << 
				 "' destination: FINISHED.  Library node call, ALL_CHILDREN_WAITING_OR_FINISHED " <<
				 "true and POST_CONDITION false or unknown.");
		return ITERATION_ENDED_STATE;
	  }
	  else {
		debugMsg("Node:getDestState",
				 " '" << m_nodeId.toString() << 
				 "' destination: ITERATION_ENDED.  Library node call and " <<
				 "ALL_CHILDREN_WAITING_OR_FINISHED and POST_CONDITION true.");
		return ITERATION_ENDED_STATE;
	  }
	}
	debugMsg("Node:getDestState",
			   " '" << m_nodeId.toString() << 
			 "' destination: no state. Library node call and ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
	return NO_NODE_STATE;
  }


  //
  // Transition handlers
  //

  void LibraryCallNode::transitionFromExecuting(NodeState destState)
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
	deactivateExecutable();
  }

  void LibraryCallNode::transitionFromFailing(NodeState destState)
  {
	checkError(destState == ITERATION_ENDED_STATE ||
			   destState == FINISHED_STATE,
			   "Attempting to transition to invalid state '"
		       << StateVariable::nodeStateName(destState).toString() << "'");

	deactivateChildrenWaitingOrFinishedCondition();

	m_ancestorEndExpression->deactivate();
	m_ancestorInvariantExpression->deactivate();
  }

  void LibraryCallNode::transitionFromFinishing(NodeState destState)
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
	  m_ancestorEndExpression->deactivate();
	  m_ancestorInvariantExpression->deactivate();
	}
  }

  void LibraryCallNode::transitionToExecuting()
  {
	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();

	m_ancestorEndExpression->activate();
	m_ancestorInvariantExpression->activate();

	setState(EXECUTING_STATE);
	execute();
  }

  void LibraryCallNode::transitionToFinishing()
  {
	activateAncestorInvariantCondition();
	activateChildrenWaitingOrFinishedCondition();
	activatePostCondition();
  }

  void LibraryCallNode::transitionToFailing()
  {
	activateChildrenWaitingOrFinishedCondition();
  }

  void LibraryCallNode::specializedActivate()
  {
	// Activate shared state expressions
	m_executingExpression->activate();
	m_finishedExpression->activate();
	m_waitingExpression->activate();

    // Activate all children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->activate();
  }

}
