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
// Member function definitions and explicit instantiation for UserVariable classes
//

#include "UserVariable.hh"

#include "Error.hh"
//#include "Node.hh"

namespace PLEXIL {

  /**
   * @brief Default constructor.
   */
  template <typename T>
  UserVariable<T>::UserVariable()
    : m_name("anonymous"),
      m_known(false),
      m_initialKnown(false),
      m_savedKnown(false)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const T &value, 
                                const NodeId &node,
                                const std::string &name)
    : m_node(node),
      m_name(name),
      m_value(value),
      m_initialValue(value),
      m_savedValue(value), // redundant?
      m_known(true),
      m_initialKnown(true),
      m_savedKnown(true)
  {
  }
    
  /**
   * @brief Destructor.
   * @note Specializations may have more work to do.
   */
  template <typename T>
  UserVariable<T>::~UserVariable()
  {
  }

  //
  // Essential Expression API
  //

  template <typename T>
  const char *UserVariable<T>::exprName() const
  {
    return "Variable";
  }

  template <typename T>
  const ValueType UserVariable<T>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  // Specialize above for known types
  template <>
  const ValueType UserVariable<double>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  const ValueType UserVariable<int32_t>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  const ValueType UserVariable<bool>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  const ValueType UserVariable<std::string>::valueType() const
  {
    return STRING_TYPE;
  }

  // TODO: Array types

  template <typename T>
  bool UserVariable<T>::isKnown() const
  {
    return isActive() && m_known;
  }

  // TODO: specialize on type, e.g. for bool, array, quotes around string
  template <typename T>
  void UserVariable<T>::printValue(std::ostream& s) const
  {
    if (m_known)
      s << m_value;
    else
      s << "(UNKNOWN)";
  }

  // Same type
  template <typename T>
  bool UserVariable<T>::getValue(T &result) const
  {
    if (!isActive())
      return false;
    if (m_known)
      result = m_value;
    return m_known;
  }

  // Compatible types
  template <>
  template <>
  bool UserVariable<int32_t>::getValue(double& result) const
  {
    if (!isActive())
      return false;
    if (m_known)
      result = (double) m_value;
    return m_known;
  }

  // Same type
  template <typename T>
  void UserVariable<T>::setValue(const T &value)
  {
    bool changed = !m_known || value != m_value;
    assertTrueMsg(checkValue(value), "Error: " << value << " is an invalid value for this variable");
    m_value = value;
    m_known = true;
    if (changed)
      this->publishChange();
  }

  // Compatible types

  // std::string from char *
  template <>
  void UserVariable<std::string>::setValue(const char *value)
  {
    std::string svalue(value);
    bool changed = !m_known || svalue != m_value;
    assertTrueMsg(checkValue(svalue), "Error: " << value << " is an invalid value for this variable");
    m_value = svalue;
    m_known = true;
    if (changed)
      this->publishChange();
  }

  template <>
  template <>
  void UserVariable<double>::setValue(const int32_t &value)
  {
    double dvalue = (double) value;
    bool changed = !m_known || dvalue != m_value;
    assertTrueMsg(checkValue(dvalue), "Error: " << value << " is an invalid value for this variable");
    m_value = dvalue;
    m_known = true;
    if (changed)
      this->publishChange();
  }

  template <typename T>
  void UserVariable<T>::setUnknown()
  {
    bool changed = m_known;
    m_known = false;
    if (changed)
      this->publishChange();
  }

  template <typename T>
  void UserVariable<T>::reset()
  {
    bool changed = (m_known != m_initialKnown) || (m_value != m_initialValue);
    m_savedValue = m_value = m_initialValue;
    m_savedKnown = m_known = m_initialKnown;
    if (changed)
      this->publishChange();
  }

  template <typename T>
  void UserVariable<T>::saveCurrentValue()
  {
    m_savedValue = m_value;
    m_savedKnown = m_known;
  }

  template <typename T>
  void UserVariable<T>::restoreSavedValue()
  {
    bool changed = (m_known != m_savedKnown) || (m_value != m_savedValue);
    m_value = m_savedValue;
    m_known = m_savedKnown;
    if (changed)
      this->publishChange();
  }

  template <typename T>
  const std::string& UserVariable<T>::getName() const
  {
    return m_name;
  }

  template <typename T>
  const NodeId &UserVariable<T>::getNode() const
  {
    return m_node;
  }

  template <typename T>
  const ExpressionId& UserVariable<T>::getBaseVariable() const
  {
    return Expression::getId();
  }

  //
  // Explicit instantiations
  //

  template class UserVariable<double>;
  template class UserVariable<int32_t>;
  // template class UserVariable<uint16_t>;
  template class UserVariable<bool>;
  template class UserVariable<std::string>;


} // namespace PLEXIL
