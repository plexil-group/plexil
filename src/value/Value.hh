/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "Array.hh" // includes ValueType.hh, CommandHandle.hh, NodeConstants.hh, <vector>

#include <memory> // std::unique_ptr

// Explicit instantiation
namespace std
{
  template class unique_ptr<PLEXIL::Array>;
  template class unique_ptr<PLEXIL::String>;
}

namespace PLEXIL
{

  //! \class Value
  //! \brief An encapsulation representing any possible value in the PLEXIL language.
  //! \note Implemented as a tagged (discriminated) union.
  //! \note Of use when there is no way of knowing the PLEXIL type of a value at C++ compile time.
  //! \ingroup Values
  class Value final
  {
  private:
    // Local typedefs
    using ArrayPtr = std::unique_ptr<Array>;
    using StringPtr = std::unique_ptr<String>;

  public:

    //! \brief Default constructor. Constructs a Value with type UNKNOWN_TYPE
    //         and UNKNOWN value.
    Value();

    //! \brief Copy constructor.
    Value(Value const &);

    //! \brief Move constructor.
    Value(Value &&);

    //! \brief Constructor from a Boolean value.
    //! \param val The Boolean value.
    Value(Boolean val);

    //! \brief Constructor from an Integer value.
    //! \param val The Integer value.
    Value(Integer val);

    //! \brief Constructor from a Real value.
    //! \param val The Real value.
    Value(Real val);

    //! \brief Constructor from a NodeState value.
    //! \param val The NodeState value.
    Value(NodeState val);

    //! \brief Constructor from a NodeOutcome value.
    //! \param val The NodeOutcome value.
    Value(NodeOutcome val);

    //! \brief Constructor from a FailureType value.
    //! \param val The FailureType value.
    Value(FailureType val);

    //! \brief Constructor from a CommandHandleValue.
    //! \param val The CommandHandleValue.
    Value(CommandHandleValue val);

    //! \brief Constructor from a String value.
    //! \param val Const reference to the String value.
    Value(String const &val);

    //! \brief Constructor from a null-terminated character string.
    //! \param val Pointer to the const character string.
    Value(char const *val); // for convenience

    //! \brief Constructor from a (generic) Array.
    //! \param val Const reference to the Array.
    Value(Array const &val);

    //! \brief Constructor from a BooleanArray.
    //! \param val Const reference to the BooleanArray.
    Value(BooleanArray const &val);

    //! \brief Constructor from an IntegerArray.
    //! \param val Const reference to the IntegerArray.
    Value(IntegerArray const &val);

    //! \brief Constructor from a RealArray.
    //! \param val Const reference to the RealArray.
    Value(RealArray const &val);

    //! \brief Constructor from a StringArray.
    //! \param val Const reference to the StringArray.
    Value(StringArray const &val);

    //! \brief Constructor for typed UNKNOWN.
    //! \param typ The desired ValueType of the result.
    Value(ValueType typ);

    //! \brief Constructor from a std::vector of Value.
    //! \param vals Const reference to the vector.
    //! \note The Value instances must all have the same ValueType.
    Value(std::vector<Value> const &vals);
    
    //! \brief Destructor.
    ~Value();
    
    //! \brief Copy assignment from another Value.
    //! \param other Const reference to the Value being copied.
    Value &operator=(Value const &);

    //! \brief Move assignment from another Value.
    //! \param other Lvalue reference to the Value being copied.
    Value &operator=(Value &&);

    //! \brief Assignment operator from Boolean value.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(Boolean val);

    //! \brief Assignment operator from Integer value.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(Integer val);

    //! \brief Assignment operator from Real value.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(Real val);

    //! \brief Assignment operator from String.
    //! \param val Const reference to the new value.
    //! \return Reference to *this.
    Value &operator=(String const &val);

    //! \brief Assignment operator from character array.
    //! \param val Const pointer to null-terminated character string.
    //! \return Reference to *this.
    Value &operator=(char const *val);

    // TODO: templatize

    //! \brief Assignment operator from ArrayImpl.
    //! \param val Const reference to the array.
    //! \return Reference to *this.
    Value &operator=(BooleanArray const &val);
    Value &operator=(IntegerArray const &val);
    Value &operator=(RealArray const &val);
    Value &operator=(StringArray const &val);

    //! \brief Assignment operator from NodeState.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(NodeState val);

    //! \brief Assignment operator from NodeOutcome.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(NodeOutcome val);

    //! \brief Assignment operator from FailureType.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(FailureType val);

    //! \brief Assignment operator from CommandHandleValue.
    //! \param val The new value.
    //! \return Reference to *this.
    Value &operator=(CommandHandleValue val);

    //! \brief Set the value to UNKNOWN.
    void setUnknown();

    //! \brief Return the ValueType of this Value.
    ValueType valueType() const;

    //! \brief Is the Value known?
    //! \return true if known, false otherwise.
    bool isKnown() const;

    //! \brief Get the value and store it in the given result variable.
    //! \param result Reference to the result variable.
    //! \return True if the value is known and representable in the result type,
    //!         false otherwise.
    bool getValue(Boolean &result) const;
    bool getValue(Integer &result) const;
    bool getValue(Real &result) const;
    bool getValue(String &result) const;
    bool getValue(NodeState &result) const;
    bool getValue(NodeOutcome &result) const;
    bool getValue(FailureType &result) const;
    bool getValue(CommandHandleValue &result) const;

    //! \brief Get a const pointer to the value and store it in the given result variable.
    //! \param ptr Reference to the result pointer variable.
    //! \return True if the value is known and representable in the result type,
    //!         false otherwise.
    bool getValuePointer(String const *&ptr) const;
    bool getValuePointer(Array const *&ptr) const;
    bool getValuePointer(BooleanArray const *&ptr) const;
    bool getValuePointer(IntegerArray const *&ptr) const;
    bool getValuePointer(RealArray const *&ptr) const;
    bool getValuePointer(StringArray const *&ptr) const;

    Value toValue() const;

    //! \brief Equality test.
    //! \param other Const reference to another Value.
    //! \return True if the two objects are identical, false if not.
    bool equals(Value const &) const;

    //! \brief Less-than comparison operator.
    //! \param other Const reference to another Value.
    //! \return True if this object is less than the other object.
    //! \note Defines a total ordering of all possible Value values.
    //! \note For use by std::map and similar templates.
    bool lessThan(Value const &) const; // for (e.g.) std::map

    //! \brief Write a printed representation of the object to an output stream.
    //! \param s The stream.
    void print(std::ostream &s) const;

    //! \brief Get a printed representation of the value of this object as a string.
    //! \return The string.
    std::string valueToString() const;

    //
    // De/Serialization API
    //

    //! \brief Write a serial representation of this object to the given
    //!        character array buffer.
    //! \param b First character of the buffer to write to.
    //! \return Pointer to the character after the last character written.
    char *serialize(char *b) const; 

    //! \brief Read a serial representation from a buffer into this object.
    //! \param b Pointer to first character of the serial representation.
    //! \return Pointer to the character after the last character read.
    char const *deserialize(char const *b);

    //! \brief Get the number of bytes required by a serial
    //!        representation of this object.
    //! \return The size.
    size_t serialSize() const; 

  private:
    
    //! \brief Prepare the object to be assigned a new value. 
    void cleanup();
    
    //! \brief Prepare the object to be assigned a new value,
    //!        if the previous value was a String.
    void cleanupForString();

    //! \brief Prepare the object to be assigned a new value,
    //!        if the previous value was an Array.
    void cleanupForArray();
    
    union {
      Boolean                  booleanValue;
      NodeState                stateValue;
      NodeOutcome              outcomeValue;
      FailureType              failureValue;
      CommandHandleValue       commandHandleValue;
      Integer                  integerValue;
      Real                     realValue;
      StringPtr                stringValue;
      ArrayPtr                 arrayValue;
    };

    //! \brief The type of the contained value.
    ValueType m_type;

    //! \brief True if the current value is known, false otherwise.
    bool m_known;
  };

  //! \brief Overloaded formatted output operator for Value.
  //! \ingroup Values
  std::ostream &operator<<(std::ostream &, Value const &);

  //! \brief Overloaded equality operator for Value.
  //! \ingroup Values
  inline bool operator==(Value const &a, Value const &b)
  {
    return a.equals(b);
  }

  //! \brief Overloaded inequality operator for Value.
  //! \ingroup Values
  inline bool operator!=(Value const &a, Value const &b)
  {
    return !a.equals(b);
  }

  //! \brief Overloaded less-than operator for Value.
  //! \ingroup Values
  inline bool operator<(Value const &a, Value const &b)
  {
    return a.lessThan(b);
  }

  //! \brief Overloaded less-than-or-equal operator for Value.
  //! \ingroup Values
  inline bool operator<=(Value const &a, Value const &b)
  {
    return !b.lessThan(a);
  }

  //! \brief Overloaded greater-than operator for Value.
  //! \ingroup Values
  inline bool operator>(Value const &a, Value const &b)
  {
    return b.lessThan(a);
  }

  //! \brief Overloaded greater-than-or-equal operator for Value.
  //! \ingroup Values
  inline bool operator>=(Value const &a, Value const &b)
  {
    return !a.lessThan(b);
  }

} // namespace PLEXIL

#endif // PLEXIL_VALUE_HH
