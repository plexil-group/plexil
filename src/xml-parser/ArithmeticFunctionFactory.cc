/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "createExpression.hh"
#include "Expression.hh"
#include "Error.hh"
#include "Function.hh"
#include "Operator.hh"
#include "ParserException.hh" // checkParserException()
#include "parser-utils.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{

  ArithmeticFunctionFactory::ArithmeticFunctionFactory(Operator const *integerOp,
                                                       Operator const *realOp,
                                                       std::string const &name)
    : FunctionFactory(NULL, name),
      m_intOp(integerOp),
      m_realOp(realOp)
  {
  }

  ArithmeticFunctionFactory::~ArithmeticFunctionFactory()
  {
  }

  // Common case logic
  static ValueType arithmeticCommonType(ValueType const types[], size_t len)
  {
    assertTrue_1(len > 0); // must have at least one operand

    ValueType result = UNKNOWN_TYPE;
    switch (types[0]) {
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
    case UNKNOWN_TYPE: // e.g. undeclared/indeterminate
      result = REAL_TYPE;
      break;

    case INTEGER_TYPE:
      result = INTEGER_TYPE;
      break;

    default: // anything else is not a valid type in an arithmetic expression
      return UNKNOWN_TYPE;
    }

    for (size_t i = 1; i < len; ++i) {
      switch (types[i]) {
      case REAL_TYPE:
      case DATE_TYPE:
      case DURATION_TYPE:
      case UNKNOWN_TYPE:
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

  ValueType ArithmeticFunctionFactory::check(char const *nodeId, xml_node const expr) const
  {
    // Check arg count
    size_t n = std::distance(expr.begin(), expr.end());
    // *** FIXME ***
    // Kludge: can't determine operator's type without any parameters
    checkParserExceptionWithLocation(n > 0,
                                     expr,
                                     "Wrong number of operands for operator "
                                     << expr.name());
    

    // Recurse over children
    ValueType types[n];
    xml_node subexp = expr.first_child();
    for (size_t i = 0; i < n; ++i) {
      types[i] = checkExpression(nodeId, subexp);
      subexp = subexp.next_sibling();
    }

    // Determine return type if possible
    return arithmeticCommonType(types, n);
  }

  Operator const *ArithmeticFunctionFactory::selectOperator(ValueType type) const
  {
    switch (type) {
    case INTEGER_TYPE:
      return m_intOp;

    case REAL_TYPE:
      return m_realOp;
      
    default:
      checkParserException(false,
                           "createExpression: invalid or unimplemented return type "
                           << valueTypeName(type)
                           << " for operator " << this->m_name);
      return NULL;
    }
  }

  Expression *ArithmeticFunctionFactory::allocate(xml_node const expr,
                                                  NodeConnector *node,
                                                  bool & wasCreated,
                                                  ValueType returnType) const
  {
    // Need to check operands to determine operator type
    size_t n = std::distance(expr.begin(), expr.end());
    Expression *exprs[n];
    bool garbage[n];
    size_t i = 0;
    try {
      for (xml_node subexp = expr.first_child();
           i < n;
           subexp = subexp.next_sibling(), ++i)
        exprs[i] = createExpression(subexp, node, garbage[i]);
    }
    catch (ParserException & /* e */) {
      // Clean up expressions we successfully constructed
      for (size_t j = 0; j < i; ++j)
        if (garbage[j])
          delete exprs[j];
      throw;
    }

    if (returnType == UNKNOWN_TYPE) {
      // Unspecified - default it from types of parameters
      ValueType types[n];
      for (size_t i = 0; i < n ; ++i)
        types[i] = exprs[i]->valueType();
      returnType = arithmeticCommonType(types, n);
    }

    if (returnType == UNKNOWN_TYPE) {
      // Clean up before throwing
      for (i = 0; i < n; ++i)
        if (garbage[i])
          delete exprs[i];
      reportParserExceptionWithLocation(expr,
                                        "Type inconsistency or indeterminacy in arithmetic expression");
    }
    
    Operator const *oper = this->selectOperator(returnType);
    if (!oper) {
      // Clean up before throwing
      for (i = 0; i < n; ++i)
        if (garbage[i])
          delete exprs[i];
      reportParserExceptionWithLocation(expr,
                                        "Operator " << expr.name()
                                        << " not implemented for return type "
                                        << valueTypeName(returnType));
    }

    if (!oper->checkArgCount(n)) {
      // Clean up before throwing
      for (i = 0; i < n; ++i)
        if (garbage[i])
          delete exprs[i];
      reportParserExceptionWithLocation(expr,
                                        "Wrong number of operands for operator "
                                        << expr.name());
    }

    Function *result = makeFunction(oper, n);
    for (i = 0; i < n; ++i)
      result->setArgument(i, exprs[i], garbage[i]);

    wasCreated = true;
    return result;
  }

} // namespace PLEXIL
