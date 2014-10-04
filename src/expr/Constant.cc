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

//
// Member function definitions and explicit instantiation for Constant classes
//

#include "Constant.hh"

#include "ArrayImpl.hh"

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  template <typename T>
  Constant<T>::Constant()
    : Expression(),
      ExpressionImpl<T>(),
      m_known(false)
  {
  }

  Constant<std::string>::Constant()
    : Expression(),
      ExpressionImpl<std::string>(),
      m_known(false)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant()
    : Expression(),
      ExpressionImpl<ArrayImpl<T> >(),
      m_known(false)
  {
  }

  /**
   * @brief Copy constructor.
   */
  template <typename T>
  Constant<T>::Constant(const Constant &other)
  : Expression(),
    ExpressionImpl<T>(),
    m_value(other.m_value),
    m_known(other.m_known)
  {
  }

  Constant<std::string>::Constant(const Constant &other)
  : Expression(),
    ExpressionImpl<std::string>(),
    m_value(other.m_value),
    m_known(other.m_known)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant(const Constant &other)
    : Expression(),
      ExpressionImpl<ArrayImpl<T> >(),
    m_value(other.m_value),
    m_known(other.m_known)
  {
  }

  /**
   * @brief Constructor from value type.
   */
  template <typename T>
  Constant<T>::Constant(const T &value)
  : Expression(),
    ExpressionImpl<T>(),
    m_value(value),
    m_known(true)
  {
  }

  Constant<std::string>::Constant(const std::string &value)
  : Expression(),
    ExpressionImpl<std::string>(),
    m_value(value),
    m_known(true)
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::Constant(const ArrayImpl<T> &value)
    : Expression(),
      ExpressionImpl<ArrayImpl<T> >(),
    m_value(value),
    m_known(true)
  {
  }

  /**
   * @brief Constructors from char *.
   */

  // *** TODO: More types ***
  Constant<std::string>::Constant(const char *value)
  : Expression(),
    ExpressionImpl<std::string>(),
    m_value(value),
    m_known(true)
  {
  }

  /**
   * @brief Destructor.
   */
  template <typename T>
  Constant<T>::~Constant()
  {
  }

  Constant<std::string>::~Constant()
  {
  }

  template <typename T>
  Constant<ArrayImpl<T> >::~Constant()
  {
  }

  /**
   * @brief Return a print name for the expression type.
   * @return A constant character string.
   */
  template <typename T>
  const char *Constant<T>::exprName() const
  {
    return "Constant";
  }

  const char *Constant<std::string>::exprName() const
  {
    return "Constant";
  }

  template <typename T>
  const char *Constant<ArrayImpl<T> >::exprName() const
  {
    return "Constant";
  }

  /**
   * @brief Retrieve the value of this Expression in its native type.
   * @param The appropriately typed place to put the result.
   * @return True if known, false if unknown.
   */
  template <typename T>
  bool Constant<T>::getValueImpl(T& result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  bool Constant<std::string>::getValueImpl(std::string& result) const
  {
    if (m_known)
      result = m_value;
    return m_known;
  }

  /**
   * @brief Retrieve a pointer to the (const) value of this Expression.
   * @param ptr Reference to the pointer variable to receive the result.
   * @return True if known, false if unknown.
   */
  bool Constant<std::string>::getValuePointerImpl(std::string const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::getValuePointerImpl(ArrayImpl<T> const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  /**
   * @brief Query whether the expression's value is known.
   * @return True if known, false otherwise.
   */
  template <typename T>
  bool Constant<T>::isKnown() const
  {
    return m_known;
  }

  bool Constant<std::string>::isKnown() const
  {
    return m_known;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::isKnown() const
  {
    return m_known;
  }

  /**
   * @brief Query whether this expression is constant, i.e. incapable of change.
   * @return True if assignable, false otherwise.
   */
  template <typename T>
  bool Constant<T>::isConstant() const
  {
    return true;
  }

  bool Constant<std::string>::isConstant() const
  {
    return true;
  }

  template <typename T>
  bool Constant<ArrayImpl<T> >::isConstant() const
  {
    return true;
  }

  /**
   * @brief Is this expression active (i.e. propagating value changes?)
   * @return true if this Expression is active, false if it is not.
   */
  template <typename T>
  bool Constant<T>::isActive() const
  {
    return true; // constants are always active
  }
 
  bool Constant<std::string>::isActive() const
  {
    return true; // constants are always active
  }
 
  template <typename T>
  bool Constant<ArrayImpl<T> >::isActive() const
  {
    return true; // constants are always active
  }

  /**
   * @brief Make this expression active.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::activate()
  {
  }

  void Constant<std::string>::activate()
  {
  }

  template <typename T>
  void Constant<ArrayImpl<T> >::activate()
  {
  }

  /**
   * @brief Make this expression inactive.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::deactivate()
  {
  }

  void Constant<std::string>::deactivate()
  {
  }

  template <typename T>
  void Constant<ArrayImpl<T> >::deactivate()
  {
  }

  /**
   * @brief Add a listener for changes to this Expression's value.
   * @param ptr Pointer to the listener to notify.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::addListener(ExpressionListener * /* ptr */)
  {
  }

  void Constant<std::string>::addListener(ExpressionListener * /* ptr */)
  {
  }

  template <typename T>
  void Constant<ArrayImpl<T> >::addListener(ExpressionListener * /* ptr */)
  {
  }

  template <typename T>
  void Constant<T>::removeListener(ExpressionListener * /* ptr */)
  {
  }

  void Constant<std::string>::removeListener(ExpressionListener * /* ptr */)
  {
  }

  template <typename T>
  void Constant<ArrayImpl<T> >::removeListener(ExpressionListener * /* ptr */)
  {
  }

  /**
   * @brief Notify this expression that a subexpression's value has changed.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::notifyChanged(Expression const * /* src */)
  {
  }

  void Constant<std::string>::notifyChanged(Expression const * /* src */)
  {
  }

  template <typename T>
  void Constant<ArrayImpl<T> >::notifyChanged(Expression const * /* src */)
  {
  }

  //
  // Explicit instantiations
  //
  template class Constant<bool>;
  template class Constant<uint16_t>; // for internal node constants
  template class Constant<int32_t>;
  template class Constant<double>;
  // template class Constant<std::string>; redundant

  template class Constant<BooleanArray>;
  template class Constant<IntegerArray>;
  template class Constant<RealArray>;
  template class Constant<StringArray>;

} // namespace PLEXIL
