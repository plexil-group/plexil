/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "PlanError.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL 
{
  Expression::Expression()
  {
  }

  Expression::~Expression()
  {
  }

  // Default method.
  char const *Expression::getName() const
  {
    return "";
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

  void Expression::print(std::ostream& str) const
  {
    str << '(' << this->exprName() << ' ' 
      << valueTypeName(this->valueType()) << ' ';
    this->printSpecialized(str);
	str << this << " [" << (this->isActive() ? 'a' : 'i')
	  << "](";
	this->printValue(str);
	str << ')';
    this->printSubexpressions(str);
    str << ')';
  }

  // Default method, derived classes can elaborate
  void Expression::printSpecialized(std::ostream & /* str */) const
  {
  }

  // Default method, derived classes can elaborate
  void Expression::printSubexpressions(std::ostream & /* str */) const
  {
  }

  // Stream-style print operator
  std::ostream& operator<<(std::ostream& str, const Expression& exp)
  {
    exp.print(str);
    return str;
  }

  std::string Expression::toString() const
  {
    std::ostringstream strm;
    this->print(strm);
    return strm.str();
  }

  std::string Expression::valueString() const
  {
    std::ostringstream strm;
    this->printValue(strm);
    return strm.str();
  }

  // Default methods always throw PlanError

#define DEFINE_DEFAULT_GET_VALUE_METHOD(_TYPE_) \
  bool Expression::getValue(_TYPE_ & /* result */) const    \
  { \
    reportPlanError("Can't get a " << PlexilValueType<_TYPE_>::typeName \
                    << " value from a " << valueTypeName(this->valueType()) << " expression"); \
    return false; \
  }

  DEFINE_DEFAULT_GET_VALUE_METHOD(Boolean)
  DEFINE_DEFAULT_GET_VALUE_METHOD(Integer)
  DEFINE_DEFAULT_GET_VALUE_METHOD(String)

#undef DEFINE_DEFAULT_GET_VALUE_METHOD

  bool Expression::getValue(uint16_t & /* result */) const
  {
    reportPlanError("Can't get a Plexil internal value from a "
                   << valueTypeName(this->valueType()) << " expression");
    return false;
  }

  // Conversion method for Integer-valued expressions
  bool Expression::getValue(Real &result) const
  {
    checkPlanError(this->valueType() == INTEGER_TYPE,
                   "Can't get a " << PlexilValueType<Real>::typeName
                   << " value from a " << valueTypeName(this->valueType()) << " expression");
    Integer temp;
    if (this->getValue(temp)) {
      result = (Real) temp;
      return true;
    }
    return false; // unknown
  }

#define DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(_TYPE_) \
  bool Expression::getValuePointer(_TYPE_ const *& /* ptr */) const \
  { \
    reportPlanError("Can't get a pointer to " << PlexilValueType<_TYPE_>::typeName \
                    << " from a " << valueTypeName(this->valueType()) << " expression"); \
    return false; \
  }

  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(String)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(Array)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(BooleanArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(IntegerArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(RealArray)
  DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_DEFAULT_GET_VALUE_POINTER_METHOD

} // namespace PLEXIL
