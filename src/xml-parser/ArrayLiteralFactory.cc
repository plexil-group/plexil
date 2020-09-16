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

#include "ArrayLiteralFactory.hh"

#include "Constant.hh"
#include "parser-utils.hh"
#include "PlexilSchema.hh"

#include "pugixml.hpp"

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL
{
  ArrayLiteralFactory::ArrayLiteralFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  ArrayLiteralFactory::~ArrayLiteralFactory()
  {
  }

  //
  // First pass: XML checks
  //

  // General case
  template <typename T>
  void checkArrayLiteral(char const *eltTypeName, pugi::xml_node const expr)
  {
    pugi::xml_node thisElement = expr.first_child();
    while (thisElement) {
      checkTagSuffix(VAL_SUFFIX, thisElement);
      // Check type
      const char* thisElementTag = thisElement.name();
      checkParserExceptionWithLocation(!strncmp(thisElementTag, 
                                                eltTypeName, 
                                                strlen(thisElementTag) - strlen(VAL_SUFFIX)),
                                       thisElement,
                                       "Type mismatch: element " << thisElementTag
                                       << " in array value of type " << eltTypeName);

      // Get array element value
      checkNotEmpty(thisElement);
      T temp;
      parseValue(thisElement.child_value(), temp); // for effect; will throw if format error
      thisElement = thisElement.next_sibling();
    }
  }

  // Specialization for String
  template <>
  void checkArrayLiteral<String>(char const *eltTypeName, pugi::xml_node const expr)
  {
    // gather elements
    pugi::xml_node thisElement = expr.first_child();
    while (thisElement) {
      // Check type
      checkParserExceptionWithLocation(testTag(STRING_VAL_TAG, thisElement),
                                       thisElement,
                                       "Array literal type mismatch: element " << thisElement.name()
                                       << " in array value of type \"" << eltTypeName);
      thisElement = thisElement.next_sibling();
    }
  }

  ValueType ArrayLiteralFactory::check(char const *nodeId, pugi::xml_node expr) const
  {
    // confirm that we have an array value
    checkTag(ARRAY_VAL_TAG, expr);

    // confirm that we have an element type
    checkAttr(TYPE_TAG, expr);
    const char* valueType = expr.attribute(TYPE_TAG).value();
    ValueType valtyp = parseValueType(valueType);
    checkParserExceptionWithLocation(valtyp != UNKNOWN_TYPE,
                                     expr, // should be attribute
                                     "Node \"" << nodeId
                                     << "\": Unknown array element Type value \""
                                     << valueType << "\"");

    // Check contents
    switch (valtyp) {
    case BOOLEAN_TYPE:
      checkArrayLiteral<Boolean>(valueType, expr);
      break;

    case INTEGER_TYPE:
      checkArrayLiteral<Integer>(valueType, expr);
      break;

    case REAL_TYPE:
      checkArrayLiteral<Real>(valueType, expr);
      break;

    case STRING_TYPE:
      checkArrayLiteral<String>(valueType, expr);
      break;

    default:
      reportParserExceptionWithLocation(expr, // should be attribute
                                        "Node \"" << nodeId
                                        << "\": Invalid or unimplemented array element Type value \""
                                        << valueType << "\"");
      break;
    }
    return arrayType(valtyp);
  }

  //
  // Second pass: construction
  //

  template <typename T>
  Expression *createArrayLiteral(char const *eltTypeName, pugi::xml_node const expr)
  {
    // gather elements
    std::vector<T> values;
    values.reserve(std::distance(expr.begin(), expr.end()));

    pugi::xml_node thisElement = expr.first_child();
    size_t i = 0;
    std::vector<size_t> unknowns;
    while (thisElement) {
      const char* thisElementValue = thisElement.child_value();
      T temp;
      if (parseValue<T>(thisElementValue, temp)) // will throw if format error
        values.push_back(temp);
      else {
        unknowns.push_back(i);
        values.push_back(T()); // push a placeholder for unknown
      }
      thisElement = thisElement.next_sibling();
      ++i;
    }

    // Handle unknowns here
    ArrayImpl<T> initVals(values);
    for (std::vector<size_t>::const_iterator it = unknowns.begin();
         it != unknowns.end();
         ++it)
      initVals.setElementUnknown(*it);

    return new Constant<ArrayImpl<T> >(initVals);
  }

  template <>
  Expression *createArrayLiteral<String>(char const *eltTypeName, pugi::xml_node const expr)
  {
    // gather elements
    std::vector<std::string> values;
    values.reserve(std::distance(expr.begin(), expr.end()));

    pugi::xml_node thisElement = expr.first_child();
    size_t i = 0;
    while (thisElement) {
      values.push_back(std::string(thisElement.child_value()));
      thisElement = thisElement.next_sibling();
      ++i;
    }
    return new Constant<ArrayImpl<String> >(ArrayImpl<String>(values));
  }

  Expression *ArrayLiteralFactory::allocate(pugi::xml_node const expr,
                                            NodeConnector * /* node */,
                                            bool &wasCreated,
                                            ValueType /* returnType */) const
  {
    const char* valueType = expr.attribute(TYPE_TAG).value();
    ValueType valtyp = parseValueType(valueType);
    wasCreated = true;

    switch (valtyp) {
    case BOOLEAN_TYPE:
      return createArrayLiteral<Boolean>(valueType, expr);

    case INTEGER_TYPE:
      return createArrayLiteral<Integer>(valueType, expr);

    case REAL_TYPE:
      return createArrayLiteral<Real>(valueType, expr);

    case STRING_TYPE:
      return createArrayLiteral<String>(valueType, expr);

    default:
      reportParserExceptionWithLocation(expr, // should be attribute
                                        "Invalid or unimplemented array element Type value \""
                                        << valueType << "\"");
      return NULL;
    }
  }

  // Explicit instantiations
  // (Are these redundant with the calls above?)
  template void checkArrayLiteral<Boolean>(char const *eltTypeName, pugi::xml_node const expr);
  template void checkArrayLiteral<Integer>(char const *eltTypeName, pugi::xml_node const expr);
  template void checkArrayLiteral<Real>(char const *eltTypeName, pugi::xml_node const expr);

  template Expression *createArrayLiteral<Boolean>(char const *eltTypeName, pugi::xml_node const expr);
  template Expression *createArrayLiteral<Integer>(char const *eltTypeName, pugi::xml_node const expr);
  template Expression *createArrayLiteral<Real>(char const *eltTypeName, pugi::xml_node const expr);

} // namespace PLEXIL
