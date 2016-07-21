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

#include "SetValueImpl.hh"

#include "Error.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{

  // Default methods
  template <typename T>
  void SetValueImpl<T>::setValueImpl(GetValue const &val)
  {
    if (val.isKnown()) {
      assertTrueMsg(val.valueType() == PlexilValueType<T>::value,
		    "setValue: can't assign a " << valueTypeName(val.valueType())
		    << " value to a " << PlexilValueType<T>::typeName
		    << " object");
      T temp;
      val.getValue(temp); // for effect
      this->setValueImpl(temp);
    }
    else
      this->setUnknown();
  }

  template <>
  void SetValueImpl<Real>::setValueImpl(GetValue const &val)
  {
    if (val.isKnown()) {
      assertTrueMsg(val.valueType() == REAL_TYPE
		    || val.valueType() == INTEGER_TYPE,
		    "setValue: can't assign a " << valueTypeName(val.valueType())
		    << " value to a Real object");
      Real temp;
      val.getValue(temp);
      this->setValueImpl(temp);
    }
    else
      this->setUnknown();
  }

  void SetValueImpl<String>::setValueImpl(GetValue const &val)
  {
    if (val.isKnown()) {
      assertTrueMsg(val.valueType() == STRING_TYPE,
		    "setValue: can't assign a " << valueTypeName(val.valueType())
		    << " value to a String object");
      String const *ptr;
      val.getValuePointer(ptr);
      this->setValueImpl(*ptr);
    }
    else
      this->setUnknown();
  }

  template <typename T>
  void SetValueImpl<ArrayImpl<T> >::setValueImpl(GetValue const &val)
  {
    if (val.isKnown()) {
      assertTrueMsg(val.valueType() == PlexilValueType<ArrayImpl<T> >::value,
		    "setValue: can't assign a " << valueTypeName(val.valueType())
		    << " value to a " << PlexilValueType<ArrayImpl<T> >::typeName
		    << " object");
      ArrayImpl<T> const *ptr;
      val.getValuePointer(ptr);
      this->setValueImpl(*ptr);
    }
    else
      this->setUnknown();
  }

  // Type mismatch methods

  template <typename T>
  template <typename U>
  void SetValueImpl<T>::setValueImpl(U const &val)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "setValue: can't assign a " << PlexilValueType<U>::typeName
		  << " value to a " << PlexilValueType<T>::typeName
		  << " object");
  }

  template <typename U>
  void SetValueImpl<String>::setValueImpl(U const &val)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "setValue: can't assign a " << PlexilValueType<U>::typeName
		  << " value to a String object");
  }

  template <typename T>
  template <typename U>
  void SetValueImpl<ArrayImpl<T> >::setValueImpl(U const &val)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "setValue: can't assign a " << PlexilValueType<U>::typeName
		  << " value to a " << PlexilValueType<ArrayImpl<T> >::typeName
		  << " object");
  }

  template <>
  template <>
  void SetValueImpl<Real>::setValueImpl(Integer const &val)
  {
    this->setValueImpl((Real) val);
  }

  template <typename T>
  template <typename U>
  bool SetValueImpl<T>::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getMutableValuePointer: can't get a " << PlexilValueType<U>::typeName
		  << " pointer from a " << PlexilValueType<T>::typeName
		  << " object");
    return false;
  }

  template <typename U>
  bool SetValueImpl<String>::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getMutableValuePointer: can't get a " << PlexilValueType<U>::typeName
		  << " pointer from a String object");
    return false;
  }

  template <typename T>
  template <typename U>
  bool SetValueImpl<ArrayImpl<T> >::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "getMutableValuePointer: can't get a " << PlexilValueType<U>::typeName
		  << " pointer from a " << PlexilValueType<ArrayImpl<T> >::typeName
		  << " object");
    return false;
  }

  template <typename T>
  bool SetValueImpl<ArrayImpl<T> >::getMutableValuePointerImpl(Array *&ptr)
  {
    ArrayImpl<T> *temp;
    bool result;
    if ((result = getMutableValuePointerImpl(temp)))
      ptr = dynamic_cast<Array *>(temp); // static_cast wasn't legal?!
    return result;
  }

  //
  // Explicit instantiations
  //
  template class SetValueImpl<Boolean>;
  template void SetValueImpl<Boolean>::setValueImpl(Integer const &);
  template void SetValueImpl<Boolean>::setValueImpl(Real const &);
  template void SetValueImpl<Boolean>::setValueImpl(NodeState const &);
  template void SetValueImpl<Boolean>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<Boolean>::setValueImpl(FailureType const &);
  template void SetValueImpl<Boolean>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<Boolean>::setValueImpl(String const &);
  template void SetValueImpl<Boolean>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<Boolean>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<Boolean>::setValueImpl(RealArray const &);
  template void SetValueImpl<Boolean>::setValueImpl(StringArray const &);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(Array *&);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<Boolean>::getMutableValuePointerImpl(StringArray *&);
  
  template class SetValueImpl<Integer>;
  template void SetValueImpl<Integer>::setValueImpl(Boolean const &);
  template void SetValueImpl<Integer>::setValueImpl(Real const &);
  template void SetValueImpl<Integer>::setValueImpl(NodeState const &);
  template void SetValueImpl<Integer>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<Integer>::setValueImpl(FailureType const &);
  template void SetValueImpl<Integer>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<Integer>::setValueImpl(String const &);
  template void SetValueImpl<Integer>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<Integer>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<Integer>::setValueImpl(RealArray const &);
  template void SetValueImpl<Integer>::setValueImpl(StringArray const &);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(Array *&);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<Integer>::getMutableValuePointerImpl(StringArray *&);

  template class SetValueImpl<Real>;
  template void SetValueImpl<Real>::setValueImpl(Boolean const &);
  template void SetValueImpl<Real>::setValueImpl(Integer const &);
  template void SetValueImpl<Real>::setValueImpl(NodeState const &);
  template void SetValueImpl<Real>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<Real>::setValueImpl(FailureType const &);
  template void SetValueImpl<Real>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<Real>::setValueImpl(String const &);
  template void SetValueImpl<Real>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<Real>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<Real>::setValueImpl(RealArray const &);
  template void SetValueImpl<Real>::setValueImpl(StringArray const &);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(Array *&);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<Real>::getMutableValuePointerImpl(StringArray *&);

  //template class SetValueImpl<String>; // already explicitly defined
  template void SetValueImpl<String>::setValueImpl(Boolean const &);
  template void SetValueImpl<String>::setValueImpl(Integer const &);
  template void SetValueImpl<String>::setValueImpl(Real const &);
  template void SetValueImpl<String>::setValueImpl(NodeState const &);
  template void SetValueImpl<String>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<String>::setValueImpl(FailureType const &);
  template void SetValueImpl<String>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<String>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<String>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<String>::setValueImpl(RealArray const &);
  template void SetValueImpl<String>::setValueImpl(StringArray const &);
  template bool SetValueImpl<String>::getMutableValuePointerImpl(Array *&);
  template bool SetValueImpl<String>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<String>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<String>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<String>::getMutableValuePointerImpl(StringArray *&);

  template class SetValueImpl<BooleanArray>;
  template void SetValueImpl<BooleanArray>::setValueImpl(Boolean const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(Integer const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(Real const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(NodeState const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(FailureType const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(String const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(RealArray const &);
  template void SetValueImpl<BooleanArray>::setValueImpl(StringArray const &);
  template bool SetValueImpl<BooleanArray>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<BooleanArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<BooleanArray>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<BooleanArray>::getMutableValuePointerImpl(StringArray *&);

  template class SetValueImpl<IntegerArray>;
  template void SetValueImpl<IntegerArray>::setValueImpl(Boolean const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(Integer const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(Real const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(NodeState const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(FailureType const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(String const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(RealArray const &);
  template void SetValueImpl<IntegerArray>::setValueImpl(StringArray const &);
  template bool SetValueImpl<IntegerArray>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<IntegerArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<IntegerArray>::getMutableValuePointerImpl(RealArray *&);
  template bool SetValueImpl<IntegerArray>::getMutableValuePointerImpl(StringArray *&);

  template class SetValueImpl<RealArray>;
  template void SetValueImpl<RealArray>::setValueImpl(Boolean const &);
  template void SetValueImpl<RealArray>::setValueImpl(Integer const &);
  template void SetValueImpl<RealArray>::setValueImpl(Real const &);
  template void SetValueImpl<RealArray>::setValueImpl(NodeState const &);
  template void SetValueImpl<RealArray>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<RealArray>::setValueImpl(FailureType const &);
  template void SetValueImpl<RealArray>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<RealArray>::setValueImpl(String const &);
  template void SetValueImpl<RealArray>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<RealArray>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<RealArray>::setValueImpl(StringArray const &);
  template bool SetValueImpl<RealArray>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<RealArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<RealArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<RealArray>::getMutableValuePointerImpl(StringArray *&);

  template class SetValueImpl<StringArray>;
  template void SetValueImpl<StringArray>::setValueImpl(Boolean const &);
  template void SetValueImpl<StringArray>::setValueImpl(Integer const &);
  template void SetValueImpl<StringArray>::setValueImpl(Real const &);
  template void SetValueImpl<StringArray>::setValueImpl(String const &);
  template void SetValueImpl<StringArray>::setValueImpl(NodeState const &);
  template void SetValueImpl<StringArray>::setValueImpl(NodeOutcome const &);
  template void SetValueImpl<StringArray>::setValueImpl(FailureType const &);
  template void SetValueImpl<StringArray>::setValueImpl(CommandHandleValue const &);
  template void SetValueImpl<StringArray>::setValueImpl(BooleanArray const &);
  template void SetValueImpl<StringArray>::setValueImpl(IntegerArray const &);
  template void SetValueImpl<StringArray>::setValueImpl(RealArray const &);
  template bool SetValueImpl<StringArray>::getMutableValuePointerImpl(String *&);
  template bool SetValueImpl<StringArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool SetValueImpl<StringArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool SetValueImpl<StringArray>::getMutableValuePointerImpl(RealArray *&);

  // These are currently unused

  // template class SetValueImpl<NodeState>;
  // template void SetValueImpl<NodeState>::setValueImpl(Boolean const &);
  // template void SetValueImpl<NodeState>::setValueImpl(Integer const &);
  // template void SetValueImpl<NodeState>::setValueImpl(Real const &);
  // template void SetValueImpl<NodeState>::setValueImpl(NodeOutcome const &);
  // template void SetValueImpl<NodeState>::setValueImpl(FailureType const &);
  // template void SetValueImpl<NodeState>::setValueImpl(CommandHandleValue const &);
  // template void SetValueImpl<NodeState>::setValueImpl(String const &);
  // template void SetValueImpl<NodeState>::setValueImpl(BooleanArray const &);
  // template void SetValueImpl<NodeState>::setValueImpl(IntegerArray const &);
  // template void SetValueImpl<NodeState>::setValueImpl(RealArray const &);
  // template void SetValueImpl<NodeState>::setValueImpl(StringArray const &);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(String *&);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(Array *&);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(BooleanArray *&);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(IntegerArray *&);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(RealArray *&);
  // template bool SetValueImpl<NodeState>::getMutableValuePointerImpl(StringArray *&);

  // template class SetValueImpl<NodeOutcome>;
  // template void SetValueImpl<NodeOutcome>::setValueImpl(Boolean const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(Integer const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(Real const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(NodeState const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(FailureType const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(CommandHandleValue const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(String const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(BooleanArray const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(IntegerArray const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(RealArray const &);
  // template void SetValueImpl<NodeOutcome>::setValueImpl(StringArray const &);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(String *&);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(Array *&);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(BooleanArray *&);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(IntegerArray *&);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(RealArray *&);
  // template bool SetValueImpl<NodeOutcome>::getMutableValuePointerImpl(StringArray *&);

  // template class SetValueImpl<FailureType>;
  // template void SetValueImpl<FailureType>::setValueImpl(Boolean const &);
  // template void SetValueImpl<FailureType>::setValueImpl(Integer const &);
  // template void SetValueImpl<FailureType>::setValueImpl(Real const &);
  // template void SetValueImpl<FailureType>::setValueImpl(NodeState const &);
  // template void SetValueImpl<FailureType>::setValueImpl(NodeOutcome const &);
  // template void SetValueImpl<FailureType>::setValueImpl(CommandHandleValue const &);
  // template void SetValueImpl<FailureType>::setValueImpl(String const &);
  // template void SetValueImpl<FailureType>::setValueImpl(BooleanArray const &);
  // template void SetValueImpl<FailureType>::setValueImpl(IntegerArray const &);
  // template void SetValueImpl<FailureType>::setValueImpl(RealArray const &);
  // template void SetValueImpl<FailureType>::setValueImpl(StringArray const &);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(String *&);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(Array *&);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(BooleanArray *&);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(IntegerArray *&);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(RealArray *&);
  // template bool SetValueImpl<FailureType>::getMutableValuePointerImpl(StringArray *&);

  // template class SetValueImpl<CommandHandleValue>;
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(Boolean const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(Integer const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(Real const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(NodeState const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(NodeOutcome const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(FailureType const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(String const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(BooleanArray const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(IntegerArray const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(RealArray const &);
  // template void SetValueImpl<CommandHandleValue>::setValueImpl(StringArray const &);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(String *&);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(Array *&);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(BooleanArray *&);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(IntegerArray *&);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(RealArray *&);
  // template bool SetValueImpl<CommandHandleValue>::getMutableValuePointerImpl(StringArray *&);

}
