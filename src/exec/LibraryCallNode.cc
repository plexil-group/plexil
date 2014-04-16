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
#include "Array.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
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
    : ListNode(nodeProto, exec, parent)
  {
    checkError(nodeProto->nodeType() == NodeType_LibraryNodeCall,
               "Invalid node type \"" << PlexilParser::nodeTypeString(nodeProto->nodeType())
               << "\" for a LibraryCallNode");

    // Create library call node
    debugMsg("Node:node", "Creating library node call.");
    // XML parser should have checked for this
    const PlexilLibNodeCallBody* body = nodeProto->body();
    checkError(body != NULL,
               "Node " << m_nodeId.toString() << " is a library node call but doesn't have a " <<
               "library node call body.");
    createLibraryNode(body); // constructs default end condition
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  LibraryCallNode::LibraryCallNode(const LabelStr& type,
                                   const LabelStr& name, 
                                   const NodeState state,
                                   const ExecConnectorId& exec,
                                   const NodeId& parent)
    : ListNode(type, name, state, exec, parent)
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

    cleanUpConditions();
    cleanUpNodeBody();
    cleanUpVars(); // flush alias vars
  }

  void LibraryCallNode::createLibraryNode(const PlexilLibNodeCallBody* body)
  {
    // get node body
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

      // Assign aliases for the In interface variables
      createAliases(libInterface->in(), aliasesCopy, true);

      // Assign aliases for the InOut interface variables
      createAliases(libInterface->inOut(), aliasesCopy, false);

      // Barf if formal parameter is not known
      assertTrueMsg(aliasesCopy.empty(),
                    "Interface variable \"" << aliasesCopy.begin()->first.toString() 
                    << "\" not found in library node \"" << libNode->nodeId()
                    << "\", called from node '" << getNodeId().toString() << "'");
    }

    // Construct the child
    m_children.push_back(NodeFactory::createNode(body->libNode(), m_exec, m_id));
  }

  // Check aliases against interfaceVars.
  // Remove all that are found from aliases.
  // If a variable exists in interfaceVars but not aliases:
  //  - and it has a default value, generate the variable with the default value;
  //  - and it doesn't have a default value, signal an error.

  void LibraryCallNode::createAliases(const std::vector<PlexilVarRef*>& interfaceVars,
                                      PlexilAliasMap& aliases,
                                      bool isIn)
  {
    // check each variable in the interface to ensure it is
    // referenced in the alias list
    for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
         var != interfaceVars.end();
         ++var) {
      // get var label and matching value in alias list
      LabelStr varLabel((*var)->name());
      PlexilExprId& aliasValue = aliases[varLabel];
      const std::string& varName(varLabel.toString());

      // check that the expression is consistent with the interface variable
      if (aliasValue.isId()) {
        VariableId actualVar;
        if (Id<PlexilVarRef>::convertable(aliasValue)) {
          actualVar = Node::findVariable((const PlexilVarRef*) aliasValue);
          assertTrueMsg(actualVar.isId(),
                        "Node " << m_nodeId.toString()
                        << ": Can't find variable named \"" << aliasValue->name()
                        << "\" for " << (isIn ? "In" : "InOut" )
                        << "alias variable \"" << varName);

          if (isIn) {
            // Construct const wrapper
            if (actualVar->isArray()) {
              actualVar = 
                (new ArrayAliasVariable(varName,
                                        NodeConnector::getId(),
                                        (ExpressionId) actualVar,
                                        false,
                                        isIn))->getId();
            }
            else {
              actualVar = 
                (new AliasVariable(varName,
                                   NodeConnector::getId(),
                                   (ExpressionId) actualVar,
                                   false,
                                   isIn))->getId();
            }
            debugMsg("LibraryCallNode:createAliases",
                     " Node \"" << m_nodeId.toString()
                     << "\": Constructed const alias wrapper for \"" << varName
                     << "\" to variable " << *actualVar);
            m_localVariables.push_back(actualVar);
          }
          else {
            debugMsg("LibraryCallNode:createAliases",
                     " Node \"" << m_nodeId.toString()
                     << "\": Aliasing \"" << varName
                     << "\" to variable " << *actualVar);
          }
        }
        else if (Id<PlexilArrayElement>::convertable(aliasValue)) {
          // Expression is an array reference
          // Construct the expression
          bool wasCreated = false;
          ExpressionId expr =
            ExpressionFactory::createInstance(aliasValue,
                                              NodeConnector::getId(),
                                              wasCreated);

          // Construct a wrapper for it
          actualVar = (new AliasVariable(varName,
                                         NodeConnector::getId(),
                                         expr,
                                         wasCreated,
                                         isIn))->getId();
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId.toString()
                   << "\": Constructed alias wrapper for \"" << varName
                     << "\" to array element " << *expr);
          m_localVariables.push_back(actualVar);
        }
        else {
          // Expression is not a variable or array reference
          // Can't do this for InOut
          assertTrueMsg(isIn,
                        "Node " << m_nodeId.toString()
                        << ": Alias value for InOut interface variable \""
                        << varName
                        << "\" is not a variable or array reference");

          // Construct the expression
          bool wasCreated = false;
          ExpressionId expr =
            ExpressionFactory::createInstance(aliasValue,
                                              NodeConnector::getId(),
                                              wasCreated);

          // Construct a const wrapper for it
          actualVar = 
            (new AliasVariable(varName, NodeConnector::getId(), expr, wasCreated, isIn))->getId();
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId.toString()
                   << "\": Constructed alias wrapper for \"" << varName
                   << "\" to expression " << *expr);
          m_localVariables.push_back(actualVar);
        }

        // Add to alias map
        m_aliasVariables[varLabel] = actualVar;
        
        // remove value for alias copy for later checking
        aliases.erase(varLabel);
      }
    }
  }

  const VariableId& LibraryCallNode::findVariable(const LabelStr& name, bool recursive)
  {
    if (recursive) {
      // Check alias map only
      if (m_aliasVariables.find(name) != m_aliasVariables.end())
        return m_aliasVariables[name];
      else
        return VariableId::noId();
    }
    else {
      return Node::findVariable(name, false);
    }
  }

  // Specific behaviors for derived classes
  void LibraryCallNode::specializedPostInitLate(const PlexilNodeId& node)
  {
    // Get node body
    const PlexilLibNodeCallBody* body = (const PlexilLibNodeCallBody*) node->body();
    check_error(body != NULL);
    //call postInit on the child
    m_children.front()->postInit(body->libNode());
  }

}
