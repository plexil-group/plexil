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

#include "ArrayVariableReferenceFactory.hh"

#include "Error.hh"
#include "Expression.hh"
#include "ExpressionFactory.hh"
#include "findDeclarations.hh"
#include "NodeConnector.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  class ArrayVariableReferenceFactory : public ExpressionFactory
  {
  public:
    ArrayVariableReferenceFactory(const std::string& name)
      : ExpressionFactory(name)
    {
    }

    virtual ~ArrayVariableReferenceFactory() = default;

    ValueType check(char const *nodeId, pugi::xml_node expr, ValueType desiredType) const
    {
      checkNotEmpty(expr);
      char const *varName = expr.child_value();
      checkParserExceptionWithLocation(*varName,
                                       expr,
                                       "Node \"" << nodeId
                                       << "\": Empty or malformed " << expr.name() << " element");

      // Get array type from context
      pugi::xml_node const decl = findArrayDeclaration(expr, varName);
      checkParserExceptionWithLocation(decl,
                                       expr,
                                       "No array variable named \"" << varName << "\" found");
      char const *eltTypeName = decl.child_value(TYPE_TAG);
      checkParserExceptionWithLocation(eltTypeName && *eltTypeName,
                                       decl,
                                       "Internal error: Empty " << TYPE_TAG
                                       << " in declaration of array variable \"" << varName << '"');
      ValueType aryEltType = parseValueType(eltTypeName);
      checkParserExceptionWithLocation(aryEltType != UNKNOWN_TYPE,
                                       decl,
                                       "Internal error: Invalid " << TYPE_TAG
                                       << " value in declaration of \"" << varName << '"');
      ValueType aryType = arrayType(aryEltType);
      checkParserExceptionWithLocation(aryType != UNKNOWN_TYPE,
                                       decl,
                                       "Internal error: Invalid array element type " << eltTypeName
                                       << " in declaration of \"" << varName << '"');
      return aryType;
    }

    Expression *allocate(pugi::xml_node const expr,
                         NodeConnector *node,
                         bool & wasCreated,
                         ValueType /* desiredType */) const
    {
      assertTrue_1(node); // internal error
      char const *varName = expr.child_value();
      Expression *result = node->findVariable(varName);
      checkParserExceptionWithLocation(result,
                                       expr,
                                       "No variable named " << varName << " accessible in this context");
      checkParserExceptionWithLocation(isArrayType(result->valueType()),
                                       expr,
                                       "Variable " << varName << " is not an array variable");
      wasCreated = false;
      return result;
    }

  private:
    // Default, copy, assign all prohibited
    ArrayVariableReferenceFactory() = delete;
    ArrayVariableReferenceFactory(const ArrayVariableReferenceFactory &) = delete;
    ArrayVariableReferenceFactory(ArrayVariableReferenceFactory &&) = delete;
    ArrayVariableReferenceFactory &operator=(const ArrayVariableReferenceFactory &) = delete;
    ArrayVariableReferenceFactory &operator=(ArrayVariableReferenceFactory &&) = delete;
  };

  ExpressionFactory *
  makeArrayVariableReferenceFactory(const std::string& name)
  {
    return new ArrayVariableReferenceFactory(name);
  }

}
