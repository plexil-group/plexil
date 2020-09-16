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

#include "UserVariableFactory.hh"

#include "createExpression.hh"
#include "Error.hh"
#include "parser-utils.hh"
#include "ParserException.hh"
#include "PlexilSchema.hh"
#include "UserVariable.hh"

#include "pugixml.hpp"

using pugi::xml_node;

namespace PLEXIL
{
  UserVariableFactory::UserVariableFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  UserVariableFactory::~UserVariableFactory()
  {
  }

  ValueType UserVariableFactory::check(char const *nodeId, pugi::xml_node const expr) const
  {
    // We know the declaration has a name and a valid type;
    // see checkVariableDeclaration() in parseNode.cc
    xml_node temp = expr.first_child().next_sibling(); // must be Type element
    ValueType typ = parseValueType(temp.child_value());

    temp = temp.next_sibling();
    if (temp) {
      // Check that initializer isn't completely bogus
      checkParserExceptionWithLocation(testTag(INITIALVAL_TAG, temp),
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": Invalid " << temp.name() << " element in "
                                       << expr.name() << ' ' << expr.child_value(NAME_TAG));
      checkParserExceptionWithLocation(temp.first_child(),
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": Invalid " << INITIALVAL_TAG << " contents in "
                                       << expr.name() << ' ' << expr.child_value(NAME_TAG));
      temp = temp.first_child();

      // *** N.B. ***
      // The schema used to restrict initializers to literals.
      // Now restricts to literals and variables
      // we may choose to broaden this in the future.
      // Comment out this check if so.
    
      checkParserExceptionWithLocation(testTagSuffix(VAL_SUFFIX, temp)
                                       || testTagSuffix(VAR_SUFFIX, temp),
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": Invalid " << INITIALVAL_TAG << " contents in "
                                       << expr.name() << ' ' << expr.child_value(NAME_TAG));

      // Check initializer type if possible
      ValueType v = checkExpression(nodeId, temp);
      checkParserExceptionWithLocation(v == typ || v == UNKNOWN_TYPE,
                                       temp,
                                       "Node \"" << nodeId
                                       << "\": " << valueTypeName(typ)
                                       << " variable " << expr.child_value(NAME_TAG)
                                       << " has " << INITIALVAL_TAG
                                       << " of incompatible type " << valueTypeName(v));
    }

    return typ;
  }

  // N.B. Construction of initializer expression happens later.

  Expression *UserVariableFactory::allocate(xml_node const expr,
                                            NodeConnector * /* node */,
                                            bool &wasCreated,
                                            ValueType /* returnType */) const
  {
    xml_node temp = expr.first_child();
    char const *name = temp.child_value();
    temp = temp.next_sibling();
    ValueType typ = parseValueType(temp.child_value());

    wasCreated = true;
    switch (typ) {
    case BOOLEAN_TYPE:
      return new BooleanVariable(name);

    case INTEGER_TYPE:
      return new IntegerVariable(name);

    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
    case REAL_TYPE:
      return new RealVariable(name);

    case STRING_TYPE:
      return new StringVariable(name);

    default:
      errorMsg("UserVariableFactory::allocate: Internal type error");
      return NULL;
    }
  }

} // namespace PLEXIL
