/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "Node.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "Expressions.hh"
#include "ExpressionFactory.hh"
#include "ExternalInterface.hh"
#include "NodeFactory.hh"
#include "PlexilExec.hh"
#include "Variables.hh"
#include "XMLUtils.hh"

#include <algorithm> // for find_if
#include <vector>
#include <sstream>
#include <iomanip> // for setprecision

namespace PLEXIL {
  const std::vector<double>& Node::ALL_CONDITIONS() {
    static std::vector<double>* sl_allConds = NULL;
    if (sl_allConds == NULL)
      {
		// *** N.B.: Order MUST agree with enum conditionIndex!
		sl_allConds = new std::vector<double>();
		// User specified conditions
		sl_allConds->push_back(SKIP_CONDITION());
		sl_allConds->push_back(START_CONDITION());
		sl_allConds->push_back(END_CONDITION());
		sl_allConds->push_back(INVARIANT_CONDITION());
		sl_allConds->push_back(PRE_CONDITION());
		sl_allConds->push_back(POST_CONDITION());
		sl_allConds->push_back(REPEAT_CONDITION());
		// Internal conditions for all nodes
		sl_allConds->push_back(ANCESTOR_INVARIANT_CONDITION());
		sl_allConds->push_back(ANCESTOR_END_CONDITION());
		sl_allConds->push_back(PARENT_EXECUTING_CONDITION());
		sl_allConds->push_back(PARENT_WAITING_CONDITION());
		sl_allConds->push_back(PARENT_FINISHED_CONDITION());
		// Only for list or library call nodes
		sl_allConds->push_back(CHILDREN_WAITING_OR_FINISHED());
		// Only for command nodes
		sl_allConds->push_back(ABORT_COMPLETE());
		sl_allConds->push_back(COMMAND_HANDLE_RECEIVED_CONDITION());
		// inexpensive sanity check
		assertTrue(sl_allConds->size() == conditionIndexMax,
				   "INTERNAL ERROR: Inconsistency between conditionIndex enum and ALL_CONDITIONS");
      }
    return *sl_allConds;
  }

  unsigned int Node::getConditionIndex(const LabelStr& cName) {
    double nameKey = cName.getKey();
    const std::vector<double>& allConds = ALL_CONDITIONS();
    for (size_t i = 0; i < conditionIndexMax; i++) {
      if (allConds[i] == nameKey)
		return i;
    }
    assertTrueMsg(ALWAYS_FAIL,
				  cName.toString() << " is not a valid condition name");
  }

  LabelStr Node::getConditionName(unsigned int idx)
  {
    return LabelStr(ALL_CONDITIONS()[idx]);
  }

  class ConditionChangeListener : public ExpressionListener 
  {
  public:
    ConditionChangeListener(const NodeId& node, const LabelStr& cond)
      : ExpressionListener(), m_node(node), m_cond(cond) 
	{
	}
  
	void notifyValueChanged(const ExpressionId& /* expression */) 
	{
      debugMsg("Node:conditionChange",
			   m_cond.toString() << " may have changed value in " <<
			   m_node->getNodeId().toString());
      m_node->conditionChanged();
    }

  private:
    NodeId m_node;
    const LabelStr& m_cond;
  };

  class RealNodeConnector : public NodeConnector
  {
  public:
    RealNodeConnector(const NodeId& node) 
	: NodeConnector(), m_node(node) 
    {
    }

    const VariableId& findVariable(const PlexilVarRef* ref)
    {
      return m_node->findVariable(ref);
    }

    const VariableId& findVariable(const LabelStr& name, bool recursive = false)
    {
      return m_node->findVariable(name, recursive);
    }

    const ExecConnectorId& getExec()
    {
      return m_node->getExec();
    }

    const NodeId& getNode() const
    {
      return m_node;
    }

  private:
    NodeId m_node;
  };

  const LabelStr& 
  Node::nodeTypeToLabelStr(PlexilNodeType nodeType)
  {
    switch(nodeType) {
	case NodeType_NodeList:
	  return LIST();

	case NodeType_Command:
	  return COMMAND();

	case NodeType_Assignment:
	  return ASSIGNMENT();

	case NodeType_Update:
	  return UPDATE();

	case NodeType_Empty:
	  return EMPTY();

	case NodeType_LibraryNodeCall:
	  return LIBRARYNODECALL();

	  // fall thru case
	default:
	  checkError(ALWAYS_FAIL,
				 "Invalid node type " << nodeType);
	  break;
	}
    return EMPTY_LABEL();
  }

  Node::Node(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent)
    : m_id(this),
	  m_parent(parent),
	  m_exec(exec),
      m_connector((new RealNodeConnector(m_id))->getId()),
	  m_node(node),
	  m_nodeId(node->nodeId()),
	  m_nodeType(nodeTypeToLabelStr(node->nodeType())), // Can throw exception
	  m_sortedVariableNames(new std::vector<double>()),
      m_priority(node->priority()),
      m_state(INACTIVE_STATE),
	  m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false),
	  m_cleanedConditions(false),
	  m_cleanedVars(false),
      m_transitioning(false),
	  m_checkConditionsPending(false)
  {
	debugMsg("Node:node", "Creating node \"" << node->nodeId() << "\"");

	commonInit();
	setConditionDefaults();

	// Instantiate declared variables
	createDeclaredVars(node->declarations());

	// get interface variables
	getVarsFromInterface(node->interface());
  }

  // Used only by module test
  Node::Node(const LabelStr& type, const LabelStr& name, const NodeState state,
			 const bool skip, const bool start, const bool pre, const bool invariant, const bool post,
			 const bool end, const bool repeat, const bool ancestorInvariant,
			 const bool ancestorEnd, const bool parentExecuting, const bool childrenFinished,
			 const bool commandAbort, const bool parentWaiting, 
			 const bool parentFinished, const bool cmdHdlRcvdCondition, const ExecConnectorId& exec)
    : m_id(this),
	  m_parent(NodeId::noId()),
	  m_exec(exec),
	  m_node(PlexilNodeId::noId()),
	  m_nodeId(name),
	  m_nodeType(type),
	  m_sortedVariableNames(new std::vector<double>()),
      m_state(state),
	  m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false), 
	  m_cleanedConditions(false), 
	  m_cleanedVars(false),
      m_transitioning(false), 
	  m_checkConditionsPending(false)
  {
    commonInit();
	activateInternalVariables();

	// N.B.: Must be same order as ALL_CONDITIONS() and conditionIndex enum!
    bool values[conditionIndexMax] =
	  {skip, start, end, invariant, pre, post, repeat,
	   ancestorInvariant, ancestorEnd, parentExecuting, parentWaiting, parentFinished, 
	   childrenFinished, commandAbort, cmdHdlRcvdCondition};
    for (unsigned int i = 0; i < conditionIndexMax; i++) {
      debugMsg("Node:node",
			   "Creating internal variable " << LabelStr(ALL_CONDITIONS()[i]).toString() <<
			   " with value " << values[i] << " for node " << m_nodeId.toString());
      ExpressionId expr = (new BooleanVariable((double) values[i]))->getId();
      m_conditions[i] = expr;
      expr->addListener(m_listeners[i]);
      m_garbageConditions[i] = true;
    }

	// Construct ack
	m_ack = (new BooleanVariable(BooleanVariable::UNKNOWN()))->getId();
  }

  void Node::commonInit() {
    debugMsg("Node:node", "Instantiating internal variables...");
    // Instantiate state/outcome/failure variables
	// The contortions with getKey() are an attempt to minimize LabelStr copying
    m_variablesByName[STATE().getKey()] = m_stateVariable = (new StateVariable())->getId();
    ((StateVariable*) m_stateVariable)->setNodeState(m_state);

	m_variablesByName[OUTCOME().getKey()] = m_outcomeVariable = (new OutcomeVariable())->getId();
    m_variablesByName[FAILURE_TYPE().getKey()] = m_failureTypeVariable = (new FailureVariable())->getId();

    //instantiate timepoint variables
    debugMsg("Node:node", "Instantiating timepoint variables.");
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      ExpressionId stp = (new RealVariable())->getId();
      double stpName = START_TIMEPOINT_NAMES()[s];
      m_startTimepoints[s] = m_variablesByName[stpName] = stp;

      ExpressionId etp = (new RealVariable())->getId();
      const LabelStr& etpName = END_TIMEPOINT_NAMES()[s];
      m_endTimepoints[s] = m_variablesByName[etpName] = etp;
    }

	// construct condition listeners (but not conditions)
	// and initialize m_garbageConditions
	for (size_t i = 0; i < conditionIndexMax; i++) {
	  m_listeners[i] = 
		(new ConditionChangeListener(m_id, ALL_CONDITIONS()[i]))->getId();
	  m_garbageConditions[i] = false;
	}
  }

  // Use existing Boolean constants for the condition defaults
  void Node::setConditionDefaults() {
	// These may be user-specified
	// End condition will be overridden 
    m_conditions[skipIdx] = BooleanVariable::FALSE_EXP();
    m_conditions[startIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[endIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[invariantIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[preIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[postIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[repeatIdx] = BooleanVariable::FALSE_EXP();

	// These will be overridden in any non-root node,
	// but they depend on user-specified conditions,
	// so do these in createConditions() below.
    m_conditions[ancestorInvariantIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[ancestorEndIdx] = BooleanVariable::FALSE_EXP();

	if (m_parent.isId()) {
	  // These conditions only depend on the node state variable,
	  // which is already initialized.
	  ExpressionId parentExecuting =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::EXECUTING_EXP()))->getId();
	  ExpressionListenerId parentExecutingListener = m_listeners[parentExecutingIdx];
	  parentExecuting->addListener(parentExecutingListener);
	  m_conditions[parentExecutingIdx] = parentExecuting;
	  m_garbageConditions[parentExecutingIdx] = true;

	  ExpressionId parentWaiting =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::WAITING_EXP()))->getId();
	  ExpressionListenerId parentWaitingListener = m_listeners[parentWaitingIdx];
	  parentWaiting->addListener(parentWaitingListener);
	  m_conditions[parentWaitingIdx] = parentWaiting;
	  m_garbageConditions[parentWaitingIdx] = true;

	  ExpressionId parentFinished =
		(new Equality(m_parent->getStateVariable(),
					  StateVariable::FINISHED_EXP()))->getId();
	  ExpressionListenerId parentFinishedListener = m_listeners[parentFinishedIdx];
	  parentFinished->addListener(parentFinishedListener);
	  m_conditions[parentFinishedIdx] = parentFinished;
	  m_garbageConditions[parentFinishedIdx] = true;
	}
	else {
	  // Dummies for root node
	  m_conditions[parentExecutingIdx] = BooleanVariable::TRUE_EXP();
	  m_conditions[parentWaitingIdx] = BooleanVariable::FALSE_EXP();
	  m_conditions[parentFinishedIdx] = BooleanVariable::FALSE_EXP();
	}

	// This will be overridden in any node with children (List or LibraryNodeCall)
    m_conditions[childrenWaitingOrFinishedIdx] = BooleanVariable::UNKNOWN_EXP();

	// This will be overridden in Command and Update nodes
    m_conditions[abortCompleteIdx] = BooleanVariable::UNKNOWN_EXP();

	// This will be overridden in Command nodes
    m_conditions[commandHandleReceivedIdx] = BooleanVariable::TRUE_EXP();
  }

  void Node::createDeclaredVars(const std::vector<PlexilVarId>& vars) {
    for (std::vector<PlexilVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {
	  // get the variable name
	  const std::string& name = (*it)->name();
	  LabelStr nameLabel(name);

	  // if it's an array, make me an array
	  if (Id<PlexilArrayVar>::convertable((*it)->getId())) {
		PlexilValue* value = (*it)->value();
		VariableId varId =
		  (VariableId)
		  ExpressionFactory::createInstance(value->name(), 
											value->getId(),
											m_connector);
		// FIXME: CHECK FOR DUPLICATE NAMES

		m_variablesByName[nameLabel] = varId;
		((VariableImpl*) varId)->setName(name);
		m_localVariables.push_back(varId);
		debugMsg("Node:createDeclaredVars",
				 " for node '" << m_nodeId.toString()
				 << "': created array variable "
				 << varId->toString()  << " as '"
				 << name << "'");
	  }

	  // otherwise create a non-array variable
	  else {
		PlexilValue* value = (*it)->value();
		VariableId varId =
		  (VariableId)
		  ExpressionFactory::createInstance(value->name(), 
											value->getId(),
											m_connector);
		// FIXME: CHECK FOR DUPLICATE NAMES

		m_variablesByName[nameLabel] = varId;
		((VariableImpl*) varId)->setName(name);
		m_localVariables.push_back(varId);
		debugMsg("Node:createDeclaredVars",
				 " for node '" << m_nodeId.toString()
				 << "': created variable " 
				 << varId->toString() << " as '"
				 << name << "'");
	  }
    }
  }

  void Node::getVarsFromInterface(const PlexilInterfaceId& intf)
  {
	if (!intf.isValid())
	  return;
      
	debugMsg("Node:getVarsFromInterface",
			 "Getting interface vars for node '" << m_nodeId.toString() << "'");
	checkError(m_parent.isId(), "Bizarre.  An interface on a parentless node.");
	//CHECK FOR DUPLICATE NAMES
	for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
		 it != intf->in().end();
		 ++it) {
		PlexilVarRef* varRef = *it;
		VariableId expr = m_parent->findVariable(varRef);
		// FIXME: push this check up into XML parser
		checkError(expr.isId(),
				   "No variable named '" << varRef->name() <<
				   "' in parent of node '" << m_nodeId.toString() << "'");
		// FIXME: push this check up into XML parser
		checkError(Id<VariableImpl>::convertable(expr)
				   || Id<AliasVariable>::convertable(expr),
				   "Expression named '" << varRef->name() <<
				   "' in parent of node '" << m_nodeId.toString() <<
				   "' is not a variable.");

		// Generate a constant alias for this variable
		VariableId alias =
		  (new AliasVariable(varRef->name(),
							 m_connector,
							 (Id<Variable>) expr,
							 true))->getId();

		// add alias to this node
		debugMsg("Node:getVarsFromInterface", 
				 " for node " << m_nodeId.c_str()
				 << ": Adding In variable "
				 << alias->toString()
				 << " as '" << varRef->name()
				 << "'"); 
		m_localVariables.push_back(alias);
		m_variablesByName[LabelStr(varRef->name())] = alias;
      }
      
	for (std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
		 it != intf->inOut().end();
		 ++it) {
	  VariableId expr = m_parent->findVariable(*it);
	  // FIXME: push this check up into XML parser
	  checkError(expr.isId(),
				 "No variable named '" << (*it)->name() <<
				 "' in parent of node '" << m_nodeId.toString() << "'");
         
	  // add variable to this node
	  debugMsg("Node:getVarsFromInterface", 
			   " for node '" << m_nodeId.c_str()
			   << "': Adding InOut variable "
			   << expr->toString()
			   << " as '" << (*it)->name()
			   << "'"); 
	  m_variablesByName[LabelStr((*it)->name())] = expr;
	}
  }

  void Node::createConditions(const std::map<std::string, PlexilExprId>& conds) 
  {
	if (m_parent.isId()) {
	  // Construct ancestor invariant and ancestor end
	  // Both were previously set to constants in setConditionDefaults()
	  ExpressionId ancestorInvariant =
		(new Conjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_INVARIANT_CONDITION()),
												 m_connector))->getId(),
						 true,
						 (new TransparentWrapper(m_parent->getCondition(INVARIANT_CONDITION()),
												 m_connector))->getId(),
						 true))->getId();
	  ExpressionListenerId ancestorInvariantListener = m_listeners[ancestorInvariantIdx];
	  ancestorInvariant->addListener(ancestorInvariantListener);
	  m_conditions[ancestorInvariantIdx] = ancestorInvariant;
	  m_garbageConditions[ancestorInvariantIdx] = true;;

	  ExpressionId ancestorEnd =
		(new Disjunction((new TransparentWrapper(m_parent->getCondition(ANCESTOR_END_CONDITION()),
												 m_connector))->getId(),
						 true,
						 (new TransparentWrapper(m_parent->getCondition(END_CONDITION()),
												 m_connector))->getId(),
						 true))->getId();
	  ExpressionListenerId ancestorEndListener = m_listeners[ancestorEndIdx];
	  ancestorEnd->addListener(ancestorEndListener);
	  m_conditions[ancestorEndIdx] = ancestorEnd;
	  m_garbageConditions[ancestorEndIdx] = true;
	}

	// Let the derived class do its thing
	createSpecializedConditions();

	// Add user-specified conditions
	for (std::map<std::string, PlexilExprId>::const_iterator it = conds.begin(); 
		 it != conds.end(); 
		 ++it) {
	  const LabelStr condName(it->first);
	  unsigned int condIdx = getConditionIndex(condName);

	  // Delete existing condition if required
	  // (e.g. explicit override of default end condition for list or library call node)
	  if (m_garbageConditions[condIdx]) {
		m_conditions[condIdx]->removeListener(m_listeners[condIdx]);
		delete (Expression*) m_conditions[condIdx];
		m_garbageConditions[condIdx] = false;
	  }

	  m_conditions[condIdx] = 
		ExpressionFactory::createInstance(it->second->name(), 
										  it->second,
										  m_connector, 
										  m_garbageConditions[condIdx]);
	  m_conditions[condIdx]->addListener(m_listeners[condIdx]);
	}

	// Create conditions that may wrap user-defined conditions
	createConditionWrappers();
  }

  // Default method
  void Node::createSpecializedConditions()
  {
  }

  // Default method
  void Node::createConditionWrappers()
  {
  }

  Node::~Node() 
  {
	debugMsg("Node:~Node", " base class destructor for " << m_nodeId.toString());

	// Remove anything that refers to variables, either ours or another node's
    cleanUpConditions();

	// Now safe to delete variables
    cleanUpVars();

	delete m_sortedVariableNames;
    delete (RealNodeConnector*) m_connector;
    m_id.remove();
  }

  void Node::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

	debugMsg("Node:cleanUpConditions", " for " << m_nodeId.toString());

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
 
    // Clean up children
	cleanUpChildConditions();

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

  // Default method.
  void Node::cleanUpChildConditions()
  {
  }

  // Default method.
  void Node::cleanUpNodeBody()
  {
  }

  void Node::cleanUpVars() 
  {
    if (m_cleanedVars)
      return;
    checkError(m_cleanedConditions,
			   "Have to clean up variables before conditions can be cleaned.");

	debugMsg("Node:cleanUpVars", " for " << m_nodeId.toString());

	// Clear map
	m_variablesByName.clear();

	// Delete user-spec'd variables
    for (std::vector<VariableId>::iterator it = m_localVariables.begin(); it != m_localVariables.end(); ++it) {
	  debugMsg("Node:cleanUpVars",
			   "<" << m_nodeId.toString() << "> Removing " << ((VariableImpl*)(*it))->getName());
	  delete (Variable*) (*it);
    }
	m_localVariables.clear();

	// Delete timepoint variables
	for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
	  delete (Variable*) m_startTimepoints[s];
	  delete (Variable*) m_endTimepoints[s];
	  m_startTimepoints[s] = m_endTimepoints[s] = VariableId::noId();
	}

	// Delete internal variables
    if (m_ack.isId()) {
      delete (Variable*) m_ack;
	  m_ack = VariableId::noId();
	}
	delete (Variable*) m_outcomeVariable;
	m_outcomeVariable = VariableId::noId();
	delete (Variable*) m_failureTypeVariable;
	m_failureTypeVariable = VariableId::noId();
	delete (Variable*) m_stateVariable;
	m_stateVariable = VariableId::noId();

    m_cleanedVars = true;
  }

  void Node::postInit() 
  {
    checkError(!m_postInitCalled, "Called postInit on node '" << m_nodeId.toString() << "' twice.");
    m_postInitCalled = true;

    debugMsg("Node:postInit", "Creating conditions for node '" << m_nodeId.toString() << "'");
    //create conditions and listeners
    createConditions(m_node->conditions());

    //create assignment/command
	specializedPostInit();
  }

  // Default method
  void Node::specializedPostInit()
  {
  }

  // Make the node (and its children, if any) active.
  void Node::activate()
  {
	// Activate internal variables
	activateInternalVariables();

	// These are the only conditions we care about in the INACTIVE state.
	// See getDestStateFromInactive().
	m_conditions[parentExecutingIdx]->activate();
    m_listeners[parentExecutingIdx]->activate();
	m_conditions[parentFinishedIdx]->activate();
    m_listeners[parentFinishedIdx]->activate();
	
	specializedActivate();
  }

  // Default method
  void Node::specializedActivate()
  {
  }

  void Node::activateInternalVariables()
  {
	// Activate internal variables
    m_stateVariable->activate();
	// TODO: figure out if these should be activated on entering EXECUTING state
    m_outcomeVariable->activate();
    m_failureTypeVariable->activate();

	// Activate timepoints
	// TODO: figure out if they should be inactive until entering the corresponding state
	for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
	  m_startTimepoints[s]->activate();
	  m_endTimepoints[s]->activate();
	}

	specializedActivateInternalVariables();
  }

  // Default method
  void Node::specializedActivateInternalVariables()
  {
  }

  ExpressionId& Node::getCondition(const LabelStr& name) {
    return m_conditions[getConditionIndex(name)];
  }

  // Default method.
  const std::vector<NodeId>& Node::getChildren() const
  {
	static std::vector<NodeId> sl_emptyNodeVec;
	return sl_emptyNodeVec;
  }

  double Node::getAcknowledgementValue() const 
  {
    return ((Variable*)m_ack)->getValue();
  }

  /**
   * @brief Notifies the node that one of its conditions has changed.
   */
  void Node::conditionChanged()
  {
    if (m_checkConditionsPending)
      return;
    m_exec->notifyNodeConditionChanged(m_id);
    m_checkConditionsPending = true;
  }

  /**
   * @brief Evaluates the conditions to see if the node is eligible to transition.
   */
  void Node::checkConditions() {
    m_checkConditionsPending = false;
    checkError(m_stateVariable->getValue() == StateVariable::nodeStateName(m_state).getKey(),
			   "Node state not synchronized for node " << m_nodeId.toString()
			   << "; node state = " << m_state
			   << ", node state name = \"" << Expression::valueToString(m_stateVariable->getValue()) << "\"");

    if(m_transitioning)
      return;

    debugMsg("Node:checkConditions",
			 "Checking condition change for node " << m_nodeId.toString());
    NodeState toState(getDestState());
    debugMsg("Node:checkConditions",
			 "Can (possibly) transition to " << StateVariable::nodeStateName(toState).toString());
    if (toState != m_lastQuery) {
      if (toState != NO_NODE_STATE ||
		  (toState == NO_NODE_STATE &&
		   m_lastQuery != NO_NODE_STATE))
		m_exec->handleConditionsChanged(m_id);
      m_lastQuery = toState;
    }
  }

  NodeState Node::getDestState() 
  {
    debugMsg("Node:getDestState",
			 "Getting destination state for " << m_nodeId.toString() << " from state " <<
			 getStateName().toString());
    // return m_stateManager->getDestState(m_id);
	switch (m_state) {
	case INACTIVE_STATE:
	  return getDestStateFromInactive();

	case WAITING_STATE:
	  return getDestStateFromWaiting();

	case EXECUTING_STATE:
	  return getDestStateFromExecuting();

	case FINISHING_STATE:
	  return getDestStateFromFinishing();

	case FINISHED_STATE:
	  return getDestStateFromFinished();

	case FAILING_STATE:
	  return getDestStateFromFailing();

	case ITERATION_ENDED_STATE:
	  return getDestStateFromIterationEnded();

	default:
	  checkError(ALWAYS_FAIL,
				 "Node::getDestState: invalid node state " << m_state);
	  return NO_NODE_STATE;
	}
  }

  //
  // Next-state logic
  //

  // Default method
  NodeState Node::getDestStateFromInactive()
  {
	checkError(isParentExecutingConditionActive(), 
			   "Parent executing for " << m_nodeId.toString() << " is inactive.");
	checkError(isParentFinishedConditionActive(), 
			   "Parent finished for " << m_nodeId.toString() << " is inactive.");

	if (getParentFinishedCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: FINISHED.");
	  condDebugMsg(getParentFinishedCondition()->getValue() ==
				   BooleanVariable::TRUE_VALUE(),
				   "Node:getDestState", "PARENT_FINISHED_CONDITION true.");
	  return FINISHED_STATE;
	}
	if (getParentExecutingCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: WAITING.  PARENT_EXECUTING_CONDITION true");
	  return WAITING_STATE;
	}
	debugMsg("Node:getDestState", "Destination: no state.");
	return NO_NODE_STATE;
  }

  // Default method
  NodeState Node::getDestStateFromWaiting()
  {
	checkError(isAncestorInvariantConditionActive(), "Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
	checkError(isAncestorEndConditionActive(), "Ancestor end for " << m_nodeId.toString() << " is inactive.");
	checkError(isSkipConditionActive(), "Skip for " << m_nodeId.toString() << " is inactive.");
	checkError(isStartConditionActive(), "Start for " << m_nodeId.toString() << " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE() ||
		getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE() ||
		getSkipCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: FINISHED.");
	  condDebugMsg(getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
				   "Node:getDestState", "ANCESTOR_INVARIANT_CONDITION false.");
	  condDebugMsg(getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE(),
				   "Node:getDestState", "ANCESTOR_END_CONDITION true.");
	  condDebugMsg(getSkipCondition()->getValue() == BooleanVariable::TRUE_VALUE(),
				   "Node:getDestState", "SKIP_CONDITION true.");
	  return FINISHED_STATE;
	}
	if (getStartCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if (getPreCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
		debugMsg("Node:getDestState", "Destination: EXECUTING.  START_CONDITION and PRE_CONDITION are both true.");
		return EXECUTING_STATE;
	  }
	  else {
		debugMsg("Node:getDestState", "Destination: ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
		return ITERATION_ENDED_STATE;
	  }
	}
	debugMsg("Node:getDestState", "Destination: no state.  START_CONDITION false or unknown");
	return NO_NODE_STATE;
  }

  // Empty node method
  NodeState Node::getDestStateFromExecuting()
  {
	checkError(getType() == Node::EMPTY(),
			   "Expected empty node, got " <<
			   getType().toString());

	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
	checkError(isInvariantConditionActive(),
			   "Invariant for " << m_nodeId.toString() << " is inactive.");
	checkError(isEndConditionActive(),
			   "End for " << m_nodeId.toString() << " is inactive.");

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

  // Default method
  NodeState Node::getDestStateFromFinishing()
  {
    checkError(ALWAYS_FAIL,
			   "Attempted to compute destination state for node " << m_nodeId.toString()
			   << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  NodeState Node::getDestStateFromFinished()
  {
	checkError(isParentWaitingConditionActive(),
			   "Parent waiting for " << m_nodeId.toString() << " is inactive.");

	if (getParentWaitingCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState", "Destination: INACTIVE.  PARENT_WAITING true.");
	  return INACTIVE_STATE;
	}
	debugMsg("Node:getDestState", "Destination: no state.  PARENT_WAITING false or unknown.");
	return NO_NODE_STATE;
  }

  // Default method
  NodeState Node::getDestStateFromFailing()
  {
    checkError(ALWAYS_FAIL,
			   "Attempted to compute destination state for node " << m_nodeId.toString()
			   << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  NodeState Node::getDestStateFromIterationEnded()
  {
	checkError(isAncestorInvariantConditionActive(),
			   "Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
	checkError(isAncestorEndConditionActive(),
			   "Ancestor end for " << m_nodeId.toString() << " is inactive.");
	checkError(isRepeatConditionActive(),
			   "Repeat for " << m_nodeId.toString() << " is inactive.");

	if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE() ||
		getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE() ||
		getRepeatCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
	  debugMsg("Node:getDestState", "'" << m_nodeId.toString() << "' destination: FINISHED.");
	  condDebugMsg(getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
				   "Node:getDestState", "ANCESTOR_INVARIANT false.");
	  condDebugMsg(getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE(),
				   "Node:getDestState", "ANCESTOR_END true.");
	  condDebugMsg(getRepeatCondition()->getValue() == BooleanVariable::FALSE_VALUE(),
				   "Node:getDestState", "REPEAT_CONDITION false.");
	  return FINISHED_STATE;
	}
	if (getRepeatCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  debugMsg("Node:getDestState",
			   "'" << m_nodeId.toString() << "' destination: WAITING.  REPEAT_UNTIL true.");
	  return WAITING_STATE;
	}
	debugMsg("Node:getDestState",
			 "'" << m_nodeId.toString() << "' destination: no state.  ANCESTOR_END false or unknown and REPEAT unknown.");
	return NO_NODE_STATE;
  }

  //
  // State transition logic
  //

  // This method is currently used only by exec-test-module.
  // Its logic has been absorbed into transition() below to avoid redundant calls to getDestState().
  bool Node::canTransition()
  {
    NodeState toState = getDestState();
    return toState != NO_NODE_STATE && toState != m_state;
  }

  void Node::transition(const double time) 
  {
    checkError(m_stateVariable->getValue() == StateVariable::nodeStateName(m_state).getKey(),
			   "Node state not synchronized for node " << m_nodeId.toString()
			   << "; node state = " << m_state
			   << ", node state name = \"" << Expression::valueToString(m_stateVariable->getValue()) << "\"");
    checkError(!m_transitioning,
			   "Node " << m_nodeId.toString() << " is already transitioning.");

    m_transitioning = true;
    NodeState prevState = m_state;
	NodeState destState = getDestState();
    checkError(destState != NO_NODE_STATE
			   && destState != m_state,
			   "Attempted to transition node " << m_nodeId.toString() <<
			   " when it is ineligible.");
	
	transitionFrom(destState);
	transitionTo(destState);

    debugMsg("Node:transition", "Transitioning '" << m_nodeId.toString() <<
			 "' from " << StateVariable::nodeStateName(prevState).toString() <<
			 " to " << StateVariable::nodeStateName(destState).toString());
    condDebugMsg((destState == FINISHED_STATE),
                 "Node:outcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    condDebugMsg((destState == FINISHED_STATE && getOutcome() == OutcomeVariable::FAILURE()),
                 "Node:failure",
                 "Failure type of '" << m_nodeId.toString() <<
                 "' is " << getFailureType().toString());
    condDebugMsg((destState == ITERATION_ENDED_STATE),
                 "Node:iterationOutcome",
                 "Outcome of '" << m_nodeId.toString() <<
                 "' is " << getOutcome().toString());
    debugMsg("Node:times",
			 "Setting end time " << LabelStr(END_TIMEPOINT_NAMES()[prevState]).toString()
			 << " = " << std::setprecision(15) << time);
    debugMsg("Node:times",
			 "Setting start time " << LabelStr(START_TIMEPOINT_NAMES()[destState]).toString()
			 << " = " << std::setprecision(15) << time);
    m_endTimepoints[prevState]->setValue(time);
    m_startTimepoints[destState]->setValue(time);
    m_transitioning = false;
    conditionChanged(); // was checkConditions();
  }

  // Empty node method 
  void Node::transitionFrom(NodeState destState)
  {
	switch (m_state) {
	case INACTIVE_STATE:
	  transitionFromInactive(destState);
	  break;

	case WAITING_STATE:
	  transitionFromWaiting(destState);
	  break;

	case EXECUTING_STATE:
	  transitionFromExecuting(destState);
	  break;

	case FINISHING_STATE:
	  transitionFromFinishing(destState);
	  break;

	case FINISHED_STATE:
	  transitionFromFinished(destState);
	  break;

	case FAILING_STATE:
	  transitionFromFailing(destState);
	  break;

	case ITERATION_ENDED_STATE:
	  transitionFromIterationEnded(destState);
	  break;

	default:
	  checkError(ALWAYS_FAIL,
				 "Node::transitionFrom: Invalid node state " << m_state);
	}
  }

  // Empty node method 
  void Node::transitionTo(NodeState destState)
  {
	switch (destState) {
	case INACTIVE_STATE:
	  transitionToInactive();
	  break;

	case WAITING_STATE:
	  transitionToWaiting();
	  break;

	case EXECUTING_STATE:
	  transitionToExecuting();
	  break;

	case FINISHING_STATE:
	  transitionToFinishing();
	  break;

	case FINISHED_STATE:
	  transitionToFinished();
	  break;

	case FAILING_STATE:
	  transitionToFailing();
	  break;

	case ITERATION_ENDED_STATE:
	  transitionToIterationEnded();
	  break;

	default:
	  checkError(ALWAYS_FAIL,
				 "Node::transitionTo: Invalid destination state " << destState);
	}

	setState(destState);
  }

  // Default method
  void Node::transitionFromInactive(NodeState destState)
  {
	checkError(destState == WAITING_STATE ||
			   destState == FINISHED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");
	deactivateParentExecutingCondition();
	deactivateParentFinishedCondition();
	if (destState == FINISHED_STATE)
	  getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
  }

  // Default method
  void Node::transitionFromWaiting(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == EXECUTING_STATE ||
			   destState == ITERATION_ENDED_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");
	deactivateStartCondition();
	deactivateSkipCondition();
	deactivateAncestorEndCondition();
	deactivateAncestorInvariantCondition();
	deactivatePreCondition();

	if (destState == FINISHED_STATE)
	  getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
	else if (destState == ITERATION_ENDED_STATE) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::PRE_CONDITION_FAILED());
	}
  }

  // Empty node method
  void Node::transitionFromExecuting(NodeState destState)
  {
	checkError(m_nodeType == Node::EMPTY(),
			   "Expected empty node, got " <<
			   m_nodeType.toString());
	checkError(destState == FINISHED_STATE ||
			   destState == ITERATION_ENDED_STATE,
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
	else if(getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
	  if(getPostCondition()->getValue() == BooleanVariable::TRUE_VALUE())
		getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
	  else {
		getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
		getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
	  }
	}
	else {
	  checkError(ALWAYS_FAIL, "Shouldn't get here.");
	}

	deactivateAncestorInvariantCondition();
	deactivateInvariantCondition();
	deactivateEndCondition();
	deactivatePostCondition();
  }

  // Default method
  void Node::transitionFromFinishing(NodeState destState)
  {
	checkError(ALWAYS_FAIL,
			   "No transition from FINISHING state defined for this node");
  }

  // Default method
  void Node::transitionFromFinished(NodeState destState)
  {
	checkError(destState == INACTIVE_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");
	deactivateParentWaitingCondition();
	reset();
  }

  // Default method
  void Node::transitionFromFailing(NodeState destState)
  {
	checkError(ALWAYS_FAIL,
			   "No transition from FAILING state defined for this node");
  }

  // Default method
  void Node::transitionFromIterationEnded(NodeState destState)
  {
	checkError(destState == FINISHED_STATE ||
			   destState == WAITING_STATE,
			   "Attempting to transition to invalid state '"
			   << StateVariable::nodeStateName(destState).toString() << "'");

	if (getAncestorInvariantCondition()->getValue() ==
		BooleanVariable::FALSE_VALUE()) {
	  getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
	  getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
	}

	deactivateRepeatCondition();
	deactivateAncestorEndCondition();
	deactivateAncestorInvariantCondition();

	if (destState == WAITING_STATE)
	  reset();
  }


  // Default method
  void Node::transitionToInactive()
  {
	activateParentExecutingCondition();
	activateParentFinishedCondition();
  }

  // Default method
  void Node::transitionToWaiting()
  {
	activateStartCondition();
	activatePreCondition();
	activateSkipCondition();
	activateAncestorEndCondition();
	activateAncestorInvariantCondition();
  }

  // Empty node method
  void Node::transitionToExecuting()
  {
	checkError(getType() == Node::EMPTY(),
			   "Expected empty node, got " << getType().toString());

	activateAncestorInvariantCondition();
	activateInvariantCondition();
	activateEndCondition();
	activatePostCondition();
  }

  // Default method
  void Node::transitionToFinishing()
  {
	checkError(ALWAYS_FAIL,
			   "No transition to FINISHING state defined for this node");
  }

  // Default method
  void Node::transitionToFinished()
  {
	activateParentWaitingCondition();
  }

  // Default method
  void Node::transitionToFailing()
  {
	checkError(ALWAYS_FAIL,
			   "No transition to FAILING state defined for this node");
  }

  // Default method
  void Node::transitionToIterationEnded() 
  {
	activateRepeatCondition();
	activateAncestorEndCondition();
	activateAncestorInvariantCondition();
  }

  const VariableId& Node::getInternalVariable(const LabelStr& name) const{
    checkError(m_variablesByName.find(name) != m_variablesByName.end(),
			   "No variable named " << name.toString() << " in " << m_nodeId.toString());
    return m_variablesByName.find(name)->second;
  }

  const LabelStr& Node::getStateName() const {
    return StateVariable::nodeStateName(m_state);
  }

  NodeState Node::getState() const {
    return m_state;
  }

  // Some transition handlers call this twice.
  void Node::setState(NodeState newValue) {
    checkError(newValue < NO_NODE_STATE,
			   "Attempted to set an invalid NodeState value");
	if (newValue == m_state)
	  return;
    m_state = newValue;
    ((StateVariable*) m_stateVariable)->setNodeState(newValue);
  }

  double Node::getCurrentStateStartTime() const
  {
    return m_startTimepoints[m_state]->getValue();
  }

  double Node::getCurrentStateEndTime() const
  {
    return m_endTimepoints[m_state]->getValue();
  }

  const LabelStr Node::getOutcome() {
    return m_outcomeVariable->getValue();
  }

  const LabelStr Node::getFailureType() {
    return m_failureTypeVariable->getValue();
  }

  // Searches ancestors when required
  const VariableId& Node::findVariable(const LabelStr& name, bool recursive)
  {
    debugMsg("Node:findVariable",
			 " for node '" << m_nodeId.toString()
			 << "', searching by name for \"" << name.toString() << "\"");
    VariableMap::const_iterator it = m_variablesByName.find(name.getKey());
	if (it != m_variablesByName.end()) {
	  debugMsg("Node:findVariable",
			   " Returning " << it->second->toString());
	  return it ->second;
	}

	// Not found locally - try ancestors if possible
	// Stop at library call nodes, as interfaces there are explicit
	// FIXME: Figure out how to implement firewall by subclassing
	if (m_parent.isId()
		&& m_parent->m_nodeType != LIBRARYNODECALL()) {
	  const VariableId& result = m_parent->findVariable(name, true);
	  if (result.isId()) {
		// Found it - cache for later reuse
		m_variablesByName[name.getKey()] = result;
		return result;
	  }
	  // Not found 
	  else if (recursive)
		return VariableId::noId(); // so that error happens at approriate level
	  // else fall through to failure
	}

	// FIXME: push this check up into XML parser
	checkError(ALWAYS_FAIL,
			   "No variable named \"" << name.toString() << "\" accessible from node " <<
			   m_nodeId.toString());
	return VariableId::noId();
  }

  const VariableId& Node::findVariable(const PlexilVarRef* ref)
  {
	debugMsg("Node:findVariable",
			 " for node '" << m_nodeId.toString()
			 << "', searching for variable '" << ref->name() << "'");
      
	if (Id<PlexilInternalVar>::convertable(ref->getId())) {
	  PlexilInternalVar* var = (PlexilInternalVar*) ref;
	  PlexilNodeRef* nodeRef = var->ref();
	  NodeId node = NodeId::noId();
         
	  switch(nodeRef->dir()) {
	  case PlexilNodeRef::SELF:
		node = m_id;
		break;

	  case PlexilNodeRef::PARENT:
		// FIXME: push this check up into XML parser
		checkError(m_parent.isValid(),
				   "Parent node reference in root node " << 
				   m_nodeId.toString());
		node = m_parent;
		break;

	  case PlexilNodeRef::CHILD:
		{
		  node = findChild(LabelStr(nodeRef->name()));
		  // FIXME: push this check up into XML parser
		  checkError(node.isId(),
					 "No child named '" << nodeRef->name() << 
					 "' in " << m_nodeId.toString());
		  break;
		}

	  case PlexilNodeRef::SIBLING: 
		{
		  // FIXME: push this check up into XML parser
		  checkError(m_parent.isValid(),
					 "Sibling node reference in root node " << 
					 m_nodeId.toString());
		  node = m_parent->findChild(LabelStr(nodeRef->name()));
		  // FIXME: push this check up into XML parser
		  checkError(node.isId(),
					 "No sibling named '" << nodeRef->name() << 
					 "' of " << m_nodeId.toString());
		  break;
		}

	  case PlexilNodeRef::NO_DIR:
	  default:
		// FIXME: catch this error in XML parsing
		checkError(ALWAYS_FAIL,
				   "Invalid direction in node reference from " <<
				   m_nodeId.toString());
		return VariableId::noId();
	  }

	  std::string name;
	  if(Id<PlexilTimepointVar>::convertable(var->getId())) {
		PlexilTimepointVar* tp = (PlexilTimepointVar*) var;
		name = tp->state() + "." + tp->timepoint();
	  }
	  else
		name = var->name();
	  debugMsg("Node:findVariable", 
			   " Found internal variable \"" << name << "\"");
	  return node->getInternalVariable(LabelStr(name));
	}
	else {
	  return findVariable(LabelStr(ref->name()));
	}
  }

  // Default method
  NodeId Node::findChild(const LabelStr& childName) const
  {
	return NodeId::noId();
  }

  void Node::activatePair(unsigned int idx) {
    checkError(m_listeners[idx].isId() && m_conditions[idx].isId(),
			   "No condition/listener pair exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:activatePair",
			 "Activating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    m_listeners[idx]->activate();
    m_conditions[idx]->activate();
  }

  void Node::deactivatePair(unsigned int idx) {
    checkError(m_listeners[idx].isId() && m_conditions[idx].isId(),
			   "No condition/listener pair exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:deactivatePair",
			 "Deactivating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    m_conditions[idx]->deactivate();
    if (m_listeners[idx]->isActive())
      m_listeners[idx]->deactivate();
  }

  bool Node::pairActive(unsigned int idx) {
    checkError(idx < conditionIndexMax,
			   "Invalid condition index " << idx);
    bool listenActive = m_listeners[idx]->isActive();
    condDebugMsg(!listenActive, 
				 "Node:pairActive",
				 "Listener for " << getConditionName(idx).toString() << " in " << m_nodeId.toString() <<
				 " is inactive.");
    bool condActive = m_conditions[idx]->isActive();
    condDebugMsg(!condActive, 
				 "Node:pairActive",
				 "Condition " << getConditionName(idx).toString() << " in " << m_nodeId.toString() <<
				 " is inactive.");
    return listenActive && condActive;
  }

  void Node::execute() 
  {
    debugMsg("Node:execute", "Executing node " << m_nodeId.toString());
    // activate local variables
    for (std::vector<VariableId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         vit++) {
	  (*vit)->activate();
	}
	handleExecution();
  }

  void Node::handleExecution()
  {
    checkError(m_state == EXECUTING_STATE,
			   "Node \"" << m_nodeId.toString()
			   << "\" told to handle execution, but it's in state '" <<
			   getStateName().toString() << "'");
	// legacy message for unit test
    debugMsg("PlexilExec:handleNeedsExecution",
			 "Storing action for node '" << m_nodeId.toString() <<
			 "' of type '" << m_nodeType.toString() << 
             "' to be executed.");

	// Here only to placate the unit test
	m_exec->notifyExecuted(getId());

	specializedHandleExecution();
  }

  // default method
  void Node::specializedHandleExecution()
  {
  }

  void Node::reset()
  {
    debugMsg("Node:reset", "Re-setting node " << m_nodeId.toString());

    //reset outcome and failure type
	m_outcomeVariable->reset();
	m_failureTypeVariable->reset();

    //reset timepoints
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      m_startTimepoints[s]->reset();
      m_endTimepoints[s]->reset();
    }

    for (std::vector<VariableId>::const_iterator it = m_localVariables.begin();
		 it != m_localVariables.end();
		 ++it)
      (*it)->reset();

	specializedReset();
  }

  // Default method
  void Node::specializedReset()
  {
  }

  // Default method
  void Node::abort() 
  {
    debugMsg("Node:abort", "Aborting node " << m_nodeId.toString());
	debugMsg("Warning", "No abort for node type " << getType().toString() << " yet.");
	//checkError(ALWAYS_FAIL, "No abort currently for node type " << getType().toString());
  }

  /* old version
	 void Node::lockConditions() {
	 for(ExpressionMap::iterator it = m_conditionsByName.begin();
	 it != m_conditionsByName.end(); ++it) {
	 ExpressionId expr = it->second;
	 check_error(expr.isValid());
	 if(pairActive(it->first) && !expr->isLocked()) {
	 debugMsg("Node:lockConditions",
	 "In " << m_nodeId.toString() << ", locking " <<
	 LabelStr(it->first).toString() << " " << expr->toString());
	 expr->lock();
	 }
	 }
	 }
  */

  // Optimized version
  // N.B. we omit the validity check on the condition expression
  // because this is a critical path method in the inner loop of the Exec.
  void Node::lockConditions() {
    for (unsigned int i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = m_conditions[i];
      checkError(m_listeners[i].isId(), // isValid() ??
				 "Node::lockConditions: no listener named " << getConditionName(i).toString());
      if (m_listeners[i]->isActive()
		  && expr->isActive()
		  && !expr->isLocked()) {
		debugMsg("Node:lockConditions",
				 "In " << m_nodeId.toString() << ", locking " <<
				 getConditionName(i).toString() << " " << expr->toString());
		expr->lock();
      }
    }
  }

  // As above, skip the Id validity check because this is a critical path function.
  void Node::unlockConditions() {
    for (unsigned int i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = m_conditions[i];
      // check_error(expr.isValid());
      if(expr->isLocked()) {
		debugMsg("Node:unlockConditions",
				 "In " << m_nodeId.toString() << ", unlocking " <<
				 getConditionName(i).toString() << " " << expr->toString());
		expr->unlock();
      }
    }
  }

  void Node::deactivateExecutable() 
  {
	specializedDeactivateExecutable();
	deactivateLocalVariables();
  }

  // Default method
  void Node::specializedDeactivateExecutable()
  {
  }

  void Node::deactivateLocalVariables()
  {
    for (std::vector<VariableId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         vit++)
	  (*vit)->deactivate();
  }

  std::string Node::toString(const unsigned int indent)
  {
    std::ostringstream retval;
	print(retval, indent);
    return retval.str();
  }

  std::ostream& operator<<(std::ostream& stream, const Node& node)
  {
	node.print(stream, 0);
	return stream;
  }

  void Node::print(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');

    stream << indentStr << m_nodeId.toString() << "{\n";
    stream << indentStr << " State: " << m_stateVariable->toString() <<
      " (" << m_startTimepoints[m_state]->getValue() << ")\n";
    if (m_state == FINISHED_STATE) {
      stream << indentStr << " Outcome: " << m_outcomeVariable->toString() << '\n';
      if (m_failureTypeVariable->getValue() != Expression::UNKNOWN())
		stream << indentStr << " Failure type: " <<
		  m_failureTypeVariable->toString() << '\n';
	  // Print variables, starting with command handle
	  printCommandHandle(stream, indent, false);
	  printVariables(stream, indent);
    }
    else if (m_state != INACTIVE_STATE) {
	  // Print conditions
      for (unsigned int i = 0; i < conditionIndexMax; ++i) {
		stream << indentStr << " " << getConditionName(i).toString() << ": " <<
		  m_conditions[i]->toString() << '\n';
      }
	  // Print variables, starting with command handle (if appropriate)
	  printCommandHandle(stream, indent, true);
	  printVariables(stream, indent);
    }
	// print children
    for (std::vector<NodeId>::const_iterator it = getChildren().begin(); it != getChildren().end(); ++it) {
      stream << (*it)->toString(indent + 2);
    }
    stream << indentStr << "}" << std::endl;
  }

  // Print variables
  void Node::printVariables(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
	ensureSortedVariableNames(); // for effect
	for (std::vector<double>::const_iterator it = m_sortedVariableNames->begin();
		 it != m_sortedVariableNames->end();
		 it++) {
	  stream << indentStr << " " << LabelStr(*it).toString() << ": " <<
		getInternalVariable(LabelStr(*it))->toString() << '\n';
	}
  }

  // Default method does nothing
  void Node::printCommandHandle(std::ostream& stream, const unsigned int indent, bool always) const
  {
  }

  // Helper used below
  bool labelStrLessThan(double a, double b)
  {
	return LabelStr(a).toString() < LabelStr(b).toString();
  }

  void Node::ensureSortedVariableNames() const
  {
	checkError(m_sortedVariableNames != NULL,
			   "Internal error: m_sortedVariableNames is null!");
	if (m_sortedVariableNames->empty()) {
	  // Collect the variable names
	  for (VariableMap::const_iterator it = m_variablesByName.begin();
		   it != m_variablesByName.end();
		   it++) {
		double nameKey = it->first;
		if (nameKey == STATE().getKey()
			|| nameKey == OUTCOME().getKey()
			|| nameKey == FAILURE_TYPE().getKey()
			|| nameKey == COMMAND_HANDLE().getKey()
			|| LabelStr(nameKey).countElements(".") > 1)
		  continue;
		m_sortedVariableNames->push_back(it->first);
	  }
	  // Sort the names
	  std::sort(m_sortedVariableNames->begin(),
				m_sortedVariableNames->end(),
				labelStrLessThan);
	}
  }

  // Static "constants"
  const std::vector<double>& Node::START_TIMEPOINT_NAMES() {
    static std::vector<double>* startNames = NULL;
    if (startNames == NULL) {
      startNames = new std::vector<double>();
      startNames->reserve(NODE_STATE_MAX);
      for (std::vector<LabelStr>::const_iterator it = StateVariable::ALL_STATES().begin();
		   it != StateVariable::ALL_STATES().end(); 
		   ++it) {
		const std::string& state = it->toString();
		LabelStr startName(state + ".START");
		startNames->push_back(startName.getKey());
      }
    }
    return *startNames;
  }

  const std::vector<double>& Node::END_TIMEPOINT_NAMES() {
    static std::vector<double>* endNames = NULL;
    if (endNames == NULL) {
      endNames = new std::vector<double>();
      endNames->reserve(NODE_STATE_MAX);
      for (std::vector<LabelStr>::const_iterator it = StateVariable::ALL_STATES().begin();
		   it != StateVariable::ALL_STATES().end(); 
		   ++it) {
		const std::string& state = it->toString();
		LabelStr endName(state + ".END");
		endNames->push_back(endName.getKey());
      }
    }
    return *endNames;
  }

}
