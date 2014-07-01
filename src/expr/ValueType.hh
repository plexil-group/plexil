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

#ifndef PLEXIL_VALUE_TYPE_HH
#define PLEXIL_VALUE_TYPE_HH

#include "ParserException.hh"

#include <iosfwd>
#include <string>
#include <vector>

namespace PLEXIL
{
  // Forward references
  template <typename T> class ArrayImpl;

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
  // Stolen from PlexilPlan.hh
  extern std::string const BOOL_STR;
  extern std::string const INTEGER_STR;
  extern std::string const REAL_STR;
  extern std::string const DATE_STR;
  extern std::string const DURATION_STR;
  extern std::string const STRING_STR;
  extern std::string const ARRAY_STR;
  extern std::string const BOOLEAN_ARRAY_STR;
  extern std::string const INTEGER_ARRAY_STR;
  extern std::string const REAL_ARRAY_STR;
  extern std::string const STRING_ARRAY_STR;
  extern std::string const NODE_STATE_STR;
  extern std::string const NODE_OUTCOME_STR;
  extern std::string const NODE_FAILURE_STR;
  extern std::string const NODE_COMMAND_HANDLE_STR;

  extern std::string const UNKNOWN_STR;

  extern std::string const VARIABLE_STR;
  extern std::string const VALUE_STR;

  // Utility functions
  const std::string &valueTypeName(ValueType ty);
  const std::string &typeNameAsValue(ValueType ty);
  const std::string &typeNameAsVariable(ValueType ty);

  bool isUserType(ValueType ty);
  bool isInternalType(ValueType ty);
  bool isNumericType(ValueType ty);

  bool isScalarType(ValueType ty);
  bool isArrayType(ValueType ty);
  ValueType arrayElementType(ValueType ty);
  ValueType arrayType(ValueType elTy);

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
  bool parseValue(std::string const &s, T &result)
    throw (ParserException);

}

#endif // PLEXIL_VALUE_TYPE_HH
