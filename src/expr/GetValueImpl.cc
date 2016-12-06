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

#include "GetValueImpl.hh"

#include "ArrayImpl.hh"
#include "Error.hh"
#include "PlexilTypeTraits.hh"
#include "Value.hh"

namespace PLEXIL
{
  template <typename T>
  ValueType GetValueImpl<T>::valueType() const
  {
    return PlexilValueType<T>::value;
  }

  ValueType GetValueImpl<String>::valueType() const
  {
    return PlexilValueType<String>::value;
  }

  template <typename T>
  ValueType GetValueImpl<ArrayImpl<T> >::valueType() const
  {
    return PlexilValueType<ArrayImpl<T> >::value;
  }

  template <typename T>
  bool GetValueImpl<T>::isKnown() const
  {
    T dummy;
    return this->getValueImpl(dummy);
  }

  bool GetValueImpl<String>::isKnown() const
  {
    String const *dummy;
    return this->getValuePointerImpl(dummy);
  }

  template <typename T>
  bool GetValueImpl<ArrayImpl<T> >::isKnown() const
  {
    ArrayImpl<T> const *dummy;
    return this->getValuePointerImpl(dummy);
  }


  template <typename T>
  Value GetValueImpl<T>::toValue() const
  {
    T temp;
    bool known = this->getValueImpl(temp);
    if (known)
      return Value(temp);
    else
      return Value(0, this->valueType());
  }

  Value GetValueImpl<String>::toValue() const
  {
    std::string const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }

  template <typename T>
  Value GetValueImpl<ArrayImpl<T> >::toValue() const
  {
    ArrayImpl<T> const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }

  template <typename T>
  template <typename U>
  bool GetValueImpl<T>::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValue: trying to get a " << PlexilValueType<U>::typeName
		  << " value from a " << PlexilValueType<T>::typeName << " typed object");
    return false;
  }

  template <typename U>
  bool GetValueImpl<String>::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValue: trying to get a " << PlexilValueType<U>::typeName
		  << " value from a " << PlexilValueType<String>::typeName << " typed object");
    return false;
  }

  template <typename T>
  bool GetValueImpl<ArrayImpl<T> >::getValueImpl(ArrayImpl<T> &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValue: not implemented for "
		  << PlexilValueType<ArrayImpl<T> >::typeName
		  << " typed objects");
    return false;
  }

  template <typename T>
  template <typename U>
  bool GetValueImpl<ArrayImpl<T> >::getValueImpl(U &result) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValue: trying to get a " << PlexilValueType<U>::typeName
		  << " value from a " << PlexilValueType<ArrayImpl<T> >::typeName << " typed object");
    return false;
  }

  // More conversions can be added as required.
  template <>
  template <>
  bool GetValueImpl<Integer>::getValueImpl(Real &result) const
  {
    Integer temp;
    if (!this->getValueImpl(temp))
      return false;
    result = (Real) temp;
    return true;
  }

  // Report error for scalar types
  template <typename T>
  bool GetValueImpl<T>::getValuePointerImpl(T const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValuePointer not implemented for "
		  << PlexilValueType<T>::typeName
		  << " typed objects");
    return false;
  }

  template <typename T>
  template <typename U>
  bool GetValueImpl<T>::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
		  << " pointer value from a " << PlexilValueType<T>::typeName << " typed object");
    return false;
  }

  // Report error for string types
  template <typename U>
  bool GetValueImpl<String>::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
		  << " pointer value from a " << PlexilValueType<String>::typeName << " typed object");
    return false;
  }

  // Downcast default for arrays
  template <typename T>
  bool GetValueImpl<ArrayImpl<T> >::getValuePointerImpl(Array const *&ptr) const
  {
    ArrayImpl<T> const *temp;
    if (!this->getValuePointerImpl(temp))
      return false;
    ptr = static_cast<Array const *>(temp);
    return true;
  }

  // Report error for type mismatch
  template <typename T>
  template <typename U>
  bool GetValueImpl<ArrayImpl<T> >::getValuePointerImpl(U const *& /* ptr */) const
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getValuePointer: trying to get a " << PlexilValueType<U>::typeName
		  << " pointer value from a " << PlexilValueType<ArrayImpl<T> >::typeName << " typed object");
    return false;
  }

  template <typename T>
  void GetValueImpl<T>::printValue(std::ostream &s) const
  {
    T val;
    if (this->getValue(val))
      PLEXIL::printValue(val, s);
    else
      s << "UNKNOWN"; 
  }

  void GetValueImpl<String>::printValue(std::ostream &s) const
  {
    String const *val;
    if (this->getValuePointer(val))
      PLEXIL::printValue(*val, s);
    else
      s << "UNKNOWN"; 
  }

  template <typename T>
  void GetValueImpl<ArrayImpl<T> >::printValue(std::ostream &s) const
  {
    ArrayImpl<T> const *val;
    if (this->getValuePointer(val))
      PLEXIL::printValue(*val, s);
    else
      s << "UNKNOWN"; 
  }

  // Explicit instantiations
  template class GetValueImpl<Boolean>;

  template bool GetValueImpl<Boolean>::getValueImpl(Integer &) const;
  template bool GetValueImpl<Boolean>::getValueImpl(Real &) const;
  template bool GetValueImpl<Boolean>::getValueImpl(String &) const;

  template bool GetValueImpl<Boolean>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<Boolean>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<Boolean>::getValuePointerImpl(Array const *&) const;
  template bool GetValueImpl<Boolean>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<Boolean>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<Boolean>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<Boolean>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<Integer>;

  template bool GetValueImpl<Integer>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<Integer>::getValueImpl(String &) const;

  template bool GetValueImpl<Integer>::getValueImpl(NodeState &) const;
  template bool GetValueImpl<Integer>::getValueImpl(NodeOutcome &) const;
  template bool GetValueImpl<Integer>::getValueImpl(FailureType &) const;
  template bool GetValueImpl<Integer>::getValueImpl(CommandHandleValue &) const;

  template bool GetValueImpl<Integer>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<Integer>::getValuePointerImpl(Array const *&) const;
  template bool GetValueImpl<Integer>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<Integer>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<Integer>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<Integer>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<Real>;

  template bool GetValueImpl<Real>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<Real>::getValueImpl(Integer &) const;
  template bool GetValueImpl<Real>::getValueImpl(String &) const;

  template bool GetValueImpl<Real>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<Real>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<Real>::getValuePointerImpl(Array const *&) const;
  template bool GetValueImpl<Real>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<Real>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<Real>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<Real>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<uint16_t>;

  template bool GetValueImpl<NodeState>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<NodeState>::getValueImpl(Integer &) const;
  template bool GetValueImpl<NodeState>::getValueImpl(Real &) const;
  template bool GetValueImpl<NodeState>::getValueImpl(String &) const;

  template bool GetValueImpl<NodeState>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<NodeState>::getValuePointerImpl(Array const *&) const;
  template bool GetValueImpl<NodeState>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<NodeState>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<NodeState>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<NodeState>::getValuePointerImpl(StringArray const *&) const;

  // template class GetValueImpl<String>;

  template bool GetValueImpl<String>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<String>::getValueImpl(Integer &) const;
  template bool GetValueImpl<String>::getValueImpl(Real &) const;

  template bool GetValueImpl<String>::getValueImpl(uint16_t &) const;
  
  template bool GetValueImpl<String>::getValuePointerImpl(Array const *&) const;
  template bool GetValueImpl<String>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<String>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<String>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<String>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<BooleanArray>;

  template bool GetValueImpl<BooleanArray>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<BooleanArray>::getValueImpl(Integer &) const;
  template bool GetValueImpl<BooleanArray>::getValueImpl(Real &) const;
  template bool GetValueImpl<BooleanArray>::getValueImpl(String &) const;

  template bool GetValueImpl<BooleanArray>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<BooleanArray>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<BooleanArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<BooleanArray>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<BooleanArray>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<IntegerArray>;

  template bool GetValueImpl<IntegerArray>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<IntegerArray>::getValueImpl(Integer &) const;
  template bool GetValueImpl<IntegerArray>::getValueImpl(Real &) const;
  template bool GetValueImpl<IntegerArray>::getValueImpl(String &) const;

  template bool GetValueImpl<IntegerArray>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<IntegerArray>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<IntegerArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<IntegerArray>::getValuePointerImpl(RealArray const *&) const;
  template bool GetValueImpl<IntegerArray>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<RealArray>;

  template bool GetValueImpl<RealArray>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<RealArray>::getValueImpl(Integer &) const;
  template bool GetValueImpl<RealArray>::getValueImpl(Real &) const;
  template bool GetValueImpl<RealArray>::getValueImpl(String &) const;

  template bool GetValueImpl<RealArray>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<RealArray>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<RealArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<RealArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<RealArray>::getValuePointerImpl(StringArray const *&) const;

  template class GetValueImpl<StringArray>;

  template bool GetValueImpl<StringArray>::getValueImpl(Boolean &) const;
  template bool GetValueImpl<StringArray>::getValueImpl(Integer &) const;
  template bool GetValueImpl<StringArray>::getValueImpl(Real &) const;
  template bool GetValueImpl<StringArray>::getValueImpl(String &) const;

  template bool GetValueImpl<StringArray>::getValueImpl(uint16_t &) const;

  template bool GetValueImpl<StringArray>::getValuePointerImpl(String const *&) const;
  template bool GetValueImpl<StringArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool GetValueImpl<StringArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool GetValueImpl<StringArray>::getValuePointerImpl(RealArray const *&) const;

} // namespace PLEXIL
