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

  template <typename T>
  UserVariable<T>::UserVariable()
    : Assignable(),
      ExpressionImpl<T>(),
      m_name("anonymous"),
      m_known(false),
      m_initialKnown(false),
      m_savedKnown(false)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const T &initVal)
    : Assignable(),
      ExpressionImpl<T>(),
      m_name("anonymous"),
      m_value(initVal),
      m_initialValue(initVal),
      m_savedValue(initVal),
      m_known(true),
      m_initialKnown(true),
      m_savedKnown(true)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const NodeId &node,
                                const std::string &name)
    : Assignable(),
      ExpressionImpl<T>(),
      m_node(node),
      m_name(name),
      m_known(false),
      m_initialKnown(false),
      m_savedKnown(false)
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
  bool UserVariable<T>::isKnown() const
  {
    return isActive() && m_known;
  }

  template <typename T>
  bool UserVariable<T>::getValueImpl(T &result) const
  {
    if (!isActive())
      return false;
    if (m_known)
      result = m_value;
    return m_known;
  }

  template <typename T>
  bool UserVariable<T>::getValuePointerImpl(T const *&ptr) const
  {
    if (!isActive())
      return false;
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  void UserVariable<T>::setInitialValue(const T &value)
  {
    assertTrue_2(checkValue(value), "setInitialValue: Value is invalid for this variable");
    m_initialValue = m_value = value;
    m_initialKnown = m_known = true;
  }

  template <typename T>
  void UserVariable<T>::setValue(const T &value)
  {
    bool changed = !m_known || value != m_value;
    assertTrueMsg(checkValue(value), "setValue: Value is invalid for this variable");
    m_value = value;
    m_known = true;
    if (changed)
      this->publishChange();
  }

  template <typename T>
  void UserVariable<T>::setInitialUnknown()
  {
    m_initialKnown = m_known = false;
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
    bool changed = ((m_known != m_initialKnown)
                    || (m_known && m_initialKnown && (m_value != m_initialValue)));
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
  // For access by array variables
  //
  template <typename T>
  T &UserVariable<T>::getValueReference()
  {
    return m_value;
  }
  
  //
  // Explicit instantiations
  //

  template class UserVariable<bool>;
  // template class UserVariable<uint16_t>;
  template class UserVariable<int32_t>;
  template class UserVariable<double>;
  //template class UserVariable<std::string>;

  template class UserVariable<std::vector<bool> >;
  // template class UserVariable<std::vector<uint16_t> >;
  template class UserVariable<std::vector<int32_t> >;
  template class UserVariable<std::vector<double> >;
  template class UserVariable<std::vector<std::string> >;

  //
  // Specializations
  //

  //
  // StringVariable wrapper
  //

  StringVariable::StringVariable()
    : UserVariable<std::string>()
  {
  }

  StringVariable::StringVariable(const std::string &initVal)
    : UserVariable<std::string>(initVal)
  {
  }

  StringVariable::StringVariable(const char *initVal)
    : UserVariable<std::string>(std::string(initVal))
  {
  }

  StringVariable::StringVariable(const NodeId &node,
                                 const std::string &name)
    : UserVariable<std::string>(node, name)
  {
  }

  StringVariable::~StringVariable()
  {
  }

  void StringVariable::setInitialValue(const char *val)
  {
    UserVariable<std::string>::setInitialValue(std::string(val));
  }

  // Because C++ sucks.
  void StringVariable::setInitialValue(const std::string &val)
  {
    UserVariable<std::string>::setInitialValue(val);
  }

  void StringVariable::setValue(const char *val)
  {
    UserVariable<std::string>::setValue(std::string(val));
  }

  // Because C++ sucks.
  void StringVariable::setValue(const std::string &val)
  {
    UserVariable<std::string>::setValue(val);
  }

} // namespace PLEXIL
