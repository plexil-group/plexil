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

#include "ComparisonFactory.hh"

#include "createExpression.hh"
#include "Expression.hh"
#include "Function.hh"
#include "ParserException.hh" // checkParserException() macro
#include "parser-utils.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{

  ComparisonFactory::ComparisonFactory(Operator const *integerOp,
                                       Operator const *realOp,
                                       Operator const *stringOp,
                                       std::string const &name)
    : FunctionFactory(NULL, name),
      m_intOp(integerOp),
      m_realOp(realOp),
      m_stringOp(stringOp)
  {
  }

  ComparisonFactory::~ComparisonFactory()
  {
  }

  static ValueType comparisonCommonType(ValueType const types[2])
  {
    ValueType arg1Type = types[0];

    switch (arg1Type) {
      // Aliases for Real
    case DATE_TYPE:
    case DURATION_TYPE:
      arg1Type = REAL_TYPE;

      // Legal comparison types
    case REAL_TYPE:
    case INTEGER_TYPE:
    case STRING_TYPE:

      // Defer decision to below
    case UNKNOWN_TYPE:
      break;

    default: // Not a valid comparison type
      return UNKNOWN_TYPE;
    }

    switch (types[1]) {
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
      if (arg1Type == INTEGER_TYPE || arg1Type == REAL_TYPE || arg1Type == UNKNOWN_TYPE)
        return REAL_TYPE;
      return UNKNOWN_TYPE; // invalid comparison

    case INTEGER_TYPE:
      if (arg1Type == INTEGER_TYPE)
        return INTEGER_TYPE;
      else if (arg1Type == REAL_TYPE || arg1Type == UNKNOWN_TYPE)
        return REAL_TYPE;
      return UNKNOWN_TYPE; // invalid comparison

    case STRING_TYPE:
      if (arg1Type == STRING_TYPE || arg1Type == UNKNOWN_TYPE)
        return STRING_TYPE;
      return UNKNOWN_TYPE; // invalid comparison

    case UNKNOWN_TYPE:
      if (arg1Type == INTEGER_TYPE || arg1Type == REAL_TYPE)
        return REAL_TYPE;
      else if (arg1Type == STRING_TYPE)
        return STRING_TYPE;
      return UNKNOWN_TYPE;

    default:
      return UNKNOWN_TYPE; // invalid comparison
    }
  }

  ValueType ComparisonFactory::check(char const *nodeId, pugi::xml_node const expr) const
  {
    // Check arg count
    checkParserExceptionWithLocation(std::distance(expr.begin(), expr.end()) == 2,
                                     expr,
                                     "Comparison " << expr.name()
                                     << " requires exactly 2 operands");
    // Recurse over children
    ValueType types[2];
    types[0] = checkExpression(nodeId, expr.first_child());
    types[1] = checkExpression(nodeId, expr.first_child().next_sibling());

    // Check comparability
    checkParserExceptionWithLocation(comparisonCommonType(types) != UNKNOWN_TYPE,
                                     expr,
                                     "Inconsistent or unimplemented operand types for comparison "
                                     << expr.name());

    return BOOLEAN_TYPE;
  }

  Operator const *ComparisonFactory::selectOperator(ValueType type) const
  {
    switch (type) {
    case INTEGER_TYPE:
      return m_intOp;

    case REAL_TYPE:
      return m_realOp;
      
    case STRING_TYPE:
      return m_stringOp;

    default:
      checkParserException(false,
                           "createExpression: invalid or unimplemented argument type "
                           << valueTypeName(type)
                           << " for comparison operator " << this->m_name);
      return NULL;
    }
  }

  Expression *ComparisonFactory::allocate(xml_node const expr,
                                          NodeConnector *node,
                                          bool & wasCreated,
                                          ValueType returnType) const
  {
    // Check for internal error
    assertTrue_1(std::distance(expr.begin(), expr.end()) == 2);
    bool garbage[2];
    Expression *exprs[2];
    exprs[0] = createExpression(expr.first_child(), node, garbage[0]);
    try {
      exprs[1] = createExpression(expr.first_child().next_sibling(), node, garbage[1]);
    }
    catch (ParserException & /* e */) {
      // Clean up other expression if was constructed
      if (garbage[0])
        delete exprs[0];
      throw;
    }

    ValueType types[2];
    types[0] = exprs[0]->valueType();
    types[1] = exprs[1]->valueType();
    ValueType commonType = comparisonCommonType(types);

    if (commonType == UNKNOWN_TYPE) {
      // Clean up before throwing
      if (garbage[0])
        delete exprs[0];
      if (garbage[1])
        delete exprs[1];
      reportParserExceptionWithLocation(expr,
                                        "Type inconsistency or indeterminacy in comparison "
                                        << expr.name());
    }
    
    Operator const *oper = this->selectOperator(commonType);
    // Shouldn't happen, but...
    if (!oper) {
      // Clean up before throwing
      if (garbage[0])
        delete exprs[0];
      if (garbage[1])
        delete exprs[1];
      reportParserExceptionWithLocation(expr,
                                        "Operator " << expr.name()
                                        << " not implemented for return type "
                                        << valueTypeName(returnType));
    }

    Function *result = makeFunction(oper, 2);
    result->setArgument(0, exprs[0], garbage[0]);
    result->setArgument(1, exprs[1], garbage[1]);

    wasCreated = true;
    return result;
  }

} // namespace PLEXIL
