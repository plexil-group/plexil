/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "ValueType.hh"

namespace PLEXIL
{

  //
  // Type aliases
  //

  typedef bool        Boolean;
  typedef int32_t     Integer;
  typedef double      Real;
  typedef std::string String;

  // Subject to change in the future.
  typedef double      Duration;
  typedef double      Time;

  // Array types declared in ArrayFwd.hh, defined in ArrayImpl.hh:
  // BooleanArray
  // IntegerArray
  // RealArray
  // StringArray


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
    Value(char const *val); // for convenience
    Value(BooleanArray const &val);
    Value(IntegerArray const &val);
    Value(RealArray const &val);
    Value(StringArray const &val);

    // Constructs the appropriate array type.
    // Used by TestExternalInterface.
    Value(std::vector<Value> const &vals);
    
    ~Value();
    
    Value &operator=(Value const &);
    Value &operator=(bool val);
    Value &operator=(uint16_t enumVal);
    Value &operator=(int32_t val);
    Value &operator=(double val);
    Value &operator=(std::string const &val);
    Value &operator=(char const *val);
    Value &operator=(BooleanArray const &val);
    Value &operator=(IntegerArray const &val);
    Value &operator=(RealArray const &val);
    Value &operator=(StringArray const &val);
    void setUnknown();

    ValueType valueType() const;
    bool isKnown() const;

    bool getValue(bool &result) const;
    bool getValue(uint16_t &result) const;
    bool getValue(int32_t &result) const;
    bool getValue(double &result) const;
    bool getValue(std::string &result) const;

    bool getValuePointer(std::string const *&ptr) const;
    bool getValuePointer(Array const *&ptr) const;
    bool getValuePointer(BooleanArray const *&ptr) const;
    bool getValuePointer(IntegerArray const *&ptr) const;
    bool getValuePointer(RealArray const *&ptr) const;
    bool getValuePointer(StringArray const *&ptr) const;

    bool equals(Value const &) const;
    bool lessThan(Value const &) const; // for (e.g.) std::map

    void print(std::ostream &s) const;
    std::string valueToString() const;

  private:
    // Delete the previous object, if any.
    void cleanup();
    
    union {
      bool          booleanValue;
      uint16_t      enumValue;
      int32_t       integerValue;
      double        realValue;
      std::string  *stringValue;
      Array        *arrayValue;
    } m_value;
    ValueType m_type;
    bool m_known;
  };

  std::ostream &operator<<(std::ostream &, Value const &);

  inline bool operator==(Value const &a, Value const &b)
  {
    return a.equals(b);
  }
  inline bool operator!=(Value const &a, Value const &b)
  {
    return !a.equals(b);
  }

  inline bool operator<(Value const &a, Value const &b)
  {
    return a.lessThan(b);
  }
  inline bool operator<=(Value const &a, Value const &b)
  {
    return !b.lessThan(a);
  }
  inline bool operator>(Value const &a, Value const &b)
  {
    return b.lessThan(a);
  }
  inline bool operator>=(Value const &a, Value const &b)
  {
    return !a.lessThan(b);
  }

} // namespace PLEXIL

#endif // PLEXIL_VALUE_HH
