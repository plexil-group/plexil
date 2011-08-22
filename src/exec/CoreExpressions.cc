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


#include "CoreExpressions.hh"
#include "Calculables.hh"
#include "Debug.hh"
#include "Node.hh"
#include "Variables.hh"

#include <cmath> // for fabs()
#include <list>
#include <string>
#include <sstream>
#include <stdint.h> // for int32_t

namespace PLEXIL 
{

  //
  // StateVariable
  //

  StateVariable::StateVariable(const bool isConst) : VariableImpl(INACTIVE(), isConst) {}

  StateVariable::StateVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
	       "Attempted to initialize a state variable with invalid value "
	       << Expression::valueToString(value));
  }

  StateVariable::StateVariable(const PlexilExprId& expr, 
							   const NodeConnectorId& node,
							   const bool isConst)
	: VariableImpl(expr, node, isConst) 
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_STATE,
	       "Expected NodeState value.  Found '" << PlexilParser::valueTypeString(val->type()) << "'");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a state variable with invalid value "
	       << Expression::valueToString(value));
  }

  // N.B. Depends on ALL_STATES() matching order of NodeState enumeration.
  bool StateVariable::checkValue(const double val) {
    for (size_t s = INACTIVE_STATE; s < NO_NODE_STATE; s++) {
      if (val == ALL_STATES()[s])
	return true;
    }
    return false;
  }

  std::string StateVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "state(" << LabelStr(m_value).toString() << "))";
    return retval.str();
  }

  void StateVariable::setNodeState(NodeState newValue)
  {
    checkError(newValue < NO_NODE_STATE,
	       "Attempted to set an invalid NodeState value");
    this->setValue(ALL_STATES()[newValue].getKey());
  }

  const std::vector<LabelStr>& StateVariable::ALL_STATES() {
    static std::vector<LabelStr>* allStates = NULL;
    if (allStates == NULL) {
      allStates = new std::vector<LabelStr>();
      allStates->reserve(NODE_STATE_MAX);
      allStates->push_back(INACTIVE());
      allStates->push_back(WAITING());
      allStates->push_back(EXECUTING());
      allStates->push_back(FINISHING());
      allStates->push_back(FINISHED());
      allStates->push_back(FAILING());
      allStates->push_back(ITERATION_ENDED());
      allStates->push_back(NO_STATE());
    }
    return *allStates;
  }

  ExpressionId& StateVariable::INACTIVE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(INACTIVE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::WAITING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(WAITING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::EXECUTING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(EXECUTING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FINISHING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FINISHED_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FINISHED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::FAILING_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(FAILING(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::ITERATION_ENDED_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(ITERATION_ENDED(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }
  ExpressionId& StateVariable::NO_STATE_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new StateVariable(NO_STATE(), true))->getId();
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  const LabelStr& StateVariable::nodeStateName(NodeState state)
  {
    return ALL_STATES()[state];
  }

  NodeState StateVariable::nodeStateFromName(double nameAsLabelStrKey)
  {
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++)
      if (ALL_STATES()[s].getKey() == nameAsLabelStrKey)
	return (NodeState) s;
    return NO_NODE_STATE;
  }


  OutcomeVariable::OutcomeVariable(const bool isConst) : VariableImpl(isConst) {}
  OutcomeVariable::OutcomeVariable(const double value, const bool isConst)
    : VariableImpl(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  OutcomeVariable::OutcomeVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::NODE_OUTCOME,
	       "Expected NodeOutcome value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool OutcomeVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == SUCCESS() || val == FAILURE() || val == SKIPPED());
  }

  std::string OutcomeVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " outcome)";
    return retval.str();
  }


  FailureVariable::FailureVariable(const bool isConst) : VariableImpl(isConst) {}

  FailureVariable::FailureVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  FailureVariable::FailureVariable(const PlexilExprId& expr, 
								   const NodeConnectorId& node,
								   const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::FAILURE_TYPE,
	       "Expected NodeFailure value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  std::string FailureVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " failure)";
    return retval.str();
  }

  bool FailureVariable::checkValue(const double val) {
    return val == UNKNOWN()
	  || val == PRE_CONDITION_FAILED()
	  || val == POST_CONDITION_FAILED()
	  || val == INVARIANT_CONDITION_FAILED()
	  || val == PARENT_FAILED();
  }

  CommandHandleVariable::CommandHandleVariable(const bool isConst) : VariableImpl(isConst) {}
  CommandHandleVariable::CommandHandleVariable(const double value, const bool isConst)
    : VariableImpl(isConst) {
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }
  CommandHandleVariable::CommandHandleVariable(const PlexilExprId& expr, 
											   const NodeConnectorId& node,
                                               const bool isConst)
	: VariableImpl(expr, node, isConst)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    PlexilValue* val = (PlexilValue*) expr;
    checkError(val->type() == PLEXIL::COMMAND_HANDLE,
	       "Expected NodeCommandHandle value.  Found " << PlexilParser::valueTypeString(val->type()) << ".");
    LabelStr value(val->value());
    m_value = m_initialValue = value;
    checkError(checkValue(value),
	       "Attempted to initialize a variable with an invalid value.");
  }

  bool CommandHandleVariable::checkValue(const double val) {
    return (val == UNKNOWN() || val == COMMAND_SENT_TO_SYSTEM() || val == COMMAND_ACCEPTED() || val == COMMAND_RCVD_BY_SYSTEM() || val == COMMAND_SUCCESS() || val == COMMAND_DENIED() || val == COMMAND_FAILED());
  }

  std::string CommandHandleVariable::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << " command_handle)";
    return retval.str();
  }

  AllChildrenFinishedCondition::AllChildrenFinishedCondition(std::vector<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::vector<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenFinishedCondition::~AllChildrenFinishedCondition() {
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      if(node->getStateName() == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(expr->getValue() == StateVariable::FINISHED() &&
       m_lastValues[expr] != StateVariable::FINISHED()) {
      debugMsg("AllChildrenFinished:increment",
	       "State var " << expr->toString() << " is now FINISHED.  Incrementing count.");
      m_count++;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in finished than were actually there.");
      if(m_count == m_total) {
	debugMsg("AllChildrenFinished:increment",
		 "Counted " << m_count << " children finished of " << m_total <<
		 ".  Setting TRUE.");
	internalSetValue(BooleanVariable::TRUE());
      }
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenFinishedCondition::decrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(expr->getValue() != StateVariable::FINISHED() &&
       m_lastValues[expr] == StateVariable::FINISHED()) {
      debugMsg("AllChildrenFinished:decrement",
	       "State var " << expr->toString() <<
	       " is no longer FINISHED.  Decrementing count.");
      m_count--;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in finished than were actually there.");
      if(getValue() == BooleanVariable::TRUE()) {
	debugMsg("AllChildrenFinished:decrement",
		 m_count << " children of " << m_total << " are FINISHED.  Setting FALSE.");
	internalSetValue(BooleanVariable::FALSE());
      }
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenFinishedCondition::handleActivate(const bool changed) {
    if(changed)
      m_listener.activate();
    Calculable::handleActivate(changed);
  }

  void AllChildrenFinishedCondition::handleDeactivate(const bool changed) {
    if(changed)
      m_listener.deactivate();
    Calculable::handleDeactivate(changed);
  }

  double AllChildrenFinishedCondition::recalculate() {
    m_count = 0;
    for(std::vector<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = UNKNOWN();
      m_lastValues[expr] = expr->getValue();

      if(m_lastValues[expr] == StateVariable::FINISHED())
	++m_count;
    }
    checkError(m_count <= m_total,
	       "Error: somehow counted more nodes in waiting or finished (" << m_count <<
	       ") than were actually there (" << m_total << ").");
    if(m_count == m_total) {
      debugMsg("AllChildrenFinished:recalculate",
	       "Counted " << m_count << " of " << m_total <<
	       " children FINISHED.  Setting TRUE.");
      return BooleanVariable::TRUE();
    }
    else {
      debugMsg("AllChildrenFinished:recalculate",
	       "Counted " << m_count << " of " << m_total <<
	       " children FINISHED.  Setting FALSE.");
      return BooleanVariable::FALSE();
    }
  }

  bool AllChildrenFinishedCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  AllChildrenFinishedCondition::FinishedListener::FinishedListener(AllChildrenFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond) {}

  void
  AllChildrenFinishedCondition::FinishedListener::notifyValueChanged(const ExpressionId& expression) 
  {
    checkError(dynamic_cast<const StateVariable*>((const Expression*)expression) != NULL,
	       "Finished listener not listening on a state variable.");
    if(expression->getValue() == StateVariable::FINISHED())
      m_cond.incrementCount(expression);
    else
      m_cond.decrementCount(expression);
  }

  std::string AllChildrenFinishedCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "childrenFinished(" << m_count << ":" << m_total << "))";
    return retval.str();
  }

  /***************************/

  AllChildrenWaitingOrFinishedCondition::AllChildrenWaitingOrFinishedCondition(std::vector<NodeId>& children)
    : Calculable(), m_listener(*this), m_total(0), m_count(0), m_constructed(false) {
    for(std::vector<NodeId>::iterator it = children.begin(); it != children.end(); ++it) {
      NodeId child = *it;
      check_error(child.isValid());
      addChild(child);
    }
    internalSetValue(recalculate());
    m_constructed = true;
  }

  AllChildrenWaitingOrFinishedCondition::~AllChildrenWaitingOrFinishedCondition() {
    for(std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      NodeId child = *it;
      child->getStateVariable()->removeListener(m_listener.getId());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::addChild(const NodeId& node) {
    m_children.push_back(node);
    ++m_total;
    node->getStateVariable()->addListener(m_listener.getId());
    if(m_constructed) {
      const LabelStr& state = node->getStateName();
      if (state == StateVariable::WAITING() ||
	  state == StateVariable::FINISHED())
	incrementCount(node->getStateVariable());
      else if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
  }

  void AllChildrenWaitingOrFinishedCondition::incrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if((expr->getValue() == StateVariable::WAITING() || expr->getValue() == StateVariable::FINISHED()) &&
       !(m_lastValues[expr] == StateVariable::WAITING() || m_lastValues[expr] == StateVariable::FINISHED())) {
      m_count++;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in waiting or finished than were actually there.");
      if(m_count == m_total)
	internalSetValue(BooleanVariable::TRUE());
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenWaitingOrFinishedCondition::decrementCount(const ExpressionId& expr) {
    if(m_lastValues.find(expr) == m_lastValues.end())
      m_lastValues[expr] = UNKNOWN();
    if(!(expr->getValue() == StateVariable::WAITING() || expr->getValue() == StateVariable::FINISHED()) &&
       (m_lastValues[expr] == StateVariable::WAITING() || m_lastValues[expr] == StateVariable::FINISHED())) {
      m_count--;
      checkError(m_count <= m_total,
		 "Error: somehow counted more nodes in waiting or finished than were actually there.");
      if(getValue() == BooleanVariable::TRUE())
	internalSetValue(BooleanVariable::FALSE());
    }
    m_lastValues[expr] = expr->getValue();
  }

  void AllChildrenWaitingOrFinishedCondition::handleActivate(const bool changed) {
    if(changed)
      m_listener.activate();
    Calculable::handleActivate(changed);
  }

  void AllChildrenWaitingOrFinishedCondition::handleDeactivate(const bool changed) {
    if(changed)
      m_listener.deactivate();
    Calculable::handleDeactivate(changed);
  }

  double AllChildrenWaitingOrFinishedCondition::recalculate() {
    m_count = 0;
    for(std::vector<NodeId>::const_iterator it = m_children.begin(); it != m_children.end();
	++it) {
      NodeId child = *it;
      check_error(child.isValid());
      ExpressionId expr = child->getStateVariable();
      if(m_lastValues.find(expr) == m_lastValues.end())
	m_lastValues[expr] = UNKNOWN();
      m_lastValues[expr] = expr->getValue();

      if(m_lastValues[expr] == StateVariable::WAITING() ||
	 m_lastValues[expr] == StateVariable::FINISHED())
	++m_count;
    }
    checkError(m_count <= m_total,
	       "Error: somehow counted more nodes in waiting or finished (" << m_count <<
	       ") than were actually there (" << m_total << ").");
    if(m_count == m_total)
      return BooleanVariable::TRUE();
    else
      return BooleanVariable::FALSE();
  }

  bool AllChildrenWaitingOrFinishedCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond)
    : ExpressionListener(), m_cond(cond) {}

  void 
  AllChildrenWaitingOrFinishedCondition::WaitingOrFinishedListener::notifyValueChanged(const ExpressionId& expression)
  {
    checkError(dynamic_cast<StateVariable*>((Expression*)expression) != NULL,
	       "Waiting or finished listener not listening on a state variable.");
    if(expression->getValue() == StateVariable::WAITING() ||
       expression->getValue() == StateVariable::FINISHED())
      m_cond.incrementCount(expression);
    else
      m_cond.decrementCount(expression);
  }

  std::string AllChildrenWaitingOrFinishedCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "childrenWaitingOrFinished(" << m_count << ":" << m_total << "))";
    return retval.str();
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

    bool first = true;
    for(std::vector<PlexilExprId>::const_iterator it = op->subExprs().begin();
	it != op->subExprs().end(); ++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      debugMsg("InternalCondition:InternalCondition",
	       "Adding " << subExpr->toString() << " as " << (garbage ? "" : "non-") << " garbage.");
      if(garbage)
	m_garbage.insert(subExpr);
      if(first) {
	m_first = subExpr;
	first = false;
      }
      else
	m_second = subExpr;
    }

    checkError(m_first.isValid() && m_second.isValid(),
	       "Expected two subexpressions in " << expr->name());

    //m_subexpressions.clear();

    if(op->getOp() == "EQInternal")
      m_expr = (new Equality(m_first, m_second))->getId();
    else if(op->getOp() == "NEInternal")
      m_expr = (new Inequality(m_first, m_second))->getId();
    addSubexpression(m_expr, false);
  }

  InternalCondition::~InternalCondition() {
    removeSubexpression(m_expr);
    delete (Expression*) m_expr;
  }

  double InternalCondition::recalculate() {return m_expr->getValue();}

  bool InternalCondition::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  std::string InternalCondition::toString() const {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << m_expr->toString();
    retval << ")";
    return retval.str();
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

  std::string InterruptibleCommandHandleValues::toString() const 
  {
	std::ostringstream retval;
	retval << Expression::toString();
	retval << "interruptibleCommandHandleValues(" << m_e->toString();
	retval << "))";
	return retval.str();
  }

  double InterruptibleCommandHandleValues::recalculate()
  {
	double v = m_e->getValue();
	if(v == Expression::UNKNOWN())
	  return false;
	else if ((v == CommandHandleVariable::COMMAND_DENIED()) || 
			 (v == CommandHandleVariable::COMMAND_FAILED()))
	  return true;
	return false;
  }

  bool InterruptibleCommandHandleValues::checkValue(const double val)
  {
	return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE();
  }

  AllCommandHandleValues::AllCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node)
	: UnaryExpression(expr, node) {}
  AllCommandHandleValues::AllCommandHandleValues(ExpressionId e) : UnaryExpression(e) {}

  std::string AllCommandHandleValues::toString() const 
  {
	std::ostringstream retval;
	retval << Expression::toString();
	retval << "allCommandHandleValues(" << m_e->toString();
	retval << "))";
	return retval.str();
  }

  double AllCommandHandleValues::recalculate()
  {
	double v = m_e->getValue();
	if(v == Expression::UNKNOWN())
	  return false;
	else if((v == CommandHandleVariable::COMMAND_DENIED()) || 
			(v == CommandHandleVariable::COMMAND_FAILED()) ||
			(v == CommandHandleVariable::COMMAND_SENT_TO_SYSTEM()) ||
			(v == CommandHandleVariable::COMMAND_ACCEPTED()) ||
			(v == CommandHandleVariable::COMMAND_RCVD_BY_SYSTEM()) ||
			(v == CommandHandleVariable::COMMAND_SUCCESS()))
	  return true;
	return false;
  }

  bool AllCommandHandleValues::checkValue(const double val)
  {
	return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE();
  }

}
