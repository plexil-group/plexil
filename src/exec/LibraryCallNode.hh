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

#ifndef LIBRARY_CALL_NODE_HH
#define LIBRARY_CALL_NODE_HH

#include "ListNode.hh"

namespace PLEXIL
{

  class LibraryCallNode : public ListNode
  {
  public:

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    // *** TO BE DELETED ***
    LibraryCallNode(PlexilNode const *node, Node *parent = NULL);

    /**
     * @brief The constructor.
     * @param nodeId The name of this node.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    LibraryCallNode(char const *nodeId, Node *parent = NULL);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    LibraryCallNode(const std::string& type,
                    const std::string& name,
                    NodeState state,
                    Node *parent = NULL);

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~LibraryCallNode();

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     */
    virtual PlexilNodeType getType() const
    {
      return NodeType_LibraryNodeCall;
    }

    virtual Expression *findVariable(const std::string& name, bool recursive = false);

    // For 1st pass of plan parser
    bool addAlias(std::string const &name);

    // For 2nd pass of plan parser
    void setAlias(std::string const &name, Expression *exp);

  protected:

    // Specific behaviors for derived classes
    // *** TO BE DELETED ***
    virtual void specializedPostInitLate(PlexilNode const *node);

  private:

    // *** TO BE DELETED ***
    void createLibraryNode(PlexilLibNodeCallBody const *body);

    // *** TO BE DELETED ***
    void createInAliases(const std::vector<PlexilVarRef*>& interfaceVars,
                         PlexilAliasMap& aliases);

    // *** TO BE DELETED ***
    void createInOutAliases(const std::vector<PlexilVarRef*>& interfaceVars,
                            PlexilAliasMap& aliases);

    VariableMap m_aliasVariables;
  };

}

#endif // LIBRARY_CALL_NODE_HH

