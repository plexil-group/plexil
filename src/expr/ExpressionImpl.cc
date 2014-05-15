/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

namespace PLEXIL
{
  // Default methods.
  template <typename T>
  bool ExpressionImpl<T>::isKnown() const
  {
    T const *dummy;
    return this->getValuePointerImpl(dummy);
  }

  // For immediate types
  template <>
  bool ExpressionImpl<bool>::isKnown() const
  {
    bool dummy;
    return this->getValueImpl(dummy);
  }

  template <>
  bool ExpressionImpl<uint16_t>::isKnown() const
  {
    uint16_t dummy;
    return this->getValueImpl(dummy);
  }

  template <>
  bool ExpressionImpl<int32_t>::isKnown() const
  {
    int32_t dummy;
    return this->getValueImpl(dummy);
  }

  template <>
  bool ExpressionImpl<double>::isKnown() const
  {
    double dummy;
    return this->getValueImpl(dummy);
  }

  // Default methods.
  template <typename T>
  const ValueType ExpressionImpl<T>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<bool>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<int32_t>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<double>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  const ValueType ExpressionImpl<std::string>::valueType() const
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
  template <typename U>
  bool ExpressionImpl<T>::getValueImpl(U &result) const
  {
    check_error_2(ALWAYS_FAIL, "getValue: value type error");
    return false;
  }

  // More conversions can be added as required.
  template <>
  template <>
  bool ExpressionImpl<int32_t>::getValueImpl(double &result) const
  {
    int32_t temp;
    if (!this->getValueImpl(temp))
      return false;
    result = (double) temp;
    return true;
  }

  // Report error for type mismatch
  template <typename T>
  template <typename U>
  bool ExpressionImpl<T>::getValuePointerImpl(U const *& /* ptr */) const
  {
    check_error_2(ALWAYS_FAIL, "getValuePointer: value type error");
    return false;
  }

  //
  // Explicit instantiations
  //
  template class ExpressionImpl<bool>;
  template class ExpressionImpl<uint16_t>;
  template class ExpressionImpl<int32_t>;
  template class ExpressionImpl<double>;
  template class ExpressionImpl<std::string>;

  template class ExpressionImpl<BooleanArray>;
  template class ExpressionImpl<IntegerArray>;
  template class ExpressionImpl<RealArray>;
  template class ExpressionImpl<StringArray>;

} // namespace PLEXIL
