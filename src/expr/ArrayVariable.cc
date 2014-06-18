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
#include "ArrayConstant.hh"

namespace PLEXIL
{

  template <typename T>
  ArrayVariable<T>::ArrayVariable()
    : UserVariable<ArrayImpl<T> >()
  {
  }

  template <typename T>
  ArrayVariable<T>::ArrayVariable(ArrayImpl<T> const & initVal)
    : UserVariable<ArrayImpl<T> >(NodeConnectorId::noId(),
                                  std::string("anonymous"),
                                  (new ArrayConstant<T >(initVal))->getId(),
      true)
  {
  }

  template <typename T>
  ArrayVariable<T>::ArrayVariable(const NodeConnectorId &node,
                                  const std::string &name,
                                  const ExpressionId &size,
                                  const ExpressionId &initializer,
                                  bool sizeIsGarbage,
                                  bool initializerIsGarbage)
    : UserVariable<ArrayImpl<T> >(node, name, initializer, initializerIsGarbage),
      m_size(size),
      m_sizeIsGarbage(sizeIsGarbage)
  {
  }

  template <typename T>
  ArrayVariable<T>::~ArrayVariable()
  {
    if (m_sizeIsGarbage)
      delete (Expression *) m_size;
  }

  template <typename T>
  void ArrayVariable<T>::handleActivate()
  {
    if (Superclass::m_initializer.isId()) {
      ArrayImpl<T> const *valuePtr;
      if (Superclass::m_initializer->getValuePointer(valuePtr)) {
        // Choose the greater of the spec'd size or the length of the initializer
        size_t size = valuePtr->size();
        if (m_size.isId()) {
          int32_t specSize;
          if (m_size->getValue(specSize)) {
            assertTrue_2(specSize >= 0, "Array initialization: Negative array size illegal");
            if (size < (size_t) specSize)
              size = (size_t) specSize;
          }
        }
        Superclass::m_value.resize(size);
        Superclass::m_value = *valuePtr;
        Superclass::m_known = true;
      }
    }
    else {
      reserve();
    }
    if (Superclass::m_known)
      this->publishChange(Expression::getId());
  }

  template <typename T>
  bool ArrayVariable<T>::getValuePointerImpl(Array const *&result) const
  {
    if (!this->isActive() || !Superclass::m_known)
      return false;
    result = static_cast<Array const *>(&(this->m_value));
    return true;
  }

  template <typename T>
  bool ArrayVariable<T>::getMutableValuePointer(Array *&result)
  {
    if (!this->isActive() || !Superclass::m_known)
      return false;
    result = static_cast<Array *>(&(this->m_value));
    return true;
  }

  template <typename T>
  void ArrayVariable<T>::reserve()
  {
    if (m_size.isId()) {
      int32_t size;
      if (m_size->getValue(size)) {
        assertTrue_2(size >= 0, "Array initialization: Negative array size illegal");
        Superclass::m_value.resize(size);
        Superclass::m_known = true; // array is known, not its contents
      }
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

