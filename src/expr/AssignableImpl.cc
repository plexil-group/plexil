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

#include "AssignableImpl.hh"
#include "Error.hh"

namespace PLEXIL
{

  //
  // setValue wrappers
  //

  // Immediate types
  template <typename T>
  void AssignableImpl<T>::setValueImpl(Expression const *valex)
  {
    T value;
    if (valex->getValue(value))
      this->setValueImpl(value);
    else
      this->setUnknown();
  }

  template <typename T>
  void AssignableImpl<T>::setValueImpl(Value const &val)
  {
    T value;
    if (val.getValue(value))
      this->setValueImpl(value);
    else
      this->setUnknown();
  }

  // Special case for string
  void AssignableImpl<String>::setValueImpl(Expression const *valex)
  {
    String const *valptr;
    if (valex->getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  void AssignableImpl<String>::setValueImpl(Value const &val)
  {
    String const *valptr;
    if (val.getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  // Array types
  template <typename T>
  void AssignableImpl<ArrayImpl<T> >::setValueImpl(Expression const *valex)
  {
    ArrayImpl<T> const *valptr;
    if (valex->getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  template <typename T>
  void AssignableImpl<ArrayImpl<T> >::setValueImpl(Value const &val)
  {
    ArrayImpl<T> const *valptr;
    if (val.getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  // Generalized type mismatch
  template <typename T>
  template <typename U>
  void AssignableImpl<T>::setValueImpl(U const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  template <typename U>
  void AssignableImpl<String>::setValueImpl(U const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  template <typename T>
  template <typename U>
  void AssignableImpl<ArrayImpl<T> >::setValueImpl(U const &val)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  // Conversions
  void AssignableImpl<String>::setValueImpl(char const *val)
  {
    String const temp(val);
    this->setValueImpl(temp);
  }

  template <>
  template <>
  void AssignableImpl<Real>::setValueImpl(Integer const &val)
  {
    this->setValueImpl((Real) val);
  }

  // Specific type mismatch
  template <typename T>
  void AssignableImpl<T>::setValueImpl(char const * /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  template <typename T>
  void AssignableImpl<ArrayImpl<T> >::setValueImpl(char const * /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  //
  // getMutableValuePointerImpl
  //

  template <typename T>
  template <typename U>
  bool AssignableImpl<T>::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::getMutableValuePointer: type error");
    return false;
  }

  template <typename U>
  bool AssignableImpl<String>::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::getMutableValuePointer: type error");
    return false;
  }

  template <typename T>
  template <typename U>
  bool AssignableImpl<ArrayImpl<T> >::getMutableValuePointerImpl(U *& ptr)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::getMutableValuePointer: type error");
    return false;
  }

  template <typename T>
  bool AssignableImpl<ArrayImpl<T> >::getMutableValuePointerImpl(Array *&ptr)
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
  template class AssignableImpl<Boolean>;
  template void AssignableImpl<Boolean>::setValueImpl(Integer const &);
  template void AssignableImpl<Boolean>::setValueImpl(Real const &);
  template void AssignableImpl<Boolean>::setValueImpl(NodeState const &);
  template void AssignableImpl<Boolean>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<Boolean>::setValueImpl(FailureType const &);
  template void AssignableImpl<Boolean>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<Boolean>::setValueImpl(String const &);
  template void AssignableImpl<Boolean>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<Boolean>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<Boolean>::setValueImpl(RealArray const &);
  template void AssignableImpl<Boolean>::setValueImpl(StringArray const &);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<Boolean>::getMutableValuePointerImpl(StringArray *&);
  
  template class AssignableImpl<Integer>;
  template void AssignableImpl<Integer>::setValueImpl(Boolean const &);
  template void AssignableImpl<Integer>::setValueImpl(Real const &);
  template void AssignableImpl<Integer>::setValueImpl(NodeState const &);
  template void AssignableImpl<Integer>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<Integer>::setValueImpl(FailureType const &);
  template void AssignableImpl<Integer>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<Integer>::setValueImpl(String const &);
  template void AssignableImpl<Integer>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<Integer>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<Integer>::setValueImpl(RealArray const &);
  template void AssignableImpl<Integer>::setValueImpl(StringArray const &);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<Integer>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<Real>;
  template void AssignableImpl<Real>::setValueImpl(Boolean const &);
  template void AssignableImpl<Real>::setValueImpl(Integer const &);
  template void AssignableImpl<Real>::setValueImpl(NodeState const &);
  template void AssignableImpl<Real>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<Real>::setValueImpl(FailureType const &);
  template void AssignableImpl<Real>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<Real>::setValueImpl(String const &);
  template void AssignableImpl<Real>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<Real>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<Real>::setValueImpl(RealArray const &);
  template void AssignableImpl<Real>::setValueImpl(StringArray const &);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<Real>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<NodeState>;
  template void AssignableImpl<NodeState>::setValueImpl(Boolean const &);
  template void AssignableImpl<NodeState>::setValueImpl(Integer const &);
  template void AssignableImpl<NodeState>::setValueImpl(Real const &);
  template void AssignableImpl<NodeState>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<NodeState>::setValueImpl(FailureType const &);
  template void AssignableImpl<NodeState>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<NodeState>::setValueImpl(String const &);
  template void AssignableImpl<NodeState>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<NodeState>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<NodeState>::setValueImpl(RealArray const &);
  template void AssignableImpl<NodeState>::setValueImpl(StringArray const &);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<NodeState>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<NodeOutcome>;
  template void AssignableImpl<NodeOutcome>::setValueImpl(Boolean const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(Integer const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(Real const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(NodeState const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(FailureType const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(String const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(RealArray const &);
  template void AssignableImpl<NodeOutcome>::setValueImpl(StringArray const &);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<NodeOutcome>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<FailureType>;
  template void AssignableImpl<FailureType>::setValueImpl(Boolean const &);
  template void AssignableImpl<FailureType>::setValueImpl(Integer const &);
  template void AssignableImpl<FailureType>::setValueImpl(Real const &);
  template void AssignableImpl<FailureType>::setValueImpl(NodeState const &);
  template void AssignableImpl<FailureType>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<FailureType>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<FailureType>::setValueImpl(String const &);
  template void AssignableImpl<FailureType>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<FailureType>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<FailureType>::setValueImpl(RealArray const &);
  template void AssignableImpl<FailureType>::setValueImpl(StringArray const &);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<FailureType>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<CommandHandleValue>;
  template void AssignableImpl<CommandHandleValue>::setValueImpl(Boolean const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(Integer const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(Real const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(NodeState const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(FailureType const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(String const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(RealArray const &);
  template void AssignableImpl<CommandHandleValue>::setValueImpl(StringArray const &);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<CommandHandleValue>::getMutableValuePointerImpl(StringArray *&);

  //template class AssignableImpl<String>; // already explicitly defined
  template void AssignableImpl<String>::setValueImpl(Boolean const &);
  template void AssignableImpl<String>::setValueImpl(Integer const &);
  template void AssignableImpl<String>::setValueImpl(Real const &);
  template void AssignableImpl<String>::setValueImpl(NodeState const &);
  template void AssignableImpl<String>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<String>::setValueImpl(FailureType const &);
  template void AssignableImpl<String>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<String>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<String>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<String>::setValueImpl(RealArray const &);
  template void AssignableImpl<String>::setValueImpl(StringArray const &);
  template bool AssignableImpl<String>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<String>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<String>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<String>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<String>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<BooleanArray>;
  template void AssignableImpl<BooleanArray>::setValueImpl(Boolean const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(Integer const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(Real const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(NodeState const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(FailureType const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(String const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(RealArray const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<IntegerArray>;
  template void AssignableImpl<IntegerArray>::setValueImpl(Boolean const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(Integer const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(Real const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(NodeState const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(FailureType const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(String const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(RealArray const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<RealArray>;
  template void AssignableImpl<RealArray>::setValueImpl(Boolean const &);
  template void AssignableImpl<RealArray>::setValueImpl(Integer const &);
  template void AssignableImpl<RealArray>::setValueImpl(Real const &);
  template void AssignableImpl<RealArray>::setValueImpl(NodeState const &);
  template void AssignableImpl<RealArray>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<RealArray>::setValueImpl(FailureType const &);
  template void AssignableImpl<RealArray>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<RealArray>::setValueImpl(String const &);
  template void AssignableImpl<RealArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<RealArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<RealArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<StringArray>;
  template void AssignableImpl<StringArray>::setValueImpl(Boolean const &);
  template void AssignableImpl<StringArray>::setValueImpl(uint16_t const &);
  template void AssignableImpl<StringArray>::setValueImpl(Integer const &);
  template void AssignableImpl<StringArray>::setValueImpl(Real const &);
  template void AssignableImpl<StringArray>::setValueImpl(String const &);
  template void AssignableImpl<StringArray>::setValueImpl(NodeState const &);
  template void AssignableImpl<StringArray>::setValueImpl(NodeOutcome const &);
  template void AssignableImpl<StringArray>::setValueImpl(FailureType const &);
  template void AssignableImpl<StringArray>::setValueImpl(CommandHandleValue const &);
  template void AssignableImpl<StringArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<StringArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<StringArray>::setValueImpl(RealArray const &);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(String *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(RealArray *&);

} // namespace PLEXIL
