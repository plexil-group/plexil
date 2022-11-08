// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ASSIGNMENT_NODE_HH
#define ASSIGNMENT_NODE_HH

#include "NodeImpl.hh"

namespace PLEXIL
{
  // Forward references
  class Assignable;
  class Assignment;

  //! \class AssignmentNode
  //! \brief Implements the PLEXIL Assignment Node.
  //! \see Assignment
  //! \ingroup Exec-Core
  class AssignmentNode final : public NodeImpl
  {
  public:

    //! \brief Primary constructor.
    //! \param nodeId The name of this node, as a pointer to const character string.
    //! \param parent (Optional) Pointer to the parent Node, if any.
    AssignmentNode(char const *nodeId, NodeImpl *parent = nullptr);

    //! \brief Alternate constructor.  Used only by Exec test module.
    //! \param type Const reference to a node type name.
    //! \param name Const reference to the name to give this instance.
    //! \param state The state to assign this node.
    //! \param parent (Optional) The parent of this node; may be null.
    AssignmentNode(const std::string &type,
                   const std::string &name,
                   NodeState state,
                   NodeImpl *parent = nullptr);

    //! \brief Virtual destructor.
    virtual ~AssignmentNode();

    //! \brief Get the type of this node.
    //! \return The type of this node.
    virtual PlexilNodeType getType() const override
    {
      return NodeType_Assignment;
    }

    //! \brief Get the node's assignment variable.
    //! \return Pointer to the variable, as an Assignable.
    virtual Assignable *getAssignmentVariable() const override;

    //! \brief Does this node need to acquire resources before it can execute?
    //! \return true if resources must be acquired, false otherwise.
    virtual bool acquiresResources() const override
    {
      return true; // a variable is a resource which must be acquired
    }

    //! \brief Remove the node from the pending queues of any resources
    //!        it was trying to acquire.
    virtual void releaseResourceReservations() override;

    //! \brief Get a pointer to the Assignment object.
    //! \return Pointer to the Assignment.
    //! \note Only used by the plan parser and its unit tests.
    Assignment *getAssignment()
    {
      return m_assignment.get();
    }

    //! \brief Set the assignment.
    //! \param assn The assignment object.
    //! \note Only used by the plan parser.
    void setAssignment(Assignment *assn);
    
  protected:

    //
    // Specific behaviors for AssignmentNode
    //

    //! \brief Perform the execution operations appropriate to the node type.
    virtual void specializedHandleExecution(PlexilExec *exec) override;

    //! \brief Perform deactivations appropriate to the node type.
    virtual void specializedDeactivateExecutable(PlexilExec *exec) override;

    //! \brief Determine the destination state from EXECUTING.
    //! \return True if the new destination state differs from last check; false otherwise.
    virtual bool getDestStateFromExecuting() override;

    //! \brief Determine the destination state from FAILING.
    //! \return True if the new destination state differs from last check; false otherwise.
    virtual bool getDestStateFromFailing() override;

    //! \brief Transition out of EXECUTING state.
    virtual void transitionFromExecuting(PlexilExec *exec) override;

    //! \brief Transition out of FAILING state.
    virtual void transitionFromFailing(PlexilExec *exec) override;

    //! \brief Transition into EXECUTING state.
    virtual void transitionToExecuting() override;;

    //! \brief Transition into FAILING state.
    virtual void transitionToFailing(PlexilExec *exec) override;

    //! \brief Transition to ITERATION_ENDED state.
    virtual void transitionToIterationEnded() override;

    //! \brief Transition to FINISHED state.
    virtual void transitionToFinished() override;

  private:

    // Copy, move constructor, assignment operators deliberately not implemented.
    AssignmentNode(AssignmentNode const &) = delete;
    AssignmentNode(AssignmentNode &&) = delete;
    AssignmentNode &operator=(AssignmentNode const &) = delete;
    AssignmentNode &operator=(AssignmentNode &&) = delete;

    //! \brief Construct a dummy Assignment.
    //! \note Only called by the unit test variant constructor.
    void initDummyAssignment();

    //! \brief Pointer to the Assignment object.
    std::unique_ptr<Assignment> m_assignment;
  };

}

#endif // ASSIGNMENT_NODE_HH
