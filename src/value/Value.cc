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

#include "Value.hh"

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "Error.hh"
#include "NodeConstants.hh"

namespace PLEXIL
{

  //
  // Constructors
  //

  Value::Value()
    : realValue(0.0),
      m_type(UNKNOWN_TYPE),
      m_known(false)
  {}

  Value::Value(Value const &other)
    : realValue(0.0),
      m_type(other.m_type),
      m_known(other.m_known)
  {
    if (!m_known)
      return;
    switch (m_type) {
      // Copy the original's value
    case BOOLEAN_TYPE:
      booleanValue = other.booleanValue;
      break;

    case INTEGER_TYPE:
      integerValue = other.integerValue;
      break;

    case REAL_TYPE:
      realValue = other.realValue;
      break;

    case NODE_STATE_TYPE:
      stateValue = other.stateValue;
      break;
      
    case OUTCOME_TYPE:
      outcomeValue = other.outcomeValue;
      break;

    case FAILURE_TYPE:
      failureValue = other.failureValue;
      break;
      
    case COMMAND_HANDLE_TYPE:
      commandHandleValue = other.commandHandleValue;
      break;

    case STRING_TYPE:
      new (&stringValue) std::unique_ptr<String>(new std::string(*other.stringValue));
      break;

    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      new (&arrayValue) std::unique_ptr<Array>(other.arrayValue->clone());
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value copy constructor: invalid or unknown type");
      break;
    }
  }

  Value::Value(Value &&other)
    : realValue(0.0),
      m_type(other.m_type),
      m_known(other.m_known)
  {
    if (!m_known)
      return;

    switch (m_type) {
      // Immediate data - copy it
    case BOOLEAN_TYPE:
      booleanValue = other.booleanValue;
      break;

    case INTEGER_TYPE:
      integerValue = other.integerValue;
      break;

    case REAL_TYPE:
      realValue = other.realValue;
      break;

    case NODE_STATE_TYPE:
      stateValue = other.stateValue;
      break;

    case OUTCOME_TYPE:
      outcomeValue = other.outcomeValue;
      break;

    case FAILURE_TYPE:
      failureValue = other.failureValue;
      break;
      
    case COMMAND_HANDLE_TYPE:
      commandHandleValue = other.commandHandleValue;
      break;

      // Pointer data - move it
    case STRING_TYPE:
      new (&stringValue) std::unique_ptr<String>(std::move(other.stringValue));
      break;

      // Copy the entire array
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      new (&arrayValue) std::unique_ptr<Array>(std::move(other.arrayValue));
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value move constructor: unknown type");
      break;
    }
  }

  Value::Value(bool val)
    : booleanValue(val),
      m_type(BOOLEAN_TYPE),
      m_known(true)
  {
  }

  Value::Value(NodeState val)
    : stateValue(val),
      m_type(NODE_STATE_TYPE),
      m_known(true)
  {
  }

  Value::Value(NodeOutcome val)
    : outcomeValue(val),
      m_type(OUTCOME_TYPE),
      m_known(true)
  {
  }

  Value::Value(FailureType val)
    : failureValue(val),
      m_type(FAILURE_TYPE),
      m_known(true)
  {
  }

  Value::Value(CommandHandleValue val)
    : commandHandleValue(val),
      m_type(COMMAND_HANDLE_TYPE),
      m_known(true)
  {
  }

  Value::Value(uint8_t enumVal, ValueType typ)
    : realValue(0.0), // don't know what type we are yet
      m_type(typ),
      m_known(enumVal != 0)
  {
    if (enumVal == 0) {
      m_known = false;
      switch (m_type) {
        case STRING_TYPE:
          new (&stringValue) std::unique_ptr<String>();
          break;

        case BOOLEAN_ARRAY_TYPE:
        case INTEGER_ARRAY_TYPE:
        case REAL_ARRAY_TYPE:
        case STRING_ARRAY_TYPE:
          new (&arrayValue) std::unique_ptr<Array>();
          break;

        default:
          break;
        }
      return;
    }

    // Enum val of some sort
    switch (m_type) {
      // Internal enumerations
    case NODE_STATE_TYPE:
      stateValue = (NodeState) enumVal;
      return;
      
    case OUTCOME_TYPE:
      outcomeValue = (NodeOutcome) enumVal;
      return;

    case FAILURE_TYPE:
      failureValue = (FailureType) enumVal;
      return;

    case COMMAND_HANDLE_TYPE:
      commandHandleValue = (CommandHandleValue) enumVal;
      return;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value constructor: illegal value for type");
      return;
    }
  }
      
  Value::Value(int32_t val)
    : integerValue(val),
      m_type(INTEGER_TYPE),
      m_known(true)
  {
  }

  Value::Value(double val)
    : realValue(val),
      m_type(REAL_TYPE),
      m_known(true)
  {
  }

  Value::Value(std::string const &val)
    : stringValue(new std::string(val)),
      m_type(STRING_TYPE),
      m_known(true)
  {
  }

  Value::Value(char const *val)
    : stringValue(new std::string(val)),
      m_type(STRING_TYPE),
      m_known(true)
  {
  }

  Value::Value(BooleanArray const &val)
    : arrayValue(val.clone()),
      m_type(BOOLEAN_ARRAY_TYPE),
      m_known(true)
  {
    
  }

  Value::Value(IntegerArray const &val)
    : arrayValue(val.clone()),
      m_type(INTEGER_ARRAY_TYPE),
      m_known(true)
  {
  }

  Value::Value(RealArray const &val)
    : arrayValue(val.clone()),
      m_type(REAL_ARRAY_TYPE),
      m_known(true)
  {
    
  }

  Value::Value(StringArray const &val)
    : arrayValue(val.clone()),
      m_type(STRING_ARRAY_TYPE),
      m_known(true)
  {
  }

  Value::Value(std::vector<Value> const &vals)
    : arrayValue(), // we can be sure result is an array
      m_type(UNKNOWN_TYPE),
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
      arrayValue.reset(static_cast<Array *>(ary));
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
      arrayValue.reset(static_cast<Array *>(ary));
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
      arrayValue.reset(static_cast<Array *>(ary));
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
      arrayValue.reset(static_cast<Array *>(ary));
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

  //
  // Destructor
  //
    
  Value::~Value()
  {
    cleanup();
  }

  //
  // Assignment
  //

  // Copy assignment
  Value &Value::operator=(Value const &other)
  {
    if (this == &other)
      return *this; // assigning to self, nothing to do

    if (!other.m_known) {
      cleanup();
      m_type = other.m_type;
      return *this;
    }

    switch (other.m_type) {
      // Copy the original's value
    case BOOLEAN_TYPE:
      cleanup();
      booleanValue = other.booleanValue;
      break;

    case INTEGER_TYPE:
      cleanup();
      integerValue = other.integerValue;
      break;

    case REAL_TYPE:
      cleanup();
      realValue = other.realValue;
      break;

    case NODE_STATE_TYPE:
      cleanup();
      stateValue = other.stateValue;
      break;

    case OUTCOME_TYPE:
      cleanup();
      outcomeValue = other.outcomeValue;
      break;

    case FAILURE_TYPE:
      cleanup();
      failureValue = other.failureValue;
      break;

    case COMMAND_HANDLE_TYPE:
      cleanup();
      commandHandleValue = other.commandHandleValue;
      break;

    case STRING_TYPE:
      cleanupForString();
      stringValue.reset(new std::string(*other.stringValue));
      break;

    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      cleanupForArray();
      arrayValue.reset(other.arrayValue->clone());
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value copy assignment: invalid or unknown type");
      break;
    }
    m_known = true;
    m_type = other.m_type;
    return *this;
  }

  // Move assignment
  Value &Value::operator=(Value &&other)
  {
    if (this == &other)
      return *this; // assigning to self, nothing to do

    if (!other.m_known) {
      cleanup();
      m_type = other.m_type;
      return *this;
    }

    switch (other.m_type) {
      // Immediate data - copy it
    case BOOLEAN_TYPE:
      cleanup();
      booleanValue = other.booleanValue;
      break;

    case INTEGER_TYPE:
      cleanup();
      integerValue = other.integerValue;
      break;

    case REAL_TYPE:
      cleanup();
      realValue = other.realValue;
      break;

    case NODE_STATE_TYPE:
      cleanup();
      stateValue = other.stateValue;
      break;

    case OUTCOME_TYPE:
      cleanup();
      outcomeValue = other.outcomeValue;
      break;

    case FAILURE_TYPE:
      cleanup();
      failureValue = other.failureValue;
      break;

    case COMMAND_HANDLE_TYPE:
      cleanup();
      commandHandleValue = other.commandHandleValue;
      break;

      // Pointer data - move it
    case STRING_TYPE:
      cleanupForString();
      stringValue = std::move(other.stringValue);
      break;

      // Copy the entire array
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      cleanupForArray();
      arrayValue = std::move(other.arrayValue);
      break;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value move assignment: invalid or unknown type");
      break;
    }
    m_known = true;
    m_type = other.m_type;

    return *this;
  }

  Value &Value::operator=(bool val)
  {
    cleanup();
    booleanValue = val;
    m_type = BOOLEAN_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(NodeState val)
  {
    cleanup();
    stateValue = val;
    m_type = NODE_STATE_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(NodeOutcome val)
  {
    cleanup();
    outcomeValue = val;
    m_type = OUTCOME_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(FailureType val)
  {
    cleanup();
    failureValue = val;
    m_type = FAILURE_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(CommandHandleValue val)
  {
    cleanup();
    commandHandleValue = val;
    m_type = COMMAND_HANDLE_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(int32_t val)
  {
    cleanup();
    integerValue = val;
    m_type = INTEGER_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(double val)
  {
    cleanup();
    realValue = val;
    m_type = REAL_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(std::string const &val)
  {
    cleanupForString();
    stringValue.reset(new std::string(val));
    m_type = STRING_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(char const *val)
  {
    cleanupForString();
    stringValue.reset(new std::string(val));
    m_type = STRING_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(BooleanArray const &val)
  {
    cleanupForArray();
    arrayValue.reset(val.clone());
    m_type = BOOLEAN_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(IntegerArray const &val)
  {
    cleanupForArray();
    arrayValue.reset(val.clone());
    m_type = INTEGER_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(RealArray const &val)
  {
    cleanupForArray();
    arrayValue.reset(val.clone());
    m_type = REAL_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(StringArray const &val)
  {
    cleanupForArray();
    arrayValue.reset(val.clone());
    m_type = STRING_ARRAY_TYPE;
    m_known = true;
    return *this;
  }

  void Value::setUnknown()
  {
    cleanup();
  }

  // Do whatever is necessary to delete the previous contents
  void Value::cleanup()
  {
    switch (m_type) {
    case STRING_TYPE:
      stringValue.reset();
      break;
      
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      arrayValue.reset();
      break;

    default:
      realValue = 0;
      break;
    }
    m_known = false;
    m_type = UNKNOWN_TYPE;
  }

  void Value::cleanupForString()
  {
    switch (m_type) {
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      // Release the old value
      arrayValue.reset();
      // and fall through...

    default:
      // Initialize the string pointer
      new (&stringValue) std::unique_ptr<String>();
      break;

    case STRING_TYPE:
      // do nothing
      break;
    }
  }

  void Value::cleanupForArray()
  {
    switch (m_type) {
    case STRING_TYPE:
      // Release old value
      stringValue.reset();
      // and fall through...

    default:
      // Initialize the array pointer
      new (&arrayValue) std::unique_ptr<Array>();
      break;
      
    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      break;

    }
  }

  //
  // Accessors
  //

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
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a Boolean value");
      return false;
    }
    result = booleanValue;
    return true;
  }

  bool Value::getValue(NodeState &result) const
  {
    if (!m_known)
      return false;
    if (m_type != NODE_STATE_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a NodeState value");
      return false;
    }

    result = stateValue;
    return true;
  }

  bool Value::getValue(NodeOutcome &result) const
  {
    if (!m_known)
      return false;
    if (m_type != OUTCOME_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a NodeOutcome value");
      return false;
    }

    result = outcomeValue;
    return true;
  }

  bool Value::getValue(FailureType &result) const
  {
    if (!m_known)
      return false;
    if (m_type != FAILURE_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a FailureType value");
      return false;
    }

    result = failureValue;
    return true;
  }

  bool Value::getValue(CommandHandleValue &result) const
  {
    if (!m_known)
      return false;
    if (m_type != COMMAND_HANDLE_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a CommandHandle value");
      return false;
    }

    result = commandHandleValue;
    return true;
  }

  bool Value::getValue(int32_t &result) const
  {
    if (!m_known)
      return false;
    if (m_type != INTEGER_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not an Integer value");
      return false;
    }
    result = integerValue;
    return true;
  }

  bool Value::getValue(double &result) const
  {
    if (!m_known)
      return false;
    switch (m_type) {
    case REAL_TYPE:
      result = realValue;
      return true;

    case INTEGER_TYPE:
      result = (double) integerValue;
      return true;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a Real or Integer value");
      return false;
    }
  }

  bool Value::getValue(std::string &result) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValue: not a String value");
      return false;
    }
    result = *stringValue;
    return true;
  }

  bool Value::getValuePointer(std::string const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not a String value");
      return false;
    }
    ptr = stringValue.get();
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
      ptr = arrayValue.get();
      return true;

    default:
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not an Array value");
      return false;
    }
  }

  bool Value::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != BOOLEAN_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not a BooleanArray value");
      return false;
    }
    ptr = dynamic_cast<BooleanArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != INTEGER_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not an IntegerArray value");
      return false;
    }
    ptr = dynamic_cast<IntegerArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(RealArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != REAL_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not a RealArray value");
      return false;
    }
    ptr = dynamic_cast<RealArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(StringArray const *&ptr) const
  {
    if (!m_known)
      return false;
    if (m_type != STRING_ARRAY_TYPE) {
      assertTrue_2(ALWAYS_FAIL, "Value::getValuePointer: not a StringArray value");
      return false;
    }
    ptr = dynamic_cast<StringArray const *>(arrayValue.get());
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
      printValue<bool>(booleanValue, s);
      break;

    case INTEGER_TYPE:
      printValue<int32_t>(integerValue, s);
      break;

    case REAL_TYPE:
      printValue<double>(realValue, s);
      break;

    case STRING_TYPE:
      printValue<std::string>(*stringValue, s);
      break;

    case BOOLEAN_ARRAY_TYPE:
      printValue<bool>(*dynamic_cast<BooleanArray const *>(arrayValue.get()), s);
      break;

    case INTEGER_ARRAY_TYPE:
      printValue<int32_t>(*dynamic_cast<IntegerArray const *>(arrayValue.get()), s);
      break;

    case REAL_ARRAY_TYPE:
      printValue<double>(*dynamic_cast<RealArray const *>(arrayValue.get()), s);
      break;

    case STRING_ARRAY_TYPE:
      printValue<std::string>(*dynamic_cast<StringArray const *>(arrayValue.get()), s);
      break;

    case NODE_STATE_TYPE:
      printValue<NodeState>(stateValue, s);
      break;

    case OUTCOME_TYPE:
      printValue<NodeOutcome>(outcomeValue, s);
      break;

    case FAILURE_TYPE:
      printValue<FailureType>(failureValue, s);
      break;

    case COMMAND_HANDLE_TYPE:
      printValue<CommandHandleValue>(commandHandleValue, s);
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
        return integerValue == other.integerValue;
      }
      else if (other.m_type == REAL_TYPE) {
        if (!m_known)
          return true;
        return other.realValue == (double) integerValue;
      }
      else
        return false; // type mismatch
      
    case REAL_TYPE:
      if (other.m_type == m_type) {
        if (!m_known)
          return true;
        return realValue == other.realValue;
      }
      else if (other.m_type == INTEGER_TYPE) {
        if (!m_known)
          return true;
        return realValue == (double) other.integerValue;
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
        return booleanValue == other.booleanValue;

      case NODE_STATE_TYPE:
        return stateValue == other.stateValue;

      case OUTCOME_TYPE:
        return outcomeValue == other.outcomeValue;

      case FAILURE_TYPE:
        return failureValue == other.failureValue;
        
      case COMMAND_HANDLE_TYPE:
        return commandHandleValue == other.commandHandleValue;
      
      case STRING_TYPE:
        return *stringValue == *other.stringValue;

      case BOOLEAN_ARRAY_TYPE:
      case INTEGER_ARRAY_TYPE:
      case REAL_ARRAY_TYPE:
      case STRING_ARRAY_TYPE:
        return *arrayValue == *other.arrayValue;

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
          return integerValue < other.integerValue;
        else 
          return false; // unknown integer values are equal
      }
      else if (REAL_TYPE == other.m_type) {
        if (m_known)
          return ((double) integerValue) < other.realValue;
        else 
          return true; // real unknown > int unknown
      }
      else 
        return m_type < other.m_type;

    case REAL_TYPE:
      if (m_type == other.m_type) {
        if (m_known)
          return realValue < other.realValue;
        else
          return false; // unknown real values are equal
      }
      else if (INTEGER_TYPE == other.m_type) {
        if (m_known)
          return realValue < (double) other.integerValue;
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
        return ((int) booleanValue) < ((int) other.booleanValue);

      case NODE_STATE_TYPE:
        return stateValue < other.stateValue;

      case OUTCOME_TYPE:
        return outcomeValue < other.outcomeValue;

      case FAILURE_TYPE:
        return failureValue < other.failureValue;
        
      case COMMAND_HANDLE_TYPE:
        return commandHandleValue < other.commandHandleValue;
      
      case STRING_TYPE:
        return *stringValue < *other.stringValue;

      case BOOLEAN_ARRAY_TYPE:
        return 
                              *dynamic_cast<BooleanArray const *>(arrayValue.get()) < 
        *dynamic_cast<BooleanArray const *>(other.arrayValue.get());

      case INTEGER_ARRAY_TYPE:
        return 
        *dynamic_cast<IntegerArray const *>(arrayValue.get()) < 
        *dynamic_cast<IntegerArray const *>(other.arrayValue.get());

      case REAL_ARRAY_TYPE:
        return 
        *dynamic_cast<RealArray const *>(arrayValue.get()) < 
        *dynamic_cast<RealArray const *>(other.arrayValue.get());

      case STRING_ARRAY_TYPE:
        return 
        *dynamic_cast<StringArray const *>(arrayValue.get()) < 
        *dynamic_cast<StringArray const *>(other.arrayValue.get());

      default:
        assertTrue_2(ALWAYS_FAIL, "Value::lessThan: unknown value type");
        return false;
      }
  }

} // namespace PLEXIL
