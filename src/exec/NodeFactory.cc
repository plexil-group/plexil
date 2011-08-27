/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
#include "UpdateNode.hh"
// #include "Node.hh" // redundant?

namespace PLEXIL
{
  NodeFactory::NodeFactory(PlexilNodeType nodeType)
	: m_nodeType(nodeType)
  {
	checkError(factoryMap()[nodeType] == NULL,
			   "A NodeFactory is already registered for node type " << PlexilParser::nodeTypeString(nodeType));
	factoryMap()[nodeType] = this;
  }

  NodeFactory::~NodeFactory()
  {
	factoryMap()[m_nodeType] = NULL;
  }

  NodeFactory** NodeFactory::factoryMap()
  {
	static NodeFactory* *sl_factories = NULL;
	if (sl_factories == NULL) {
	  sl_factories = new NodeFactory*[NodeType_error];
	  for (size_t i = 0; i < NodeType_error; i++)
		sl_factories[i] = NULL;
	}
	return sl_factories;
  }

  /**
   * @brief Primary factory method.
   */
  NodeId NodeFactory::createNode(const PlexilNodeId& nodeProto, 
								 const ExecConnectorId& exec, 
								 const NodeId& parent)
  {
	checkError(nodeProto.isValid(), "Invalid PlexilNodeId");
	PlexilNodeType nodeType = nodeProto->nodeType();
	checkError(nodeType > NodeType_uninitialized
			   && nodeType < NodeType_error,
			   "Invalid node type value " << nodeType);
	ensureNodeFactoriesRegistered();
	NodeFactory* factory = factoryMap()[nodeType];
	checkError(factory != NULL, 
			   "No NodeFactory registered for node type " << PlexilParser::nodeTypeString(nodeType));
	NodeId result = factory->create(nodeProto, exec, parent);
	// common post process here?
	return result;
  }

  /**
   * @brief Alternate factory method.  Used only by Exec test module.
   */
  NodeId NodeFactory::createNode(const LabelStr& type, 
								 const LabelStr& name, 
								 const NodeState state,
								 const bool skip,
								 const bool start,
								 const bool pre,
								 const bool invariant, 
								 const bool post,
								 const bool end,
								 const bool repeat,
								 const bool ancestorInvariant,
								 const bool ancestorEnd,
								 const bool parentExecuting,
								 const bool childrenFinished,
								 const bool commandAbort,
								 const bool parentWaiting,
								 const bool parentFinished,
								 const bool cmdHdlRcvdCondition,
								 const ExecConnectorId& exec)
  {
	PlexilNodeType nodeType = PlexilParser::parseNodeType(type.toString());
	checkError(nodeType > NodeType_uninitialized
			   && nodeType < NodeType_error,
			   "Invalid node type string " << type.toString());
	ensureNodeFactoriesRegistered();
	NodeFactory* factory = factoryMap()[nodeType];
	checkError(factory != NULL, 
			   "No NodeFactory registered for node type " << type.toString());
	NodeId result = factory->create(type, name, state,
									skip, start, pre,
									invariant, post, end, repeat,
									ancestorInvariant, ancestorEnd,
									parentExecuting, childrenFinished,
									commandAbort, parentWaiting, parentFinished,
									cmdHdlRcvdCondition,
									exec);
	// common post process here?
	return result;
  }

#define REGISTER_NODE_FACTORY(CLASS,NODE_TYPE) { new PLEXIL::ConcreteNodeFactory<CLASS>(NODE_TYPE); }

  void NodeFactory::ensureNodeFactoriesRegistered()
  {
	static bool sl_registered = false;
	if (sl_registered)
	  return;

	// Only one Node class at this time... this will change!
	REGISTER_NODE_FACTORY(Node, NodeType_NodeList);
	REGISTER_NODE_FACTORY(CommandNode, NodeType_Command);
	REGISTER_NODE_FACTORY(AssignmentNode, NodeType_Assignment);
	REGISTER_NODE_FACTORY(UpdateNode, NodeType_Update);
	REGISTER_NODE_FACTORY(Node, NodeType_Request); // soon to go away
	REGISTER_NODE_FACTORY(Node, NodeType_Empty);
	REGISTER_NODE_FACTORY(Node, NodeType_LibraryNodeCall);
	sl_registered = true;
  }

}
