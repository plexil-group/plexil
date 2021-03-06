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

#ifndef PLEXIL_VALUE_TYPE_HH
#define PLEXIL_VALUE_TYPE_HH

#include "ArrayFwd.hh"
#include "CommandHandle.hh"
#include "NodeConstants.hh"
#include "ParserException.hh"

#include <iosfwd>

#if defined(HAVE_CSTDDEF)
#include <cstddef> // size_t
#elif defined(HAVE_STDDEF_H)
#include <stddef.h> // size_t
#endif

namespace PLEXIL
{

  //
  // Type aliases
  //

  typedef bool        Boolean;
  typedef int32_t     Integer;
  typedef double      Real;
  typedef std::string String;

  // Subject to change in the future.
  typedef double      Duration;
  typedef double      Time;

  // Array types declared in ArrayFwd.hh, defined in ArrayImpl.hh:
  // BooleanArray
  // IntegerArray
  // RealArray
  // StringArray

  //
  // PLEXIL expression data types
  //

  enum ValueType : uint8_t {
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

      STATE_TYPE = 32, // Lookup or Command descriptor, mostly for external use

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
  constexpr char const BOOLEAN_STR[] = "Boolean";
  constexpr char const INTEGER_STR[] = "Integer";
  constexpr char const REAL_STR[] = "Real";
  constexpr char const DATE_STR[] = "Date";
  constexpr char const DURATION_STR[] = "Duration";
  constexpr char const STRING_STR[] = "String";
  constexpr char const ARRAY_STR[] = "Array";
  constexpr char const BOOLEAN_ARRAY_STR[] = "BooleanArray";
  constexpr char const INTEGER_ARRAY_STR[] = "IntegerArray";
  constexpr char const REAL_ARRAY_STR[] = "RealArray";
  constexpr char const STRING_ARRAY_STR[] = "StringArray";
  constexpr char const STATE_STR[] = "State";
  constexpr char const NODE_STATE_STR[] = "NodeState";
  constexpr char const NODE_OUTCOME_STR[] = "NodeOutcome";
  constexpr char const NODE_FAILURE_STR[] = "NodeFailure";
  constexpr char const NODE_COMMAND_HANDLE_STR[] = "NodeCommandHandle";

  constexpr char const VAL_SUFFIX[] = "Value";
  constexpr char const VAR_SUFFIX[] = "Variable";

  constexpr char const UNKNOWN_STR[] = "UNKNOWN";

  // Utility functions
  const std::string &valueTypeName(ValueType ty);

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

  // Returns true if dest type can receive src type, false otherwise.
  extern bool areTypesCompatible(ValueType dest, ValueType src);

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
  bool parseValue(char const *s, T &result);

  template <typename T>
  bool parseValue(std::string const &s, T &result)
  {
    return parseValue<T>(s.c_str(), result);
  }

  /**
   * @brief Write a binary version of the object to the given buffer.
   * @param o The object.
   * @param b Pointer to the insertion point in the buffer.
   * @return Pointer to first byte after the object; nullptr if failed.
   */

  template <typename T>
  char *serialize(T const &o, char *b);

  /**
   * @brief Read a binary representation from the buffer and store it to the result object.
   * @param o The result object.
   * @param b Pointer to the representation in the buffer.
   * @return Pointer to first byte after the object; nullptr if failed.
   */

  template <typename T>
  char const *deserialize(T &o, char const *b);

  /**
   * @brief Calculate the size of the serial representation of the object.
   * @param o The object.
   * @return Number of bytes; 0 if the object is not serializable.
   */

  template <typename T>
  size_t serialSize(T const &o);
   
}

#endif // PLEXIL_VALUE_TYPE_HH
