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

#include "Expression.hh"

#include "Error.hh"

#include <iostream>
#include <sstream>

namespace PLEXIL 
{
  Expression::Expression()
  {
  }

  Expression::Expression(Expression const &)
  {
  }

  Expression &Expression::operator=(Expression const &)
  {
    return *this;
  }

  Expression::~Expression()
  {
  }

  // Default method.
  char const *Expression::getName() const
  {
    static char const *sl_empty = "";
    return sl_empty;
  }

  // Default method.
  bool Expression::isAssignable() const
  {
    return false;
  }

  // Default methods.
  Assignable *Expression::asAssignable()
  {
    return NULL;
  }

  Assignable const *Expression::asAssignable() const
  {
    return NULL;
  }

  // Default method.
  bool Expression::isConstant() const
  {
    return false;
  }

  // Default method.
  Expression *Expression::getBaseExpression()
  {
    return this;
  }

  Expression const *Expression::getBaseExpression() const
  {
    return this;
  }

  void Expression::print(std::ostream& s) const
  {
    s << '(' << this->exprName() << ' ' 
      << valueTypeName(this->valueType()) << ' ';
    this->printSpecialized(s);
	s << this << " [" << (this->isActive() ? 'a' : 'i')
	  << "](";
	this->printValue(s);
	s << ')';
    this->printSubexpressions(s);
    s << ')';
  }

  // Default method, derived classes can elaborate
  void Expression::printSpecialized(std::ostream &s) const
  {
  }

  // Default method, derived classes can elaborate
  void Expression::printSubexpressions(std::ostream & /* s */) const
  {
  }

  // Stream-style print operator
  std::ostream& operator<<(std::ostream& s, const Expression& e)
  {
    e.print(s);
    return s;
  }

  std::string Expression::toString() const
  {
    std::ostringstream s;
    this->print(s);
    return s.str();
  }

  std::string Expression::valueString() const
  {
    std::ostringstream s;
    this->printValue(s);
    return s.str();
  }

  // Default method, does nothing.
  void Expression::notifyChanged(Expression const * /* src */)
  {
  }

  bool Expression::getValue(bool &) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValue() not implemented for Boolean for this expression");
    return false;
  }

  bool Expression::getValue(uint16_t &) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValue() not implemented for internal types for this expression");
    return false;
  }

  bool Expression::getValue(int32_t &) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValue() not implemented for Integer for this expression");
    return false;
  }

  bool Expression::getValue(double &) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValue() not implemented for Real for this expression");
    return false;
  }

  bool Expression::getValue(std::string &) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValue() not implemented for String for this expression");
    return false;
  }

  bool Expression::getValuePointer(std::string const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for String for this expression");
    return false;
  }

  bool Expression::getValuePointer(Array const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for Array for this expression");
    return false;
  }

  bool Expression::getValuePointer(BooleanArray const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for BooleanArray for this expression");
    return false;
  }

  bool Expression::getValuePointer(IntegerArray const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for IntegerArray for this expression");
    return false;
  }

  bool Expression::getValuePointer(RealArray const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for RealArray for this expression");
    return false;
  }

  bool Expression::getValuePointer(StringArray const *& /* ptr */) const
  {
    assertTrue_2(ALWAYS_FAIL, "getValuePointer() not implemented for StringArray for this expression");
    return false;
  }
    

} // namespace PLEXIL
