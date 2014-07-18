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

#include "Constant.hh"
#include "Error.hh"
#include "ExpressionConstants.hh"
//#include "NodeConnector.hh"
#include "Value.hh"

namespace PLEXIL {

  template <typename T>
  UserVariable<T>::UserVariable()
    : NotifierImpl(),
      ExpressionImpl<T>(),
    AssignableImpl<T>(),
    m_initializer(NULL),
    m_name("anonymous"),
    m_known(false),
    m_savedKnown(false)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const T &initVal)
  : NotifierImpl(),
    ExpressionImpl<T>(),
    AssignableImpl<T>(),
    m_initializer(new Constant<T>(initVal)),
    m_name("anonymous"),
    m_known(false),
    m_savedKnown(false),
    m_initializerIsGarbage(true)
  {
  }

  // Only two possible constant initializers for BooleanVariable
  template <>
  UserVariable<bool>::UserVariable(const bool &initVal)
  : NotifierImpl(),
    ExpressionImpl<bool>(),
    AssignableImpl<bool>(),
    m_initializer(initVal ? TRUE_EXP() : FALSE_EXP()),
    m_name("anonymous"),
    m_known(false),
    m_savedKnown(false),
    m_initializerIsGarbage(false)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(NodeConnector *node,
                                const std::string &name,
                                Expression *initializer,
                                bool initializerIsGarbage)
    : NotifierImpl(),
      ExpressionImpl<T>(),
    AssignableImpl<T>(),
    m_initializer(initializer),
    m_node(node),
    m_name(name),
    m_known(false),
    m_savedKnown(false),
    m_initializerIsGarbage(initializerIsGarbage)
  {
  }
    
  /**
   * @brief Destructor.
   * @note Specializations may have more work to do.
   */
  template <typename T>
  UserVariable<T>::~UserVariable()
  {
    if (m_initializerIsGarbage)
      delete (Expression *) m_initializer;
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
    return this->isActive() && m_known;
  }

  template <typename T>
  bool UserVariable<T>::getValueImpl(T &result) const
  {
    if (!this->isActive())
      return false;
    if (m_known)
      result = m_value;
    return m_known;
  }

  template <typename T>
  bool UserVariable<T>::getValuePointerImpl(T const *&ptr) const
  {
    if (!this->isActive())
      return false;
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool UserVariable<T>::getValuePointerImpl(Array const *&ptr) const
  {
    assertTrue_2(ALWAYS_FAIL, "UserVariable::getValuePointer type error");
    return false;
  }

  template <typename T>
  bool UserVariable<T>::getMutableValuePointerImpl(T *&ptr)
  {
    if (!this->isActive())
      return false;
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  // A variable takes its initial value when first activated,
  // or after being reset and reactivated.
  template <typename T>
  void UserVariable<T>::handleActivate()
  {
    if (m_initializer) {
      m_initializer->activate();
      m_known = m_initializer->getValue(m_value);
    }
    if (m_known)
      this->publishChange(this);
  }

  template <typename T>
  void UserVariable<T>::handleDeactivate()
  {
    // Clear saved value
    m_savedKnown = false;
    if (m_initializer)
      m_initializer->deactivate();
  }

  // Specializations for non-scalar types
  template <>
  void UserVariable<std::string>::handleActivate()
  {
    if (m_initializer) {
      m_initializer->activate();
      std::string const *valptr;
      m_known = m_initializer->getValuePointer(valptr);
      m_value = *valptr;
    }
    if (m_known)
      this->publishChange(this);
  }

  template <>
  void UserVariable<std::string>::handleDeactivate()
  {
    // Clear saved value
    m_savedValue.clear();
    m_savedKnown = false;
    if (m_initializer)
      m_initializer->deactivate();
  }

  template <typename T>
  void UserVariable<T>::printSpecialized(std::ostream &s) const
  {
    s << m_name << ' ';
  }

  template <typename T>
  void UserVariable<T>::setValueImpl(const T &value)
  {
    bool changed = !m_known || value != m_value;
    m_value = value;
    m_known = true;
    if (changed)
      this->publishChange(this);
  }

  template <typename T>
  void UserVariable<T>::setUnknown()
  {
    bool changed = m_known;
    m_known = false;
    if (changed)
      this->publishChange(this);
  }

  // This should only be called when inactive, therefore doesn't need to report changes.
  template <typename T>
  void UserVariable<T>::reset()
  {
    assertTrueMsg(!this->isActive(), "UserVariable " << *this << " reset while active");
    m_savedKnown = m_known = false;
  }

  template <typename T>
  void UserVariable<T>::saveCurrentValue()
  {
    m_savedValue = m_value;
    m_savedKnown = m_known;
  }

  // Should only be called when active.
  template <typename T>
  void UserVariable<T>::restoreSavedValue()
  {
    bool changed = (m_known != m_savedKnown) || (m_value != m_savedValue);
    m_value = m_savedValue;
    m_known = m_savedKnown;
    if (changed)
      this->publishChange(this);
  }

  template <typename T>
  const std::string &UserVariable<T>::getName() const
  {
    return m_name;
  }

  template <typename T>
  void UserVariable<T>::setName(const std::string &name)
  {
    m_name = name;
  }

  template <typename T>
  NodeConnector *UserVariable<T>::getNode()
  {
    return m_node;
  }

  template <typename T>
  NodeConnector const *UserVariable<T>::getNode() const
  {
    return m_node;
  }

  template <typename T>
  Assignable *UserVariable<T>::getBaseVariable()
  {
    return Assignable::asAssignable();
  }

  template <typename T>
  Assignable const *UserVariable<T>::getBaseVariable() const
  {
    return Assignable::asAssignable();
  }

  //
  // Explicit instantiations
  //

  template class UserVariable<bool>;
  // template class UserVariable<uint16_t>;
  template class UserVariable<int32_t>;
  template class UserVariable<double>;
  template class UserVariable<std::string>;

} // namespace PLEXIL
