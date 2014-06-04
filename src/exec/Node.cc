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

#include "Node.hh"

#include "Alias.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
// #include "Expressions.hh"
#include "ExpressionConstants.hh"
#include "ExpressionFactory.hh"
#include "ExternalInterface.hh"
#include "NodeConstants.hh"
#include "NodeFactory.hh"
#include "PlexilExec.hh"
#include "UserVariable.hh"
#include "XMLUtils.hh"
#include "lifecycle-utils.h"

#include <algorithm> // for sort
#include <cfloat>    // for DBL_MAX
#include <iomanip>   // for setprecision (NOT CURRENTLY USED)
#include <sstream>

namespace PLEXIL {

  // Initialize class static variables
  std::vector<std::string>* Node::s_allConditions = NULL;

  const std::vector<std::string>& Node::ALL_CONDITIONS() {
    static bool sl_inited = false;
    if (!sl_inited) {
      s_allConditions = new std::vector<std::string>();
      addFinalizer(&purgeAllConditions);
      s_allConditions->reserve(conditionIndexMax);

      // *** N.B.: Order MUST agree with enum ConditionIndex!
      // Conditions on parent
      s_allConditions->push_back(ANCESTOR_END_CONDITION());
      s_allConditions->push_back(ANCESTOR_EXIT_CONDITION());
      s_allConditions->push_back(ANCESTOR_INVARIANT_CONDITION());
      // User specified conditions
      s_allConditions->push_back(SKIP_CONDITION());
      s_allConditions->push_back(START_CONDITION());
      s_allConditions->push_back(END_CONDITION());
      s_allConditions->push_back(EXIT_CONDITION());
      s_allConditions->push_back(INVARIANT_CONDITION());
      s_allConditions->push_back(PRE_CONDITION());
      s_allConditions->push_back(POST_CONDITION());
      s_allConditions->push_back(REPEAT_CONDITION());
      // For all but Empty nodes
      s_allConditions->push_back(ACTION_COMPLETE());
      // For all but Empty and Update nodes
      s_allConditions->push_back(ABORT_COMPLETE());
      sl_inited = true;

      // inexpensive sanity check
      assertTrue(s_allConditions->size() == conditionIndexMax,
                 "INTERNAL ERROR: Inconsistency between conditionIndex enum and ALL_CONDITIONS");
    }
    return *s_allConditions;
  }

  void Node::purgeAllConditions()
  {
    delete s_allConditions;
    s_allConditions = NULL;
  }

  std::string (&Node::START_TIMEPOINT_NAMES())[NO_NODE_STATE] 
  {
    static std::string sl_startTimepointNames[NO_NODE_STATE];
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&purgeStartTimepointNames);
      for (int i = INACTIVE_STATE; i < NO_NODE_STATE; ++i) {
        const std::string& state = nodeStateName(i);
        sl_startTimepointNames[i] = std::string(state + ".START");
      }
      sl_inited = true;
    }
    return sl_startTimepointNames;
  }

  void Node::purgeStartTimepointNames()
  {
    for (int i = INACTIVE_STATE; i < NO_NODE_STATE; ++i)
      START_TIMEPOINT_NAMES()[i] = "";
  }

  std::string (&Node::END_TIMEPOINT_NAMES())[NO_NODE_STATE]
  {
    static std::string sl_endTimepointNames[NO_NODE_STATE];
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&purgeEndTimepointNames);
      for (size_t i = INACTIVE_STATE; i < NO_NODE_STATE; ++i) {
        const std::string& state = nodeStateName(i);
        sl_endTimepointNames[i] = std::string(state + ".END");
      }
      sl_inited = true;
    }
    return sl_endTimepointNames;
  }

  void Node::purgeEndTimepointNames()
  {
    for (size_t i = INACTIVE_STATE; i < NO_NODE_STATE; ++i)
      END_TIMEPOINT_NAMES()[i] = "";
  }

  size_t Node::getConditionIndex(const std::string& cName) {
    const std::vector<std::string>& allConds = ALL_CONDITIONS();
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (allConds[i] == cName)
        return i;
    }
    assertTrueMsg(ALWAYS_FAIL,
                  cName << " is not a valid condition name");
    return conditionIndexMax; // make compiler happy
  }

  const std::string& Node::getConditionName(size_t idx)
  {
    return ALL_CONDITIONS()[idx];
  }

  const std::string& 
  Node::nodeTypeToString(PlexilNodeType nodeType)
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
    static std::string sl_empty;
    return sl_empty;
  }

  Node::Node(const PlexilNodeId& node, const NodeId& parent)
    : NodeConnector(),
      m_id(this, NodeConnector::getId()),
      m_parent(parent),
      m_listener(*this),
      m_nodeId(node->nodeId()),
      m_nodeType(nodeTypeToString(node->nodeType())), // Can throw exception
      m_sortedVariableNames(new std::vector<std::string>()),
      m_stateVariable(*this),
      m_outcomeVariable(*this),
      m_failureTypeVariable(*this),
      m_state(INACTIVE_STATE),
      m_lastQuery(NO_NODE_STATE),
      m_outcome(NO_OUTCOME),
      m_failureType(NO_FAILURE),
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
  Node::Node(const std::string& type, 
             const std::string& name, 
             const NodeState state,
             const NodeId& parent)
    : NodeConnector(),
      m_id(this, NodeConnector::getId()),
      m_parent(parent),
      m_listener(*this),
      m_nodeId(name),
      m_nodeType(type),
      m_sortedVariableNames(new std::vector<std::string>()),
      m_stateVariable(*this),
      m_outcomeVariable(*this),
      m_failureTypeVariable(*this),
      m_state(state),
      m_lastQuery(NO_NODE_STATE),
      m_outcome(NO_OUTCOME),
      m_failureType(NO_FAILURE),
      m_postInitCalled(false), 
      m_cleanedConditions(false), 
      m_cleanedVars(false),
      m_checkConditionsPending(false)
  {
    commonInit();

    for (size_t i = 0; i < conditionIndexMax; ++i) {
      ExpressionId expr = (new BooleanVariable(false))->getId();
      debugMsg("Node:node",
               " Created internal variable "
               << ALL_CONDITIONS()[i] <<
               " with value FALSE for node " << m_nodeId);
      m_conditions[i] = expr;
      m_garbageConditions[i] = true;
      if (i != preIdx && i != postIdx)
        getCondition(i)->addListener(m_listener.getId());
    }

    // Activate the conditions required by the provided state
    switch (m_state) {

    case INACTIVE_STATE:
      break;

    case WAITING_STATE:
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
      activateExitCondition();
      activateInvariantCondition();
      activatePreSkipStartConditions();
      break;

    case EXECUTING_STATE:
      activateAncestorExitInvariantConditions();
      activateEndCondition();
      activateExitCondition();
      activateInvariantCondition();
      activatePostCondition();
      break;

    case FAILING_STATE:
      assertTrueMsg(m_nodeType != EMPTY(),
                    "Node module test constructor: FAILING state invalid for " << m_nodeType << " nodes");
      // Defer to subclass
      break;

    case FINISHING_STATE:
      assertTrueMsg(m_nodeType != EMPTY(),
                    "Node module test constructor: FINISHING state invalid for " << m_nodeType << " nodes");
      // Defer to subclass
      break;

    case ITERATION_ENDED_STATE:
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
      activateRepeatCondition();
      break;

    case FINISHED_STATE:
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL, "Node module test constructor: Invalid state " << state);
    }
  }

  // N.B.: called from base class constructor
  void Node::commonInit() {
    debugMsg("Node:node", "Instantiating internal variables...");
    // Register state/outcome/failure variables
    m_variablesByName[STATE()] = m_stateVariable.getId();
    m_variablesByName[OUTCOME()] = m_outcomeVariable.getId();
    m_variablesByName[FAILURE_TYPE()] = m_failureTypeVariable.getId();

    // initialize m_garbageConditions
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      m_garbageConditions[i] = false;
    }
  }

  // instantiate timepoint variables, but only for states node can actually reach
  // N.B.: can't call this from base class constructor.
  void Node::constructTimepointVariables()
  {
    debugMsg("Node:node", "Instantiating timepoint variables.");
    for (int s = INACTIVE_STATE; s <= nodeStateMax(); ++s) {
      ExpressionId stp = (new RealVariable())->getId();
      m_startTimepoints[s] = m_variablesByName[START_TIMEPOINT_NAMES()[s]] = stp;

      ExpressionId etp = (new RealVariable())->getId();
      m_endTimepoints[s] = m_variablesByName[END_TIMEPOINT_NAMES()[s]] = etp;
    }
  }

  // Use existing Boolean constants for the condition defaults
  void Node::setConditionDefaults() 
  {
    // These may be user-specified
    // End condition will be overridden 
    m_conditions[skipIdx] = FALSE_EXP();
    m_conditions[startIdx] = TRUE_EXP();
    m_conditions[endIdx] = TRUE_EXP();
    m_conditions[exitIdx] = FALSE_EXP();
    m_conditions[invariantIdx] = TRUE_EXP();
    m_conditions[preIdx] = TRUE_EXP();
    m_conditions[postIdx] = TRUE_EXP();
    m_conditions[repeatIdx] = FALSE_EXP();
  }

  void Node::createDeclaredVars(const std::vector<PlexilVarId>& vars) {
    for (std::vector<PlexilVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      const PlexilVarId var = *it;
      // get the variable name
      const std::string& name = (*it)->name();
      std::string nameLabel(name);
      // Check for duplicate names
      // FIXME: push up into XML parser
      assertTrueMsg(m_variablesByName.find(nameLabel) == m_variablesByName.end(),
                    "Node \"" << m_nodeId << "\" already has a variable named \"" << name << "\"");
      AssignableId varId =
        (AssignableId)
        ExpressionFactory::createInstance(var->factoryTypeString(), 
                                          var,
                                          NodeConnector::getId());
      m_variablesByName[varId->getName()] = varId;
      m_localVariables.push_back(varId);
      debugMsg("Node:createDeclaredVars",
               " for node '" << m_nodeId
               << "': created variable " 
               << varId->toString() << " as '"
               << name << "'");
    }
  }

  void Node::getVarsFromInterface(const PlexilInterfaceId& intf)
  {
    check_error(intf.isValid());
    debugMsg("Node:getVarsFromInterface",
             "Getting interface vars for node '" << m_nodeId << "'");
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
      std::string name(varRef->name());
      assertTrueMsg(m_variablesByName.find(name) == m_variablesByName.end(),
                    "Node \"" << m_nodeId
                    << ": 'In' variable name \"" << varRef->name() << "\" is already in use");

      ExpressionId expr = getInVariable(varRef, parentIsLibCall);
      check_error(expr.isValid());

      // make it accessible
      debugMsg("Node:getVarsFromInterface", 
               " for node \"" << m_nodeId.c_str()
               << "\": Adding In variable " << expr->toString()
               << " as \"" << varRef->name() << "\""); 
      m_variablesByName[name] = expr;
    }
      
    for (std::vector<PlexilVarRef*>::const_iterator it = intf->inOut().begin();
         it != intf->inOut().end();
         ++it) {
      PlexilVarRef* varRef = *it;

      // Check for duplicate name
      std::string name(varRef->name());
      assertTrueMsg(m_variablesByName.find(name) == m_variablesByName.end(),
                    "Node \"" << m_nodeId
                    << ": 'InOut' variable name \"" << varRef->name() << "\" is already in use");

      AssignableId expr = getInOutVariable(varRef, parentIsLibCall);
      check_error(expr.isValid());
         
      // make it accessible
      debugMsg("Node:getVarsFromInterface", 
               " for node \"" << m_nodeId.c_str()
               << "\": Adding InOut variable " << expr->toString()
               << " as \"" << varRef->name() << "\""); 
      m_variablesByName[name] = expr;
    }
  }

  ExpressionId Node::getInVariable(const PlexilVarRef* varRef, bool parentIsLibCall)
  {
    // Get the variable from the parent
    // findVariable(..., true) tells LibraryCallNode to only search alias vars
    std::string varLabel(varRef->name());
    ExpressionId expr = m_parent->findVariable(varLabel, true);
    if (expr.isId()) {
      // Try to avoid constructing alias var
      if (!parentIsLibCall && expr->isAssignable()) {
        // Construct const wrapper
        expr = (new Alias(getId(), varRef->name(), expr, false))->getId();
        debugMsg("Node::getInVariable",
                 " Node \"" << m_nodeId
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
            // FIXME: generates temporary string when table lookup could be used
            expr = ExpressionFactory::createInstance(PlexilParser::valueTypeString(varRef->type()) + "Value",
                                                     defaultVal,
                                                     NodeConnector::getId(),
                                                     wasConstructed);
            if (wasConstructed)
              m_localVariables.push_back(expr);
          }
        }
      }

      assertTrueMsg(expr.isId(),
                    "In node \"" << m_nodeId
                    << "\" 'In' interface: Parent has no "
                    << (parentIsLibCall ? "alias " : "variable ")
                    << "named \"" << varRef->name() << "\""
                    << (parentIsLibCall ? ", and no default value is defined" : ""));
    }
    return expr;
  }

  AssignableId Node::getInOutVariable(const PlexilVarRef* varRef, bool parentIsLibCall)
  {
    // Get the variable from the parent
    // findVariable(..., true) tells LibraryCallNode to only search alias vars
    ExpressionId expr = m_parent->findVariable(std::string(varRef->name()), true);
    if (expr.isId()) {
      assertTrueMsg(expr->isAssignable(),
                    "In node \"" << m_nodeId
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
            // FIXME: generates temporary string when table lookup could be used
            expr = ExpressionFactory::createInstance(PlexilParser::valueTypeString(varRef->type()) + "Variable",
                                                     defaultVal,
                                                     NodeConnector::getId(),
                                                     wasConstructed);
            if (wasConstructed)
              m_localVariables.push_back(expr);
          }
        }
      }

      assertTrueMsg(expr.isId(),
                    "In node \"" << m_nodeId
                    << "\" 'InOut' interface: Parent has no "
                    << (parentIsLibCall ? "alias " : "variable ")
                    << "named \"" << varRef->name() << "\""
                    << (parentIsLibCall ? ", and no default value is defined" : ""));
    }
    return expr;
  }

  void Node::postInit(const PlexilNodeId& node) 
  {
    checkError(!m_postInitCalled, "Called postInit on node '" << m_nodeId << "' twice.");
    m_postInitCalled = true;
    
    // create assignment/command/update
    specializedPostInit(node);

    // create conditions
    debugMsg("Node:postInit", "Creating conditions for node '" << m_nodeId << "'");
    createConditions(node->conditions());

    // late post init (for NodeList)
    specializedPostInitLate(node);
  }

  void Node::createConditions(const std::vector<std::pair<PlexilExprId, std::string> >& conds) 
  {
    // Attach listeners to ancestor invariant and ancestor end conditions
    // Root node doesn't need them because the default conditions are constants
    if (m_parent.isId()) {
      ExpressionId ancestorEnd = getAncestorEndCondition();
      assertTrueMsg(ancestorEnd.isId(),
                    "Internal error: ancestor end condition is null!");
      ancestorEnd->addListener(m_listener.getId());

      ExpressionId ancestorExit = getAncestorExitCondition();
      assertTrueMsg(ancestorExit.isId(),
                    "Internal error: ancestor exit condition is null!");
      ancestorExit->addListener(m_listener.getId());

      ExpressionId ancestorInvariant = getAncestorInvariantCondition();
      assertTrueMsg(ancestorInvariant.isId(),
                    "Internal error: ancestor invariant condition is null!");
      ancestorInvariant->addListener(m_listener.getId());
    }

    // Let the derived class do its thing (currently only ListNode)
    createSpecializedConditions();

    // Add user-specified conditions
    for (std::vector<std::pair <PlexilExprId, std::string> >::const_iterator it = conds.begin(); 
         it != conds.end(); 
         ++it) {
      size_t condIdx = getConditionIndex(std::string(it->second));

      // Delete existing condition if required
      // (e.g. explicit override of default end condition for list or library call node)

      // Remove appropriate listener before deleting
      removeConditionListener(condIdx);

      if (m_garbageConditions[condIdx]) {
        delete (Expression*) m_conditions[condIdx];
        m_garbageConditions[condIdx] = false;
      }

      m_conditions[condIdx] = 
        ExpressionFactory::createInstance(it->first,
                                          NodeConnector::getId(), 
                                          m_garbageConditions[condIdx]);

      // Add listener
      switch (condIdx) {

      case postIdx:
      case preIdx:
        break; // these conditions don't need listeners

      default:
        m_conditions[condIdx]->addListener(m_listener.getId());
        break;
      }
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
  void Node::specializedPostInit(const PlexilNodeId& /* node */)
  {
  }

  // Default method
  void Node::specializedPostInitLate(const PlexilNodeId& /* node */)
  {
  }

  Node::~Node() 
  {
    debugMsg("Node:~Node", " base class destructor for " << m_nodeId);

    // Remove conditions as they may refer to variables, either ours or another node's
    // Derived classes should also call this
    cleanUpConditions();

    // cleanUpNodeBody(); // NOT USEFUL here - derived classes MUST call this!

    // Now safe to delete variables
    cleanUpVars();

    delete m_sortedVariableNames;
    m_id.removeDerived(NodeConnector::getId());
  }

  void Node::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

    debugMsg("Node:cleanUpConditions", " for " << m_nodeId);

    // Remove condition listeners
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      ExpressionId cond = getCondition(i);
      if (cond.isId())
        cond->removeListener(m_listener.getId());
    }

    // Clean up conditions
    // N.B.: Ancestor-end and ancestor-invariant MUST be cleaned up before
    // end and invariant, respectively. 
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (m_garbageConditions[i]) {
        debugMsg("Node:cleanUpConds",
                 "<" << m_nodeId << "> Removing condition " << getConditionName(i));
        delete (Expression*) m_conditions[i];
      }
      m_conditions[i] = ExpressionId::noId();
      m_garbageConditions[i] = false;
    }

    m_cleanedConditions = true;
  }

  void Node::removeConditionListener(size_t condIdx)
  {
    switch (condIdx) {

       // These conditions don't have listeners
    case postIdx:
    case preIdx:
      return;

      // These conditions are owned by the parent
    case ancestorEndIdx:
    case ancestorExitIdx:
    case ancestorInvariantIdx:
      getCondition(condIdx)->removeListener(m_listener.getId());
      break;

    default:
      m_conditions[condIdx]->removeListener(m_listener.getId());
      break;
    }
  }

  // Default method.
  void Node::cleanUpNodeBody()
  {
  }

  // Called from base class destructor and possibly derived as well.
  void Node::cleanUpVars() 
  {
    if (m_cleanedVars)
      return;
    checkError(m_cleanedConditions,
               "Have to clean up variables before conditions can be cleaned.");

    debugMsg("Node:cleanUpVars", " for " << m_nodeId);

    // Clear map
    m_variablesByName.clear();

    // Delete user-spec'd variables
    for (std::vector<ExpressionId>::iterator it = m_localVariables.begin(); it != m_localVariables.end(); ++it) {
      debugMsg("Node:cleanUpVars",
               "<" << m_nodeId << "> Removing " << **it);
      delete (Expression *) (*it);
    }
    m_localVariables.clear();

    // Delete timepoint variables
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; ++s) {
      if (m_startTimepoints[s].isId()) {
        delete (Expression *) m_startTimepoints[s];
        delete (Expression *) m_endTimepoints[s];
        m_startTimepoints[s] = m_endTimepoints[s] = ExpressionId::noId();
      }
    }

    // Delete internal variables
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
    m_stateVariable.activate();
    m_outcomeVariable.activate();
    m_failureTypeVariable.activate();

    // Activate timepoints
    for (int s = INACTIVE_STATE; s <= nodeStateMax(); ++s) {
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
        return FALSE_EXP();

    case ancestorInvariantIdx:

      if (m_parent.isId())
        return m_parent->m_conditions[idx];
      else
        return TRUE_EXP();

    default:
      return m_conditions[idx];
    }
  }

  const ExpressionId& Node::getCondition(const std::string& name) const {
    return getCondition(getConditionIndex(name));
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
      return; // already in the queue
    debugMsg("Node:conditionChanged", " for node " << m_nodeId);
    g_exec->notifyNodeConditionChanged(m_id);
    m_checkConditionsPending = true;
  }

  /**
   * @brief Evaluates the conditions to see if the node is eligible to transition.
   */
  void Node::checkConditions() {
    debugMsg("Node:checkConditions",
             "Checking condition change for node " << m_nodeId);
    NodeState toState(getDestState());
    debugMsg("Node:checkConditions",
             "Can (possibly) transition to " << nodeStateName(toState));
    if (toState != m_lastQuery) {
      g_exec->handleConditionsChanged(m_id, toState);
      m_lastQuery = toState;
    }
    m_checkConditionsPending = false;
  }

  NodeState Node::getDestState() 
  {
    debugMsg("Node:getDestState",
             "Getting destination state for " << m_nodeId << " from state " <<
             getStateName());

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

  // For unit test only.
  void Node::transition(NodeState destState) 
  {
    double sl_time = 0;
    transition(destState, sl_time);
  }

  void Node::transition(NodeState destState, double time) 
  {
    checkError(destState != NO_NODE_STATE
               && destState != m_state,
               "Attempted to transition node " << m_nodeId <<
               " when it is ineligible.");

    NodeState prevState = (NodeState) m_state;
    
    transitionFrom(destState);
    transitionTo(destState);

    debugMsg("Node:transition", "Transitioning '" << m_nodeId <<
             "' from " << nodeStateName(prevState) <<
             " to " << nodeStateName(destState));
    condDebugMsg((destState == FINISHED_STATE),
                 "Node:outcome",
                 "Outcome of '" << m_nodeId <<
                 "' is " << outcomeName((NodeOutcome) m_outcome));
    condDebugMsg((destState == FINISHED_STATE && m_outcome == FAILURE_OUTCOME),
                 "Node:failure",
                 "Failure type of '" << m_nodeId <<
                 "' is " << failureTypeName((FailureType) m_failureType));
    condDebugMsg((destState == ITERATION_ENDED_STATE),
                 "Node:iterationOutcome",
                 "Outcome of '" << m_nodeId <<
                 "' is " << outcomeName((NodeOutcome) m_outcome));
    debugMsg("Node:times",
             "Setting '" << m_nodeId
             << "' end time " << END_TIMEPOINT_NAMES()[prevState]
             << " = start time " << START_TIMEPOINT_NAMES()[destState]
             << " = " << time);
    // FIXME - Need better way to record transition times
    m_endTimepoints[prevState]->getAssignableId()->setValue(time);
    m_startTimepoints[destState]->getAssignableId()->setValue(time);
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
  // Conditions active: If parent in EXECUTING - AncestorExit, AncestorEnd, AncestorInvariant,
  //                    else none
  // Legal successor states: WAITING, FINISHED

  // Default method
  void Node::transitionToInactive()
  {
  }

  // Default method
  NodeState Node::getDestStateFromInactive()
  {
    bool temp;
    if (m_parent.isId()) {
      switch (m_parent->getState()) {

      case FINISHED_STATE:
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << "' destination: FINISHED. Parent state == FINISHED.");
        return FINISHED_STATE;

      case EXECUTING_STATE: {
        // N.B. Ancestor-exit, ancestor-invariant, ancestor-end should have been activated by parent
        ExpressionId cond = getAncestorExitCondition();
        checkError(cond->isActive(),
                   "Node::getDestStateFromInactive: Ancestor exit for " << m_nodeId << " is inactive.");
        if (cond->getValue(temp) && temp) {
          debugMsg("Node:getDestState",
                   " '" << m_nodeId << "' destination: FINISHED. Parent EXECUTING and ANCESTOR_EXIT_CONDITION true.");
          return FINISHED_STATE;
        }

        cond = getAncestorInvariantCondition();
        checkError(cond->isActive(),
                   "Node::getDestStateFromInactive: Ancestor invariant for " << m_nodeId << " is inactive.");
        if (cond->getValue(temp) && !temp) {
          debugMsg("Node:getDestState",
                   " '" << m_nodeId << "' destination: FINISHED. Parent EXECUTING and ANCESTOR_INVARIANT_CONDITION false.");
          return FINISHED_STATE;
        }

        cond = getAncestorEndCondition();
        checkError(cond->isActive(),
                   "Node::getDestStateFromInactive: Ancestor end for " << m_nodeId << " is inactive.");
        if (cond->getValue(temp) && temp) {
          debugMsg("Node:getDestState",
                   " '" << m_nodeId << "' destination: FINISHED. Parent EXECUTING and ANCESTOR_END_CONDITION true.");
          return FINISHED_STATE;
        }

        debugMsg("Node:getDestState",
                 " '" << m_nodeId << "' destination: WAITING. Parent state == EXECUTING.");
        return WAITING_STATE;
      }

      default:
        debugMsg("Node:getDestState", 
                 " '" << m_nodeId << "' destination: no state.");
        return NO_NODE_STATE;
      }
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: WAITING. Root node.");
      return WAITING_STATE;
    }
  }

  // Default method
  void Node::transitionFromInactive(NodeState destState)
  {
    checkError(destState == WAITING_STATE || destState == FINISHED_STATE,
               "Attempting to transition from INACTIVE to invalid state '"
               << nodeStateName(destState) << "'");
    if (destState == FINISHED_STATE) {
      setNodeOutcome(SKIPPED_OUTCOME);
    }
    else { // WAITING
      activateAncestorEndCondition();
      activateAncestorExitInvariantConditions();
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
    activatePreSkipStartConditions();
  }

  // Default method
  NodeState Node::getDestStateFromWaiting()
  {
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromWaiting: Ancestor exit for " << m_nodeId << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromWaiting: Exit condition for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromWaiting: Ancestor invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_INVARIANT_CONDITION false.");
      return FINISHED_STATE;
    }

    cond = getAncestorEndCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromWaiting: Ancestor end for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_END_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getSkipCondition();
    checkError(cond->isActive(), 
               "Node::getDestStateFromWaiting: Skip for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. SKIP_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getStartCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromWaiting: Start for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      cond = getPreCondition();
      checkError(cond->isActive(),
                 "Node::getDestStateFromWaiting: Pre for " << m_nodeId << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << "' destination: EXECUTING. START_CONDITION and PRE_CONDITION are both true.");
        return EXECUTING_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId << "' destination: ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
        return ITERATION_ENDED_STATE;
      }
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId << "' destination: no state. START_CONDITION false or unknown");
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromWaiting(NodeState destState)
  {
    deactivatePreSkipStartConditions();
    switch (destState) {

    case EXECUTING_STATE:
      deactivateAncestorEndCondition();
      break;

    case ITERATION_ENDED_STATE:
      deactivateExitCondition();
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(PRE_CONDITION_FAILED);
      break;

    case FINISHED_STATE:
      deactivateAncestorEndCondition();
      deactivateAncestorExitInvariantConditions();
      deactivateExitCondition();
      setNodeOutcome(SKIPPED_OUTCOME);
      break;

    default:
      checkError(ALWAYS_FAIL,
                 "Attempting to transition from WAITING to invalid state '"
                 << nodeStateName(destState) << "'");
      break;
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
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Ancestor exit for " << m_nodeId << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Exit condition for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: ITERATION_ENDED. EXIT_CONDITION true.");
      return ITERATION_ENDED_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Ancestor invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. Ancestor invariant false.");
      return FINISHED_STATE;
    }

    cond = getInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: Invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: ITERATION_ENDED. Invariant false.");
      return ITERATION_ENDED_STATE;
    }

    cond = getEndCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromExecuting: End for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: ITERATION_ENDED. End condition true.");
      return ITERATION_ENDED_STATE;
    }

    return NO_NODE_STATE;
  }

  // Empty node method
  void Node::transitionFromExecuting(NodeState destState)
  {
    checkError(m_nodeType == Node::EMPTY(),
               "Expected empty node, got " << m_nodeType);
    checkError(destState == FINISHED_STATE || destState == ITERATION_ENDED_STATE,
               "Attempting to transition from EXECUTING to invalid state '"
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
    // Below here, we know EndCondition is true
    else if (getPostCondition()->getValue(temp) && temp)
      setNodeOutcome(SUCCESS_OUTCOME);
    else {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(POST_CONDITION_FAILED);
    }

    deactivateEndCondition();
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();
    if (destState == FINISHED_STATE) {
      deactivateAncestorExitInvariantConditions();
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
    ExpressionId cond = getAncestorExitCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromIterationEnded: Ancestor exit for " << m_nodeId << " is inactive.");
    bool temp;
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_EXIT_CONDITION true.");
      return FINISHED_STATE;
    }

    cond = getAncestorInvariantCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromIterationEnded: Ancestor invariant for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && !temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_INVARIANT false.");
      return FINISHED_STATE;
    }

    cond = getAncestorEndCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromIterationEnded: Ancestor end for " << m_nodeId << " is inactive.");
    if (cond->getValue(temp) && temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. ANCESTOR_END true.");
      return FINISHED_STATE;
    }

    cond = getRepeatCondition();
    checkError(cond->isActive(),
               "Node::getDestStateFromIterationEnded: Repeat for " << m_nodeId << " is inactive.");
    if (!cond->getValue(temp)) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: no state. ANCESTOR_END false or unknown and REPEAT unknown.");
      return NO_NODE_STATE;
    } 
    if (temp) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: WAITING. REPEAT_CONDITION true.");
      return WAITING_STATE;
    }
    else {
      debugMsg("Node:getDestState",
               " '" << m_nodeId << "' destination: FINISHED. REPEAT_CONDITION false.");
      return FINISHED_STATE;
    }
  }

  // Default method
  void Node::transitionFromIterationEnded(NodeState destState)
  {
    checkError(destState == FINISHED_STATE || destState == WAITING_STATE,
               "Attempting to transition from ITERATION_ENDED to invalid state '"
               << nodeStateName(destState) << "'");

    bool temp;
    if (getAncestorExitCondition()->getValue(temp) && temp) {
      setNodeOutcome(INTERRUPTED_OUTCOME);
      setNodeFailureType(PARENT_EXITED);
    }
    else if (getAncestorInvariantCondition()->getValue(temp) && !temp) {
      setNodeOutcome(FAILURE_OUTCOME);
      setNodeFailureType(PARENT_FAILED);
    }

    deactivateRepeatCondition();

    if (destState == FINISHED_STATE) {
      deactivateAncestorEndCondition();
      deactivateAncestorExitInvariantConditions();
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
               " '" << m_nodeId << "' destination: INACTIVE.  Parent state == WAITING.");
      return INACTIVE_STATE;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId << "' destination: no state.");
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFinished(NodeState destState)
  {
    checkError(destState == INACTIVE_STATE,
               "Attempting to transition from FINISHED to invalid state '"
               << nodeStateName(destState) << "'");
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
               "Attempted to compute destination state from FINISHING for node " << m_nodeId
               << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFinishing(NodeState /* destState */)
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
               "Attempted to compute destination state from FAILING for node " << m_nodeId
               << " of type " << getType());
    return NO_NODE_STATE;
  }

  // Default method
  void Node::transitionFromFailing(NodeState /* destState */)
  {
    checkError(ALWAYS_FAIL,
               "No transition from FAILING state defined for this node");
  }

  // ***
  // *** END NODE STATE LOGIC ***
  // ***

  const ExpressionId& Node::getInternalVariable(const std::string& name) const
  {
    VariableMap::const_iterator it = m_variablesByName.find(name);
    checkError(it != m_variablesByName.end(),
               "No variable named " << name << " in " << m_nodeId);
    return it->second;
  }

  std::string const &Node::getStateName() const {
    return nodeStateName(m_state);
  }

  NodeState Node::getState() const {
    return (NodeState) m_state;
  }

  // Some transition handlers call this twice.
  void Node::setState(NodeState newValue) {
    checkError(newValue <= nodeStateMax(),
               "Attempted to set an invalid NodeState value for this node");
    if (newValue == m_state)
      return;
    m_state = newValue;
    if (newValue == FINISHED_STATE && m_parent.isNoId()) {
      // Mark this node as ready to be deleted -
      // with no parent, it cannot be reset.
      g_exec->markRootNodeFinished(m_id);
    }
    m_stateVariable.changed();
  }

  double Node::getCurrentStateStartTime() const
  {
    double result;
    assertTrue_2(m_startTimepoints[m_state]->getValue(result),
                 "getCurrentStateStartTime: state's start time is unknown");
    return result;
  }

  double Node::getCurrentStateEndTime() const
  {
    double result = DBL_MAX;
    // Unknown will leave result unmodified
    m_endTimepoints[m_state]->getValue(result);
    return result;
  }

  void Node::setNodeOutcome(NodeOutcome o)
  {
    checkError(o >= NO_OUTCOME && o < OUTCOME_MAX,
               "Node::setNodeOutcome: invalid outcome value");
    m_outcome = o;
    m_outcomeVariable.changed();
  }

  NodeOutcome Node::getOutcome() const
  {
    return (NodeOutcome) m_outcome;
  }

  void Node::setNodeFailureType(FailureType f)
  {
    checkError(f >= NO_FAILURE && f < FAILURE_TYPE_MAX,
               "Node::setNodeFailureType: invalid failureType value");
    m_failureType = f;
    m_failureTypeVariable.changed();
  }

  FailureType Node::getFailureType() const
  {
    return (FailureType) m_failureType;
  }

  // Searches ancestors when required
  const ExpressionId& Node::findVariable(const std::string& name, bool recursive)
  {
    debugMsg("Node:findVariable",
             " for node '" << m_nodeId
             << "', searching by name for \"" << name << "\"");
    VariableMap::const_iterator it = m_variablesByName.find(name);
    if (it != m_variablesByName.end()) {
      debugMsg("Node:findVariable",
               " Returning " << it->second->toString());
      return it ->second;
    }

    // Not found locally - try ancestors if possible
    // Stop at library call nodes, as interfaces there are explicit
    if (m_parent.isId()
        && m_parent->m_nodeType != LIBRARYNODECALL()) {
      const ExpressionId& result = m_parent->findVariable(name, true);
      if (result.isId()) {
        // Found it - cache for later reuse
        m_variablesByName[name] = result;
        return result;
      }
      // Not found 
      else if (recursive)
        return ExpressionId::noId(); // so that error happens at approriate level
      // else fall through to failure
    }

    // FIXME: push this check up into XML parser
    checkError(ALWAYS_FAIL,
               "No variable named \"" << name << "\" accessible from node " <<
               m_nodeId);
    return ExpressionId::noId();
  }

  const ExpressionId& Node::findVariable(const PlexilVarRef* ref)
  {
    debugMsg("Node:findVariable",
             " for node '" << m_nodeId
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
                   m_nodeId);
        node = m_parent;
        break;

      case PlexilNodeRef::CHILD:
        {
          node = findChild(std::string(nodeRef->name()));
          // FIXME: push this check up into XML parser
          checkError(node.isId(),
                     "No child named '" << nodeRef->name() << 
                     "' in " << m_nodeId);
          break;
        }

      case PlexilNodeRef::SIBLING: 
        {
          // FIXME: push this check up into XML parser
          checkError(m_parent.isValid(),
                     "Sibling node reference in root node " << 
                     m_nodeId);
          node = m_parent->findChild(std::string(nodeRef->name()));
          // FIXME: push this check up into XML parser
          checkError(node.isId(),
                     "No sibling named '" << nodeRef->name() << 
                     "' of " << m_nodeId);
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
                     m_nodeId);
          checkError(nodeRef->name() == node->getNodeId(),
                     "Grandparent node reference for '" << nodeRef->name()
                     << "' found node '" << node->getNodeId() << "' instead");
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
                     m_nodeId);
          node = ancestor->findChild(std::string(nodeRef->name()));
          // FIXME: push this check up into XML parser
          checkError(node.isId(),
                     "No uncle named '" << nodeRef->name() << 
                     "' of " << m_nodeId);
          break;
        }

      default:
        // FIXME: catch this error in XML parsing
        checkError(ALWAYS_FAIL,
                   "Invalid direction in node reference from " <<
                   m_nodeId);
        return ExpressionId::noId();
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
      return node->getInternalVariable(std::string(name));
    }
    else {
      return findVariable(std::string(ref->name()));
    }
  }

  // Default method
  NodeId Node::findChild(const std::string& /* childName */) const
  {
    return NodeId::noId();
  }

  //
  // Conditions
  //

  // These are special because parent owns the condition expression
  void Node::activateAncestorEndCondition()
  {
  }
  void Node::activateAncestorExitInvariantConditions()
  {
  }

  // User conditions
  void Node::activatePreSkipStartConditions()
  {
    checkError(m_conditions[preIdx].isId(),
               "No PreCondition exists in node \"" << m_nodeId << "\"");
    checkError(m_conditions[skipIdx].isId(),
               "No SkipCondition exists in node \"" << m_nodeId << "\"");
    checkError(m_conditions[startIdx].isId(),
               "No StartCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activatePreSkipStartConditions",
             "Activating PreCondition, SkipCondition, and StartCondition in node \"" << m_nodeId << "\"");
    m_conditions[preIdx]->activate();
    m_conditions[skipIdx]->activate();
    m_conditions[startIdx]->activate();
  }

  void Node::activateEndCondition()
  {
    checkError(m_conditions[endIdx].isId(),
               "No EndCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateEndCondition",
             "Activating EndCondition in node \"" << m_nodeId << "\"");
    m_conditions[endIdx]->activate();
  }

  void Node::activateExitCondition()
  {
    checkError(m_conditions[exitIdx].isId(),
               "No ExitCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateExitCondition",
             "Activating ExitCondition in node \"" << m_nodeId << "\"");
    m_conditions[exitIdx]->activate();
  }

  void Node::activateInvariantCondition()
  {
    checkError(m_conditions[invariantIdx].isId(),
               "No InvariantCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateInvariantCondition",
             "Activating InvariantCondition in node \"" << m_nodeId << "\"");
    m_conditions[invariantIdx]->activate();
  }

  void Node::activatePostCondition()
  {
    checkError(m_conditions[postIdx].isId(),
               "No PostCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activatePostCondition",
             "Activating PostCondition in node \"" << m_nodeId << "\"");
    m_conditions[postIdx]->activate();
  }

  void Node::activateRepeatCondition()
  {
    checkError(m_conditions[repeatIdx].isId(),
               "No RepeatCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateRepeatCondition",
             "Activating RepeatCondition in node \"" << m_nodeId << "\"");
    m_conditions[repeatIdx]->activate();
  }

  // These are for specialized node types
  void Node::activateActionCompleteCondition()
  {
    checkError(m_conditions[actionCompleteIdx].isId(),
               "No ActionCompleteCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateActionCompleteCondition",
             "Activating ActionCompleteCondition in node \"" << m_nodeId << "\"");
    m_conditions[actionCompleteIdx]->activate();
  }

  void Node::activateAbortCompleteCondition()
  {
    checkError(m_conditions[abortCompleteIdx].isId(),
               "No AbortCompleteCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:activateAbortCompleteCondition",
             "Activating AbortCompleteCondition in node \"" << m_nodeId << "\"");
    m_conditions[abortCompleteIdx]->activate();
  }

  // These are special because parent owns the condition expression
  void Node::deactivateAncestorEndCondition()
  {
  }
  void Node::deactivateAncestorExitInvariantConditions()
  {
  }

  // User conditions
  void Node::deactivatePreSkipStartConditions()
  {
    checkError(m_conditions[preIdx].isId(),
               "No PreCondition exists in node \"" << m_nodeId << "\"");
    checkError(m_conditions[skipIdx].isId(),
               "No SkipCondition exists in node \"" << m_nodeId << "\"");
    checkError(m_conditions[startIdx].isId(),
               "No StartCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivatePreSkipStartConditions",
             "Deactivating PreCondition, SkipCondition, and StartCondition in node \"" << m_nodeId << "\"");
    m_conditions[preIdx]->deactivate();
    m_conditions[skipIdx]->deactivate();
    m_conditions[startIdx]->deactivate();
  }

  void Node::deactivateEndCondition()
  {
    checkError(m_conditions[endIdx].isId(),
               "No EndCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateEndCondition",
             "Deactivating EndCondition in node \"" << m_nodeId << "\"");
    m_conditions[endIdx]->deactivate();
  }

  void Node::deactivateExitCondition()
  {
    checkError(m_conditions[exitIdx].isId(),
               "No ExitCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateExitCondition",
             "Deactivating ExitCondition in node \"" << m_nodeId << "\"");
    m_conditions[exitIdx]->deactivate();
  }

  void Node::deactivateInvariantCondition()
  {
    checkError(m_conditions[invariantIdx].isId(),
               "No InvariantCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateInvariantCondition",
             "Deactivating InvariantCondition in node \"" << m_nodeId << "\"");
    m_conditions[invariantIdx]->deactivate();
  }

  void Node::deactivatePostCondition()
  {
    checkError(m_conditions[postIdx].isId(),
               "No PostCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivatePostCondition",
             "Deactivating PostCondition in node \"" << m_nodeId << "\"");
    m_conditions[postIdx]->deactivate();
  }

  void Node::deactivateRepeatCondition()
  {
    checkError(m_conditions[repeatIdx].isId(),
               "No RepeatCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateRepeatCondition",
             "Deactivating RepeatCondition in node \"" << m_nodeId << "\"");
    m_conditions[repeatIdx]->deactivate();
  }

  // These are for specialized node types
  void Node::deactivateActionCompleteCondition()
  {
    checkError(m_conditions[actionCompleteIdx].isId(),
               "No ActionCompleteCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateActionCompleteCondition",
             "Deactivating ActionCompleteCondition in node \"" << m_nodeId << "\"");
    m_conditions[actionCompleteIdx]->deactivate();
  }

  void Node::deactivateAbortCompleteCondition()
  {
    checkError(m_conditions[abortCompleteIdx].isId(),
               "No AbortCompleteCondition exists in node \"" << m_nodeId << "\"");
    debugMsg("Node:deactivateAbortCompleteCondition",
             "Deactivating AbortCompleteCondition in node \"" << m_nodeId << "\"");
    m_conditions[abortCompleteIdx]->deactivate();
  }

  void Node::execute() 
  {
    checkError(m_state == EXECUTING_STATE,
               "Node \"" << m_nodeId
               << "\" told to handle execution, but it's in state '" <<
               getStateName() << "'");
    debugMsg("Node:execute", "Executing node " << m_nodeId);

    activateLocalVariables();

    // legacy message for unit test
    debugMsg("PlexilExec:handleNeedsExecution",
             "Storing action for node '" << m_nodeId <<
             "' of type '" << m_nodeType << 
             "' to be executed.");

    // Here only to placate the unit test
    g_exec->notifyExecuted(getId());

    specializedHandleExecution();
  }

  // default method
  void Node::specializedHandleExecution()
  {
  }

  void Node::reset()
  {
    debugMsg("Node:reset", "Re-setting node " << m_nodeId);

    //reset outcome and failure type
    m_outcome = NO_OUTCOME;
    m_failureType = NO_FAILURE;

    //reset timepoints
    for (int s = INACTIVE_STATE; s <= nodeStateMax(); ++s) {
      m_startTimepoints[s]->deactivate();
      m_startTimepoints[s]->getAssignableId()->reset();
      m_startTimepoints[s]->activate();
      m_endTimepoints[s]->deactivate();
      m_endTimepoints[s]->getAssignableId()->reset();
      m_endTimepoints[s]->activate();
    }

    for (std::vector<ExpressionId>::const_iterator it = m_localVariables.begin();
         it != m_localVariables.end();
         ++it)
      if ((*it)->isAssignable())
        (*it)->getAssignableId()->reset();

    specializedReset();
  }

  // Default method
  void Node::specializedReset()
  {
  }

  // Default method
  void Node::abort() 
  {
    checkError(ALWAYS_FAIL, "Abort illegal for node type " << getType());
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

  void Node::activateLocalVariables()
  {
    for (std::vector<ExpressionId>::iterator vit = m_localVariables.begin();
         vit != m_localVariables.end();
         ++vit)
      (*vit)->activate();
  }

  void Node::deactivateLocalVariables()
  {
    for (std::vector<ExpressionId>::iterator vit = m_localVariables.begin();
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

    stream << indentStr << m_nodeId << "{\n";
    stream << indentStr << " State: " << getStateName() <<
      " (" << getCurrentStateStartTime() << ")\n";
    if (m_state == FINISHED_STATE) {
      stream << indentStr << " Outcome: " << outcomeName((NodeOutcome) m_outcome) << '\n';
      if (m_failureType != NO_FAILURE)
        stream << indentStr << " Failure type: " <<
          failureTypeName((FailureType) m_failureType) << '\n';
      // Print variables, starting with command handle
      printCommandHandle(stream, indent);
      printVariables(stream, indent);
    }
    else if (m_state != INACTIVE_STATE) {
      // Print conditions
      for (size_t i = 0; i < conditionIndexMax; ++i) {
        if (getCondition(i).isId()) {
          stream << indentStr << " " << getConditionName(i) << ": " <<
            getCondition(i)->toString() << '\n';
        }
      }
      // Print variables, starting with command handle
      printCommandHandle(stream, indent);
      printVariables(stream, indent);
    }
    // print children
    for (std::vector<NodeId>::const_iterator it = getChildren().begin(); it != getChildren().end(); ++it)
      (*it)->print(stream, indent + 2);
    stream << indentStr << "}" << std::endl;
  }

  // Default method does nothing
  void Node::printCommandHandle(std::ostream& /* stream */, const unsigned int /* indent */) const
  {
  }

  // Print variables
  void Node::printVariables(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');
    ensureSortedVariableNames(); // for effect
    for (std::vector<std::string>::const_iterator it = m_sortedVariableNames->begin();
         it != m_sortedVariableNames->end();
         ++it) {
      stream << indentStr << " " << *it << ": " <<
        *(getInternalVariable(*it)) << '\n';
    }
  }

  const ExecListenerHubId& Node::getExecListenerHub() const
  {
    if (g_exec.isNoId()) {
      static ExecListenerHubId sl_hubNoId;
      return sl_hubNoId;
    }
    else
      return g_exec->getExecListenerHub();
  }

  // Used to be a LabelStr method
  static size_t countElements(const std::string &str, const char* delimiters)
  {
    assertTrueMsg(delimiters != NULL && delimiters[0] != '\0',
                  "'NULL' and empty string are not valid delimiters");
    size_t result = 0;

    // Skip delimiters at beginning. Note the "not_of".
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
      // Found a token
      ++result;
      // Skip next delimiter.
      lastPos = str.find_first_not_of(delimiters, pos);
      // Find next non-delimiter.
      pos = str.find_first_of(delimiters, lastPos);
    }
    return result;
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
        const std::string& name = it->first;
        if (name == STATE()
            || name == OUTCOME()
            || name == FAILURE_TYPE()
            || name == COMMAND_HANDLE()
            || countElements(name, ".") > 1)
          continue;
        m_sortedVariableNames->push_back(it->first);
      }
      // Sort the names
      std::sort(m_sortedVariableNames->begin(),
                m_sortedVariableNames->end());
    }
  }

}
