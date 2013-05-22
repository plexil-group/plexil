/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#include "LabelStr.hh"
#include "StoredArray.hh"

#include <cmath> // for fabs()
#include <iomanip> // for setprecision()
#include <limits>

namespace PLEXIL
{
  DEFINE_GLOBAL_CONST(double, g_maxReal,
                      std::numeric_limits<double>::max())
  DEFINE_GLOBAL_CONST(Value, UNKNOWN, Value())
  DEFINE_GLOBAL_CONST(double, g_epsilon, 0.00001)

  /**
   * @brief Default constructor. Sets value to unknown.
   */
  Value::Value()
    : m_value(UNKNOWN_VALUE())
  {
  }

  /**
   * @brief Copy constructor.
   * If the Value being copied is a LabelStr or StoredArray,
   * properly manages reference counts.
   */
  Value::Value(const Value& other)
    : m_value(other.m_value)
  {
    if (LabelStr::rangeCheck(m_value)) {
      assertTrue(LabelStr::itemStore().newReference(m_value),
                 "Value copy constructor: LabelStr key is invalid");
    }
    else if (StoredArray::rangeCheck(m_value)) {
      assertTrue(StoredArray::itemStore().newReference(m_value),
                 "Value copy constructor: StoredArray key is invalid");
    }
  }

  /**
   * @brief Constructor from std::string.
   * Effectively creates or reuses a LabelStr for the value.
   */
  Value::Value(const std::string& val)
    : m_value(LabelStr::itemStore().storeItem(val))
  {
  }

  /**
   * @brief Constructor from character string constant.
   * @param str A null terminated string
   * @param permanent A value of true means the LabelStr value should be considered
   * a permanent constant (i.e. not reference counted).
   * Effectively creates or reuses a LabelStr for the value.
   */
  Value::Value(const char* val, bool permanent)
    : m_value(LabelStr::itemStore().storeItem(std::string(val), permanent))
  {
  }

  /**
   * @brief Constructor from LabelStr. Increments reference count.
   */
  Value::Value(const LabelStr& val)
    : m_value(val.getKey())
  {
    assertTrue(LabelStr::itemStore().newReference(val.getKey()),
               "Value constructor from LabelStr: Invalid LabelStr");
  }

  /**
   * @brief Constructor from StoredArray.
   * Stores a reference to the shared array and increments reference count.
   */
  Value::Value(const StoredArray& val)
    : m_value(val.getKey())
  {
    assertTrue(StoredArray::itemStore().newReference(val.getKey()),
               "Value constructor from StoredArray: Invalid StoredArray");
  }

  /**
   * @brief Constructor from array.
   * Effectively creates a new StoredArray instance.
   */
  Value::Value(const StoredArray_value_t& val)
    : m_value(StoredArray::itemStore().storeItem(val))
  {
  }

  /**
   * @brief Destructor. 
   * If the current value is a LabelStr or StoredArray, decrements the reference count.
   */
  Value::~Value()
  {
    unassign();
  }

  /**
   * @brief Assignment operator from Value.
   * @param other The Value to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const Value& other)
  {
    if (m_value != other.m_value) {
      unassign();
      m_value = other.m_value;
      if (LabelStr::rangeCheck(m_value)) {
        assertTrue(LabelStr::itemStore().newReference(m_value),
                   "Value assignment operator: LabelStr key is invalid");
      }
      else if (StoredArray::rangeCheck(m_value)) {
        assertTrue(StoredArray::itemStore().newReference(m_value),
                   "Value assignment operator: StoredArray key is invalid");
      }
    }
    return *this;
  }

  /**
   * @brief Assignment operator from Boolean.
   * @param other The bool to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(bool newVal)
  {
    if (m_value != (double) newVal) {
      unassign();
      m_value = (double) newVal;
    }
    return *this;
  }

  /**
   * @brief Assignment operator from integer.
   * @param other The integer to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(int32_t newVal)
  {
    if (m_value != (double) newVal) {
      unassign();
      m_value = (double) newVal;
    }
    return *this;
  }

  /**
   * @brief Assignment operator from floating point value.
   * @param other The double to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(double newVal)
  {
    if (m_value != newVal) {
      unassign();
      m_value = newVal;
    }
    return *this;
  }

  /**
   * @brief Assignment operator from LabelStr.
   * @param other The bool to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const LabelStr& newVal)
  {
    if (m_value != newVal.getKey()) {
      unassign();
      m_value = newVal.getKey();
      assertTrue(LabelStr::itemStore().newReference(m_value),
                 "Value assigment operator: Invalid LabelStr");
    }
    return *this;
  }

  /**
   * @brief Assignment operator from string.
   * @param other The string to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const std::string& newVal)
  {
    // TODO?: optimize for case where current value == newVal
    unassign();
    m_value = LabelStr::itemStore().storeItem(newVal);
    return *this;
  }

  /**
   * @brief Assignment operator from character string constant.
   * @param other The string to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const char* newVal)
  {
    return operator=(std::string(newVal));
  }

  /**
   * @brief Assignment operator from StoredArray.
   * @param other The array to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const StoredArray& newVal)
  {
    if (m_value != newVal.getKey()) {
      unassign();
      m_value = newVal.getKey();
      assertTrue(StoredArray::itemStore().newReference(m_value),
                 "Value assignment operator: Invalid StoredArray");
    }
    return *this;
  }

  /**
   * @brief Assignment operator from array.
   * @param other The array to assign.
   * @return Reference to this.
   */
  Value& Value::operator=(const StoredArray_value_t& newVal)
  {
    unassign();
    m_value = StoredArray::itemStore().storeItem(newVal);
    return *this;
  }

  /**
   * @brief Store a copy of a StoredArray.
   * @param other The array to copy.
   */
  void Value::copyArray(const StoredArray& newVal)
  {
    unassign();
    m_value = StoredArray::itemStore().storeItem(newVal.getConstArray());
  }

  /**
   * @brief Set the value to unknown.
   * @note Properly decrements reference counts when required.
   */
  void Value::setUnknown()
  {
    if (isUnknown())
      return;
    unassign();
    m_value = UNKNOWN_VALUE();
  }

  /**
   * @brief Equality operator.
   */
  bool Value::operator==(const Value& other) const
  {
    return m_value == other.m_value;
  }

  /**
   * @brief Equality operator.
   * @return True if the Value is equal to the argument, false otherwise.
   */
  bool Value::operator==(const bool& b) const
  {
    if (!isBoolean())
      return false;
    return getBoolValue() == b;
  }

  /**
   * @brief Equality operator.
   * @return True if the Value is equal to the argument, false otherwise.
   */
  bool Value::operator==(const int32_t& i) const
  {
    if (!isInteger())
      return false;
    return getIntValue() == i;
  }

  /**
   * @brief Equality operator.
   * @return True if the Value is equal to the argument, false otherwise.
   */
  bool Value::operator==(const double& d) const
  {
    if (isUnknown())
      return false;
    return m_value == d;
  }

  /**
   * @brief Equality operator.
   * @return True if the Value is equal to the argument, false otherwise.
   */
  bool Value::operator==(const LabelStr& s) const
  {
    if (!isString())
      return false;
    return m_value == s.getKey();
  }

  /**
   * @brief Equality operator.
   * @return True if the Value is equal to the argument, false otherwise.
   * @note Array instances can be equivalent but distinct.
   */
  bool Value::operator==(const StoredArray& a) const
  {
    if (!isArray())
      return false;
    if (m_value == a.getKey())
      return true;
    return getConstArrayValue() == a.getConstArray();
  }

  /**
   * @brief Inequality operator.
   * @return False if the Value is equal to the argument, true otherwise.
   */
  bool Value::operator!=(const LabelStr& s) const
  {
    return !operator==(s);
  }

  /**
   * @brief Inequality operator.
   * @return False if the Value is equal to the argument, true otherwise.
   */
  bool Value::operator!=(const StoredArray& a) const
  {
    return !operator==(a);
  }

  /**
   * @brief Tests whether value is unknown.
   * @return True if unknown, false otherwise.
   */
  bool Value::isUnknown() const
  {
    return m_value == UNKNOWN_VALUE();
  }

  /**
   * @brief Tests whether value is unknown.
   * @return True if unknown, false otherwise.
   */
  bool Value::isUnknown(double value)
  {
    return value == UNKNOWN_VALUE();
  }

  /**
   * @brief Tests whether value is a string.
   * @return True if a string, false otherwise.
   */
  bool Value::isString() const
  {
    return LabelStr::isString(m_value);
  }

  /**
   * @brief Tests whether value is an array.
   * @return True if a string, false otherwise.
   */
  bool Value::isArray() const
  {
    return StoredArray::isKey(m_value);
  }

  /**
   * @brief Tests whether value is a valid Boolean.
   * @return True if a Boolean, false otherwise.
   */
  bool Value::isBoolean() const
  {
    return m_value == 0.0 || m_value == 1.0;
  }

  /**
   * @brief Tests whether value is an integer.
   * @return True if an integer, false otherwise.
   */
  bool Value::isInteger() const
  {
    // N.B. Unknown check falls out of int32_t range check
    return !LabelStr::rangeCheck(m_value)
      && !StoredArray::rangeCheck(m_value)
      && m_value >= std::numeric_limits<int32_t>::min()
      && m_value <= std::numeric_limits<int32_t>::max()
      && fabs(m_value - ((double) (int32_t) m_value)) < g_epsilon();
  }

  /**
   * @brief Tests whether value is an integer.
   * @return True if an integer, false otherwise.
   */
  bool Value::isUnsignedInteger() const
  {
    // N.B. Unknown check falls out of int32_t range check
    return !LabelStr::rangeCheck(m_value)
      && !StoredArray::rangeCheck(m_value)
      && m_value >= 0
      && m_value <= std::numeric_limits<uint32_t>::max()
      && fabs(m_value - ((double) (uint32_t) m_value)) < g_epsilon();
  }

  /**
   * @brief Tests whether value is a valid Real.
   * @return True if a Real, false otherwise.
   */
  bool Value::isReal() const
  {
    return !isUnknown()
      && !LabelStr::rangeCheck(m_value)
      && !StoredArray::rangeCheck(m_value)
      && m_value <= g_maxReal()
      && m_value >= -g_maxReal();
  }

  /**
   * @brief Get value as an integer.
   * @return The integer value.
   */
  int32_t Value::getIntValue() const
  {
    return (int32_t) m_value;
  }

  /**
   * @brief Get value as an unsigned integer.
   * @return The unsigned integer value.
   */
  uint32_t Value::getUIntValue() const
  {
    return (uint32_t) m_value;
  }

  /**
   * @brief Get value as a floating point number.
   * @return The floating point value.
   */
  double Value::getDoubleValue() const
  {
    return m_value;
  }


  /**
   * @brief Get value as a floating point number.
   * @return The floating point value.
   * @note Will cause failed assertion if the value is a string or array.
   */
  Value::operator double() const
  {
    assertTrue(!LabelStr::rangeCheck(m_value)
               && !StoredArray::rangeCheck(m_value),
               "Valus is not a valid double");
    return m_value;
  }


  /**
   * @brief Get value as a string.
   * @return The string value.
   * @note Will cause failed assertion if the value is not a string.
   */
  const std::string& Value::getStringValue() const
  {
    return LabelStr::toString(m_value);
  }

  /**
   * @brief Get value as a character string constant.
   * @return The char* value.
   * @note Will cause failed assertion if the value is not a string.
   */
  const char* Value::c_str() const
  {
    return LabelStr::c_str(m_value);
  }

  /**
   * @brief Get value as a StoredArray instance.
   * @return The StoredArray.
   * @note Will cause failed assertion if the value is not an array.
   */
  StoredArray Value::getStoredArrayValue() const
  {
    return StoredArray(m_value);
  }

  /**
   * @brief Get value as an array.
   * @return The array value.
   * @note Will cause failed assertion if the value is not an array.
   */
  StoredArray_value_t& Value::getArrayValue()
  {
    return StoredArray::getArray(m_value);
  }

  /**
   * @brief Get value as a const reference to an array.
   * @return The array value.
   * @note Will cause failed assertion if the value is not an array.
   */
  const StoredArray_value_t& Value::getConstArrayValue() const
  {
    return StoredArray::getConstArray(m_value);
  }

  /**
   * @brief Print the value onto the stream.
   * @param s Output stream.
   */
  void Value::print(std::ostream& s) const
  {
	if (isUnknown())
	  s << "UNKNOWN";
    else if (isString())
      s << getStringValue();
    else if (isArray())
      // FIXME - make StoredArray output to stream
	  s << getStoredArrayValue().toString();
	// below this point must be a number
    else if (m_value == g_maxReal())
      s << "inf";
    else if (m_value == -g_maxReal())
      s << "-inf";
	else {
	  // Print floats with max precision - they may be times.
      s << std::setprecision(15) << m_value;
	}
  }

  /**
   * @brief Print the value onto the stream.
   * @param s Output stream.
   * @param v The const Value reference.
   * @return The output stream.
   */
  std::ostream& operator<<(std::ostream& s, const Value& v)
  {
    v.print(s);
    return s;
  }

  /**
   * @brief Print the value to a newly allocated string.
   * @return The string.
   * @note To be renamed toString().
   * @see Value::print
   */
  std::string Value::valueToString() const
  {
    return valueToString(m_value);
  }

  /**
   * @brief Print the double, interpreted as a Value, to a newly allocated string.
   * @return The string.
   */
  std::string Value::valueToString(double value)
  {
	if (value == UNKNOWN_VALUE())
	  return "UNKNOWN";
    else if (LabelStr::isString(value))
      return LabelStr::toString(value);
    else if (StoredArray::isKey(value))
	  return StoredArray(value).toString();
	// below this point must be a number
    else if (value == g_maxReal())
      return "inf";
    else if (value == -g_maxReal())
      return "-inf";
	else {
      std::ostringstream s;
	  // Print floats with max precision - they may be times.
      s << std::setprecision(15) << value;
      return s.str();
    }
  }


  /**
   * @brief Handle cleanup of a value to be overwritten or deleted.
   */
  void Value::unassign()
  {
    if (LabelStr::rangeCheck(m_value)) {
      LabelStr::itemStore().deleteReference(m_value);
    }
    else if (StoredArray::rangeCheck(m_value)) {
      StoredArray::itemStore().deleteReference(m_value);
    }
  }

  /**
   * @brief The constant signifying an unknown value.
   * @return The unknown value.
   */
  double Value::UNKNOWN_VALUE()
  {
    double sl_unknown = 
      std::numeric_limits<double>::has_infinity ?
      std::numeric_limits<double>::infinity() :
      std::numeric_limits<double>::max();
    return sl_unknown;
  }

}
