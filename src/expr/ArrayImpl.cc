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

#include "ArrayImpl.hh"
#include "Error.hh"

namespace PLEXIL
{

  template <typename T>
  ArrayImpl<T>::ArrayImpl()
    : ArrayAdapter<ArrayImpl<T> >()
  {
  }

  template <typename T>
  ArrayImpl<T>::ArrayImpl(ArrayImpl<T> const &orig)
    : ArrayAdapter<ArrayImpl<T> >(orig),
      m_contents(orig.m_contents)
  {
  }

  template <typename T>
  ArrayImpl<T>::ArrayImpl(size_t size)
  : ArrayAdapter<ArrayImpl<T> >(size, false),
    m_contents(size)
  {
  }

  template <typename T>
  ArrayImpl<T>::ArrayImpl(std::vector<T> const &initval)
    : ArrayAdapter<ArrayImpl<T> >(initval.size(), true),
      m_contents(initval)
  {
  }

  template <typename T>
  ArrayImpl<T>::~ArrayImpl()
  {
  }

  template <typename T>
  ArrayImpl<T> &ArrayImpl<T>::operator=(ArrayImpl<T> const &orig)
  {
    Array::operator=(orig);
    m_contents = orig.m_contents;
    return *this;
  }

  template <typename T>
  void ArrayImpl<T>::resize(size_t size)
  {
    Array::resize(size);
    m_contents.resize(size);
  }

  template <typename T>
  bool ArrayImpl<T>::getElementImpl(size_t index, T &result) const
  {
    if (!Array::checkIndex(index))
      return false;
    if (!Array::m_known[index])
      return false;
    result = m_contents[index];
    return true;
  }

  // Default
  template <typename T>
  template <typename U>
  bool ArrayImpl<T>::getElementImpl(size_t index, U & /* result */) const
  {
    assertTrue_2(ALWAYS_FAIL, "Array::getElement: type error");
    return false;
  }

  // Conversion
  template <>
  template <>
  bool ArrayImpl<int32_t>::getElementImpl(size_t index, double &result) const
  {
    if (!Array::checkIndex(index))
      return false;
    if (!Array::m_known[index])
      return false;
    result = (double) m_contents[index];
    return true;
  }

  template <typename T>
  bool ArrayImpl<T>::getElementPointerImpl(size_t index, T const *&result) const
  {
    if (!Array::checkIndex(index))
      return false;
    if (!Array::m_known[index])
      return false;
    result = &m_contents[index];
    return true;
  }

  // Limitation of std::vector<bool>
  template <>
  bool ArrayImpl<bool>::getElementPointerImpl(size_t index, bool const *&result) const
  {
    assertTrue_2(ALWAYS_FAIL, "Array::getElementPointer not implemented for BooleanArray");
    return false;
  }

  // Default
  template <typename T>
  template <typename U>
  bool ArrayImpl<T>::getElementPointerImpl(size_t index, U const *& /* result */) const
  {
    assertTrue_2(ALWAYS_FAIL, "Array::getElementPointer: type error");
    return false;
  }

  template <typename T>
  void ArrayImpl<T>::getContentsVectorImpl(std::vector<T> const *&result) const
  {
    result = &m_contents;
  }

  // Default
  template <typename T>
  template <typename U>
  void ArrayImpl<T>::getContentsVectorImpl(std::vector<U> const *&result) const
  {
    assertTrue_2(ALWAYS_FAIL, "Array::getContentsVector: type error");
  }

  template <typename T>
  void ArrayImpl<T>::setElementImpl(size_t index, T const &newval)
  {
    if (!Array::checkIndex(index))
      return;
    m_contents[index] = newval;
    Array::m_known[index] = true;
  }

  // Default
  template <typename T>
  template <typename U>
  void ArrayImpl<T>::setElementImpl(size_t index, U const &newval)
  {
    assertTrue_2(ALWAYS_FAIL, "Array::setElement: type error");
  }

  // Conversion
  template <>
  template <>
  void ArrayImpl<double>::setElementImpl(size_t index, int32_t const &newval)
  {
    if (!Array::checkIndex(index))
      return;
    m_contents[index] = (double) newval;
    Array::m_known[index] = true;
  }

  template <typename T>
  void ArrayImpl<T>::print(std::ostream &s) const
  {
    // *** TODO
    // Print head
    // Print contents
    // Print tail
  }

  template <typename T>
  bool operator==(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    if (!(a.getKnownVector() == b.getKnownVector()))
      return false;
    std::vector<T> const *avec, *bvec;
    a.getContentsVector(avec);
    b.getContentsVector(bvec);
    return *avec == *bvec;
  }

  template <typename T>
  bool operator!=(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    return !(a == b);
  }

  //
  // Explicit instantiations
  //
  template class ArrayImpl<bool>;
  template class ArrayImpl<int32_t>;
  template class ArrayImpl<double>;
  template class ArrayImpl<std::string>;

  template bool operator==(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator==(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator==(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator==(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  template bool operator!=(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator!=(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator!=(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator!=(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

} // namespace PLEXIL
