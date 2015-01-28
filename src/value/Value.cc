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

#include "Value.hh"

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "Error.hh"
#include "NodeConstants.hh"

namespace PLEXIL
{

  Value::Value()
    : m_type(UNKNOWN_TYPE),
      m_known(false)
  {}

  Value::Value(Value const &other)
    : m_type(other.m_type),
      m_known(other.m_known)
  {
    if (!m_known)
      return;
    switch (m_type) {
      // Unknown - do nothing
    case UNKNOWN_TYPE:
      break;

      // Immediate data - copy the union
    case BOOLEAN_TYPE:
    case INTEGER_TYPE:
    case REAL_TYPE:
    case NODE_STATE_TYPE:
    case OUTCOME_TYPE:
    case FAILURE_TYPE:
    case COMMAND_HANDLE_TYPE:
      m_value = other.m_value;
      break;

      // Copy the actual value
    case STRING_TYPE:
      m_value.stringValue = new std::string(*other.m_value.stringValue);
      break;

      // Copy the entire array
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      m_value.arrayValue = other.m_value.arrayValue->clone();
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value copy constructor: unknown type");
      break;
    }
  }

  Value::Value(bool val)
    : m_type(BOOLEAN_TYPE),
      m_known(true)
  {
    m_value.booleanValue = val;
  }

  Value::Value(uint16_t enumVal, ValueType typ)
    : m_type(typ)
  {
    switch (m_type) {
      // Internal enumerations
    case NODE_STATE_TYPE:
    case OUTCOME_TYPE:
    case FAILURE_TYPE:
    case COMMAND_HANDLE_TYPE:
      m_value.enumValue = enumVal;
      m_known = true;
      break;

    default:
      m_known = false;
    }
  }
      
  Value::Value(int32_t val)
    : m_type(INTEGER_TYPE),
      m_known(true)
  {
    m_value.integerValue = val;
  }

  Value::Value(double val)
    : m_type(REAL_TYPE),
      m_known(true)
  {
    m_value.realValue = val;
  }

  Value::Value(std::string const &val)
    : m_type(STRING_TYPE),
      m_known(true)
  {
    m_value.stringValue = new std::string(val);
  }

  Value::Value(char const *val)
    : m_type(STRING_TYPE),
      m_known(true)
  {
    m_value.stringValue = new std::string(val);
  }

  Value::Value(BooleanArray const &val)
    : m_type(BOOLEAN_ARRAY_TYPE),
      m_known(true)
  {
    m_value.arrayValue = static_cast<Array *>(new BooleanArray(val));
  }

  Value::Value(IntegerArray const &val)
    : m_type(INTEGER_ARRAY_TYPE),
      m_known(true)
  {
    m_value.arrayValue = static_cast<Array *>(new IntegerArray(val));
  }

  Value::Value(RealArray const &val)
    : m_type(REAL_ARRAY_TYPE),
      m_known(true)
  {
    m_value.arrayValue = static_cast<Array *>(new RealArray(val));
  }

  Value::Value(StringArray const &val)
    : m_type(STRING_ARRAY_TYPE),
      m_known(true)
  {
    m_value.arrayValue = static_cast<Array *>(new StringArray(val));
  }

  Value::Value(std::vector<Value> const &vals)
    : m_type(UNKNOWN_TYPE),
      m_known(true)
  {
    size_t len = vals.size();

    // Determine element type
    ValueType eltType = UNKNOWN_TYPE;
    // FIXME: Handle booleans
    for (size_t i = 0; i < len; ++i) {
      ValueType itype = vals[i].valueType();
      if (eltType == UNKNOWN_TYPE)
        eltType = itype;
      else if (eltType == INTEGER_TYPE && itype == REAL_TYPE)
        eltType = itype; // promote int to real
      else if (eltType != itype) {
        assertTrue_2(ALWAYS_FAIL, "Value constructor: Inconsistent value types in vector");
        m_known = false;
      }
      // else type is consistent
    }

    assertTrue_2(eltType != UNKNOWN_TYPE,
                 "Value constructor: Can't make array of all unknowns");
    assertTrue_2(eltType < SCALAR_TYPE_MAX,
                 "Value constructor: Can't make array of arrays");

    // Construct array value
    switch (eltType) {
    case BOOLEAN_TYPE: {
      m_type = BOOLEAN_ARRAY_TYPE;
      BooleanArray *ary = new BooleanArray(len);
      m_value.arrayValue = static_cast<Array *>(ary);
      for (size_t i = 0; i < len; ++i) {
        bool temp;
        if (vals[i].getValue(temp))
          ary->setElement(i, temp);
        else
          ary->setElementUnknown(i);
      }
      break;
    }

    case INTEGER_TYPE: {
      m_type = INTEGER_ARRAY_TYPE;
      IntegerArray *ary = new IntegerArray(len);
      m_value.arrayValue = static_cast<Array *>(ary);
      for (size_t i = 0; i < len; ++i) {
        int32_t temp;
        if (vals[i].getValue(temp))
          ary->setElement(i, temp);
        else
          ary->setElementUnknown(i);
      }
      break;
    }

    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
    case REAL_TYPE: {
      m_type = REAL_ARRAY_TYPE;
      RealArray *ary = new RealArray(len);
      m_value.arrayValue = static_cast<Array *>(ary);
      for (size_t i = 0; i < len; ++i) {
        double temp;
        if (vals[i].getValue(temp))
          ary->setElement(i, temp);
        else
          ary->setElementUnknown(i);
      }
      break;
    }

    case STRING_TYPE: {
      m_type = STRING_ARRAY_TYPE;
      StringArray *ary = new StringArray(len);
      m_value.arrayValue = static_cast<Array *>(ary);
      for (size_t i = 0; i < len; ++i) {
        std::string const *temp;
        if (vals[i].getValuePointer(temp))
          ary->setElement(i, *temp);
        else
          ary->setElementUnknown(i);
      }
      break;
    }

    default:
      assertTrue_2(ALWAYS_FAIL, "Value constructor: Unknown or unimplemented element type");
    }
  }
    
  Value::~Value()
  {
    cleanup();
  }
    
  Value &Value::operator=(Value const &other)
  {
    if (this == &other)
      return *this; // assigning to self, nothing to do

    cleanup();
    m_type = other.m_type;
    m_known = other.m_known;
    if (m_known) {
      switch (m_type) {
        // Unknown - do nothing
      case UNKNOWN_TYPE:
        break;

        // Immediate data - just copy the union
      case BOOLEAN_TYPE:
      case INTEGER_TYPE:
      case REAL_TYPE:
      case NODE_STATE_TYPE:
      case OUTCOME_TYPE:
      case FAILURE_TYPE:
      case COMMAND_HANDLE_TYPE:
        m_value = other.m_value;
        break;

        // Copy the actual value
      case STRING_TYPE:
        m_value.stringValue = new std::string(*other.m_value.stringValue);
        break;

      case BOOLEAN_ARRAY_TYPE:
      case INTEGER_ARRAY_TYPE:
      case REAL_ARRAY_TYPE:
      case STRING_ARRAY_TYPE:
        m_value.arrayValue = other.m_value.arrayValue->clone();
        break;

      default:
        assertTrue_2(ALWAYS_FAIL, "Value copy constructor: unknown type");
        break;
      }
    }
    return *this;
  }

  Value &Value::operator=(bool val)
  {
    cleanup();
    m_value.booleanValue = val;
    m_type = BOOLEAN_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(uint16_t enumVal)
  {
    cleanup();
    m_value.enumValue = enumVal;
    // *** FIXME: Have to determine type ***
    // For now assume command handle
    m_type = COMMAND_HANDLE_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(int32_t val)
  {
    cleanup();
    m_value.integerValue = val;
    m_type = INTEGER_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(double val)
  {
    cleanup();
    m_value.realValue = val;
    m_type = REAL_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(std::string const &val)
  {
    cleanup();
    m_value.stringValue = new std::string(val);
    m_type = STRING_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(char const *val)
  {
    cleanup();
    m_value.stringValue = new std::string(val);
    m_type = STRING_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(BooleanArray const &val)
  {
    cleanup();
    m_value.arrayValue = val.clone();
    m_type = BOOLEAN_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(IntegerArray const &val)
  {
    cleanup();
    m_value.arrayValue = val.clone();
    m_type = INTEGER_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(RealArray const &val)
  {
    cleanup();
    m_value.arrayValue = val.clone();
    m_type = REAL_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(StringArray const &val)
  {
    cleanup();
    m_value.arrayValue = val.clone();
    m_type = STRING_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  void Value::setUnknown()
  {
    m_known = false;
    cleanup();
  }

  // Do whatever is necessary to delete the previous contents
  void Value::cleanup()
  {
    if (!m_known)
      return;

    switch (m_type) {
    case STRING_TYPE:
      delete m_value.stringValue;
      m_value.stringValue = NULL;
      break;
      
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      delete m_value.arrayValue;
      m_value.arrayValue = NULL;
      break;

    default:
      break;
    }
  }

  ValueType Value::valueType() const
  {
    return m_type;
  }

  bool Value::isKnown() const
  {
    return m_known;
  }

  bool Value::getValue(bool &result) const
  {
    if (!m_known)
      return false;
    if (m_type != BOOLEAN_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: type error");
      return false;
    }
    result = m_value.booleanValue;
    return true;
  }

  bool Value::getValue(uint16_t &result) const
  {
    if (!m_known)
      return false;

    switch (m_type) {
    case NODE_STATE_TYPE:
    case OUTCOME_TYPE:
    case FAILURE_TYPE:
    case COMMAND_HANDLE_TYPE:
      result = m_value.enumValue;
      return true;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: type error");
      return false;
    }
  }

  bool Value::getValue(int32_t &result) const
  {
    if (!m_known)
      return false;
    if (m_type != INTEGER_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: type error");
      return false;
    }
    result = m_value.integerValue;
    return true;
  }

  bool Value::getValue(double &result) const
  {
    if (!m_known)
      return false;
    switch (m_type) {
    case REAL_TYPE:
      result = m_value.realValue;
      return true;

    case INTEGER_TYPE:
      result = (double) m_value.integerValue;
      return true;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: type error");
      return false;
    }
  }

  bool Value::getValue(std::string &result) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: type error");
      return false;
    }
    result = *m_value.stringValue;
    return true;
  }

  bool Value::getValuePointer(std::string const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
    ptr = m_value.stringValue;
    return true;
  }

  bool Value::getValuePointer(Array const *&ptr) const
  {
    if (!m_known)
      return false;
    switch (m_type) {
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      ptr = m_value.arrayValue;
      return true;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
  }

  bool Value::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != BOOLEAN_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
    ptr = dynamic_cast<BooleanArray const *>(m_value.arrayValue);
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != INTEGER_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
    ptr = dynamic_cast<IntegerArray const *>(m_value.arrayValue);
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(RealArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != REAL_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
    ptr = dynamic_cast<RealArray const *>(m_value.arrayValue);
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(StringArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: type error");
      return false;
    }
    ptr = dynamic_cast<StringArray const *>(m_value.arrayValue);
    assertTrue_1(ptr);
    return true;
  }

  void Value::print(std::ostream &s) const
  {
    if (!m_known) {
      s << "[unknown_value]"; 
      return;
    }
    switch (m_type) {
    case BOOLEAN_TYPE:
      printValue<bool>(m_value.booleanValue, s);
      break;

    case INTEGER_TYPE:
      printValue<int32_t>(m_value.integerValue, s);
      break;

    case REAL_TYPE:
      printValue<double>(m_value.realValue, s);
      break;

    case STRING_TYPE:
      printValue<std::string>(*m_value.stringValue, s);
      break;

    case BOOLEAN_ARRAY_TYPE:
      printValue<bool>(*dynamic_cast<BooleanArray const *>(m_value.arrayValue), s);
      break;

    case INTEGER_ARRAY_TYPE:
      printValue<int32_t>(*dynamic_cast<IntegerArray const *>(m_value.arrayValue), s);
      break;

    case REAL_ARRAY_TYPE:
      printValue<double>(*dynamic_cast<RealArray const *>(m_value.arrayValue), s);
      break;

    case STRING_ARRAY_TYPE:
      printValue<std::string>(*dynamic_cast<StringArray const *>(m_value.arrayValue), s);
      break;

    case NODE_STATE_TYPE:
      s << nodeStateName(m_value.enumValue);
      break;

    case OUTCOME_TYPE:
      s << outcomeName(m_value.enumValue);
      break;

    case FAILURE_TYPE:
      s << failureTypeName(m_value.enumValue);
      break;

    case COMMAND_HANDLE_TYPE:
      s << commandHandleValueName(m_value.enumValue);
      break;

    default:
      s << "[invalid_type]";
      break;
    }
  }

  std::ostream &operator<<(std::ostream &s, Value const &v)
  {
    v.print(s);
    return s;
  }

  std::string Value::valueToString() const
  {
    std::ostringstream s;
    print(s);
    return s.str();
  }

  // Issues:
  // - is unknown always equal to unknown?
  bool Value::equals(Value const &other) const
  {
    if (m_known != other.m_known)
      return false; // known != unknown, always
    switch (m_type) {
      
    case INTEGER_TYPE:
      if (other.m_type == m_type) {
        if (!m_known)
          return true;
        return m_value.integerValue == other.m_value.integerValue;
      }
      else if (other.m_type == REAL_TYPE) {
        if (!m_known)
          return true;
        return other.m_value.realValue == (double) m_value.integerValue;
      }
      else
        return false; // type mismatch
      
    case REAL_TYPE:
      if (other.m_type == m_type) {
        if (!m_known)
          return true;
        return m_value.realValue == other.m_value.realValue;
      }
      else if (other.m_type == INTEGER_TYPE) {
        if (!m_known)
          return true;
        return m_value.realValue == (double) other.m_value.integerValue;
      }
      else
        return false; // type mismatch

    default: 
      if (other.m_type != m_type)
        return false;
      if (!m_known)
        return true;
      switch (m_type) {
      case BOOLEAN_TYPE:
        return m_value.booleanValue == other.m_value.booleanValue;

      case NODE_STATE_TYPE:
      case OUTCOME_TYPE:
      case FAILURE_TYPE:
      case COMMAND_HANDLE_TYPE:
        return m_value.enumValue == other.m_value.enumValue;
      
      case STRING_TYPE:
        return *m_value.stringValue == *other.m_value.stringValue;

      case BOOLEAN_ARRAY_TYPE:
      case INTEGER_ARRAY_TYPE:
      case REAL_ARRAY_TYPE:
      case STRING_ARRAY_TYPE:
        return *m_value.arrayValue == *other.m_value.arrayValue;

      default:
        assertTrue_2(ALWAYS_FAIL, "Value::equals: unknown value type");
        return false;
      }
    }
  }

  bool Value::lessThan(Value const &other) const
  {
    // unknown < known
    if (!m_known && other.m_known)
      return true;
    if (m_known && !other.m_known)
      return false;

    switch (m_type) {
    case INTEGER_TYPE:
      if (m_type == other.m_type) {
        if (m_known)
          return m_value.integerValue < other.m_value.integerValue;
        else 
          return false; // unknown integer values are equal
      }
      else if (REAL_TYPE == other.m_type) {
        if (m_known)
          return ((double) m_value.integerValue) < other.m_value.realValue;
        else 
          return true; // real unknown > int unknown
      }
      else 
        return m_type < other.m_type;

    case REAL_TYPE:
      if (m_type == other.m_type) {
        if (m_known)
          return m_value.realValue < other.m_value.realValue;
        else
          return false; // unknown real values are equal
      }
      else if (INTEGER_TYPE == other.m_type) {
        if (m_known)
          return m_value.realValue < (double) other.m_value.integerValue;
        else
          return false; // real unknown > int unknown
      }
      else 
        return m_type < other.m_type;

    default:
      // Unequal types 
      if (m_type < other.m_type)
        return true;
      else if (m_type > other.m_type)
        return false;
    }

    // Types are equal
    if (!m_known)
      return false; // unknowns of same type are equal

    switch (m_type) {
      case BOOLEAN_TYPE:
        return ((int) m_value.booleanValue) < ((int) other.m_value.booleanValue);

      case NODE_STATE_TYPE:
      case OUTCOME_TYPE:
      case FAILURE_TYPE:
      case COMMAND_HANDLE_TYPE:
        return m_value.enumValue < other.m_value.enumValue;
      
      case STRING_TYPE:
        return *m_value.stringValue < *other.m_value.stringValue;

      case BOOLEAN_ARRAY_TYPE:
        return 
          *dynamic_cast<BooleanArray const *>(m_value.arrayValue) < 
          *dynamic_cast<BooleanArray const *>(other.m_value.arrayValue);

      case INTEGER_ARRAY_TYPE:
        return 
          *dynamic_cast<IntegerArray const *>(m_value.arrayValue) < 
          *dynamic_cast<IntegerArray const *>(other.m_value.arrayValue);

      case REAL_ARRAY_TYPE:
        return 
          *dynamic_cast<RealArray const *>(m_value.arrayValue) < 
          *dynamic_cast<RealArray const *>(other.m_value.arrayValue);

      case STRING_ARRAY_TYPE:
        return 
          *dynamic_cast<StringArray const *>(m_value.arrayValue) < 
          *dynamic_cast<StringArray const *>(other.m_value.arrayValue);

      default:
        assertTrue_2(ALWAYS_FAIL, "Value::lessThan: unknown value type");
        return false;
      }
  }

} // namespace PLEXIL
