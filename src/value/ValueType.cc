/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include <plexil-config.h>

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "Error.hh"
#include "map-utils.hh"
#include "NodeConstants.hh"
#include "SimpleMap.hh"
#include "stricmp.h"

#include <cerrno>
#include <cmath>   // for HUGE_VAL
#include <cstdlib> // for strtod(), strtol()
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

namespace PLEXIL
{
  // Initialize type name strings
  char const *BOOLEAN_STR = "Boolean";
  char const *INTEGER_STR = "Integer";
  char const *REAL_STR = "Real";
  char const *DATE_STR = "Date";
  char const *DURATION_STR = "Duration";
  char const *STRING_STR = "String";
  char const *ARRAY_STR = "Array";
  char const *BOOLEAN_ARRAY_STR = "BooleanArray";
  char const *INTEGER_ARRAY_STR = "IntegerArray";
  char const *REAL_ARRAY_STR = "RealArray";
  char const *STRING_ARRAY_STR = "StringArray";
  char const *NODE_STATE_STR = "NodeState";
  char const *NODE_OUTCOME_STR = "NodeOutcome";
  char const *NODE_FAILURE_STR = "NodeFailure";
  char const *NODE_COMMAND_HANDLE_STR = "NodeCommandHandle";

  char const *VAL_SUFFIX = "Value";
  char const *VAR_SUFFIX = "Variable";

  char const *UNKNOWN_STR = "UNKNOWN";

  const std::string &valueTypeName(ValueType ty)
  {
    switch (ty) {
    case BOOLEAN_TYPE:
      static std::string const sl_boolean(BOOLEAN_STR);
      return sl_boolean;

    case INTEGER_TYPE:
      static std::string const sl_integer(INTEGER_STR);
      return sl_integer;

    case REAL_TYPE:
      static std::string const sl_real(REAL_STR);
      return sl_real;
      
    case STRING_TYPE:
      static std::string const sl_string(STRING_STR);
      return sl_string;

    case DATE_TYPE:
      static std::string const sl_date(DATE_STR);
      return sl_date;

    case DURATION_TYPE:
      static std::string const sl_duration(DURATION_STR);
      return sl_duration;

      // Array types
    case BOOLEAN_ARRAY_TYPE:
      static std::string const sl_boolean_array(BOOLEAN_ARRAY_STR);
      return sl_boolean_array;

    case INTEGER_ARRAY_TYPE:
      static std::string const sl_integer_array(INTEGER_ARRAY_STR);
      return sl_integer_array;

    case REAL_ARRAY_TYPE:
      static std::string const sl_real_array(REAL_ARRAY_STR);
      return sl_real_array;

    case STRING_ARRAY_TYPE:
      static std::string const sl_string_array(STRING_ARRAY_STR);
      return sl_string_array;

      // Internal types
    case NODE_STATE_TYPE:
      static std::string const sl_state(NODE_STATE_STR);
      return sl_state;

    case OUTCOME_TYPE:
      static std::string const sl_outcome(NODE_OUTCOME_STR);
      return sl_outcome;

    case FAILURE_TYPE:
      static std::string const sl_failure(NODE_FAILURE_STR);
      return sl_failure;

    case COMMAND_HANDLE_TYPE:
      static std::string const sl_command_handle(NODE_COMMAND_HANDLE_STR);
      return sl_command_handle;

    default:
      static std::string const sl_unknown(UNKNOWN_STR);
      return sl_unknown;
    }
  }

  const std::string &typeNameAsValue(ValueType ty)
  {
    static std::string const sl_val = VAL_SUFFIX;

    switch (ty) {
    case BOOLEAN_TYPE:
      static std::string const sl_boolval = BOOLEAN_STR + sl_val;
      return sl_boolval;

    case INTEGER_TYPE:
      static std::string const sl_intval = INTEGER_STR + sl_val;
      return sl_intval;

    case REAL_TYPE:
      static std::string const sl_realval = REAL_STR + sl_val;
      return sl_realval;
      
    case STRING_TYPE:
      static std::string const sl_stringval = STRING_STR + sl_val;
      return sl_stringval;

    case DATE_TYPE:
      static std::string const sl_dateval = DATE_STR + sl_val;
      return sl_dateval;

    case DURATION_TYPE:
      static std::string const sl_durval = DURATION_STR + sl_val;
      return sl_durval;

      // Array types
    case BOOLEAN_ARRAY_TYPE:
      static std::string const sl_boolarrval = BOOLEAN_ARRAY_STR + sl_val;
      return sl_boolarrval;

    case INTEGER_ARRAY_TYPE:
      static std::string const sl_intarrval = INTEGER_ARRAY_STR + sl_val;
      return sl_intarrval;

    case REAL_ARRAY_TYPE:
      static std::string const sl_realarrval = REAL_ARRAY_STR + sl_val;
      return sl_realarrval;

    case STRING_ARRAY_TYPE:
      static std::string const sl_stringarrval = STRING_ARRAY_STR + sl_val;
      return sl_stringarrval;

      // Internal types
    case NODE_STATE_TYPE:
      static std::string const sl_nsval = NODE_STATE_STR + sl_val;
      return sl_nsval;

    case OUTCOME_TYPE:
      static std::string const sl_outcomeval = NODE_OUTCOME_STR + sl_val;
      return sl_outcomeval;

    case FAILURE_TYPE:
      static std::string const sl_failval = NODE_FAILURE_STR + sl_val;
      return sl_failval;

    case COMMAND_HANDLE_TYPE:
      static std::string const sl_handleval = NODE_COMMAND_HANDLE_STR + sl_val;
      return sl_handleval;

    default:
      return sl_val;
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
  
  bool isNumericType(ValueType ty)
  {
    switch (ty) {
    case INTEGER_TYPE:
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
      return true;

    default:
      return false;
    }
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

  bool areTypesCompatible(ValueType dest, ValueType src)
  {
    if (src == UNKNOWN_TYPE) // e.g. undeclared or computed lookups, commands
      return true;

    if (dest == src)
      return true; // same type is always compatible

    switch (dest) {
      // Date and duration can receive real or integer
    case DATE_TYPE:
    case DURATION_TYPE:
      return src == REAL_TYPE || src == INTEGER_TYPE;

      // Real can receive integer, date, duration
    case REAL_TYPE:
      return src == INTEGER_TYPE || src == DATE_TYPE || src == DURATION_TYPE;

      // Generic array can receive any array
    case ARRAY_TYPE:
      return isArrayType(src);

    default: // unknown, unimplemented, or unsupported
      return false;
    }
  }

  //
  // ValueType parsing
  //

  typedef SimpleMap<std::string, ValueType> NameTypeTable;

  // Size argument to constructor should be at least as big as # of entries in table
  static NameTypeTable s_nameTypeTable(15);

  static void initNameTypeTable()
  {
    if (s_nameTypeTable.empty()) {
      s_nameTypeTable.insert(BOOLEAN_STR, BOOLEAN_TYPE);
      s_nameTypeTable.insert(INTEGER_STR, INTEGER_TYPE);
      s_nameTypeTable.insert(REAL_STR, REAL_TYPE);
      s_nameTypeTable.insert(STRING_STR, STRING_TYPE);
      s_nameTypeTable.insert(DATE_STR, DATE_TYPE);
      s_nameTypeTable.insert(DURATION_STR, DURATION_TYPE);

      s_nameTypeTable.insert(ARRAY_STR, ARRAY_TYPE);
      s_nameTypeTable.insert(BOOLEAN_ARRAY_STR, BOOLEAN_ARRAY_TYPE);
      s_nameTypeTable.insert(INTEGER_ARRAY_STR, INTEGER_ARRAY_TYPE);
      s_nameTypeTable.insert(REAL_ARRAY_STR, REAL_ARRAY_TYPE);
      s_nameTypeTable.insert(STRING_ARRAY_STR, STRING_ARRAY_TYPE);

      s_nameTypeTable.insert(NODE_STATE_STR, NODE_STATE_TYPE);
      s_nameTypeTable.insert(NODE_OUTCOME_STR, OUTCOME_TYPE);
      s_nameTypeTable.insert(NODE_FAILURE_STR, FAILURE_TYPE);
      s_nameTypeTable.insert(NODE_COMMAND_HANDLE_STR, COMMAND_HANDLE_TYPE);
    }
  }

  ValueType parseValueType(char const *typeStr)
  {
    if (!typeStr)
      return UNKNOWN_TYPE;
    initNameTypeTable();
    NameTypeTable::const_iterator it = 
      s_nameTypeTable.find<char const *, CStringComparator>(typeStr);
    if (it == s_nameTypeTable.end())
      return UNKNOWN_TYPE;
    else
      return it->second;
  }

  ValueType parseValueType(const std::string& typeStr)
  {
    initNameTypeTable();
    NameTypeTable::const_iterator it = s_nameTypeTable.find(typeStr);
    if (it == s_nameTypeTable.end())
      return UNKNOWN_TYPE;
    else
      return it->second;
  }

  size_t scanValueTypePrefix(char const *typeStr, ValueType &result)
  {
    if (!typeStr)
      return 0;
    initNameTypeTable();
    NameTypeTable::const_iterator it = 
      s_nameTypeTable.findLast<char const *, StringPrefixComparator>(typeStr);
    if (it == s_nameTypeTable.end())
      return 0; // failure
    result = it->second;
    return it->first.size();
  }

  template <typename T>
  void printValue(const T &val, std::ostream &s)
  {
    s << val;
  }

  // Specialization for Real
  template <>
  void printValue(const double &val, std::ostream &s)
  {
    s << std::setprecision(15) << val;
  }
  
  // Specialization for internal enums
  template <>
  void printValue(const uint16_t &val, std::ostream &s)
  {
    if (isNodeStateValid(val))
      s << nodeStateName(val);
    else if (isNodeOutcomeValid(val))
      s << outcomeName(val);
    else if (isFailureTypeValid(val))
      s << failureTypeName(val);
    else if (isCommandHandleValid(val))
      s << commandHandleValueName(val);
    else
      s << "<INVALID ENUM " << val << ">";
  }

  template <typename T>
  void printValue(ArrayImpl<T> const &val, std::ostream &s)
  {
    s << val;
  }

  /**
   * @brief Parse one value from the incoming stream.
   * @param s Input stream.
   * @param result Reference to the place to store the result.
   * @return True if known, false if unknown.
   * @note If false, the result variable will not be modified.
   */

  template <>
  bool parseValue(char const *s, bool &result)
    throw (ParserException)
  {
    assertTrue_1(s);
    switch (strlen(s)) {
    case 1:
      if (*s == '0') {
        result = false;
        return true;
      }
      if (*s == '1') {
        result = true;
        return true;
      }
      break;

    case 4:
      if (0 == stricmp(s, "true")) {
        result = true;
        return true;
      }
      break;
 
    case 5:
      if (0 == stricmp(s, "false")) {
        result = false;
        return true;
      }
      break;

    default:
      if (0 == strcmp(s, "UNKNOWN"))
        return false;
      break;
    }
    // No match
    checkParserException(ALWAYS_FAIL,
                         "parseValue: \"" << s << "\" is not a valid Boolean value");
  }

  template <>
  bool parseValue<int32_t>(char const *s, int32_t &result)
    throw (ParserException)
  {
    assertTrue_1(s);
    if (!*s || 0 == strcmp(s, "UNKNOWN"))
      return false;

    char * ends;
    errno = 0;
    long temp = strtol(s, &ends, 0);
    checkParserException(ends != s && *ends == '\0',
                         "parseValue: \"" << s << "\" is an invalid value for an Integer");
    checkParserException(errno == 0
                         && temp <= std::numeric_limits<int32_t>::max()
                         && temp >= std::numeric_limits<int32_t>::min(),
                         "parseValue: " << s << " is out of range for an Integer");
    result = (int32_t) temp;
    return true;
  }

  template <>
  bool parseValue<double>(char const *s, double &result)
    throw (ParserException)
  {
    assertTrue_1(s);
    if (!*s || 0 == strcmp(s, "UNKNOWN"))
      return false;

    char * ends;
    errno = 0;
    double temp = strtod(s, &ends);
    checkParserException(ends != s && *ends == '\0',
                         "parseValue: \"" << s << "\" is an invalid value for a Real");
    checkParserException(temp != HUGE_VAL && temp != -HUGE_VAL,
                         "parseValue: " << s << " is out of range for a Real");
    result = temp;
    return true;
  }

  // Empty string is valid
  template <>
  bool parseValue(char const *s, std::string &result)
    throw (ParserException)
  {
    assertTrue_1(s);
    result = s;
    return true;
  }

  //
  // Explicit instantiation
  //
  template void printValue(bool const &, std::ostream &);
  // template void printValue(uint16_t const &, std::ostream &); // redundant
  template void printValue(int32_t const &, std::ostream &);
  template void printValue(double const &, std::ostream &);
  template void printValue(std::string const &, std::ostream &);

  template void printValue(ArrayImpl<bool> const &, std::ostream &);
  template void printValue(ArrayImpl<int32_t> const &, std::ostream &);
  template void printValue(ArrayImpl<double> const &, std::ostream &);
  template void printValue(ArrayImpl<std::string> const &, std::ostream &);

  // array types NYI

}
