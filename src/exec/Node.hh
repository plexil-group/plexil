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

#include "plexil-stdint.h" // int32_t, also includes plexil-config.h

#include "ExpressionListener.hh"
#include "NodeConnector.hh"
#include "NodeConstants.hh"
#include "PlexilNodeType.hh"

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL {

  // Forward references
  class Assignable;

  enum QueueStatus {
                    QUEUE_NONE = 0,          // not in any queue
                    QUEUE_CHECK,             // in check-conditions queue
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

    virtual ~Node()
    {
    }

    //
    // LinkedQueue API used by PlexilExec
    //

    virtual Node *next() const = 0;
    virtual Node **nextPtr() = 0;

    // Make the node active.
    virtual void activateNode() = 0;
        
    //
    // NodeConnector API to expressions
    //

    /**
     * @brief Accessor for the NodeId as it was written in the XML.
     * @return This node's node id.
     */
    virtual std::string const &getNodeId() const = 0;

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
     * @note Default method; only assignment nodes care about priority.
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
     * @brief Accessor for an assignment node's assigned variable.
     */
    virtual Assignable *getAssignmentVariable() const = 0;

    /**
     * @brief Gets the type of this node.
     */
    virtual PlexilNodeType getType() const = 0;

    /**
     * @brief Gets the parent of this node.
     */
    virtual Node const *getParent() const = 0;

    //
    // For convenience of PlexilExec queue management
    //

    virtual uint8_t getQueueStatus() const = 0;
    virtual void setQueueStatus(uint8_t newval) = 0;

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
    Node(Node const &);
    Node &operator=(Node const &);
  };

  // See implementation in NodeImpl.cc
  std::ostream& operator<<(std::ostream &stream, Node const &node);

}

#endif
