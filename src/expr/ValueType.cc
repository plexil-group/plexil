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

#include "ValueType.hh"

#include "Array.hh"

#include <iostream>
#include <sstream>

namespace PLEXIL
{

  const std::string &valueTypeName(ValueType ty)
  {
    switch (ty) {
    case BOOLEAN_TYPE:
      static const std::string sl_boolean("Boolean");
      return sl_boolean;

    case INTEGER_TYPE:
      static const std::string sl_integer("Integer");
      return sl_integer;

    case REAL_TYPE:
      static const std::string sl_real("Real");
      return sl_real;
      
    case STRING_TYPE:
      static const std::string sl_string("String");
      return sl_string;

    case DATE_TYPE:
      static const std::string sl_date("Date");
      return sl_date;

    case DURATION_TYPE:
      static const std::string sl_duration("Duration");
      return sl_duration;

      // Array types
    case BOOLEAN_ARRAY_TYPE:
      static const std::string sl_boolean_array("BooleanArray");
      return sl_boolean_array;

    case INTEGER_ARRAY_TYPE:
      static const std::string sl_integer_array("IntegerArray");
      return sl_integer_array;

    case REAL_ARRAY_TYPE:
      static const std::string sl_real_array("RealArray");
      return sl_real_array;

    case STRING_ARRAY_TYPE:
      static const std::string sl_string_array("StringArray");
      return sl_string_array;

      // Internal types
    case NODE_STATE_TYPE:
      static const std::string sl_node_state("NodeState");
      return sl_node_state;

    case OUTCOME_TYPE:
      static const std::string sl_outcome("NodeOutcome");
      return sl_outcome;

    case FAILURE_TYPE:
      static const std::string sl_failure("NodeFailure");
      return sl_failure;

    case COMMAND_HANDLE_TYPE:
      static const std::string sl_command_handle("NodeCommandHandle");
      return sl_failure;

    default:
      static const std::string sl_unknown("UNKNOWN");
      return sl_unknown;
    }
  }
  
  bool isUserType(ValueType ty)
  {
    return (ty > UNKNOWN_TYPE && ty < ARRAY_TYPE_MAX);
  }
  
  bool isInternalType(ValueType ty)
  {
    return (ty > INTERNAL_TYPE_OFFSET && ty < TYPE_MAX);
  }

  bool isScalarType(ValueType ty)
  {
    return (ty > UNKNOWN_TYPE && ty < SCALAR_TYPE_MAX);
  }

  bool isArrayType(ValueType ty)
  {
    return (ty > ARRAY_TYPE && ty < ARRAY_TYPE_MAX);
  }

  ValueType arrayElementType(ValueType ty)
  {
    if (ty <= ARRAY_TYPE || ty >= ARRAY_TYPE_MAX)
      return UNKNOWN_TYPE;
    return (ValueType) (ty - ARRAY_TYPE);
  }

  ValueType arrayType(ValueType elTy)
  {
    if (elTy <= UNKNOWN_TYPE || elTy > STRING_TYPE)
      return UNKNOWN_TYPE;
    return (ValueType) (elTy + ARRAY_TYPE);
  }

  template <typename T>
  void printValue(const T &val, std::ostream &s)
  {
    s << val;
  }

  template <typename T>
  void printValue(const ArrayImpl<T> &val, std::ostream &s)
  {
    // TODO - should delegate to array itself
    s << "printValue not yet implemented for arrays";
  }

  /**
   * @brief Parse one value from the incoming stream.
   * @param s Input stream.
   * @param result Reference to the place to store the result.
   * @return True if known, false if unknown or error.
   * @note If false, the result variable will not be modified.
   */

  template <typename NUM>
  bool parseValue(std::string const &s, NUM &result)
  {
    if (s.empty() || s == "UNKNOWN")
      return false;

    NUM temp;
    std::istringstream is(s);
    is >> temp;
    if (is.fail())
      return false;
    result = temp;
    return true;
  }

  template <>
  bool parseValue(std::string const &s, bool &result)
  {
    switch (s.length()) {
      
    case 1:
      if (s == "0") {
        result = false;
        return true;
      }
      if (s == "1") {
        result = true;
        return true;
      }
      return false;

    case 4:
      if (s == "true" || s == "TRUE") {
        result = true;
        return true;
      }
      return false;

    case 5:
      if (s == "false" || s == "FALSE") {
        result = false;
        return true;
      }
      // fall thru to...

    default:
      return false;
    }
  }

  // Empty sring is interpreted as UNKNOWN; is this a good idea?
  template <>
  bool parseValue(std::string const &s, std::string &result)
  {
    if (s.empty())
      return false;
    result = s;
    return true;
  }

  //
  // Explicit instantiation
  //
  template void printValue(bool const &, std::ostream &);
  template void printValue(int32_t const &, std::ostream &);
  template void printValue(double const &, std::ostream &);
  template void printValue(std::string const &, std::ostream &);
  // array types NYI

  template bool parseValue(std::string const &, bool &);
  template bool parseValue(std::string const &, int32_t &);
  template bool parseValue(std::string const &, double &);
  template bool parseValue(std::string const &, std::string &);
  // array types NYI

}
