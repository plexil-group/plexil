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

#include "Operator.hh"
#include "Error.hh"
#include "Expression.hh"

namespace PLEXIL
{

  template <typename R>
  Operator<R>::Operator()
  {
  }

  template <typename R>
  Operator<R>::Operator(const Operator<R> &other)
  {
    m_name = other.m_name;
  }

  template <typename R>
  Operator<R>::~Operator()
  {
  }

  template <typename R>
  const std::string& Operator<R>::getName() const
  {
    return m_name;
  }

  template <typename R>
  void Operator<R>::setName(std::string const &name)
  {
    m_name = name;
  }

  // Default method for unspecialized types
  template <typename R>
  const ValueType Operator<R>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  // Specific types
  template <>
  const ValueType Operator<double>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  const ValueType Operator<int32_t>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  const ValueType Operator<bool>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  const ValueType Operator<std::string>::valueType() const
  {
    return STRING_TYPE;
  }

  // Error if not overridden by derived classes
  template <typename R>
  bool Operator<R>::operator()(R &result, const ExpressionId &arg) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << m_name << " of one arg not implemented for this return type");
    return false;
  }

  template <typename R>
  bool Operator<R>::operator()(R &result, const ExpressionId &argA, const ExpressionId &argB) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << m_name << " of two args not implemented for this return type");
    return false;
  }

  template <typename R>
  bool Operator<R>::operator()(R &result, const std::vector<ExpressionId> &arg) const
  {
    assertTrueMsg(ALWAYS_FAIL,
                  "Operator " << m_name << " of three or more args not implemented for this return type");
    return false;
  }

  //
  // Explicit instantiations
  //

  template class Operator<double>;
  template class Operator<int32_t>;
  // template class Operator<uint16_t>;
  template class Operator<bool>;
  template class Operator<std::string>;

} // namespace PLEXIL
