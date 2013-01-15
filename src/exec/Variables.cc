/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#include "Variables.hh"
#include "Debug.hh"
#include "ExecListenerHub.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"

#include <cmath> // for fabs()
#include <cstdlib> // for strtod(), strtoll()

namespace PLEXIL
{

  StringVariable::StringVariable()
    : VariableImpl(false)
  {}

  StringVariable::StringVariable(const Value& value, const bool isConst)
    : VariableImpl(value, isConst)
  {
    assertTrueMsg(checkValue(value),
                  "Attempted to initialize string variable to an invalid value \""
                  << value << "\"");
  }

  StringVariable::StringVariable(const LabelStr& value, const bool isConst)
    : VariableImpl(Value(value), isConst)
  {
    assertTrueMsg(checkValue(value),
                  "Attempted to initialize string variable to an invalid value \"" << value.toString() << "\"");
  }

  StringVariable::StringVariable(const PlexilExprId& expr,
                                 const NodeConnectorId& node,
                                 const bool isConst)
    : VariableImpl(expr, node, isConst)
  {
    assertTrueMsg(expr.isValid(), "Attempt to create a StringVariable from an invalid Id");
    const PlexilValue* value = NULL;
    if (Id<PlexilVar>::convertable(expr)) {
      const Id<PlexilVar> var = (const Id<PlexilVar>) expr;
      // If the ExpressionFactory is correctly set up, should NEVER EVER happen
      assertTrueMsg(var->type() == STRING,
                    "Attempt to create a StringVariable from a non-STRING PlexilVar");
      value = var->value();
    }
    else if (Id<PlexilValue>::convertable(expr)) {
      value = (const PlexilValue*) expr;
      assertTrueMsg(isConst, "Attempt to create a StringValue that is not const");
    }
    else {
      assertTrueMsg(ALWAYS_FAIL, "Expected a PlexilVar or PlexilValue");
    }

    if (value == NULL) {
      m_initialValue = m_value = UNKNOWN();
    }
    else {
      assertTrueMsg(value->type() == STRING,
                    "Attempt to create a StringVariable from a non-STRING PlexilValue");
      m_initialValue = m_value = value->value();
    }
  }

  void StringVariable::print(std::ostream& s) const 
  {
    VariableImpl::print(s);
    s << "string)";
  }

  bool StringVariable::checkValue(const Value& val) const
  {
    return val.isUnknown() || val.isString();
  }

  RealVariable::RealVariable()
    : VariableImpl(false)
  {
  }

  RealVariable::RealVariable(const Value& value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    assertTrueMsg(checkValue(value),
                  "Attempted to initialize a Real variable with invalid value \"" << value << "\"");
  }

  RealVariable::RealVariable(const PlexilExprId& expr,
                             const NodeConnectorId& node,
                             const bool isConst)
    : VariableImpl(expr, node, isConst) 
  {
    assertTrueMsg(expr.isValid(), "Attempt to create a RealVariable from an invalid Id");
    const PlexilValue* value = NULL;
    if (Id<PlexilVar>::convertable(expr)) {
      const Id<PlexilVar> var = (const Id<PlexilVar>) expr;
      // If the ExpressionFactory is correctly set up, should NEVER EVER happen
      assertTrueMsg(var->type() == REAL,
                    "Attempt to create a RealVariable from a non-REAL PlexilVar");
      value = var->value();
    }
    else if (Id<PlexilValue>::convertable(expr)) {
      value = (const PlexilValue*) expr;
      assertTrueMsg(isConst, "Attempt to create a RealValue that is not const");
    }
    else {
      assertTrueMsg(ALWAYS_FAIL, "Expected a PlexilVar or PlexilValue");
    }

    if (value == NULL)
      m_initialValue = m_value = UNKNOWN();
    else {
      assertTrueMsg(value->type() == REAL,
                    "Attempt to create a RealVariable from a non-REAL PlexilVar");

      // Parse initial value
      const char* valstr = value->value().c_str();
      // FIXME: handle 'NaN' ???
      // FIXME: check for [\-+]INF

      char* endptr = NULL;
      double value = strtod(valstr, &endptr);
      assertTrueMsg(endptr != valstr && !*endptr,
                    "Initial value \"" << valstr << "\" not a valid Real for RealVariable");
      m_initialValue = m_value = value;
    }
  }

  void RealVariable::print(std::ostream& s) const 
  {
    VariableImpl::print(s);
    s << "real)";
  }
   
  bool RealVariable::checkValue(const Value& val) const
  {
    return val.isUnknown()
      || (val.getDoubleValue() >= REAL_MINUS_INFINITY
          && val.getDoubleValue() <= REAL_PLUS_INFINITY);
  }

  ExpressionId& RealVariable::ZERO_EXP() {
    static ExpressionId sl_zero_exp;
    if (sl_zero_exp.isNoId()) {
      VariableImpl* var = new RealVariable(0.0, true);
      var->setName("Real constant 0");
      sl_zero_exp = var->getId();
    }
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& RealVariable::ONE_EXP() {
    static ExpressionId sl_one_exp;
    if (sl_one_exp.isNoId()) {
      VariableImpl* var = new RealVariable(1.0, true);
      var->setName("Real constant 1");
      sl_one_exp = var->getId();
    }
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& RealVariable::MINUS_ONE_EXP() {
    static ExpressionId sl_minus_one_exp;
    if (sl_minus_one_exp.isNoId()) {
      VariableImpl* var = new RealVariable(-1.0, true);
      var->setName("Real constant -1");
      sl_minus_one_exp = var->getId();
    }
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  IntegerVariable::IntegerVariable()
    : VariableImpl(false)
  {
  }

  IntegerVariable::IntegerVariable(const Value& value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    assertTrueMsg(checkValue(value),
                  "Attempted to initialize an Integer variable to invalid value \"" << value << "\"");
  }

  IntegerVariable::IntegerVariable(const PlexilExprId& expr, 
                                   const NodeConnectorId& node,
                                   const bool isConst)
    : VariableImpl(expr, node, isConst) 
  {
    assertTrueMsg(expr.isValid(), "Attempt to create an IntegerVariable from an invalid Id");
    const PlexilValue* value = NULL;
    if (Id<PlexilVar>::convertable(expr)) {
      const Id<PlexilVar> var = (const Id<PlexilVar>) expr;
      // If the ExpressionFactory is correctly set up, should NEVER EVER happen
      assertTrueMsg(var->type() == INTEGER,
                    "Attempt to create an IntegerVariable from a non-INTEGER PlexilVar");
      value = var->value();
    }
    else if (Id<PlexilValue>::convertable(expr)) {
      value = (const PlexilValue*) expr;
      assertTrueMsg(isConst, "Attempt to create an IntegerValue that is not const");
    }
    else {
      assertTrueMsg(ALWAYS_FAIL, "Expected a PlexilVar or PlexilValue");
    }

    if (value == NULL)
      m_initialValue = m_value = UNKNOWN();
    else {
      assertTrueMsg(value->type() == INTEGER,
                    "Attempt to create an IntegerVariable from a non-INTEGER PlexilVar");

      // Parse initial value
      const char* valstr = value->value().c_str();
      char* endptr = NULL;
      int64_t intval = strtoll(valstr, &endptr, 10);
      assertTrueMsg(endptr != valstr && !*endptr,
                    "Initial value \"" << valstr << "\" not a valid Integer for IntegerVariable");
      m_initialValue = m_value = (double) intval;
    }
  }

  void IntegerVariable::print(std::ostream& s) const
  {
    VariableImpl::print(s);
    s << "int)";
  }

  bool IntegerVariable::checkValue(const Value& val) const
  {
    if (val.isUnknown())
      return true;
    double d = val.getDoubleValue();
    if (d < (double) MINUS_INFINITY || d > (double) PLUS_INFINITY)
      return false;
    if (fabs(d - ((double) (int32_t) d)) < EPSILON)
      return true;
    return false;
  }

  ExpressionId& IntegerVariable::ZERO_EXP()
  {
    static ExpressionId sl_zero_exp;
    if (sl_zero_exp.isNoId()) {
      VariableImpl* var = new IntegerVariable(0.0, true);
      var->setName("Integer constant 0");
      sl_zero_exp = var->getId();
    }
    if(!sl_zero_exp->isActive())
      sl_zero_exp->activate();
    return sl_zero_exp;
  }

  ExpressionId& IntegerVariable::ONE_EXP()
  {
    static ExpressionId sl_one_exp;
    if (sl_one_exp.isNoId()) {
      VariableImpl* var = new IntegerVariable(1.0, true);
      var->setName("Integer constant 1");
      sl_one_exp = var->getId();
    }
    if(!sl_one_exp->isActive())
      sl_one_exp->activate();
    return sl_one_exp;
  }

  ExpressionId& IntegerVariable::MINUS_ONE_EXP()
  {
    static ExpressionId sl_minus_one_exp;
    if (sl_minus_one_exp.isNoId()) {
      VariableImpl* var = new IntegerVariable(-1.0, true);
      var->setName("Integer constant -1");
      sl_minus_one_exp = var->getId();
    }
    if(!sl_minus_one_exp->isActive())
      sl_minus_one_exp->activate();
    return sl_minus_one_exp;
  }

  TimepointVariable::TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node)
    : AliasVariable(expr->name(),
                    node,
                    node->findVariable((PlexilVarRef*)expr),
                    false,
                    true)
  {
    checkError(Id<PlexilTimepointVar>::convertable(expr),
           "Expected NodeTimepoint element, got " << expr->name());
  }

}
