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

#include "Expression.hh"
#include "ExpressionFactory.hh"
#include "findDeclarations.hh"
#include "NodeConnector.hh"
#include "parser-utils.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  //
  // Generic variable references
  //

  class VariableReferenceFactory : public ExpressionFactory
  {
  public:
    VariableReferenceFactory(std::string const &name, ValueType typ)
      : ExpressionFactory(name),
        m_type (typ)
    {
    }

    ~VariableReferenceFactory() = default;

    virtual ValueType check(char const *nodeId,
                            pugi::xml_node const expr,
                            ValueType /* desiredType */) const
    {
      checkNotEmpty(expr);
      char const *varName = expr.child_value();
      checkParserExceptionWithLocation(*varName,
                                       expr,
                                       "Node \"" << nodeId
                                       << "\": Empty or malformed " << expr.name() << " element");
  
      ValueType expected = UNKNOWN_TYPE;
      if (testTagPrefix(BOOLEAN_STR, expr))
        expected = BOOLEAN_TYPE;
      else if (testTagPrefix(INTEGER_STR, expr))
        expected = INTEGER_TYPE;
      else if (testTagPrefix(STRING_STR, expr))
        expected = STRING_TYPE;
      else if (testTagPrefix(REAL_STR, expr))
        expected = REAL_TYPE;

      checkParserExceptionWithLocation(expected != UNKNOWN_TYPE,
                                       expr,
                                       "Internal error: unrecognized variable tag \""
                                       << expr.name() << '"');
      pugi::xml_node const decl = findVariableDeclaration(expr, varName);
      checkParserExceptionWithLocation(decl,
                                       expr,
                                       "No " << valueTypeName(expected)
                                       << " variable named \"" << varName << "\" found");
      char const *typeName = decl.child_value(TYPE_TAG);
      checkParserExceptionWithLocation(typeName && *typeName,
                                       decl,
                                       "Internal error: Empty " << TYPE_TAG
                                       << " in declararation of \"" << varName << '"');
      checkParserExceptionWithLocation(expected == parseValueType(typeName),
                                       expr,
                                       "Variable " << varName << " is declared " << typeName
                                       << ", but reference is for a(n) " << expr.name());
      return expected;
    }

    virtual Expression *allocate(pugi::xml_node const expr,
                                 NodeConnector *node,
                                 bool & wasCreated,
                                 ValueType returnType) const
    {
      assertTrue_1(node); // internal error
      checkNotEmpty(expr);
      char const *varName = expr.child_value();
      checkParserExceptionWithLocation(*varName,
                                       expr,
                                       "Empty or malformed " << expr.name() << " element");
      Expression *result = node->findVariable(varName);
      checkParserExceptionWithLocation(result,
                                       expr,
                                       "No variable named " << varName << " accessible in this context");
      bool match = (m_type == result->valueType());
      // *** FIXME? ***
      // Shouldn't be parsing reference to Integer variables as Real
      if (!match
          && m_type == REAL_TYPE
          && result->valueType() == INTEGER_TYPE)
        match = true; // expecting Real, but naming an Integer variable
      checkParserExceptionWithLocation(match,
                                       expr,
                                       "Variable " << varName
                                       << " has invalid type " << valueTypeName(result->valueType())
                                       << " for a " << expr.name());
      wasCreated = false;
      return result;
    }

  private:
    // Default, copy, assign prohibited
    VariableReferenceFactory() = delete;
    VariableReferenceFactory(VariableReferenceFactory const &) = delete;
    VariableReferenceFactory(VariableReferenceFactory &&) = delete;
    VariableReferenceFactory &operator=(VariableReferenceFactory const &) = delete;
    VariableReferenceFactory &operator=(VariableReferenceFactory &&) = delete;

    ValueType const m_type;
  };

  ExpressionFactory *makeVariableReferenceFactory(std::string const &name, ValueType typ)
  {
    return new VariableReferenceFactory(name, typ);
  }

} // namespace PLEXIL
