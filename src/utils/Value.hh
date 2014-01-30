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

#ifndef PLEXIL_VALUE_HH
#define PLEXIL_VALUE_HH

#include "plexil-config.h"
#include "ConstantMacros.hh"

// Define int32_t
#ifdef HAVE_STDINT_H
#include <stdint.h> // NOTE: not cstdint!
#elif defined(__VXWORKS__)
#include <vxWorks.h>
#endif

#include <string>
#include <vector>

// *** THIS IS A KLUDGE *** but I don't see a better way at the moment
// Must be kept consistent with StoredArray_value_t; see StoredArray.hh.
#define STORED_ARRAY_VALUE_T std::vector<Value>

namespace PLEXIL
{

  /**
   * @brief Constant to signify the largest real value
   */
  DECLARE_GLOBAL_CONST(double, g_maxReal)

  /**
   * @brief Constant to use as a comparison tolerance factor
   */
  DECLARE_GLOBAL_CONST(double, g_epsilon)

  /**
   * @brief Constant to signify an uninitialized value
   */
  class Value;
  DECLARE_GLOBAL_CONST(Value, UNKNOWN)

  // Forward references
  class LabelStr;
  class StoredArray;

  /**
   * @class Value
   * @brief The One True Representation of Plexil expression values.
   * A polymorphic storage container for all legal value types in the Plexil language.
   * Maintains reference counts for those value subtypes which implement them.
   * @note NOT INTENDED TO BE SUBCLASSED!
   */
  class Value
  {
  public:

    /**
     * @brief Default constructor. Sets value to unknown.
     */
    Value();

    /**
     * @brief Copy constructor.
     * If the Value being copied is a LabelStr or StoredArray,
     * properly manages reference counts.
     */
    Value(const Value& other);

    /**
     * @brief Constructor from Boolean value.
     */
    inline Value(bool val)
      : m_value((double) val)
    {
    }

    /**
     * @brief Constructor from integer value.
     */
    inline Value(int32_t val)
      : m_value((double) val)
    {
    }

    /**
     * @brief Constructor from 32-bit unsigned value.
     */
    inline Value(uint32_t val)
      : m_value((double) val)
    {
    }

    /**
     * @brief Constructor from floating point value.
     */
    inline Value(double val)
    : m_value(val)
    {
    }

    /**
     * @brief Constructor from std::string. Effectively creates or reuses a LabelStr for the value.
     */
    Value(const std::string& val);

    /**
     * @brief Constructor from character string constant.
     * @param str A null terminated string
     * @param permanent A value of true means the LabelStr value should be considered
     * a permanent constant (i.e. not reference counted).
     * Effectively creates or reuses a LabelStr for the value.
     */
    Value(const char* val, bool permanent = false);

    /**
     * @brief Constructor from LabelStr. Increments reference count.
     */
    Value(const LabelStr& val);

    /**
     * @brief Constructor from StoredArray.
     * Stores a reference to the shared array and increments reference count.
     */
    Value(const StoredArray& val);

    /**
     * @brief Constructor from array.
     * Effectively creates a new StoredArray instance.
     */
    Value(const STORED_ARRAY_VALUE_T& val);

    /**
     * @brief Destructor. 
     * If the current value is a LabelStr or StoredArray, decrements the reference count.
     */
    ~Value();

    /**
     * @brief Assignment operator from Value.
     * @param other The Value to assign.
     * @return Reference to this.
     */
    Value& operator=(const Value& other);

    /**
     * @brief Assignment operator from Boolean.
     * @param other The bool to assign.
     * @return Reference to this.
     */
    Value& operator=(bool newVal);

    /**
     * @brief Assignment operator from integer.
     * @param other The integer to assign.
     * @return Reference to this.
     */
    Value& operator=(int32_t newVal);

    /**
     * @brief Assignment operator from floating point value.
     * @param other The double to assign.
     * @return Reference to this.
     */
    Value& operator=(double newVal);

    /**
     * @brief Assignment operator from LabelStr.
     * @param other The bool to assign.
     * @return Reference to this.
     */
    Value& operator=(const LabelStr& newVal);

    /**
     * @brief Assignment operator from string.
     * @param other The string to assign.
     * @return Reference to this.
     */
    Value& operator=(const std::string& newVal);

    /**
     * @brief Assignment operator from character string constant.
     * @param other The string to assign.
     * @return Reference to this.
     */
    Value& operator=(const char* newVal);

    /**
     * @brief Assignment operator from StoredArray.
     * @param other The array to assign.
     * @return Reference to this.
     */
    Value& operator=(const StoredArray& newVal);

    /**
     * @brief Assignment operator from array.
     * @param other The array to assign.
     * @return Reference to this.
     */
    Value& operator=(const STORED_ARRAY_VALUE_T& newVal);

    /**
     * @brief Store a copy of a StoredArray.
     * @param other The array to copy.
     */
    void copyArray(const StoredArray& newVal);

    /**
     * @brief Set the value to unknown.
     * @note Properly decrements reference counts when required.
     */
    void setUnknown();

    /**
     * @brief Equality operator.
     * @return True if the two Values are identical, false otherwise.
     * @note Array instances can be equivalent but distinct.
     */
    bool operator==(const Value& other) const;

    /**
     * @brief Equality operator.
     * @return True if the Value is equal to the argument, false otherwise.
     */
    bool operator==(const bool& b) const;

    /**
     * @brief Equality operator.
     * @return True if the Value is equal to the argument, false otherwise.
     */
    bool operator==(const int32_t& i) const;

    /**
     * @brief Equality operator.
     * @return True if the Value is equal to the argument, false otherwise.
     */
    bool operator==(const double& d) const;

    /**
     * @brief Equality operator.
     * @return True if the Value is equal to the argument, false otherwise.
     */
    bool operator==(const LabelStr& s) const;

    /**
     * @brief Equality operator.
     * @return True if the Value is equal to the argument, false otherwise.
     */
    bool operator==(const StoredArray& a) const;

    /**
     * @brief Inequality operator.
     * @return false if the two Values are identical, true otherwise.
     * @note Array instances can be equivalent but distinct.
     */
    inline bool operator!=(const Value& other) const 
    {
      return !operator==(other);
    }

    /**
     * @brief Inequality operator.
     * @return False if the Value is equal to the argument, true otherwise.
     */
    inline bool operator!=(const bool& b) const
    {
      return !operator==(b);
    }

    /**
     * @brief Inequality operator.
     * @return False if the Value is equal to the argument, true otherwise.
     */
    inline bool operator!=(const int32_t& i) const
    {
      return !operator==(i);
    }

    /**
     * @brief Inequality operator.
     * @return False if the Value is equal to the argument, true otherwise.
     */
    inline bool operator!=(const double& d) const
    {
      return !operator==(d);
    }

    /**
     * @brief Inequality operator.
     * @return False if the Value is equal to the argument, true otherwise.
     */
    bool operator!=(const LabelStr& s) const;

    /**
     * @brief Inequality operator.
     * @return False if the Value is equal to the argument, true otherwise.
     * @note Array instances can be equivalent but distinct.
     */
    bool operator!=(const StoredArray& a) const;

    /**
     * @brief Less-than operator.
     * @param other The other value to compare.
     * @return True if the raw value is less than other's raw value.
     * @note Here only for use with templates that require it, e.g. std::map.
     */
    inline bool operator<(const Value& other) const
    {
      return m_value < other.m_value;
    }

    /**
     * @brief Tests whether value is unknown.
     * @return True if unknown, false otherwise.
     */
    bool isUnknown() const;

    /**
     * @brief Tests whether the value is unknown.
     * @return True if unknown, false otherwise.
     */
    static bool isUnknown(double value);

    /**
     * @brief Tests whether value is a string.
     * @return True if a string, false otherwise.
     */
    bool isString() const;

    /**
     * @brief Tests whether value is an array.
     * @return True if a string, false otherwise.
     */
    bool isArray() const;

    /**
     * @brief Tests whether value is a valid Boolean.
     * @return True if a Boolean, false otherwise.
     */
    bool isBoolean() const;

    /**
     * @brief Tests whether value is a valid integer.
     * @return True if an integer, false otherwise.
     */
    bool isInteger() const;

    /**
     * @brief Tests whether value is a valid unsigned integer.
     * @return True if an unsigned integer, false otherwise.
     */
    bool isUnsignedInteger() const;

    /**
     * @brief Tests whether value is a valid Real.
     * @return True if a Real, false otherwise.
     */
    bool isReal() const;

    /**
     * @brief Get raw value.
     * @return The value of the instance variable m_value.
     * @note USE WITH EXTREME CAUTION!
     */
    inline const double& getRawValue() const
    {
      return m_value;
    }

    /**
     * @brief Get value as a Boolean.
     * @return The Boolean value.
     * @note Does NOT check whether value is unknown, a string, or an array.
     * Caller must do that.
     */
    inline bool getBoolValue() const
    {
      return 0.0 != m_value;
    }

    /**
     * @brief Get value as an integer.
     * @return The integer value.
     * @note Does NOT check whether value is a valid integer.
     *       Caller should use the isInteger() method.
     */
    int32_t getIntValue() const;

    /**
     * @brief Get value as an unsigned integer.
     * @return The unsigned integer value.
     * @note Does NOT check whether value is a valid integer.
     *       Caller should use the isUnsignedInteger() method.
     */
    uint32_t getUIntValue() const;

    /**
     * @brief Get value as a floating point number.
     * @return The floating point value.
     */
    double getDoubleValue() const;

    /**
     * @brief Get value as a floating point number.
     * @return The floating point value.
     * @note Will cause failed assertion if the value is a string or array.
     */
    operator double() const;

    /**
     * @brief Get value as a string.
     * @return The string value.
     * @note Will cause failed assertion if the value is not a string.
     */
    const std::string& getStringValue() const;

    /**
     * @brief Get value as a character string constant.
     * @return The char* value.
     * @note Will cause failed assertion if the value is not a string.
     */
    const char* c_str() const;

    /**
     * @brief Get value as a StoredArray instance.
     * @return The StoredArray.
     * @note Will cause failed assertion if the value is not an array.
     */
    StoredArray getStoredArrayValue() const;

    /**
     * @brief Get value as an array.
     * @return The array value.
     * @note Will cause failed assertion if the value is not a string.
     */
    STORED_ARRAY_VALUE_T& getArrayValue();

    /**
     * @brief Get value as a const reference to an array.
     * @return The array value.
     * @note Will cause failed assertion if the value is not a string.
     */
    const STORED_ARRAY_VALUE_T& getConstArrayValue() const;

    /**
     * @brief Print the value onto the stream.
     * @param s Output stream.
     */
    void print(std::ostream& s) const;

    /**
     * @brief Print the value to a newly allocated string.
     * @return The string.
     * @note To be renamed toString().
     */
    std::string valueToString() const;

    /**
     * @brief Print the double, interpreted as a Value, to a newly allocated string.
     * @return The string.
     */
    static std::string valueToString(double value);

    /**
     * @brief The constant signifying an unknown value.
     * @return The unknown value.
     */
    static double UNKNOWN_VALUE();

  private:

    friend class LabelStr;
    friend class StoredArray;

    /**
     * @brief Handle cleanup of a value to be overwritten or deleted.
     */
    inline void unassign();

    double m_value;
  };

  std::ostream& operator<<(std::ostream& s, const Value& v);

}

#endif // PLEXIL_VALUE_HH
