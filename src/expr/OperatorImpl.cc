/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#include "ArrayFwd.hh"
#include "Expression.hh"
#include "Function.hh"
#include "PlanError.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{

  template <typename R>
  OperatorImpl<R>::OperatorImpl(std::string const &name)
    : Operator(name)
  {
  }

  OperatorImpl<Integer>::OperatorImpl(std::string const &name)
    : Operator(name)
  {
  }

  template <typename R>
  OperatorImpl<ArrayImpl<R> >::OperatorImpl(std::string const &name)
    : Operator(name)
  {
  }

  template <typename R>
  ValueType OperatorImpl<R>::valueType() const
  {
    return PlexilValueType<R>::value;
  }

  ValueType OperatorImpl<Integer>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <typename R>
  ValueType OperatorImpl<ArrayImpl<R> >::valueType() const
  {
    return PlexilValueType<R>::arrayValue;
  }

  // Allocate small objects from a pool per type

  // Booleans, Integers, Reals, internal values don't use a cache
  template <typename R>
  void *OperatorImpl<R>::allocateCache() const
  {
    return NULL;
  }

  void *OperatorImpl<Integer>::allocateCache() const
  {
    return NULL;
  }

  // Strings, arrays need to allocate a cache for getValuePointer()
  template <>
  void *OperatorImpl<String>::allocateCache() const
  {
    return static_cast<void *>(new String);
  }

  template <typename R>
  void *OperatorImpl<ArrayImpl<R> >::allocateCache() const
  {
    return static_cast<void *>(new ArrayImpl<R>);
  }

  // Booleans, Integers, Reals, internal values don't use a cache
  template <typename R>
  void OperatorImpl<R>::deleteCache(void *ptr) const
  {
  }

  void OperatorImpl<Integer>::deleteCache(void *ptr) const
  {
  }

  // Strings, arrays need to allocate a cache for getValuePointer()
  template <>
  void OperatorImpl<String>::deleteCache(void *ptr) const
  {
    delete static_cast<String *>(ptr);
  }

  template <typename R>
  void OperatorImpl<ArrayImpl<R> >::deleteCache(void *ptr) const
  {
    delete static_cast<ArrayImpl<R> *>(ptr);
  }

  template <typename R>
  bool OperatorImpl<R>::operator()(R &result, Expression const *arg) const
  {
    return this->calc(result, arg);
  }

  bool OperatorImpl<Integer>::operator()(Integer &result, Expression const *arg) const
  {
    return this->calc(result, arg);
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::operator()(ArrayImpl<R> &result, Expression const *arg) const
  {
    return this->calc(result, arg);
  }

  template <typename R>
  bool OperatorImpl<R>::operator()(R &result,
                                   Expression const *arg0,
                                   Expression const *arg1) const
  {
    return this->calc(result, arg0, arg1);
  }

  bool OperatorImpl<Integer>::operator()(Integer &result,
                                         Expression const *arg0,
                                         Expression const *arg1) const
  {
    return this->calc(result, arg0, arg1);
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::operator()(ArrayImpl<R> &result,
                                               Expression const *arg0,
                                               Expression const *arg1) const
  {
    return this->calc(result, arg0, arg1);
  }

  template <typename R>
  bool OperatorImpl<R>::operator()(R &result, Function const &args) const
  {
    return this->calc(result, args);
  }

  bool OperatorImpl<Integer>::operator()(Integer &result, Function const &args) const
  {
    return this->calc(result, args);
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::operator()(ArrayImpl<R> &result, Function const &args) const
  {
    return this->calc(result, args);
  }

  //
  // Conversion methods
  //

  bool OperatorImpl<Integer>::operator()(Real &result, Expression const *arg) const
  {
    Integer temp;
    if (!this->calc(temp, arg))
      return false;
    result = (Real) temp;
    return true;
  }

  bool OperatorImpl<Integer>::operator()(Real &result,
                                         Expression const *arg0,
                                         Expression const *arg1) const
  {
    Integer temp;
    if (!this->calc(temp, arg0, arg1))
      return false;
    result = (Real) temp;
    return true;
  }

  bool OperatorImpl<Integer>::operator()(Real &result, Function const &args) const
  {
    Integer temp;
    if (!this->calc(temp, args))
      return false;
    result = (Real) temp;
    return true;
  }

  // Convenience methods

  template <typename R>
  bool OperatorImpl<R>::isKnown(Function const &func) const
  {
    R dummy;
    // N.B.: We do this roundabout call back to the function
    // so the function can dispatch to the appropriate calc() method
    // based on its argument count. See Function.cc.
    return func.getValue(dummy);
  }

  bool OperatorImpl<Integer>::isKnown(Function const &func) const
  {
    Integer dummy;
    return func.getValue(dummy);
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::isKnown(Function const &func) const
  {
    ArrayImpl<R> dummy;
    return func.getValue(dummy);
  }

  template <typename R>
  void OperatorImpl<R>::printValue(std::ostream &s, Function const &exprs) const
  {
    R temp;
    if (exprs.getValue(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  void OperatorImpl<Integer>::printValue(std::ostream &s, Function const &exprs) const
  {
    Integer temp;
    if (exprs.getValue(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename R>
  void OperatorImpl<ArrayImpl<R> >::printValue(std::ostream &s, Function const &exprs) const
  {
    ArrayImpl<R> temp;
    if (exprs.getValue(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  template <typename R>
  Value OperatorImpl<R>::toValue(Function const &exprs) const
  {
    R temp;
    if (exprs.getValue(temp))
      return Value(temp);
    else
      return Value(0, PlexilValueType<R>::value);
  }

  Value OperatorImpl<Integer>::toValue(Function const &exprs) const
  {
    Integer temp;
    if (exprs.getValue(temp))
      return Value(temp);
    else
      return Value(0, INTEGER_TYPE);
  }

  template <typename R>
  Value OperatorImpl<ArrayImpl<R> >::toValue(Function const &exprs) const
  {
    ArrayImpl<R> temp;
    if (exprs.getValue(temp))
      return Value(temp);
    else
      return Value(0, PlexilValueType<ArrayImpl<R> >::value);
  }

  // Default methods
  template <typename R>
  bool OperatorImpl<R>::calc(R & /* result */, Expression const * /* arg */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for one-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<R>::calc(R & /* result */,
                             Expression const * /* arg0 */,
                             Expression const * /* arg1 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for two-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<R>::calc(R & /* result */, Function const & /* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for three or more arg case");
    return false;
  }

  bool OperatorImpl<Integer>::calc(Integer & /* result */, Expression const */* arg */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for one-arg case");
    return false;
  }

  bool OperatorImpl<Integer>::calc(Integer & /* result */,
                                   Expression const * /* arg1 */,
                                   Expression const * /* arg2 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for one-arg case");
    return false;
  }

  bool OperatorImpl<Integer>::calc(Integer & /* result */, Function const &/* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for one-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> & /* result */,
                                         Expression const * /* arg */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for one-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> & /* result */,
                                         Expression const * /* arg0 */,
                                         Expression const * /* arg1 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for two-arg case");
    return false;
  }

  template <typename R>
  bool OperatorImpl<ArrayImpl<R> >::calc(ArrayImpl<R> & /* result */,
                                         Function const & /* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " not implemented for three or more arg case");
    return false;
  }

  //
  // Explicit instantiations
  //

  template class OperatorImpl<Real>;
  // template class OperatorImpl<Integer>; // redundant
  template class OperatorImpl<Boolean>;
  template class OperatorImpl<String>;

  // later?
  // template class OperatorImpl<BooleanArray>;
  // template class OperatorImpl<IntegerArray>;
  // template class OperatorImpl<RealArray>;
  // template class OperatorImpl<StringArray>;

} // namespace PLEXIL
