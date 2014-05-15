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
//#include "Node.hh"

namespace PLEXIL {

  template <typename T>
  UserVariable<T>::UserVariable()
    : NotifierImpl(),
      ExpressionImpl<T>(),
      Assignable(),
      m_name("anonymous"),
      m_known(false),
      m_savedKnown(false)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const T &initVal)
  : NotifierImpl(),
    ExpressionImpl<T>(),
    Assignable(),
    m_initializer((new Constant<T>(initVal))->getId()),
    m_name("anonymous"),
    m_known(false),
    m_savedKnown(false),
    m_initializerIsGarbage(true)
  {
  }

  template <typename T>
  UserVariable<T>::UserVariable(const NodeId &node,
                                const std::string &name,
                                const ExpressionId &initializer,
                                bool initializerIsGarbage)
    : NotifierImpl(),
      ExpressionImpl<T>(),
      Assignable(),
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
  bool UserVariable<T>::getMutableValuePointer(T *&ptr)
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
    if (m_initializer.isId()) {
      m_initializer->activate();
      T const *valptr;
      m_known = m_initializer->getValuePointer(valptr);
      m_value = *valptr;
    }
    if (m_known)
      this->publishChange(getId());
  }

  template <>
  void UserVariable<bool>::handleActivate()
  {
    if (m_initializer.isId()) {
      m_initializer->activate();
      m_known = m_initializer->getValue(m_value);
    }
    if (m_known)
      this->publishChange(getId());
  }

  template <>
  void UserVariable<int32_t>::handleActivate()
  {
    if (m_initializer.isId()) {
      m_initializer->activate();
      m_known = m_initializer->getValue(m_value);
    }
    if (m_known)
      this->publishChange(getId());
  }

  template <>
  void UserVariable<double>::handleActivate()
  {
    if (m_initializer.isId()) {
      m_initializer->activate();
      m_known = m_initializer->getValue(m_value);
    }
    if (m_known)
      this->publishChange(getId());
  }

  template <typename T>
  void UserVariable<T>::handleDeactivate()
  {
    if (m_initializer.isId())
      m_initializer->deactivate();
  }


  template <typename T>
  void UserVariable<T>::setValue(ExpressionId const &valex)
  {
    assertTrue_2(this->isActive(), "setValue while inactive");
    T const *newval;
    if (valex->getValuePointer(newval))
      setValue(*newval);
    else 
      setUnknown();
  }

  // Scalar types
  template <>
  void UserVariable<bool>::setValue(ExpressionId const &valex)
  {
    assertTrue_2(this->isActive(), "setValue while inactive");
    bool newval;
    if (valex->getValue(newval))
      setValue(newval);
    else 
      setUnknown();
  }

  template <>
  void UserVariable<int32_t>::setValue(ExpressionId const &valex)
  {
    assertTrue_2(this->isActive(), "setValue while inactive");
    int32_t newval;
    if (valex->getValue(newval))
      setValue(newval);
    else 
      setUnknown();
  }

  template <>
  void UserVariable<double>::setValue(ExpressionId const &valex)
  {
    assertTrue_2(this->isActive(), "setValue while inactive");
    double newval;
    if (valex->getValue(newval))
      setValue(newval);
    else 
      setUnknown();
  }

  template <typename T>
  void UserVariable<T>::setValue(const T &value)
  {
    bool changed = !m_known || value != m_value;
    assertTrueMsg(checkValue(value), "setValue: Value is invalid for this variable");
    m_value = value;
    m_known = true;
    if (changed)
      this->publishChange(getId());
  }

  template <typename T>
  bool UserVariable<T>::checkValue(const T &value)
  {
    return true;
  }

  template <typename T>
  void UserVariable<T>::setUnknown()
  {
    bool changed = m_known;
    m_known = false;
    if (changed)
      this->publishChange(getId());
  }

  // This should only be called when inactive, therefore doesn't need to report changes.
  template <typename T>
  void UserVariable<T>::reset()
  {
    assertTrue_2(!this->isActive(), "UserVariable: reset while active");
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
      this->publishChange(getId());
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
  const AssignableId& UserVariable<T>::getBaseVariable() const
  {
    return Assignable::getAssignableId();
  }

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
                                 const std::string &name,
                                 const ExpressionId &init,
                                 bool initIsGarbage)
    : UserVariable<std::string>(node, name, init, initIsGarbage)
  {
  }

  StringVariable::~StringVariable()
  {
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
  
  //
  // Explicit instantiations
  //

  template class UserVariable<bool>;
  // template class UserVariable<uint16_t>;
  template class UserVariable<int32_t>;
  template class UserVariable<double>;
  // see above for string variables

  // Required by ArrayVariable
  template class UserVariable<Array<bool> >;
  template class UserVariable<Array<int32_t> >;
  template class UserVariable<Array<double> >;
  template class UserVariable<Array<std::string> >;

} // namespace PLEXIL
