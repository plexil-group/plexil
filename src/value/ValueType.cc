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

#include "plexil-config.h"

#include "ValueType.hh"

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "map-utils.hh"
#include "NodeConstants.hh"
#include "ParserException.hh"
#include "stricmp.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CMATH)
#include <cmath>   // for HUGE_VAL
#elif defined(HAVE_MATH_H)
#include <math.h>
#endif

#if defined(HAVE_CSTDLIB)
#include <cstdlib> // for strtod(), strtol()
#elif defined(HAVE_STDLIB_H)
#include <stdlib.h>
#endif

#if defined(HAVE_CSTRING)
#include <cstring> // strlen(), strcmp() etc.
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL
{
  // Initialize type name strings
  char const BOOLEAN_STR[] = "Boolean";
  char const INTEGER_STR[] = "Integer";
  char const REAL_STR[] = "Real";
  char const DATE_STR[] = "Date";
  char const DURATION_STR[] = "Duration";
  char const STRING_STR[] = "String";
  char const ARRAY_STR[] = "Array";
  char const BOOLEAN_ARRAY_STR[] = "BooleanArray";
  char const INTEGER_ARRAY_STR[] = "IntegerArray";
  char const REAL_ARRAY_STR[] = "RealArray";
  char const STRING_ARRAY_STR[] = "StringArray";
  char const STATE_STR[] = "State";
  char const NODE_STATE_STR[] = "NodeState";
  char const NODE_OUTCOME_STR[] = "NodeOutcome";
  char const NODE_FAILURE_STR[] = "NodeFailure";
  char const NODE_COMMAND_HANDLE_STR[] = "NodeCommandHandle";

  char const VAL_SUFFIX[] = "Value";

  char const UNKNOWN_STR[] = "UNKNOWN";

  const std::string &valueTypeName(ValueType typ)
  {
    switch (typ) {
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

    case STATE_TYPE:
      static std::string const sl_state(STATE_STR);
      return sl_state;

      // Internal types
    case NODE_STATE_TYPE:
      static std::string const sl_node_state(NODE_STATE_STR);
      return sl_node_state;

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

  bool isUserType(ValueType typ)
  {
    return typ > UNKNOWN_TYPE && typ < ARRAY_TYPE_MAX;
  }
  
  bool isInternalType(ValueType typ)
  {
    return typ > INTERNAL_TYPE_OFFSET && typ < TYPE_MAX;
  }
  
  bool isNumericType(ValueType typ)
  {
    switch (typ) {
    case INTEGER_TYPE:
    case REAL_TYPE:
    case DATE_TYPE:
    case DURATION_TYPE:
      return true;

    default:
      return false;
    }
  }

  bool isScalarType(ValueType typ)
  {
    return typ > UNKNOWN_TYPE && typ < SCALAR_TYPE_MAX;
  }

  bool isArrayType(ValueType typ)
  {
    return typ > ARRAY_TYPE && typ < ARRAY_TYPE_MAX;
  }

  ValueType arrayElementType(ValueType typ)
  {
    if (typ <= ARRAY_TYPE || typ >= ARRAY_TYPE_MAX)
      return UNKNOWN_TYPE;
    return (ValueType) (typ - ARRAY_TYPE);
  }

  ValueType arrayType(ValueType elTy)
  {
    if (elTy <= UNKNOWN_TYPE || elTy > STRING_TYPE)
      return UNKNOWN_TYPE;
    return (ValueType) (elTy + ARRAY_TYPE);
  }

  bool areTypesCompatible(ValueType dest, ValueType src)
  {
    if (dest == UNKNOWN_TYPE) // e.g. parameters declared Any
      return true;

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

  // Optimized version, makes at most one call to strlen() and one to strcmp()
  ValueType parseValueType(char const *typeStr)
  {
    if (!typeStr)
      return UNKNOWN_TYPE;
    switch (*typeStr) {
    case 'A': // Array
      if (!strcmp(typeStr, ARRAY_STR))
        return ARRAY_TYPE;
      break;

    case 'B': // Boolean, BooleanArray
      switch (strlen(typeStr)) {
      case 7:
        if (!strcmp(typeStr, BOOLEAN_STR))
          return BOOLEAN_TYPE;
        break;

      case 12:
        if (!strcmp(typeStr, BOOLEAN_ARRAY_STR))
          return BOOLEAN_ARRAY_TYPE;
        break;

      default:
        break;
      }
      
      break;

    case 'D': // Date, Duration
      if (!strcmp(typeStr, DATE_STR))
        return DATE_TYPE;
      if (!strcmp(typeStr, DURATION_STR))
        return DURATION_TYPE;
      break;

    case 'I': // Integer, IntegerArray
      switch (strlen(typeStr)) {
      case 7:
        if (!strcmp(typeStr, INTEGER_STR))
          return INTEGER_TYPE;
        break;

      case 12:
        if (!strcmp(typeStr, INTEGER_ARRAY_STR))
          return INTEGER_ARRAY_TYPE;
        break;

      default:
        break;
      }

      break;

    case 'N': // NodeState, NodeOutcome, NodeFailure, NodeCommandHandle
      switch (strlen(typeStr)) {
      case 9:
        if (!strcmp(typeStr, NODE_STATE_STR))
          return NODE_STATE_TYPE;
        break;

      case 11:
        if (typeStr[4] == 'F' && !strcmp(typeStr, NODE_FAILURE_STR))
          return FAILURE_TYPE;
        if (!strcmp(typeStr, NODE_OUTCOME_STR))
          return OUTCOME_TYPE;
        break;

      case 17:
        if (!strcmp(typeStr, NODE_COMMAND_HANDLE_STR))
          return COMMAND_HANDLE_TYPE;
        break;

      default:
        break;
      }
      break;

    case 'R': // Real, RealArray
      switch (strlen(typeStr)) {
      case 4:
        if (!strcmp(typeStr, REAL_STR))
          return REAL_TYPE;
        break;

      case 9:
        if (!strcmp(typeStr, REAL_ARRAY_STR))
          return REAL_ARRAY_TYPE;
        break;

      default:
        break;
      }

      break;

    case 'S': // State, String, StringArray
      switch (strlen(typeStr)) {
      case 5:
        if (!strcmp(typeStr, STATE_STR))
          return STATE_TYPE;
        break;
        
      case 6:
        if (!strcmp(typeStr, STRING_STR))
          return STRING_TYPE;
        break;

      case 11:
        if (!strcmp(typeStr, STRING_ARRAY_STR))
          return STRING_ARRAY_TYPE;
        break;

      default:
        break;
      }

      break;

    default:
      break;
    }
    // Fall-through return
    return UNKNOWN_TYPE;
  }

  ValueType parseValueType(const std::string& typeStr)
  {
    return parseValueType(typeStr.c_str());
  }

  template <typename T>
  void printValue(const T &val, std::ostream &str)
  {
    str << val;
  }

  // Specialization for Real
  template <>
  void printValue(const Real &val, std::ostream &str)
  {
    str << std::setprecision(15) << val;
  }
  
  // Specialization for internal enums
  template <>
  void printValue(const uint16_t &val, std::ostream &str)
  {
    if (isNodeStateValid(val))
      str << nodeStateName(val);
    else if (isNodeOutcomeValid(val))
      str << outcomeName(val);
    else if (isFailureTypeValid(val))
      str << failureTypeName(val);
    else if (isCommandHandleValid(val))
      str << commandHandleValueName(val);
    else
      str << "<INVALID ENUM " << val << ">";
  }

  template <typename T>
  void printValue(ArrayImpl<T> const &val, std::ostream &str)
  {
    str << val;
  }

  /**
   * @brief Parse one value from the incoming stream.
   * @param s Input stream.
   * @param result Reference to the place to store the result.
   * @return True if known, false if unknown.
   * @note If false, the result variable will not be modified.
   */

  template <>
  bool parseValue(char const *str, Boolean &result)
  {
    assertTrue_1(str);

    switch (strlen(str)) {
    case 1:
      if (*str == '0') {
        result = false;
        return true;
      }
      if (*str == '1') {
        result = true;
        return true;
      }
      break;

    case 4:
      if (!stricmp(str, "true")) {
        result = true;
        return true;
      }
      break;
 
    case 5:
      if (!stricmp(str, "false")) {
        result = false;
        return true;
      }
      break;

    case 7:
      if (!strcmp(str, "UNKNOWN"))
        return false;
      break;

    default:
      break;
    }
    // No match
    reportParserException("parseValue: \"" << str << "\" is not a valid Boolean value");
  }

  template <>
  bool parseValue<Integer>(char const *str, Integer &result)
  {
    assertTrue_1(str);
    if (!*str || 0 == strcmp(str, "UNKNOWN"))
      return false;

    char * ends;
    errno = 0;
    long temp = strtol(str, &ends, 0);
    checkParserException(ends != str && *ends == '\0',
                         "parseValue: \"" << str << "\" is an invalid value for an Integer");
    checkParserException(errno == 0
                         && temp <= std::numeric_limits<Integer>::max()
                         && temp >= std::numeric_limits<Integer>::min(),
                         "parseValue: " << str << " is out of range for an Integer");
    result = (Integer) temp;
    return true;
  }

  template <>
  bool parseValue<Real>(char const *str, Real &result)
  {
    assertTrue_1(str);
    if (!*str || 0 == strcmp(str, "UNKNOWN"))
      return false;

    char * ends;
    errno = 0;
    Real temp = strtod(str, &ends);
    checkParserException(ends != str && *ends == '\0',
                         "parseValue: \"" << str << "\" is an invalid value for a Real");
    checkParserException(temp != HUGE_VAL && temp != -HUGE_VAL,
                         "parseValue: " << str << " is out of range for a Real");
    result = temp;
    return true;
  }

  // Empty string is valid
  template <>
  bool parseValue(char const *str, String &result)
  {
    assertTrue_1(str);
    result = str;
    return true;
  }

  //
  // Serialization
  //

  // Default methods
  template <typename T>
  char *serialize(T const &/* o */, char */* buf */)
  {
    return NULL;
  }

  template <typename T>
  size_t serialSize(T const &/* o */)
  {
    return 0;
  }

  template <typename T>
  char const *deserialize(T &o, char const *buf)
  {
    return NULL;
  }

  //
  // Boolean
  //

  template <>
  char *serialize<Boolean>(Boolean const &o, char *buf)
  {
    *buf++ = BOOLEAN_TYPE;
    *buf++ = (char) o;
    return buf;
  }

  template <>
  char const *deserialize<Boolean>(Boolean &o, char const *buf)
  {
    if (BOOLEAN_TYPE != (ValueType) *buf++)
      return NULL;
    o = (Boolean) *buf++;
    return buf;
  }

  template <>
  size_t serialSize<Boolean>(Boolean const &o)
  {
    return 2;
  }

  //
  // CommandHandleValue
  //

  template <>
  char *serialize<CommandHandleValue>(CommandHandleValue const &o, char *buf)
  {
    *buf++ = COMMAND_HANDLE_TYPE;
    *buf++ = (char) o;
    return buf;
  }

  // For all internal enum types
  // Currently only COMMAND_HANDLE_TYPE supported
  template <>
  char const *deserialize<uint16_t>(uint16_t &o, char const *buf)
  {
    switch((ValueType) *buf++) {
      // Future (?)
    // case NODE_STATE_TYPE:
    // case OUTCOME_TYPE:
    // case FAILURE_TYPE:
    case COMMAND_HANDLE_TYPE:
      break;
      
    default:
      return NULL;
    }

    o = (uint16_t) *buf++;
    return buf;
  }

  template <>
  size_t serialSize<CommandHandleValue>(CommandHandleValue const &/* o */)
  {
    return 2;
  }
  

  //
  // Integer
  //

  template <>
  char *serialize<Integer>(Integer const &o, char *buf)
  {
    *buf++ = INTEGER_TYPE;
    // Store in big-endian format
    *buf++ = (char) (0xFF & (o >> 24));
    *buf++ = (char) (0xFF & (o >> 16));
    *buf++ = (char) (0xFF & (o >> 8));
    *buf++ = (char) (0xFF & o);
    return buf;
  }

  template <>
  char const *deserialize<Integer>(Integer &o, char const *buf)
  {
    if (INTEGER_TYPE != (ValueType) *buf++)
      return NULL;
    uint32_t result = ((uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++) << 8;
    result = (result + (uint32_t) (unsigned char) *buf++);
    o = (Integer) result;
    return buf;
  }

  template <>
  size_t serialSize<Integer>(Integer const &o)
  {
    return 5;
  }

  //
  // Real
  //

  template <>
  char *serialize<Real>(Real const &o, char *buf)
  {
    *buf++ = REAL_TYPE;
    union {
      Real r;
      uint64_t l;
    };
    r = o;
    // Store in big-endian format
    *buf++ = (char) (0xFF & (l >> 56));
    *buf++ = (char) (0xFF & (l >> 48));
    *buf++ = (char) (0xFF & (l >> 40));
    *buf++ = (char) (0xFF & (l >> 32));
    *buf++ = (char) (0xFF & (l >> 24));
    *buf++ = (char) (0xFF & (l >> 16));
    *buf++ = (char) (0xFF & (l >> 8));
    *buf++ = (char) (0xFF & l);
    return buf;
  }

  template <>
  char const *deserialize<Real>(Real &o, char const *buf)
  {
    if (REAL_TYPE != (ValueType) *buf++)
      return NULL;
    union {
      Real r;
      uint64_t l;
    };
    l = (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++; l = l << 8;
    l += (uint64_t) (unsigned char) *buf++;
    o = r;
    return buf;
  }

  template <>
  size_t serialSize<Real>(Real const &o)
  {
    return 9;
  }

  //
  // String
  //

  template <>
  char *serialize<String>(String const &o, char *buf)
  {
    size_t siz = o.size();
    if (siz > 0xFFFFFF)
      return NULL; // too big

    *buf++ = STRING_TYPE;
    // Put 3 bytes of size first - std::string may contain embedded NUL
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);
    memcpy(buf, o.c_str(), siz);
    return buf + siz;
  }

  template <>
  char const *deserialize<String>(String &o, char const *buf)
  {
    if (STRING_TYPE != (ValueType) *buf++)
      return NULL;

    // Get 3 bytes of size
    size_t siz = ((size_t) (unsigned char) *buf++) << 8;
    siz = (siz + (size_t) (unsigned char) *buf++) << 8;
    siz = siz + (size_t) (unsigned char) *buf++;

    o.replace(o.begin(), o.end(), buf, siz);
    return buf + siz;
  }

  template <>
  size_t serialSize<String>(String const &o)
  {
    return o.size() + 4;
  }

  //
  // Character string
  //

  template <>
  char *serialize<char const *>(char const * const &o, char *buf)
  {
    size_t siz = strlen(o);
    if (siz > 0xFFFFFF)
      return NULL; // too big

    *buf++ = STRING_TYPE;
    // Put 3 bytes of size first
    *buf++ = (char) (0xFF & (siz >> 16));
    *buf++ = (char) (0xFF & (siz >> 8));
    *buf++ = (char) (0xFF & siz);
    memcpy(buf, o, siz);
    return buf + siz;
  }

  template <>
  char const *deserialize<char *>(char *&o, char const *buf)
  {
    if (STRING_TYPE != (ValueType) *buf++)
      return NULL;

    // Get 3 bytes of size
    size_t siz = ((size_t) (unsigned char) *buf++) << 8;
    siz = (siz + (size_t) (unsigned char) *buf++) << 8;
    siz = siz + (size_t) (unsigned char) *buf++;

    o = (char *) malloc(siz + 1);
    memcpy(o, buf, siz);
    o[siz] = '\0'; 

    return buf + siz;
  }

  template <>
  size_t serialSize<char const *>(char const * const &o)
  {
    return strlen(o) + 4;
  }

  //
  // Explicit instantiation
  //
  template void printValue(Boolean const &, std::ostream &);
  // template void printValue(uint16_t const &, std::ostream &); // redundant
  template void printValue(Integer const &, std::ostream &);
  // template void printValue(Real const &, std::ostream &); // redundant
  template void printValue(String const &, std::ostream &);

  template void printValue(ArrayImpl<Boolean> const &, std::ostream &);
  template void printValue(ArrayImpl<Integer> const &, std::ostream &);
  template void printValue(ArrayImpl<Real> const &, std::ostream &);
  template void printValue(ArrayImpl<String> const &, std::ostream &);

  // array types NYI

}
