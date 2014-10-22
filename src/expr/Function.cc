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

#include "Function.hh"

#include "ArrayImpl.hh"
#include "ExprVec.hh"
#include "Operator.hh"
#include "Value.hh"

namespace PLEXIL
{
  Function::Function(Operator const *op, ExprVec *exprs)
    : NotifierImpl(),
      m_op(op),
      m_exprVec(exprs),
      m_valueCache(op->allocateCache())
  {
    m_exprVec->addListener(this);
  }

  Function::Function(Operator const *op,
                     Expression *expr, 
                     bool garbage)
    : NotifierImpl(),
      m_op(op),
      m_exprVec(makeExprVec(1)),
      m_valueCache(op->allocateCache())
  {
    m_exprVec->setArgument(0, expr, garbage);
    m_exprVec->addListener(this);
  }

  Function::Function(Operator const *op,
                     Expression *expr1, Expression *expr2, 
                     bool garbage1, bool garbage2)
    : NotifierImpl(),
      m_op(op),
      m_exprVec(makeExprVec(2)),
      m_valueCache(op->allocateCache())
  {
    m_exprVec->setArgument(0, expr1, garbage1);
    m_exprVec->setArgument(1, expr2, garbage2);
    m_exprVec->addListener(this);
  }

  Function::~Function()
  {
    m_op->deleteCache(m_valueCache);
    m_exprVec->removeListener(this);
    delete m_exprVec;
  }

  const char *Function::exprName() const
  {
    return m_op->getName().c_str();
  }

  const ValueType Function::valueType() const
  {
    return m_op->valueType();
  }

  void Function::handleActivate()
  {
    m_exprVec->activate();
  }

  void Function::handleDeactivate()
  {
    m_exprVec->deactivate();
  }

  bool Function::isKnown() const
  {
    if (!isActive())
      return false;
    // Delegate to operator
    return m_op->calcNative(m_valueCache, *m_exprVec);
  }

  void Function::printValue(std::ostream &s) const
  {
    if (!isActive()) {
      s << "UNKNOWN";
      return;
    }
    m_op->printValue(s, m_valueCache, *m_exprVec);
  }

  void Function::printSubexpressions(std::ostream &s) const
  {
    m_exprVec->print(s);
  }

  Value Function::toValue() const
  {
    return m_op->toValue(m_valueCache, *m_exprVec);
  }

  bool Function::getValue(bool &result) const
  {
    if (!isActive())
      return false;
    return m_exprVec->apply(m_op, result);
  }

  bool Function::getValue(int32_t &result) const
  {
    if (!isActive())
      return false;
    return m_exprVec->apply(m_op, result);
  }

  bool Function::getValue(double &result) const
  {
    if (!isActive())
      return false;
    return m_exprVec->apply(m_op, result);
  }

  bool Function::getValue(std::string &result) const
  {
    if (!isActive())
      return false;
    return m_exprVec->apply(m_op, result);
  }

  bool Function::getValuePointer(std::string const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<std::string *>(m_valueCache));
    if (result)
      ptr = static_cast<std::string const *>(m_valueCache); // trust me
    return result;
  }

  // Generic Array
  bool Function::getValuePointer(Array const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<Array *>(m_valueCache));
    if (result)
      ptr = static_cast<Array const *>(m_valueCache); // trust me
    return result;
  }

  // Specific array types
  bool Function::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<BooleanArray *>(m_valueCache));
    if (result)
      ptr = static_cast<BooleanArray const *>(m_valueCache); // trust me
    return result;
  }

  bool Function::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<IntegerArray *>(m_valueCache));
    if (result)
      ptr = static_cast<IntegerArray const *>(m_valueCache); // trust me
    return result;
  }

  bool Function::getValuePointer(RealArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<RealArray *>(m_valueCache));
    if (result)
      ptr = static_cast<RealArray const *>(m_valueCache); // trust me
    return result;
  }

  bool Function::getValuePointer(StringArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = m_exprVec->apply(m_op, *static_cast<StringArray *>(m_valueCache));
    if (result)
      ptr = static_cast<StringArray const *>(m_valueCache); // trust me
    return result;
  }

} // namespace PLEXIL
