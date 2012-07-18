/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    ListNode(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent = NodeId::noId());

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    ListNode(const LabelStr& type,
             const LabelStr& name,
             const NodeState state,
             const ExecConnectorId& exec = ExecConnectorId::noId(),
             const NodeId& parent = NodeId::noId());

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~ListNode();

    virtual const std::vector<NodeId>& getChildren() const { return m_children; }

    /**
     * @brief Sets the state variable to the new state.
     * @param newValue The new node state.
     * @note This method notifies the children of a change in the parent node's state.
     */
    virtual void setState(NodeState newValue);

  protected:

    virtual NodeId findChild(const LabelStr& childName) const;

    // Specific behaviors for derived classes
    virtual void specializedPostInitLate(const PlexilNodeId& node);
    virtual void createSpecializedConditions();
    virtual void createConditionWrappers();
    virtual void specializedActivate();

    virtual void cleanUpConditions();
    virtual void cleanUpChildConditions();
    virtual void cleanUpNodeBody();

    virtual NodeState getDestStateFromExecuting();
    virtual NodeState getDestStateFromFailing();
    virtual NodeState getDestStateFromFinishing();

    virtual void transitionFromExecuting(NodeState toState);
    virtual void transitionFromFinishing(NodeState toState);
    virtual void transitionFromFailing(NodeState toState);

    virtual void transitionToExecuting();
    virtual void transitionToFinishing();
    virtual void transitionToFailing();

    // Shared with derived class LibraryCallNode
    std::vector<NodeId> m_children; /*<! Vector of child nodes. */

  private:

    void createChildNodes(const PlexilListBody* body);

  };

}

#endif // LIST_NODE_HH
