/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NODE_HH
#define PLEXIL_NODE_HH

#include "Expression.hh"
#include "NodeConnector.hh"
#include "NodeConstants.hh"
#include "PlexilNodeType.hh"

// Take care of annoying VxWorks macro
#undef UPDATE

// *** For debug use only ***
// Uncomment this if we don't trust the condition activation/deactivation logic
// #define PARANOID_ABOUT_CONDITION_ACTIVATION 1

namespace PLEXIL {

  // Forward references
  class Mutex;

  //
  // QueueStatus - Used by PlexilExec and Node classes for queue management
  // There are four queues:
  //  - the candidate queue, scheduled for condition evaluation;
  //  - the pending queue, eligible for execution and waiting on one or more mutexes;
  //  - the transition queue, nodes in the process of state transition; and
  //  - the deletion queue, top level nodes which have run and are no longer active.
  //
  // A node can be in at most one queue at any time.
  // A node in the candidate queue cannot be re-added before its conditions are evaluated.
  // A node in the pending queue can have a condition check scheduled, or be notified
  //  that a mutex it's waiting on has been released, or both.
  //
  enum QueueStatus : uint8_t {
    QUEUE_NONE = 0,          // not in any queue
    QUEUE_CHECK,             // in check-conditions queue
    QUEUE_PENDING_TRY,       // just added, or waiting for a mutex which was just released
    QUEUE_PENDING_TRY_CHECK, // just added, or waiting for a mutex which was just released
                             // AND check-conditions requested
    QUEUE_PENDING,           // waiting for a mutex
    QUEUE_PENDING_CHECK,     // waiting for a mutex AND check-conditions requested
    QUEUE_TRANSITION,        // in state transition queue
    QUEUE_TRANSITION_CHECK,  // in state transition queue AND check-conditions requested
    QUEUE_DELETE             // no longer eligible to transition
  };

  /**
   * @brief The interface for a Node in the plan.
   */
  class Node :
    public NodeConnector,
    public ExpressionListener
  {
  public:

    // Default constructor is protected

    virtual ~Node() = default;

    //
    // LinkedQueue API used by PlexilExec
    //

    virtual Node *next() const = 0;
    virtual Node **nextPtr() = 0;

    // Make the node active.
    virtual void activateNode() = 0;

    /**
     * @brief Gets the destination state of this node, were it to transition,
     *        based on the values of its conditions.
     * @return True if the new destination state is different from the last check, false otherwise.
     */
    virtual bool getDestState() = 0;

    /**
     * @brief Gets the previously calculated destination state of this node.
     * @return The destination state.
     * @note Should only be called by PlexilExec::resolveVariableConflicts() and unit tests.
     */
    virtual NodeState getNextState() const = 0;

    /**
     * @brief Commit a pending state transition based on the statuses of various conditions.
     * @param time The time of the transition.
     */
    virtual void transition(double time = 0.0) = 0; // FIXME - need a better representation

    /**
     * @brief Accessor for the priority of a node.  The priority is used to resolve resource conflicts.
     * @return the priority of this node.
     */
    virtual int32_t getPriority() const = 0;

    // ExpressionListener API
    virtual void notifyChanged() = 0;

    /**
     * @brief Gets the current state of this node.
     * @return the current node state as a NodeState (enum) value.
     */
    virtual NodeState getState() const = 0;

    /**
     * @brief Gets the outcome of this node.
     * @return the current outcome as an enum value.
     */
    virtual NodeOutcome getOutcome() const = 0;

    /**
     * @brief Gets the failure type of this node.
     * @return the current failure type as an enum value.
     */
    virtual FailureType getFailureType() const = 0;

    /**
     * @brief Mark the node as eligible for recheck of conditions.
     * @return true if it should be added to candidate queue, false otherwise
     */
    virtual bool scheduleCheckConditions() = 0;

    /**
     * @brief Clear the check-conditions "flag".
     * @return The resulting QueueStatus.
     */
    virtual QueueStatus conditionsChecked() = 0;

    /**
     * @brief Accessor for an assignment node's assigned variable.
     */
    virtual Expression *getAssignmentVariable() const = 0;

    /**
     * @brief Gets the type of this node.
     */
    virtual PlexilNodeType getType() const = 0;

    /**
     * @brief Gets the parent of this node.
     */
    virtual Node const *getParent() const = 0;

    /**
     * @brief Gets the mutexes used by this node.
     * @note May return nullptr.
     */
    virtual std::vector<Mutex *> const *getUsingMutexes() const = 0;

    /**
     * @brief Notify that some mutex on which we're pending is now available.
     * @note Used by Mutex class only.
     */
    virtual void notifyMutexAvailable() = 0;

    //
    // For convenience of PlexilExec queue management
    //

    virtual QueueStatus getQueueStatus() const = 0;
    virtual void setQueueStatus(QueueStatus newval) = 0;

    virtual std::string toString(const unsigned int indent = 0) = 0;
    virtual void print(std::ostream& stream, const unsigned int indent = 0) const = 0;

  protected:

    // Only available to derived classes.
    Node()
      : NodeConnector(),
        ExpressionListener()
    {
    }

  private:
    // Not implemented
    Node(Node const &) = delete;
    Node(Node &&) = delete;
    Node &operator=(Node const &) = delete;
    Node &operator=(Node &&) = delete;
  };

  // See implementation in NodeImpl.cc
  std::ostream& operator<<(std::ostream &stream, Node const &node);

}

#endif
