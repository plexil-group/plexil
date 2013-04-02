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

#include "Calculables.hh"
#include "BooleanVariable.hh"
#include "ExpressionFactory.hh"
#include "Variables.hh"

#include <algorithm> // for std::max(), std::min()
#include <cmath>

namespace PLEXIL
{

  //
  // Unary expressions
  //

  bool LogicalNegation::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value LogicalNegation::recalculate()
  {
    const Value& v = m_e->getValue();
    checkError(v.isBoolean() || v.isUnknown(),
               "Invalid value in logical negation: " << v);
    if (v.isUnknown())
      return UNKNOWN();
    return Value(!v.getBoolValue());
  }

  void LogicalNegation::print(std::ostream& s) const
  {
    Expression::print(s);
    s << '!' << *m_e << ')';
  }

  AbsoluteValue::AbsoluteValue(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

  AbsoluteValue::AbsoluteValue(ExpressionId e)
    : UnaryExpression(e) {}

  bool AbsoluteValue::checkValue(const Value& val) const
  {
    return val.isUnknown() || val.getDoubleValue() >= 0;
  }

  Value AbsoluteValue::recalculate()
  {
    const Value& v = m_e->getValue();

    if (v.isUnknown())
      return UNKNOWN();
    return Value(fabs(v.getDoubleValue()));
  }

  void AbsoluteValue::print(std::ostream& s) const 
  {
    printAsFnCall(s);
  }

  PlexilType AbsoluteValue::getValueType() const
  {
    return m_e->getValueType();
  }

  SquareRoot::SquareRoot(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

  SquareRoot::SquareRoot(ExpressionId e)
    : UnaryExpression(e) {}

  Value SquareRoot::recalculate() {
    const Value& v = m_e->getValue();
    if (v.isUnknown())
      return UNKNOWN();
    checkError(v.getDoubleValue() >= 0,
               "SquareRoot:recalculate: Tried to take square root of a negative value.");
    return Value(sqrt(v.getDoubleValue()));
  }

  bool SquareRoot::checkValue(const Value& val) const
  {
    return val.isUnknown() || val.getDoubleValue() >= 0;
  }

  void SquareRoot::print(std::ostream& s) const 
  {
    printAsFnCall(s);
  }


 IsKnown::IsKnown(const PlexilExprId& expr, const NodeConnectorId& node)
    : UnaryExpression(expr, node) {}

 IsKnown::IsKnown(ExpressionId e)
    : UnaryExpression(e) {}

  Value IsKnown::recalculate()
  {
    return Value(!m_e->getValue().isUnknown());
  }

  bool IsKnown::checkValue(const Value& val) const
  {
    return val.isBoolean();
  }

  void IsKnown::print(std::ostream& s) const
  {
    printAsFnCall(s);
  }

  //
  // Binary expressions
  //

  //
  // N-Ary expressions
  //
   
  bool Conjunction::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value Conjunction::recalculate()
  {
    // result is assumed to be true. from this point
    // the result may only be demoted to UNKNOWN or false
    Value result(true);

    // compute and store values for all subexpressions
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end();
         ++child) {
      const Value& value = (*child)->getValue();
      // if the value is false, the expression is false, we're done
      if (value == BooleanVariable::FALSE_VALUE()) {
        return value;
      }
      // if the value is unknown, the expression might be unknown,
      // but we need to keep looking
      else if (value.isUnknown()) {
        if (!result.isUnknown())
          result.setUnknown();
      }
      else {
        checkError(value == BooleanVariable::TRUE_VALUE(),
                   "Invalid (non-boolean) conjunction value "
                   << (*child)->getValue()
                   << " from expression " << (*child)->toString());
      }
    }

    return result;
  }

  bool Disjunction::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }
  
  Value Disjunction::recalculate()
  {
    // result is assumed to be false. from this point
    // the result may only be demoted to UNKNOWN or true
    Value result(false);
      
    // compute and store values for all subexpressions
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end();
         ++child) {
      const Value& value = (*child)->getValue();
      // if the value is true, the expression is true, we're done
      if (value == BooleanVariable::TRUE_VALUE())
        return value;
      // if the value is unknown, the expression might be
      // unknown, but we need to keep looking
      else if (value.isUnknown()) {
        if (!result.isUnknown())
          result.setUnknown();
      }
      else {
        checkError(value == BooleanVariable::FALSE_VALUE(),
                   "Invalid (non-boolean) disjunction value "
                   << (*child)->getValue()
                   << " from expression " << (*child)->toString());
      }
    }
    
    return result;
  }

  bool ExclusiveDisjunction::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }
  
  Value ExclusiveDisjunction::recalculate()
  {
    assertTrueMsg(!m_subexpressions.empty(),
                  "Exclusive OR expression " << this->toString()
                  << "requires one or more subexpressions.");

    std::vector<Value> values;
      
    // compute and store values for all subexpressions
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end();
         ++child) {
      const Value& value = (*child)->getValue();
      checkError(checkValue(value),
                 "Invalid exclusive or value: " << value);
      values.push_back(value);
    }

    // inspect values of all subexpressions
    Value result;
    for (std::vector<Value>::iterator value = values.begin();
         value != values.end();
         ++value) {
      // if the value is unknown, the entire expression is unknown
      if (value->isUnknown())
        return UNKNOWN();

      // if this is the first value init result to its value
      if (value == values.begin())
        result = *value;

      // otherwise the value is the XOR of the result and the new value
      else
        result = 
          (result.getBoolValue() ? !value->getBoolValue() : value->getBoolValue());
    }
    return result;
  }

  //
  // String Concatenation
  //

  Concatenation::Concatenation(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node), m_label()
  {
  }

  Concatenation::Concatenation(const ExpressionId& a, const ExpressionId& b)
    : NaryExpression(a, b), m_label()
  {
  }

  Concatenation::Concatenation(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
    : NaryExpression(a, aGarbage, b, bGarbage), m_label()
  {
  }

  bool Concatenation::checkValue(const Value& val) const
  {
    return val.isUnknown() || val.isString();
  }

  Value Concatenation::recalculate()
  {   
    std::ostringstream retval; 
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); 
         ++child) {
      const Value& value = (*child)->getValue();
      if (value.isUnknown())
        return UNKNOWN();
      checkError(value.isString(),
                 "Invalid concatenation value: " << value);
      retval << value.getStringValue();
    }
    return Value(retval.str());
  }

  //
  // Comparisons
  //

  bool Equality::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value Equality::recalculate() 
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();

    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1 == v2);
  }

  bool Inequality::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value Inequality::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1 != v2);
  }

  bool LessThan::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value LessThan::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() < v2.getDoubleValue());
  }

  bool LessEqual::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value LessEqual::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() <= v2.getDoubleValue());
  }

  bool GreaterThan::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value GreaterThan::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() > v2.getDoubleValue());
  }

  bool GreaterEqual::checkValue(const Value& val) const
  {
    return val.isBoolean() || val.isUnknown();
  }

  Value GreaterEqual::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() >= v2.getDoubleValue());
  }


  //
  // Arithmetic expressions
  //

  Value Addition::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() + v2.getDoubleValue());
  }

  PlexilType Addition::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }

  Value Subtraction::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() - v2.getDoubleValue());
  }

  PlexilType Subtraction::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }


  Value Multiplication::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(v1.getDoubleValue() * v2.getDoubleValue());
  }

  PlexilType Multiplication::getValueType() const
  {
    PlexilType aType = m_a->getValueType();
    if (aType == REAL)
      return REAL;
    PlexilType bType = m_b->getValueType();
    if (aType == bType)
      return aType;
    // default to real
    return REAL;
  }


  Value Division::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    assertTrue(v2.getDoubleValue() != 0.0, "Attempt to divide by zero");
    return Value(v1.getDoubleValue() / v2.getDoubleValue());
  }

  PlexilType Division::getValueType() const
  {
    return REAL;
  }


  Value Modulo::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    assertTrue(v2.getDoubleValue() != 0.0, "Attempt to divide by zero");
    return Value(fmod(v1.getDoubleValue(), v2.getDoubleValue()));
  }

  PlexilType Modulo::getValueType() const
  {
    return REAL;
  }


  Value Maximum::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(std::max(v1.getDoubleValue(), v2.getDoubleValue()));
  }

  void Maximum::print(std::ostream& s) const
  {
    printAsFnCall(s);
  }

  PlexilType Maximum::getValueType() const
  {
    return REAL;
  }


  Value Minimum::recalculate()
  {
    const Value& v1 = m_a->getValue();
    const Value& v2 = m_b->getValue();
    if (v1.isUnknown() || v2.isUnknown())
      return UNKNOWN();
    return Value(std::min(v1.getDoubleValue(), v2.getDoubleValue()));
  }

  void Minimum::print(std::ostream& s) const
  {
    printAsFnCall(s);
  }

  PlexilType Minimum::getValueType() const
  {
    return REAL;
  }

}
