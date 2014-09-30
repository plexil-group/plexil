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

  Expression *ArithmeticFunctionFactory::allocate(PlexilExpr const *expr,
                                                  NodeConnector *node,
                                                  bool & wasCreated) const
  {
    PlexilOp const *op = dynamic_cast<PlexilOp const *>(expr);
    checkParserException(op, "Not a PlexilOp");

    std::vector<PlexilExpr *> const &args = op->subExprs();
    // Have to have at least one arg to check types on
    checkParserException(args.size() > 0,
                         "Can't create arithmetic expression of no arguments");
    ExprVec *exprVec = constructExprVec(args, node);
    ValueType type = this->commonType(exprVec);
    checkParserException(type != UNKNOWN_TYPE,
                         "Type inconsistency or indeterminacy in arithmetic expression");
    Operator const *oper = this->selectOperator(type);
    checkParserException(oper->checkArgCount(args.size()),
                         "Wrong number of operands for operator "
                         << oper->getName());

    wasCreated = true;
    return new Function(oper, exprVec);
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

} // namespace PLEXIL
