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

#include "Node.hh"
#include "Array.hh"
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

#include <algorithm> // for sort
#include <vector>
#include <sstream>
#include <iomanip> // for setprecision

namespace PLEXIL {

  ConditionChangeListener::ConditionChangeListener(Node& node, size_t condIdx)
    : ExpressionListener(), m_node(node), m_cond(Node::ALL_CONDITIONS()[condIdx])
  {
  }

  void ConditionChangeListener::notifyValueChanged(const ExpressionId& /* expression */) 
  {
    debugMsg("Node:conditionChange",
             m_cond.toString() << " may have changed value in " << m_node.getNodeId().toString());
    m_node.conditionChanged();
  }

  const std::vector<double>& Node::ALL_CONDITIONS() {
    static std::vector<double>* sl_allConds = NULL;
    if (sl_allConds == NULL) {
      // *** N.B.: Order MUST agree with enum ConditionIndex!
      sl_allConds = new std::vector<double>();
      // Conditions on parent
      sl_allConds->push_back(ANCESTOR_END_CONDITION());
      sl_allConds->push_back(ANCESTOR_EXIT_CONDITION());
      sl_allConds->push_back(ANCESTOR_INVARIANT_CONDITION());
      // User specified conditions
      sl_allConds->push_back(SKIP_CONDITION());
      sl_allConds->push_back(START_CONDITION());
      sl_allConds->push_back(END_CONDITION());
      sl_allConds->push_back(EXIT_CONDITION());
      sl_allConds->push_back(INVARIANT_CONDITION());
      sl_allConds->push_back(PRE_CONDITION());
      sl_allConds->push_back(POST_CONDITION());
      sl_allConds->push_back(REPEAT_CONDITION());
      // For all but Empty nodes
      sl_allConds->push_back(ACTION_COMPLETE());
      // For all but Empty and Update nodes
      sl_allConds->push_back(ABORT_COMPLETE());
      // inexpensive sanity check
      assertTrue(sl_allConds->size() == conditionIndexMax,
                 "INTERNAL ERROR: Inconsistency between conditionIndex enum and ALL_CONDITIONS");
    }
    return *sl_allConds;
  }

  size_t Node::getConditionIndex(const LabelStr& cName) {
    double nameKey = cName.getKey();
    const std::vector<double>& allConds = ALL_CONDITIONS();
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (allConds[i] == nameKey)
        return i;
    }
    assertTrueMsg(ALWAYS_FAIL,
                  cName.toString() << " is not a valid condition name");
    return conditionIndexMax; // make compiler happy
  }

  LabelStr Node::getConditionName(size_t idx)
  {
    return LabelStr(ALL_CONDITIONS()[idx]);
  }

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
    : NodeConnector(),
      m_id(this, NodeConnector::getId()),
      m_parent(parent),
      m_exec(exec),
      m_nodeId(node->nodeId()),
      m_nodeType(nodeTypeToLabelStr(node->nodeType())), // Can throw exception
      m_sortedVariableNames(new std::vector<double>()),
      m_state(INACTIVE_STATE),
      m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false),
      m_cleanedConditions(false),
      m_cleanedVars(false),
      m_checkConditionsPending(false)
  {
    debugMsg("Node:node", "Creating node \"" << node->nodeId() << "\"");

    commonInit();
    setConditionDefaults();

    // Instantiate declared variables
    createDeclaredVars(node->declarations());

    // get interface variables
    if (node->interface().isId())
      getVarsFromInterface(node->interface());
  }

  // Used only by module test
  Node::Node(const LabelStr& type, const LabelStr& name, const NodeState state,
             const ExecConnectorId& exec,
             const NodeId& parent)
    : NodeConnector(),
      m_id(this, NodeConnector::getId()),
      m_parent(parent),
      m_exec(exec),
      m_nodeId(name),
      m_nodeType(type),
      m_sortedVariableNames(new std::vector<double>()),
      m_state(state),
      m_lastQuery(NO_NODE_STATE),
      m_postInitCalled(false), 
      m_cleanedConditions(false), 
      m_cleanedVars(false),
      m_checkConditionsPending(false)
  {
    commonInit();
    activateInternalVariables();

    for (size_t i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = (new BooleanVariable(BooleanVariable::FALSE_VALUE(), false))->getId();
      debugMsg("Node:node",
               " Created internal variable "
               << LabelStr(ALL_CONDITIONS()[i]).toString() <<
               " with value FALSE for node " << m_nodeId.toString());
      m_conditions[i] = expr;
      m_garbageConditions[i] = true;
      getCondition(i)->addListener(makeConditionListener(i)); // may not be same as expr for parent-state conditions
    }

    // Activate the conditions required by the provided state
    switch (m_state) {

    case INACTIVE_STATE:
      break;

    case WAITING_STATE:
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
      activateExitCondition();
      activatePreCondition();
      activateSkipCondition();
      activateStartCondition();
      break;

    case EXECUTING_STATE:
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
      activateEndCondition();
      activateExitCondition();
      activateInvariantCondition();
      activatePostCondition();
      break;

    case FAILING_STATE:
      assertTrueMsg(m_nodeType != EMPTY(),
                    "Node module test constructor: FAILING state invalid for " << m_nodeType.toString() << " nodes");
      // Defer to subclass
      break;

    case FINISHING_STATE:
      assertTrueMsg(m_nodeType != EMPTY(),
                    "Node module test constructor: FINISHING state invalid for " << m_nodeType.toString() << " nodes");
      // Defer to subclass
      break;

    case ITERATION_ENDED_STATE:
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
      activateRepeatCondition();
      break;

    case FINISHED_STATE:
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL, "Node module test constructor: Invalid state " << state);
    }
  }

  void Node::commonInit() {
    debugMsg("Node:node", "Instantiating internal variables...");
    // Instantiate state/outcome/failure variables
    // The contortions with getKey() are an attempt to minimize LabelStr copying
    m_variablesByName[STATE().getKey()] = m_stateVariable =
      (new StateVariable(m_nodeId.toString()))->getId();
    ((StateVariable*) m_stateVariable)->setNodeState(m_state);

    m_variablesByName[OUTCOME().getKey()] = m_outcomeVariable =
      (new OutcomeVariable(m_nodeId.toString()))->getId();

    m_variablesByName[FAILURE_TYPE().getKey()] = m_failureTypeVariable =
      (new FailureVariable(m_nodeId.toString()))->getId();

    //instantiate timepoint variables
    // FIXME: Don't instantiate variables for node states that node type doesn't implement
    debugMsg("Node:node", "Instantiating timepoint variables.");
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; ++s) {
      ExpressionId stp = (new RealVariable())->getId();
      double stpName = START_TIMEPOINT_NAMES()[s];
      m_startTimepoints[s] = m_variablesByName[stpName] = stp;

      ExpressionId etp = (new RealVariable())->getId();
      const LabelStr& etpName = END_TIMEPOINT_NAMES()[s];
      m_endTimepoints[s] = m_variablesByName[etpName] = etp;
    }

    // initialize m_garbageConditions
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      m_garbageConditions[i] = false;
    }
  }

  // Use existing Boolean constants for the condition defaults
  void Node::setConditionDefaults() 
  {
    // These may be user-specified
    // End condition will be overridden 
    m_conditions[skipIdx] = BooleanVariable::FALSE_EXP();
    m_conditions[startIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[endIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[exitIdx] = BooleanVariable::FALSE_EXP();
    m_conditions[invariantIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[preIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[postIdx] = BooleanVariable::TRUE_EXP();
    m_conditions[repeatIdx] = BooleanVariable::FALSE_EXP();
  }

  void Node::createDeclaredVars(const std::vector<PlexilVarId>& vars) {
    for (std::vector<PlexilVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      const PlexilVarId var = *it;
      // get the variable name
      const std::string& name = (*it)->name();
      LabelStr nameLabel(name);
      VariableId varId =
        (VariableId)
        ExpressionFactory::createInstance(PlexilParser::valueTypeString(var->type()), 
                                          var,
                                          NodeConnector::getId());
      // Check for duplicate names
      // FIXME: push up into XML parser
      assertTrueMsg(m_variablesByName.find(nameLabel.getKey()) == m_variablesByName.end(),
                    "Node \"" << m_nodeId.toString() << "\" already has a variable named \"" << name << "\"");
      m_variablesByName[nameLabel.getKey()] = varId;
      m_localVariables.push_back(varId);
      debugMsg("Node:createDeclaredVars",
               " for node '" << m_nodeId.toString()
               << "': created " 
               << (varId->getValueType() == ARRAY ? "array " : "")
               << "variable " 
               << varId->toString() << " as '"
               << name << "'");
    }
  }

  void Node::getVarsFromInterface(const PlexilInterfaceId& intf)
  {
    check_error(intf.isValid());
    debugMsg("Node:getVarsFromInterface",
             "Getting interface vars for node '" << m_nodeId.toString() << "'");
    assertTrueMsg(m_parent.isId(),
                  "Node \"" << m_nodeId
                  << "\" has an Interface but no parent; may be a library node without a caller.");

    bool parentIsLibCall = m_parent->getType() == LIBRARYNODECALL();

    // Process In variables
    for (std::vector<PlexilVarRef*>::const_iterator it = intf->in().begin();
         it != intf->in().end();
         ++it) {
      PlexilVarRef* varRef = *it;

      // Check for duplicate name
      double nameKey = LabelStr(varRef->name()).getKey();
      assertTrueMsg(m_variablesByName.find(nameKey) == m_variablesByName.end(),
                    "Node \"" << m_nodeId.toString()
                    << ": 'In' variable name \"" << varRef->name() << "\" is already in use");

      VariableId expr = getInVariable(varRef, parentIsLibCall);
      check_error(expr.isValid());

      // make it accessible
      debugMsg("Node:getVarsFromInterface", 
               " for node \"" << m_nodeId.c_str()
               << "\": Adding In variable " << expr->toString()
               << " as \"" << varRef->name() << "\""); 
      m_variablesByName[nameKey] = expr;
    }
      
    for (std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
         it != intf->inOut().end();
         ++it) {
      PlexilVarRef* varRef = *it;

      // Check for duplicate name
      double nameKey = LabelStr(varRef->name()).getKey();
      assertTrueMsg(m_variablesByName.find(nameKey) == m_variablesByName.end(),
                    "Node \"" << m_nodeId.toString()
                    << ": 'InOut' variable name \"" << varRef->name() << "\" is already in use");

      VariableId expr = getInOutVariable(varRef, parentIsLibCall);
      check_error(expr.isValid());
         
      // make it accessible
      debugMsg("Node:getVarsFromInterface", 
               " for node \"" << m_nodeId.c_str()
               << "\": Adding InOut variable " << expr->toString()
               << " as \"" << varRef->name() << "\""); 
      m_variablesByName[nameKey] = expr;
    }
  }

  VariableId Node::getInVariable(const PlexilVarRef* varRef, bool parentIsLibCall)
  {
    // Get the variable from the parent
    // findVariable(..., true) tells LibraryCallNode to only search alias vars
    LabelStr varLabel(varRef->name()); // intern variable name
    VariableId expr = m_parent->findVariable(varLabel, true);
    if (expr.isId()) {
      // Try to avoid constructing alias var
      if (!parentIsLibCall && !expr->isConst()) {
        // Construct const wrapper
        if (expr->isArray()) {
          expr =
            (new ArrayAliasVariable(varLabel.toString(), NodeConnector::getId(), expr, false, true))->getId();
        }
        else {
          expr =
            (new AliasVariable(varLabel.toString(), NodeConnector::getId(), expr, false, true))->getId();
        }
        debugMsg("Node::getInVariable",
                 " Node \"" << m_nodeId.toString()
                 << "\": Constructed const alias wrapper for \"" << varRef->name()
                 << "\" to variable " << *expr);
        m_localVariables.push_back(expr);
      }
    }
    else {
      // FIXME: should we generate default even if not a library call?
      if (parentIsLibCall) {
        // Get default, if any
        PlexilExprId defaultVal = varRef->defaultValue();
        if (defaultVal.isId()) {
          if (Id<PlexilVarRef>::convertable(defaultVal)) {
            // Get a reference to the variable
            expr = findVariable((PlexilVarRef*) defaultVal);
          }
          else {
            // construct constant local "variable" with default value
            bool wasConstructed = false;
            expr = ExpressionFactory::createInstance(LabelStr(PlexilParser::valueTypeString(varRef->type()) + "Value"),
                                                     defaultVal,
                                                     NodeConnector::getId(),
                                                     wasConstructed);
            if (wasConstructed)
              m_localVariables.push_back(expr);
          }
        }
      }

      assertTrueMsg(expr.isId(),
                    "In node \"" << m_nodeId.toString()
                    << "\" 'In' interface: Parent has no "
                    << (parentIsLibCall ? "alias " : "variable ")
                    << "named \"" << varRef->name() << "\""
                    << (parentIsLibCall ? ", and no default value is defined" : ""));
    }
    return expr;
  }

  VariableId Node::getInOutVariable(const PlexilVarRef* varRef, bool parentIsLibCall)
  {
    // Get the variable from the parent
    // findVariable(..., true) tells LibraryCallNode to only search alias vars
    VariableId expr = m_parent->findVariable(LabelStr(varRef->name()), true);
    if (expr.isId()) {
      assertTrueMsg(!expr->isConst(),
                    "In node \"" << m_nodeId.toString()
                    << "\" 'InOut' interface: "
                    << (parentIsLibCall ? "Alias for \"" : "Variable \"")
                    << varRef->name() << "\", "
                    << *expr << ", is read-only");
    }
    else {
      // FIXME: should we generate default even if not a library call?
      if (parentIsLibCall) {
        // Get default, if any
        PlexilExprId defaultVal = varRef->defaultValue();
        if (defaultVal.isId()) {
          if (Id<PlexilVarRef>::convertable(defaultVal)) {
            // Get a reference to the variable
            expr = findVariable((PlexilVarRef*) defaultVal);
          }
          else {
            // construct local "variable" with default value
            bool wasConstructed = false;
            expr = ExpressionFactory::createInstance(LabelStr(PlexilParser::valueTypeString(varRef->type()) + "Variable"),
                                                     defaultVal,
                                                     NodeConnector::getId(),
                                                     wasConstructed);
            if (wasConstructed)
              m_localVariables.push_back(expr);
          }
        }
      }

      assertTrueMsg(expr.isId(),
                    "In node \"" << m_nodeId.toString()
                    << "\" 'InOut' interface: Parent has no "
                    << (parentIsLibCall ? "alias " : "variable ")
                    << "named \"" << varRef->name() << "\""
                    << (parentIsLibCall ? ", and no default value is defined" : ""));
    }
    return expr;
  }

  void Node::postInit(const PlexilNodeId& node) 
  {
    checkError(!m_postInitCalled, "Called postInit on node '" << m_nodeId.toString() << "' twice.");
    m_postInitCalled = true;

    // create assignment/command/update
    specializedPostInit(node);

    // create conditions and listeners
    debugMsg("Node:postInit", "Creating conditions for node '" << m_nodeId.toString() << "'");
    createConditions(node->conditions());

    // late post init (for NodeList)
    specializedPostInitLate(node);
  }

  void Node::createConditions(const std::map<std::string, PlexilExprId>& conds) 
  {
    // Attach listeners to ancestor invariant and ancestor end conditions
    // Root node doesn't need them because the default conditions are constants
    if (m_parent.isId()) {
      ExpressionId ancestorEnd = getAncestorEndCondition();
      assertTrueMsg(ancestorEnd.isId(),
                    "Internal error: ancestor end condition is null!");
      ancestorEnd->addListener(makeConditionListener(ancestorEndIdx));

      ExpressionId ancestorExit = getAncestorExitCondition();
      assertTrueMsg(ancestorExit.isId(),
                    "Internal error: ancestor exit condition is null!");
      ancestorExit->addListener(makeConditionListener(ancestorExitIdx));

      ExpressionId ancestorInvariant = getAncestorInvariantCondition();
      assertTrueMsg(ancestorInvariant.isId(),
                    "Internal error: ancestor invariant condition is null!");
      ancestorInvariant->addListener(makeConditionListener(ancestorInvariantIdx));
    }

    // Let the derived class do its thing (currently only ListNode)
    createSpecializedConditions();

    // Add user-specified conditions
    for (std::map<std::string, PlexilExprId>::const_iterator it = conds.begin(); 
         it != conds.end(); 
         ++it) {
      const LabelStr condName(it->first);
      size_t condIdx = getConditionIndex(condName);

      // Delete existing condition if required
      // (e.g. explicit override of default end condition for list or library call node)
      if (m_listeners[condIdx].isId())
        m_conditions[condIdx]->removeListener(m_listeners[condIdx]);
      if (m_garbageConditions[condIdx]) {
        delete (Expression*) m_conditions[condIdx];
        m_garbageConditions[condIdx] = false;
      }

      if (m_listeners[condIdx].isNoId())
        makeConditionListener(condIdx); // for effect
      m_conditions[condIdx] = 
        ExpressionFactory::createInstance(it->second->name(), 
                                          it->second,
                                          NodeConnector::getId(), 
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

  // Default method
  void Node::specializedPostInit(const PlexilNodeId& node)
  {
  }

  // Default method
  void Node::specializedPostInitLate(const PlexilNodeId& node)
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
    m_id.removeDerived(NodeConnector::getId());
  }

  void Node::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

    debugMsg("Node:cleanUpConditions", " for " << m_nodeId.toString());

    // Clean up condition listeners
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (m_listeners[i].isId()) {
        debugMsg("Node:cleanUpConditions",
                 "<" << m_nodeId.toString() << "> Removing condition listener for " <<
                 getConditionName(i).toString());
        ExpressionId cond = getCondition(i);
        if (cond.isId())
          cond->removeListener(m_listeners[i]);
        delete (ExpressionListener*) m_listeners[i];
        m_listeners[i] = ExpressionListenerId::noId();
      }
    }

    // Clean up conditions
    // N.B.: Ancestor-end and ancestor-invariant MUST be cleaned up before
    // end and invariant, respectively. 
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (m_garbageConditions[i]) {
        debugMsg("Node:cleanUpConds",
                 "<" << m_nodeId.toString() << "> Removing condition " << getConditionName(i).toString());
        delete (Expression*) m_conditions[i];
      }
      m_conditions[i] = ExpressionId::noId();
      m_garbageConditions[i] = false;
    }

    m_cleanedConditions = true;
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
               "<" << m_nodeId.toString() << "> Removing " << **it);
      delete (Variable*) (*it);
    }
    m_localVariables.clear();

    // Delete timepoint variables
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; ++s) {
      delete (Variable*) m_startTimepoints[s];
      delete (Variable*) m_endTimepoints[s];
      m_startTimepoints[s] = m_endTimepoints[s] = VariableId::noId();
    }

    // Delete internal variables
    delete (Variable*) m_outcomeVariable;
    m_outcomeVariable = VariableId::noId();
    delete (Variable*) m_failureTypeVariable;
    m_failureTypeVariable = VariableId::noId();
    delete (Variable*) m_stateVariable;
    m_stateVariable = VariableId::noId();

    m_cleanedVars = true;
  }

  // Make the node (and its children, if any) active.
  void Node::activate()
  {
    // Activate internal variables
    activateInternalVariables();

    // Activate conditions needed for INACTIVE state
    transitionToInactive();

    // Other initializations as required by node type
    specializedActivate();
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
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; ++s) {
      m_startTimepoints[s]->activate();
      m_endTimepoints[s]->activate();
    }

    specializedActivateInternalVariables();
  }

  // Default method
  void Node::specializedActivateInternalVariables()
  {
  }

  // Default method
  void Node::specializedActivate()
  {
  }

  const ExpressionId& Node::getCondition(size_t idx) const
  {
    switch (idx) {

    case ancestorEndIdx:
    case ancestorExitIdx:

      if (m_parent.isId())
        return m_parent->m_conditions[idx];
      else
        return BooleanVariable::FALSE_EXP();

    case ancestorInvariantIdx:

      if (m_parent.isId())
        return m_parent->m_conditions[idx];
      else
        return BooleanVariable::TRUE_EXP();

    default:
      return m_conditions[idx];
    }
  }

  const ExpressionId& Node::getCondition(const LabelStr& name) const {
    return getCondition(getConditionIndex(name));
  }

  ExpressionListenerId Node::makeConditionListener(size_t idx)
  {
    assertTrueMsg(m_listeners[idx].isNoId(),
                  "Node::makeConditionListener: Node " << m_nodeId.toString()
                  << " already has a listener for condition " << getConditionName(idx));
    return m_listeners[idx] = 
      (new ConditionChangeListener(*this, idx))->getId();
  }

  // Default method.
  const std::vector<NodeId>& Node::getChildren() const
  {
    static std::vector<NodeId> sl_emptyNodeVec;
    return sl_emptyNodeVec;
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

    debugMsg("Node:checkConditions",
             "Checking condition change for node " << m_nodeId.toString());
    NodeState toState(getDestState());
    debugMsg("Node:checkConditions",
             "Can (possibly) transition to " << StateVariable::nodeStateName(toState).toString());
    if (toState != m_lastQuery) {
      m_exec->handleConditionsChanged(m_id, toState);
      m_lastQuery = toState;
    }
  }

  NodeState Node::getDestState() 
  {
    debugMsg("Node:getDestState",
             "Getting destination state for " << m_nodeId.toString() << " from state " <<
             getStateName().toString());

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
  // State transition logic
  //

  // This method is used only by exec-test-module.
  // Its logic has been absorbed into transition() below to avoid redundant calls to getDestState().
  bool Node::canTransition()
  {
    NodeState toState = getDestState();
    return toState != NO_NODE_STATE && toState != m_state;
  }

  void Node::transition(NodeState destState, const double time) 
  {
    checkError(destState != NO_NODE_STATE
               && destState != m_state,
               "Attempted to transition node " << m_nodeId.toString() <<
               " when it is ineligible.");
    checkError(m_stateVariable->getValue() == StateVariable::nodeStateName(m_state).getKey(),
               "Node state not synchronized for node " << m_nodeId.toString()
               << "; node state = " << m_state
               << ", node state name = \"" << Expression::valueToString(m_stateVariable->getValue()) << "\"");

    NodeState prevState = m_state;
    
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
             "Setting '" << m_nodeId.toString()
             << "' end time " << LabelStr(END_TIMEPOINT_NAMES()[prevState]).toString()
             << " = start time " << LabelStr(START_TIMEPOINT_NAMES()[destState]).toString()
             << " = " << std::setprecision(15) << time);
    m_endTimepoints[prevState]->setValue(time);
    m_startTimepoints[destState]->setValue(time);
    conditionChanged(); // was checkConditions();
  }

  // Common method 
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

  // Common method 
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
    if (destState == EXECUTING_STATE)
      execute();
  }

  //
  // INACTIVE
  //
  // Description and methods here apply to ALL nodes
  //
  // Start state
  // Legal predecessor states: FINISHED
  // Conditions active:
  // Legal successor states: WAITING, FINISHED

  // Default method
  void Node::transitionToInactive()
  {
  }

  // Default method
  NodeState Node::getDestStateFromInactive()
  {
    if (m_parent.isId()) {
      switch (m_parent->getState()) {

      case FINISHED_STATE:
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: FINISHED. Parent state == FINISHED.");
        return FINISHED_STATE;

      case EXECUTING_STATE:
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: WAITING. Parent state == EXECUTING.");
        return WAITING_STATE;

      default:
        debugMsg("Node:getDestState", 
                 " '" << m_nodeId.toString() << "' destination: no state.");
        return NO_NODE_STATE;
      }
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: WAITING. Root node.");
      return WAITING_STATE;
    }
  }

  // Default method
  void Node::transitionFromInactive(NodeState destState)
  {
    checkError(destState == WAITING_STATE || destState == FINISHED_STATE,
               "Attempting to transition from INACTIVE to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");
    if (destState == FINISHED_STATE) {
      getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
    }
    else { // WAITING
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
    }
  }

  //
  // WAITING
  //
  // Description and methods here apply to ALL nodes.
  //
  // Legal predecessor states: INACTIVE, ITERATION_ENDED
  // Conditions active: AncestorEnd, AncestorExit, AncestorInvariant, Exit, Pre, Skip, Start
  // Legal successor states: EXECUTING, FINISHED, ITERATION_ENDED

  // ** N.B. Preceding state must ensure that AncestorEnd, AncestorExit, and AncestorInvariant are active.

  // Default method
  void Node::transitionToWaiting()
  {
    activateExitCondition();
    activatePreCondition();
    activateSkipCondition();
    activateStartCondition();
  }

  // Default method
  NodeState Node::getDestStateFromWaiting()
  {
    checkError(isAncestorExitConditionActive(),
               "Node::getDestStateFromWaiting: Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isExitConditionActive(),
               "Node::getDestStateFromWaiting: Exit condition for " << m_nodeId.toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Node::getDestStateFromWaiting: Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_INVARIANT_CONDITION false.");
      return FINISHED_STATE;
    }

    checkError(isAncestorEndConditionActive(),
               "Node::getDestStateFromWaiting: Ancestor end for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_END_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isSkipConditionActive(), 
               "Node::getDestStateFromWaiting: Skip for " << m_nodeId.toString() << " is inactive.");
    if (getSkipCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. SKIP_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isStartConditionActive(), 
               "Node::getDestStateFromWaiting: Start for " << m_nodeId.toString() << " is inactive.");
    if (getStartCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      checkError(isPreConditionActive(), 
                 "Node::getDestStateFromWaiting: Pre for " << m_nodeId.toString() << " is inactive.");
      if (getPreCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: EXECUTING. START_CONDITION and PRE_CONDITION are both true.");
        return EXECUTING_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
        return ITERATION_ENDED_STATE;
      }
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state. START_CONDITION false or unknown");
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromWaiting(NodeState destState)
  {
    checkError(destState == FINISHED_STATE ||
               destState == EXECUTING_STATE ||
               destState == ITERATION_ENDED_STATE,
               "Attempting to transition from WAITING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    deactivatePreCondition();
    deactivateSkipCondition();
    deactivateStartCondition();

    if (destState == FINISHED_STATE) {
      deactivateAncestorEndCondition();
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
      deactivateExitCondition();
      getOutcomeVariable()->setValue(OutcomeVariable::SKIPPED());
    }
    else if (destState == ITERATION_ENDED_STATE) {
      deactivateExitCondition();
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::PRE_CONDITION_FAILED());
    }
    else { // EXECUTING
      deactivateAncestorEndCondition();
    }
  }

  //
  // EXECUTING 
  // 
  // Description and methods here are for Empty node only
  //
  // Legal predecessor states: WAITING
  // Conditions active: AncestorExit, AncestorInvariant, End, Exit, Invariant, Post
  // Legal successor states: FINISHED, ITERATION_ENDED

  // Default method
  void Node::transitionToExecuting()
  {
    activateInvariantCondition();
    activateEndCondition();
    activatePostCondition();
  }

  // Default method
  NodeState Node::getDestStateFromExecuting()
  {
    checkError(isAncestorExitConditionActive(),
               "Node::getDestStateFromExecuting: Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isExitConditionActive(),
               "Node::getDestStateFromExecuting: Exit condition for " << m_nodeId.toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. EXIT_CONDITION true.");
      return ITERATION_ENDED_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Node::getDestStateFromExecuting: Ancestor invariant for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. Ancestor invariant false.");
      return FINISHED_STATE;
    }

    checkError(isInvariantConditionActive(),
               "Node::getDestStateFromExecuting: Invariant for " << m_nodeId.toString() << " is inactive.");
    if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. Invariant false.");
      return ITERATION_ENDED_STATE;
    }

    checkError(isEndConditionActive(),
               "Node::getDestStateFromExecuting: End for " << m_nodeId.toString() << " is inactive.");
    if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. End condition true.");
      return ITERATION_ENDED_STATE;
    }

    return NO_NODE_STATE;
  }

  // Empty node method
  void Node::transitionFromExecuting(NodeState destState)
  {
    checkError(m_nodeType == Node::EMPTY(),
               "Expected empty node, got " << m_nodeType.toString());
    checkError(destState == FINISHED_STATE || destState == ITERATION_ENDED_STATE,
               "Attempting to transition from EXECUTING to invalid state '"
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
    // Below here, we know EndCondition is true
    else if (getPostCondition()->getValue() == BooleanVariable::TRUE_VALUE())
      getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
    else {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
    }

    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();
    if (destState == FINISHED_STATE) {
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
    }
    else { // ITERATION_ENDED
      activateAncestorEndCondition();
    }

    deactivateExecutable(); // ??
  }

  //
  // ITERATION_ENDED
  //
  // Description and methods here apply to ALL nodes
  //
  // Legal predecessor states: EXECUTING, FAILING, FINISHING, WAITING
  // Conditions active: AncestorEnd, AncestorExit, AncestorInvariant, Repeat
  // Legal successor states: FINISHED, WAITING

  // *** N.B.: Preceding state must ensure that AncestorEnd, AncestorExit, and AncestorInvariant are active!

  // Default method
  void Node::transitionToIterationEnded() 
  {
    activateRepeatCondition();
  }

  // Default method
  NodeState Node::getDestStateFromIterationEnded()
  {
    checkError(isAncestorExitConditionActive(),
               "Node::getDestStateFromIterationEnded: Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Node::getDestStateFromIterationEnded: Ancestor invariant for " << m_nodeId.toString() << " is inactive.");

    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_INVARIANT false.");
      return FINISHED_STATE;
    }

    checkError(isAncestorEndConditionActive(),
               "Node::getDestStateFromIterationEnded: Ancestor end for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. ANCESTOR_END true.");
      return FINISHED_STATE;
    }

    checkError(isRepeatConditionActive(),
               "Node::getDestStateFromIterationEnded: Repeat for " << m_nodeId.toString() << " is inactive.");
    double repeat = getRepeatCondition()->getValue();
    if (repeat == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHED. REPEAT_CONDITION false.");
      return FINISHED_STATE;
    }
    else if (repeat == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: WAITING. REPEAT_UNTIL true.");
      return WAITING_STATE;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state. ANCESTOR_END false or unknown and REPEAT unknown.");
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromIterationEnded(NodeState destState)
  {
    checkError(destState == FINISHED_STATE || destState == WAITING_STATE,
               "Attempting to transition from ITERATION_ENDED to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
    }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
    }

    deactivateRepeatCondition();

    if (destState == FINISHED_STATE) {
      deactivateAncestorEndCondition();
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
    }
    else { // WAITING
      reset();
    }
  }

  //
  // FINISHED
  //
  // Description and methods here apply to ALL nodes
  //
  // Legal predecessor states: EXECUTING, FAILING, FINISHING, INACTIVE, ITERATION_ENDED, WAITING
  // Conditions active:
  // Legal successor states: INACTIVE

  // Default method
  void Node::transitionToFinished()
  {
  }

  // Default method
  NodeState Node::getDestStateFromFinished()
  {
    if (m_parent.isId() && m_parent->getState() == WAITING_STATE) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: INACTIVE.  Parent state == WAITING.");
      return INACTIVE_STATE;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state.");
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFinished(NodeState destState)
  {
    checkError(destState == INACTIVE_STATE,
               "Attempting to transition from FINISHED to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");
    reset();
  }

  //
  // FINISHING (legal for ListNode and LibraryCallNode only)
  //
  // Description and methods here apply to all other node types.
  //
  // Legal predecessor states: n/a
  // Conditions active: n/a
  // Legal successor states: n/a

  // Default method
  void Node::transitionToFinishing()
  {
    checkError(ALWAYS_FAIL,
               "No transition to FINISHING state defined for this node");
  }

  // Default method
  NodeState Node::getDestStateFromFinishing()
  {
    checkError(ALWAYS_FAIL,
               "Attempted to compute destination state from FINISHING for node " << m_nodeId.toString()
               << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFinishing(NodeState destState)
  {
    checkError(ALWAYS_FAIL,
               "No transition from FINISHING state defined for this node");
  }

  //
  // FAILING (legal for Command, Update, ListNode, and LibraryCallNode only)
  //
  // Description and methods here apply to Empty and Assignment nodes.
  //
  // Legal predecessor states: n/a
  // Conditions active: n/a
  // Legal successor states: n/a

  // Default method
  void Node::transitionToFailing()
  {
    checkError(ALWAYS_FAIL,
               "No transition to FAILING state defined for this node");
  }

  // Default method
  NodeState Node::getDestStateFromFailing()
  {
    checkError(ALWAYS_FAIL,
               "Attempted to compute destination state from FAILING for node " << m_nodeId.toString()
               << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFailing(NodeState destState)
  {
    checkError(ALWAYS_FAIL,
               "No transition from FAILING state defined for this node");
  }

  // ***
  // *** END NODE STATE LOGIC ***
  // ***

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

  const LabelStr Node::getOutcome() 
  {
    double key = m_outcomeVariable->getValue();
    if (key != Expression::UNKNOWN())
      return LabelStr(key);
    else {
      return LabelStr("UNKNOWN");
    }
  }

  const LabelStr Node::getFailureType() {
    double key = m_failureTypeVariable->getValue();
    if (key != Expression::UNKNOWN())
      return LabelStr(key);
    else {
      return LabelStr("UNKNOWN");
    }
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

      case PlexilNodeRef::GRANDPARENT:
        {
          node = m_parent;
          for (int i = 1; i < nodeRef->generation() && node.isValid(); ++i) {
            node = node->getParent();
          }
          // FIXME: push this check up into XML parser?
          checkError(node.isValid(),
                     "Grandparent node reference above root node from " << 
                     m_nodeId.toString());
          checkError(nodeRef->name() == node->getNodeId().toString(),
                     "Grandparent node reference for '" << nodeRef->name()
                     << "' found node '" << node->getNodeId().toString() << "' instead");
          break;
        }

      case PlexilNodeRef::UNCLE:
        {
          NodeId ancestor = m_parent;
          for (int i = 1; i < nodeRef->generation() && ancestor.isValid(); ++i) {
            ancestor = ancestor->getParent();
          }
          // FIXME: push this check up into XML parser?
          checkError(ancestor.isValid(),
                     "Grandparent node reference above root node from " << 
                     m_nodeId.toString());
          node = ancestor->findChild(LabelStr(nodeRef->name()));
          // FIXME: push this check up into XML parser
          checkError(node.isId(),
                     "No uncle named '" << nodeRef->name() << 
                     "' of " << m_nodeId.toString());
          break;
        }

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

  void Node::activatePair(size_t idx) 
  {
    // checkError(idx < conditionIndexMax, "Invalid condition index " << idx);
    checkError(getCondition(idx).isId(),
               "No condition exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:activatePair",
             "Activating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    if (m_listeners[idx].isId())
      m_listeners[idx]->activate();
    getCondition(idx)->activate();
  }

  void Node::deactivatePair(size_t idx) 
  {
    // checkError(idx < conditionIndexMax, "Invalid condition index " << idx);
    checkError(getCondition(idx).isId(),
               "No condition exists for '" << getConditionName(idx).toString() << "'");
    debugMsg("Node:deactivatePair",
             "Deactivating '" << getConditionName(idx).toString() << "' in node '" << m_nodeId.toString() << "'");
    getCondition(idx)->deactivate();
    if (m_listeners[idx].isId() && m_listeners[idx]->isActive())
      m_listeners[idx]->deactivate();
  }

  bool Node::pairActive(size_t idx) 
  {
    // checkError(idx < conditionIndexMax, "Invalid condition index " << idx);

    ExpressionId condition = getCondition(idx);
    checkError(condition.isId(),
               "Node::pairActive: Node " << m_nodeId.toString()
               << " has no condition for " << getConditionName(idx).toString());
    if (!condition->isActive())
      return false;

    // N.B. Root nodes will not have listeners on parent conditions,
    // which are constants and thus cannot change.
    if (m_listeners[idx].isNoId())
      return true;

    return m_listeners[idx]->isActive();
  }

  void Node::execute() 
  {
    debugMsg("Node:execute", "Executing node " << m_nodeId.toString());
    // activate local variables
    for (std::vector<VariableId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         ++vit) {
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
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; ++s) {
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

  // Optimized version
  // N.B. we omit the validity check on the condition expression
  // because this is a critical path method in the inner loop of the Exec.
  void Node::lockConditions() 
  {
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (m_listeners[i].isId()
          && m_listeners[i]->isActive()) {
        ExpressionId expr = getCondition(i);
        checkError(expr.isId(),
                   "Node::lockConditions: condition " << getConditionName(i).toString()
                   << " is null in node " << m_nodeId.toString());
        if (expr->isActive() && !expr->isLocked()) {
          debugMsg("Node:lockConditions",
                   "In " << m_nodeId.toString() << ", locking " <<
                   getConditionName(i).toString() << " " << expr->toString());
          expr->lock();
        }
      }
    }
  }

  // As above, skip the Id validity check because this is a critical path function.
  void Node::unlockConditions() 
  {
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = getCondition(i);
      if (expr.isId() && expr->isLocked()) {
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
         ++vit)
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
      printCommandHandle(stream, indent);
      printVariables(stream, indent);
    }
    else if (m_state != INACTIVE_STATE) {
      // Print conditions
      for (size_t i = 0; i < conditionIndexMax; ++i) {
        if (getCondition(i).isId()) {
          stream << indentStr << " " << getConditionName(i).toString() << ": " <<
            getCondition(i)->toString() << '\n';
        }
      }
      // Print variables, starting with command handle
      printCommandHandle(stream, indent);
      printVariables(stream, indent);
    }
    // print children
    for (std::vector<NodeId>::const_iterator it = getChildren().begin(); it != getChildren().end(); ++it) {
      stream << (*it)->toString(indent + 2);
    }
    stream << indentStr << "}" << std::endl;
  }

  // Default method does nothing
  void Node::printCommandHandle(std::ostream& stream, const unsigned int indent) const
  {
  }

  // Print variables
  void Node::printVariables(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
    ensureSortedVariableNames(); // for effect
    for (std::vector<double>::const_iterator it = m_sortedVariableNames->begin();
         it != m_sortedVariableNames->end();
         ++it) {
      stream << indentStr << " " << LabelStr(*it).toString() << ": " <<
        *(getInternalVariable(LabelStr(*it))) << '\n';
    }
  }

  const ExecListenerHubId& Node::getExecListenerHub() const
  {
    if (m_exec.isNoId()) {
      static ExecListenerHubId sl_hubNoId;
      return sl_hubNoId;
    }
    else
      return m_exec->getExecListenerHub();
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
           ++it) {
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
      startNames->reserve(NO_NODE_STATE);
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
      endNames->reserve(NO_NODE_STATE);
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
