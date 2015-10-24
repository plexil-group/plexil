/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_VALUE_TYPE_HH
#define PLEXIL_VALUE_TYPE_HH

#include "ParserException.hh"

#include "ArrayFwd.hh"

#include <iosfwd>
#include <vector>

namespace PLEXIL
{
  //
  // PLEXIL expression data types
  //

  enum ValueType {
      UNKNOWN_TYPE = 0,
      // User scalar types
      BOOLEAN_TYPE,
      INTEGER_TYPE,
      REAL_TYPE,
      STRING_TYPE,
      DATE_TYPE,     // TODO: what format?
      DURATION_TYPE, //  ""    ""    ""
      // more to come
      SCALAR_TYPE_MAX,

      // User array types
      ARRAY_TYPE = 16, // Not a valid type, but an offset from scalar types
      BOOLEAN_ARRAY_TYPE,
      INTEGER_ARRAY_TYPE,
      REAL_ARRAY_TYPE,
      STRING_ARRAY_TYPE,
      // more to come?

      ARRAY_TYPE_MAX,

      // Internal types
      INTERNAL_TYPE_OFFSET = 48, // Not a valid type
      NODE_STATE_TYPE,
      OUTCOME_TYPE,
      FAILURE_TYPE,
      COMMAND_HANDLE_TYPE,
      // more?
      TYPE_MAX
    };

  // Type name string constants
  extern char const *BOOLEAN_STR;
  extern char const *INTEGER_STR;
  extern char const *REAL_STR;
  extern char const *DATE_STR;
  extern char const *DURATION_STR;
  extern char const *STRING_STR;
  extern char const *ARRAY_STR;
  extern char const *BOOLEAN_ARRAY_STR;
  extern char const *INTEGER_ARRAY_STR;
  extern char const *REAL_ARRAY_STR;
  extern char const *STRING_ARRAY_STR;
  extern char const *NODE_STATE_STR;
  extern char const *NODE_OUTCOME_STR;
  extern char const *NODE_FAILURE_STR;
  extern char const *NODE_COMMAND_HANDLE_STR;

  extern char const *VAL_SUFFIX;
  extern char const *VAR_SUFFIX;

  extern char const *UNKNOWN_STR;

  // Utility functions
  const std::string &valueTypeName(ValueType ty);
  // still used by XML parser for array initialization
  const std::string &typeNameAsValue(ValueType ty);

  extern bool isUserType(ValueType ty);
  extern bool isInternalType(ValueType ty);
  extern bool isNumericType(ValueType ty);

  extern bool isScalarType(ValueType ty);
  extern bool isArrayType(ValueType ty);

  extern ValueType arrayElementType(ValueType ty);
  extern ValueType arrayType(ValueType elTy);

  // Find the type whose name exactly matches the given string.
  extern ValueType parseValueType(char const *typeStr);
  extern ValueType parseValueType(const std::string &typeStr);

  /**
   * @brief Find the longest type name contained as a prefix in the given string.
   * @param str The string to scan.
   * @param result Reference to the place to put the ValueType.
   * @return Length of the scanned prefix. If 0, no valid type prefix was found.
   */
  extern size_t scanValueTypePrefix(char const *str, ValueType &result);

  // Returns true if dest type can receive src type, false otherwise.
  extern bool areTypesCompatible(ValueType dest, ValueType src);

  template <typename T>
  void printValue(ArrayImpl<T> const &val, std::ostream &s);

  template <typename T>
  void printValue(T const &val, std::ostream &s);

  /**
   * @brief Parse one value from the incoming stream.
   * @param s Input string.
   * @param result Reference to the place to store the result.
   * @return True if known, false if unknown.
   * @note 
   * @note If false, the result variable will not be modified.
   */
  template <typename T>
  bool parseValue(char const *s, T &result)
    throw (ParserException);

  template <typename T>
  bool parseValue(std::string const &s, T &result)
    throw (ParserException)
  {
    return parseValue<T>(s.c_str(), result);
  }

}

#endif // PLEXIL_VALUE_TYPE_HH
