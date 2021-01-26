/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "NodeImpl.hh"

#include "NodeFunction.hh"

namespace PLEXIL
{
  class ListNode : public NodeImpl
  {
  public:

    /**
     * @brief The constructor.
     * @param nodeId The name of this node.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    ListNode(char const *nodeId, NodeImpl *parent = nullptr);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    ListNode(const std::string& type,
             const std::string& name,
             NodeState state,
             NodeImpl *parent = nullptr);

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~ListNode();

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     */
    virtual PlexilNodeType getType() const override
    {
      return NodeType_NodeList;
    }

    virtual std::vector<NodeImplPtr>& getChildren() override
    {
      return m_children;
    }

    virtual const std::vector<NodeImplPtr>& getChildren() const override
    {
      return m_children; 
    }

    /**
     * @brief Add a child to this node.
     * @param node The child.
     * @note Intended for use by the plan parser and unit tests.
     */
    void addChild(NodeImpl *node);

    virtual NodeImpl *findChild(char const *childName) override;
    virtual NodeImpl const *findChild(char const *childName) const override;

    /**
     * @brief Reserve space for the given number of children.
     * @note For use by parsers. An optional optimization.
     */
    void reserveChildren(size_t n);

    // For initialization and parsing.
    virtual NodeVariableMap const *getChildVariableMap() const override;

    //! Sets the state variable to the new state.
    //! @param exec The PlexilExec instance.
    //! @param newValue The new node state.
    //! @param tym Time of the transition.
    //! @note This wrapper method notifies the children of a change in
    //! the parent node's state.
    virtual void setState(PlexilExec *exec, NodeState newValue, double tym) override;

  protected:

    virtual void specializedCreateConditionWrappers() override;
    virtual void specializedActivate() override;

    virtual void cleanUpConditions() override;
    void cleanUpChildConditions(); // only used by ListNode
    virtual void cleanUpNodeBody() override;

    virtual bool getDestStateFromExecuting() override;
    virtual bool getDestStateFromFailing() override;
    virtual bool getDestStateFromFinishing() override;

    virtual void transitionFromExecuting() override;
    virtual void transitionFromFinishing() override;
    virtual void transitionFromFailing() override;

    virtual void transitionToExecuting() override;
    virtual void transitionToFinishing() override;
    virtual void transitionToFailing(PlexilExec *exec) override;

    NodeFunction m_actionCompleteFn;
    NodeFunction m_allFinishedFn;
    // Shared with derived class LibraryCallNode
    std::vector<NodeImplPtr> m_children; /*<! Vector of child nodes. */
  };

}

#endif // LIST_NODE_HH
