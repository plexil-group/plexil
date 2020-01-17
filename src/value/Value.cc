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

#include "Value.hh"

#include "ArrayImpl.hh"
#include "CommandHandle.hh"
#include "NodeConstants.hh"
#include "PlanError.hh"

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
      new (&stringValue) std::unique_ptr<String>(new String(*other.stringValue));
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

  Value::Value(Boolean val)
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
      
  Value::Value(Integer val)
    : integerValue(val),
      m_type(INTEGER_TYPE),
      m_known(true)
  {
  }

  Value::Value(Real val)
    : realValue(val),
      m_type(REAL_TYPE),
      m_known(true)
  {
  }

  Value::Value(String const &val)
    : stringValue(new String(val)),
      m_type(STRING_TYPE),
      m_known(true)
  {
  }

  Value::Value(char const *val)
    : stringValue(new String(val)),
      m_type(STRING_TYPE),
      m_known(true)
  {
  }

  Value::Value(Array const &val)
    : arrayValue(val.clone()),
      m_type(arrayType(val.getElementType())),
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
        checkPlanError(ALWAYS_FAIL,
                       "Value constructor: Inconsistent value types in vector");
        m_known = false;
      }
      // else type is consistent
    }

    checkPlanError(eltType != UNKNOWN_TYPE,
                   "Value constructor: Can't make array of all unknowns");
    checkPlanError(eltType < SCALAR_TYPE_MAX,
                   "Value constructor: Can't make array of arrays");

    // Construct array value
    switch (eltType) {
    case BOOLEAN_TYPE: {
      m_type = BOOLEAN_ARRAY_TYPE;
      BooleanArray *ary = new BooleanArray(len);
      arrayValue.reset(static_cast<Array *>(ary));
      for (size_t i = 0; i < len; ++i) {
        Boolean temp;
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
        Integer temp;
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
        Real temp;
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
        String const *temp;
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
      stringValue.reset(new String(*other.stringValue));
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

  Value &Value::operator=(Boolean val)
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

  Value &Value::operator=(Integer val)
  {
    cleanup();
    integerValue = val;
    m_type = INTEGER_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(Real val)
  {
    cleanup();
    realValue = val;
    m_type = REAL_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(String const &val)
  {
    cleanupForString();
    stringValue.reset(new String(val));
    m_type = STRING_TYPE;
    m_known = true;
    return *this;
  }

  Value &Value::operator=(char const *val)
  {
    cleanupForString();
    stringValue.reset(new String(val));
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

  bool Value::getValue(Boolean &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == BOOLEAN_TYPE,
                   "Attempt to get Boolean value of a "
                   << valueTypeName(m_type) << " Value");
    result = booleanValue;
    return true;
  }

  bool Value::getValue(NodeState &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == NODE_STATE_TYPE,
                   "Attempt to get a NodeState value from a "
                   << valueTypeName(m_type) << " Value");
    result = stateValue;
    return true;
  }

  bool Value::getValue(NodeOutcome &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == OUTCOME_TYPE,
                   "Attempt to get a NodeOutcome value from a "
                   << valueTypeName(m_type) << " Value");
    result = outcomeValue;
    return true;
  }

  bool Value::getValue(FailureType &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == FAILURE_TYPE,
                   "Attempt to get a FailureType value from a "
                   << valueTypeName(m_type) << " Value");
    result = failureValue;
    return true;
  }

  bool Value::getValue(CommandHandleValue &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == COMMAND_HANDLE_TYPE,
                   "Attempt to get a CommandHandle value from a "
                   << valueTypeName(m_type) << " Value");
    result = commandHandleValue;
    return true;
  }

  bool Value::getValue(Integer &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == INTEGER_TYPE,
                   "Attempt to get an Integer value from a "
                   << valueTypeName(m_type) << " Value");
    result = integerValue;
    return true;
  }

  bool Value::getValue(Real &result) const
  {
    if (!m_known)
      return false;
    switch (m_type) {
    case REAL_TYPE:
      result = realValue;
      return true;

    case INTEGER_TYPE:
      result = (Real) integerValue;
      return true;

    default:
      checkPlanError(ALWAYS_FAIL,
                     "Attempt to get a Real value from a "
                     << valueTypeName(m_type) << " Value");
      return false;
    }
  }

  bool Value::getValue(String &result) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == STRING_TYPE,
                   "Attempt to get a String value from a "
                   << valueTypeName(m_type) << " Value");
    result = *stringValue;
    return true;
  }

  bool Value::getValuePointer(String const *&ptr) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == STRING_TYPE,
                   "Attempt to get a String value from a "
                   << valueTypeName(m_type) << " Value");
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
      checkPlanError(ALWAYS_FAIL,
                     "Attempt to get an Array value from a "
                     << valueTypeName(m_type) << " Value");
      return false;
    }
  }

  bool Value::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == BOOLEAN_ARRAY_TYPE,
                   "Attempt to get a BooleanArray value from a "
                   << valueTypeName(m_type) << " Value");
    ptr = dynamic_cast<BooleanArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == INTEGER_ARRAY_TYPE,
                   "Attempt to get a IntegerArray value from a "
                   << valueTypeName(m_type) << " Value");
    ptr = dynamic_cast<IntegerArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(RealArray const *&ptr) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == REAL_ARRAY_TYPE,
                   "Attempt to get a RealArray value from a "
                   << valueTypeName(m_type) << " Value");
    ptr = dynamic_cast<RealArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  bool Value::getValuePointer(StringArray const *&ptr) const
  {
    if (!m_known)
      return false;
    checkPlanError(m_type == STRING_ARRAY_TYPE,
                   "Attempt to get a StringArray value from a "
                   << valueTypeName(m_type) << " Value");
    ptr = dynamic_cast<StringArray const *>(arrayValue.get());
    assertTrue_1(ptr);
    return true;
  }

  Value Value::toValue() const
  {
    return Value(*this);
  }

  void Value::printValue(std::ostream &s) const
  {
    if (!m_known) {
      s << "[unknown_value]"; 
      return;
    }
    switch (m_type) {
    case BOOLEAN_TYPE:
      PLEXIL::printValue<Boolean>(booleanValue, s);
      break;

    case INTEGER_TYPE:
      PLEXIL::printValue<Integer>(integerValue, s);
      break;

    case REAL_TYPE:
      PLEXIL::printValue<Real>(realValue, s);
      break;

    case STRING_TYPE:
      PLEXIL::printValue<String>(*stringValue, s);
      break;

    case BOOLEAN_ARRAY_TYPE:
      PLEXIL::printValue<BooleanArray>(*dynamic_cast<BooleanArray const *>(arrayValue.get()), s);
      break;

    case INTEGER_ARRAY_TYPE:
      PLEXIL::printValue<IntegerArray>(*dynamic_cast<IntegerArray const *>(arrayValue.get()), s);
      break;

    case REAL_ARRAY_TYPE:
      PLEXIL::printValue<RealArray>(*dynamic_cast<RealArray const *>(arrayValue.get()), s);
      break;

    case STRING_ARRAY_TYPE:
      PLEXIL::printValue<StringArray>(*dynamic_cast<StringArray const *>(arrayValue.get()), s);
      break;

    case NODE_STATE_TYPE:
      PLEXIL::printValue<NodeState>(stateValue, s);
      break;

    case OUTCOME_TYPE:
      PLEXIL::printValue<NodeOutcome>(outcomeValue, s);
      break;

    case FAILURE_TYPE:
      PLEXIL::printValue<FailureType>(failureValue, s);
      break;

    case COMMAND_HANDLE_TYPE:
      PLEXIL::printValue<CommandHandleValue>(commandHandleValue, s);
      break;

    default:
      s << "[invalid_type]";
      break;
    }
  }

  std::ostream &operator<<(std::ostream &s, Value const &v)
  {
    v.printValue(s);
    return s;
  }

  std::string Value::valueToString() const
  {
    std::ostringstream s;
    printValue(s);
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
        return other.realValue == (Real) integerValue;
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
        return realValue == (Real) other.integerValue;
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
          return ((Real) integerValue) < other.realValue;
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
          return realValue < (Real) other.integerValue;
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

  char *Value::serialize(char *b) const
  {
    if (!m_known) {
      *b++ = UNKNOWN_TYPE;
      return b;
    }
    switch (m_type) {
    case BOOLEAN_TYPE:
      return PLEXIL::serialize(booleanValue, b);

    case INTEGER_TYPE:
      return PLEXIL::serialize(integerValue, b);
      
    case REAL_TYPE:
      return PLEXIL::serialize(realValue, b);

    case STRING_TYPE:
      return PLEXIL::serialize(*stringValue, b);

    case COMMAND_HANDLE_TYPE:
      return PLEXIL::serialize(commandHandleValue, b);

    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      return PLEXIL::serialize(*arrayValue, b);

    default: // invalid/unimplemented
      return NULL;
    }
  }
  
  char const *Value::deserialize(char const *b)
  {
    ValueType t = (ValueType) *b;
    if (t != m_type)
      cleanup();

    switch (t) {
    case UNKNOWN_TYPE:
      setUnknown();
      return ++b;

    case BOOLEAN_TYPE:
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize(booleanValue, b);

    case INTEGER_TYPE:
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize(integerValue, b);

    case REAL_TYPE:
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize(realValue, b);

    case STRING_TYPE:
      if (m_type != STRING_TYPE || !stringValue)
        stringValue.reset(new String());
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize(*stringValue, b);

    case COMMAND_HANDLE_TYPE:
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize(commandHandleValue, b);

    case BOOLEAN_ARRAY_TYPE:
      if (m_type != BOOLEAN_ARRAY_TYPE || !arrayValue)
        arrayValue.reset(new BooleanArray());
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize((BooleanArray &) *arrayValue, b);

    case INTEGER_ARRAY_TYPE:
      if (m_type != INTEGER_ARRAY_TYPE || !arrayValue)
        arrayValue.reset(new IntegerArray());
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize((IntegerArray &) *arrayValue, b);

    case REAL_ARRAY_TYPE:
      if (m_type != REAL_ARRAY_TYPE || !arrayValue)
        arrayValue.reset(new RealArray());
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize((RealArray &) *arrayValue, b);

    case STRING_ARRAY_TYPE:
      if (m_type != STRING_ARRAY_TYPE || !arrayValue)
        arrayValue.reset(new StringArray());
      m_type = t;
      m_known = true;
      return PLEXIL::deserialize((StringArray &) *arrayValue, b);

    default: // invalid
      return NULL;
    }
  }

  size_t Value::serialSize() const
  {
    if (!isKnown())
      return 1;

    switch (m_type) {
    case BOOLEAN_TYPE:
      return PLEXIL::serialSize(booleanValue);

    case INTEGER_TYPE:
      return PLEXIL::serialSize(integerValue);

    case REAL_TYPE:
      return PLEXIL::serialSize(realValue);

    case STRING_TYPE:
      return PLEXIL::serialSize(*stringValue);

    case COMMAND_HANDLE_TYPE:
      return PLEXIL::serialSize(commandHandleValue);

    case BOOLEAN_ARRAY_TYPE:
    case INTEGER_ARRAY_TYPE:
    case REAL_ARRAY_TYPE:
    case STRING_ARRAY_TYPE:
      return PLEXIL::serialSize(*arrayValue);

    default: // invalid/unimplemented
      return 0;
    }
  }

  template <> char *serialize(Value const &o, char *b)
  {
    return o.serialize(b);
  }

  template <> char const *deserialize(Value &o, char const *b)
  {
    return o.deserialize(b);
  }

  template <> size_t serialSize(Value const &o)
  {
    return o.serialSize();
  }

} // namespace PLEXIL
