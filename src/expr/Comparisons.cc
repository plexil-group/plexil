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

#include "Comparisons.hh"
#include "Expression.hh"

namespace PLEXIL
{

  //
  // IsKnown
  //
  IsKnown::IsKnown()
    : OperatorImpl<bool>("IsKnown")
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
    : OperatorImpl<bool>("EQ")
  {
  }

  template <typename T>
  Equal<T>::~Equal()
  {
  }

  template <typename T>
  bool Equal<T>::checkArgCount(size_t count) const
  {
    return count == 2;
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

  //
  // NotEqual
  //

  template <typename T>
  NotEqual<T>::NotEqual()
    : OperatorImpl<bool>("NEQ")
  {
  }

  template <typename T>
  NotEqual<T>::~NotEqual()
  {
  }

  template <typename T>
  bool NotEqual<T>::checkArgCount(size_t count) const
  {
    return count == 2;
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

  //
  // GreaterThan
  //

  template <typename T>
  GreaterThan<T>::GreaterThan()
    : OperatorImpl<bool>("GT")
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
    : OperatorImpl<bool>("GEQ")
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
    : OperatorImpl<bool>("LT")
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
    : OperatorImpl<bool>("LEQ")
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

  template class Equal<bool>;
  template class Equal<uint16_t>;
  template class Equal<int32_t>;
  template class Equal<double>;
  template class Equal<std::string>;

  template class NotEqual<bool>;
  template class NotEqual<uint16_t>;
  template class NotEqual<int32_t>;
  template class NotEqual<double>;
  template class NotEqual<std::string>;

  // Comparisons below don't make sense for Booleans

  template class GreaterThan<int32_t>;
  template class GreaterThan<double>;
  template class GreaterThan<std::string>;

  template class GreaterEqual<int32_t>;
  template class GreaterEqual<double>;
  template class GreaterEqual<std::string>;

  template class LessThan<int32_t>;
  template class LessThan<double>;
  template class LessThan<std::string>;

  template class LessEqual<int32_t>;
  template class LessEqual<double>;
  template class LessEqual<std::string>;

}
