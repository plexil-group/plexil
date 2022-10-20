// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_VALUE_TYPE_HH
#define PLEXIL_VALUE_TYPE_HH

#include "ArrayFwd.hh" 
#include "CommandHandle.hh"
#include "NodeConstants.hh"

#include <cstddef> // size_t
#include <iosfwd>  // std::ostream

namespace PLEXIL
{

  //! \defgroup Values Value representations

  //
  // Type aliases
  //

  //! \typedef Boolean
  //! \brief The PLEXIL Boolean type.
  //! \ingroup Values
  typedef bool        Boolean;

  //! \typedef Integer
  //! \brief The PLEXIL Integer type.
  //! \ingroup Values
  typedef int32_t     Integer;

  //! \typedef Real
  //! \brief The PLEXIL Real type.
  //! \ingroup Values
  typedef double      Real;

  //! \typedef String
  //! \brief The PLEXIL String type.
  //! \ingroup Values
  typedef std::string String;

  // Subject to change in the future.

  //! \typedef Duration
  //! \brief The PLEXIL Duration type is represented as a Real.
  //! \note The implementation of this type may change in future releases.
  //! \ingroup Values
  typedef double      Duration;

  //! \typedef Time
  //! \brief The PLEXIL Time type is represented as a Real.
  //! \note The implementation of this type may change in future releases.
  //! \ingroup Values
  typedef double      Time;

  // Array types declared in ArrayFwd.hh, defined in ArrayImpl.hh:
  // BooleanArray
  // IntegerArray
  // RealArray
  // StringArray

  //
  // PLEXIL expression data types
  //

  //! \enum ValueType
  //! \brief The PLEXIL data type of a value.
  //! \ingroup Values
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

  //! \brief Get the printed name of a ValueType value.
  //! \param ty The ValueType.
  //! \return Const reference to the name string.
  //! \ingroup Values
  const std::string &valueTypeName(ValueType ty);

  //! \brief Is the given type a user type?
  //! \param ty The ValueType.
  //! \return True if it is a user type, false otherwise.
  //! \ingroup Values
  extern bool isUserType(ValueType ty);

  //! \brief Is the given type an internal type?
  //! \param ty The ValueType.
  //! \return True if it is an internal type, false otherwise.
  //! \ingroup Values
  extern bool isInternalType(ValueType ty);

  //! \brief Is this type a numeric type?
  //! \param ty The ValueType.
  //! \return True if it is a numeric type, false otherwise.
  //! \ingroup Values
  extern bool isNumericType(ValueType ty);

  //! \brief Is this type a scalar (non-array) type?
  //! \param ty The ValueType.
  //! \return True if the type is a scalar type, false if it is an array type.
  //! \ingroup Values
  extern bool isScalarType(ValueType ty);

  //! \brief Is this type an array type?
  //! \param ty The ValueType.
  //! \return True if the type is an array type, false otherwise.
  //! \ingroup Values
  extern bool isArrayType(ValueType ty);

  //! \brief Get the type of an element of an array having the given type.
  //! \param ty The array ValueType.
  //! \return The element type, or UNKNOWN_TYPE if the parameter is not an array type.
  //! \ingroup Values
  extern ValueType arrayElementType(ValueType ty);

  //! \brief Get the type of an array whose elements are of the given type.
  //! \param elTy The element ValueType.
  //! \return The array type, or UNKNOWN_TYPE if the parameter has no associated array type.
  //! \ingroup Values
  extern ValueType arrayType(ValueType elTy);

  //! \brief Find the type whose name exactly matches the given string.
  //! \param typeStr Pointer to a const chararacter string.
  //! \return The value type, or UNKNOWN_TYPE if the string cannot be parsed as a type name.
  //! \ingroup Values
  extern ValueType parseValueType(char const *typeStr);

  //! \brief Find the type whose name exactly matches the given string.
  //! \param typeStr Const reference to a string.
  //! \return The value type, or UNKNOWN_TYPE if the string cannot be parsed as a type name.
  //! \ingroup Values
  extern ValueType parseValueType(const std::string &typeStr);

  //! \brief Returns true if dest type can receive src type, false otherwise.
  //! \param dest The value type of the destination.
  //! \param src The value type of the source.
  //! \return True if dest can receive src's type, false otherwise.
  //! \ingroup Values
  extern bool areTypesCompatible(ValueType dest, ValueType src);

  //! \brief Function template to print a value of an arbitrary type on a stream.
  //! \param val Const reference to the value.
  //! \param s Reference to the stream.
  //! \ingroup Values
  template <typename T>
  void printValue(T const &val, std::ostream &s);

  //! \brief Function template to parse one value from a character string.
  //! \param str Input string, as a const pointer to char.
  //! \param result Reference to the result variable.
  //! \return True if known, false if unknown or unparseable as the desired type.
  //! \note If the return value is false, the result variable was not modified.
  //! \ingroup Values
  template <typename T>
  bool parseValue(char const *s, T &result);

  //! \brief Function template to parse one value from a std::string.
  //! \param str Const reference to the string
  //! \param result Reference to the result variable.
  //! \return True if known, false if unknown or unparseable as the desired type.
  //! \note If the return value is false, the result variable was not modified.
  //! \ingroup Values
  template <typename T>
  bool parseValue(std::string const &s, T &result)
  {
    return parseValue<T>(s.c_str(), result);
  }

  //! \brief Function template to write a binary representation of the object to the given buffer.
  //! \param o The object.
  //! \param b Pointer to the insertion point in the buffer.
  //! \return Pointer to first byte after the object; NULL if failed.
  //! \ingroup Values
  template <typename T>
  char *serialize(T const &o, char *b);

  //! \brief Function template to read a binary representation from the buffer and store it to the result object.
  //! \param o The result object.
  //! \param b Pointer to the representation in the buffer.
  //! \return Pointer to first byte after the object; NULL if failed.
  //! \ingroup Values
  template <typename T>
  char const *deserialize(T &o, char const *b);

  //! \brief Function template to calculate the size of the serial representation of the object.
  //! \param o The object.
  //! \return Number of bytes; 0 if the object is not serializable.
  //! \ingroup Values
  template <typename T>
  size_t serialSize(T const &o);
   
}

#endif // PLEXIL_VALUE_TYPE_HH
