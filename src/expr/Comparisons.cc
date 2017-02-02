/* Copyright (c) 2006-2017, Universities Space Research Association (USRA).
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

#include "Comparisons.hh"

#include "Function.hh"
#include "PlexilTypeTraits.hh"

namespace PLEXIL
{

  //
  // IsKnown
  //
  IsKnown::IsKnown()
    : OperatorImpl<Boolean>("IsKnown")
  {
  }

  IsKnown::~IsKnown()
  {
  }

  bool IsKnown::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool IsKnown::operator()(bool &result, Expression const *arg) const
  {
    result = arg->isKnown();
    return true; // result of active IsKnown is always known
  }

  //
  // Equal
  //

  template <typename T>
  Equal<T>::Equal()
    : OperatorImpl<Boolean>("EQ")
  {
  }

  template <typename T>
  Equal<ArrayImpl<T> >::Equal()
    : OperatorImpl<Boolean>("EQ")
  {
  }

  template <typename T>
  Equal<T>::~Equal()
  {
  }

  template <typename T>
  Equal<ArrayImpl<T> >::~Equal()
  {
  }

  template <typename T>
  bool Equal<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool Equal<ArrayImpl<T> >::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool Equal<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool Equal<ArrayImpl<T> >::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::arrayValue);
  }

  template <>
  bool Equal<uint16_t>::checkArgTypes(Function const *ev) const
  {
    ValueType vta = (*ev)[0]->valueType();
    if (!isInternalType(vta) && vta != UNKNOWN_TYPE)
      return false;
    ValueType vtb = (*ev)[1]->valueType();
    if (vta != vtb && vtb != UNKNOWN_TYPE)
      return false;
    return true;
  }

  template <typename T>
  bool Equal<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA == tempB);
    return true;
  }

  // a.k.a. EQInternal
  template <>
  bool Equal<uint16_t>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    if (argA->valueType() != argB->valueType())
      return false; // type mismatch
    uint16_t tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown -> result unknown
    result = (tempA == tempB);
    return true;
  }

  // Don't allocate temporary strings
  template <>
  bool Equal<String>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    String const *tempA, *tempB;
    if (!argA->getValuePointer(tempA) || !argB->getValuePointer(tempB))
      return false; // some value unknown
    result = (*tempA == *tempB);
    return true;
  }

  // Don't allocate temporary arrays
  template <typename T>
  bool Equal<ArrayImpl<T> >::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    ArrayImpl<T> const *tempA, *tempB;
    if (!argA->getValuePointer(tempA) || !argB->getValuePointer(tempB))
      return false; // some value unknown
    result = (*tempA == *tempB);
    return true;
  }

  //
  // NotEqual
  //

  template <typename T>
  NotEqual<T>::NotEqual()
    : OperatorImpl<Boolean>("NEQ")
  {
  }

  template <typename T>
  NotEqual<ArrayImpl<T> >::NotEqual()
    : OperatorImpl<Boolean>("NEQ")
  {
  }

  template <typename T>
  NotEqual<T>::~NotEqual()
  {
  }

  template <typename T>
  NotEqual<ArrayImpl<T> >::~NotEqual()
  {
  }

  template <typename T>
  bool NotEqual<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool NotEqual<ArrayImpl<T> >::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool NotEqual<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool NotEqual<ArrayImpl<T> >::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::arrayValue);
  }

  template <>
  bool NotEqual<uint16_t>::checkArgTypes(Function const *ev) const
  {
    ValueType vta = (*ev)[0]->valueType();
    if (!isInternalType(vta) && vta != UNKNOWN_TYPE)
      return false;
    ValueType vtb = (*ev)[1]->valueType();
    if (vta != vtb && vtb != UNKNOWN_TYPE)
      return false;
    return true;
  }

  template <typename T>
  bool NotEqual<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA != tempB);
    return true;
  }

  // a.k.a. NEInternal
  template <>
  bool NotEqual<uint16_t>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    if (argA->valueType() != argB->valueType())
      return true; // type mismatch
    uint16_t tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown -> result unknown
    result = (tempA != tempB);
    return true;
  }

  template <>
  bool NotEqual<String>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    String const *tempA, *tempB;
    if (!argA->getValuePointer(tempA) || !argB->getValuePointer(tempB))
      return false; // some value unknown
    result = (*tempA != *tempB);
    return true;
  }

  template <typename T>
  bool NotEqual<ArrayImpl<T> >::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    ArrayImpl<T> const *tempA, *tempB;
    if (!argA->getValuePointer(tempA) || !argB->getValuePointer(tempB))
      return false; // some value unknown
    result = (*tempA != *tempB);
    return true;
  }

  //
  // GreaterThan
  //

  template <typename T>
  GreaterThan<T>::GreaterThan()
    : OperatorImpl<Boolean>("GT")
  {
  }

  template <typename T>
  GreaterThan<T>::~GreaterThan()
  {
  }

  template <typename T>
  bool GreaterThan<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool GreaterThan<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool GreaterThan<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA > tempB);
    return true;
  }

  //
  // GreaterEqual
  //

  template <typename T>
  GreaterEqual<T>::GreaterEqual()
    : OperatorImpl<Boolean>("GEQ")
  {
  }

  template <typename T>
  GreaterEqual<T>::~GreaterEqual()
  {
  }

  template <typename T>
  bool GreaterEqual<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool GreaterEqual<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool GreaterEqual<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA >= tempB);
    return true;
  }

  //
  // LessThan
  //

  template <typename T>
  LessThan<T>::LessThan()
    : OperatorImpl<Boolean>("LT")
  {
  }

  template <typename T>
  LessThan<T>::~LessThan()
  {
  }

  template <typename T>
  bool LessThan<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool LessThan<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool LessThan<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA < tempB);
    return true;
  }

  //
  // LessEqual
  //

  template <typename T>
  LessEqual<T>::LessEqual()
    : OperatorImpl<Boolean>("LEQ")
  {
  }

  template <typename T>
  LessEqual<T>::~LessEqual()
  {
  }

  template <typename T>
  bool LessEqual<T>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  template <typename T>
  bool LessEqual<T>::checkArgTypes(Function const *ev) const
  {
    return ev->allSameTypeOrUnknown(PlexilValueType<T>::value);
  }

  template <typename T>
  bool LessEqual<T>::operator()(bool &result, Expression const *argA, Expression const *argB) const
  {
    T tempA, tempB;
    if (!argA->getValue(tempA) || !argB->getValue(tempB))
      return false; // some value unknown
    result = (tempA <= tempB);
    return true;
  }

  //
  // Explicit instantiations of template classes
  //

  template class Equal<Boolean>;
  template class Equal<uint16_t>;
  template class Equal<Integer>;
  template class Equal<Real>;
  template class Equal<String>;
  template class Equal<BooleanArray>;
  template class Equal<IntegerArray>;
  template class Equal<RealArray>;
  template class Equal<StringArray>;

  template class NotEqual<Boolean>;
  template class NotEqual<uint16_t>;
  template class NotEqual<Integer>;
  template class NotEqual<Real>;
  template class NotEqual<String>;
  template class NotEqual<BooleanArray>;
  template class NotEqual<IntegerArray>;
  template class NotEqual<RealArray>;
  template class NotEqual<StringArray>;

  // Comparisons below don't make sense for Booleans

  template class GreaterThan<Integer>;
  template class GreaterThan<Real>;
  template class GreaterThan<String>;

  template class GreaterEqual<Integer>;
  template class GreaterEqual<Real>;
  template class GreaterEqual<String>;

  template class LessThan<Integer>;
  template class LessThan<Real>;
  template class LessThan<String>;

  template class LessEqual<Integer>;
  template class LessEqual<Real>;
  template class LessEqual<String>;

}
