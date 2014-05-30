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

#include "AssignableImpl.hh"
#include "Error.hh"
#include "Value.hh"

namespace PLEXIL
{
  template <typename T>
  AssignableImpl<T>::AssignableImpl()
    : AssignableShim<AssignableImpl<T> >()
  {
  }

  template <typename T>
  AssignableImpl<T>::~AssignableImpl()
  {
  }

  //
  // setValue wrappers
  //

  // Default
  template <typename T>
  void AssignableImpl<T>::setValueImpl(ExpressionId const &valex)
  {
    T const *valptr;
    if (valex->getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  template <typename T>
  void AssignableImpl<T>::setValueImpl(Value const &val)
  {
    T const *valptr;
    if (val.getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  // Immediate types
  template <>
  void AssignableImpl<bool>::setValueImpl(ExpressionId const &valex)
  {
    bool temp;
    if (valex->getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  template <>
  void AssignableImpl<int32_t>::setValueImpl(ExpressionId const &valex)
  {
    int32_t temp;
    if (valex->getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  template <>
  void AssignableImpl<double>::setValueImpl(ExpressionId const &valex)
  {
    double temp;
    if (valex->getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  template <>
  void AssignableImpl<bool>::setValueImpl(Value const &val)
  {
    bool temp;
    if (val.getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  template <>
  void AssignableImpl<int32_t>::setValueImpl(Value const &val)
  {
    int32_t temp;
    if (val.getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  template <>
  void AssignableImpl<double>::setValueImpl(Value const &val)
  {
    double temp;
    if (val.getValue(temp))
      this->setValueImpl(temp);
    else
      this->setUnknown();
  }

  //
  // setValueImpl
  //

  // Type errors
  template <typename T>
  template <typename U>
  void AssignableImpl<T>::setValueImpl(U const & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  template <typename T>
  void AssignableImpl<T>::setValueImpl(char const * /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::setValue: type error");
  }

  // Conversions
  template <>
  void AssignableImpl<std::string>::setValueImpl(char const *val)
  {
    std::string temp(val);
    this->setValueImpl(temp);
  }

  template <>
  template <>
  void AssignableImpl<double>::setValueImpl(int32_t const &val)
  {
    this->setValueImpl((double) val);
  }

  //
  // getMutableValuePointerImpl
  //

  // Type errors
  template <typename T>
  template <typename U>
  bool AssignableImpl<T>::getMutableValuePointerImpl(U *& /* ptr */)
  {
    assertTrue_2(ALWAYS_FAIL, "Assignable::getMutableValuePointer: type error");
    return false;
  }

  //
  // Explicit instantiations
  //
  template class AssignableImpl<bool>;
  template class AssignableImpl<int32_t>;
  template class AssignableImpl<double>;
  template class AssignableImpl<std::string>;

  template class AssignableImpl<BooleanArray>;
  template class AssignableImpl<IntegerArray>;
  template class AssignableImpl<RealArray>;
  template class AssignableImpl<StringArray>;

} // namespace PLEXIL
