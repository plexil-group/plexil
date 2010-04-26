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

#include "Expression.hh"
#include "Expressions.hh"
//#include "ExternalInterface.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "StateCache.hh"
#include "Node.hh"
#include "PlexilExec.hh"
#include <sstream>
#include <cmath>

namespace PLEXIL {

  // *** To do:
  //  - implement multiple return values from lookups

  Lookup::Lookup(const PlexilExprId& expr, const NodeConnectorId& node)
    : Variable(false), 
      m_cache(node->getExec()->getStateCache()),
      m_dest(1, m_id),
      m_state(Expression::UNKNOWN(),
              std::vector<double>(((PlexilLookup*)expr)->state()->args().size(),
                                  Expression::UNKNOWN())),
      m_listener(m_id)
  {
     checkError(Id<PlexilLookup>::convertable(expr), "Expected a lookup.");
     PlexilLookup* lookup = (PlexilLookup*) expr;
     PlexilState* state = lookup->state();
     
     // create the correct form of the expression for this name
     m_stateNameExpr = 
       ExpressionFactory::createInstance(state->nameExpr()->name(), 
                                         state->nameExpr(), node);
     m_stateNameExpr->addListener(m_listener.getId());

     // handle argument lookup
     getArguments(state->args(), node);
  }

  Lookup::~Lookup() 
  {
    // disconnect listeners
    m_stateNameExpr->removeListener(m_listener.getId());
    for (std::vector<ExpressionId>::iterator it = m_params.begin(); 
         it != m_params.end();
         ++it)
      (*it)->removeListener(m_listener.getId());

    // safe to delete anything in the garbage
    for (std::vector<ExpressionId>::iterator it = m_garbage.begin(); 
         it != m_garbage.end();
         ++it)
      delete (*it).operator->();

    // N.B. Can't delete state name expression because it may be shared
    // (e.g. variable references)
    m_stateNameExpr.remove();
  }

  void Lookup::getArguments(const std::vector<PlexilExprId>& args,
			    const NodeConnectorId& node) 
  {
    for (std::vector<PlexilExprId>::const_iterator it = args.begin(); it != args.end(); ++it) 
      {
        ExpressionId param;
        if (Id<PlexilVarRef>::convertable(*it))
          {
            param = node->findVariable((PlexilVarRef*)*it);
          }
        else 
          {
            param = ExpressionFactory::createInstance((*it)->name(), *it, node);
            check_error(param.isValid());
            m_garbage.push_back(param);
          }
	m_params.push_back(param);
        param->addListener(m_listener.getId());
	debugMsg("Lookup:getArguments",
		 " " << toString() << " added listener for " << param->toString());
      }
  }

  void Lookup::handleActivate(const bool changed) 
  {
    if (!changed)
      return;

    debugMsg("Lookup:handleActivate", " for " << toString());

    for (std::vector<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it)
      {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    check_error(m_stateNameExpr.isValid());
    m_stateNameExpr->activate();
    updateState();
    registerLookup();
    // Safe to activate once lookup is registered
    m_listener.activate();
  }

  void Lookup::handleDeactivate(const bool changed) {
    if(!changed)
      return;

    debugMsg("Lookup:handleDeactivate", " for " << toString());

    m_listener.deactivate();
    unregisterLookup();
    for(std::vector<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
    m_stateNameExpr->deactivate();
  }

  void Lookup::updateState()
  {
    checkError(m_stateNameExpr->isActive(),
               "Can't update state for lookup with an inactive name state expression: " << toString());
    m_state.first = m_stateNameExpr->getValue();
    std::vector<ExpressionId>::const_iterator it = m_params.begin();
    std::vector<double>::iterator sit = m_state.second.begin();
    while (it != m_params.end() && sit != m_state.second.end())
      {
        ExpressionId expr = *it;
        check_error(expr.isValid());
        checkError(expr->isActive(),
                   "Can't update state for lookup with an inactive parameter: " << toString());
        (*sit) = ((*it)->getValue());
        it++;
        sit++;
      }
  }

  bool Lookup::isStateCurrent() const
  {
    checkError(m_stateNameExpr->isActive(),
               "Can't compare state to lookup with an inactive name state expression: " << toString());
    if (m_state.first != m_stateNameExpr->getValue())
      return false;
    std::vector<ExpressionId>::const_iterator it = m_params.begin();
    std::vector<double>::const_iterator sit = m_state.second.begin();
    while (it != m_params.end() && sit != m_state.second.end())
      {
        ExpressionId expr = *it;
        check_error(expr.isValid());
        checkError(expr->isActive(),
                   "Can't compare state to lookup with an inactive parameter: " << toString());
        if ((*it)->getValue() != *sit)
          return false;
        it++;
        sit++;
      }
    return true;
  }

  void Lookup::registerLookup() 
  {
    handleRegistration();
  }
   
  void Lookup::unregisterLookup() {
    handleUnregistration();
  }

  // *** this should be extended to use the global declarations

  PlexilType Lookup::getValueType() const
  {
    return PLEXIL::UNKNOWN_TYPE;
  }


  LookupNow::LookupNow(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node) 
  {
    checkError(Id<PlexilLookupNow>::convertable(expr), "Expected LookupNow.");
  }

  void LookupNow::handleChange(const ExpressionId& /* exp */)
  {
    // need to notify state cache if cached lookup is no longer valid
    if (!isStateCurrent())
      {
	debugMsg("LookupNow:handleChange",
		 " state changed, updating state cache");
        const State oldState(m_state);
        updateState();
        handleRegistrationChange(oldState);
      }
  }

  void LookupNow::handleRegistration() 
  {
    m_cache->lookupNow(m_id, m_dest, m_state);
  }

  // Simply reinvokes StateCache::lookupNow().

  void LookupNow::handleRegistrationChange(const State& /* oldState */)
  {
    m_cache->lookupNow(m_id, m_dest, m_state);
  }

  void LookupNow::handleUnregistration() 
  {
  }

  std::string LookupNow::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "LookupNow(" << m_stateNameExpr->getValue() << "(";
    for(std::vector<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end(); ++it)
      retval << ", " << (*it)->toString();
    retval << ")))";
    return retval.str();
  }

  LookupOnChange::LookupOnChange(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node)
  {
    checkError(Id<PlexilChangeLookup>::convertable(expr), "Expected LookupOnChange");
    PlexilChangeLookup* lookup = (PlexilChangeLookup*) expr;

    if(lookup->tolerances().empty())
      m_tolerance = RealVariable::ZERO_EXP();
    else 
      {
        if(Id<PlexilVarRef>::convertable(lookup->tolerances()[0]))
          m_tolerance = node->findVariable((PlexilVarRef*)lookup->tolerances()[0]);
        else
          {
            m_tolerance = ExpressionFactory::createInstance(lookup->tolerances()[0]->name(),
                                                            lookup->tolerances()[0]);
            m_garbage.push_back(m_tolerance);
          }
        m_tolerance->addListener(m_listener.getId());
      }
  }

  LookupOnChange::~LookupOnChange()
  {
    m_tolerance->removeListener(m_listener.getId());
  }

  std::string LookupOnChange::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "LookupOnChange(" << m_stateNameExpr->getValue() << "(";
    for(std::vector<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end();
	++it)
      retval << ", " << (*it)->toString();
    retval << "), " << m_tolerance->toString() << "))";
    return retval.str();
  }

  void LookupOnChange::handleRegistration() {
    m_tolerance->activate();
    m_cache->registerChangeLookup(m_id, m_dest, m_state, std::vector<double>(1, m_tolerance->getValue()));
  }

  void LookupOnChange::handleUnregistration() {
    m_tolerance->deactivate();
    m_cache->unregisterChangeLookup(m_id);
  }

  void LookupOnChange::handleChange(const ExpressionId& exp)
  {
    // need to notify state cache if cached lookup is no longer valid
    if (isStateCurrent() && exp != m_tolerance)
      return;
    debugMsg("LookupOnChange:handleChange",
	     " state changed, updating state cache");
    const State oldState(m_state);
    updateState();
    handleRegistrationChange(oldState);
  }

  // *** To do:
  //  - optimize by adding specific method for this case to StateCache class

  void LookupOnChange::handleRegistrationChange(const State& /* oldState */)
  {
    m_cache->unregisterChangeLookup(m_id);
    m_cache->registerChangeLookup(m_id, m_dest, m_state, std::vector<double>(1, m_tolerance->getValue()));
  }

  AbsoluteValue::AbsoluteValue(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

  AbsoluteValue::AbsoluteValue(ExpressionId e)
    : UnaryExpression(e) {}

  bool AbsoluteValue::checkValue(const double val) {
    return val >= 0;
  }

  double AbsoluteValue::recalculate() {
    double v = m_e->getValue();

    if(v == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return fabs(v);
  }

  std::string AbsoluteValue::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "abs(" << m_e->toString();
    retval << "))";
    return retval.str();
  }

  PlexilType AbsoluteValue::getValueType() const
  {
    return m_e->getValueType();
  }

  SquareRoot::SquareRoot(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

  SquareRoot::SquareRoot(ExpressionId e)
    : UnaryExpression(e) {}

  double SquareRoot::recalculate() {
    double v = m_e->getValue();
    checkError(v >= 0, "Tried to get the sqrt of a negative number.  We don't support complex values yet.");
    if(v == Expression::UNKNOWN())
      return Expression::UNKNOWN();
    return sqrt(v);
  }

  bool SquareRoot::checkValue(const double val) {
    return val >= 0;
  }

  std::string SquareRoot::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "sqrt(" << m_e->toString();
    retval << "))";
    return retval.str();
  }


 IsKnown::IsKnown(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

 IsKnown::IsKnown(ExpressionId e)
    : UnaryExpression(e) {}

  double IsKnown::recalculate() {
    double v = m_e->getValue();
    if(v == Expression::UNKNOWN())
      return false;
    return true;
  }

  bool IsKnown::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE();
  }

  std::string IsKnown::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "isknown(" << m_e->toString();
    retval << "))";
    return retval.str();
  }

  void initializeExpressions()
  {
    static bool initializeExpressions_called = false;
    if (!initializeExpressions_called) 
      {
	REGISTER_EXPRESSION(ArrayElement, ArrayElement);
	REGISTER_EXPRESSION(Conjunction, AND);
	REGISTER_EXPRESSION(Disjunction, OR);
	REGISTER_EXPRESSION(Concatenation, CONCAT);
	REGISTER_EXPRESSION(ExclusiveDisjunction, XOR);
	REGISTER_EXPRESSION(LogicalNegation, NOT);
	REGISTER_EXPRESSION(IsKnown, IsKnown);
	REGISTER_EXPRESSION(Equality, EQ);
	REGISTER_EXPRESSION(Equality, EQNumeric);
	REGISTER_EXPRESSION(Equality, EQBoolean);
	REGISTER_EXPRESSION(Equality, EQString);
	REGISTER_EXPRESSION(Inequality, NE);
	REGISTER_EXPRESSION(Inequality, NENumeric);
	REGISTER_EXPRESSION(Inequality, NEBoolean);
	REGISTER_EXPRESSION(Inequality, NEString);
	REGISTER_EXPRESSION(LessThan, LT);
	REGISTER_EXPRESSION(LessEqual, LE);
	REGISTER_EXPRESSION(GreaterThan, GT);
	REGISTER_EXPRESSION(GreaterEqual, GE);
	REGISTER_EXPRESSION(Addition, ADD);
	REGISTER_EXPRESSION(Subtraction, SUB);
	REGISTER_EXPRESSION(Multiplication, MUL);
	REGISTER_EXPRESSION(Division, DIV);
	REGISTER_EXPRESSION(BooleanVariable, BooleanValue);
	//REGISTER_EXPRESSION(BooleanVariable, BooleanVariable);
	REGISTER_EXPRESSION(IntegerVariable, IntegerValue);
	//REGISTER_EXPRESSION(IntegerVariable, IntegerVariable);
	REGISTER_EXPRESSION(RealVariable, RealValue);
	//REGISTER_EXPRESSION(RealVariable, RealVariable);
	REGISTER_EXPRESSION(StringVariable, StringValue);
	//REGISTER_EXPRESSION(ArrayVariable, ArrayVariable);
	REGISTER_EXPRESSION(ArrayVariable, ArrayValue);
	REGISTER_EXPRESSION(Concatenation, Concat);
        REGISTER_EXPRESSION(StringVariable, StringVariable);
	REGISTER_EXPRESSION(StateVariable, NodeStateValue);
	REGISTER_EXPRESSION(OutcomeVariable, NodeOutcomeValue);
	REGISTER_EXPRESSION(FailureVariable, NodeFailureValue);
	REGISTER_EXPRESSION(CommandHandleVariable, NodeCommandHandleValue);
	REGISTER_EXPRESSION(InternalCondition, EQInternal);
	REGISTER_EXPRESSION(InternalCondition, NEInternal);
	REGISTER_EXPRESSION(LookupNow, LookupNow);
	REGISTER_EXPRESSION(LookupOnChange, LookupOnChange);
	REGISTER_EXPRESSION(TimepointVariable, NodeTimepointValue);
	//REGISTER_EXPRESSION(TimepointVariable, NodeTimepointValue);
	REGISTER_EXPRESSION(AbsoluteValue, ABS);
	REGISTER_EXPRESSION(SquareRoot, SQRT);
	initializeExpressions_called = true;
      }
  }

} // namespace PLEXIL
