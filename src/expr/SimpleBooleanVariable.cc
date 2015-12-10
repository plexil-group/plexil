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

#include "SimpleBooleanVariable.hh"

#include "Error.hh"

#include <cstdlib> // free()
#include <cstring> // strdup()

namespace PLEXIL
{

  SimpleBooleanVariable::SimpleBooleanVariable()
    : NotifierImpl(),
      ExpressionImpl<bool>(),
    AssignableImpl<bool>(),
    m_name(NULL),
    m_value(false)
  {
  }

  SimpleBooleanVariable::~SimpleBooleanVariable()
  {
    free((void *) m_name);
  }

  char const *SimpleBooleanVariable::exprName() const
  {
    return "InternalVariable";
  }

  char const *SimpleBooleanVariable::getName() const
  {
    if (m_name)
      return m_name;
    static char const *sl_dummy = "";
    return sl_dummy;
  }

  void SimpleBooleanVariable::setName(std::string const &name)
  {
    if (m_name)
      delete m_name;
    m_name = strdup(name.c_str());
  }

  void SimpleBooleanVariable::printSpecialized(std::ostream &s) const
  {
    s << m_name << ' ';
  }

  Assignable *SimpleBooleanVariable::getBaseVariable()
  {
    return static_cast<Assignable *>(this);
  }

  Assignable const *SimpleBooleanVariable::getBaseVariable() const
  {
    return static_cast<Assignable const *>(this);
  }

  //
  // Value API
  //
  
  // A SimpleBooleanVariable's value is known whenever it is active.
  bool SimpleBooleanVariable::isKnown() const
  {
    return this->isActive();
  }

  bool SimpleBooleanVariable::getValueImpl(bool &result) const
  {
    if (this->isActive()) {
      result = m_value;
      return true;
    }
    return false;
  }

  void SimpleBooleanVariable::setValueImpl(bool const &value)
  {
    if (this->isActive()) {
      if (m_value != value) {
        m_value = value;
        this->publishChange(this);
      }
    }
  }

  void SimpleBooleanVariable::setUnknown()
  {
    assertTrue_2(ALWAYS_FAIL, "Not implemented for this class");
  }

  void SimpleBooleanVariable::reset()
  {
    m_value = false;
  }

  void SimpleBooleanVariable::saveCurrentValue()
  {
    assertTrue_2(ALWAYS_FAIL, "Not implemented for this class");
  }

  void SimpleBooleanVariable::restoreSavedValue()
  {
    assertTrue_2(ALWAYS_FAIL, "Not implemented for this class");
  }

  Value SimpleBooleanVariable::getSavedValue() const
  {
    assertTrue_2(ALWAYS_FAIL, "Not implemented for this class");
    return Value();
  }
  
  //
  // NotifierImpl API
  // 

  // SimpleBooleanVariable doesn't depend on anything else.
  void SimpleBooleanVariable::notifyChanged(Expression const * /* src */)
  {
  }

} // namespace PLEXIL
