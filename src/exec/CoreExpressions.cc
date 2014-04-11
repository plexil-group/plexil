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


#include "CoreExpressions.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "Debug.hh"
#include "Node.hh"
#include "lifecycle-utils.h"

#include <cmath> // for fabs()
#include <string>
#include <sstream>

namespace PLEXIL 
{

  //
  // StateVariable
  //

  // Init class static variable
  std::vector<Value> *StateVariable::s_allStateNames = NULL;

  // Called from Node::commonInit().
  StateVariable::StateVariable(const std::string& name)
    : VariableImpl((uint32_t) INACTIVE_STATE, false)
  {
    setName(name);
  }

  // Used only to construct class constants INACTIVE_EXP(), et al.
  StateVariable::StateVariable(const uint32_t value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
               "Attempted to initialize a state variable with invalid value " << value);
  }

  // ExpressionFactory entry point. Should only be used to construct literals.
  StateVariable::StateVariable(const PlexilExprId& expr, 
                               const NodeConnectorId& node,
                               const bool isConst)
    : VariableImpl(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    checkError(isConst, "Cannot construct a freestanding NodeStateVariable.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_STATE,
               "Expected NodeState value.  Found '" << PlexilParser::valueTypeString(val->type()) << "'");
    Value value(nameToNodeState(LabelStr(val->value())));
    checkError(checkValue(value),
               "Attempted to initialize a state variable with invalid value " << val->value());
    m_value = m_initialValue = value;
  }

  bool StateVariable::checkValue(const Value& val) const
  {
    if (!val.isInteger())
      return false;
    uint32_t valAsInt = val.getUIntValue();
    return valAsInt >= INACTIVE_STATE && valAsInt < NO_NODE_STATE;
  }

  void StateVariable::print(std::ostream& s) const 
  {
    VariableImpl::print(s);
    s << "state)";
  }

  void StateVariable::printValue(std::ostream& s) const
  {
    s << nodeStateName(getValue().getUIntValue());
  }

  void StateVariable::setNodeState(uint32_t newValue)
  {
    checkError(newValue < NO_NODE_STATE,
               "Attempted to set an invalid NodeState value");
    this->setValue(newValue);
  }

  // Must be in same order as enum NodeState. See ExecDefs.hh.
  const std::vector<Value>& StateVariable::ALL_STATE_NAMES()
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      s_allStateNames = new std::vector<Value>();
      addFinalizer(&purgeAllStateNames);
      s_allStateNames->reserve(NODE_STATE_MAX);
      s_allStateNames->push_back(INACTIVE());
      s_allStateNames->push_back(WAITING());
      s_allStateNames->push_back(EXECUTING());
      s_allStateNames->push_back(ITERATION_ENDED());
      s_allStateNames->push_back(FINISHED());
      s_allStateNames->push_back(FAILING());
      s_allStateNames->push_back(FINISHING());
      s_allStateNames->push_back(NO_STATE());
      sl_inited = true;
    }
    return *s_allStateNames;
  }

  void StateVariable::purgeAllStateNames()
  {
    delete s_allStateNames;
    s_allStateNames = NULL;
  }

  const Value& StateVariable::nodeStateName(uint32_t state)
  {
    return ALL_STATE_NAMES()[state];
  }

  // N.B. Depends on ALL_STATE_NAMES() matching order of NodeState enumeration.
  uint32_t StateVariable::nameToNodeState(const LabelStr& stateName)
  {
    for (uint32_t s = INACTIVE_STATE; s < NO_NODE_STATE; ++s) {
      if (stateName == ALL_STATE_NAMES()[s])
        return s;
    }
    return NO_NODE_STATE;
  }

  // Called only by Node::commonInit().
  OutcomeVariable::OutcomeVariable(const std::string& name)
    : VariableImpl(false)
  {
    setName(name);
  }

  // ExpressionFactory entry point. Should only be used to construct literals.
  OutcomeVariable::OutcomeVariable(const PlexilExprId& expr, 
                                   const NodeConnectorId& node,
                                   const bool isConst)
    : VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    checkError(isConst, "Cannot construct a freestanding NodeOutcomeVariable.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_OUTCOME,
               "Expected NodeOutcome value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    Value value(val->value());
    checkError(checkValue(value),
               "Attempted to initialize a variable with an invalid value.");
    m_value = m_initialValue = value;
  }

  bool OutcomeVariable::checkValue(const Value& val) const
  {
    return val == UNKNOWN()
      || val == SUCCESS()
      || val == FAILURE()
      || val == SKIPPED()
      || val == INTERRUPTED();
  }

  void OutcomeVariable::print(std::ostream& s) const
  {
    VariableImpl::print(s);
    s << "outcome)";
  }


  // Called only from Node::commonInit().
  FailureVariable::FailureVariable(const std::string& name)
    : VariableImpl(false)
  {
    setName(name);
  }

  // ExpressionFactory entry point. Should only be used to construct literals.
  FailureVariable::FailureVariable(const PlexilExprId& expr, 
                                   const NodeConnectorId& node,
                                   const bool isConst)
    : VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    checkError(isConst, "Cannot construct a freestanding NodeFailureTypeVariable.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::FAILURE_TYPE,
               "Expected NodeFailure value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    Value value(val->value());
    checkError(checkValue(value),
               "Attempted to initialize a variable with an invalid value.");
    m_value = m_initialValue = value;
  }

  void FailureVariable::print(std::ostream& s) const
  {
    VariableImpl::print(s);
    s << "failure)";
  }

  bool FailureVariable::checkValue(const Value& val) const
  {
    return val == UNKNOWN()
      || val == PRE_CONDITION_FAILED()
      || val == POST_CONDITION_FAILED()
      || val == INVARIANT_CONDITION_FAILED()
      || val == PARENT_FAILED()
      || val == PARENT_EXITED()
      || val == EXITED();
  }

  // Called only from Command constructors.
  CommandHandleVariable::CommandHandleVariable(const std::string& name)
    : VariableImpl()
  {
    setName(name);
  }

  // ExpressionFactory entry point. Should only be used to construct literals.
  CommandHandleVariable::CommandHandleVariable(const PlexilExprId& expr, 
                                               const NodeConnectorId& node,
                                               const bool isConst)
    : VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    checkError(isConst, "Cannot construct a freestanding NodeCommandHandleVariable.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::COMMAND_HANDLE,
               "Expected NodeCommandHandle value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    Value value(val->value());
    checkError(checkValue(value),
               "Attempted to initialize a variable with an invalid value.");
    m_value = m_initialValue = value;
  }

  bool CommandHandleVariable::checkValue(const Value& val) const
  {
    return val == UNKNOWN()
      || val == COMMAND_SENT_TO_SYSTEM()
      || val == COMMAND_ACCEPTED()
      || val == COMMAND_RCVD_BY_SYSTEM()
      || val == COMMAND_SUCCESS()
      || val == COMMAND_DENIED()
      || val == COMMAND_FAILED();
  }

  void CommandHandleVariable::print(std::ostream& s) const 
  {
    VariableImpl::print(s);
    s << "command_handle)";
  }

  AllChildrenFinishedCondition::AllChildrenFinishedCondition(const std::vector<NodeId>& children)
    : Calculable(), 
      m_total(children.size()),
      m_count(0),
      m_stateVariables(children.size(), VariableId::noId()),
      m_childListeners(children.size(), FinishedListener(*this))
  {
    for (size_t i = 0; i < m_total; ++i) {
      const NodeId& child = children[i];
      check_error(child.isValid());
      VariableId sv = m_stateVariables[i] = child->getStateVariable();
      check_error(sv.isValid());
      sv->addListener(m_childListeners[i].getId());
    }
    internalSetValue(recalculate());
  }

  AllChildrenFinishedCondition::~AllChildrenFinishedCondition() 
  {
    for (size_t i = 0; i < m_total; ++i) {
      m_stateVariables[i]->removeListener(m_childListeners[i].getId());
    }
  }

  void AllChildrenFinishedCondition::incrementCount() 
  {
    ++m_count;
    checkError(m_count <= m_total,
               "Internal error: somehow counted more nodes in finished than were actually there.");
    if (m_count == m_total) {
      debugMsg("AllChildrenFinished:increment",
               "Counted " << m_count << " children finished of " << m_total <<
               ".  Setting TRUE.");
      internalSetValue(BooleanVariable::TRUE_VALUE());
    }
  }

  void AllChildrenFinishedCondition::decrementCount()
  {
    checkError(m_count > 0,
               "Internal error: somehow counted more nodes unfinished than were actually there.");
    --m_count;
    if (getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("AllChildrenFinished:decrement",
               m_count << " children of " << m_total << " are FINISHED.  Setting FALSE.");
      internalSetValue(BooleanVariable::FALSE_VALUE());
    }
  }

  void AllChildrenFinishedCondition::handleActivate(const bool changed)
  {
    if (changed) {
      for (size_t i = 0 ; i < m_total; ++i)
        m_childListeners[i].activate();
    }
    Calculable::handleActivate(changed);
  }

  void AllChildrenFinishedCondition::handleDeactivate(const bool changed)
  {
    if (changed) {
      for (size_t i = 0 ; i < m_total; ++i)
        m_childListeners[i].deactivate();
    }
    Calculable::handleDeactivate(changed);
  }

  Value AllChildrenFinishedCondition::recalculate()
  {
    m_count = 0;
    for (size_t i = 0; i < m_total; ++i) {
      uint32_t state = m_stateVariables[i]->getValue().getUIntValue();
      m_childListeners[i].setLastState(state);
      if (state == FINISHED_STATE)
        ++m_count;
    }
    if (m_count == m_total) {
      debugMsg("AllChildrenFinished:recalculate",
               "Counted " << m_count << " of " << m_total <<
               " children FINISHED.  Setting TRUE.");
      return BooleanVariable::TRUE_VALUE();
    }
    else {
      debugMsg("AllChildrenFinished:recalculate",
               "Counted " << m_count << " of " << m_total <<
               " children FINISHED.  Setting FALSE.");
      return BooleanVariable::FALSE_VALUE();
    }
  }

  bool AllChildrenFinishedCondition::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  AllChildrenFinishedCondition::FinishedListener::FinishedListener(AllChildrenFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond), m_lastState(NO_NODE_STATE)
  {
  }

  AllChildrenFinishedCondition::FinishedListener::FinishedListener(const FinishedListener& orig)
    : ExpressionListener(), m_cond(orig.m_cond), m_lastState(orig.m_lastState)
  {
  }

  void
  AllChildrenFinishedCondition::FinishedListener::notifyValueChanged(const ExpressionId& expression) 
  {
    uint32_t newState = expression->getValue().getUIntValue();
    if (newState == FINISHED_STATE && m_lastState != newState) {
      debugMsg("AllChildrenFinished:increment",
               "State var " << *expression << " is now FINISHED.  Incrementing count.");
      m_cond.incrementCount();
    }
    else if (m_lastState == FINISHED_STATE && m_lastState != newState) {
      debugMsg("AllChildrenFinished:decrement",
               "State var " << *expression << " is no longer FINISHED.  Decrementing count.");
      m_cond.decrementCount();
    }
    m_lastState = newState;
  }

  void AllChildrenFinishedCondition::print(std::ostream& s) const 
  {
    Expression::print(s);
    s << "childrenFinished(" << m_count << ":" << m_total << "))";
  }

  /***************************/

  AllChildrenWaitingOrFinishedCondition::AllChildrenWaitingOrFinishedCondition(const std::vector<NodeId>& children)
    : Calculable(),
      m_total(children.size()),
      m_count(0),
      m_stateVariables(children.size(), VariableId::noId()),
      m_childListeners(children.size(), WaitingOrFinishedListener(*this))
  {
    for (size_t i = 0; i < m_total; ++i) {
      const NodeId& child = children[i];
      check_error(child.isValid());
      VariableId sv = m_stateVariables[i] = child->getStateVariable();
      check_error(sv.isValid());
      sv->addListener(m_childListeners[i].getId());
    }
    internalSetValue(recalculate());
  }

  AllChildrenWaitingOrFinishedCondition::~AllChildrenWaitingOrFinishedCondition()
  {
    for (size_t i = 0; i < m_total; ++i) {
      m_stateVariables[i]->removeListener(m_childListeners[i].getId());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::incrementCount()
  {
    ++m_count;
    checkError(m_count <= m_total,
               "Internal error: somehow counted more nodes in finished than were actually there.");
    if (m_count == m_total) {
      debugMsg("AllChildrenWaitingOrFinished:increment",
               "Counted " << m_count << " children waiting or finished of " << m_total <<
               ".  Setting TRUE.");
      internalSetValue(BooleanVariable::TRUE_VALUE());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::decrementCount()
  {
    checkError(m_count > 0,
               "Internal error: somehow counted more nodes unfinished than were actually there.");
    --m_count;
    if (getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("AllChildrenWaitingOrFinished:decrement",
               m_count << " children of " << m_total << " are WAITING or FINISHED.  Setting FALSE.");
      internalSetValue(BooleanVariable::FALSE_VALUE());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::handleActivate(const bool changed)
  {
    if (changed) {
      for (size_t i = 0 ; i < m_total; ++i)
        m_childListeners[i].activate();
    }
    Calculable::handleActivate(changed);
  }

  void AllChildrenWaitingOrFinishedCondition::handleDeactivate(const bool changed)
  {
    if (changed) {
      for (size_t i = 0 ; i < m_total; ++i)
        m_childListeners[i].deactivate();
    }
    Calculable::handleDeactivate(changed);
  }

  Value AllChildrenWaitingOrFinishedCondition::recalculate()
  {
    m_count = 0;
    for (size_t i = 0; i < m_total; ++i) {
      uint32_t state = m_stateVariables[i]->getValue().getUIntValue();
      m_childListeners[i].setLastState(state);
      if (state == FINISHED_STATE || state == WAITING_STATE)
        ++m_count;
    }
    if (m_count == m_total) {
      debugMsg("AllChildrenWaitingOrFinished:recalculate",
               "Counted " << m_count << " of " << m_total <<
               " children WAITING or FINISHED.  Setting TRUE.");
      return BooleanVariable::TRUE_VALUE();
    }
    else {
      debugMsg("AllChildrenWaitingOrFinished:recalculate",
               "Counted " << m_count << " of " << m_total <<
               " children WAITING or FINISHED.  Setting FALSE.");
      return BooleanVariable::FALSE_VALUE();
    }
  }

  bool AllChildrenWaitingOrFinishedCondition::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond), m_lastState(NO_NODE_STATE)
  {
  }

  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::WaitingOrFinishedListener(const WaitingOrFinishedListener& orig)
    : ExpressionListener(), m_cond(orig.m_cond), m_lastState(orig.m_lastState)
  {
  }

  void 
  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::notifyValueChanged(const ExpressionId& expression)
  {
    bool was = m_lastState == WAITING_STATE || m_lastState == FINISHED_STATE;
    uint32_t newState = expression->getValue().getUIntValue();
    bool is = newState == WAITING_STATE || newState == FINISHED_STATE;
    if (is && !was) {
      debugMsg("AllChildrenWaitingOrFinished:increment",
               "State var " << *expression << " is now WAITING or FINISHED.  Incrementing count.");
      m_cond.incrementCount();
    }
    else if (was && !is) {
      debugMsg("AllChildrenWaitingOrFinished:decrement",
               "State var " << *expression << " is no longer WAITING orFINISHED.  Decrementing count.");
      m_cond.decrementCount();
    }
    m_lastState = newState;
  }

  void AllChildrenWaitingOrFinishedCondition::print(std::ostream& s) const
  {
    Expression::print(s);
    s << "childrenWaitingOrFinished(" << m_count << ":" << m_total << "))";
  }

  /*
    <(N?)EQInternal>
    <NodeStateVariable> ||
    <NodeStateValue>
    OR
    <NodeOutcomeVariable> ||
    <NodeOutcomeValue>
    </(N?)EQInternal>
  */

  InternalCondition::InternalCondition(const PlexilExprId& /* xml */)
    : Calculable() {
    checkError(ALWAYS_FAIL, "Internal conditions require a Node argument");
  }

  InternalCondition::InternalCondition(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable() {
    checkError(Id<PlexilOp>::convertable(expr), "Expected an op.");
    PlexilOp* op = (PlexilOp*) expr;
    checkError(op->getOp() == "EQInternal" || op->getOp() == "NEInternal",
               "Expected EQInternal or NEInternal");

    bool isFirst = true;
    ExpressionId firstExpr, secondExpr;
    for (std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin();
         it != op->subExprs().end();
         ++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      debugMsg("InternalCondition:InternalCondition",
               "Adding " << *subExpr << " as " << (garbage ? "" : "non-") << " garbage.");
      if (garbage)
        m_garbage.insert(subExpr);
      if (isFirst) {
        firstExpr = subExpr;
        isFirst = false;
      }
      else
        secondExpr = subExpr;
    }

    checkError(firstExpr.isValid() && secondExpr.isValid(),
               "Expected two subexpressions in " << expr->name());

    //m_subexpressions.clear();

    if(op->getOp() == "EQInternal")
      m_expr = (new Equality(firstExpr, secondExpr))->getId();
    else if(op->getOp() == "NEInternal")
      m_expr = (new Inequality(firstExpr, secondExpr))->getId();
    addSubexpression(m_expr, false);
  }

  InternalCondition::~InternalCondition() {
    removeSubexpression(m_expr);
    delete (Expression*) m_expr;
  }

  Value InternalCondition::recalculate()
  {
    return m_expr->getValue();
  }

  bool InternalCondition::checkValue(const Value& val) const
  {
    return val.isUnknown()
      || val == BooleanVariable::FALSE_VALUE()
      || val == BooleanVariable::TRUE_VALUE();
  }

  void InternalCondition::print(std::ostream& s) const 
  {
    Expression::print(s);
    s << *m_expr << ")";
  }

  InterruptibleCommandHandleValues::InterruptibleCommandHandleValues(const PlexilExprId& expr, 
                                                                     const NodeConnectorId& node)
    : UnaryExpression(expr, node)
  {
  }

  InterruptibleCommandHandleValues::InterruptibleCommandHandleValues(ExpressionId e)
    : UnaryExpression(e)
  {
  }

  void InterruptibleCommandHandleValues::print(std::ostream& s) const 
  {
    Expression::print(s);
    s << "interruptibleCommandHandleValues(" << *m_e << "))";
  }

  Value InterruptibleCommandHandleValues::recalculate()
  {
    const Value& v = m_e->getValue();
    if (v.isUnknown())
      return BooleanVariable::FALSE_VALUE();
    else if ((v == CommandHandleVariable::COMMAND_DENIED()) || 
             (v == CommandHandleVariable::COMMAND_FAILED()))
      return BooleanVariable::TRUE_VALUE();
    return BooleanVariable::FALSE_VALUE();
  }

  bool InterruptibleCommandHandleValues::checkValue(const Value& val) const
  {
    return val == BooleanVariable::TRUE_VALUE()
      || val == BooleanVariable::FALSE_VALUE();
  }

}
