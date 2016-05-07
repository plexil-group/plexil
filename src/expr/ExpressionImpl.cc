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

#include "ExpressionImpl.hh"

#include "ArrayImpl.hh"
#include "Error.hh"
#include "Value.hh"

namespace PLEXIL
{
  // Default methods.
  template <typename T>
  bool ExpressionImpl<T>::isKnown() const
  {
    T dummy;
    return this->getValueImpl(dummy);
  }

  bool ExpressionImpl<String>::isKnown() const
  {
    std::string const *dummy;
    return this->getValuePointerImpl(dummy);
  }

  template <typename T>
  bool ExpressionImpl<ArrayImpl<T> >::isKnown() const
  {
    ArrayImpl<T> const *dummy;
    return this->getValuePointerImpl(dummy);
  }

  // Default methods.
  template <typename T>
  const ValueType ExpressionImpl<T>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<Boolean>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<Integer>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<Real>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<NodeState>::valueType() const
  {
    return NODE_STATE_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<NodeOutcome>::valueType() const
  {
    return OUTCOME_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<FailureType>::valueType() const
  {
    return FAILURE_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<CommandHandleValue>::valueType() const
  {
    return COMMAND_HANDLE_TYPE;
  }

  const ValueType ExpressionImpl<String>::valueType() const
  {
    return STRING_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<BooleanArray>::valueType() const
  {
    return BOOLEAN_ARRAY_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<IntegerArray>::valueType() const
  {
    return INTEGER_ARRAY_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<RealArray>::valueType() const
  {
    return REAL_ARRAY_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<StringArray>::valueType() const
  {
    return STRING_ARRAY_TYPE;
  }

  /**
   * @brief Print the expression's value to the given stream.
   * @param s The output stream.
   * @note Default method, specializations should be implemented as appropriate.
   */
  template <typename T>
  void ExpressionImpl<T>::printValue(std::ostream &s) const
  {
    T temp;
    if (this->getValueImpl(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename T>
  void ExpressionImpl<ArrayImpl<T> >::printValue(std::ostream &s) const
  {
    ArrayImpl<T> const *temp;
    if (this->getValuePointerImpl(temp))
      PLEXIL::printValue(*temp, s);
    else
      s << "UNKNOWN";
  }

  void ExpressionImpl<String>::printValue(std::ostream &s) const
  {
    String const *temp;
    if (this->getValuePointerImpl(temp))
      PLEXIL::printValue(*temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename T>
  template <typename U>
  bool ExpressionImpl<T>::getValueImpl(U &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue: value type error");
    return false;
  }

  template <typename U>
  bool ExpressionImpl<String>::getValueImpl(U &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue: value type error");
    return false;
  }

  // Report error for array types
  template <typename T>
  bool ExpressionImpl<ArrayImpl<T> >::getValueImpl(ArrayImpl<T> &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue not implemented for array types");
    return false;
  }

  template <typename T>
  template <typename U>
  bool ExpressionImpl<ArrayImpl<T> >::getValueImpl(U &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue: value type error");
    return false;
  }

  // More conversions can be added as required.
  template <>
  template <>
  bool ExpressionImpl<Integer>::getValueImpl(Real &result) const
  {
    Integer temp;
    if (!this->getValueImpl(temp))
      return false;
    result = (Real) temp;
    return true;
  }

  // Report error for scalar types
  template <typename T>
  bool ExpressionImpl<T>::getValuePointerImpl(T const *& /* ptr */) const
  {
    check_error_2(ALWAYS_FAIL, "getValuePointer not implemented for this type");
    return false;
  }

  template <typename T>
  template <typename U>
  bool ExpressionImpl<T>::getValuePointerImpl(U const *& /* ptr */) const
  {
    check_error_2(ALWAYS_FAIL, "getValuePointer: value type error");
    return false;
  }

  // Report error for string types
  template <typename U>
  bool ExpressionImpl<String>::getValuePointerImpl(U const *& /* ptr */) const
  {
    check_error_2(ALWAYS_FAIL, "getValuePointer: value type error");
    return false;
  }

  // Downcast default for arrays
  template <typename T>
  bool ExpressionImpl<ArrayImpl<T> >::getValuePointerImpl(Array const *&ptr) const
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
  bool ExpressionImpl<ArrayImpl<T> >::getValuePointerImpl(U const *& /* ptr */) const
  {
    check_error_2(ALWAYS_FAIL, "getValuePointer: value type error");
    return false;
  }

  template <typename T>
  Value ExpressionImpl<T>::toValue() const
  {
    T temp;
    bool known = this->getValueImpl(temp);
    if (known)
      return Value(temp);
    else
      return Value(0, this->valueType());
  }

  template <typename T>
  Value ExpressionImpl<ArrayImpl<T> >::toValue() const
  {
    ArrayImpl<T> const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }

  Value ExpressionImpl<String>::toValue() const
  {
    std::string const *ptr;
    bool known = this->getValuePointerImpl(ptr);
    if (known)
      return Value(*ptr);
    else
      return Value(0, this->valueType());
  }


  //
  // Explicit instantiations
  //
  template class ExpressionImpl<Boolean>;

  template bool ExpressionImpl<Boolean>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<Boolean>::getValueImpl(Real &) const;
  template bool ExpressionImpl<Boolean>::getValueImpl(String &) const;

  template bool ExpressionImpl<Boolean>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<Boolean>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<Boolean>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<Boolean>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<Boolean>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<Boolean>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<Boolean>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<Boolean>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<Boolean>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<Boolean>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<Integer>;

  template bool ExpressionImpl<Integer>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<Integer>::getValueImpl(String &) const;

  template bool ExpressionImpl<Integer>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<Integer>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<Integer>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<Integer>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<Integer>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<Integer>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<Integer>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<Integer>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<Integer>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<Integer>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<Real>;

  template bool ExpressionImpl<Real>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<Real>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<Real>::getValueImpl(String &) const;

  template bool ExpressionImpl<Real>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<Real>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<Real>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<Real>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<Real>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<Real>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<Real>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<Real>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<Real>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<Real>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<NodeState>;

  template bool ExpressionImpl<NodeState>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<NodeState>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<NodeState>::getValueImpl(Real &) const;
  template bool ExpressionImpl<NodeState>::getValueImpl(String &) const;

  template bool ExpressionImpl<NodeState>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<NodeState>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<NodeState>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<NodeState>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<NodeState>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<NodeState>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<NodeState>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<NodeState>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<NodeState>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<NodeOutcome>;

  template bool ExpressionImpl<NodeOutcome>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<NodeOutcome>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<NodeOutcome>::getValueImpl(Real &) const;
  template bool ExpressionImpl<NodeOutcome>::getValueImpl(String &) const;

  template bool ExpressionImpl<NodeOutcome>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<NodeOutcome>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<NodeOutcome>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<NodeOutcome>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<FailureType>;

  template bool ExpressionImpl<FailureType>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<FailureType>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<FailureType>::getValueImpl(Real &) const;
  template bool ExpressionImpl<FailureType>::getValueImpl(String &) const;

  template bool ExpressionImpl<FailureType>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<FailureType>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<FailureType>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<FailureType>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<FailureType>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<FailureType>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<FailureType>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<FailureType>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<FailureType>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<CommandHandleValue>;

  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(Real &) const;
  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(String &) const;

  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<CommandHandleValue>::getValueImpl(FailureType &) const;

  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<CommandHandleValue>::getValuePointerImpl(StringArray const *&) const;

  //template class ExpressionImpl<String>; // explicitly defined in header file

  template bool ExpressionImpl<String>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<String>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<String>::getValueImpl(Real &) const;

  template bool ExpressionImpl<String>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<String>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<String>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<String>::getValueImpl(CommandHandleValue &) const;
  
  template bool ExpressionImpl<String>::getValuePointerImpl(Array const *&) const;
  template bool ExpressionImpl<String>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<String>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<String>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<String>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<BooleanArray>;

  template bool ExpressionImpl<BooleanArray>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(Real &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(String &) const;

  template bool ExpressionImpl<BooleanArray>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<BooleanArray>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<BooleanArray>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<BooleanArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<BooleanArray>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<BooleanArray>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<IntegerArray>;

  template bool ExpressionImpl<IntegerArray>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(Real &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(String &) const;

  template bool ExpressionImpl<IntegerArray>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<IntegerArray>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<IntegerArray>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<IntegerArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<IntegerArray>::getValuePointerImpl(RealArray const *&) const;
  template bool ExpressionImpl<IntegerArray>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<RealArray>;

  template bool ExpressionImpl<RealArray>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(Real &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(String &) const;

  template bool ExpressionImpl<RealArray>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<RealArray>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<RealArray>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<RealArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<RealArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<RealArray>::getValuePointerImpl(StringArray const *&) const;

  template class ExpressionImpl<StringArray>;

  template bool ExpressionImpl<StringArray>::getValueImpl(Boolean &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(Integer &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(Real &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(String &) const;

  template bool ExpressionImpl<StringArray>::getValueImpl(NodeState &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(NodeOutcome &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(FailureType &) const;
  template bool ExpressionImpl<StringArray>::getValueImpl(CommandHandleValue &) const;

  template bool ExpressionImpl<StringArray>::getValuePointerImpl(String const *&) const;
  template bool ExpressionImpl<StringArray>::getValuePointerImpl(BooleanArray const *&) const;
  template bool ExpressionImpl<StringArray>::getValuePointerImpl(IntegerArray const *&) const;
  template bool ExpressionImpl<StringArray>::getValuePointerImpl(RealArray const *&) const;


} // namespace PLEXIL
