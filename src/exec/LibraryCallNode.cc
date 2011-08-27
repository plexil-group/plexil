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

#include "LibraryCallNode.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExpressionFactory.hh"
#include "NodeFactory.hh"
#include "Variables.hh"

namespace PLEXIL
{

  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  LibraryCallNode::LibraryCallNode(const PlexilNodeId& nodeProto, 
								   const ExecConnectorId& exec, 
								   const NodeId& parent)
	: Node(nodeProto, exec, parent)
  {
	checkError(nodeProto->nodeType() == NodeType_LibraryNodeCall,
			   "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
			   << "\" for a LibraryCallNode");

	// Create library call node
	debugMsg("Node:node", "Creating library node call.");
	// XML parser should have checked for this
	checkError(Id<PlexilLibNodeCallBody>::convertable(nodeProto->body()),
			   "Node " << m_nodeId.toString() << " is a library node call but doesn't have a " <<
			   "library node call body.");
	createLibraryNode(); // constructs default end condition
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  LibraryCallNode::LibraryCallNode(const LabelStr& type, const LabelStr& name, const NodeState state,
								   const bool skip, const bool start, const bool pre,
								   const bool invariant, const bool post, const bool end, const bool repeat,
								   const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
								   const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
								   const bool parentFinished, const bool cmdHdlRcvdCondition,
								   const ExecConnectorId& exec)
	: Node(type, name, state, 
		   skip, start, pre, invariant, post, end, repeat,
		   ancestorInvariant, ancestorEnd, parentExecuting, childrenFinished,
		   commandAbort, parentWaiting, parentFinished, cmdHdlRcvdCondition,
		   exec)
  {
	checkError(type == LIBRARYNODECALL(),
			   "Invalid node type \"" << type.toString() << "\" for a LibraryCallNode");
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  LibraryCallNode::~LibraryCallNode()
  {
	debugMsg("LibraryCallNode:~LibraryCallNode", " destructor for " << m_nodeId.toString());

	// Redundant with base class destructor
	cleanUpConditions();
	cleanUpNodeBody();
	// cleanUpVars(); // base destructor should handle this

  }

  void LibraryCallNode::cleanUpChildConditions()
  {
	debugMsg("LibraryCallNode:cleanUpChildConditions", " for " << m_nodeId.toString());
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();
  }

  void LibraryCallNode::cleanUpNodeBody()
  {
	debugMsg("LibraryCallNode:cleanUpNodeBody", " for " << m_nodeId.toString());
	// Delete child
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }
	m_children.clear();
  }

  void LibraryCallNode::createLibraryNode()
  {
	// get node body
	const PlexilLibNodeCallBody* body = (PlexilLibNodeCallBody*) m_node->body();
	checkError(body != NULL,
			   "Node " << m_nodeId.toString() << ": createLibraryNode: Node has no library node call body");
      
	// get the lib node and its interface
	const PlexilNodeId& libNode = body->libNode();
	const PlexilInterfaceId& libInterface = libNode->interface();
      
	// if there is no interface, there must be no variables
	if (libInterface.isNoId()) {
	  checkError(body->aliases().size() == 0,
				 "Variable aliases in '" << m_nodeId.toString() <<
				 "' do not match interface in '" << 
				 libNode->nodeId() << "'");
	}
      
	// otherwise check variables in interface
	else {
	  // make a copy of the alias map
	  PlexilAliasMap aliasesCopy(body->aliases());

	  // check whether all "In" variables in the interface are referenced in the alias list
	  testLibraryNodeParameters(libNode, libInterface->in(), aliasesCopy);

	  // check whether all "InOut" variables in the interface are referenced in the alias list
	  testLibraryNodeParameters(libNode, libInterface->inOut(), aliasesCopy);

	  // check that every veriable in alias list has been referenced
	  // or has a default value
	  checkError(aliasesCopy.size() == 0, "Unknown variable '"
				 << LabelStr(aliasesCopy.begin()->first).toString() 
				 << "' passed in call to '" << libNode->nodeId() << "' from '"
				 << getNodeId().toString() << "'");
	}

	// link aliases to variables or values
	for (PlexilAliasMap::const_iterator alias = body->aliases().begin();
		 alias != body->aliases().end();
		 ++alias) {
	  LabelStr paramName(alias->first);
         
	  // if this is a variable reference, look it up
	  if (Id<PlexilVarRef>::convertable(alias->second)) {
		const PlexilVarRef* paramVar = alias->second;
            
		// find variable in interface
		const PlexilVarRef* iVar = libInterface->findVar(paramName.toString());
		checkError(iVar != NULL,
				   "Variable '" << paramName.toString()
				   << "' referenced in '" << getNodeId().toString()
				   << "' does not appear in interface of '" 
				   << libNode->nodeId() << "' ");
            
		// check type
		checkError(iVar->type() == paramVar->type(), 
				   "Variable type mismatch between formal parameter '" 
				   << iVar->name() << "' (" << iVar->type() << ") and actual variable '" 
				   << paramVar->name() << "' (" << paramVar->type() << ") "
				   << "' referenced in '" << getNodeId().toString() << "'");

		// find the expression form
		VariableId var = findVariable(alias->second);
		// FIXME: push this check up into XML parser
		checkError(var.isId(), "Unknown variable '" 
				   << alias->second->name()
				   << "' referenced in call to '" << libNode->nodeId() << "' from '"
				   << getNodeId().toString() << "'");
		// FIXME: push this check up into XML parser
		checkError(Id<VariableImpl>::convertable(var)
				   || Id<AliasVariable>::convertable(var),
				   "Expression not a variable '" 
				   << alias->second->name()
				   << "' referenced in call to '" << libNode->nodeId() << "' from '"
				   << getNodeId().toString() << "'");

		if (iVar->type() == ARRAY) {
		  // check for array element type match
		  // iVar should have variable definition
		  const PlexilArrayVarId ivarDef = (const PlexilArrayVarId&) iVar->variable();
		  checkError(ivarDef.isId(), 
					 "Internal error: interface array variable '" << paramName.c_str()
					 << "' is missing its variable definition");
		  const Id<ArrayVariable> arrayVar = (const Id<ArrayVariable>&) var;
		  checkError(arrayVar.isId(),
					 "Internal error: variable '" << alias->second->name()
					 << "' doesn't seem to be an array variable");
		  checkError(arrayVar->getElementType() == ivarDef->elementType(),
					 "Array variable type mismatch between formal parameter '"
					 << iVar->name() << "' (" << ivarDef->elementType() 
					 << ") and actual variable '" << paramVar->name()
					 << "' (" << arrayVar->getElementType()
					 << ") referenced in '" << getNodeId().toString() << "'");
		}

		// check that read only variables appear in the the In interface
		checkError(!var->isConst() || libInterface->findInVar(paramName.toString()),
				   "Constant variable '" << alias->second->name()
				   << "' referenced in '" << getNodeId().toString() 
				   << "' is aliased to '" <<  iVar->name()
				   << "' declared as InOut in '" << libNode->nodeId() << "'");

		// add this variable to node
		debugMsg("Node:createLibraryNode",
				 " aliasing parameter variable " << paramName.c_str() << " to actual variable " << paramVar->name());
		m_variablesByName[paramName] = var;
	  }
         
	  // if this is a value, create a local variable for it
	  else if (Id<PlexilValue>::convertable(alias->second))
		{
		  Id<PlexilValue> value = (Id<PlexilValue>)alias->second;
		  debugMsg("Node:createLibraryNode",
				   " Constructing variable for " << value->name()
				   << " literal with value " << value->value() 
				   << " as library node interface variable " << paramName.c_str());
		  VariableId varId = 
			(VariableId)
			ExpressionFactory::createInstance(value->name(),
											  value->getId(),
											  m_connector);
		  m_variablesByName[paramName] = varId;
		  m_localVariables.push_back(varId);
		}
	  else
		checkError(false, 
				   "Unexpected expression type '" << alias->second->name()
				   << "' in: " << getNodeId().toString());
	}
      
	m_children.push_back(NodeFactory::createNode(body->libNode(), m_exec, m_id));
  }

  // Check aliases against interfaceVars.
  // Remove all that are found from aliases.
  // If a variable exists in interfaceVars but not aliases:
  //  - and it has a default value, generate the variable with the default value;
  //  - and it doesn't have a default value, signal an error.
  // libNode is only used for error message generation.

  void LibraryCallNode::testLibraryNodeParameters(const PlexilNodeId& libNode, 
												  const std::vector<PlexilVarRef*>& interfaceVars,
												  PlexilAliasMap& aliases)
  {
	// check each variable in the interface to ensure it is
	// referenced in the alias list
      
	for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
		 var != interfaceVars.end(); ++var) {
	  // get var label and matching value in alias list
	  LabelStr varLabel((*var)->name());
	  PlexilExprId& aliasValue = aliases[varLabel];

	  // check that variable referenced in alias list         
	  // if it is missing then check whether formal parameter has a default value
	  if (!aliasValue.isId()) {
		if ((*var)->defaultValue().isId()) {
		  // check that the default value is valid
		  checkError(Id<PlexilValue>::convertable((*var)->defaultValue()), 
					 "Expected PlexilValue.");
		  const Id<PlexilValue>& defaultValue = 
			(Id<PlexilValue>&)(*var)->defaultValue();
		  checkError(defaultValue->value() != "UNKNOWN",
					 "Interface variable '" << 
					 varLabel.toString() <<
					 "' in library node '" << libNode->nodeId() << 
					 "' missing in call from '" << getNodeId().toString() <<
					 "' and no default is specified");
          
		  // construct variable with default value
		  debugMsg("Node:testLibraryNodeParameters", 
				   "Constructing defaulted interface variable " << varLabel.c_str());
		  VariableId newVar =
			(VariableId)
			ExpressionFactory::createInstance(PlexilParser::valueTypeString((*var)->type()) + "Value",
											  defaultValue->getId(),
											  m_connector);
		  m_variablesByName[varLabel] = newVar;
		  m_localVariables.push_back(newVar);
		}
		else {
		  // no default value provided
		  checkError(ALWAYS_FAIL,
					 "Interface variable '" << 
					 varLabel.toString() <<
					 "' in library node '" << libNode->nodeId() << 
					 "' missing in call from '" << getNodeId().toString() <<
					 "' and no default is specified");
		}
	  }

	  // remove value for alias copy for later checking
	  aliases.erase(varLabel);
	}
  }

  // Specific behaviors for derived classes
  void LibraryCallNode::specializedPostInit()
  {
    // call postInit on the child
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->postInit();
  }

  void LibraryCallNode::createSpecializedConditions()
  {
	// Construct conditions
	ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
	ExpressionListenerId listener = m_listeners[childrenWaitingOrFinishedIdx];
	cond->addListener(listener);
	m_conditions[childrenWaitingOrFinishedIdx] = cond;
	m_garbageConditions[childrenWaitingOrFinishedIdx] = true;

	ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
	listener = m_listeners[endIdx];
	endCond->addListener(listener);
	m_conditions[endIdx] = endCond;
	m_garbageConditions[endIdx] = true;
  }

  void LibraryCallNode::specializedActivate()
  {
    // Activate all children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->activate();
  }

}
