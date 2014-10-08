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

#include "NodeFactory.hh"

#include "AssignmentNode.hh"
#include "CommandNode.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LibraryCallNode.hh"
#include "ListNode.hh"
#include "UpdateNode.hh"
#include "lifecycle-utils.h"

namespace PLEXIL
{
  static NodeFactory* s_nodeFactories[NodeType_error];

  static void purgeNodeFactories()
  {
    NodeFactory* tmp;
    for (size_t i = 0; i < NodeType_error; ++i)
      if ((tmp = s_nodeFactories[i]))
        delete tmp;
  }

  static void initializeNodeFactories()
  {
    addFinalizer(&purgeNodeFactories);
    // Ensure entire map is correctly initialized
    s_nodeFactories[NodeType_uninitialized] = NULL;
    s_nodeFactories[NodeType_NodeList] = new ConcreteNodeFactory<ListNode>(NodeType_NodeList);
    s_nodeFactories[NodeType_Command] = new ConcreteNodeFactory<CommandNode>(NodeType_Command);
    s_nodeFactories[NodeType_Assignment] = new ConcreteNodeFactory<AssignmentNode>(NodeType_Assignment);
    s_nodeFactories[NodeType_Update] = new ConcreteNodeFactory<UpdateNode>(NodeType_Update);
    s_nodeFactories[NodeType_Empty] = new ConcreteNodeFactory<Node>(NodeType_Empty);
    s_nodeFactories[NodeType_LibraryNodeCall] = new ConcreteNodeFactory<LibraryCallNode>(NodeType_LibraryNodeCall);
  }

  static NodeFactory* getNodeFactory(PlexilNodeType nodeType)
  {
    static bool s_inited = false;
    if (!s_inited) {
      initializeNodeFactories();
      s_inited = true;
    }
    return s_nodeFactories[nodeType];
  }

  NodeFactory::NodeFactory(PlexilNodeType nodeType)
    : m_nodeType(nodeType)
  {
  }

  NodeFactory::~NodeFactory()
  {
    // Clear our entry in table
    s_nodeFactories[m_nodeType] = NULL;
  }

  /**
   * @brief Primary factory method.
   */
  Node *NodeFactory::createNode(char const *name, 
                                PlexilNodeType nodeType,
                                Node *parent)
  {
    assertTrue_2((nodeType > NodeType_uninitialized)
		 && (nodeType < NodeType_error),
		 "createNode: Invalid node type value");
    NodeFactory* factory = getNodeFactory(nodeType);
    assertTrue_2(factory != NULL, "Internal error: no node factory for valid node type");
    Node *result = factory->create(name, parent);
    // common post process here
    return result;
  }

  /**
   * @brief Alternate factory method.  Used only by Exec test module.
   */
  Node *NodeFactory::createNode(const std::string& type, 
                                const std::string& name, 
                                NodeState state,
                                Node *parent)
  {
    PlexilNodeType nodeType = parseNodeType(type);
    checkError(nodeType > NodeType_uninitialized
               && nodeType < NodeType_error,
               "Invalid node type string " << type);
    NodeFactory* factory = getNodeFactory(nodeType);
    checkError(factory != NULL, 
               "No NodeFactory registered for node type " << type);
    Node *result = factory->create(type, name, state, parent);
    // common post process here
    result->activateInternalVariables();
    return result;
  }

  template<class NODE_TYPE>
  Node *ConcreteNodeFactory<NODE_TYPE>::create(char const *name, 
                                               Node *parent) const
  {
    return new NODE_TYPE(name, parent);
  }

  template<class NODE_TYPE>
  Node *ConcreteNodeFactory<NODE_TYPE>::create(const std::string& type,
                                               const std::string& name, 
                                               NodeState state,
                                               Node *parent) const
  {
    // Shouldn't happen
    checkError(parseNodeType(type) == m_nodeType,
               "Factory for node type " << nodeTypeString(m_nodeType)
               << " invoked on node type " << type);
    return new NODE_TYPE(type, name, state, parent);
  }

}
