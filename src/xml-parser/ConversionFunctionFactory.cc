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

#include "ConversionFunctionFactory.hh"

#include "createExpression.hh"
#include "Function.hh"
#include "ParserException.hh"
#include "parser-utils.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{

  ConversionFunctionFactory::ConversionFunctionFactory(Operator const *integerOp,
                                                       Operator const *realOp,
                                                       std::string const &name)
    : FunctionFactory(NULL, name),
      m_intOp(integerOp),
      m_realOp(realOp)
  {
  }

  ConversionFunctionFactory::~ConversionFunctionFactory()
  {
  }

  ValueType ConversionFunctionFactory::check(char const *nodeId, xml_node const expr) const
  {
    // Check arg count
    size_t n = std::distance(expr.begin(), expr.end());
    checkParserExceptionWithLocation(n == 1,
                                     expr,
                                     "Operator " << expr.name()
                                     << " requires exactly one argument");
    

    // Recurse over children
    xml_node subexp = expr.first_child();
    ValueType t = checkExpression(nodeId, subexp);
    checkParserExceptionWithLocation(isNumericType(t) || t == UNKNOWN_TYPE,
                                     subexp,
                                     "Argument to " << expr.name() << " must be numeric");


    // KLUDGE
    return INTEGER_TYPE;
  }

  Operator const *ConversionFunctionFactory::selectOperator(ValueType returnType) const
  {
    switch (returnType) {
    case INTEGER_TYPE:
      return m_intOp;

    case REAL_TYPE:
      return m_realOp;
      
    default:
      checkParserException(false,
                           "createExpression: invalid or unimplemented return type "
                           << valueTypeName(returnType)
                           << " for operator " << this->m_name);
      return NULL;
    }
  }

  Expression *ConversionFunctionFactory::allocate(xml_node const expr,
                                                  NodeConnector *node,
                                                  bool & wasCreated,
                                                  ValueType returnType) const
  {
    if (returnType == UNKNOWN_TYPE) {
      // Unspecified - default to Integer

      // DEBUG 
      std::cerr << "ConversionFunctionFactory::allocate: unspecified return type for "
                << expr.name() << std::endl;
      returnType = INTEGER_TYPE;
    }
    
    Operator const *oper = this->selectOperator(returnType);
    if (!oper) {
      reportParserExceptionWithLocation(expr,
                                        "Operator " << expr.name()
                                        << " not implemented for return type "
                                        << valueTypeName(returnType));
    }

    Function *result = makeFunction(oper, 1);
    bool garbage;
    Expression *arg = createExpression(expr.first_child(), node, garbage);
    result->setArgument(0, arg, garbage);

    wasCreated = true;
    return result;
  }

} // namespace PLEXIL
