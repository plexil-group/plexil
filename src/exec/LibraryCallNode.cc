/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "LibraryCallNode.hh"

#include "Debug.hh"
#include "Error.hh"
#include "NodeVariableMap.hh"

namespace PLEXIL
{

  LibraryCallNode::LibraryCallNode(char const *nodeId, NodeImpl *parent)
    : ListNode(nodeId, parent),
      m_aliasMap(nullptr)
  {
  }

  LibraryCallNode::LibraryCallNode(const std::string& type,
                                   const std::string& name, 
                                   NodeState state,
                                   NodeImpl *parent)
    : ListNode(type, name, state, parent),
      m_aliasMap(nullptr)
  {
    checkError(type == LIBRARYNODECALL,
               "Invalid node type " << type << " for a LibraryCallNode");
  }

  void LibraryCallNode::cleanUpNodeBody()
  {
    if (m_cleanedBody)
      return;

    debugMsg("LibraryCallNode:cleanUpNodeBody", " for " << m_nodeId);

    // Aliases may point to expressions owned by m_localVariables,
    // so delete alias map first.
    delete m_aliasMap.release();

    ListNode::cleanUpNodeBody();
  }

  void LibraryCallNode::allocateAliasMap(size_t n)
  {
    m_aliasMap.reset(new NodeVariableMap(nullptr));
    m_aliasMap->grow(n);
  }

  // For plan parser.
  bool LibraryCallNode::addAlias(char const *name, Expression *exp, bool isGarbage)
  {
    assertTrue_1(m_aliasMap);
    if (m_aliasMap->find(name) != m_aliasMap->end())
      return false; // duplicate
    (*m_aliasMap)[name] = exp;
    if (isGarbage) {
      // Allocate a place to store alias if it doesn't already exist.
      if (!m_localVariables)
        m_localVariables.reset(new std::vector<std::unique_ptr<Expression>>());

      // N.B. Aliases can refer to local variables,
      // so ensure the alias gets cleaned up first by inserting it in the front.
      m_localVariables->insert(m_localVariables->begin(),
                               std::unique_ptr<Expression>(exp)); // std::make_unique() is C++14
    }
    return true;
  }

  // LibraryCall nodes don't allow children to refer to ancestor environment
  NodeVariableMap const *LibraryCallNode::getChildVariableMap() const
  {
    return m_aliasMap.get();
  }

}
