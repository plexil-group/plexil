/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include <memory> // std::unique_ptr
#include <vector>

namespace PLEXIL
{

  /**
   * @class Value
   * @brief An encapsulation representing any possible value in the PLEXIL language.
   * @note Implemented a tagged (discriminated) union.
   * @note Of use when there is no way of knowing the PLEXIL type of a value
   *       at C++ compile time.
   */
  class Value final
  {
  public:

    Value();
    Value(Value const &);
    Value(Value &&);

    Value(Boolean val);
    Value(Integer val);
    Value(Real val);
    Value(NodeState val);
    Value(NodeOutcome val);
    Value(FailureType val);
    Value(CommandHandleValue val);
    Value(String const &val);
    Value(char const *val); // for convenience
    Value(Array const &val);
    Value(BooleanArray const &val);
    Value(IntegerArray const &val);
    Value(RealArray const &val);
    Value(StringArray const &val);

    Value(uint8_t enumVal, ValueType typ); // Typed UNKNOWN

    // Constructs the appropriate array type.
    // Used by TestExternalInterface.
    Value(std::vector<Value> const &vals);
    
    ~Value();
    
    Value &operator=(Value const &);
    Value &operator=(Value &&);
    Value &operator=(Boolean val);
    Value &operator=(Integer val);
    Value &operator=(Real val);
    Value &operator=(String const &val);
    Value &operator=(char const *val);
    // TODO: templatize
    Value &operator=(BooleanArray const &val);
    Value &operator=(IntegerArray const &val);
    Value &operator=(RealArray const &val);
    Value &operator=(StringArray const &val);

    Value &operator=(NodeState val);
    Value &operator=(NodeOutcome val);
    Value &operator=(FailureType val);
    Value &operator=(CommandHandleValue val);

    void setUnknown();

    ValueType valueType() const;
    bool isKnown() const;

    bool getValue(Boolean &result) const;
    bool getValue(Integer &result) const;
    bool getValue(Real &result) const;
    bool getValue(String &result) const;

    bool getValuePointer(String const *&ptr) const;
    bool getValuePointer(Array const *&ptr) const;
    bool getValuePointer(BooleanArray const *&ptr) const;
    bool getValuePointer(IntegerArray const *&ptr) const;
    bool getValuePointer(RealArray const *&ptr) const;
    bool getValuePointer(StringArray const *&ptr) const;

    bool getValue(NodeState &result) const;
    bool getValue(NodeOutcome &result) const;
    bool getValue(FailureType &result) const;
    bool getValue(CommandHandleValue &result) const;

    Value toValue() const;

    bool equals(Value const &) const;
    bool lessThan(Value const &) const; // for (e.g.) std::map

    void printValue(std::ostream &s) const;
    std::string valueToString() const;

    char *serialize(char *b) const; 
    char const *deserialize(char const *b);
    size_t serialSize() const; 

  private:
    
    // Prepare to be assigned a new value
    void cleanup();
    void cleanupForString();
    void cleanupForArray();
    
    union {
      Boolean                  booleanValue;
      NodeState                stateValue;
      NodeOutcome              outcomeValue;
      FailureType              failureValue;
      CommandHandleValue       commandHandleValue;
      Integer                  integerValue;
      Real                     realValue;
      std::unique_ptr<String>  stringValue;
      std::unique_ptr<Array>   arrayValue;
    };
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
