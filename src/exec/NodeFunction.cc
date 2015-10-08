/* Copyright (c) 2006-2015, Universities Space Research Association (USRA).
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

#include "NodeFunction.hh"

#include "ArrayImpl.hh"
#include "NodeOperator.hh"
#include "Value.hh"

namespace PLEXIL
{
  NodeFunction::NodeFunction(NodeOperator const *op, Node *node)
    : NotifierImpl(),
      m_op(op),
      m_node(node),
      m_valueCache(op->allocateCache())
  {
  }

  NodeFunction::~NodeFunction()
  {
    m_op->deleteCache(m_valueCache);
  }

  const char *NodeFunction::exprName() const
  {
    return m_op->getName().c_str();
  }

  const ValueType NodeFunction::valueType() const
  {
    return m_op->valueType();
  }

  void NodeFunction::handleActivate()
  {
  }

  void NodeFunction::handleDeactivate()
  {
  }

  bool NodeFunction::isKnown() const
  {
    if (!isActive())
      return false;
    // Delegate to operator
    return m_op->calcNative(m_valueCache, m_node);
  }

  void NodeFunction::printValue(std::ostream &s) const
  {
    if (!isActive()) {
      s << "UNKNOWN";
      return;
    }
    m_op->printValue(s, m_valueCache, m_node);
  }

  void NodeFunction::printSubexpressions(std::ostream & /* s */) const
  {
    // TODO
  }

  Value NodeFunction::toValue() const
  {
    return m_op->toValue(m_valueCache, m_node);
  }

  bool NodeFunction::getValue(bool &result) const
  {
    if (!isActive())
      return false;
    return (*m_op)(result, m_node);
  }

  bool NodeFunction::getValue(int32_t &result) const
  {
    if (!isActive())
      return false;
    return (*m_op)(result, m_node);
  }

  bool NodeFunction::getValue(double &result) const
  {
    if (!isActive())
      return false;
    return (*m_op)(result, m_node);
  }

  bool NodeFunction::getValue(std::string &result) const
  {
    if (!isActive())
      return false;
    return (*m_op)(result, m_node);
  }

  bool NodeFunction::getValuePointer(std::string const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<std::string *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<std::string const *>(m_valueCache); // trust me
    return result;
  }

  // Generic Array
  bool NodeFunction::getValuePointer(Array const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<Array *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<Array const *>(m_valueCache); // trust me
    return result;
  }

  // Specific array types
  bool NodeFunction::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<BooleanArray *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<BooleanArray const *>(m_valueCache); // trust me
    return result;
  }

  bool NodeFunction::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<IntegerArray *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<IntegerArray const *>(m_valueCache); // trust me
    return result;
  }

  bool NodeFunction::getValuePointer(RealArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<RealArray *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<RealArray const *>(m_valueCache); // trust me
    return result;
  }

  bool NodeFunction::getValuePointer(StringArray const *&ptr) const
  {
    if (!isActive())
      return false;
    bool result = (*m_op)(*static_cast<StringArray *>(m_valueCache), m_node);
    if (result)
      ptr = static_cast<StringArray const *>(m_valueCache); // trust me
    return result;
  }

} // namespace PLEXIL
