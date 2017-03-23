/* Copyright (c) 2006-2017, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_EXEC_HH
#define PLEXIL_EXEC_HH

#include "ExecConnector.hh"
#include "LinkedQueue.hh"
#include "NodeTransition.hh"

#include <list>
#include <map>
#include <queue>

namespace PLEXIL 
{
  // Forward references
  class Expression;
  class ExecListenerBase;

  /**
   * @brief The core PLEXIL executive.
   */
  class PlexilExec final : public ExecConnector
  {
  public:
    /**
     * @brief Default constructor.
     */
    PlexilExec();

    /**
     * @brief Destructor.
     */
    ~PlexilExec();

    //
    // API to application
    //

    /**
     * @brief Prepare the given plan for execution.
     * @param The plan's root node.
     * @return True if succesful, false otherwise.
     */
    virtual bool addPlan(Node *root);

    /**
     * @brief Begins a single "macro step" i.e. the entire quiescence cycle.
     */
    virtual void step(double startTime); // *** FIXME: use real time type ***

    /**
     * @brief Returns true if the Exec needs to be stepped.
     */
    virtual bool needsStep() const;

    /**
     * @brief Set the ExecListener instance.
     */
    virtual void setExecListener(ExecListenerBase *l)
    {
      m_listener = l;
    }

    /**
     * @brief Get the ExecListener instance.
     * @return The ExecListener. May be NULL.
     */
    virtual ExecListenerBase *getExecListener()
    {
      return m_listener;
    }

    /**
     * @brief Queries whether all plans are finished.
     * @return true if all finished, false otherwise.
     */
    virtual bool allPlansFinished() const;

    /**
     * @brief Deletes any finished root nodes.
     */
    virtual void deleteFinishedPlans();

    //
    // API to Node classes
    //

    /**
     * @brief Schedule this assignment for execution.
     */
    virtual void enqueueAssignment(Assignment *assign);

    /**
     * @brief Schedule this assignment for retraction.
     */
    virtual void enqueueAssignmentForRetraction(Assignment *assign);

    /**
     * @brief Mark node as finished and no longer eligible for execution.
     */
    virtual void markRootNodeFinished(Node *node);

    /**
     * @brief Place a node in the candidate queue.
     * @param node The node which is eligible for state change.
     */
    virtual void addCandidateNode(Node *node); // used by Node

    //
    // Used by Launcher
    //

    /**
     * @brief Get the list of active plans.
     */
    virtual std::list<Node *> const &getPlans() const;

  private:

    // Not implemented
    PlexilExec(PlexilExec const &) = delete;
    PlexilExec(PlexilExec &&) = delete;
    PlexilExec &operator=(PlexilExec const &) = delete;
    PlexilExec &operator=(PlexilExec &&) = delete;

    // Comparison function for pending queue
    struct PendingCompare
    {
      bool operator() (Node const &x, Node const &y) const;
    };

    /**
     * @brief Check the node for its role in resource conflicts (mutex or variable)
     * @param node The node that is eligible to transition.
     * @return true if conflict possible, false if not.
     * @note Puts nodes with potential conflict roles into a special queue.
     */
    bool checkResourceConflicts(Node *node);

    /**
     * @brief Resolve conflicts among potentially executing variables.
     */
    void resolveResourceConflicts();

    //
    // Internal queue management
    //

    Node *getCandidateNode();

    void addPendingNode(Node *node);
    void removePendingNode(Node *node);

    void addStateChangeNode(Node *node);
    Node *getStateChangeNode();

    void addFinishedRootNode(Node *node);
    Node *getFinishedRootNode();

    void printConditionCheckQueue() const;
    void printPendingQueue() const;
    void printStateChangeQueue() const;

    /**
     * @brief Batch-perform internal assignments queued up from a quiescence step.
     */
    void performAssignments();

    LinkedQueue<Node> m_candidateQueue;    /*<! Nodes whose conditions have changed and may be eligible to transition. */
    PriorityQueue<Node, PendingCompare> m_pendingQueue; /*<! Nodes waiting to acquire a mutex. */ 
    LinkedQueue<Node> m_stateChangeQueue;  /*<! Nodes awaiting state transition.*/
    LinkedQueue<Node> m_finishedRootNodes; /*<! Root nodes which are no longer eligible to execute. */

    LinkedQueue<Assignment> m_assignmentsToExecute;
    LinkedQueue<Assignment> m_assignmentsToRetract;
    std::list<Node *> m_plan; /*<! The root of the plan.*/
    std::vector<NodeTransition> m_transitionsToPublish;
    ExecListenerBase *m_listener;
    unsigned int m_queuePos;
    bool m_finishedRootNodesDeleted; /*<! True if at least one finished plan has been deleted */
  };

}

#endif
