/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
#include "ExpressionFactory.hh"
#include "Variables.hh"

namespace PLEXIL
{

  //
  // Unary expressions
  //

  bool LogicalNegation::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LogicalNegation::recalculate() {
    double v = m_e->getValue();
    checkError(v == 1.0 || v == 0.0 || v == UNKNOWN(),
	       "Invalid value in logical negation: " << v);
    if (v == 1.0)
      return 0.0;
    else if (v == 0.0)
      return 1.0;
    else if (v == UNKNOWN())
      return UNKNOWN();
    check_error(ALWAYS_FAIL);
    return -1.0;
  }

  std::string LogicalNegation::toString() const
  {
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "!" << m_e->toString();
    retval << ")";
    return retval.str();
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
    std::ostringstream retval;
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
    std::ostringstream retval;
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
    std::ostringstream retval;
    retval << Expression::toString();
    retval << "isknown(" << m_e->toString();
    retval << "))";
    return retval.str();
  }

  //
  // Binary expressions
  //

  //
  // N-Ary expressions
  //
   
  bool Conjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Conjunction::recalculate()
  {
    // result is assumed to be true. from this point
    // the result may only be demoted to UNKNOWN or false
     
    double result = BooleanVariable::TRUE();
    double value = 0;
     
    // compute and store values for all subexpressions
     
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        value = (*child)->getValue();
        
        // validate values 
       
       checkError(checkValue(value), "Invalid (non-boolean) conjunction value '"
        		<< (*child)->valueString() << "' was returned to condition expression. "
        		<< "More details condition expression: " << (*child)->toString());
	
        // if the value is false, the expression is false, we're done
        
        if (value == BooleanVariable::FALSE())
          {	    
            result = BooleanVariable::FALSE();
            break;
          }
	  
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
        
        if (value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    
    // return the result
     
    return result;
  }
   
  std::string Conjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " && " 
           : ")");
      }
    return retval.str();
  }

  bool Disjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }
  
  double Disjunction::recalculate()
  {
    // result is assumed to be false. from this point
    // the result may only be demoted to UNKNOWN or true
      
    double result = BooleanVariable::FALSE();
    double value = 0;
      
    // compute and store values for all subexpressions
      
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        value = (*child)->getValue();
         
        // validate values 
         
        checkError(checkValue(value), "Invalid (non-boolean) disjunction value '"
        		<< (*child)->valueString() << "' was returned to condition expression. "
        		<< "More details condition expression: " << (*child)->toString());
	
        // if the value is true, the expression is true, we're done
         
        if (value == BooleanVariable::TRUE())
          {
            result = BooleanVariable::TRUE();
            break;
          }
	  
        // if the value is unknown, the expression might be
        // unknown, but we need to keep looking
         
        if (value == BooleanVariable::UNKNOWN())
          result = BooleanVariable::UNKNOWN();
      }
    
    // return the result
      
    return result;
  }


  std::string Disjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " || " 
           : ")");
      }
    return retval.str();
  }

  bool ExclusiveDisjunction::checkValue(const double val)
  {
    return 
      val == BooleanVariable::TRUE() || 
      val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }
  
  double ExclusiveDisjunction::recalculate()
  {
    // make a new list for values
      
    std::list<double> values;
      
    // compute and store values for all subexpressions
      
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        values.push_back(value);
         
        // validate values (your and important part of things!)
         
        checkError(checkValue(value), "Invalid exclusive or value: " << value);
      }
    // confirm we've got enough values

    checkError(values.size()  > 0, "Exclusive OR expression " <<
               this->toString() << "requires one or more subexpressions.");
      
    // inspect values of all subexpressions

    double result = 0;
    for (std::list<double>::iterator value = values.begin();
         value != values.end(); ++value)
      {
        // if the value is unknow, the entire expression is unknown

        if (*value == BooleanVariable::UNKNOWN())
          {
            result = BooleanVariable::UNKNOWN();
            break;
          }
        // if this is the first value init result to it's value

        if (value == values.begin())
          result = *value;

        // otherwise the value is the XOR of the result and the new value

        else
          result = 
            (result == BooleanVariable::TRUE() && 
             *value == BooleanVariable::FALSE()) ||
            (result == BooleanVariable::FALSE() && 
             *value == BooleanVariable::TRUE());
      }
    // return the result
      
    return result;
  }

  std::string ExclusiveDisjunction::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " ^^ " 
           : ")");
      }
    return retval.str();
  }

  bool Concatenation::checkValue(const double val)
  {
    return LabelStr::isString(val);
  }

  double Concatenation::recalculate()
  {   
    std::ostringstream retval; 
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
        double value = (*child)->getValue();
        // values.push_back(value);
        
        // validate values (you look ma'valouse!)
        checkError(checkValue(value), "Invalid concatenation value: " << value);

        // if a sub expression is UNKNOWN return UNKNOWN
        if (value == UNKNOWN()){
          // LabelStr ls2 (value);
          return UNKNOWN();
        }
        LabelStr ls1 (value);
        retval << ls1.toString();
      }
    LabelStr retvalLabel(retval.str());
    return retvalLabel.getKey();
  }

  std::string Concatenation::toString() const
  {
    std::ostringstream retval;
    retval << NaryExpression::toString() << "(";
    for (ExpressionVectorConstIter child = m_subexpressions.begin();
         child != m_subexpressions.end(); ++child)
      {
         
        retval << (*child)->toString() << 
          (*child != m_subexpressions.back()
           ? " + " 
           : ")");
      }
    return retval.str();
  }

  //
  // Comparisons
  //

  bool Equality::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Equality::recalculate() {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();

    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 == v2);
    return value;
  }

  std::string Equality::toString() const {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " == " << m_b->toString() << "))";
    return retval.str();
  }

  bool Inequality::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double Inequality::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 != v2);
    return value;
  }

  std::string Inequality::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " != " << m_b->toString() << "))";
    return retval.str();
  }

  bool LessThan::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LessThan::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    double value;
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      value = UNKNOWN();
    else
      value = (double) (v1 < v2);
    return value;
  }

  std::string LessThan::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " < " << m_b->toString() << "))";
    return retval.str();
  }

  bool LessEqual::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double LessEqual::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 <= v2);
  }

  std::string LessEqual::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " <= " << m_b->toString() << "))";
    return retval.str();
  }

  bool GreaterThan::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double GreaterThan::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 > v2);
  }

  std::string GreaterThan::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " > " << m_b->toString() << "))";
    return retval.str();
  }

  bool GreaterEqual::checkValue(const double val) {
    return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE() ||
      val == BooleanVariable::UNKNOWN();
  }

  double GreaterEqual::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 >= v2);
  }

  std::string GreaterEqual::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " >= " << m_b->toString() << "))";
    return retval.str();
  }


  //
  // Arithmetic expressions
  //

  double Addition::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if (v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 + v2);
  }

  std::string Addition::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " + " << m_b->toString() << "))";
    return retval.str();
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


  double Subtraction::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 - v2);
  }


  std::string Subtraction::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " - " << m_b->toString() << "))";
    return retval.str();
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


  double Multiplication::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();
    return (double) (v1 * v2);
  }

  std::string Multiplication::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " * " << m_b->toString() << "))";
    return retval.str();
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


  double Division::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();

    check_error(v2 != 0.0, "Attempt to divide by zero");

    return (double) (v1 / v2);
  }

  std::string Division::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " / " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Division::getValueType() const
  {
    return REAL;
  }


  double Modulo::recalculate()
  {
    double v1 = m_a->getValue();
    double v2 = m_b->getValue();
    if(v1 == UNKNOWN() || v2 == UNKNOWN())
      return UNKNOWN();

    check_error(v2 != 0.0, "Attempt to divide by zero");

    return (double) fmod (v1, v2);
  }

  std::string Modulo::toString() const
  {
    std::ostringstream retval;
    retval << BinaryExpression::toString();
    retval << "(" << m_a->toString() << " % " << m_b->toString() << "))";
    return retval.str();
  }

  PlexilType Modulo::getValueType() const
  {
    return REAL;
  }

}
