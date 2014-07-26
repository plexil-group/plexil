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
//#include <string>  // included by Error.hh
//#include <sstream> // included by Error.hh

namespace PLEXIL
{
  Array::Array()
  {
  }

  Array::Array(Array const &orig)
    : m_known(orig.m_known)
  {
  }

  Array::Array(size_t size, bool known)
  : m_known(size, known)
  {
  }

  Array::~Array()
  {
  }

  Array &Array::operator=(Array const &other)
  {
    m_known = other.m_known;
    return *this;
  }

  size_t Array::size() const
  {
    return m_known.size();
  }

  bool Array::elementKnown(size_t index) const
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "Array::elementKnown: Index exceeds array size");
      return false;
    }
    return m_known[index];
  }

  void Array::resize(size_t size)
  {
    if (!checkIndex(size)) {
      m_known.resize(size, false);
    }
  }

  void Array::setElementUnknown(size_t index)
  {
    if (!checkIndex(index)) {
      check_error_2(ALWAYS_FAIL, "Array::setElementUnknown: Index exceeds array size");
      return;
    }
    m_known[index] = false;
  }

  bool Array::operator==(Array const &other) const
  {
    return m_known == other.m_known;
  }

  //
  // TODO:
  // - Define boundary case size == 0 for any/allElementsKnown
  //

  bool Array::allElementsKnown() const
  {
    for (size_t i = 0; i < m_known.size(); ++i)
      if (!m_known[i])
        return false;
    return true;
  }

  bool Array::anyElementsKnown() const
  {
    for (size_t i = 0; i < m_known.size(); ++i)
      if (m_known[i])
        return true;
    return false;
  }

  std::string Array::toString() const
  {
    std::ostringstream s;
    this->print(s);
    return s.str();
  }

  std::ostream &operator<<(std::ostream &s, Array const &a)
  {
    a.print(s);
    return s;
  }

} // namespace PLEXIL
