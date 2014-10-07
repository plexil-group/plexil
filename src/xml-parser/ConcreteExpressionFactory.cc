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

#include "ConcreteExpressionFactory.hh"

#include "ArrayReference.hh"
#include "ArrayVariable.hh"
#include "Constant.hh"
#include "Error.hh"
#include "ExpressionConstants.hh"
#include "NodeConnector.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"
#include "UserVariable.hh"

#include "pugixml.hpp"

#include <sstream>

// Local convenience macros
#define ENSURE_EXPRESSION_FACTORY(CLASS) template class PLEXIL::ConcreteExpressionFactory<CLASS >;

namespace PLEXIL
{

  //
  // Factories for scalar constants
  //

  // N.B. For all but string types, the value string may not be empty.
  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::allocate(pugi::xml_node const expr,
                                                                NodeConnector * /* node */,
                                                                bool &wasCreated) const
  {
    // confirm that we have a value element
    checkTagSuffix(VAL_SUFFIX, expr);

    // establish value type
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_SUFFIX));
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     expr,
                                     "Unrecognized value type \"" << tag << "\"");

    // check for empty value
    if (typ != STRING_TYPE)
      checkParserExceptionWithLocation(expr.first_child() && *(expr.child_value()),
                                       expr,
                                       "Empty value is not valid for \"" << tag << "\"");

    wasCreated = true;
    return this->create(expr);
  }

  // Since there are exactly 3 possible Boolean constants, return references to them.
  template <>
  Expression *ConcreteExpressionFactory<Constant<bool> >::allocate(pugi::xml_node const expr,
                                                                   NodeConnector * /* node */,
                                                                   bool &wasCreated) const
  {
    // confirm that we have a value element
    checkTagSuffix(VAL_SUFFIX, expr);

    // establish value type
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_SUFFIX));
    checkParserExceptionWithLocation(typ == BOOLEAN_TYPE,
                                     expr,
                                     "Internal error: Boolean constant factory invoked on \"" << tag << "\"");

    // check for empty value
    checkParserExceptionWithLocation(expr.first_child() && *(expr.child_value()),
                                     expr,
                                     "Empty value is not valid for \"" << tag << "\"");

    bool value;
    bool known = parseValue(expr.child_value(), value);
    // if we got here, there was no parsing exception
    wasCreated = false;
    if (!known)
      return UNKNOWN_BOOLEAN_EXP();
    else if (value)
      return TRUE_EXP();
    else
      return FALSE_EXP();
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::create(pugi::xml_node const tmpl) const
  {
    T value;
    bool known = parseValue<T>(tmpl.child_value(), value);
    if (known)
      return new Constant<T>(value);
    else
      return new Constant<T>();
  }
  
  // String is different

  template <>
  Expression *ConcreteExpressionFactory<Constant<std::string> >::create(pugi::xml_node const tmpl) const
  {
    const char* tag = tmpl.name();
    checkParserExceptionWithLocation(STRING_TYPE == parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_SUFFIX)),
                                     tmpl,
                                     "Internal error: Constant expression is not a String");

    return new Constant<std::string>(tmpl.child_value());
  }

  //
  // Factories for scalar variables
  //

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::allocate(pugi::xml_node const expr,
                                                        NodeConnector *node,
                                                        bool &wasCreated) const
  {
    // Variable reference - look it up
    checkTagSuffix(VAR_SUFFIX, expr);
    checkNotEmpty(expr);
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAR_SUFFIX));
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     expr,
                                     "Unknown variable type \"" << tag << "\"");
    checkParserExceptionWithLocation(node,
                                     expr,
                                     "Internal error: Variable reference with null node");
    const char *varName = expr.child_value();
    Expression *result = node->findVariable(std::string(varName));
    checkParserExceptionWithLocation(result,
                                     expr,
                                     "Can't find variable named " << varName);
    checkParserExceptionWithLocation(result->valueType() == typ,
                                     expr,
                                     "Variable " << varName
                                     << " is type " << valueTypeName(result->valueType())
                                     << ", but reference is for type " << valueTypeName(typ));
    wasCreated = false;
    return result;
  }

  //
  // Array references
  //

  // Common subroutine
  static void parseArrayElement(pugi::xml_node const expr,
                                NodeConnector *node,
                                Expression *&arrayExpr,
                                Expression *&indexExpr,
                                bool &arrayCreated,
                                bool &indexCreated)
  {
    // Syntax checks
    checkHasChildElement(expr);
    pugi::xml_node nameXml = expr.first_child();
    checkParserExceptionWithLocation(nameXml && testTag(NAME_TAG, nameXml),
                                     expr,
                                     "ArrayElement has no Name element");
    checkNotEmpty(nameXml);
    pugi::xml_node indexXml = nameXml.next_sibling();
    checkParserExceptionWithLocation(indexXml && testTag(INDEX_TAG, indexXml),
                                     expr,
                                     "ArrayElement has no Index element");
    checkHasChildElement(indexXml);
    indexXml = indexXml.first_child();
    checkParserExceptionWithLocation(indexXml.type() == pugi::node_element,
                                     indexXml,
                                     "ArrayElement Index is not an element");

    // Checks on array
    const char *arrayName = nameXml.child_value();
    arrayExpr = node->findVariable(std::string(arrayName));
    checkParserExceptionWithLocation(arrayExpr,
                                     nameXml,
                                     "No array variable named \""
                                     << arrayName << "\" accessible from node "
                                     << node->getNodeId());
    checkParserExceptionWithLocation(isArrayType(arrayExpr->valueType()),
                                     nameXml,
                                     "Variable \"" << arrayName
                                     << "\" is not an array variable");

    // Checks on index
    indexExpr = createExpression(indexXml, node, indexCreated);
    assertTrue_1(indexExpr);
    ValueType indexType = indexExpr->valueType();
    checkParserExceptionWithLocation(indexType == INTEGER_TYPE || indexType == UNKNOWN_TYPE,
                                     indexXml,
                                     "Array index expression is not numeric");
  }

  Expression *
  ConcreteExpressionFactory<ArrayReference>::allocate(pugi::xml_node const expr,
                                                      NodeConnector *node,
                                                      bool & wasCreated) const
  {
    Expression *arrayExpr = NULL;
    Expression *indexExpr = NULL;
    bool arrayCreated = false;
    bool indexCreated = false;
    
    parseArrayElement(expr, node, arrayExpr, indexExpr, arrayCreated, indexCreated);

    wasCreated = true;
    return new ArrayReference(arrayExpr, indexExpr, arrayCreated, indexCreated);
  }

  Expression *createMutableArrayReference(pugi::xml_node const expr,
                                          NodeConnector *node,
                                          bool & wasCreated)
  {
    Expression *arrayExpr = NULL;
    Expression *indexExpr = NULL;
    bool arrayCreated = false;
    bool indexCreated = false;
    
    parseArrayElement(expr, node, arrayExpr, indexExpr, arrayCreated, indexCreated);

    wasCreated = true;
    return new MutableArrayReference(arrayExpr, indexExpr, arrayCreated, indexCreated);
  }

  // Generic variable references
  Expression *
  VariableReferenceFactory::allocate(pugi::xml_node const expr,
                                     NodeConnector *node,
                                     bool & wasCreated) const
  {
    checkParserExceptionWithLocation(testTagSuffix(VAR_SUFFIX, expr),
                                     expr,
                                     "Internal error: not a variable reference");
    assertTrue_1(node); // internal error
    checkNotEmpty(expr);
    ValueType typ = parseValueTypePrefix(expr.name() , strlen(expr.name()) - strlen(VAR_SUFFIX));
    checkParserExceptionWithLocation(typ != UNKNOWN_TYPE,
                                     expr,
                                     "Unknown variable reference type " << expr.name());
    char const *varName = expr.child_value();
    // Look it up
    std::string const varRef(varName);
    Expression *result = node->findVariable(varRef);
    checkParserExceptionWithLocation(result,
                                     expr,
                                     "Can't find variable named " << varName);
    if (typ == ARRAY_TYPE) {
      checkParserExceptionWithLocation(isArrayType(result->valueType()),
                                       expr,
                                       "Variable " << varName
                                       << " has invalid type " << valueTypeName(result->valueType())
                                       << " for a " << expr.name());
    }
    else {
      checkParserExceptionWithLocation(typ == result->valueType(),
                                       expr,
                                       "Variable " << varName
                                       << " has invalid type " << valueTypeName(result->valueType())
                                       << " for a " << expr.name());
    }
    wasCreated = false;
    return result;
  }

  // Explicit instantiations
  ENSURE_EXPRESSION_FACTORY(BooleanConstant);
  ENSURE_EXPRESSION_FACTORY(IntegerConstant);
  ENSURE_EXPRESSION_FACTORY(RealConstant);
  ENSURE_EXPRESSION_FACTORY(StringConstant);

  ENSURE_EXPRESSION_FACTORY(BooleanVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerVariable);
  ENSURE_EXPRESSION_FACTORY(RealVariable);
  ENSURE_EXPRESSION_FACTORY(StringVariable);
  ENSURE_EXPRESSION_FACTORY(BooleanArrayVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerArrayVariable);
  ENSURE_EXPRESSION_FACTORY(RealArrayVariable);
  ENSURE_EXPRESSION_FACTORY(StringArrayVariable);

} // namespace PLEXIL

