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

#include "LibraryCallNode.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ExecConnector.hh"

namespace PLEXIL
{

  LibraryCallNode::LibraryCallNode(char const *nodeId, Node *parent)
    : ListNode(nodeId, parent)
  {
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  LibraryCallNode::LibraryCallNode(const std::string& type,
                                   const std::string& name, 
                                   NodeState state,
                                   Node *parent)
    : ListNode(type, name, state, parent)
  {
    checkError(type == LIBRARYNODECALL,
               "Invalid node type \"" << type << "\" for a LibraryCallNode");
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  LibraryCallNode::~LibraryCallNode()
  {
    debugMsg("LibraryCallNode:~LibraryCallNode", " destructor for " << m_nodeId);

    cleanUpConditions();
    cleanUpNodeBody();
    cleanUpVars(); // flush alias vars
  }

  // For plan parser.
  bool LibraryCallNode::addAlias(char const *name, Expression *exp, bool isGarbage)
  {
    if (m_aliasVariables.find(name) != m_aliasVariables.end())
      return false; // alias by same name already exists
    m_aliasVariables[name] = exp;
    if (isGarbage)
      m_localVariables.push_back(exp);
    return true;
  }

  NodeVariableMap *LibraryCallNode::getChildVariableMap()
  {
    return &m_aliasVariables;
  }

}
