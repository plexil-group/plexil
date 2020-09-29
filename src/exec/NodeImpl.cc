/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "NodeImpl.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ExternalInterface.hh"
#include "NodeConstants.hh"
#include "NodeTimepointValue.hh"
#include "PlexilExec.hh"
#include "NodeVariableMap.hh"
#include "SimpleMap.hh"
#include "UserVariable.hh"

#include <iomanip>   // std::setprecision
#include <sstream>

#if defined(HAVE_CFLOAT)
#include <cfloat>   // DBL_MAX
#elif defined(HAVE_FLOAT_H)
#include <float.h>  // DBL_MAX
#endif

#if defined(HAVE_CSTRING)
#include <cstring>   // strcmp(), strnlen()
#elif defined(HAVE_STRING_H)
#include <string.h>  // strcmp(), strnlen()
#endif

namespace PLEXIL
{

  //
  // Static members
  //

  // NOTE: this used to be 100000000, which somehow gets printed as
  // scientific notation in XML and doesn't parse correctly.
  const int32_t NodeImpl::WORST_PRIORITY = 100000;

  char const * const NodeImpl::ALL_CONDITIONS[] =
    {
      "AncestorExitCondition",
      "AncestorInvariantCondition",
      "AncestorEndCondition",
      "SkipCondition",
      "StartCondition",
      "PreCondition",
      "ExitCondition",
      "InvariantCondition",
      "EndCondition",
      "PostCondition",
      "RepeatCondition",
      "ActionCompleteCondition",
      "AbortCompleteCondition"
    };

  // gperf-inspired version
  static NodeImpl::ConditionIndex getConditionIndex(char const *cName)
  {
    NodeImpl::ConditionIndex result = NodeImpl::conditionIndexMax;
    switch (*cName) {
    case 'A':
      // AbortCompleteCondition
      // ActionCompleteCondition
      // AncestorEndCondition
      // AncestorExitCondition
      // AncestorInvariantCondition
      // Check length to ensure there is a cName[10]
      if (strnlen(cName, 11) < 11)
        return NodeImpl::conditionIndexMax;

      switch (cName[10]) { // First completely unique character
      case 'd':
        result = NodeImpl::ancestorEndIdx;
        break;

      case 'e':
        result = NodeImpl::abortCompleteIdx;
        break;

      case 'i':
        result = NodeImpl::ancestorExitIdx;
        break;

      case 'l':
        result = NodeImpl::abortCompleteIdx;
        break;

      case 'v':
        result = NodeImpl::ancestorInvariantIdx;
        break;

      default:
        return NodeImpl::conditionIndexMax;
      }
      break;

    case 'E': // EndCondition, ExitCondition
      result = (cName[1] == 'n') ? NodeImpl::endIdx : NodeImpl::exitIdx;
      break;

    case 'I': // InvariantCondition
      result = NodeImpl::invariantIdx;
      break;
      
    case 'P': // PostCondition, PreCondition
      result = (cName[1] == 'o') ? NodeImpl::postIdx : NodeImpl::preIdx;
      break;

    case 'R': // RepeatCondition
      result = NodeImpl::repeatIdx;
      break;

    case 'S': // SkipCondition, StartCondition
      result = (cName[1] == 'k') ? NodeImpl::skipIdx : NodeImpl::startIdx;
      break;

    default:
      return NodeImpl::conditionIndexMax;
    }

    if (strcmp(cName, NodeImpl::ALL_CONDITIONS[result]))
      return NodeImpl::conditionIndexMax;
    return result;
  }

  char const *NodeImpl::getConditionName(size_t idx)
  {
    return ALL_CONDITIONS[idx];
  }

  NodeImpl::NodeImpl(char const *nodeId, NodeImpl *parent)
    : Node(),
      Notifier(),
      m_next(NULL),
      m_queueStatus(0),
      m_state(INACTIVE_STATE),
      m_outcome(NO_OUTCOME),
      m_failureType(NO_FAILURE),
      m_nextState(NO_NODE_STATE),
      m_nextOutcome(NO_OUTCOME),
      m_nextFailureType(NO_FAILURE),
      m_parent(parent),
      m_conditions(),
      m_localVariables(NULL),
      m_stateVariable(*this),
      m_outcomeVariable(*this),
      m_failureTypeVariable(*this),
      m_variablesByName(NULL),
      m_nodeId(nodeId),
      m_currentStateStartTime(0.0),
      m_timepoints(NULL),
      m_garbageConditions(),
      m_cleanedConditions(false),
      m_cleanedVars(false),
      m_cleanedBody(false)
  {
    debugMsg("NodeImpl:NodeImpl", " Constructor for \"" << m_nodeId << "\"");
    commonInit();
  }

  // Used only by module test
  NodeImpl::NodeImpl(const std::string& type, 
                     const std::string& name, 
                     NodeState state,
                     NodeImpl *parent)
    : Node(),
      m_next(NULL),
      m_queueStatus(0),
      m_state(state),
      m_outcome(NO_OUTCOME),
      m_failureType(NO_FAILURE),
      m_nextState(NO_NODE_STATE),
      m_nextOutcome(NO_OUTCOME),
      m_nextFailureType(NO_FAILURE),
      m_parent(parent),
      m_conditions(),
      m_localVariables(NULL),
      m_stateVariable(*this),
      m_outcomeVariable(*this),
      m_failureTypeVariable(*this),
      m_variablesByName(NULL),
      m_nodeId(name),
      m_currentStateStartTime(0.0),
      m_timepoints(NULL),
      m_garbageConditions(),
      m_cleanedConditions(false), 
      m_cleanedVars(false),
      m_cleanedBody(false)
  {
    static Value const falseValue(false);

    commonInit();

    for (size_t i = 0; i < conditionIndexMax; ++i) {
      std::string varName = m_nodeId + ' ' + ALL_CONDITIONS[i];
      BooleanVariable *expr = new BooleanVariable(varName.c_str());
      expr->setValue(falseValue);
      debugMsg("NodeImpl:NodeImpl", ' ' << m_nodeId
               << " Created internal variable " << varName <<
               " with value FALSE");
      m_conditions[i] = expr;
      m_garbageConditions[i] = true;
      // N.B. Ancestor-end, ancestor-exit, and ancestor-invariant belong to parent;
      // will be NULL if this node has no parent
      if (i != preIdx && i != postIdx && getCondition(i))
        getCondition(i)->addListener(this);
    }

    PlexilNodeType nodeType = parseNodeType(type.c_str());
    // Activate the conditions required by the provided state
    switch (m_state) {

    case INACTIVE_STATE:
      break;

    case WAITING_STATE:
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
      activateExitCondition();
      activateInvariantCondition();
      activatePreSkipStartConditions();
      break;

    case EXECUTING_STATE:
      activateAncestorExitInvariantConditions();
      activateExitCondition();
      activateInvariantCondition();
      activateEndCondition();
      activatePostCondition();
      break;

    case FAILING_STATE:
      assertTrueMsg(nodeType != NodeType_Empty,
                    "Node module test constructor: FAILING state invalid for Empty nodes");
      // Defer to subclass
      break;

    case FINISHING_STATE:
      assertTrueMsg(nodeType != NodeType_Empty,
                    "Node module test constructor: FINISHING state invalid for Empty nodes");
      // Defer to subclass
      break;

    case ITERATION_ENDED_STATE:
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
      activateRepeatCondition();
      break;

    case FINISHED_STATE:
      break;

    default:
      errorMsg("Node module test constructor: Invalid state " << state);
    }
  }

  // N.B.: called from base class constructor
  void NodeImpl::commonInit() {
    debugMsg("NodeImpl:NodeImpl", " common initialization");

    // Initialize transition trace
    logTransition(g_interface->currentTime(), (NodeState) m_state);
  }

  void NodeImpl::allocateVariables(size_t n)
  {
    assertTrue_1(!m_localVariables); // illegal to call this twice
    m_localVariables = new std::vector<Expression *>();
    m_localVariables->reserve(n);
    m_variablesByName =
      new NodeVariableMap(m_parent ? m_parent->getChildVariableMap() : NULL);
    m_variablesByName->grow(n);
  }

  // Default method.
  NodeVariableMap const *NodeImpl::getChildVariableMap() const
  {
    return NULL; // this node has no children
  }

  bool NodeImpl::addLocalVariable(char const *name, Expression *var)
  {
    assertTrueMsg(m_localVariables && m_variablesByName,
                  "Internal error: failed to allocate variables");
    if (m_variablesByName->find(name) != m_variablesByName->end())
      return false; // duplicate
    (*m_variablesByName)[name] = var;
    m_localVariables->push_back(var);
    return true;
  }

  void NodeImpl::finalizeConditions()
  {
    // Create conditions that may wrap user-defined conditions
    createConditionWrappers();

    //
    // *** N.B. ***
    // This should be the only place where listeners are added to conditions.
    //

    // Add listeners to local conditions
    for (size_t condIdx = skipIdx; condIdx < conditionIndexMax; ++condIdx)
      switch (condIdx) {

      case postIdx:
      case preIdx:
        break; // these conditions don't need listeners

      default:
        if (m_conditions[condIdx])
          m_conditions[condIdx]->addListener(this);
        break;
      }

    // Attach listeners to ancestor invariant and ancestor end conditions
    // Root node doesn't need them because the default conditions are constants
    if (m_parent) {
      Expression *ancestorCond = getAncestorExitCondition();
      if (ancestorCond)
        ancestorCond->addListener(this);

      ancestorCond = getAncestorInvariantCondition();
      if (ancestorCond)
        ancestorCond->addListener(this);

      ancestorCond = getAncestorEndCondition();
      if (ancestorCond)
        ancestorCond->addListener(this);
    }
  }

  void NodeImpl::addUserCondition(char const *cname, Expression *cond, bool isGarbage)
  {
    assertTrue_2(cname, "Null condition name");
    ConditionIndex which = getConditionIndex(cname);

    // This should have been checked by the parser's check pass
    assertTrueMsg(which >= skipIdx && which <= repeatIdx,
                  "Internal error: Invalid condition name \"" << cname << "\" for user condition");

    m_conditions[which] = cond;
    m_garbageConditions[which] = isGarbage;
  }

  void NodeImpl::createConditionWrappers()
  {
    this->specializedCreateConditionWrappers();
  }

  // Default method does nothing.
  void NodeImpl::specializedCreateConditionWrappers()
  {
  }

  NodeImpl::~NodeImpl() 
  {
    debugMsg("NodeImpl:~NodeImpl", " base class destructor for " << m_nodeId);

    // Remove conditions as they may refer to variables, either ours or another node's
    // Derived classes' destructors should also call this
    cleanUpConditions();

    // cleanUpNodeBody(); // NOT USEFUL here - derived classes MUST call this!

    // Now safe to delete variables
    cleanUpVars();

    // Delete timepoints, if any
    NodeTimepointValue *temp = m_timepoints;
    while (temp) {
      m_timepoints = temp->next();
      delete temp;
      temp = m_timepoints;
    }
  }

  void NodeImpl::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

    debugMsg("Node:cleanUpConditions", " for " << m_nodeId);

    // Remove listeners from ancestor invariant and ancestor end conditions
    if (m_parent) {
      Expression *ancestorCond = getAncestorExitCondition();
      if (ancestorCond)
        ancestorCond->removeListener(this);

      ancestorCond = getAncestorInvariantCondition();
      if (ancestorCond)
        ancestorCond->removeListener(this);

      ancestorCond = getAncestorEndCondition();
      if (ancestorCond)
        ancestorCond->removeListener(this);
    }

    // Remove condition listeners
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      Expression *cond = getCondition(i);
      if (cond)
        cond->removeListener(this);
    }

    // Clean up conditions
    // N.B.: Ancestor-end, ancestor-exit, and ancestor-invariant
    // MUST be cleaned up before end, exit, and invariant, respectively. 
    for (size_t i = 0; i < conditionIndexMax; ++i) {
      if (m_garbageConditions[i]) {
        debugMsg("Node:cleanUpConds",
                 "<" << m_nodeId << "> Removing condition " << getConditionName(i));
        delete m_conditions[i];
      }
      m_conditions[i] = NULL;
      m_garbageConditions[i] = false;
    }

    m_cleanedConditions = true;
  }

  // Default method.
  void NodeImpl::cleanUpNodeBody()
  {
  }

  // Called from base class destructor and possibly derived as well.
  void NodeImpl::cleanUpVars() 
  {
    if (m_cleanedVars)
      return;

    checkError(m_cleanedConditions,
               "Have to clean up variables before conditions can be cleaned.");

    debugMsg("Node:cleanUpVars", " for " << m_nodeId);

    // Delete map
    delete m_variablesByName;

    // Delete user-spec'd variables
    if (m_localVariables) {
      for (std::vector<Expression *>::iterator it = m_localVariables->begin();
           it != m_localVariables->end();
           ++it) {
        debugMsg("Node:cleanUpVars",
                 "<" << m_nodeId << "> Removing " << **it);
        delete (Expression *) (*it);
      }
      delete m_localVariables;
    }

    // Delete internal variables
    m_cleanedVars = true;
  }

  // Make the node (and its children, if any) active.
  void NodeImpl::activateNode()
  {
    // Activate conditions needed for INACTIVE state
    transitionToInactive();

    // Other initializations as required by node type
    specializedActivate();

    this->publishChange();
  }

  // Default method
  void NodeImpl::specializedActivate()
  {
  }

  Expression *NodeImpl::getCondition(size_t idx)
  {
    switch (idx) {

    case ancestorEndIdx:
    case ancestorExitIdx:
    case ancestorInvariantIdx:
      if (m_parent)
        return m_parent->m_conditions[idx];
      return NULL;

    default:
      return m_conditions[idx];
    }
  }

  Expression const *NodeImpl::getCondition(size_t idx) const
  {
    switch (idx) {

    case ancestorEndIdx:
    case ancestorExitIdx:
    case ancestorInvariantIdx:
      if (m_parent)
        return m_parent->m_conditions[idx];
      return NULL;

    default:
      return m_conditions[idx];
    }
  }

  // Default methods.
  std::vector<NodeImpl *>& NodeImpl::getChildren()
  {
    static std::vector<NodeImpl *> sl_emptyNodeVec;
    return sl_emptyNodeVec;
  }

  const std::vector<NodeImpl *>& NodeImpl::getChildren() const
  {
    static const std::vector<NodeImpl *> sl_emptyNodeVec;
    return sl_emptyNodeVec;
  }

  /**
   * @brief Notifies the node that one of its conditions has changed.
   * @note Renamed from conditionChanged.
   */

  // In addition to expressions to which this node listens, can be called by
  // ListNodeImpl::setState(), NodeImpl::setState().

  void NodeImpl::notifyChanged()
  {
    switch (m_queueStatus) {
    case QUEUE_NONE:              // add to check queue
      m_queueStatus = QUEUE_CHECK;
      g_exec->addCandidateNode(this);
      return;
      
    case QUEUE_TRANSITION:        // state transition pending, defer adding to queue
      m_queueStatus = QUEUE_TRANSITION_CHECK;
      return;

    case QUEUE_CHECK:             // already a candidate, nothing to do
    case QUEUE_TRANSITION_CHECK:  // will become a candidate after pending transition
    case QUEUE_DELETE:            // cannot possibly be a candidate, silently ignore (?)
      return;

    default:                      // Invalid queue state
      errorMsg("NodeImpl::notifyChanged for node " << m_nodeId << ": invalid queue state");
      return;
    }
  }

  /**
   * @brief Gets the destination state of this node, were it to transition, based on the values of various conditions.
   * @return True if the new destination state is different from the last check, false otherwise.
   * @note Sets m_nextState, m_nextOutcome, m_nextFailureType as a side effect.
   */
  bool NodeImpl::getDestState()
  {
    debugMsg("Node:getDestState",
             "Getting destination state for " << m_nodeId << ' ' << this << " from state " <<
             nodeStateName(m_state));

    // clear this for sake of unit test
    m_nextState = NO_NODE_STATE;

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
      errorMsg("NodeImpl::getDestState: invalid node state " << m_state);
      return false;
    }
  }

  //
  // State transition logic
  //

  void NodeImpl::transition(double time) 
  {
    // Fail silently
    if (m_nextState == m_state)
      return;

    debugMsg("Node:transition", "Transitioning " << m_nodeId << ' ' << this
             << " from " << nodeStateName(m_state)
             << " to " << nodeStateName(m_nextState)
             << " at " << std::setprecision(15) << time);
    
    transitionFrom();
    transitionTo(time);

    // Clear pending-transition variables
    m_nextState = NO_NODE_STATE;
    m_nextOutcome = NO_OUTCOME;
    m_nextFailureType = NO_FAILURE;

    condDebugMsg(m_state == FINISHED_STATE || m_state == ITERATION_ENDED_STATE,
                 "Node:outcome",
                 "Outcome of " << m_nodeId << ' ' << this <<
                 " is " << outcomeName((NodeOutcome) m_outcome));
    condDebugMsg(m_outcome == FAILURE_OUTCOME
                 && (m_state == FINISHED_STATE || m_state == ITERATION_ENDED_STATE),
                 "Node:failure",
                 "Failure type of " << m_nodeId << ' ' << this <<
                 " is " << failureTypeName((FailureType) m_failureType));

    this->publishChange();
  }

  // Common method 
  void NodeImpl::transitionFrom()
  {
    switch (m_state) {
    case INACTIVE_STATE:
      transitionFromInactive();
      break;

    case WAITING_STATE:
      transitionFromWaiting();
      break;

    case EXECUTING_STATE:
      transitionFromExecuting();
      break;

    case FINISHING_STATE:
      transitionFromFinishing();
      break;

    case FINISHED_STATE:
      transitionFromFinished();
      break;

    case FAILING_STATE:
      transitionFromFailing();
      break;

    case ITERATION_ENDED_STATE:
      transitionFromIterationEnded();
      break;

    default:
      errorMsg("NodeImpl::transitionFrom: Invalid node state " << m_state);
    }
  }

  // Common method 
  void NodeImpl::transitionTo(double time)
  {
    switch (m_nextState) {
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
      errorMsg("NodeImpl::transitionTo: Invalid destination state " << m_nextState);
    }

    setState((NodeState) m_nextState, time);
    if (m_nextOutcome != NO_OUTCOME) {
      setNodeOutcome((NodeOutcome) m_nextOutcome);
      if (m_nextFailureType != NO_FAILURE) 
        setNodeFailureType((FailureType) m_nextFailureType);
    }
    if (m_nextState == EXECUTING_STATE)
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

  // Common method
  void NodeImpl::transitionToInactive()
  {
  }

  // Common method
  bool NodeImpl::getDestStateFromInactive()
  {
    if (m_parent) {
      switch (m_parent->getState()) {

      case FINISHED_STATE:
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state) <<
                 " -> FINISHED. Parent state == FINISHED.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;

      case EXECUTING_STATE: {
        // N.B. Ancestor-exit, ancestor-invariant, ancestor-end should have been activated by parent
        bool temp;
        Expression *cond;
        if ((cond = getAncestorExitCondition())) {
          checkError(cond->isActive(),
                     "NodeImpl::getDestStateFromInactive: Ancestor exit for "
                     << m_nodeId << ' ' << this << " is inactive.");
          if (cond->getValue(temp) && temp) {
            debugMsg("Node:getDestState",
                     ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                     << " -> FINISHED. Parent EXECUTING and ANCESTOR_EXIT_CONDITION true.");
            m_nextState = FINISHED_STATE;
            m_nextOutcome = SKIPPED_OUTCOME;
            return true;
          }
        }

        if ((cond = getAncestorInvariantCondition())) {
          checkError(cond->isActive(),
                     "NodeImpl::getDestStateFromInactive: Ancestor invariant for "
                     << m_nodeId << ' ' << this << " is inactive.");
          if (cond->getValue(temp) && !temp) {
            debugMsg("Node:getDestState",
                     ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                     << " -> FINISHED. Parent EXECUTING and ANCESTOR_INVARIANT_CONDITION false.");
            m_nextState = FINISHED_STATE;
            m_nextOutcome = SKIPPED_OUTCOME;
            return true;
          }
        }

        if ((cond = getAncestorEndCondition())) {
          checkError(cond->isActive(),
                     "NodeImpl::getDestStateFromInactive: Ancestor end for "
                     << m_nodeId << ' ' << this << " is inactive.");
          if (cond->getValue(temp) && temp) {
            debugMsg("Node:getDestState",
                     ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                     << " -> FINISHED. Parent EXECUTING and ANCESTOR_END_CONDITION true.");
            m_nextState = FINISHED_STATE;
            m_nextOutcome = SKIPPED_OUTCOME;
            return true;
          }
        }

        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> WAITING. Parent state == EXECUTING.");
        m_nextState = WAITING_STATE;
        return true;
      }

      default:
        debugMsg("Node:getDestState", 
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> no change.");
        return false;
      }
    }
    else {
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> WAITING. Root node.");
      m_nextState = WAITING_STATE;
      return true;
    }
  }

  // Common method
  void NodeImpl::transitionFromInactive()
  {
    if (m_nextState == WAITING_STATE) {
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
      return;
    }
    // Only other legal transition is to FINISHED,
    // in which case no action is required.
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

  // Common method
  void NodeImpl::transitionToWaiting()
  {
    activateExitCondition();
    activatePreSkipStartConditions();
  }

  // Common method
  bool NodeImpl::getDestStateFromWaiting()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Ancestor exit for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_EXIT_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Exit condition for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. EXIT_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Ancestor invariant for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_INVARIANT_CONDITION false.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;
      }
    }

    if ((cond = getAncestorEndCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Ancestor end for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_END_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;
      }
    }

    if ((cond = getSkipCondition())) {
      checkError(cond->isActive(), 
                 "NodeImpl::getDestStateFromWaiting: Skip for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. SKIP_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = SKIPPED_OUTCOME;
        return true;
      }
    }

    if ((cond = getStartCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Start for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (!cond->getValue(temp) || !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> no change. START_CONDITION false or unknown");
        return false;
      }
    }
    if ((cond = getPreCondition()) && (!cond->getValue(temp) || !temp)) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromWaiting: Pre for "
                 << m_nodeId << ' ' << this << " is inactive.");
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> ITERATION_ENDED. START_CONDITION true and PRE_CONDITION false or unknown.");
      m_nextState = ITERATION_ENDED_STATE;
      m_nextOutcome = FAILURE_OUTCOME;
      m_nextFailureType = PRE_CONDITION_FAILED;
      return true;
    }
    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> EXECUTING. START_CONDITION and PRE_CONDITION are both true.");
    m_nextState = EXECUTING_STATE;
    return true;
  }

  // Common method
  void NodeImpl::transitionFromWaiting()
  {
    deactivatePreSkipStartConditions();
    switch (m_nextState) {

    case EXECUTING_STATE:
      deactivateAncestorEndCondition();
      break;

    case FINISHED_STATE:
      deactivateAncestorExitInvariantConditions();
      deactivateAncestorEndCondition();
      // fall through
    case ITERATION_ENDED_STATE:
      deactivateExitCondition();
      break;

    default:
      errorMsg("Attempting to transition from WAITING to invalid state "
               << nodeStateName(m_nextState));
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
  void NodeImpl::transitionToExecuting()
  {
    activateLocalVariables();

    activateInvariantCondition();
    activateEndCondition();
    activatePostCondition();
  }

  // Default method
  bool NodeImpl::getDestStateFromExecuting()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: Ancestor exit for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_EXIT_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: Exit condition for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> ITERATION_ENDED. EXIT_CONDITION true.");
        m_nextState = ITERATION_ENDED_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: Ancestor invariant for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. Ancestor invariant false.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getInvariantCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: Invariant for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> ITERATION_ENDED. Invariant false.");
        m_nextState = ITERATION_ENDED_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = INVARIANT_CONDITION_FAILED;
        return true;
      }
    }

    if ((cond = getEndCondition()) && (!cond->getValue(temp) || !temp)) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: End for "
                 << m_nodeId << ' ' << this << " is inactive.");
      return false;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> ITERATION_ENDED. End condition true.");
    m_nextState = ITERATION_ENDED_STATE;
    if ((cond = getPostCondition()) && (!cond->getValue(temp) || !temp)) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromExecuting: Post for "
                 << m_nodeId << ' ' << this << " is inactive.");
      m_nextOutcome = FAILURE_OUTCOME;
      m_nextFailureType = POST_CONDITION_FAILED;
    }
    else
      m_nextOutcome = SUCCESS_OUTCOME;
    return true;
  }

  // Empty node method
  void NodeImpl::transitionFromExecuting()
  {
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivateEndCondition();
    deactivatePostCondition();
    switch (m_nextState) {

    case FINISHED_STATE:
      deactivateAncestorExitInvariantConditions();
      break;

    case ITERATION_ENDED_STATE:
      activateAncestorEndCondition();
      break;

    default:
      errorMsg("Attempting to transition empty node from EXECUTING to invalid state "
               << nodeStateName(m_nextState));
      break;
    }

    deactivateExecutable();
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

  // Common method
  void NodeImpl::transitionToIterationEnded() 
  {
    activateRepeatCondition();
  }

  // Common method
  bool NodeImpl::getDestStateFromIterationEnded()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromIterationEnded: Ancestor exit for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_EXIT_CONDITION true.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromIterationEnded: Ancestor invariant for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_INVARIANT false.");
        m_nextState = FINISHED_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getAncestorEndCondition())) {
      checkError(cond->isActive(),
                 "NodeImpl::getDestStateFromIterationEnded: Ancestor end for "
                 << m_nodeId << ' ' << this << " is inactive.");
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. ANCESTOR_END true.");
        m_nextState = FINISHED_STATE;
        // outcome, failure type already set
        return true;
      }
    }

    if ((cond = getRepeatCondition())) {
      if (!cond->getValue(temp)) {
        checkError(cond->isActive(),
                   "NodeImpl::getDestStateFromIterationEnded: Repeat for "
                   << m_nodeId << ' ' << this << " is inactive.");
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> no change. ANCESTOR_END false or unknown and REPEAT unknown.");
        return false;
      } 
      if (temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> WAITING. REPEAT_CONDITION true.");
        m_nextState = WAITING_STATE;
        return true;
      }
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> FINISHED. REPEAT_CONDITION false.");
    m_nextState = FINISHED_STATE;
    return true;
  }

  // Common method
  void NodeImpl::transitionFromIterationEnded()
  {
    deactivateRepeatCondition();

    switch (m_nextState) {

    case FINISHED_STATE:
      deactivateAncestorExitInvariantConditions();
      deactivateAncestorEndCondition();
      return;

    case WAITING_STATE:
      reset();
      return;

    default:
      errorMsg("Attempting to transition from ITERATION_ENDED to invalid state "
               << nodeStateName(m_nextState));
      return;
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

  // Common method
  void NodeImpl::transitionToFinished()
  {
  }

  // Common method
  bool NodeImpl::getDestStateFromFinished()
  {
    if (m_parent && m_parent->getState() == WAITING_STATE) {
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> INACTIVE. Parent state == WAITING.");
      m_nextState = INACTIVE_STATE;
      return true;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> no change.");
    return false;
  }

  // Common method
  void NodeImpl::transitionFromFinished()
  {
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
  void NodeImpl::transitionToFinishing()
  {
    errorMsg("No transition to FINISHING state defined for this node");
  }

  // Default method
  bool NodeImpl::getDestStateFromFinishing()
  {
    errorMsg("Attempted to compute destination state from FINISHING for node "
             << m_nodeId << ' ' << this << " of type " << getType());
    return false;
  }

  // Default method
  void NodeImpl::transitionFromFinishing()
  {
    errorMsg("No transition from FINISHING state defined for this node");
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
  void NodeImpl::transitionToFailing()
  {
    errorMsg("No transition to FAILING state defined for this node");
  }

  // Default method
  bool NodeImpl::getDestStateFromFailing()
  {
    errorMsg("Attempted to compute destination state from FAILING for node "
             << m_nodeId << ' ' << this << " of type " << getType());
    return false;
  }

  // Default method
  void NodeImpl::transitionFromFailing()
  {
    errorMsg("No transition from FAILING state defined for this node");
  }

  // ***
  // *** END NODE STATE LOGIC ***
  // ***

  NodeState NodeImpl::getState() const {
    return (NodeState) m_state;
  }

  // Some transition handlers call this twice.
  // Called from NodeImpl::transitionTo(), ListNodeImpl::setState() (wrapper method)
  void NodeImpl::setState(NodeState newValue, double tym) // FIXME
  {
    if (newValue == m_state)
      return;
    logTransition(tym, newValue);
    m_state = newValue;
    if (m_state == FINISHED_STATE && !m_parent)
      // Mark this node as ready to be deleted -
      // with no parent, it cannot be reset, therefore cannot transition again.
      g_exec->markRootNodeFinished(this); // puts node on exec's finished queue
    else
      notifyChanged(); // check for potential of additional transitions
  }

  //
  // Transition time trace methods
  //

  void NodeImpl::logTransition(double tym, NodeState newState)
  {
    m_currentStateStartTime = tym;
    if (!m_timepoints)
      return;

    NodeTimepointValue *tp = m_timepoints;
    if (newState == INACTIVE_STATE) {
      // Reset timepoints
      while (tp) {
        tp->reset();
        tp = tp->next();
      }
      tp = m_timepoints;
    }

    // Update relevant timepoints
    while (tp) {
      if ((tp->state() == m_state && tp->isEnd())
          || (tp->state() == newState && !tp->isEnd()))
        tp->setValue(tym);
      tp = tp->next();
    }
  }

  double NodeImpl::getCurrentStateStartTime() const
  {
    return m_currentStateStartTime;
  }

  /**
   * @brief Gets the time at which this node entered the given state.
   * @param state The state.
   * @return Time value as a double.
   * @note Used by GanttListener and PlanDebugListener.
   */
  double NodeImpl::getStateStartTime(NodeState state) const
  {
    if (state == (NodeState) m_state)
      return m_currentStateStartTime;

    // Search for the desired state
    double result = -DBL_MAX; // default value if not found
    NodeTimepointValue *tp = m_timepoints;
    while (tp) {
      if (tp->state() == state && !tp->isEnd()) {
        tp->getValue(result);
        return result;
      }
      tp = tp->next();
    }

    // Not found
    return result;
  }

  void NodeImpl::setNodeOutcome(NodeOutcome o)
  {
    m_outcome = o;
  }

  NodeOutcome NodeImpl::getOutcome() const
  {
    return (NodeOutcome) m_outcome;
  }

  void NodeImpl::setNodeFailureType(FailureType f)
  {
    m_failureType = f;
  }

  FailureType NodeImpl::getFailureType() const
  {
    return (FailureType) m_failureType;
  }

  Expression *NodeImpl::ensureTimepoint(NodeState st, bool isEnd)
  {
    NodeTimepointValue *result = m_timepoints;
    while (result) {
      if (st == result->state() && isEnd == result->isEnd())
        return result;
      result = result->next();
    }

    // Not found, create it
    result = new NodeTimepointValue(this, st, isEnd);
    result->setNext(m_timepoints);
    m_timepoints = result;
    return result;
  }

  // Searches ancestors' maps when required
  Expression *NodeImpl::findVariable(char const *name)
  {
    debugMsg("Node:findVariable",
             " node " << m_nodeId << ", for " << name);
    Expression *result = NULL;
    if (m_variablesByName) {
      result = m_variablesByName->findVariable(name); // searches ancestor maps
      condDebugMsg(result,
                   "Node:findVariable",
                   " node " << m_nodeId << " returning " << result->toString());
      condDebugMsg(!result,
                   "Node:findVariable",
                   " node " << m_nodeId << " not found in local map");
      return result;
    }
    else if (m_parent) {
      NodeVariableMap const *map = m_parent->getChildVariableMap();
      if (map) {
        result = map->findVariable(name);
        condDebugMsg(result,
                     "Node:findVariable",
                     " node " << m_nodeId
                     << " returning " << result->toString() << " from ancestor map");
        condDebugMsg(!result,
                     "Node:findVariable",
                     " node " << m_nodeId  << " not found in ancestor map");
        return result;
      }
    }
    // else fall through
    debugMsg("Node:findVariable",
             " node " << m_nodeId
             << " not found, no local map and no ancestor map");
    return NULL;
  }

  Expression *NodeImpl::findLocalVariable(char const *name)
  {
    if (!m_variablesByName)
      return NULL;

    NodeVariableMap::const_iterator it = m_variablesByName->find(name);
    if (it != m_variablesByName->end()) {
      debugMsg("Node:findLocalVariable",
               ' ' << m_nodeId << " Returning " << it->second->toString());
      return it->second;
    }

    debugMsg("Node:findLocalVariable", ' ' << m_nodeId << ' ' << name << " not found");
    return NULL;
  }

  // Default methods
  NodeImpl const *NodeImpl::findChild(char const * /* childName */) const
  {
    return NULL;
  }

  NodeImpl *NodeImpl::findChild(char const * /* childName */)
  {
    return NULL;
  }

  //
  // Conditions
  //

  // These are special because parent owns the condition expression
  void NodeImpl::activateAncestorEndCondition()
  {
  }
  void NodeImpl::activateAncestorExitInvariantConditions()
  {
  }

  // User conditions
  void NodeImpl::activatePreSkipStartConditions()
  {
    Expression *temp;
    if ((temp = m_conditions[skipIdx]))
      temp->activate();
    if ((temp = m_conditions[startIdx]))
      temp->activate();
    if ((temp = m_conditions[preIdx]))
      temp->activate();
  }

  void NodeImpl::activateEndCondition()
  {
    if (m_conditions[endIdx])
      m_conditions[endIdx]->activate();
  }

  void NodeImpl::activateExitCondition()
  {
    if (m_conditions[exitIdx])
      m_conditions[exitIdx]->activate();
  }

  void NodeImpl::activateInvariantCondition()
  {
    if (m_conditions[invariantIdx])
      m_conditions[invariantIdx]->activate();
  }

  void NodeImpl::activatePostCondition()
  {
    if (m_conditions[postIdx])
      m_conditions[postIdx]->activate();
  }

  void NodeImpl::activateRepeatCondition()
  {
    if (m_conditions[repeatIdx])
      m_conditions[repeatIdx]->activate();
  }

  // These are for specialized node types
  void NodeImpl::activateActionCompleteCondition()
  {
    assertTrueMsg(m_conditions[actionCompleteIdx],
                  "No ActionCompleteCondition exists in node \"" << m_nodeId << "\"");
    m_conditions[actionCompleteIdx]->activate();
  }

  void NodeImpl::activateAbortCompleteCondition()
  {
    assertTrueMsg(m_conditions[abortCompleteIdx],
                  "No AbortCompleteCondition exists in node \"" << m_nodeId << "\"");
    m_conditions[abortCompleteIdx]->activate();
  }

  // These are special because parent owns the condition expression
  void NodeImpl::deactivateAncestorEndCondition()
  {
  }
  void NodeImpl::deactivateAncestorExitInvariantConditions()
  {
  }

  // User conditions
  void NodeImpl::deactivatePreSkipStartConditions()
  {
    Expression *temp;
    if ((temp = m_conditions[skipIdx]))
      temp->deactivate();
    if ((temp = m_conditions[startIdx]))
      temp->deactivate();
    if ((temp = m_conditions[preIdx]))
      temp->deactivate();
  }

  void NodeImpl::deactivateEndCondition()
  {
    if (m_conditions[endIdx])
      m_conditions[endIdx]->deactivate();
  }

  void NodeImpl::deactivateExitCondition()
  {
    if ((m_conditions[exitIdx]))
      m_conditions[exitIdx]->deactivate();
  }

  void NodeImpl::deactivateInvariantCondition()
  {
    if ((m_conditions[invariantIdx]))
      m_conditions[invariantIdx]->deactivate();
  }

  void NodeImpl::deactivatePostCondition()
  {
    if ((m_conditions[postIdx]))
      m_conditions[postIdx]->deactivate();
  }

  void NodeImpl::deactivateRepeatCondition()
  {
    if (m_conditions[repeatIdx])
      m_conditions[repeatIdx]->deactivate();
  }

  // These are for specialized node types
  void NodeImpl::deactivateActionCompleteCondition()
  {
    m_conditions[actionCompleteIdx]->deactivate();
  }

  void NodeImpl::deactivateAbortCompleteCondition()
  {
    m_conditions[abortCompleteIdx]->deactivate();
  }

  void NodeImpl::activateLocalVariables()
  {
    if (m_localVariables) {
      for (std::vector<Expression *>::iterator vit = m_localVariables->begin();
           vit != m_localVariables->end();
           ++vit)
        (*vit)->activate();
    }
  }

  void NodeImpl::deactivateLocalVariables()
  {
    if (m_localVariables) {
      for (std::vector<Expression *>::iterator vit = m_localVariables->begin();
           vit != m_localVariables->end();
           ++vit)
        (*vit)->deactivate();
    }
  }

  void NodeImpl::execute() 
  {
    debugMsg("Node:execute", " Executing node " << m_nodeId);

    // legacy message for unit test
    debugMsg("PlexilExec:handleNeedsExecution",
             " Storing action for node " << m_nodeId << ' ' << this <<
             " of type " << nodeTypeString(this->getType()) << 
             " to be executed.");

    specializedHandleExecution();
  }

  // default method
  void NodeImpl::specializedHandleExecution()
  {
  }

  void NodeImpl::reset()
  {
    debugMsg("Node:reset", " Re-setting node " << m_nodeId);

    //reset outcome and failure type
    m_outcome = NO_OUTCOME;
    m_failureType = NO_FAILURE;
  }

  // Default method
  void NodeImpl::abort() 
  {
    errorMsg("Abort illegal for node type " << getType());
  }

  void NodeImpl::deactivateExecutable() 
  {
    specializedDeactivateExecutable();
    deactivateLocalVariables();
  }

  // Default method
  void NodeImpl::specializedDeactivateExecutable()
  {
  }

  std::string NodeImpl::toString(const unsigned int indent)
  {
    std::ostringstream retval;
    print(retval, indent);
    return retval.str();
  }

  // Defined here because Node.cc is no more
  std::ostream& operator<<(std::ostream &stream, Node const &node)
  {
    node.print(stream, 0);
    return stream;
  }

  void NodeImpl::print(std::ostream& stream, const unsigned int indent) const
  {
    std::string indentStr(indent, ' ');

    stream << indentStr << m_nodeId << "{\n";
    stream << indentStr << " State: " << nodeStateName(m_state) <<
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
        if (getCondition(i))
        stream << indentStr << ' ' << getConditionName(i) << ": "
               << getCondition(i)->toString()
               << '\n';
      }
      // Print variables, starting with command handle
      printCommandHandle(stream, indent);
      printVariables(stream, indent);
    }
    // print children
    for (std::vector<NodeImpl *>::const_iterator it = getChildren().begin();
         it != getChildren().end();
         ++it)
      (*it)->print(stream, indent + 2);
    stream << indentStr << "}" << std::endl;
  }

  // Default method does nothing
  void NodeImpl::printCommandHandle(std::ostream& /* stream */, const unsigned int /* indent */) const
  {
  }

  // Print variables
  void NodeImpl::printVariables(std::ostream& stream, const unsigned int indent) const
  {
    if (!m_variablesByName)
      return;
    
    std::string indentStr(indent, ' ');
    for (NodeVariableMap::const_iterator it = m_variablesByName->begin();
         it != m_variablesByName->end();
         ++it) {
      stream << indentStr << ' ' << it->first << ": " <<
        *(it->second) << '\n';
    }
  }



}
