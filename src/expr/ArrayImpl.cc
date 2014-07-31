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
#include "Value.hh"
#include "ValueType.hh"

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
  ArrayImpl<T>::ArrayImpl(size_t size, T const &initval)
  : ArrayAdapter<ArrayImpl<T> >(size, true),
    m_contents(size, initval)
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
  Array *ArrayImpl<T>::clone() const
  {
    return new ArrayImpl<T>(*this);
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

  template <>
  ValueType ArrayImpl<bool>::getElementType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  ValueType ArrayImpl<int32_t>::getElementType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  ValueType ArrayImpl<double>::getElementType() const
  {
    return REAL_TYPE;
  }

  template <>
  ValueType ArrayImpl<std::string>::getElementType() const
  {
    return STRING_TYPE;
  }

  template <typename T>
  Value ArrayImpl<T>::getElementValue(size_t index) const
  {
    if (!(this->checkIndex(index) && this->m_known[index]))
      return Value(); // unknown
    else
      return Value(m_contents[index]);
  }

  template <typename T>
  bool ArrayImpl<T>::getElementImpl(size_t index, T &result) const
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
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
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
      return false;
    result = (double) m_contents[index];
    return true;
  }

  // Generic equality
  template <typename T>
  bool ArrayImpl<T>::operator==(Array const &other) const
  {
    ArrayImpl<T> const *typedOther = 
      dynamic_cast<ArrayImpl<T> const *>(&other);
    if (!typedOther)
      return false;
    return operator==(*typedOther);
  }

  // Specific equality
  template <typename T>
  bool ArrayImpl<T>::operator==(ArrayImpl<T> const &other) const
  {
    if (!(this->getKnownVector() == other.getKnownVector()))
      return false;
    return m_contents == other.m_contents;
  }

  template <typename T>
  bool ArrayImpl<T>::getElementPointerImpl(size_t index, T const *&result) const
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
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

  template <typename T>
  bool ArrayImpl<T>::getMutableElementPointer(size_t index, std::string *&result)
  {
    assertTrue_2(ALWAYS_FAIL, "Array:getMutableElementPointer: type error");
    return false;
  }

  template <>
  bool ArrayImpl<std::string>::getMutableElementPointer(size_t index, std::string *&result)
  {
    if (!this->checkIndex(index))
      return false;
    if (!this->m_known[index])
      return false;
    result = &m_contents[index];
    return true;
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
    if (!this->checkIndex(index))
      return;
    m_contents[index] = newval;
    this->m_known[index] = true;
  }

  template <typename T>
  void ArrayImpl<T>::setElementValue(size_t index, Value const &value)
  {
    if (!this->checkIndex(index))
      return;
    T temp;
    bool known = value.getValue(temp);
    if (known)
      m_contents[index] = temp;
    this->m_known[index] = known;
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
    if (!this->checkIndex(index))
      return;
    m_contents[index] = (double) newval;
    this->m_known[index] = true;
  }

  template <typename T>
  void ArrayImpl<T>::print(std::ostream &s) const
  {
    s << "#(";

    size_t len = this->size();
    size_t i = 0;

    while (i < len) {
      T temp;
      if (getElementImpl(i, temp))
        printValue<T>(temp, s);
      else
        s << "UNKNOWN";
      if (++i < len)
        s << ' ';
    }
    // Print tail
    s << ')';
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

  // Generic
  template <typename T>
  bool operator!=(ArrayImpl<T> const &a, Array const &b)
  {
    return !(a == b);
  }

  // Specific
  template <typename T>
  bool operator!=(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    return !(a == b);
  }

  template <typename T>
  bool operator<(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    std::vector<bool> const &aKnownVec = a.getKnownVector();
    std::vector<bool> const &bKnownVec = b.getKnownVector();
    // Shorter is less
    size_t aSize = aKnownVec.size();
    size_t bSize = bKnownVec.size();
    if (aSize < bSize)
      return true;
    if (aSize > bSize)
      return false;

    // Same size
    std::vector<T> const *aVec, *bVec;
    a.getContentsVectorImpl(aVec);
    b.getContentsVectorImpl(bVec);

    for (size_t i = 0; i < aSize; ++i) {
      // Unknown is less than known
      bool aKnown = aKnownVec[i];
      bool bKnown = bKnownVec[i];
      if (!aKnown && bKnown)
        return true; // unknown < known
      if (aKnown && !bKnown)
        return false;
      if (!aKnown)
        continue; // neither known

      // Compare values
      if ((*aVec)[i] < (*bVec)[i])
        return true;
      if ((*aVec)[i] > (*bVec)[i])
        return false;
    }
    return false; // equal
  }

  // Specialization for bool
  template <>
  bool operator< <bool>(ArrayImpl<bool> const &a, ArrayImpl<bool> const &b)
  {
    std::vector<bool> const &aKnownVec = a.getKnownVector();
    std::vector<bool> const &bKnownVec = b.getKnownVector();
    // Shorter is less
    size_t aSize = aKnownVec.size();
    size_t bSize = bKnownVec.size();
    if (aSize < bSize)
      return true;
    if (aSize > bSize)
      return false;

    // Same size
    std::vector<bool> const *aVec, *bVec;
    a.getContentsVectorImpl(aVec);
    b.getContentsVectorImpl(bVec);

    for (size_t i = 0; i < aSize; ++i) {
      // Unknown is less than known
      bool aKnown = aKnownVec[i];
      bool bKnown = bKnownVec[i];
      if (!aKnown && bKnown)
        return true; // unknown < known
      if (aKnown && !bKnown)
        return false;
      if (!aKnown)
        continue; // neither known

      // False less than true
      if (!(*aVec)[i] && (*bVec)[i])
        return true;
      if ((*aVec)[i] && !(*bVec)[i])
        return false;
    }
    return false; // equal
  }

  template <typename T>
  bool operator<=(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    return !(b < a);
  }


  template <typename T>
  bool operator>(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    return b < a;
  }

  template <typename T>
  bool operator>=(ArrayImpl<T> const &a, ArrayImpl<T> const &b)
  {
    return !(a < b);
  }


  template <typename T>
  std::ostream &operator<<(std::ostream &s, ArrayImpl<T> const &ary)
  {
    ary.print(s);
    return s;
  }


  //
  // Explicit instantiations
  //
  template class ArrayImpl<bool>;
  template class ArrayImpl<int32_t>;
  template class ArrayImpl<double>;
  template class ArrayImpl<std::string>;

  template bool operator!=(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator!=(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator!=(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator!=(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  // Explicitly defined above
  // template bool operator<(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator<(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator<(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator<(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  template bool operator<=(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator<=(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator<=(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator<=(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  template bool operator>(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator>(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator>(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator>(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  template bool operator>=(ArrayImpl<bool> const &,        ArrayImpl<bool> const &);
  template bool operator>=(ArrayImpl<int32_t> const &,     ArrayImpl<int32_t> const &);
  template bool operator>=(ArrayImpl<double> const &,      ArrayImpl<double> const &);
  template bool operator>=(ArrayImpl<std::string> const &, ArrayImpl<std::string> const &);

  template std::ostream &operator<<(std::ostream &s, ArrayImpl<bool> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<int32_t> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<double> const &);
  template std::ostream &operator<<(std::ostream &s, ArrayImpl<std::string> const &);

} // namespace PLEXIL
