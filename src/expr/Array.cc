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

#include "Array.hh"
#include "Error.hh"
#include <string>

namespace PLEXIL
{
  template <typename T>
  Array<T>::Array()
  {
  }

  template <typename T>
  Array<T>::Array(Array<T> const &orig)
    : m_contents(orig.m_contents),
      m_known(orig.m_known)
  {
  }

  template <typename T>
  Array<T>::Array(size_t size)
  : m_contents(size),
    m_known(size, false)
  {
  }

  template <typename T>
  Array<T>::Array(std::vector<T> const &initval)
  : m_contents(initval),
    m_known(initval.size(), true)
  {
  }

  template <typename T>
  Array<T>::~Array()
  {
  }

  template <typename T>
  Array<T> &Array<T>::operator=(Array<T> const &other)
  {
    m_contents = other.m_contents;
    m_known = other.m_known;
    return *this;
  }

  template <typename T>
  size_t Array<T>::size() const
  {
    return m_contents.size();
  }

  template <typename T>
  bool Array<T>::elementKnown(size_t index) const
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "elementKnown: Index exceeds array size");
      return false;
    }
    return m_known[index];
  }

  template <typename T>
  bool Array<T>::getElement(size_t index, T &result) const
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "getElement: Index exceeds array size");
      return false;
    }
    if (m_known[index])
      result = m_contents[index];
    return m_known[index];
  }

  template <typename T>
  void Array<T>::resize(size_t size)
  {
    if (!checkIndex(size)) {
      m_contents.resize(size);
      m_known.resize(size, false);
    }
  }

  template <typename T>
  void Array<T>::setElement(size_t index, T const &newVal)
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "setElement: Index exceeds array size");
      return;
    }
    m_contents[index] = newVal;
    m_known[index] = true;
  }

  template <typename T>
  void Array<T>::setElementUnknown(size_t index)
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "setElementUnknown: Index exceeds array size");
      return;
    }
    m_known[index] = false;
  }

  template <typename T>
  bool operator==(Array<T> const &a, Array<T> const &b)
  {
    return a.m_contents == b.m_contents
      && a.m_known == b.m_known;
  }

  template <typename T>
  bool operator!=(Array<T> const &a, Array<T> const &b)
  {
    return !(a == b);
  }

  //
  // Explicit instantiations
  //

  template class Array<bool>;
  template class Array<int32_t>;
  template class Array<double>;
  template class Array<std::string>;

  template bool operator==<bool>(Array<bool> const &a, Array<bool> const &b);
  template bool operator==<int32_t>(Array<int32_t> const &, Array<int32_t> const &);
  template bool operator==<double>(Array<double> const &, Array<double> const &);
  template bool operator==<std::string>(Array<std::string> const &, Array<std::string> const &);

  template bool operator!=<bool>(Array<bool> const &a, Array<bool> const &b);
  template bool operator!=<int32_t>(Array<int32_t> const &, Array<int32_t> const &);
  template bool operator!=<double>(Array<double> const &, Array<double> const &);
  template bool operator!=<std::string>(Array<std::string> const &, Array<std::string> const &);

} // namespace PLEXIL
