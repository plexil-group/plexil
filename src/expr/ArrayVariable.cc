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
    : UserVariable<std::vector<T> >()
  {
  }

  // TEMPORARY, will go away soon
  template <typename T>
  ArrayVariable<T>::ArrayVariable(std::vector<T> const & initVal)
    : UserVariable<std::vector<T> >(NodeId::noId(),
                                    std::string("anonymous"),
                                    (new ArrayConstant<T>(initVal))->getId(),
                                    true)
  {
  }

  template <typename T>
  ArrayVariable<T>::ArrayVariable(const NodeId &node,
                                  const std::string &name,
                                  const ExpressionId &size,
                                  const ExpressionId &initializer,
                                  bool sizeIsGarbage,
                                  bool initializerIsGarbage)
    : UserVariable<std::vector<T> >(node, name, initializer, initializerIsGarbage),
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
  void ArrayVariable<T>::reset()
  {
    Superclass::reset();
    m_elementKnown.clear();
  }

  template <typename T>
  void ArrayVariable<T>::handleActivate()
  {
    if (Superclass::m_initializer.isId()) {
      std::vector<T> const *valuePtr;
      std::vector<bool> const *knownPtr;
      if (Superclass::m_initializer->getArrayContents(valuePtr, knownPtr)) {
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
        Superclass::m_value.reserve(size);
        Superclass::m_value = *valuePtr;
        m_elementKnown.reserve(size);
        m_elementKnown = *knownPtr;
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
  void ArrayVariable<T>::setValue(ExpressionId const &valex)
  {
    assertTrue_2(this->isActive(), "setValue while inactive");
    bool changed = false;
    std::vector<T> const *newValPtr;
    std::vector<bool> const *newKnownPtr;
    if (valex->getArrayContents(newValPtr, newKnownPtr)) {
      // is new value different?
      // TODO: evaluate cost of array comparisons
      if (!Superclass::m_known
          || m_elementKnown != *newKnownPtr
          || Superclass::m_value != *newValPtr) {
        Superclass::m_value = *newValPtr;
        Superclass::m_known = true;
        m_elementKnown = *newKnownPtr;
        changed = true;
      }
    }
    else if (Superclass::m_known) {
      Superclass::m_known = false;
      changed = true;
    }
    if (changed)
      this->publishChange(Expression::getId());
  }

  template <typename T>
  void ArrayVariable<T>::setValue(std::vector<T> const &val)
  {
    Superclass::setValue(val);
    if (Superclass::m_known)
      m_elementKnown = std::vector<bool>(Superclass::m_value.size(),
                                         true);
  }

  template <typename T>
  void ArrayVariable<T>::reserve()
  {
    if (m_size.isId()) {
      int32_t size;
      if (m_size->getValue(size)) {
        assertTrue_2(size >= 0, "Array initialization: Negative array size illegal");
        Superclass::m_value.reserve(size);
        m_elementKnown.reserve(size);
        Superclass::m_known = true; // array is known, not its contents
      }
    }
  }

  template <typename T>
  void ArrayVariable<T>::saveCurrentValue()
  {
    Superclass::saveCurrentValue();
    if (Superclass::m_known)
      m_savedElementKnown = m_elementKnown;
  }

  template <typename T>
  void ArrayVariable<T>::restoreSavedValue()
  {
    Superclass::restoreSavedValue();
    if (Superclass::m_savedKnown)
      m_elementKnown = m_savedElementKnown;
  }

  template <typename T>
  bool ArrayVariable<T>::getArrayContentsImpl(std::vector<T> const *&valuePtr, 
                                              std::vector<bool> const *&knownPtr) const
  {
    if (!Superclass::getValuePointerImpl(valuePtr))
      return false;
    knownPtr = &m_elementKnown;
    return true;
  }

  template <typename T>
  bool ArrayVariable<T>::getMutableArrayContents(std::vector<T> *&valuePtr, 
                                                 std::vector<bool> *&knownPtr)
  {
    if (!Superclass::getMutableValuePointer(valuePtr))
      return false;
    knownPtr = &m_elementKnown;
    return true;
  }
  
  //
  // Explicit instantiations
  //

  template class ArrayVariable<bool>;
  template class ArrayVariable<int32_t>;
  template class ArrayVariable<double>;
  template class ArrayVariable<std::string>;

} // namespace PLEXIL

