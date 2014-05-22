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

#ifndef PLEXIL_VALUE_HH
#define PLEXIL_VALUE_HH

#include "Array.hh"
#include "ValueType.hh"

#include <stdint.h> // deliberately NOT <cstdint>

namespace PLEXIL
{
  /**
   * @class Value
   * @brief An encapsulation representing any possible value in the PLEXIL language.
   * @note A crude implementation of a tagged (discriminated) union.
   * @note Should only be used when there is no way of knowing the type of a value
   *       at C++ compile time.
   */
  class Value
  {
  public:

    Value();
    Value(Value const &);

    Value(bool val);
    Value(uint16_t enumVal, ValueType typ); // internal values, typed UNKNOWN
    Value(int32_t val);
    Value(double val);
    Value(std::string const &val);
    Value(BooleanArray const &val);
    Value(IntegerArray const &val);
    Value(RealArray const &val);
    Value(StringArray const &val);
    
    ~Value();
    
    Value &operator=(Value const &);

    // TODO: operator= from raw values

    ValueType valueType() const;
    bool isKnown() const;

    bool getValue(bool &result) const;
    bool getValue(int32_t &result) const;
    bool getValue(double &result) const;
    bool getValue(std::string &result) const;

    bool getValuePointer(std::string const *&ptr);
    bool getValuePointer(Array const *&ptr);
    bool getValuePointer(BooleanArray const *&ptr);
    bool getValuePointer(IntegerArray const *&ptr);
    bool getValuePointer(RealArray const *&ptr);
    bool getValuePointer(StringArray const *&ptr);

    bool equals(Value const &) const;

    void print(std::ostream &s) const;

  private:
    // Delete the previous object, if any.
    void cleanup();
    
    union {
      bool          booleanValue;
      uint16_t      enumValue;
      int32_t       integerValue;
      double        realValue;
      std::string  *stringValue;
      BooleanArray *booleanArrayValue;
      IntegerArray *integerArrayValue;
      RealArray    *realArrayValue;
      StringArray  *stringArrayValue;
    } m_value;
    ValueType m_type;
    bool m_known;
  };

  inline bool operator==(Value const &a, Value const &b)
  {
    return a.equals(b);
  }

  inline bool operator!=(Value const &a, Value const &b)
  {
    return !a.equals(b);
  }

} // namespace PLEXIL

#endif // PLEXIL_VALUE_HH
