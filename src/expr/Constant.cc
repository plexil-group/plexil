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
#include "ConcreteExpressionFactory.hh"

namespace PLEXIL
{

  /**
   * @brief Default constructor.
   */
  template <typename T>
  Constant<T>::Constant()
    : ExpressionImpl<T>(),
      m_known(false)
  {
  }

  /**
   * @brief Copy constructor.
   */
  template <typename T>
  Constant<T>::Constant(const Constant &other)
  : ExpressionImpl<T>(),
    m_value(other.m_value),
    m_known(other.m_known)
  {
  }

  /**
   * @brief Constructor from value type.
   */
  template <typename T>
  Constant<T>::Constant(const T &value)
  : ExpressionImpl<T>(),
    m_value(value),
    m_known(true)
  {
  }

  /**
   * @brief Constructors from char *.
   */

  // *** TODO: More types ***
  template <>
  Constant<std::string>::Constant(const char *value)
  : ExpressionImpl<std::string>(),
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

  /**
   * @brief Return a print name for the expression type.
   * @return A constant character string.
   */
  template <typename T>
  const char *Constant<T>::exprName() const
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

  /**
   * @brief Retrieve a pointer to the (const) value of this Expression.
   * @param ptr Reference to the pointer variable to receive the result.
   * @return True if known, false if unknown.
   */
  template <typename T>
  bool Constant<T>::getValuePointerImpl(T const *&ptr) const
  {
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool Constant<T>::getValuePointerImpl(Array const *&ptr) const
  {
    assertTrue_2(ALWAYS_FAIL, "Constant::getValuePointer type error");
    return false;
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

  /**
   * @brief Query whether this expression is constant, i.e. incapable of change.
   * @return True if assignable, false otherwise.
   */
  template <typename T>
  bool Constant<T>::isConstant() const
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

  /**
   * @brief Make this expression active.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::activate()
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

  /**
   * @brief Add a listener for changes to this Expression's value.
   * @param id The Id of the listener to notify.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::addListener(ExpressionListenerId /* id */)
  {
  }

  /**
   * @brief Remove a listener from this Expression.
   * @param id The Id of the listener to remove.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::removeListener(ExpressionListenerId /* id */)
  {
  }

  /**
   * @brief Notify this expression that a subexpression's value has changed.
   * @note No-op for constants.
   */
  template <typename T>
  void Constant<T>::notifyChanged(ExpressionId /* src */)
  {
  }

  //
  // Concrete factories
  //

  // Scalar cases
  template <>
  ExpressionId ConcreteExpressionFactory<Constant<bool> >::create(const PlexilExprId& expr,
                                                                  const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "ExpressionFactory<Constant>: PlexilExpr is not a PlexilValue");
    bool isKnown = !tmpl->value().empty() || (tmpl->value() != "UNKNOWN");
    Constant<bool> *result = NULL;
    if (isKnown) {
      bool value;
      if (parseValue(tmpl->value(), value))
        return (new Constant<bool>(value))->getId();
    }
    return (new Constant<bool>())->getId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<Constant<int32_t> >::create(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "ExpressionFactory<Constant>: PlexilExpr is not a PlexilValue");
    bool isKnown = !tmpl->value().empty() || (tmpl->value() != "UNKNOWN");
    Constant<int32_t> *result = NULL;
    if (isKnown) {
      int32_t value;
      if (parseValue(tmpl->value(), value))
        return (new Constant<int32_t>(value))->getId();
    }
    return (new Constant<int32_t>())->getId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<Constant<double> >::create(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "ExpressionFactory<Constant>: PlexilExpr is not a PlexilValue");
    bool isKnown = !tmpl->value().empty() || (tmpl->value() != "UNKNOWN");
    Constant<double> *result = NULL;
    if (isKnown) {
      double value;
      if (parseValue(tmpl->value(), value))
        return (new Constant<double>(value))->getId();
    }
    return (new Constant<double>())->getId();
  }

  template <>
  ExpressionId ConcreteExpressionFactory<Constant<std::string> >::create(const PlexilExprId& expr,
                                                                    const NodeConnectorId& node) const
  {
    PlexilValue const *tmpl = (PlexilValue const *) expr;
    assertTrue_2(tmpl, "ExpressionFactory<Constant>: PlexilExpr is not a PlexilValue");
    bool isKnown = !tmpl->value().empty() || (tmpl->value() != "UNKNOWN");
    Constant<std::string> *result = NULL;
    if (isKnown) {
      std::string value;
      if (parseValue(tmpl->value(), value))
        return (new Constant<std::string>(value))->getId();
    }
    return (new Constant<std::string>())->getId();
  }

  //
  // Explicit instantiations
  //
  template class Constant<bool>;
  template class Constant<int32_t>;
  template class Constant<double>;
  template class Constant<std::string>;

  template class Constant<BooleanArray>;
  template class Constant<IntegerArray>;
  template class Constant<RealArray>;
  template class Constant<StringArray>;

  ENSURE_EXPRESSION_FACTORY(BooleanConstant);
  ENSURE_EXPRESSION_FACTORY(IntegerConstant);
  ENSURE_EXPRESSION_FACTORY(RealConstant);
  ENSURE_EXPRESSION_FACTORY(StringConstant);

} // namespace PLEXIL
