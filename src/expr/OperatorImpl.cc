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

#include "OperatorImpl.hh"

#include "allocateCache.hh"
#include "ArrayFwd.hh"
#include "Expression.hh"
#include "Function.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{
  template <typename R>
  ValueType OperatorImpl<R>::valueType() const
  {
    return PlexilValueType<R>::value;
  }

  template <typename R>
  ValueType OperatorImpl<ArrayImpl<R> >::valueType() const
  {
    return PlexilValueType<R>::arrayValue;
  }

  // Allocate small objects from a pool per type
  template <typename R>
  void *OperatorImpl<R>::allocateCache() const
  {
    return static_cast<void *>(PLEXIL::allocateCache<R>());
  }

  template <typename R>
  void OperatorImpl<R>::deleteCache(void *ptr) const
  {
    PLEXIL::deallocateCache(static_cast<R *>(ptr));
  }

  template <typename R>
  void *OperatorImpl<ArrayImpl<R> >::allocateCache() const
  {
    return static_cast<void *>(new ArrayImpl<R>);
  }

  template <typename R>
  void OperatorImpl<ArrayImpl<R> >::deleteCache(void *ptr) const
  {
    delete static_cast<ArrayImpl<R> *>(ptr);
  }

  // Convenience methods

  template <typename R>
  bool OperatorImpl<R>::calcNative(void *cache, Function const &f) const
  {
    return f.getValue(*(static_cast<R *>(cache)));
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calcNative(void *cache, Function const &f) const
  {
    return f.apply(this, *(static_cast<Array *>(cache)));
  }

  template <typename R>
  void OperatorImpl<R>::printValue(std::ostream &s, void *cache, Function const &exprs) const
  {
    if (calcNative(cache, exprs))
      PLEXIL::printValue(*(static_cast<R const *>(cache)), s);
    else
      s << "[unknown_value]";
  }

  template <typename R>
  void OperatorImpl<ArrayImpl<R> >::printValue(std::ostream &s, void *cache, Function const &exprs) const
  {
    if (calcNative(cache, exprs))
      PLEXIL::printValue(*(static_cast<ArrayImpl<R> const *>(cache)), s);
    else
      s << "[unknown_value]";
  }

  template <typename R>
  Value OperatorImpl<R>::toValue(void *cache, Function const &exprs) const
  {
    bool known = calcNative(cache, exprs);
    if (known)
      return Value(*(static_cast<R const *>(cache)));
    else
      return Value(0, PlexilValueType<R>::value);
  }

  template <typename R>
  Value OperatorImpl<ArrayImpl<R> >::toValue(void *cache, Function const &exprs) const
  {
    bool known = calcNative(cache, exprs);
    if (known)
      return Value(*(static_cast<ArrayImpl<R> const *>(cache)));
    else
      return Value(0, PlexilValueType<ArrayImpl<R> >::value);
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
  bool OperatorImpl<R>::calc(R &result, Function const &args) const
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
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> &result, Function const &args) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << this->getName() << " not implemented for three or more arg case");
    return false;
  }

  // Conversion methods
 
  template <>
  template <>
  bool OperatorImpl<Integer>::calc(Real &result, Expression const *arg) const
  {
    Integer temp;
    if (!this->calc(temp, arg))
      return false;
    result = (Real) temp;
    return true;
  }

  template <>
  template <>
  bool OperatorImpl<Integer>::calc(Real &result, Expression const *arg0, Expression const *arg1) const
  {
    Integer temp;
    if (!this->calc(temp, arg0, arg1))
      return false;
    result = (Real) temp;
    return true;
  }

  template <>
  template <>
  bool OperatorImpl<Integer>::calc(Real &result, Function const &args) const
  {
    Integer temp;
    if (!this->calc(temp, args))
      return false;
    result = (Real) temp;
    return true;
  }

  //
  // Explicit instantiations
  //

  template class OperatorImpl<Real>;
  template class OperatorImpl<Integer>;
  template class OperatorImpl<Boolean>;
  template class OperatorImpl<String>;

  // later?
  // template class OperatorImpl<BooleanArray>;
  // template class OperatorImpl<IntegerArray>;
  // template class OperatorImpl<RealArray>;
  // template class OperatorImpl<StringArray>;

} // namespace PLEXIL
