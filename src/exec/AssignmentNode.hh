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

#ifndef ASSIGNMENT_NODE_HH
#define ASSIGNMENT_NODE_HH

#include "Node.hh"

namespace PLEXIL
{
  // Forward references
  class Assignment;

  class AssignmentNode : public Node
  {
  public:

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    AssignmentNode(PlexilNode const *node,
                   Node *parent = NULL);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    AssignmentNode(const std::string &type,
                   const std::string &name,
                   NodeState state,
                   Node *parent = NULL);

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~AssignmentNode();

    /**
     * @brief Accessor for the assigned variable.
     */
    Assignable *getAssignmentVariable() const;

    /**
     * @brief Accessor for the priority of a node.  The priority is used to resolve resource conflicts.
     * @return the priority of this node.
     */
    double getPriority() const {return m_priority;}
    
  protected:

    // Specific behaviors for derived classes
    void specializedPostInit(PlexilNode const *node);
    void specializedHandleExecution();
    void specializedDeactivateExecutable();
    void specializedReset();

    // void cleanUpConditions();
    void cleanUpNodeBody();

    bool getDestStateFromExecuting();
    bool getDestStateFromFailing();

    void transitionFromExecuting(NodeState toState);
    void transitionFromFailing(NodeState toState);

    void transitionToExecuting();
    void transitionToFailing();

    // Node state limit
    NodeState nodeStateMax() const { return FAILING_STATE; }

  private:

    void createAssignment(const PlexilAssignmentBody* body);
    void createDummyAssignment(); // unit test variant
    void abort();

    Assignment *m_assignment;
    double m_priority; /*<! The priority of this node. */
  };

}

#endif // ASSIGNMENT_NODE_HH
