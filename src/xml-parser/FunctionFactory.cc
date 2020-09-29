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

#include "FunctionFactory.hh"

#include "CachedFunction.hh"
#include "createExpression.hh"
#include "Function.hh"
#include "Operator.hh"
#include "ParserException.hh"
#include "parser-utils.hh"

#include "pugixml.hpp"

namespace PLEXIL
{
  FunctionFactory::FunctionFactory(Operator const *op, std::string const &name)
    : ExpressionFactory(name),
      m_op(op)
  {
  }

  FunctionFactory::~FunctionFactory()
  {
  }

  CachedFunctionFactory::CachedFunctionFactory(Operator const *op, std::string const &name)
    : FunctionFactory(op, name)
  {
  }

  CachedFunctionFactory::~CachedFunctionFactory()
  {
  }

  ValueType FunctionFactory::check(char const *nodeId, pugi::xml_node expr) const
  {
    size_t n = std::distance(expr.begin(), expr.end());
    Operator const *oper = m_op;
    assertTrueMsg(oper, "FunctionFactory::check: no operator for " << m_name);
    checkParserExceptionWithLocation(oper->checkArgCount(n),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Wrong number of operands for operator "
                                     << oper->getName());

    // Check arguments
    // TODO: check types
    for (pugi::xml_node subexp = expr.first_child();
         subexp;
         subexp = subexp.next_sibling())
      checkExpression(nodeId, subexp);

    return oper->valueType();
  }

  Expression *FunctionFactory::allocate(pugi::xml_node const expr,
                                        NodeConnector *node,
                                        bool &wasCreated,
                                        ValueType returnType) const
  {
    size_t n = std::distance(expr.begin(), expr.end());
    Operator const *oper = m_op;
    Function *result = this->constructFunction(oper, n);
    try {
      size_t i = 0;
      for (pugi::xml_node subexp = expr.first_child();
           subexp && i < n;
           subexp = subexp.next_sibling(), ++i) {
        bool created;
        Expression *arg = createExpression(subexp, node, created, returnType);
        result->setArgument(i, arg, created);
      }
    }
    catch (ParserException & /* e */) {
      delete result;
      throw;
    }

    if (!oper->checkArgTypes(result)) {
      delete result;
      reportParserExceptionWithLocation(expr,
                                        "Operand type mismatch or unimplemented type for "
                                        << oper->getName());
    }

    wasCreated = true;
    return result;
  }

  Function *FunctionFactory::constructFunction(Operator const *op, size_t n) const
  {
    return makeFunction(op,n);
  }

  Function *CachedFunctionFactory::constructFunction(Operator const *op, size_t n) const
  {
    return makeCachedFunction(op,n);
  }

} // namespace PLEXIL
