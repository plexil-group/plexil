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

#include "ArrayVariable.hh"
#include "Constant.hh"
#include "Error.hh"

#include <cstdlib> // free()
#include <cstring> // strdup()

namespace PLEXIL
{

  template <typename T>
  ArrayVariable<T>::ArrayVariable()
    : NotifierImpl(),
      ExpressionImpl<ArrayImpl<T> >(),
      AssignableImpl<ArrayImpl<T> >(),
      m_size(NULL),
      m_initializer(NULL),
      m_name(NULL),
      m_maxSize(0),
      m_node(NULL),
      m_known(false),
      m_savedKnown(false),
      m_sizeIsGarbage(false),
      m_initializerIsGarbage(false)
  {
  }

  template <typename T>
  ArrayVariable<T>::ArrayVariable(ArrayImpl<T> const & initVal)
    : NotifierImpl(),
      ExpressionImpl<ArrayImpl<T> >(),
      AssignableImpl<ArrayImpl<T> >(),
      m_size(NULL),
      m_initializer(new Constant<ArrayImpl<T> >(initVal)),
      m_name(NULL),
      m_maxSize(0),
      m_node(NULL),
      m_known(false),
      m_savedKnown(false),
      m_sizeIsGarbage(false),
      m_initializerIsGarbage(true)
  {
  }

  template <typename T>
  ArrayVariable<T>::ArrayVariable(NodeConnector *node,
                                  char const *name,
                                  Expression *size,
                                  bool sizeIsGarbage)
    : NotifierImpl(),
      ExpressionImpl<ArrayImpl<T> >(),
      AssignableImpl<ArrayImpl<T> >(),
      m_size(size),
      m_initializer(NULL),
      m_name(strdup(name)),
      m_maxSize(0),
      m_node(node),
      m_known(false),
      m_savedKnown(false),
      m_sizeIsGarbage(sizeIsGarbage),
      m_initializerIsGarbage(false)
  {
  }

  template <typename T>
  ArrayVariable<T>::~ArrayVariable()
  {
    free((void *) m_name);
    if (m_initializerIsGarbage)
      delete m_initializer;
    if (m_sizeIsGarbage)
      delete m_size;
  }
  //
  // Essential Expression API
  //

  template <typename T>
  char const *ArrayVariable<T>::getName() const
  {
    if (m_name)
      return m_name;
    static char const *sl_anon = "anonymous";
    return sl_anon;
  }

  template <typename T>
  const char *ArrayVariable<T>::exprName() const
  {
    return "ArrayVariable";
  }

  template <typename T>
  bool ArrayVariable<T>::isKnown() const
  {
    return this->isActive() && m_known;
  }

  template <typename T>
  bool ArrayVariable<T>::getValuePointerImpl(ArrayImpl<T> const *&ptr) const
  {
    if (!this->isActive())
      return false;
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  bool ArrayVariable<T>::getMutableValuePointerImpl(ArrayImpl<T> *&ptr)
  {
    if (!this->isActive())
      return false;
    if (m_known)
      ptr = &m_value;
    return m_known;
  }

  template <typename T>
  void ArrayVariable<T>::handleActivate()
  {
    // Ensure maxSize spec is evaluated before initializer.
    if (m_size) {
      int32_t specSize;
      if (m_size->getValue(specSize)) {
        assertTrue_2(specSize >= 0, "Array initialization: Negative array size illegal");
        m_maxSize = (size_t) specSize;
      }
    }
    if (m_initializer) {
      ArrayImpl<T> const *valuePtr;
      if (m_initializer->getValuePointer(valuePtr)) {
        // If there is a max size, enforce it.
        // Else use the length of the initializer
        size_t size = valuePtr->size();
        if (m_size) {
          assertTrueMsg(size <= m_maxSize, "Array initialization: Initial value is larger than max size");
        }
        m_value = *valuePtr;
        if (m_size && size < m_maxSize)
          m_value.resize(m_maxSize);
        m_known = true;
      }
    }
    else {
      reserve();
    }
    if (m_known)
      this->publishChange(this);
  }

  template <typename T>
  void ArrayVariable<T>::handleDeactivate()
  {
    // Clear saved value
    m_savedValue.resize(0);
    m_savedKnown = false;
    if (m_initializer)
      m_initializer->deactivate();
  }

  template <typename T>
  void ArrayVariable<T>::printSpecialized(std::ostream &s) const
  {
    s << m_name << ' ';
    if (m_size)
      s << "size = " << m_maxSize << ' ';
  }

  template <typename T>
  void ArrayVariable<T>::setValueImpl(ArrayImpl<T> const &value)
  {
    bool changed = !m_known || value != m_value;
    size_t newSize = value.size();
    assertTrue_2(!m_size || newSize <= m_maxSize,
                 "ArrayVariable::setValue: New value is bigger than array declared size");
    m_value = value;
    m_known = true;
    // TODO: find more efficient way to handle arrays smaller than max
    if (newSize < m_maxSize)
      m_value.resize(m_maxSize);
    if (changed)
      this->publishChange(this);
  }

  template <typename T>
  void ArrayVariable<T>::setUnknown()
  {
    bool changed = m_known;
    m_known = false;
    if (changed)
      this->publishChange(this);
  }

  // This should only be called when inactive, therefore doesn't need to report changes.
  template <typename T>
  void ArrayVariable<T>::reset()
  {
    assertTrue_2(!this->isActive(), "ArrayVariable: reset while active");
    m_savedKnown = m_known = false;
    m_value.reset();
    m_savedValue.reset();
  }

  template <typename T>
  void ArrayVariable<T>::saveCurrentValue()
  {
    m_savedValue = m_value;
    m_savedKnown = m_known;
  }

  // Should only be called when active.
  template <typename T>
  void ArrayVariable<T>::restoreSavedValue()
  {
    bool changed = (m_known != m_savedKnown) || (m_value != m_savedValue);
    m_value = m_savedValue;
    m_known = m_savedKnown;
    if (changed)
      this->publishChange(this);
  }

  template <typename T>
  Value ArrayVariable<T>::getSavedValue() const
  {
    return Value(m_savedValue);
  }

  template <typename T>
  void ArrayVariable<T>::setName(const std::string &name)
  {
    if (m_name)
      delete m_name;
    m_name = strdup(name.c_str());
  }

  template <typename T>
  NodeConnector const *ArrayVariable<T>::getNode() const
  {
    return m_node;
  }

  template <typename T>
  NodeConnector *ArrayVariable<T>::getNode()
  {
    return m_node;
  }

  template <typename T>
  Assignable *ArrayVariable<T>::getBaseVariable()
  {
    return Assignable::asAssignable();
  }

  template <typename T>
  Assignable const *ArrayVariable<T>::getBaseVariable() const
  {
    return Assignable::asAssignable();
  }

  template <typename T>
  void ArrayVariable<T>::setInitializer(Expression *expr, bool garbage)
  {
    assertTrue_2(!m_initializer, "setInitializer() called on an array variable that already has an initializer");
    assertTrueMsg(expr->valueType() == this->valueType() || expr->valueType() == UNKNOWN_TYPE,
                  "Array variable type, " << valueTypeName(this->valueType())
                  << ", differs from initializer's type, " << valueTypeName(expr->valueType()));
    int32_t size;
    ArrayImpl<T> const *temp;
    if (m_size && m_size->getValue(size) && expr->getValuePointer(temp))
      assertTrue_2(size >= temp->size(),
                   "Array variable initial value is larger than declared array size");
    m_initializer = expr;
    m_initializerIsGarbage = garbage;
  }

  template <typename T>
  VariableConflictSet &ArrayVariable<T>::getConflictSet()
  {
    return m_conflicts;
  }

  template <typename T>
  void ArrayVariable<T>::reserve()
  {
    if (m_size && m_maxSize) {
      m_value.resize(m_maxSize);
      m_known = true; // array is known, not its contents
    }
  }
  
  //
  // Explicit instantiations
  //

  template class ArrayVariable<bool>;
  template class ArrayVariable<int32_t>;
  template class ArrayVariable<double>;
  template class ArrayVariable<std::string>;

} // namespace PLEXIL

