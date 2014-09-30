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
#include "expression-schema.hh"
#include "NodeConnector.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "PlexilExpr.hh"
#include "UserVariable.hh"

#include "pugixml.hpp"

#include <sstream>

namespace PLEXIL
{

  //
  // Factories for scalar constants
  //

  // N.B. For all but string types, the value string may not be empty.
  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::allocate(PlexilExpr const *expr,
                                                                NodeConnector * /* node */,
                                                                bool &wasCreated) const
  {
    PlexilValue const *tmpl = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(tmpl, "Expression is not a PlexilValue");

    wasCreated = true;
    return this->create(tmpl);
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::allocate(pugi::xml_node const &expr,
                                                                NodeConnector * /* node */,
                                                                bool &wasCreated) const
  {
    // confirm that we have a value element
    checkTagSuffix(VAL_TAG, expr);

    // establish value type
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_TAG));
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
  Expression *ConcreteExpressionFactory<Constant<bool> >::allocate(PlexilExpr const *expr,
                                                                   NodeConnector * /* node */,
                                                                   bool &wasCreated) const
  {
    PlexilValue const *tmpl = dynamic_cast<PlexilValue const *>(expr);
    checkParserException(tmpl, "Expression is not a PlexilValue");
    bool value;
    bool known = parseValue(tmpl->value(), value);
    // if we got here, there was no parsing exception
    wasCreated = false;
    if (!known)
      return UNKNOWN_BOOLEAN_EXP();
    else if (value)
      return TRUE_EXP();
    else
      return FALSE_EXP();
  }

  template <>
  Expression *ConcreteExpressionFactory<Constant<bool> >::allocate(pugi::xml_node const &expr,
                                                                   NodeConnector * /* node */,
                                                                   bool &wasCreated) const
  {
    // confirm that we have a value element
    checkTagSuffix(VAL_TAG, expr);

    // establish value type
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_TAG));
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
  Expression *ConcreteExpressionFactory<Constant<T> >::create(PlexilValue const *tmpl) const
  {
    T value;
    bool known = parseValue(tmpl->value(), value);
    if (known)
      return new Constant<T>(value);
    else
      return new Constant<T>();
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<T> >::create(pugi::xml_node const &tmpl) const
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
  Expression *ConcreteExpressionFactory<Constant<std::string> >::create(PlexilValue const *tmpl) const
  {
    checkParserException(tmpl->type() == STRING_TYPE, "Internal error: Constant expression is not a String");
    return new Constant<std::string>(tmpl->value());
  }

  template <>
  Expression *ConcreteExpressionFactory<Constant<std::string> >::create(pugi::xml_node const &tmpl) const
  {
    const char* tag = tmpl.name();
    checkParserExceptionWithLocation(STRING_TYPE == parseValueTypePrefix(tag, strlen(tag) - strlen(VAL_TAG)),
                                     tmpl,
                                     "Internal error: Constant expression is not a String");

    return new Constant<std::string>(tmpl.child_value());
  }

  //
  // Factories for array constants
  // *** TO BE DELETED ***
  //

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<ArrayImpl<T> > >::allocate(PlexilExpr const *expr,
                                                                            NodeConnector * /* node */,
                                                                            bool &wasCreated) const
  {
    PlexilArrayValue const *val = dynamic_cast<PlexilArrayValue const *>(expr);
    checkParserException(val, "Not an array value");

    wasCreated = true;
    return this->create(val);
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<ArrayImpl<T> > >::create(PlexilArrayValue const *val) const
  {
    unsigned arraySize = val->maxSize();
    std::vector<std::string> const &eltVals = val->values();
    ArrayImpl<T> initVals(arraySize);
    for (size_t i = 0; i < eltVals.size(); ++i) {
      // Parse an element value and push it onto the vector
      T temp;
      if (parseValue<T>(eltVals[i], temp)) // will throw if format error
        initVals.setElement(i, temp);
      else
        initVals.setElementUnknown(i);
    }
    return new Constant<ArrayImpl<T> >(initVals);
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<Constant<ArrayImpl<T> > >::allocate(pugi::xml_node const &expr,
                                                                            NodeConnector * /* node */,
                                                                            bool &wasCreated) const
  {
    assertTrue_2(ALWAYS_FAIL, "Nothing should ever call this method!");
    return NULL;
  }

  //
  // Factories for scalar variables
  //

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::allocate(PlexilExpr const *expr,
                                                        NodeConnector *node,
                                                        bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    if (varRef) {
      // Variable reference - look it up
      checkParserException(node,
                           "Variable reference with null node"); // ??
      Expression *result = node->findVariable(varRef);
      checkParserException(result, "Can't find variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilVar const *var = dynamic_cast<PlexilVar const *>(expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(false, "Expression is neither a variable definition nor a variable reference");
    return NULL;
  }

  // DeclareVariable needs to be handled elsewhere as the type name is not in the tag.

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::allocate(pugi::xml_node const &expr,
                                                        NodeConnector *node,
                                                        bool &wasCreated) const
  {
    // Variable reference - look it up
    checkTagSuffix(VAR_TAG, expr);
    checkNotEmpty(expr);
    const char* tag = expr.name();
    ValueType typ = parseValueTypePrefix(tag, strlen(tag) - strlen(VAR_TAG));
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

  template <typename T>
  Expression *
  ConcreteExpressionFactory<UserVariable<T> >::create(PlexilVar const *var,
                                                      NodeConnector *node) const
  {
    bool garbage = false;
    Assignable *result = new UserVariable<T>(node, var->varName());
    PlexilExpr const *initval = var->value(); 
    if (initval) {
      Expression *initexp = createExpression(initval, node, garbage);
      result->setInitializer(initexp, garbage);
    }
    return static_cast<Expression *>(result);
  }

  //
  // Array variables
  //

  template <typename T>
  Expression *ConcreteExpressionFactory<ArrayVariable<T> >::allocate(PlexilExpr const *expr,
                                                                     NodeConnector *node,
                                                                     bool &wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    if (varRef) {
      // Variable reference - look it up
      checkParserException(node, "Internal error: Can't find array variable reference with null node"); // ??
      Expression *result = node->findVariable(varRef);
      checkParserException(result, "Can't find array variable named " << varRef->varName());
      checkParserException(result->valueType() == varRef->type(),
                           "Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for type " << valueTypeName(varRef->type()));
      wasCreated = false;
      return result;
    }
    PlexilArrayVar const *var = dynamic_cast<PlexilArrayVar const *>(expr);
    if (var) {
      // Variable declaration - construct it
      wasCreated = true;
      return this->create(var, node);
    }
    checkParserException(false,
                         "Expression is neither a variable reference nor an array variable declaration");
    return NULL;
  }

  template <typename T>
  Expression *
  ConcreteExpressionFactory<ArrayVariable<T> >::create(PlexilArrayVar const *var,
                                                       NodeConnector *node) const
  {
    bool garbage = false;
    Expression *sizeexp = new IntegerConstant(var->maxSize());
    Expression *initexp = NULL;
    Assignable *result = new ArrayVariable<T>(node, var->varName(), sizeexp, true);
    PlexilExpr const *initval = var->value(); 
    if (initval) {
      Expression *initexp = createExpression(initval, node, garbage);
      result->setInitializer(initexp, garbage);
    }
    return static_cast<Expression *>(result);
  }

  template <typename T>
  Expression *ConcreteExpressionFactory<ArrayVariable<T> >::allocate(pugi::xml_node const &expr,
                                                                     NodeConnector *node,
                                                                     bool &wasCreated) const
  {
    assertTrue_2(ALWAYS_FAIL, "Not yet implemented");
    return NULL;
  }

  //
  // Array references
  //

  Expression *
  ConcreteExpressionFactory<ArrayReference>::allocate(PlexilExpr const *expr,
                                                      NodeConnector *node,
                                                      bool & wasCreated) const
  {
    PlexilArrayElement const * ary = 
      dynamic_cast<PlexilArrayElement const *>(expr);
    checkParserException(ary != NULL, "Expression is not a PlexilArrayElement");

    bool garbageArray;
    Expression *array = createExpression(ary->array(), node, garbageArray);
    checkParserException(array, "Array expression not found for array reference");
    checkParserException(isArrayType(array->valueType()),
                         "Array expression in array reference is not an array");
    
    bool garbageIndex;
    Expression *index = createExpression(ary->index(), node, garbageIndex);
    checkParserException(index, "Index expression not found for array reference");

    wasCreated = true;
    return new ArrayReference(array, index, garbageArray, garbageIndex);
  }

  // Common subroutine
  static void parseArrayElement(pugi::xml_node const &expr,
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
  ConcreteExpressionFactory<ArrayReference>::allocate(pugi::xml_node const &expr,
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

  Expression *createMutableArrayReference(pugi::xml_node const &expr,
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
  VariableReferenceFactory::allocate(PlexilExpr const *expr,
                                     NodeConnector *node,
                                     bool & wasCreated) const
  {
    PlexilVarRef const *varRef = dynamic_cast<PlexilVarRef const *>(expr);
    checkParserException(varRef, "Expression is not a variable reference");
    // Look it up
    wasCreated = false;
    checkParserException(node,
                         "Variable reference with null node"); // ??
    Expression *result = node->findVariable(varRef);
    checkParserException(result, "Can't find variable named " << varRef->varName());
    // FIXME: add more type checking later
    if (varRef->type() == ARRAY_TYPE) {
      checkParserException(isArrayType(result->valueType()),
                           "Variable " << varRef->varName()
                           << " is type " << valueTypeName(result->valueType())
                           << ", but reference is for array type");
    }
    return result;
  }

  Expression *
  VariableReferenceFactory::allocate(pugi::xml_node const &expr,
                                     NodeConnector *node,
                                     bool & wasCreated) const
  {
    checkParserExceptionWithLocation(testTagSuffix(VAR_TAG, expr),
                                     expr,
                                     "Internal error: not a variable reference");
    assertTrue_1(node); // internal error
    checkNotEmpty(expr);
    ValueType typ = parseValueTypePrefix(expr.name() , strlen(expr.name()) - strlen(VAR_TAG));
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
  ENSURE_EXPRESSION_FACTORY(BooleanArrayConstant);
  ENSURE_EXPRESSION_FACTORY(IntegerArrayConstant);
  ENSURE_EXPRESSION_FACTORY(RealArrayConstant);
  ENSURE_EXPRESSION_FACTORY(StringArrayConstant);

  ENSURE_EXPRESSION_FACTORY(BooleanVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerVariable);
  ENSURE_EXPRESSION_FACTORY(RealVariable);
  ENSURE_EXPRESSION_FACTORY(StringVariable);
  ENSURE_EXPRESSION_FACTORY(BooleanArrayVariable);
  ENSURE_EXPRESSION_FACTORY(IntegerArrayVariable);
  ENSURE_EXPRESSION_FACTORY(RealArrayVariable);
  ENSURE_EXPRESSION_FACTORY(StringArrayVariable);

  // Redundant with above
  // ENSURE_EXPRESSION_FACTORY(ArrayReference);

} // namespace PLEXIL

