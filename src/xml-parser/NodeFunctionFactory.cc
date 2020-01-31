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

#include "NodeFunctionFactory.hh"

#include "Error.hh"
#include "NodeFunction.hh"
#include "NodeImpl.hh"
#include "NodeOperator.hh"
#include "parseNodeReference.hh"
#include "parser-utils.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  NodeFunctionFactory::NodeFunctionFactory(NodeOperator const *op, std::string const &name)
    : ExpressionFactory(name),
      m_op(op)
  {
  }

  NodeFunctionFactory::~NodeFunctionFactory()
  {
  }

  ValueType NodeFunctionFactory::check(char const *nodeId, pugi::xml_node expr) const
  {
    assertTrueMsg(m_op, "NodeFunctionFactory::check: no operator for " << m_name);
    size_t n = std::distance(expr.begin(), expr.end());
    checkParserExceptionWithLocation(n == 1,
                                     expr,
                                     "Node \"" << nodeId
                                     << "\": Wrong number of operands for operator "
                                     << m_op->getName());

    // KLUDGE: We presume there is only one argument, a node reference.
    // Check argument
    checkNodeReference(expr.first_child());
    return m_op->valueType();
  }

  Expression *NodeFunctionFactory::allocate(pugi::xml_node const expr,
                                            NodeConnector *node,
                                            bool &wasCreated,
                                            ValueType returnType) const
  {
    NodeImpl *impl = dynamic_cast<NodeImpl *>(node);
    assertTrueMsg(impl,
                  "NodeFunctionFactory: internal error: node argument is not a NodeImpl");
    NodeImpl *refNode = parseNodeReference(expr.first_child(), impl);
    assertTrueMsg(refNode,
                  expr.name() << ": Internal error: no node matching node reference");
    wasCreated = true;
    return new NodeFunction(m_op, refNode);
  }

}
