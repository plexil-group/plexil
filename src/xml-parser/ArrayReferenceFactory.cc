/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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
#include "ArrayReferenceFactory.hh"
#include "createExpression.hh"
#include "findDeclarations.hh"
#include "NodeConnector.hh"
#include "parser-utils.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{

  // Common subroutine
  static void parseArrayElement(xml_node const expr,
                                NodeConnector *node,
                                Expression *&arrayExpr,
                                Expression *&indexExpr,
                                bool &arrayCreated,
                                bool &indexCreated)
  {
    // Syntax checks
    xml_node arrayXml = expr.first_child();
    xml_node indexXml = arrayXml.next_sibling();
    checkTag(INDEX_TAG, indexXml);

    // Checks on array
    if (testTag(NAME_TAG, arrayXml)) {
      const char *arrayName = arrayXml.child_value();
      arrayExpr = node->findVariable(arrayName);
      checkParserExceptionWithLocation(arrayExpr,
                                       arrayXml,
                                       "No array variable named \""
                                       << arrayName << "\" accessible from node "
                                       << node->getNodeId());
      checkParserExceptionWithLocation(isArrayType(arrayExpr->valueType()),
                                       arrayXml,
                                       "Variable \"" << arrayName
                                       << "\" is not an array variable");
    }
    else {
      arrayExpr = createExpression(arrayXml, node, arrayCreated);
      // Have to allow for UNKNOWN (e.g. lookups)
      ValueType exprType = arrayExpr->valueType();
      checkParserExceptionWithLocation(isArrayType(exprType) || exprType == UNKNOWN_TYPE,
                                       arrayXml,
                                       "Array expression is not an array");
    }

    // Checks on index
    indexExpr = createExpression(indexXml.first_child(), node, indexCreated);
    assertTrue_1(indexExpr);
    ValueType indexType = indexExpr->valueType();
    checkParserExceptionWithLocation(indexType == INTEGER_TYPE || indexType == UNKNOWN_TYPE,
                                     indexXml,
                                     "Array index expression is not numeric");
  }

  template <>
  ValueType factoryCheck<ArrayReference>(char const *nodeId,
                                         pugi::xml_node const expr,
                                         ValueType desiredType)
  {
    // Syntax checks
    checkHasChildElement(expr);
    xml_node const arrayXml = expr.first_child();
    checkParserExceptionWithLocation(arrayXml && arrayXml.type() == pugi::node_element,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Ill-formed ArrayElement expression");

    ValueType result = UNKNOWN_TYPE;
    if (testTag(ARRAYVAR_TAG, arrayXml) || testTag(NAME_TAG, arrayXml)) {
      // The text child of either tag has the array name
      checkNotEmpty(arrayXml);
      // Check that array is declared
      pugi::xml_node const decl = findArrayDeclaration(expr, arrayXml.child_value());
      checkParserExceptionWithLocation(decl,
                                       arrayXml,
                                       "No array named \"" << arrayXml.child_value()
                                       << '"');
      const char *typeName = decl.child_value(TYPE_TAG);
      // If this check fails, we missed something up the tree
      checkParserExceptionWithLocation(typeName && *typeName,
                                       decl,
                                       "Internal error: Ill-formed array variable declaration");
      result = parseValueType(typeName);
      // If this check fails, we missed something up the tree
      checkParserExceptionWithLocation(result != UNKNOWN_TYPE,
                                       decl,
                                       "Internal error: unrecognized array element type");
    }
    else {
      // Array-valued expression
      checkExpression(nodeId, arrayXml);
      // TODO: get array type if possible
    }

    xml_node indexXml = arrayXml.next_sibling();
    checkParserExceptionWithLocation(indexXml && testTag(INDEX_TAG, indexXml),
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": ArrayElement has no Index element");
    checkHasChildElement(indexXml);
    indexXml = indexXml.first_child();
    checkParserExceptionWithLocation(indexXml.type() == pugi::node_element,
                                     indexXml,
                                     "Node \"" << nodeId
                                     << "\": ArrayElement Index expression is not an element");
    // Check index
    // TODO: check index is an Integer
    checkExpression(nodeId, indexXml);

    // Return element type of array
    return result;
  }

  template<>
  Expression *factoryAllocate<ArrayReference>(pugi::xml_node const expr,
                                              NodeConnector *node,
                                              bool & wasCreated,
                                              ValueType returnType)
  {
    Expression *arrayExpr = nullptr;
    Expression *indexExpr = nullptr;
    bool arrayCreated = false;
    bool indexCreated = false;
    
    parseArrayElement(expr, node, arrayExpr, indexExpr, arrayCreated, indexCreated);

    wasCreated = true;
    return new ArrayReference(arrayExpr, indexExpr, arrayCreated, indexCreated);
  }
  
  ENSURE_EXPRESSION_FACTORY(ArrayReference);

  // Special case for ArrayElement as assignment target or InOut alias
  Expression *createMutableArrayReference(xml_node const expr,
                                          NodeConnector *node,
                                          bool & wasCreated)
  {
    Expression *arrayExpr = nullptr;
    Expression *indexExpr = nullptr;
    bool arrayCreated = false;
    bool indexCreated = false;
    
    parseArrayElement(expr, node, arrayExpr, indexExpr, arrayCreated, indexCreated);
    try {
      checkParserExceptionWithLocation(arrayExpr->isAssignable(),
                                       expr,
                                       "Can't create a writeable array reference on a read-only array expression");
    }
    catch (ParserException & e) {
      if (arrayCreated)
        delete arrayExpr;
      if (indexCreated)
        delete indexExpr;
    }

    wasCreated = true;
    return new MutableArrayReference(arrayExpr, indexExpr, arrayCreated, indexCreated);
  }

}
