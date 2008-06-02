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

  Lookup::Lookup(const PlexilExprId& expr, const NodeConnectorId& node)
    : Variable(false), m_cache(node->getExec()->getStateCache()) 
  {
     checkError(Id<PlexilLookup>::convertable(expr), "Expected a lookup.");
     PlexilLookup* lookup = (PlexilLookup*) expr;
     PlexilState* state = lookup->state();
     
        // create the correct form of the expression for this name

     m_stateNameExpr = ExpressionFactory::createInstance(
        state->nameExpr()->name(), state->nameExpr(), node);

        // handle argument lookup

     getArguments(state->args(), node);
  }

   Lookup::Lookup(const StateCacheId& cache, const LabelStr& stateName,
		 std::list<ExpressionId>& params)
      : Variable(false), m_cache(cache), //m_stateName(stateName), 
        m_params(params)
   {
   }
   
   Lookup::Lookup(const StateCacheId& cache, const LabelStr& stateName,
                  std::list<double>& params)
      : Variable(false), m_cache(cache)
   {
      for(std::list<double>::const_iterator it = params.begin(); it != params.end(); ++it) 
      {
         ExpressionId expr = (new Variable(*it, true))->getId();
         m_params.push_back(expr);
         m_garbage.insert(expr);
      }
   }

  Lookup::~Lookup() 
  {
    for(std::set<ExpressionId>::iterator it = m_garbage.begin(); 
        it != m_garbage.end(); ++it)
    {
       delete (Expression*) (*it);
    }
    m_stateNameExpr.remove(); //delete (Expression*)&(*m_stateNameExpr);
  }

  void Lookup::getArguments(const std::vector<PlexilExprId>& args,
			    const NodeConnectorId& node) {
    for(std::vector<PlexilExprId>::const_iterator it = args.begin(); it != args.end(); ++it) {
      if(Id<PlexilVarRef>::convertable(*it))
	m_params.push_back(node->findVariable((PlexilVarRef*)*it));
      else {
	ExpressionId param = ExpressionFactory::createInstance((*it)->name(), *it, node);
	check_error(param.isValid());
	m_params.push_back(param);
	m_garbage.insert(param);
      }
    }
  }

  void Lookup::handleActivate(const bool changed) {
    if(!changed)
      return;
    for(std::list<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    check_error(m_stateNameExpr.isValid());
    m_stateNameExpr->activate();
    registerLookup();
  }

  void Lookup::handleDeactivate(const bool changed) {
    if(!changed)
      return;
    for(std::list<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
    m_stateNameExpr->deactivate();
    unregisterLookup();
  }

  void Lookup::registerLookup() {
    std::vector<double> args;
    for(std::list<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      checkError(expr->isActive(),
		 "Can't register a lookup with an inactive parameter: " << toString());
      args.push_back((*it)->getValue());
    }
    checkError(m_stateNameExpr->isActive(),
               "Can't register a lookup with an inactive name state expression: " << toString());
    Expressions dest(1, m_id);
    State state(m_stateNameExpr->getValue(), args);
    handleRegistration(dest, state);
  }
   
  void Lookup::unregisterLookup() {
    handleUnregistration();
  }


  // *** this should be extended to use the global declarations

  PlexilType Lookup::getValueType() const
  {
    return PLEXIL::UNKNOWN;
  }

  LookupNow::LookupNow(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node) {
    checkError(Id<PlexilLookupNow>::convertable(expr), "Expected LookupNow.");
  }

  void LookupNow::handleRegistration(Expressions& dest, State& state) {
     m_cache->lookupNow(m_id, dest, state);
  }

  void LookupNow::handleUnregistration() {}

  std::string LookupNow::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "LookupNow(" << m_stateNameExpr->getValue() << "(";
    for(std::list<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end(); ++it)
      retval << ", " << (*it)->toString();
    retval << ")))";
    return retval.str();
  }

  LookupOnChange::LookupOnChange(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node) {
    checkError(Id<PlexilChangeLookup>::convertable(expr), "Expected LookupOnChange");
    PlexilChangeLookup* lookup = (PlexilChangeLookup*) expr;

    if(lookup->tolerances().empty())
      m_tolerance = RealVariable::ZERO_EXP();
    else {
      if(Id<PlexilVarRef>::convertable(lookup->tolerances()[0]))
	m_tolerance = node->findVariable((PlexilVarRef*)lookup->tolerances()[0]);
      else {
	m_tolerance = ExpressionFactory::createInstance(lookup->tolerances()[0]->name(),
							lookup->tolerances()[0]);
	m_garbage.insert(m_tolerance);
      }
    }
  }

  std::string LookupOnChange::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "LookupOnChange(" << m_stateNameExpr->getValue() << "(";
    for(std::list<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end();
	++it)
      retval << ", " << (*it)->toString();
    retval << "), " << m_tolerance->toString() << "))";
    return retval.str();
  }

  void LookupOnChange::handleRegistration(Expressions& dest, State& state) {
    m_tolerance->activate();
    m_cache->registerChangeLookup(m_id, dest, state, std::vector<double>(1, m_tolerance->getValue()));
  }

  void LookupOnChange::handleUnregistration() {
    m_tolerance->deactivate();
    m_cache->unregisterChangeLookup(m_id);
  }

  LookupWithFrequency::LookupWithFrequency(const PlexilExprId& expr,
					   const NodeConnectorId& node)
    : Lookup(expr, node) {
    checkError(Id<PlexilFrequencyLookup>::convertable(expr), "Expected LookupWithFrequency");
    PlexilFrequencyLookup* lookup = (PlexilFrequencyLookup*) expr;
    
    checkError(lookup->lowFreq().isValid(),
	       "Need at least a low frequency.");

    if(Id<PlexilVarRef>::convertable(lookup->lowFreq()))
      m_lowFrequency = node->findVariable((PlexilVarRef*)lookup);
    else {
      m_lowFrequency = ExpressionFactory::createInstance(lookup->lowFreq()->name(),
							 lookup->lowFreq());
      m_garbage.insert(m_lowFrequency);
    }

    if(lookup->highFreq().isValid()) 
      m_highFrequency = m_lowFrequency;
    else {
      if(Id<PlexilVarRef>::convertable(lookup->highFreq()))
	m_highFrequency = node->findVariable((PlexilVarRef*)lookup);
      else {
	m_highFrequency = ExpressionFactory::createInstance(lookup->highFreq()->name(),
							   lookup->highFreq());
	m_garbage.insert(m_highFrequency);
      }
    }
    checkError(m_highFrequency.isValid(), "No high frequency specified in LookupWithFrequency.");
    checkError(m_lowFrequency.isValid(), "No low frequency specified in LookupWithFrequency.");
  }

  std::string LookupWithFrequency::toString() const {
    std::stringstream retval;
    retval << Expression::toString();
    retval << "LookupWithFrequency(" << m_stateNameExpr->getValue() << "(";
    for(std::list<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end();
	++it)
      retval << ", " << (*it)->toString();
    retval << "), " << m_lowFrequency->toString() << ", " << m_highFrequency->toString() << "))";
    return retval.str();
  }

  void LookupWithFrequency::handleRegistration(Expressions& dest, State& state) {
    m_highFrequency->activate();
    m_lowFrequency->activate();
    m_cache->registerFrequencyLookup(m_id, dest, state, m_lowFrequency->getValue(), m_highFrequency->getValue());
  }

  void LookupWithFrequency::handleUnregistration() {
    m_highFrequency->deactivate();
    m_lowFrequency->deactivate();
    m_cache->unregisterFrequencyLookup(m_id);
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
	REGISTER_EXPRESSION(LookupWithFrequency, LookupWithFrequency);
	REGISTER_EXPRESSION(TimepointVariable, NodeTimepointValue);
	//REGISTER_EXPRESSION(TimepointVariable, NodeTimepointValue);
	REGISTER_EXPRESSION(AbsoluteValue, ABS);
	REGISTER_EXPRESSION(SquareRoot, SQRT);
	initializeExpressions_called = true;
      }
  }

} // namespace PLEXIL
