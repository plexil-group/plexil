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

#ifndef PLEXIL_NODE_HH
#define PLEXIL_NODE_HH

#include "ExpressionListener.hh"
#include "NodeConnector.hh"
#include "NodeConstants.hh"
#include "PlexilNodeType.hh"

#include <vector>

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL {

  // Forward references
  class Assignable;
  class Mutex;
  class PlexilExec;

  //! \enum QueueStatus
  //! \brief Node status, used by PlexilExec and Node classes for queue management.
  //!
  //! There are four queues:
  //!  - the candidate queue, scheduled for condition evaluation;
  //!  - the pending queue, eligible for execution and waiting on one or more resources
  //!    (either a mutex or an assignment variable);
  //!  - the transition queue, nodes in the process of state transition; and
  //!  - the deletion queue, top level nodes which have run and are no longer active.
  //!
  //! A node can be in at most one queue at any time.  This is a
  //! limitation of LinkedQueue.
  //!
  //! A node in the candidate queue cannot be re-added before its
  //! conditions are evaluated.
  //!
  //! A node in the pending queue can have a condition check
  //! scheduled, or be notified that a mutex it's waiting on has been
  //! released, or both.
  //!
  //! \note This really requires a state transition diagram!
  enum QueueStatus : uint8_t {
    QUEUE_NONE = 0,          //!< Node is not in any queue.
    QUEUE_CHECK,             //!< Node is in the check-conditions queue.
    QUEUE_PENDING_TRY,       //!< Node has just been added to pending queue, or is waiting for a resource which was just released.
    QUEUE_PENDING_TRY_CHECK, //!< Node has just been added to pending queue, or is waiting for a resource which was just released, AND check-conditions has been requested.
    QUEUE_PENDING,           //!< Node is eligible to execute, but is waiting for a resource.
    QUEUE_PENDING_CHECK,     //!< Node is eligible to execute, but is waiting for a resource, AND check-conditions has been requested.
    QUEUE_TRANSITION,        //!< Node is in state transition queue (i.e. is being executed).
    QUEUE_TRANSITION_CHECK,  //!< Node is in state transition queue AND check-conditions requested.
    QUEUE_DELETE,            //!< Node has finished and is no longer eligible to transition.
  };

  //! \class Node

  //! \brief A stateless virtual base class defining the PlexilExec's
  //!        interface for a Node in a PLEXIL plan.
  //! \see NodeImpl
  //! \see PlexilExec
  //! \ingroup Exec-Core
  class Node :
    public NodeConnector,
    public ExpressionListener
  {
  public:

    //
    // Accessors of static node state
    //

    //! \brief Get the type of this node.
    //! \return The PlexilNodeType value.
    virtual PlexilNodeType getType() const = 0;

    //! \brief Get the priority of this node.
    //! \return The priority.
    //! \note Used in resource conflict resolution logic.
    virtual int32_t getPriority() const = 0;

    //! \brief Get the parent of this node.
    //! \return Const pointer to the parent Node, if any.  Will return
    //!         null for root nodes.
    virtual Node const *getParent() const = 0;

    //
    // Accessors of dynamic node state
    //

    //! \brief Get the current state of this node.
    //! \return The current NodeState value.
    virtual NodeState getState() const = 0;

    //! \brief Get the outcome of this node.
    //! \return The NodeOutcome value.
    virtual NodeOutcome getOutcome() const = 0;

    //! \brief Get the failure type of this node.
    //! \return The FailureType value.
    virtual FailureType getFailureType() const = 0;

    //
    // Queue management API
    //

    //! \brief Get the pointer to the next item in the queue.
    //! \return The pointer.
    //! \see LinkedQueue
    virtual Node *next() const = 0;

    //! \brief Get the pointer to the pointer to the next item in the queue.
    //! \return The pointer.
    //! \see LinkedQueue
    virtual Node **nextPtr() = 0;

    //! \brief Get the current queue status of this Node.
    //! \return The status value.
    virtual QueueStatus getQueueStatus() const = 0;

    //! \brief Update the queue status of this Node.
    //! \param newval The new status value.
    virtual void setQueueStatus(QueueStatus newval) = 0;

    //
    // Node state transition API
    //

    //! \brief Make the node and its expressions active, i.e. eligible
    //! for condition checks.
    virtual void activateNode() = 0;

    //! \brief Notify the node that something has changed, to make it
    //!        eligible for condition checking.
    //! \param exec Pointer to the PlexilExec instance.
    //! \note This is an optimization for cases where the change is
    //!       the direct result of action by the PlexilExec.
    //! \see ExpressionListener::notifyChanged
    virtual void notify(PlexilExec *exec) = 0; 

    //! \brief Calculate the next state of this node, were it to transition,
    //!        based on the values of its conditions.
    //! \return True if the new destination state is different from
    //!         the last check, false otherwise.
    virtual bool getDestState() = 0;

    //! \brief Get the previously calculated next state of this node.
    //! \return The cached next state value.
    //! \note Should only be called by PlexilExec::resolveVariableConflicts and unit tests.
    virtual NodeState getNextState() const = 0;

    //! \brief Commit a pending node state transition.
    //! \param exec Pointer to the PlexilExec instance.
    //! \param time The time of the transition.
    virtual void transition(PlexilExec *exec, double time = 0.0) = 0;

    //
    // Resource conflict resolution logic
    //

    //! \brief Does this node need to acquire resources before it can execute?
    //! \return true if resources must be acquired, false otherwise.
    virtual bool acquiresResources() const = 0;

    //! \brief Get an Assignment node's assigned variable.
    //! \return Pointer to an Assignable.  If node is not an
    //!         AssignmentNode, will be null.
    virtual Assignable *getAssignmentVariable() const = 0;

    //! \brief Attempt to reserve the resources needed by the node.
    //!        If the attempt fails, add the node to the resources'
    //!        wait lists.
    //! \return true if reservation was successful, false if not.
    //! \see Reservable
    virtual bool tryResourceAcquisition() = 0;

    //! \brief Remove the node from the pending queues of any
    //!        resources it was trying to acquire.
    //! \see Reservable
    virtual void releaseResourceReservations() = 0;

    //
    // Printed representation
    //

    //! \brief Print a representation this node to an output stream.
    //! \param stream The output stream.
    //! \param indent (Optional) The number of spaces to indent; default is 0.
    virtual void print(std::ostream& stream, const unsigned int indent = 0) const = 0;

    //! \brief Return a string with a printed representation of this node.
    //! \param indent (Optional) The number of spaces to indent; default is 0.
    virtual std::string toString(const unsigned int indent = 0) const = 0;

  };

  //! \brief Print a formatted representation of the node to an output stream.
  //! \param stream The output stream.
  //! \param node Const reference to the Node.
  //! \return The output stream.
  std::ostream& operator<<(std::ostream &stream, Node const &node);

}

#endif
