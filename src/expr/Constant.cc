/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

//
// Member function definitions and explicit instantiation for Constant classes
//

#include "Constant.hh"

#include "ArrayImpl.hh"

namespace PLEXIL
{

  template <typename T>
  Constant<T>::Constant()
    : GetValueImpl<T>(),
      m_known(false)
  {
  }

  Constant<String>::Constant()
    : GetValueImpl<String>(),
      m_known(false)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant()
    : GetValueImpl<ArrayImpl<T> >(),
      m_known(false)
  {
  }

  template <typename T>
  Constant<T>::Constant(const Constant &other)
    : GetValueImpl<T>(),
      m_value(other.m_value),
      m_known(other.m_known)
  {
  }

  Constant<String>::Constant(const Constant &other)
    : GetValueImpl<String>(),
      m_value(other.m_value),
      m_known(other.m_known)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant(const Constant &other)
    : GetValueImpl<ArrayImpl<T> >(),
      m_value(other.m_value),
      m_known(other.m_known)
  {
  }

  template <typename T>
  Constant<T>::Constant(const T &value)
    : GetValueImpl<T>(),
      m_value(value),
      m_known(true)
  {
  }

  Constant<String>::Constant(const String &value)
    : GetValueImpl<String>(),
      m_value(value),
      m_known(true)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant(const ArrayImpl<T> &value)
    : GetValueImpl<ArrayImpl<T> >(),
      m_value(value),
      m_known(true)
  {
  }

  //
  // Constructors from char *.
  //

  // *** TODO: More types ***
  Constant<String>::Constant(const char *value)
    : GetValueImpl<String>(),
      m_value(value),
      m_known(true)
  {
  }

  template <typename T>
  const char *Constant<T>::exprName() const
  {
    return "Constant";
  }

  const char *Constant<String>::exprName() const
  {
    return "Constant";
  }

  template <typename T>
  const char *Constant<ArrayImpl<T> >::exprName() const
  {
    return "Constant";
  }

  template <typename T>
  bool Constant<T>::getValue(T &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  bool Constant<String>::getValue(String &result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  bool Constant<String>::getValuePointer(String const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::getValuePointer(ArrayImpl<T> const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool Constant<T>::isKnown() const
  {
    return m_known;
  }

  bool Constant<String>::isKnown() const
  {
    return m_known;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::isKnown() const
  {
    return m_known;
  }

  template <typename T>
  bool Constant<T>::isConstant() const
  {
    return true;
  }

  bool Constant<String>::isConstant() const
  {
    return true;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::isConstant() const
  {
    return true;
  }

  //
  // Explicit instantiations
  //
  template class Constant<Boolean>;
  template class Constant<Integer>;
  template class Constant<Real>;
  template class Constant<NodeState>;
  template class Constant<NodeOutcome>;
  template class Constant<FailureType>;
  template class Constant<CommandHandleValue>;

  template class Constant<BooleanArray>;
  template class Constant<IntegerArray>;
  template class Constant<RealArray>;
  template class Constant<StringArray>;

} // namespace PLEXIL
