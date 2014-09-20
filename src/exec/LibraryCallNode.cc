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

#include "Alias.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"
#include "NodeFactory.hh"
#include "UserVariable.hh"

namespace PLEXIL
{

  // *** TO BE DELETED ***
  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  LibraryCallNode::LibraryCallNode(PlexilNode const *nodeProto, 
                                   Node *parent)
    : ListNode(nodeProto, parent)
  {
    checkError(nodeProto->nodeType() == NodeType_LibraryNodeCall,
               "Invalid node type \"" << nodeTypeString(nodeProto->nodeType())
               << "\" for a LibraryCallNode");

    // Create library call node
    debugMsg("Node:node", "Creating library node call.");
    // XML parser should have checked for this
    PlexilLibNodeCallBody const *body =
      dynamic_cast<PlexilLibNodeCallBody const *>(nodeProto->body());
    assertTrueMsg(body,
                  "Node " << m_nodeId << " is a library node call but doesn't have a " <<
                  "library node call body.");
    createLibraryNode(body); // constructs default end condition
  }

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

  void LibraryCallNode::createLibraryNode(PlexilLibNodeCallBody const *body)
  {
    // get node body
    checkError(body != NULL,
               "Node " << m_nodeId << ": createLibraryNode: Node has no library node call body");
      
    // get the lib node and its interface
    PlexilNode const *libNode = body->libNode();
    PlexilInterface const *libInterface = libNode->interface();
      
    // if there is no interface, there must be no variables
    if (!libInterface) {
      checkError(body->aliases().size() == 0,
                 "Variable aliases in '" << m_nodeId <<
                 "' do not match interface in '" << 
                 libNode->nodeId() << "'");
    }
      
    // otherwise check variables in interface
    else {
      // make a copy of the alias map
      PlexilAliasMap aliasesCopy(body->aliases());

      // Assign aliases for the In interface variables
      createInAliases(libInterface->in(), aliasesCopy);

      // Assign aliases for the InOut interface variables
      createInOutAliases(libInterface->inOut(), aliasesCopy);

      // Barf if formal parameter is not known
      assertTrueMsg(aliasesCopy.empty(),
                    "Interface variable \"" << aliasesCopy.begin()->first 
                    << "\" not found in library node \"" << libNode->nodeId()
                    << "\", called from node '" << getNodeId() << "'");
    }

    // Construct the child
    m_children.push_back(NodeFactory::createNode(body->libNode(), this));
  }

  void LibraryCallNode::createInAliases(const std::vector<PlexilVarRef*>& interfaceVars,
                                        PlexilAliasMap& aliases)
  {
    // check each variable in the interface to ensure it is
    // referenced in the alias list
    for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
         var != interfaceVars.end();
         ++var) {
      // get var name and matching value in alias list
      const std::string& varName((*var)->varName());
      PlexilExpr const *aliasValue = aliases[varName];

      // check that the expression is consistent with the interface variable
      if (aliasValue) {
        bool wasCreated = false;
        Expression *aliasedExpr;
        PlexilVarRef const *aliasVar =
          dynamic_cast<PlexilVarRef const *>(aliasValue);
        if (aliasVar) {
          aliasedExpr = Node::findVariable(aliasVar);
          // TODO: Skip creating alias if variable already read-only
          assertTrueMsg(aliasedExpr,
                        "Node " << m_nodeId
                        << ": Can't find variable named \"" << aliasVar->varName()
                        << "\" for In alias variable \"" << varName);
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId
                   << "\": Constructing const alias wrapper for \"" << varName
                   << "\" to variable " << *aliasedExpr);
        }
        else {
          // Construct the expression
          aliasedExpr = createExpression(aliasValue,
                                         this,
                                         wasCreated);
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId
                   << "\": Constructing alias wrapper for \"" << varName
                   << "\" to expression " << *aliasedExpr);
        }

        // Construct the alias
        Expression *actualVar =             
          new Alias(this,
                    varName,
                    aliasedExpr,
                    wasCreated);

        m_localVariables.push_back(actualVar);
        m_aliasVariables[varName] = actualVar;
        
        // remove value for alias copy for later checking
        aliases.erase(varName);
      } // aliasValue.isId()
    } // for
  }

  void LibraryCallNode::createInOutAliases(const std::vector<PlexilVarRef*>& interfaceVars,
                                           PlexilAliasMap& aliases)
  {
    // check each variable in the interface to ensure it is
    // referenced in the alias list
    for (std::vector<PlexilVarRef*>::const_iterator var = interfaceVars.begin();
         var != interfaceVars.end();
         ++var) {
      // get var name and matching value in alias list
      const std::string& varName((*var)->varName());
      PlexilExpr const *aliasValue = aliases[varName];

      // check that the expression is consistent with the interface variable
      if (aliasValue) {
        Expression *actualVar = NULL;
        PlexilArrayElement const *arrayRef = NULL;
        PlexilVarRef const *aliasVar = NULL;
        if ((aliasVar = dynamic_cast<PlexilVarRef const *>(aliasValue))) {
          Expression *aliasedExpr = Node::findVariable(aliasVar);
          checkParserException(aliasedExpr,
                               "Node " << m_nodeId
                               << ": Can't find variable named \"" << aliasVar->varName()
                               << "\" for InOut alias variable \"" << varName);
          // Check that alias target is writable
          checkParserException(aliasedExpr->isAssignable(),
                               "Node " << m_nodeId
                               << ": Variable \"" << aliasVar->varName()
                               << "\" is not assignable for InOut alias variable \"" << varName);
          actualVar = aliasedExpr;
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId
                   << "\": Aliasing \"" << varName
                   << "\" to variable " << *actualVar);
        }
        else if ((arrayRef = dynamic_cast<PlexilArrayElement const *>(aliasValue))) {
          // Expression is an array reference
          // Construct the expression (will error if array read-only)
          bool wasCreated = false;
          Assignable *aref = createAssignable(arrayRef,
                                              this,
                                              wasCreated);
          // Construct a wrapper for it
          actualVar = new InOutAlias(this,
                                     varName,
                                     aref,
                                     wasCreated);
          m_localVariables.push_back(actualVar);
          debugMsg("LibraryCallNode:createAliases",
                   " Node \"" << m_nodeId
                   << "\": Constructing InOutAlias wrapper for \"" << varName
                   << "\" to array element " << *aref);
        }
        else {
          // Expression is not a variable or array reference
          // Can't do this for InOut
          checkParserException(ALWAYS_FAIL,
                               "Node " << m_nodeId
                               << ": Alias value for InOut interface variable \""
                               << varName
                               << "\" is not a variable or array reference");
        }

        m_aliasVariables[varName] = actualVar;
        
        // remove value for alias copy for later checking
        aliases.erase(varName);
      } // aliasValue.isId()
    } // for
  }

  Expression *LibraryCallNode::findVariable(const std::string& name, bool recursive)
  {
    if (recursive) {
      // Check alias map only
      if (m_aliasVariables.find(name) != m_aliasVariables.end())
        return m_aliasVariables[name];
      else
        return NULL;
    }
    else {
      return Node::findVariable(name, false);
    }
  }

  // Specific behaviors for derived classes
  void LibraryCallNode::specializedPostInitLate(PlexilNode const *node)
  {
    // Get node body
    PlexilLibNodeCallBody const *body =
      dynamic_cast<PlexilLibNodeCallBody const *>(node->body());
    assertTrue_1(body);
    //call postInit on the child
    m_children.front()->postInit(body->libNode());
  }

}
