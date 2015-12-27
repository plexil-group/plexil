/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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
  template <typename T>
  AssignableImpl<T>::~AssignableImpl()
  {
  }

  AssignableImpl<std::string>::~AssignableImpl()
  {
  }

  template <typename T>
  AssignableImpl<ArrayImpl<T> >::~AssignableImpl()
  {
  }

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
  void AssignableImpl<std::string>::setValueImpl(Expression const *valex)
  {
    std::string const *valptr;
    if (valex->getValuePointer(valptr))
      this->setValueImpl(*valptr);
    else
      this->setUnknown();
  }

  void AssignableImpl<std::string>::setValueImpl(Value const &val)
  {
    std::string const *valptr;
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
  void AssignableImpl<std::string>::setValueImpl(U const &val)
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
  void AssignableImpl<std::string>::setValueImpl(char const *val)
  {
    std::string const temp(val);
    this->setValueImpl(temp);
  }

  template <>
  template <>
  void AssignableImpl<double>::setValueImpl(int32_t const &val)
  {
    this->setValueImpl((double) val);
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
  bool AssignableImpl<std::string>::getMutableValuePointerImpl(U *& ptr)
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
  template class AssignableImpl<bool>;
  template void AssignableImpl<bool>::setValueImpl(uint16_t const &);
  template void AssignableImpl<bool>::setValueImpl(int32_t const &);
  template void AssignableImpl<bool>::setValueImpl(double const &);
  template void AssignableImpl<bool>::setValueImpl(std::string const &);
  template void AssignableImpl<bool>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<bool>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<bool>::setValueImpl(RealArray const &);
  template void AssignableImpl<bool>::setValueImpl(StringArray const &);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<bool>::getMutableValuePointerImpl(StringArray *&);
  
  template class AssignableImpl<int32_t>;
  template void AssignableImpl<int32_t>::setValueImpl(bool const &);
  template void AssignableImpl<int32_t>::setValueImpl(uint16_t const &);
  template void AssignableImpl<int32_t>::setValueImpl(double const &);
  template void AssignableImpl<int32_t>::setValueImpl(std::string const &);
  template void AssignableImpl<int32_t>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<int32_t>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<int32_t>::setValueImpl(RealArray const &);
  template void AssignableImpl<int32_t>::setValueImpl(StringArray const &);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<int32_t>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<double>;
  template void AssignableImpl<double>::setValueImpl(bool const &);
  template void AssignableImpl<double>::setValueImpl(uint16_t const &);
  template void AssignableImpl<double>::setValueImpl(int32_t const &);
  template void AssignableImpl<double>::setValueImpl(std::string const &);
  template void AssignableImpl<double>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<double>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<double>::setValueImpl(RealArray const &);
  template void AssignableImpl<double>::setValueImpl(StringArray const &);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<double>::getMutableValuePointerImpl(StringArray *&);

  //template class AssignableImpl<std::string>;
  template void AssignableImpl<std::string>::setValueImpl(bool const &);
  template void AssignableImpl<std::string>::setValueImpl(uint16_t const &);
  template void AssignableImpl<std::string>::setValueImpl(int32_t const &);
  template void AssignableImpl<std::string>::setValueImpl(double const &);
  template void AssignableImpl<std::string>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<std::string>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<std::string>::setValueImpl(RealArray const &);
  template void AssignableImpl<std::string>::setValueImpl(StringArray const &);
  template bool AssignableImpl<std::string>::getMutableValuePointerImpl(Array *&);
  template bool AssignableImpl<std::string>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<std::string>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<std::string>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<std::string>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<BooleanArray>;
  template void AssignableImpl<BooleanArray>::setValueImpl(bool const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(uint16_t const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(int32_t const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(double const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(std::string const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(RealArray const &);
  template void AssignableImpl<BooleanArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<BooleanArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<IntegerArray>;
  template void AssignableImpl<IntegerArray>::setValueImpl(bool const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(uint16_t const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(int32_t const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(double const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(std::string const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(RealArray const &);
  template void AssignableImpl<IntegerArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(RealArray *&);
  template bool AssignableImpl<IntegerArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<RealArray>;
  template void AssignableImpl<RealArray>::setValueImpl(bool const &);
  template void AssignableImpl<RealArray>::setValueImpl(uint16_t const &);
  template void AssignableImpl<RealArray>::setValueImpl(int32_t const &);
  template void AssignableImpl<RealArray>::setValueImpl(double const &);
  template void AssignableImpl<RealArray>::setValueImpl(std::string const &);
  template void AssignableImpl<RealArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<RealArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<RealArray>::setValueImpl(StringArray const &);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<RealArray>::getMutableValuePointerImpl(StringArray *&);

  template class AssignableImpl<StringArray>;
  template void AssignableImpl<StringArray>::setValueImpl(bool const &);
  template void AssignableImpl<StringArray>::setValueImpl(uint16_t const &);
  template void AssignableImpl<StringArray>::setValueImpl(int32_t const &);
  template void AssignableImpl<StringArray>::setValueImpl(double const &);
  template void AssignableImpl<StringArray>::setValueImpl(std::string const &);
  template void AssignableImpl<StringArray>::setValueImpl(BooleanArray const &);
  template void AssignableImpl<StringArray>::setValueImpl(IntegerArray const &);
  template void AssignableImpl<StringArray>::setValueImpl(RealArray const &);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(std::string *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(BooleanArray *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(IntegerArray *&);
  template bool AssignableImpl<StringArray>::getMutableValuePointerImpl(RealArray *&);

} // namespace PLEXIL
