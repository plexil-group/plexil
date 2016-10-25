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

#include "CachedValueImpl.hh"

#include "ArrayImpl.hh"
#include "Debug.hh"
#include "ExternalInterface.hh"
#include "InterfaceError.hh"
#include "PlexilTypeTraits.hh"
#include "Value.hh"

#include <iomanip>

namespace PLEXIL
{
  // Placeholder object
  VoidCachedValue::VoidCachedValue()
    : CachedValue()
  {
  }

  VoidCachedValue::~VoidCachedValue()
  {
  }
 
  CachedValue &VoidCachedValue::operator=(CachedValue const &other)
  {
    assertTrue_2(dynamic_cast<VoidCachedValue const *>(&other),
                 "VoidCachedValue: assigning from incompatible CachedValue type");
    return static_cast<CachedValue &>(*this);
  }

  CachedValue &VoidCachedValue::operator=(CachedValue &&other)
  {
    assertTrue_2(dynamic_cast<VoidCachedValue const *>(&other),
                 "VoidCachedValue: assigning from incompatible CachedValue type");
    return static_cast<CachedValue &>(*this);
  }

  ValueType VoidCachedValue::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  bool VoidCachedValue::isKnown() const
  {
    return false;
  }

  CachedValue *VoidCachedValue::clone() const
  {
    return new VoidCachedValue();
  }

  bool VoidCachedValue::operator==(CachedValue const &other) const
  {
    return !other.isKnown();
  }

  Value VoidCachedValue::toValue() const
  {
    return Value();
  }

  void VoidCachedValue::printValue(std::ostream &s) const
  {
    s << "[unknown_value]"; 
  }

  /**
   * @brief Update the cache entry with the given new value.
   * @param timestamp Sequence number.
   * @param val The new value.
   * @note The caller is responsible for deleting the object pointed to upon return.
   */

#define DEFINE_UPDATE_METHOD(_type_)                                    \
  bool VoidCachedValue::update(unsigned int /* timestamp */, _type_ const & /* val */) \
  {                                                                     \
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");        \
    return false;                                                       \
  }

  DEFINE_UPDATE_METHOD(Boolean)
  DEFINE_UPDATE_METHOD(Integer)
  DEFINE_UPDATE_METHOD(Real)
  DEFINE_UPDATE_METHOD(NodeState)
  DEFINE_UPDATE_METHOD(NodeOutcome)
  DEFINE_UPDATE_METHOD(FailureType)
  DEFINE_UPDATE_METHOD(CommandHandleValue)
  DEFINE_UPDATE_METHOD(String)

#undef DEFINE_UPDATE_METHOD

#define DEFINE_UPDATE_PTR_METHOD(_type_)                                \
  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, _type_ const * /* valPtr */) \
  {                                                                     \
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");        \
    return false;                                                       \
  }

  DEFINE_UPDATE_PTR_METHOD(String)
  DEFINE_UPDATE_PTR_METHOD(BooleanArray)
  DEFINE_UPDATE_PTR_METHOD(IntegerArray)
  DEFINE_UPDATE_PTR_METHOD(RealArray)
    DEFINE_UPDATE_PTR_METHOD(StringArray)

#undef DEFINE_UPDATE_PTR_METHOD
  
  bool VoidCachedValue::update(unsigned int timestamp, Value const &val)
  {
    assertTrue_2(!val.isKnown(), "Can't update a VoidCachedValue");
    this->m_timestamp = timestamp;
    return true;
  }

  //
  // Typed implementation
  //

  //
  // Default constructors
  //
  
  template <typename T>
  CachedValueImpl<T>::CachedValueImpl()
    : CachedValueShim<CachedValueImpl<T> >(),
    m_known(false)
  {
  }

  CachedValueImpl<String>::CachedValueImpl()
    : CachedValueShim<CachedValueImpl<String> >(),
    m_known(false)
  {
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> >::CachedValueImpl()
    : CachedValueShim<CachedValueImpl<ArrayImpl<T> > >(),
    m_known(false)
  {
  }

  //
  // Copy constructors
  //

  template <typename T>
  CachedValueImpl<T>::CachedValueImpl(CachedValueImpl<T> const &orig)
    : CachedValueShim<CachedValueImpl<T> >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  CachedValueImpl<String>::CachedValueImpl(CachedValueImpl<String> const &orig)
    : CachedValueShim<CachedValueImpl<String> >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> >::CachedValueImpl(CachedValueImpl<ArrayImpl<T> > const &orig)
    : CachedValueShim<CachedValueImpl<ArrayImpl<T> > >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  //
  // Move constructors
  //

  template <typename T>
  CachedValueImpl<T>::CachedValueImpl(CachedValueImpl<T> &&orig)
    : CachedValueShim<CachedValueImpl<T> >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  CachedValueImpl<String>::CachedValueImpl(CachedValueImpl<String> &&orig)
    : CachedValueShim<CachedValueImpl<String> >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> >::CachedValueImpl(CachedValueImpl<ArrayImpl<T> > &&orig)
    : CachedValueShim<CachedValueImpl<ArrayImpl<T> > >(orig),
    m_value(orig.m_value),
    m_known(orig.m_known)
  {
  }

  //
  // Destructor
  //

  template <typename T>
  CachedValueImpl<T>::~CachedValueImpl()
  {
  }

  CachedValueImpl<String>::~CachedValueImpl()
  {
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> >::~CachedValueImpl()
  {
  }

  //
  // Copy assignment
  //

  template <typename T>
  CachedValue &CachedValueImpl<T>::operator=(CachedValue const &other)
  {
    CachedValueImpl<T> const *otherPtr = 
      dynamic_cast<CachedValueImpl<T> const * > (&other);
    checkInterfaceError(otherPtr,
                        "Attempt to assign CachedValue of type "
                        << PlexilValueType<T>::typeName
                        << " from another of type " << valueTypeName(other.valueType()));
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  CachedValue &CachedValueImpl<String>::operator=(CachedValue const &other)
  {
    CachedValueImpl<String> const *otherPtr = 
      dynamic_cast<CachedValueImpl<String> const * > (&other);
    checkInterfaceError(otherPtr,
                        "Attempt to assign CachedValue of type String from another of type "
                        << valueTypeName(other.valueType()));
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  template <typename T>
  CachedValue &CachedValueImpl<ArrayImpl<T> >::operator=(CachedValue const &other)
  {
    CachedValueImpl<ArrayImpl<T> > const *otherPtr = 
      dynamic_cast<CachedValueImpl<ArrayImpl<T> > const *>(&other);
    checkInterfaceError(otherPtr,
                        "Attempt to assign CachedValue of type "
                        << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " from another of type " << valueTypeName(other.valueType()));
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  template <typename T>
  CachedValueImpl<T> &CachedValueImpl<T>::operator=(CachedValueImpl<T> const &other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  CachedValueImpl<String> &CachedValueImpl<String>::operator=(CachedValueImpl<String> const &other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> > &CachedValueImpl<ArrayImpl<T> >::operator=(CachedValueImpl<ArrayImpl<T> > const &other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  //
  // Move assignment
  //

  template <typename T>
  CachedValue &CachedValueImpl<T>::operator=(CachedValue &&other)
  {
    CachedValueImpl<T> const *otherPtr = 
      dynamic_cast<CachedValueImpl<T> const * > (&other);
    assertTrue_2(otherPtr, "CachedValue: invalid assignment from other type");
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  CachedValue &CachedValueImpl<String>::operator=(CachedValue &&other)
  {
    CachedValueImpl<String> const *otherPtr = 
      dynamic_cast<CachedValueImpl<String> const * > (&other);
    assertTrue_2(otherPtr, "CachedValue: invalid assignment from other type");
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  template <typename T>
  CachedValue &CachedValueImpl<ArrayImpl<T> >::operator=(CachedValue &&other)
  {
    CachedValueImpl<ArrayImpl<T> > const *otherPtr = 
      dynamic_cast<CachedValueImpl<ArrayImpl<T> > const *>(&other);
    assertTrue_2(otherPtr, "CachedValue: invalid assignment from other type");
    return static_cast<CachedValue &>(operator=(*otherPtr));
  }

  template <typename T>
  CachedValueImpl<T> &CachedValueImpl<T>::operator=(CachedValueImpl<T> &&other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  CachedValueImpl<String> &CachedValueImpl<String>::operator=(CachedValueImpl<String> &&other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> > &CachedValueImpl<ArrayImpl<T> >::operator=(CachedValueImpl<ArrayImpl<T> > &&other)
  {
    this->m_timestamp = other.getTimestamp();
    if ((m_known = other.m_known))
      m_value = other.m_value;
    return *this;
  }

  //
  // GetValue API
  //

  template <typename T>
  ValueType CachedValueImpl<T>::valueType() const
  {
    return PlexilValueType<T>::value;
  }

  ValueType CachedValueImpl<String>::valueType() const
  {
    return PlexilValueType<String>::value;
  }

  template <typename T>
  ValueType CachedValueImpl<ArrayImpl<T> >::valueType() const
  {
    return PlexilValueType<ArrayImpl<T> >::value;
  }

  template <typename T>
  bool CachedValueImpl<T>::isKnown() const
  {
    return m_known;
  }

  bool CachedValueImpl<String>::isKnown() const
  {
    return m_known;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::isKnown() const
  {
    return m_known;
  }

  template <typename T>
  Value CachedValueImpl<T>::toValue() const
  {
    T temp;
    bool known = this->getValueImpl(temp);
    if (known)
      return Value(temp);
    else
      return Value(0, this->valueType());
  }

  Value CachedValueImpl<String>::toValue() const
  {
    std::string const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }

  template <typename T>
  Value CachedValueImpl<ArrayImpl<T> >::toValue() const
  {
    ArrayImpl<T> const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }

  template <typename T>
  bool CachedValueImpl<T>::getValueImpl(T &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValue: trying to get a " << PlexilValueType<U>::typeName
                  << " value from a " << PlexilValueType<T>::typeName << " typed object");
    return false;
  }

  // More conversions can be added as required.
  template <>
  template <>
  bool CachedValueImpl<Integer>::getValueImpl(Real &result) const
  {
    if (m_known)
      result = (Real) m_value;
    return m_known;
  }

  bool CachedValueImpl<String>::getValueImpl(std::string &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  template <typename U>
  bool CachedValueImpl<String>::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValue: trying to get a " << PlexilValueType<U>::typeName
                  << " value from a " << PlexilValueType<String>::typeName << " typed object");
    return false;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::getValueImpl(ArrayImpl<T> &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValue: not implemented for "
                  << PlexilValueType<ArrayImpl<T> >::typeName
                  << " typed objects");
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValue: trying to get a " << PlexilValueType<U>::typeName
                  << " value from a " << PlexilValueType<ArrayImpl<T> >::typeName << " typed object");
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
                  << " pointer value from a " << PlexilValueType<T>::typeName << " typed object");
    return false;
  }

  bool CachedValueImpl<String>::getValuePointerImpl(std::string const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  // Report error for string types
  template <typename U>
  bool CachedValueImpl<String>::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
                  << " pointer value from a " << PlexilValueType<String>::typeName << " typed object");
    return false;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(ArrayImpl<T> const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  // Conversion method
  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(Array const *& ptr) const
  {
    if (m_known)
      ptr = dynamic_cast<Array const *>(&m_value);
    return m_known;
  }

  // Report error for type mismatch
  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
                  << " pointer value from a " << PlexilValueType<ArrayImpl<T> >::typeName << " typed object");
    return false;
  }

  template <typename T>
  void CachedValueImpl<T>::printValue(std::ostream &s) const
  {
    if (m_known)
      PLEXIL::printValue(m_value, s);
    else
      s << "[unknown_value]"; 
  }

  void CachedValueImpl<String>::printValue(std::ostream &s) const
  {
    if (m_known)
      PLEXIL::printValue(m_value, s);
    else
      s << "[unknown_value]"; 
  }

  template <typename T>
  void CachedValueImpl<ArrayImpl<T> >::printValue(std::ostream &s) const
  {
    if (m_known)
      PLEXIL::printValue(m_value, s);
    else
      s << "[unknown_value]"; 
  }

  template <typename T>
  bool CachedValueImpl<T>::setUnknown(unsigned int timestamp)
  {
    bool wasKnown = m_known;
    m_known = false;
    this->m_timestamp = timestamp;
    return wasKnown;
  }

  bool CachedValueImpl<String>::setUnknown(unsigned int timestamp)
  {
    bool wasKnown = m_known;
    m_known = false;
    this->m_timestamp = timestamp;
    return wasKnown;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::setUnknown(unsigned int timestamp)
  {
    bool wasKnown = m_known;
    m_known = false;
    this->m_timestamp = timestamp;
    return wasKnown;
  }

  template <typename T>
  bool CachedValueImpl<T>::operator==(CachedValue const &other) const
  {
    if (!m_known && other.isKnown())
      return true;
    CachedValueImpl<T> const *otherPtr = 
      dynamic_cast<CachedValueImpl<T> const *>(&other);
    if (!otherPtr)
      return false; // different type
    if (m_known != otherPtr->m_known
        || m_value != otherPtr->m_value)
      return false; // different value
    return true;
  }

  bool CachedValueImpl<String>::operator==(CachedValue const &other) const
  {
    if (!m_known && other.isKnown())
      return true;
    CachedValueImpl<String> const *otherPtr = 
      dynamic_cast<CachedValueImpl<String> const *>(&other);
    if (!otherPtr)
      return false; // different type
    if (m_known != otherPtr->m_known
        || m_value != otherPtr->m_value)
      return false; // different value
    return true;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::operator==(CachedValue const &other) const
  {
    if (!m_known && other.isKnown())
      return true;
    CachedValueImpl<ArrayImpl<T> > const *otherPtr = 
      dynamic_cast<CachedValueImpl<ArrayImpl<T> > const *>(&other);
    if (!otherPtr)
      return false; // different type
    if (m_known != otherPtr->m_known
        || m_value != otherPtr->m_value)
      return false; // different value
    return true;
  }

  template <typename T>
  CachedValue *CachedValueImpl<T>::cloneImpl() const
  {
    return new CachedValueImpl<T>(*this);
  }

  CachedValue *CachedValueImpl<String>::cloneImpl() const
  {
    return new CachedValueImpl<String>(*this);
  }

  template <typename T>
  CachedValue *CachedValueImpl<ArrayImpl<T> >::cloneImpl() const
  {
    return new CachedValueImpl<ArrayImpl<T> >(*this);
  }

  // Only implemented for scalar types (and string)
  template <typename T>
  bool CachedValueImpl<T>::updateImpl(unsigned int timestamp, T const &val)
  {
    if (!m_known || m_value != val) {
      m_value = val;
      m_known = true;
      this->m_timestamp = timestamp;
      debugMsg("CachedValue:update", " updated to " << val);
      return true;
    }
    debugMsg("CachedValue:update", " value is already " << val << ", not updating");
    return false;
  }

  // Specialization for double, solely for debug printing purposes.
  template <>
  bool CachedValueImpl<Real>::updateImpl(unsigned int timestamp, double const &val)
  {
    if (!m_known || m_value != val) {
      m_value = val;
      m_known = true;
      this->m_timestamp = timestamp;
      debugMsg("CachedValue:update", " updated to " << std::setprecision(15) << val);
      return true;
    }
    debugMsg("CachedValue:update", " value is already " << val << ", not updating");
    return false;
  }

  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, std::string const &val)
  {
    if (!m_known || m_value != val) {
      m_value = val;
      m_known = true;
      this->m_timestamp = timestamp;
      return true;
    }
    return false;
  }

  // Default wrong-type methods.
  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::updateImpl(unsigned int timestamp, U const & /* val */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << PlexilValueType<T>::typeName
                        << " CachedValue with a " << PlexilValueType<U>::typeName); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, U const & /* val */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a String CachedValue with a "
                        << PlexilValueType<U>::typeName); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::updateImpl(unsigned int timestamp, U const & /* val */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " CachedValue with a " << PlexilValueType<U>::typeName); 
    return false;
  }

  // Type conversion method.
  template <>
  template <>
  bool CachedValueImpl<Real>::updateImpl(unsigned int timestamp, int32_t const &val)
  {
    return this->updateImpl(timestamp, (double) val);
  }

  // From Value
  // Default for scalar types
  template <typename T>
  bool CachedValueImpl<T>::updateImpl(unsigned int timestamp, Value const &val)
  {
    T nativeVal;
    if (val.getValue(nativeVal))
      return this->updateImpl(timestamp, nativeVal);
    else {
      debugMsg("CachedValue:mismatch",
               " value " << val << "is wrong type for "
               << PlexilValueType<T>::typeName << " lookup");
      return setUnknown(timestamp);
    }
  }

  // Special case for string
  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, Value const &val)
  {
    std::string const *valPtr;
    if (val.getValuePointer(valPtr))
      return this->updatePtrImpl(timestamp, valPtr);
    else {
      debugMsg("CachedValue:mismatch",
               " value " << val << "is wrong type for String lookup");
      return setUnknown(timestamp);
    }
  }

  // Array method
  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::updateImpl(unsigned int timestamp, Value const &val)
  {
    ArrayImpl<T> const *valPtr;
    if (val.getValuePointer(valPtr))
      return this->updatePtrImpl(timestamp, valPtr);
    else {
      debugMsg("CachedValue:mismatch",
               " value " << val << "is wrong type for "
               << PlexilValueType<ArrayImpl<T> >::typeName << " lookup");
      return setUnknown(timestamp);
    }
  }

  //
  // updatePtrImpl
  //

  bool CachedValueImpl<String>::updatePtrImpl(unsigned int timestamp, std::string const *ptr)
  {
    if (!m_known || m_value != *ptr) {
      m_value = *ptr;
      m_known = true;
      this->m_timestamp = timestamp;
      return true;
    }
    return false;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::updatePtrImpl(unsigned int timestamp, ArrayImpl<T> const *ptr)
  {
    if (!m_known || m_value != *ptr) {
      m_value = *ptr;
      m_known = true;
      this->m_timestamp = timestamp;
      return true;
    }
    return false;
  }

  // Default wrong-type methods.
  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::updatePtrImpl(unsigned int /* timestamp */, U const * /* ptr */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << PlexilValueType<T>::typeName
                        << " CachedValue with a " << PlexilValueType<U>::typeName); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::updatePtrImpl(unsigned int /* timestamp */, U const * /* ptr */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a String CachedValue with a "
                        << PlexilValueType<U>::typeName); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::updatePtrImpl(unsigned int /* timestamp */, U const * /* ptr */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " CachedValue with a " << PlexilValueType<U>::typeName); 
    return false;
  }

  //
  // getValue() and friends
  //

  //
  // Factory
  //

  CachedValue *CachedValueFactory(ValueType vtype)
  {
    switch (vtype) {
    case BOOLEAN_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<Boolean>());

    case INTEGER_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<Integer>());

    case REAL_TYPE:
    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
      return static_cast<CachedValue *>(new CachedValueImpl<Real>());

    case STRING_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<String>());

    case BOOLEAN_ARRAY_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<BooleanArray>());

    case INTEGER_ARRAY_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<IntegerArray>());

    case REAL_ARRAY_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<RealArray>());

    case STRING_ARRAY_TYPE:
      return static_cast<CachedValue *>(new CachedValueImpl<StringArray>());

    case UNKNOWN_TYPE:
      return static_cast<CachedValue *>(new VoidCachedValue());

    default:
      assertTrue_2(ALWAYS_FAIL, "CachedValueFactory: Invalid or unimplemented value type");
      return NULL;
    }
  }

  //
  // Explicit instantiation (possibly redundant with factory above)
  //

  template class CachedValueImpl<Boolean>;

  template bool CachedValueImpl<Boolean>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<Boolean>::getValueImpl(Real &) const;
  template bool CachedValueImpl<Boolean>::getValueImpl(String &) const;

  template bool CachedValueImpl<Boolean>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<Boolean>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<Boolean>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<Boolean>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<Boolean>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<Boolean>::getValuePointerImpl(Array const *&) const;
  template bool CachedValueImpl<Boolean>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<Boolean>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<Boolean>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<Boolean>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<Integer>;

  template bool CachedValueImpl<Integer>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<Integer>::getValueImpl(String &) const;

  template bool CachedValueImpl<Integer>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<Integer>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<Integer>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<Integer>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<Integer>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<Integer>::getValuePointerImpl(Array const *&) const;
  template bool CachedValueImpl<Integer>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<Integer>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<Integer>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<Integer>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<Real>;

  template bool CachedValueImpl<Real>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<Real>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<Real>::getValueImpl(String &) const;

  template bool CachedValueImpl<Real>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<Real>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<Real>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<Real>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<Real>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<Real>::getValuePointerImpl(Array const *&) const;
  template bool CachedValueImpl<Real>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<Real>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<Real>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<Real>::getValuePointerImpl(StringArray const *&) const;

  // template class CachedValueImpl<String>; // redundant

  template bool CachedValueImpl<String>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<String>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<String>::getValueImpl(Real &) const;

  template bool CachedValueImpl<String>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<String>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<String>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<String>::getValueImpl(CommandHandleValue &) const;
  
  template bool CachedValueImpl<String>::getValuePointerImpl(Array const *&) const;
  template bool CachedValueImpl<String>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<String>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<String>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<String>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<BooleanArray>;

  template bool CachedValueImpl<BooleanArray>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(Real &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(String &) const;

  template bool CachedValueImpl<BooleanArray>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<BooleanArray>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<BooleanArray>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<BooleanArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<BooleanArray>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<BooleanArray>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<IntegerArray>;

  template bool CachedValueImpl<IntegerArray>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(Real &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(String &) const;

  template bool CachedValueImpl<IntegerArray>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<IntegerArray>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<IntegerArray>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<IntegerArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<IntegerArray>::getValuePointerImpl(RealArray const *&) const;
  template bool CachedValueImpl<IntegerArray>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<RealArray>;

  template bool CachedValueImpl<RealArray>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(Real &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(String &) const;

  template bool CachedValueImpl<RealArray>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<RealArray>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<RealArray>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<RealArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<RealArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<RealArray>::getValuePointerImpl(StringArray const *&) const;

  template class CachedValueImpl<StringArray>;

  template bool CachedValueImpl<StringArray>::getValueImpl(Boolean &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(Integer &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(Real &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(String &) const;

  template bool CachedValueImpl<StringArray>::getValueImpl(NodeState &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(NodeOutcome &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(FailureType &) const;
  template bool CachedValueImpl<StringArray>::getValueImpl(CommandHandleValue &) const;

  template bool CachedValueImpl<StringArray>::getValuePointerImpl(String const *&) const;
  template bool CachedValueImpl<StringArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool CachedValueImpl<StringArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool CachedValueImpl<StringArray>::getValuePointerImpl(RealArray const *&) const;

} // namespace PLEXIL
