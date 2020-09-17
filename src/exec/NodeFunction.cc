/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "NodeImpl.hh"
#include "NodeOperator.hh"
#include "Value.hh"

// Uncomment these if NodeFunction return types are extended
// to String or Array
// #include "ArrayImpl.hh"
// #include "PlanError.hh"
// #include "PlexilTypeTraits.hh"

#include <ostream>

namespace PLEXIL
{
  NodeFunction::NodeFunction(NodeOperator const *op, NodeImpl *node)
    : Propagator(),
      m_op(op),
      m_node(node)
  {
  }

  NodeFunction::~NodeFunction()
  {
  }

  const char *NodeFunction::exprName() const
  {
    return m_op->getName().c_str();
  }

  ValueType NodeFunction::valueType() const
  {
    return m_op->valueType();
  }

  bool NodeFunction::isKnown() const
  {
    // Delegate to operator
    return m_op->isKnown(m_node);
  }

  void NodeFunction::printValue(std::ostream &s) const
  {
    m_op->printValue(s, m_node);
  }

  void NodeFunction::printSpecialized(std::ostream & str) const
  {
    str << m_node->getNodeId();
  }

  Value NodeFunction::toValue() const
  {
    return m_op->toValue(m_node);
  }

  void NodeFunction::doSubexprs(ListenableUnaryOperator const &oper)
  {
    m_op->doPropagationSources(m_node, oper);
  }

#define DEFINE_NODE_FUNC_GET_VALUE_METHOD(_rtype) \
  bool NodeFunction::getValue(_rtype &result) const \
  { \
    return (*m_op)(result, m_node); \
  }

  DEFINE_NODE_FUNC_GET_VALUE_METHOD(Boolean)
  // Only Boolean operators implemented to date,
  // uncomment these as necessary
  // DEFINE_NODE_FUNC_GET_VALUE_METHOD(uint16_t)
  // DEFINE_NODE_FUNC_GET_VALUE_METHOD(Integer)
  // DEFINE_NODE_FUNC_GET_VALUE_METHOD(Real)
  // DEFINE_NODE_FUNC_GET_VALUE_METHOD(String)

#undef DEFINE_NODE_FUNC_GET_VALUE_METHOD

// Uncomment this if we ever need String or Array results
// #define DEFINE_NODE_FUNC_GET_VALUE_PTR_METHOD(_rtype) \
//   bool NodeFunction::getValuePointer(_rtype const *&ptr) const \
//   { \
//     reportPlanError("getValuePointer not implemented for type " << #_type \
//                     << " for " << m_op->getName());                     \
//     return false;                               \
//   }

//   // Only Boolean operators implemented to date
//   // DEFINE_NODE_FUNC_GET_VALUE_PTR_METHOD(String)
  
// #undef DEFINE_NODE_FUNC_GET_VALUE_PTR_METHOD

} // namespace PLEXIL
