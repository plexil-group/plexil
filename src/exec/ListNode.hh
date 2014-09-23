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

#ifndef LIST_NODE_HH
#define LIST_NODE_HH

#include "Node.hh"

namespace PLEXIL
{

  class ListNode : public Node
  {
  public:

    // *** TO BE DELETED ***
    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    ListNode(PlexilNode const *node, Node *parent = NULL);

    /**
     * @brief The constructor.
     * @param nodeId The name of this node.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    ListNode(char const *nodeId, Node *parent = NULL);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    ListNode(const std::string& type,
             const std::string& name,
             NodeState state,
             Node *parent = NULL);

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~ListNode();

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     */
    virtual PlexilNodeType getType() const
    {
      return NodeType_NodeList;
    }

    virtual std::vector<Node *>& getChildren()
    {
      return m_children;
    }

    virtual const std::vector<Node *>& getChildren() const
    {
      return m_children; 
    }

    /**
     * @brief Add a child to this node.
     * @param node The child.
     * @note Intended for use by the plan parser and unit tests.
     */
    void addChild(Node *node)
    {
      m_children.push_back(node);
    }

    /**
     * @brief Sets the state variable to the new state.
     * @param newValue The new node state.
     * @note This method notifies the children of a change in the parent node's state.
     */
    virtual void setState(NodeState newValue, double tym); // FIXME

  protected:

    virtual Node *findChild(const std::string& childName);
    virtual Node const *findChild(const std::string& childName) const;

    // Specific behaviors for derived classes
    virtual void specializedPostInitLate(PlexilNode const *node);
    virtual void createConditionWrappers();
    virtual void specializedActivate();

    virtual void cleanUpConditions();
    virtual void cleanUpChildConditions();
    virtual void cleanUpNodeBody();

    virtual bool getDestStateFromExecuting();
    virtual bool getDestStateFromFailing();
    virtual bool getDestStateFromFinishing();

    virtual void transitionFromExecuting();
    virtual void transitionFromFinishing();
    virtual void transitionFromFailing();

    virtual void transitionToExecuting();
    virtual void transitionToFinishing();
    virtual void transitionToFailing();

    // Shared with derived class LibraryCallNode
    std::vector<Node *> m_children; /*<! Vector of child nodes. */

    // Node state limit, shared with LibraryCallNode
    virtual NodeState nodeStateMax() const { return FINISHING_STATE; }

  private:

    void createChildNodes(PlexilListBody const *body);

  };

}

#endif // LIST_NODE_HH
