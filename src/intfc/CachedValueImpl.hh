// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_CACHED_VALUE_IMPL_HH
#define PLEXIL_CACHED_VALUE_IMPL_HH

#include "CachedValue.hh"

namespace PLEXIL
{

  //! \class VoidCachedValue
  //! \brief Placeholder object used by the state cache to represent
  //!        a Lookup value whose type is not yet known.
  //! \ingroup External-Interface
  class VoidCachedValue final : public CachedValue
  {
  public:

    //! \brief Default constructor,
    VoidCachedValue();

    //! \brief Virtual destructor.
    virtual ~VoidCachedValue() = default;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    //! \note If other is anything but a VoidCachedValue, this method
    //! reports an error.
    virtual CachedValue &operator=(CachedValue const &other);

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    //! \note The VoidCachedValue method always returns false.
    virtual bool isKnown() const;

    //! \brief Return the value type.
    //! \return The value type.
    //! \note The VoidCachedValue method always returns UNKNOWN_TYPE.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \param other Const reference to another CachedValue instance.
    //! \return True if equal, false if not.
    //! \note The VoidCachedValue method returns true if other.isKnown() returns false.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    //! \note The VoidCachedValue method always returns a default-constructed Value.
    virtual Value toValue() const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    // These stub getValue() and getValuePointer() methods always
    // return false, i.e. unknown.
    
#define DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD(_rtype_) \
    virtual bool getValue(_rtype_ &) const { return false; }

    DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD(Boolean)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD(Integer)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD(Real)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD(String)
#undef DEFINE_VOID_CACHED_VALUE_GET_VALUE_METHOD

#define DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(_rtype_) \
    virtual bool getValuePointer(_rtype_ const *&) const { return false; }

    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(String)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(Array)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(BooleanArray)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(IntegerArray)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(RealArray)
    DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_VOID_CACHED_VALUE_GET_VALUE_POINTER_METHOD

    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    //! \note The VoidCachedValue method always returns false.
    virtual bool setUnknown(unsigned int timestamp)
    {
      return false;
    }

    // For convenience of TestExternalInterface, others
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    // Deliberately unimplemented.
    VoidCachedValue(VoidCachedValue const &) = delete;
    VoidCachedValue(VoidCachedValue &&) = delete;
    VoidCachedValue &operator=(VoidCachedValue const &other) = delete;
    VoidCachedValue &operator=(VoidCachedValue &&other) = delete;
  };

  //! \brief Template class implementing CachedValue API for scalar-valued lookups.
  //! \ingroup External-Interface
  template <typename T>
  class CachedValueImpl final : public CachedValue
  {
  public:

    //! \brief Default constructor.
    CachedValueImpl();

    //! \brief Virtual destructor.
    virtual ~CachedValueImpl() = default;

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    virtual CachedValue &operator=(CachedValue const &other);

    //
    // GetValueImpl API
    //

    //! \brief Determine whether the value is known or unknown.
    //! \return true if known, false otherwise.
    virtual bool isKnown() const;

    //! \brief Return the value type.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \return True if equal, false if not.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to an appropriately typed variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(T &result) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    //
    // API to external interface
    //
    
    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    virtual bool setUnknown(unsigned int timestamp);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    //! \note If the type of the new value is inconsistent with the object's type,
    //!       the object's value is set to unknown.
    virtual bool update(unsigned int timestamp, T const &val);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if changed, false otherwise.
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    //! \brief Copy constructor.
    //! \param orig Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl(CachedValueImpl<T> const &orig) = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl<T> &operator=(CachedValueImpl<T> const &other) = default;

    // Move constructor not implemented.
    CachedValueImpl(CachedValueImpl<T> &&) = delete;

    // Assignment operators not implemented.
    CachedValueImpl<T> &operator=(CachedValueImpl<T> &&) = delete;

    //! \brief The current value.
    T m_value;

    //! \brief True if the value is known, false if unknown.
    bool m_known;
  };

  //! \brief Specialization of CachedValueImpl class template for Integer valued lookups.
  //! \note Specialized to support getValue(Real &) conversion method
  //! \ingroup External-Interface
  template <>
  class CachedValueImpl<Integer> final : public CachedValue
  {
  public:

    //! \brief Default constructor.
    CachedValueImpl();

    //! \brief Virtual destructor.
    virtual ~CachedValueImpl() = default;

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    virtual CachedValue &operator=(CachedValue const &other);

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const;

    //! \brief Return the value type.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \return True if equal, false if not.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to an appropriately typed variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(Integer &result) const;

    //! \brief Retrieve the value of this object as a Real.
    //! \param result Reference to an appropriately typed variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    //! \note Conversion method.
    virtual bool getValue(Real &result) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    virtual bool setUnknown(unsigned int timestamp);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    //! \note If the type of the new value is inconsistent with the object's type,
    //!       the object's value is set to unknown.
    virtual bool update(unsigned int timestamp, Integer const &val);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if changed, false otherwise.
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    //! \brief Copy constructor.
    //! \param orig Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl(CachedValueImpl<Integer> const &orig) = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl<Integer> &operator=(CachedValueImpl<Integer> const &other) = default;

    // Move constructor, assignment not implemented.
    CachedValueImpl(CachedValueImpl<Integer> &&) = delete;
    CachedValueImpl<Integer> &operator=(CachedValueImpl<Integer> &&) = delete;

    //! \brief The current value.
    Integer m_value;

    //! \brief True if the value is known, false if unknown.
    bool m_known;
  };

  //! \brief Specialization of CachedValueImpl class template for Real valued lookups.
  //! \note Specialized to support update(unsigned int, Integer const &) conversion method
  //! \ingroup External-Interface
  template <>
  class CachedValueImpl<Real> final : public CachedValue
  {
  public:

    //! \brief Default constructor.
    CachedValueImpl();

    //! \brief Virtual destructor.
    virtual ~CachedValueImpl() = default;

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    virtual CachedValue &operator=(CachedValue const &other);

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const;
    
    //! \brief Return the value type.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \return True if equal, false if not.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to an appropriately typed variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(Real &result) const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    virtual bool setUnknown(unsigned int timestamp);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    virtual bool update(unsigned int timestamp, Real const &val);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    //! \note Conversion method.
    virtual bool update(unsigned int timestamp, Integer const &val);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const pointer to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    //! \brief Copy constructor.
    //! \param orig Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl(CachedValueImpl<Real> const &orig) = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl<Real> &operator=(CachedValueImpl<Real> const &other) = default;

    // Move constructor, assignment not implemented.
    CachedValueImpl(CachedValueImpl<Real> &&) = delete;
    CachedValueImpl<Real> &operator=(CachedValueImpl<Real> &&) = delete;

    //! \brief The current value.
    Real m_value;

    //! \brief True if the value is known, false if unknown.
    bool m_known;
  };

  //! \brief Specialization of CachedValueImpl class template for String valued lookups.
  //! \ingroup External-Interface
  template <>
  class CachedValueImpl<String> final : public CachedValue
  {
  public:

    //! \brief Default constructor.
    CachedValueImpl();

    //! \brief Virtual destructor.
    virtual ~CachedValueImpl() = default;

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    virtual CachedValue &operator=(CachedValue const &other);

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const;
    
    //! \brief Return the value type.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \return True if equal, false if not.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param result Reference to an appropriately typed variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(String &result) const;

    //! \brief Retrieve a pointer to the (const) value of this object.
    //! \param ptr Reference to the pointer variable to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The pointer is not copied if the return value is false.
    virtual bool getValuePointer(String const *&) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    virtual bool setUnknown(unsigned int timestamp);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    virtual bool update(unsigned int timestamp, String const &val);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param valPtr Const pointer to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    virtual bool updatePtr(unsigned int timestamp, String const *valPtr);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if changed, false otherwise.
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    //! \brief Copy constructor.
    //! \param orig Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl(CachedValueImpl<String> const &orig) = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl<String> &operator=(CachedValueImpl<String> const &other) = default;

    // Move constructor, assignment not implemented.
    CachedValueImpl(CachedValueImpl<String> &&) = delete;
    CachedValueImpl<String> &operator=(CachedValueImpl<String> &&) = delete;

    //! \brief The current value.
    String m_value;

    //! \brief True if the value is known, false if unknown.
    bool m_known;
  };

  //! \brief Specialization of CachedValueImpl class template for Array valued lookups.
  //! \ingroup External-Interface
  template <typename T>
  class CachedValueImpl<ArrayImpl<T> > final : public CachedValue
  {
  public:

    //! \brief Default constructor.
    CachedValueImpl();

    //! \brief Virtual destructor.
    virtual ~CachedValueImpl() = default;

    //! \brief Create an identical copy of this object.
    //! \return Pointer to the copy.
    virtual CachedValue *clone() const;

    //! \brief Assignment operator from CachedValue.
    //! \param other Const reference to another CachedValue instance.
    //! \return Reference to *this.
    virtual CachedValue &operator=(CachedValue const &other);

    //! \brief Determine whether the value is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const;
    
    //! \brief Return the value type.
    //! \return The value type.
    virtual ValueType valueType() const;

    //! \brief Test whether two CachedValue instances are equal.
    //! \param other Const reference to a CachedValue instance.
    //! \return True if equal, false if not.
    virtual bool operator==(CachedValue const &) const;

    //! \brief Retrieve the value of this object as a generic Array.
    //! \param ptr Reference to an appropriately typed pointer to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValuePointer(Array const *&) const;

    //! \brief Retrieve the value of this object in its native type.
    //! \param ptr Reference to an appropriately typed pointer to receive the result.
    //! \return True if the value is known, false if unknown or invalid for the result type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValuePointer(ArrayImpl<T> const *&) const;

    //! \brief Get the value of this object as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const;

    //! \brief Print the object's value to the given stream.
    //! \param s The output stream.
    virtual void printValue(std::ostream &s) const;

    //! \brief Set the value to unknown.
    //! \param timestamp Sequence number, e.g. the Exec macro step count.
    //! \return True if the value was previously known, false otherwise.
    virtual bool setUnknown(unsigned int timestamp);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param valPtr Const pointer to the new value.
    //! \return True if the new value differs from the old, false otherwise.
    virtual bool updatePtr(unsigned int timestamp, ArrayImpl<T> const *valPtr);

    //! \brief Update the cache entry with the given new value.
    //! \param timestamp Sequence number.
    //! \param val Const reference to the new value.
    //! \return True if changed, false otherwise.
    virtual bool update(unsigned int timestamp, Value const &val);

  private:

    //! \brief Copy constructor.
    //! \param orig Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl(CachedValueImpl<ArrayImpl<T>> const &orig) = default;

    //! \brief Copy assignment operator.
    //! \param other Const reference to a CachedValue instance.
    //! \note Only accessible within this class.
    CachedValueImpl<ArrayImpl<T>> &operator=(CachedValueImpl<ArrayImpl<T>> const &other) = default;

    // Move constructor, assignment not implemented.
    CachedValueImpl(CachedValueImpl<ArrayImpl<T> > &&) = delete;
    CachedValueImpl<ArrayImpl<T> > &operator=(CachedValueImpl<ArrayImpl<T> > &&) = delete;

    //! \brief The current value.
    ArrayImpl<T> m_value;

    //! \brief True if the value is known, false if unknown.
    bool m_known;
  };

} // namespace PLEXIL

#endif // PLEXIL_CACHED_VALUE_IMPL_HH
