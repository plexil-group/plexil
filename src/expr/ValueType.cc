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

#include <plexil-config.h>

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "Error.hh"
#include "expression-schema.hh"
#include "NodeConstants.hh"
#include "stricmp.h"

#include <cerrno>
#include <cmath>   // for HUGE_VAL
#include <cstdlib> // for strtod(), strtol()
#include <iostream>
#include <limits>
#include <sstream>

namespace PLEXIL
{
  // Initialize type name strings
  std::string const BOOLEAN_STR = "Boolean";
  std::string const INTEGER_STR = "Integer";
  std::string const REAL_STR = "Real";
  std::string const DATE_STR = "Date";
  std::string const DURATION_STR = "Duration";
  std::string const STRING_STR = "String";
  std::string const ARRAY_STR = "Array";
  std::string const BOOLEAN_ARRAY_STR = "BooleanArray";
  std::string const INTEGER_ARRAY_STR = "IntegerArray";
  std::string const REAL_ARRAY_STR = "RealArray";
  std::string const STRING_ARRAY_STR = "StringArray";
  std::string const NODE_STATE_STR = "NodeState";
  std::string const NODE_OUTCOME_STR = "NodeOutcome";
  std::string const NODE_FAILURE_STR = "NodeFailure";
  std::string const NODE_COMMAND_HANDLE_STR = "NodeCommandHandle";
  std::string const UNKNOWN_STR = "UNKNOWN";

  const std::string &valueTypeName(ValueType ty)
  {
    switch (ty) {
    case BOOLEAN_TYPE:
      return BOOLEAN_STR;

    case INTEGER_TYPE:
      return INTEGER_STR;

    case REAL_TYPE:
      return REAL_STR;
      
    case STRING_TYPE:
      return STRING_STR;

    case DATE_TYPE:
      return DATE_STR;

    case DURATION_TYPE:
      return DURATION_STR;

      // Array types
    case BOOLEAN_ARRAY_TYPE:
      return BOOLEAN_ARRAY_STR;

    case INTEGER_ARRAY_TYPE:
      return INTEGER_ARRAY_STR;

    case REAL_ARRAY_TYPE:
      return REAL_ARRAY_STR;

    case STRING_ARRAY_TYPE:
      return STRING_ARRAY_STR;

      // Internal types
    case NODE_STATE_TYPE:
      return NODE_STATE_STR;

    case OUTCOME_TYPE:
      return NODE_OUTCOME_STR;

    case FAILURE_TYPE:
      return NODE_FAILURE_STR;

    case COMMAND_HANDLE_TYPE:
      return NODE_COMMAND_HANDLE_STR;

    default:
      return UNKNOWN_STR;
    }
  }

  const std::string &typeNameAsValue(ValueType ty)
  {
    static std::string const sl_val = VAL_TAG;

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

  const std::string &typeNameAsVariable(ValueType ty)
  {
    static std::string const sl_var = VAR_TAG;

    switch (ty) {
    case BOOLEAN_TYPE:
      static std::string const sl_boolvar = BOOLEAN_STR + sl_var;
      return sl_boolvar;

    case INTEGER_TYPE:
      static std::string const sl_intvar = INTEGER_STR + sl_var;
      return sl_intvar;

    case REAL_TYPE:
      static std::string const sl_realvar = REAL_STR + sl_var;
      return sl_realvar;
      
    case STRING_TYPE:
      static std::string const sl_stringvar = STRING_STR + sl_var;
      return sl_stringvar;

    case DATE_TYPE:
      static std::string const sl_datevar = DATE_STR + sl_var;
      return sl_datevar;

    case DURATION_TYPE:
      static std::string const sl_durvar = DURATION_STR + sl_var;
      return sl_durvar;

      // Array types

      // generic (for array reference)
    case ARRAY_TYPE:
      static std::string const sl_arrvar = ARRAY_STR + sl_var;
      return sl_arrvar;

    case BOOLEAN_ARRAY_TYPE:
      static std::string const sl_boolarrvar = BOOLEAN_ARRAY_STR + sl_var;
      return sl_boolarrvar;

    case INTEGER_ARRAY_TYPE:
      static std::string const sl_intarrvar = INTEGER_ARRAY_STR + sl_var;
      return sl_intarrvar;

    case REAL_ARRAY_TYPE:
      static std::string const sl_realarrvar = REAL_ARRAY_STR + sl_var;
      return sl_realarrvar;

    case STRING_ARRAY_TYPE:
      static std::string const sl_stringarrvar = STRING_ARRAY_STR + sl_var;
      return sl_stringarrvar;

      // Internal types
    case NODE_STATE_TYPE:
      static std::string const sl_nsvar = NODE_STATE_STR + sl_var;
      return sl_nsvar;

    case OUTCOME_TYPE:
      static std::string const sl_outcomevar = NODE_OUTCOME_STR + sl_var;
      return sl_outcomevar;

    case FAILURE_TYPE:
      static std::string const sl_failvar = NODE_FAILURE_STR + sl_var;
      return sl_failvar;

    case COMMAND_HANDLE_TYPE:
      static std::string const sl_handlevar = NODE_COMMAND_HANDLE_STR + sl_var;
      return sl_handlevar;

    default:
      return sl_var;
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

  ValueType parseValueTypePrefix(char const *str, size_t prefixLen)
  {
    switch (prefixLen) {
    case 4: 
      if (0 == REAL_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::REAL_TYPE;
      else if (0 == DATE_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::DATE_TYPE;
      else 
        return PLEXIL::UNKNOWN_TYPE;

    case 5:
      if (0 == ARRAY_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::ARRAY_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 6:
      if (0 == STRING_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::STRING_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 7:
      if (0 == INTEGER_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::INTEGER_TYPE;
      else if (0 == BOOLEAN_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::BOOLEAN_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 8:
      if (0 == DURATION_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::DURATION_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 9:
      if (0 == REAL_ARRAY_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::REAL_ARRAY_TYPE;
      else if (0 == NODE_STATE_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::NODE_STATE_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 11:
      if (0 == NODE_OUTCOME_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::OUTCOME_TYPE;
      else if (0 == NODE_FAILURE_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::FAILURE_TYPE;
      else if (0 == STRING_ARRAY_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::STRING_ARRAY_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 12:
      if (0 == BOOLEAN_ARRAY_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::BOOLEAN_ARRAY_TYPE;
      else if (0 == INTEGER_ARRAY_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::INTEGER_ARRAY_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;

    case 17:
      if (0 == NODE_COMMAND_HANDLE_STR.compare(0, prefixLen, str, prefixLen))
        return PLEXIL::COMMAND_HANDLE_TYPE;
      else
        return PLEXIL::UNKNOWN_TYPE;
      
      // default case
    default:
      return PLEXIL::UNKNOWN_TYPE;
    }
  }

  ValueType parseValueTypePrefix(const std::string & str, size_t prefixLen)
  {
    return parseValueTypePrefix(str.c_str(), prefixLen);
  }

  ValueType parseValueType(char const *typeStr)
  {
    return parseValueTypePrefix(typeStr, strlen(typeStr));
  }

  ValueType parseValueType(const std::string& typeStr)
  {
    return parseValueTypePrefix(typeStr.c_str(), typeStr.length());
  }

  template <typename T>
  void printValue(const T &val, std::ostream &s)
  {
    s << val;
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
    case 0:
      return false;
      
    case 1:
      if (*s == '0') {
        result = false;
        return true;
      }
      if (*s == '1') {
        result = true;
        return true;
      }
      return false;

    case 4:
      if (0 == stricmp(s, "true")) {
        result = true;
        return true;
      }
      return false;

    case 5:
      if (0 == stricmp(s, "false")) {
        result = false;
        return true;
      }
      // fall thru to...

    default:
      checkParserException(0 == strcmp(s, "UNKNOWN"),
                           "parseValue: \"" << s << "\" is not a valid Boolean value");
      return false;
    }
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
