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
#include "ArithmeticOperators.hh"
#include "ArrayLiteralFactory.hh"
#include "ArrayOperators.hh"
#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "ArrayVariableFactory.hh"
#include "ArrayVariableReferenceFactory.hh"
#include "BooleanOperators.hh"
#include "Comparisons.hh"
#include "ComparisonFactory.hh"
#include "ConcreteExpressionFactory.hh"
#include "ConversionFunctionFactory.hh"
#include "Constant.hh"
#include "ConversionOperators.hh"
#include "createExpression.hh"
#include "Debug.hh"
#include "Error.hh"
#include "FunctionFactory.hh"
#include "InternalExpressionFactories.hh"
#include "LookupFactory.hh"
#include "NodeConnector.hh"
#include "NodeConstantExpressions.hh"
#include "NodeFunctionFactory.hh"
#include "NodeOperators.hh"
#include "NodeVariables.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "StringOperators.hh"
#include "UserVariable.hh"
#include "UserVariableFactory.hh"

#include "pugixml.hpp"

#ifdef HAVE_STRING_H
#include <cstring>
#endif

using pugi::xml_node;


// Local convenience macros
#define REGISTER_EXPRESSION(CLASS,NAME) new PLEXIL::ConcreteExpressionFactory<CLASS >(#NAME)
#define REGISTER_NAMED_CONSTANT_FACTORY(CLASS,NAME) new PLEXIL::NamedConstantExpressionFactory<CLASS >(#NAME)

namespace PLEXIL
{

  // Code generated by gperf
  #include "ExpressionMap.hh"

  ValueType checkExpression(char const *nodeId, xml_node const expr)
  {
    char const *name = expr.name();
    checkParserExceptionWithLocation(*name,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Expression is not an XML element");
    // Delegate to factory
    debugMsg("checkExpression", " name = " << name);
    ExpressionEntry const *entry = ExpressionMap::getFactory(name, strlen(name));
    checkParserExceptionWithLocation(entry,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Unknown expression \"" << name << "\".");

    return entry->factory->check(nodeId, expr);
  }

  ValueType checkAssignable(char const *nodeId, xml_node const expr)
  {
    char const *name = expr.name();
    checkParserExceptionWithLocation(*name,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Expression is not an XML element");
    checkParserExceptionWithLocation(testSuffix(VAR_SUFFIX, name)
                                     || !strcmp(ARRAYELEMENT_TAG, name),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Expression is not a legal Assignment, Command, or InOut alias target");
    return checkExpression(nodeId, expr);
  }
  
  Expression *createExpression(xml_node const expr,
                               NodeConnector *node)
  {
    bool dummy;
    return createExpression(expr, node, dummy, UNKNOWN_TYPE);
  }

  Expression *createExpression(xml_node const expr,
                               NodeConnector *node,
                               bool& wasCreated,
                               ValueType returnType)
  {
    char const *name = expr.name();
    checkParserExceptionWithLocation(*name, 
                                     expr.parent(),
                                     "createExpression: Not an XML element");
    // Delegate to factory
    debugMsg("createExpression", " name = " << name);
    ExpressionEntry const *entry = ExpressionMap::getFactory(name, strlen(name));
    // Should have been caught in checkExpression()
    assertTrueMsg(entry,
                  "createExpression: No factory registered for name \"" << name << "\".");

    Expression *retval = entry->factory->allocate(expr, node, wasCreated, returnType);
    debugMsg("createExpression",
             " Created " << (wasCreated ? "" : "reference to ") << retval->toString());
    return retval;
  }

  //
  // createAssignable
  //

  Assignable *createAssignable(xml_node const expr,
                               NodeConnector *node,
                               bool& wasCreated)
  {
    assertTrue_2(node, "createAssignable: Internal error: Null node argument");
    char const *name = expr.name();
    // Should have been caught in checkAssignable()
    assertTrueMsg(*name, "createAssignable: Not an XML element");
    Expression *resultExpr = NULL;
    if (testSuffix(VAR_SUFFIX, name))
      resultExpr = createExpression(expr, node, wasCreated);
    else if (!strcmp(ARRAYELEMENT_TAG, name))
      resultExpr = createMutableArrayReference(expr, node, wasCreated);
    else
      reportParserExceptionWithLocation(expr,
                                        "Invalid Assignment or InOut alias target");
    assertTrue_2(resultExpr, "createAssignable: Internal error: Null expression");
    if (!resultExpr->isAssignable()) {
      if (wasCreated)
        delete resultExpr;
      resultExpr = NULL;
      reportParserExceptionWithLocation(expr,
                                        "Expression is not assignable");
    }
    return resultExpr->asAssignable();
  }

  // FIXME
  // void purgeExpressionFactories()
  // {
  //   // Uncomment this to get a better estimate of factory map size.
  //   // std::cout << "ExpressionFactory map has " << s_expressionFactoryMap.size() << " entries" << std::endl;

  //   for (ExpressionFactoryMap::iterator it = s_expressionFactoryMap.begin();
  //        it != s_expressionFactoryMap.end();
  //        ++it) {
  //     ExpressionFactory* tmp = it->second;
  //     it->second = NULL;
  //     delete tmp;
  //   }
  //   s_expressionFactoryMap.clear();
  // }

} // namespace PLEXIL
