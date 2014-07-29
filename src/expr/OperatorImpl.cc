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

#include "OperatorImpl.hh"
#include "Expression.hh"
#include "ArrayFwd.hh"

namespace PLEXIL
{
  // Default method for unspecialized types
  template <typename R>
  ValueType OperatorImpl<R>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  // Specific types
  template <>
  ValueType OperatorImpl<double>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  ValueType OperatorImpl<int32_t>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  ValueType OperatorImpl<bool>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  ValueType OperatorImpl<std::string>::valueType() const
  {
    return STRING_TYPE;
  }

  template <>
  ValueType OperatorImpl<BooleanArray>::valueType() const
  {
    return BOOLEAN_ARRAY_TYPE;
  }

  template <>
  ValueType OperatorImpl<IntegerArray>::valueType() const
  {
    return INTEGER_ARRAY_TYPE;
  }

  template <>
  ValueType OperatorImpl<RealArray>::valueType() const
  {
    return REAL_ARRAY_TYPE;
  }

  template <>
  ValueType OperatorImpl<StringArray>::valueType() const
  {
    return STRING_ARRAY_TYPE;
  }

  // Convenience methods

  template <typename R>
  bool OperatorImpl<R>::calcNative(void *cache, ExprVec const &exprs) const
  {
    return exprs.apply(this, *(static_cast<R *>(cache)));
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calcNative(void *cache, ExprVec const &exprs) const
  {
    return exprs.apply(*(static_cast<ArrayImpl<R> *>(cache)), this);
  }

  template <typename R>
  void OperatorImpl<R>::printValue(std::ostream &s, void *cache, ExprVec const &exprs) const
  {
    if (calcNative(cache, exprs))
      PLEXIL::printValue(*(static_cast<R const *>(cache)), s);
    else
      s << "UNKNOWN";
  }

  template <typename R>
  void OperatorImpl<ArrayImpl<R> >::printValue(std::ostream &s, void *cache, ExprVec const &exprs) const
  {
    if (calcNative(cache, exprs))
      PLEXIL::printValue(*(static_cast<ArrayImpl<R> const *>(cache)), s);
    else
      s << "UNKNOWN";
  }

  template <typename R>
  Value OperatorImpl<R>::toValue(void *cache, ExprVec const &exprs) const
  {
    bool known = calcNative(cache, exprs);
    if (known)
      return Value(*(static_cast<R const *>(cache)));
    else
      return Value();
  }

  template <typename R>
  Value OperatorImpl<ArrayImpl<R> >::toValue(void *cache, ExprVec const &exprs) const
  {
    bool known = calcNative(cache, exprs);
    if (known)
      return Value(*(static_cast<ArrayImpl<R> const *>(cache)));
    else
      return Value();
  }

  // Default methods
  template <typename R>
  bool OperatorImpl<R>::calc(R &result, Expression const *arg) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for one-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<R>::calc(R &result, Expression const *arg0, Expression const *arg1) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for two-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<R>::calc(R &result, ExprVec const &args) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for three or more arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> &result, Expression const *arg) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for one-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> &result, Expression const *arg0, Expression const *arg1) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for two-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> &result, ExprVec const &args) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for three or more arg case");
    return false;
  }

  // Conversion methods
 
  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, Expression const *arg) const
  {
    int32_t temp;
    if (!this->calc(temp, arg))
      return false;
    result = (double) temp;
    return true;
  }

  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, Expression const *arg0, Expression const *arg1) const
  {
    int32_t temp;
    if (!this->calc(temp, arg0, arg1))
      return false;
    result = (double) temp;
    return true;
  }

  template <>
  template <>
  bool OperatorImpl<int32_t>::calc(double &result, ExprVec const &args) const
  {
    int32_t temp;
    if (!this->calc(temp, args))
      return false;
    result = (double) temp;
    return true;
  }

  //
  // Explicit instantiations
  //

  template class OperatorImpl<double>;
  template class OperatorImpl<int32_t>;
  // template class OperatorImpl<uint16_t>;
  template class OperatorImpl<bool>;
  template class OperatorImpl<std::string>;

  // later?
  // template class OperatorImpl<BooleanArray>;
  // template class OperatorImpl<IntegerArray>;
  // template class OperatorImpl<RealArray>;
  // template class OperatorImpl<StringArray>;

} // namespace PLEXIL
