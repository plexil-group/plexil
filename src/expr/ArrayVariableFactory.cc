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

#include "ArrayVariableFactory.hh"

#include "ArrayVariable.hh"
#include "Constant.hh"
#include "Error.hh"
#include "expression-schema.hh"
#include "NodeConnector.hh"
#include "ParserException.hh"
#include "parser-utils.hh"
#include "ValueType.hh"

#include "pugixml.hpp"

#include <cstdlib>

namespace PLEXIL
{
  ArrayVariableFactory::ArrayVariableFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  ArrayVariableFactory::~ArrayVariableFactory()
  {
  }


  // *** DELETE ME ***
  Expression *ArrayVariableFactory::allocate(PlexilExpr const * /* expr */,
                                            NodeConnector * /* node */,
                                            bool & /* wasCreated */)
  {
    assertTrue_2(ALWAYS_FAIL, "Nothing should ever call this method!");
    return NULL;
  }

  Expression *ArrayVariableFactory::allocate(pugi::xml_node const &expr,
                                            NodeConnector *node,
                                            bool &wasCreated)
  {
    checkHasChildElement(expr);
    pugi::xml_node nameElt = expr.first_child();
    checkTag(NAME_TAG, nameElt);
    checkNotEmpty(nameElt);
    std::string const name(nameElt.first_child().value());

    pugi::xml_node typeElt = nameElt.next_sibling();
    checkParserExceptionWithLocation(typeElt,
                                     expr,
                                     "createExpression: DeclareVariable missing Type element");
    checkTag(TYPE_TAG, typeElt);
    ValueType typ = parseValueType(typeElt.first_child().value());
    checkParserExceptionWithLocation(isScalarType(typ),
                                     typeElt,
                                     "createExpression: Type " << typeElt.first_child().value()
                                     << " is invalid for DeclareVariable");
    pugi::xml_node sizeElt = typeElt.next_sibling();
    checkParserExceptionWithLocation(typeElt,
                                     expr,
                                     "createExpression: DeclareArray missing MaxSize element");
    checkTag(MAX_SIZE_TAG, sizeElt);
    checkNotEmpty(sizeElt);
    char const *sizeStr = sizeElt.first_child().value();
    // Syntactic check
    checkParserExceptionWithLocation(isInteger(sizeStr),
                                     sizeElt.first_child(),
                                     "createExpression: MaxSize value \"" << sizeStr << "\" is not an integer");

    char *end;
    long size = strtol(sizeStr, &end, 10);
    checkParserExceptionWithLocation(*end,
                                     sizeElt.first_child(),
                                     "createExpression: MaxSize value \"" << sizeStr << "\" is not an integer");
    checkParserExceptionWithLocation(size < 0 || size > INT32_MAX,
                                     sizeElt.first_child(),
                                     "createExpression: MaxSize value " << sizeStr << " is not a non-negative integer");
    Expression *sizeExp = new Constant<int32_t>((int32_t) size);

    pugi::xml_node initializerElt = sizeElt.next_sibling();
    Expression *initializer = NULL;
    bool initializerIsGarbage = false;
    if (initializerElt) {
      checkHasChildElement(initializerElt);
      initializer = createExpression(initializerElt.first_child(),
                                     node,
                                     initializerIsGarbage);
      checkParserExceptionWithLocation(initializer->valueType() == arrayType(typ),
                                       initializerElt.first_child(),
                                       "createExpression: Array variable initializer type differs from variable's");
      // If constant (and it should be), check size
      if (initializer->isConstant()) {
        Array const *initval = NULL;
        if (initializer->getValuePointer(initval))
          checkParserExceptionWithLocation(initval->size() <= size,
                                           initializerElt.first_child(),
                                           "createExpression: Array variable initial value is larger than array");
      }
    }

    wasCreated = true;
    switch (typ) {
    case BOOLEAN_TYPE:
      return new BooleanArrayVariable(node,
                                      name,
                                      sizeExp,
                                      initializer,
                                      true,
                                      initializerIsGarbage);

    case INTEGER_TYPE:
      return new IntegerArrayVariable(node,
                                      name,
                                      sizeExp,
                                      initializer,
                                      true,
                                      initializerIsGarbage);

    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
    case REAL_TYPE:
      return new RealArrayVariable(node,
                                   name,
                                   sizeExp,
                                   initializer,
                                   true,
                                   initializerIsGarbage);

    case STRING_TYPE:
      return new StringArrayVariable(node,
                                     name,
                                     sizeExp,
                                     initializer,
                                     true,
                                     initializerIsGarbage);

    default:
      assertTrue_2(ALWAYS_FAIL,
                   "ArrayVariableFactory::allocate: Internal type error");
      return NULL;
    }
  }

} // namespace PLEXIL
