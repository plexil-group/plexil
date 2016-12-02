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

  /**
   * @brief Update the cache entry with the given new value.
   * @param timestamp Sequence number.
   * @param val The new value.
   * @note The caller is responsible for deleting the object pointed to upon return.
   */
  bool VoidCachedValue::update(unsigned int /* timestamp */, Boolean const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }
      
  bool VoidCachedValue::update(unsigned int /* timestamp */, Integer const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::update(unsigned int /* timestamp */, Real const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::update(unsigned int /* timestamp */, String const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, String const *valPtr)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, BooleanArray const *valPtr)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, IntegerArray const *valPtr)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, RealArray const *valPtr)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::updatePtr(unsigned int /* timestamp */, StringArray const *valPtr)
  {
    assertTrue_2(ALWAYS_FAIL, "Can't update a VoidCachedValue");
    return false;
  }

  bool VoidCachedValue::update(unsigned int timestamp, Value const &val)
  {
    assertTrue_2(!val.isKnown(), "Can't update a VoidCachedValue");
    this->m_timestamp = timestamp;
    return true;
  }

  //
  // Typed implementation
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

  template <typename T>
  CachedValueImpl<T>::CachedValueImpl(CachedValueImpl<T> const &orig)
    : CachedValueShim<CachedValueImpl<T> >(),
      m_value(orig.m_value),
      m_known(orig.m_known)
  {
    this->m_timestamp = orig.getTimestamp();
  }

  CachedValueImpl<String>::CachedValueImpl(CachedValueImpl<String> const &orig)
    : CachedValueShim<CachedValueImpl<String> >(),
      m_value(orig.m_value),
      m_known(orig.m_known)
  {
    this->m_timestamp = orig.getTimestamp();
  }

  template <typename T>
  CachedValueImpl<ArrayImpl<T> >::CachedValueImpl(CachedValueImpl<ArrayImpl<T> > const &orig)
    : CachedValueShim<CachedValueImpl<ArrayImpl<T> > >(),
      m_value(orig.m_value),
      m_known(orig.m_known)
  {
    this->m_timestamp = orig.getTimestamp();
  }

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

  template <typename T>
  CachedValue &CachedValueImpl<T>::operator=(CachedValue const &other)
  {
    CachedValueImpl<T> const *otherPtr = 
      dynamic_cast<CachedValueImpl<T> const * > (&other);
    checkInterfaceError(otherPtr,
                        "Attempt to assign CachedValue of type "
                        << valueTypeName(PlexilValueType<T>::value)
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

  template <typename T>
  ValueType CachedValueImpl<T>::valueType() const
  {
    return PlexilValueType<T>::value;;
  }

  ValueType CachedValueImpl<String>::valueType() const
  {
    return STRING_TYPE;
  }

  template <typename T>
  ValueType CachedValueImpl<ArrayImpl<T> >::valueType() const
  {
    return PlexilValueType<T>::arrayValue;
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

  // Specialization for Real, solely for debug printing purposes.
  template <>
  bool CachedValueImpl<Real>::updateImpl(unsigned int timestamp, Real const &val)
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

  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, String const &val)
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
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::value)
                        << " CachedValue with a " << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, U const & /* val */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a String CachedValue with a "
                        << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::updateImpl(unsigned int timestamp, U const & /* val */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " CachedValue with a " << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  // Type conversion method.
  template <>
  template <>
  bool CachedValueImpl<Real>::updateImpl(unsigned int timestamp, Integer const &val)
  {
    return this->updateImpl(timestamp, (Real) val);
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
               << valueTypeName(PlexilValueType<T>::value) << " lookup");
      return setUnknown(timestamp);
    }
  }

  // Special case for string
  bool CachedValueImpl<String>::updateImpl(unsigned int timestamp, Value const &val)
  {
    String const *valPtr;
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
               << valueTypeName(PlexilValueType<T>::arrayValue) << " lookup");
      return setUnknown(timestamp);
    }
  }

//
// updatePtrImpl
//

bool CachedValueImpl<String>::updatePtrImpl(unsigned int timestamp, String const *ptr)
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
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::value)
                        << " CachedValue with a " << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::updatePtrImpl(unsigned int /* timestamp */, U const * /* ptr */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a String CachedValue with a "
                        << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::updatePtrImpl(unsigned int /* timestamp */, U const * /* ptr */)
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to update a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " CachedValue with a " << valueTypeName(PlexilValueType<U>::value)); 
    return false;
  }

  //
  // getValue() and friends
  //

  template <typename T>
  bool CachedValueImpl<T>::getValueImpl(T &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  bool CachedValueImpl<String>::getValueImpl(String &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  // Default wrong-type methods.
  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::getValueImpl(U & /* result */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a " << valueTypeName(PlexilValueType<T>::value)
                        << " valued Lookup"); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::getValueImpl(U & /* result */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a String valued lookup"); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::getValueImpl(U & /* result */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " valued Lookup"); 
    return false;
  }

  // Conversion method
  template <>
  template <>
  bool CachedValueImpl<Integer>::getValueImpl(Real &result) const
  {
    if (m_known)
      result = (Real) m_value;
    return m_known;
  }

  // Same-type methods
  bool CachedValueImpl<String>::getValuePointerImpl(String const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(ArrayImpl<T> const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  // Default wrong-type methods.
  template <typename T>
  template <typename U>
  bool CachedValueImpl<T>::getValuePointerImpl(U const *& /* ptr */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a " << valueTypeName(PlexilValueType<T>::value)
                        << " valued Lookup"); 
    return false;
  }

  template <typename U>
  bool CachedValueImpl<String>::getValuePointerImpl(U const *& /* ptr */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a String valued lookup"); 
    return false;
  }

  template <typename T>
  template <typename U>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(U const *& /* ptr */) const
  {
    checkInterfaceError(ALWAYS_FAIL,
                        "Attempt to get a " << valueTypeName(PlexilValueType<U>::value)
                        << " value from a " << valueTypeName(PlexilValueType<T>::arrayValue)
                        << " valued Lookup"); 
    return false;
  }

  // Conversion method
  template <typename T>
  bool CachedValueImpl<ArrayImpl<T> >::getValuePointerImpl(Array const *& ptr) const
  {
    if (m_known)
      ptr = dynamic_cast<Array const *>(&m_value);
    return m_known;
  }

  template <typename T>
  Value CachedValueImpl<T>::toValue() const
  {
    if (m_known)
      return Value(m_value);
    else
      return Value(0, valueType());
  }

  Value CachedValueImpl<String>::toValue() const
  {
    if (m_known)
      return Value(m_value);
    else
      return Value(0, valueType());
  }

  template <typename T>
  Value CachedValueImpl<ArrayImpl<T> >::toValue() const
  {
    if (m_known)
      return Value(m_value);
    else
      return Value(0, valueType());
  }

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
  template class CachedValueImpl<Integer>;
  template class CachedValueImpl<Real>;
// template class CachedValueImpl<String>; // redundant
  template class CachedValueImpl<BooleanArray>;
  template class CachedValueImpl<IntegerArray>;
  template class CachedValueImpl<RealArray>;
  template class CachedValueImpl<StringArray>;

} // namespace PLEXIL
