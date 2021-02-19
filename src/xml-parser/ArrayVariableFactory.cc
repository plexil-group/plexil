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

#include "ArrayVariableFactory.hh"

#include "ArrayVariable.hh"
#include "Constant.hh"
#include "createExpression.hh"
#include "Error.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTDLIB)
#include <cstdlib>
#elif defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

using pugi::xml_node;

namespace PLEXIL
{
  ArrayVariableFactory::ArrayVariableFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  ArrayVariableFactory::~ArrayVariableFactory()
  {
  }

  //
  // First pass: XML checks
  //

  ValueType ArrayVariableFactory::check(char const *nodeId, xml_node expr) const
  {
    // We know the declaration has a name and a valid type.
    // Check for a legal array type.
    xml_node temp = expr.first_child().next_sibling();
    ValueType eltType = parseValueType(temp.child_value());
    checkParserExceptionWithLocation(arrayType(eltType) != UNKNOWN_TYPE,
                                     temp,
                                     "Node \"" << nodeId
                                     << "\": Invalid type name " << temp.child_value()
                                     << " for " << expr.name()
                                     << ' ' << expr.first_child().child_value());

    // Everything after type is optional
    temp = temp.next_sibling();
    if (temp) {
      Integer max_size = -1;
      // Check for optional MaxSize tag
      if (testTag(MAX_SIZE_TAG, temp)) {
        // Check for non-negative integer
        char const *sz = temp.child_value();
        checkParserExceptionWithLocation(*sz,
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": Empty " << temp.name()
                                         << " in " << expr.name() << ' ' << expr.child_value(NAME_TAG));
        checkParserExceptionWithLocation(parseValue<Integer>(sz, max_size) && max_size >= 0,
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": " << temp.name()
                                         << " value " << sz
                                         << " is not a non-negative integer\n in "
                                         << expr.name() << ' ' << expr.child_value(NAME_TAG));

        temp = temp.next_sibling();
      }

      if (temp) {
        // Check initial value, if provided
        checkParserExceptionWithLocation(testTag(INITIALVAL_TAG, temp),
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": " << temp.name() << " element invalid in "
                                         << expr.name() << ' ' << expr.child_value(NAME_TAG));


        // Check that initializer isn't completely bogus
        checkParserExceptionWithLocation(temp.first_child(),
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": Invalid " << INITIALVAL_TAG << " element in "
                                         << expr.name() << ' ' << expr.child_value(NAME_TAG));
        temp = temp.first_child();

        // The schema restricts array variable initializers
        // to variables, array literals, or one scalar literal of compatible type.
        // We may choose to broaden this in the future.
        checkParserExceptionWithLocation(testTag(ARRAY_VAL_TAG, temp)
                                         || testTag(ARRAYVAR_TAG, temp)
                                         || testTagSuffix(VAL_SUFFIX, temp),
                                         temp,
                                         "Node \"" << nodeId
                                         << "\": Invalid " << INITIALVAL_TAG << " contents in "
                                         << expr.name() << ' ' << expr.child_value(NAME_TAG));

        // Check for junk after initializer
        checkParserExceptionWithLocation(!temp.next_sibling(),
                                         temp.next_sibling(),
                                         "Node \"" << nodeId
                                         << "\": " << INITIALVAL_TAG
                                         << " contains extra elements in "
                                         << expr.name() << ' ' << expr.child_value(NAME_TAG))

        // Legal initializers are ArrayValue, ArrayVariable, scalar of element type
        ValueType v = checkExpression(nodeId, temp);
        if (testTag(ARRAY_VAL_TAG, temp) || testTag(ARRAYVAR_TAG, temp)) {
          // ArrayValue, ArrayVariable cases
          checkParserExceptionWithLocation(v == arrayType(eltType),
                                           // || v == UNKNOWN_TYPE, // FIXME - for variables
                                           temp,
                                           "Node \"" << nodeId
                                           << "\": " << valueTypeName(eltType)
                                           << " array variable " << expr.child_value(NAME_TAG)
                                           << " has " << INITIALVAL_TAG
                                           << " of incompatible type " << valueTypeName(v));
        }
        else if (testTagSuffix(VAL_SUFFIX, temp)) {
          // Scalar initializer
          checkParserExceptionWithLocation(areTypesCompatible(eltType, v),
                                           temp,
                                           "Node \"" << nodeId
                                           << "\": " << valueTypeName(eltType)
                                           << " array variable " << expr.child_value(NAME_TAG)
                                           << " has " << INITIALVAL_TAG
                                           << " of incompatible type " << valueTypeName(v));
        }

        // Test that array literal size is not larger than MAX_SIZE
        if (testTag(ARRAY_VAL_TAG, temp) && max_size >= 0) {
          size_t elts = std::distance(temp.children().begin(), temp.children().end());
          checkParserExceptionWithLocation(elts <= (size_t) max_size,
                                           temp,
                                           "Node \"" << nodeId
                                           << "\": " << valueTypeName(eltType)
                                           << " initial value of array variable "
                                           << expr.child_value(NAME_TAG)
                                           << " is larger than the " << MAX_SIZE_TAG
                                           << ", " << max_size);
        }
        // any illegal initializers should have been caught above
      }
    }

    return arrayType(eltType);
  }

  //
  // Second pass: construction
  //

  // N.B. Construction of initializer expression happens later.

  Expression *ArrayVariableFactory::allocate(xml_node const expr,
                                             NodeConnector * /* node */,
                                             bool &wasCreated,
                                             ValueType /* returnType */) const
  {
    xml_node temp = expr.first_child();
    char const *name = temp.child_value();
    temp = temp.next_sibling();
    ValueType typ = parseValueType(temp.child_value());
    Expression *sizeExp = nullptr;
    bool sizeIsGarbage = false;
    temp = temp.next_sibling();
    if (testTag(MAX_SIZE_TAG, temp)) {
      char const *sizeStr = temp.child_value();
      char *end;
      long size = strtol(sizeStr, &end, 10);
      sizeExp = new Constant<int32_t>((int32_t) size);
      sizeIsGarbage = true;
    }

    wasCreated = true;
    switch (typ) {
    case BOOLEAN_TYPE:
      return new BooleanArrayVariable(name,
                                      sizeExp,
                                      sizeIsGarbage);

    case INTEGER_TYPE:
      return new IntegerArrayVariable(name,
                                      sizeExp,
                                      sizeIsGarbage);

    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
    case REAL_TYPE:
      return new RealArrayVariable(name,
                                   sizeExp,
                                   sizeIsGarbage);

    case STRING_TYPE:
      return new StringArrayVariable(name,
                                     sizeExp,
                                     sizeIsGarbage);

    default:
      errorMsg("ArrayVariableFactory::allocate: Internal type error");
      return nullptr;
    }
  }

} // namespace PLEXIL
