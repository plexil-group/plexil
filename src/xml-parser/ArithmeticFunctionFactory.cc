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

#include "ArithmeticFunctionFactory.hh"

#include "ArithmeticOperators.hh"
#include "Comparisons.hh"
#include "Error.hh"
#include "parser-utils.hh"

#include "pugixml.hpp"

namespace PLEXIL
{
  // Common case logic
  ValueType arithmeticCommonType(ExprVec const *exprs)
  {
    size_t len = exprs->size();
    assertTrue_1(len > 0);
    ValueType result = UNKNOWN_TYPE;
    switch ((*exprs)[0]->valueType()) {
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
    case UNKNOWN_TYPE: // e.g. lookup, command - assume the worst
      result = REAL_TYPE;
      break;

    case INTEGER_TYPE:
      result = INTEGER_TYPE;
      break;

    default: // not a valid type in an arithmetic expression
      return UNKNOWN_TYPE;
    }

    for (size_t i = 1; i < len; ++i) {
      switch ((*exprs)[i]->valueType()) {
      case REAL_TYPE:
      case DATE_TYPE:
      case DURATION_TYPE:
      case UNKNOWN_TYPE: // e.g. lookup, command - assume the worst
        result = REAL_TYPE;
        break;

      case INTEGER_TYPE:
        if (result != REAL_TYPE)
          result = INTEGER_TYPE;
        break;

      default:
        return UNKNOWN_TYPE; // bail out early
      }
    }
    // No type info? Choose a "safe" default.
    if (result == UNKNOWN_TYPE)
      result = REAL_TYPE;
    return result;
  }

  ArithmeticFunctionFactory::ArithmeticFunctionFactory(std::string const &name)
    : FunctionFactory(name)
  {
  }

  ArithmeticFunctionFactory::~ArithmeticFunctionFactory()
  {
  }

  Expression *ArithmeticFunctionFactory::allocate(pugi::xml_node const expr,
                                                  NodeConnector *node,
                                                  bool & wasCreated) const
  {
    // Get subexpressions
    checkHasChildElement(expr);
    ExprVec *exprVec = this->constructExprVec(expr, node);
    ValueType type = this->commonType(exprVec);
    checkParserExceptionWithLocation(type != UNKNOWN_TYPE,
                                     expr,
                                     "Type inconsistency or indeterminacy in arithmetic expression");
    Operator const *oper = this->selectOperator(type);
    checkParserExceptionWithLocation(oper->checkArgCount(exprVec->size()),
                                     expr,
                                     "Wrong number of operands for operator "
                                     << oper->getName());

    wasCreated = true;
    return new Function(oper, exprVec);
  }

  // Convenience macro
#define ENSURE_ARITHMETIC_FUNCTION_FACTORY(CLASS) template class PLEXIL::ArithmeticFunctionFactoryImpl<CLASS>;

  // Comparisons
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Equal);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(NotEqual);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(GreaterThan);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(GreaterEqual);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(LessThan);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(LessEqual);

  // Arithmetic operators
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Addition);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Subtraction);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Multiplication);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Division);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Modulo);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Minimum);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Maximum);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(AbsoluteValue);

  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Ceiling);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Floor);
  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Round);
  ENSURE_ARITHMETIC_FUNCTION_FACTORY(Truncate);
#endif // !defined(__VXWORKS__)

} // namespace PLEXIL
