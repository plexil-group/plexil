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

#include "Alias.hh"
#include "Error.hh"
#include "Value.hh"

namespace PLEXIL
{
  //
  // Alias
  //

  Alias::Alias(NodeConnector *node,
               const std::string &name,
               Expression *original,
               bool garbage)
    : NotifierImpl(),
      m_exp(original),
      m_node(node),
      m_name(name),
      m_garbage(garbage)
  {
    assertTrue_2(original, "Alias constructor: Null expression");
    m_exp->addListener(this);
  }

  Alias::~Alias()
  {
    m_exp->removeListener(this);
    if (m_garbage)
      delete m_exp;
  }

  const char *Alias::exprName() const
  {
    return "InAlias";
  }
   
  const ValueType Alias::valueType() const
  {
    return m_exp->valueType();
  }
  
  bool Alias::isKnown() const
  {
    if (!isActive())
      return false;
    return m_exp->isKnown();
  }
  
  bool Alias::isAssignable() const
  {
    return false;
  }

  bool Alias::isConstant() const
  {
    return m_exp->isConstant();
  }

  Expression *Alias::getBaseExpression()
  {
    return m_exp->getBaseExpression();
  }

  Expression const *Alias::getBaseExpression() const
  {
    return m_exp->getBaseExpression();
  }

  void Alias::printValue(std::ostream &s) const
  {
    m_exp->printValue(s);
  }

  bool Alias::getValue(bool &var) const
  {
    if (!isActive())
      return false;
    return m_exp->getValue(var);
  }

  bool Alias::getValue(double &var) const
  {
    if (!isActive())
      return false;
    return m_exp->getValue(var);
  }

  bool Alias::getValue(uint16_t &var) const
  {
    if (!isActive())
      return false;
    return m_exp->getValue(var);
  }

  bool Alias::getValue(int32_t &var) const
  {
    if (!isActive())
      return false;
    return m_exp->getValue(var);
  }

  bool Alias::getValue(std::string &var) const
  {
    if (!isActive())
      return false;
    return m_exp->getValue(var);
  }

  bool Alias::getValuePointer(std::string const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  bool Alias::getValuePointer(Array const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  bool Alias::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  bool Alias::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  bool Alias::getValuePointer(RealArray const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  bool Alias::getValuePointer(StringArray const *&ptr) const
  {
    if (!isActive())
      return false;
    return m_exp->getValuePointer(ptr);
  }

  Value Alias::toValue() const
  {
    if (!isActive())
      return Value(0, m_exp->valueType());
    else
      return m_exp->toValue();
  }

  // No-op because the variable should already be active.
  void Alias::handleActivate()
  {
  }

  void Alias::handleDeactivate()
  {
  }

  //
  // InOutAlias
  //

  InOutAlias::InOutAlias(NodeConnector *node,
                         const std::string &name,
                         Expression *original,
                         bool garbage)
    : Alias(node, name, original, garbage),
      Assignable(),
      m_target(original->asAssignable())
  {
    assertTrue_2(original->isAssignable(),
                 "InOutAlias constructor: target expression is not assignable");
  }

  InOutAlias::~InOutAlias()
  {
  }

  const char *InOutAlias::exprName() const
  {
    return "InOutAlias";
  }
  
  bool InOutAlias::isAssignable() const
  {
    return m_target->isAssignable();
  }

  void InOutAlias::reset()
  {
    assertTrue_2(!isActive(), "InOutAlias: reset while active");
  }

  void InOutAlias::setUnknown()
  {
    assertTrue_2(isActive(), "InOutAlias: setUnknown while inactive");
    m_target->setUnknown();
  }

  void InOutAlias::setValue(const double &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(const int32_t &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(const uint16_t &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(const bool &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(const std::string &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(const char *val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(BooleanArray const &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(IntegerArray const &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(RealArray const &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(StringArray const &val)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(val);
  }

  void InOutAlias::setValue(Expression const *valex)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(valex);
  }

  void InOutAlias::setValue(Value const &value)
  {
    assertTrue_2(isActive(), "InOutAlias: setValue while inactive");
    m_target->setValue(value);
  }

  bool InOutAlias::getMutableValuePointer(std::string *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  bool InOutAlias::getMutableValuePointer(Array *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  bool InOutAlias::getMutableValuePointer(BooleanArray *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  bool InOutAlias::getMutableValuePointer(IntegerArray *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  bool InOutAlias::getMutableValuePointer(RealArray *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  bool InOutAlias::getMutableValuePointer(StringArray *& ptr)
  {
    if (!isActive())
      return false;
    return m_target->getMutableValuePointer(ptr);
  }

  void InOutAlias::saveCurrentValue()
  {
    assertTrue_2(isActive(), "InOutAlias: saveCurrentValue while inactive");
    m_target->saveCurrentValue();
  }

  void InOutAlias::restoreSavedValue()
  {
    assertTrue_2(isActive(), "InOutAlias: restoreSavedValue while inactive");
    m_target->restoreSavedValue();
  }

  const std::string& InOutAlias::getName() const
  {
    return m_name;
  }

  NodeConnector const *InOutAlias::getNode() const
  {
    return m_node;
  }

  NodeConnector *InOutAlias::getNode()
  {
    return m_node;
  }

  Assignable *InOutAlias::getBaseVariable()
  {
    return m_target->getBaseVariable();
  }

  Assignable const *InOutAlias::getBaseVariable() const
  {
    return m_target->getBaseVariable();
  }

} // namespace PLEXIL
