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
#include "Error.hh"
#include "LibraryCallNode.hh"
#include "ListNode.hh"
#include "UpdateNode.hh"
#include "lifecycle-utils.h"

namespace PLEXIL
{
  NodeFactory::NodeFactory(PlexilNodeType nodeType)
    : m_nodeType(nodeType)
  {
    checkError(factoryMap()[nodeType] == NULL,
               "A NodeFactory is already registered for node type " << nodeTypeString(nodeType));
    factoryMap()[nodeType] = this;
  }

  NodeFactory::~NodeFactory()
  {
    factoryMap()[m_nodeType] = NULL;
  }

  NodeFactory** NodeFactory::factoryMap()
  {
    static NodeFactory* sl_factories[NodeType_error];
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&purge);
      sl_inited = true;
    }
    return sl_factories;
  }

  void NodeFactory::purge()
  {
    NodeFactory* tmp;
    for (size_t i = 0; i < NodeType_error; ++i)
      if ((tmp = factoryMap()[i])) {
        factoryMap()[i] = NULL;
        delete tmp;
      }
  }

  /**
   * @brief Primary factory method.
   */
  // *** TO BE DELETED ***
  Node *NodeFactory::createNode(PlexilNode const *nodeProto, 
                                Node *parent)
  {
    checkError(nodeProto, "Invalid PlexilNodeId");
    PlexilNodeType nodeType = nodeProto->nodeType();
    checkError(nodeType > NodeType_uninitialized
               && nodeType < NodeType_error,
               "Invalid node type value " << nodeType);
    ensureNodeFactoriesRegistered();
    NodeFactory* factory = factoryMap()[nodeType];
    checkError(factory != NULL, 
               "No NodeFactory registered for node type " << nodeTypeString(nodeType));
    Node *result = factory->create(nodeProto, parent);
    // common post process here
    return result;
  }

  Node *NodeFactory::createNode(char const *name, 
                                PlexilNodeType nodeType,
                                Node *parent)
  {
    assertTrueMsg(nodeType > NodeType_uninitialized
                  && nodeType < NodeType_error,
                  "Invalid node type value " << nodeType);
    ensureNodeFactoriesRegistered();
    NodeFactory* factory = factoryMap()[nodeType];
    checkError(factory != NULL, 
               "No NodeFactory registered for node type " << nodeTypeString(nodeType));
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
    ensureNodeFactoriesRegistered();
    NodeFactory* factory = factoryMap()[nodeType];
    checkError(factory != NULL, 
               "No NodeFactory registered for node type " << type);
    Node *result = factory->create(type, name, state, parent);
    // common post process here
    result->activateInternalVariables();
    return result;
  }

  // *** TO BE DELETED ***
  template<class NODE_TYPE>
  Node *ConcreteNodeFactory<NODE_TYPE>::create(PlexilNode const *nodeProto, 
                                               Node *parent) const
  {
    // Shouldn't happen
    checkError(nodeProto->nodeType() == m_nodeType,
               "Factory for node type " << nodeTypeString(m_nodeType)
               << " invoked on node type "
               << nodeTypeString(nodeProto->nodeType()));
    return new NODE_TYPE(nodeProto, parent);
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

#define REGISTER_NODE_FACTORY(CLASS,NODE_TYPE) { new PLEXIL::ConcreteNodeFactory<CLASS>(NODE_TYPE); }

  void NodeFactory::ensureNodeFactoriesRegistered()
  {
    static bool sl_registered = false;
    if (sl_registered)
      return;

    REGISTER_NODE_FACTORY(ListNode, NodeType_NodeList);
    REGISTER_NODE_FACTORY(CommandNode, NodeType_Command);
    REGISTER_NODE_FACTORY(AssignmentNode, NodeType_Assignment);
    REGISTER_NODE_FACTORY(UpdateNode, NodeType_Update);
    REGISTER_NODE_FACTORY(Node, NodeType_Empty);
    REGISTER_NODE_FACTORY(LibraryCallNode, NodeType_LibraryNodeCall);
    sl_registered = true;
  }

}
