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

#include "PlexilExec.hh"

#include "Assignable.hh"
#include "Assignment.hh"
#include "CommandImpl.hh"
#include "Debug.hh"
#include "Dispatcher.hh"
#include "Error.hh"
#include "ExecListenerBase.hh"
#include "LinkedQueue.hh"
#include "Mutex.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "ResourceArbiterInterface.hh"
#include "StateCache.hh"
#include "Update.hh"
#include "Variable.hh"

#include <algorithm> // std::remove_if()

namespace PLEXIL 
{

  // Initialization of global variable
  PlexilExec *g_exec = nullptr;

  //
  // Local classes
  //

  // Comparison function for conflict queues
  // FIXME - turn into lambda?
  struct PriorityCompare
  {
    bool operator() (Node const &x, Node const &y) const
    {
      return (x.getPriority() < y.getPriority());
    }
  };


  class PlexilExecImpl final:
    public PlexilExec
  {
  private:

    //
    // Private member variables
    //

    LinkedQueue<Node> m_candidateQueue;    /*<! Nodes whose conditions have changed and may be eligible to transition. */
    LinkedQueue<Node> m_stateChangeQueue;  /*<! Nodes awaiting state transition.*/
    LinkedQueue<Node> m_finishedRootNodes; /*<! Root nodes which are no longer eligible to execute. */
    PriorityQueue<Node, PriorityCompare> m_pendingQueue; /*<! Nodes waiting to acquire a mutex or assign a variable. */ 
    LinkedQueue<Assignment> m_assignmentsToExecute;
    LinkedQueue<Assignment> m_assignmentsToRetract;

    LinkedQueue<CommandImpl> m_commandsToExecute;
    LinkedQueue<CommandImpl> m_commandsToAbort;

    LinkedQueue<Update> m_updatesToExecute;

    std::list<NodePtr> m_plan; /*<! The root of the plan.*/
    std::vector<NodeTransition> m_transitionsToPublish;
    std::unique_ptr<ResourceArbiterInterface> m_arbiter;
    Dispatcher *m_dispatcher;
    ExecListenerBase *m_listener;
    bool m_finishedRootNodesDeleted; /*<! True if at least one finished plan has been deleted */

  public:

    //
    // Public API
    //
    
    PlexilExecImpl()
      : PlexilExec(),
        m_candidateQueue(),
        m_stateChangeQueue(),
        m_finishedRootNodes(),
        m_pendingQueue(),
        m_assignmentsToExecute(),
        m_assignmentsToRetract(),
        m_commandsToExecute(),
        m_commandsToAbort(),
        m_plan(),
        m_arbiter(makeResourceArbiter()),
        m_dispatcher(),
        m_listener(),
        m_finishedRootNodesDeleted(false)
    {}

    virtual ~PlexilExecImpl() 
    {
      m_candidateQueue.clear();
      m_stateChangeQueue.clear();
      m_finishedRootNodes.clear();
      m_pendingQueue.clear();
      m_assignmentsToExecute.clear();
      m_assignmentsToRetract.clear();
      m_commandsToExecute.clear();
      m_commandsToAbort.clear();
    }

    virtual ResourceArbiterInterface *getArbiter() override
    {
      return m_arbiter.get();
    }

    virtual void setDispatcher(Dispatcher *intf) override
    {
      m_dispatcher = intf;
    }

    virtual void setExecListener(ExecListenerBase *l) override
    {
      m_listener = l;
    }

    virtual ExecListenerBase *getExecListener() override
    {
      return m_listener;
    }

    /**
     * @brief Get the list of active plans.
     */
    virtual std::list<NodePtr> const &getPlans() const override
    {
      return m_plan;
    }

    virtual bool addPlan(Node *root) override
    {
      m_plan.emplace_back(NodePtr(root));
      debugMsg("PlexilExec:addPlan",
               "Added plan: \n" << root->toString());
      root->notifyChanged(this); // make sure root is considered first
      root->activateNode();
      return true;
    }

    /**
     * @brief Queries whether all plans are finished.
     * @return true if all finished, false otherwise.
     */

    virtual bool allPlansFinished() const override
    {
      bool result = m_finishedRootNodesDeleted; // return value in the event no plan is active

      debugMsg("PlexilExec:allPlansFinished", ' ' << m_plan.size() << " plans");
      for (NodePtr const &root : m_plan) {
        if (root->getState() != FINISHED_STATE) {
          // Some node is not finished
          debugMsg("PlexilExec:allPlansFinished", " return false");
          return false;
        }
        else
          result = true;
      }
      debugMsg("PlexilExec:allPlansFinished",
               " return " << (result ? "true" : "false"));
      return result;
    }

    virtual void markRootNodeFinished(Node *node) override
    {
      checkError(node,
                 "PlexilExec::markRootNodeFinished: node pointer is invalid");
      addFinishedRootNode(node);
    }

    virtual void deleteFinishedPlans() override
    {
      while (!m_finishedRootNodes.empty()) {
        Node *node = m_finishedRootNodes.front();
        m_finishedRootNodes.pop();
        debugMsg("PlexilExec:deleteFinishedPlans",
                 " deleting node " << node->getNodeId() << ' ' << node);
        m_plan.remove_if([node] (NodePtr const &n) -> bool
                         { return node == n.get(); });
      }
      m_finishedRootNodesDeleted = true;
      debugMsg("PlexilExec:deleteFinishedPlans",
               " now " << m_plan.size() << " root nodes");
    }

    virtual bool needsStep() const override
    {
      return !m_candidateQueue.empty();
    }

    virtual void step(double startTime) override
    {
      //
      // *** BEGIN CRITICAL SECTION ***
      //

      // Queue had better be empty when we get here!
      checkError(m_stateChangeQueue.empty(), "State change queue not empty at entry");

#ifndef NO_DEBUG_MESSAGE_SUPPORT 
      // Only used in debugMsg calls
      unsigned int stepCount = 0;
      unsigned int cycleNum = StateCache::instance().getCycleCount();
#endif

      debugMsg("PlexilExec:step", " ==>Start cycle " << cycleNum);

      // A Node is initially inserted on the pending queue when it is eligible to
      // transition to EXECUTING, and it needs to acquire one or more mutexes.
      // It is removed when:
      //  - its conditions have changed and it is no longer eligible to execute;
      //  - it has acquired the mutexes and is transitioning to EXECUTING.
      //
      // At each step, each node in the pending queue is checked.
      // 

      // BEGIN QUIESCENCE LOOP
      do {
        debugStmt("PlexilExec:step",
                  {
                    getDebugOutputStream() << "[PlexilExec:step]["
                                           << cycleNum << ":" << stepCount << "]";
                    printConditionCheckQueue();
                  });
        condDebugStmt(!m_pendingQueue.empty(),
                      "PlexilExec:step",
                      {
                        getDebugOutputStream() << "[PlexilExec:step]["
                                               << cycleNum << ":" << stepCount << "]";
                        printPendingQueue();
                      });

        // Evaluate conditions of nodes reporting a change
        while (!m_candidateQueue.empty()) {
          Node *candidate = getCandidateNode();
          bool canTransition = candidate->getDestState(); // sets node's next state
          if (canTransition) {
            debugMsg("PlexilExec:step",
                     " Node " << candidate->getNodeId() << ' ' << candidate
                     << " can transition from "
                     << nodeStateName(candidate->getState())
                     << " to " << nodeStateName(candidate->getNextState()));
            if (!resourceCheckRequired(candidate)) {
              // The node is eligible to transition now
              addStateChangeNode(candidate);
            }
            else {
              // Possibility of conflict - set it aside to evaluate as a batch
              addPendingNode(candidate);
            }
          }
        }

        // See if any on the pending queue are eligible
        if (!m_pendingQueue.empty()) {
          debugStmt("PlexilExec:step",
                    {
                      getDebugOutputStream() << "[PlexilExec:step]["
                                             << cycleNum << ":" << stepCount << "]";
                      printPendingQueue();
                    });
          resolveResourceConflicts();
        }

        if (m_stateChangeQueue.empty())
          break; // nothing to do, exit quiescence loop

        debugStmt("PlexilExec:step",
                  {
                    getDebugOutputStream() << "[PlexilExec:step]["
                                           << cycleNum << ":" << stepCount << "]";
                    printStateChangeQueue();
                  });

#ifndef NO_DEBUG_MESSAGE_SUPPORT 
        // Only used in debug messages
        unsigned int microStepCount = 0;
#endif

        // Reserve space for the transitions to be published
        if (m_listener)
          m_transitionsToPublish.reserve(m_stateChangeQueue.size());

        // Transition the nodes
        // Transition may put node on m_candidateQueue or m_finishedRootNodes
        while (!m_stateChangeQueue.empty()) {
          Node *node = getStateChangeNode();
          NodeState oldState = node->getState(); // for listener
          debugMsg("PlexilExec:step",
                   "[" << cycleNum << ":" << stepCount << ":" << microStepCount <<
                   "] Transitioning " << nodeTypeString(node->getType())
                   << " node " << node->getNodeId() << ' ' << node
                   << " from " << nodeStateName(node->getState())
                   << " to " << nodeStateName(node->getNextState()));
          node->transition(this, startTime);
          if (m_listener)
            // After transition, old state is lost, so use cached state
            m_transitionsToPublish.emplace_back(NodeTransition(node,
                                                               oldState,
                                                               node->getState()));
#ifndef NO_DEBUG_MESSAGE_SUPPORT 
          ++microStepCount;
#endif
        }

        // TODO: instrument high-water-mark of max nodes transitioned in this step

        // Publish the transitions
        // FIXME: Move call to listener outside of quiescence loop
        if (m_listener)
          m_listener->notifyOfTransitions(m_transitionsToPublish);
        m_transitionsToPublish.clear();

        // done with this batch
#ifndef NO_DEBUG_MESSAGE_SUPPORT 
        ++stepCount;
#endif
      }
      while (m_assignmentsToExecute.empty()
             && m_assignmentsToRetract.empty()
             && m_commandsToExecute.empty()
             && m_commandsToAbort.empty()
             && !m_candidateQueue.empty());
      // END QUIESCENCE LOOP
      // Perform side effects
      StateCache::instance().incrementCycleCount();
      performAssignments();
      executeOutboundQueue();
      if (m_listener)
        m_listener->stepComplete(cycleNum);

      debugMsg("PlexilExec:step", " ==>End cycle " << cycleNum);
      for (NodePtr const &node: m_plan)
        debugMsg("PlexilExec:printPlan",
                 std::endl << *const_cast<Node const *>(node.get()));

      //
      // *** END CRITICAL SECTION ***
      //
    }

    virtual void addCandidateNode(Node *node) override
    {
      m_candidateQueue.push(node);
    }

    /**
     * @brief Schedule this assignment for execution.
     */
    virtual void enqueueAssignment(Assignment *assign) override
    {
      m_assignmentsToExecute.push(assign);
    }

    /**
     * @brief Schedule this assignment for retraction.
     */
    virtual void enqueueAssignmentForRetraction(Assignment *assign) override
    {
      m_assignmentsToRetract.push(assign);
    }

    /**
     * @brief Schedule this command for execution.
     */
    virtual void enqueueCommand(CommandImpl *cmd) override
    {
      m_commandsToExecute.push(cmd);
    };

    /**
     * @brief Schedule this command to be aborted.
     */
    virtual void enqueueAbortCommand(CommandImpl *cmd) override
    {
      m_commandsToAbort.push(cmd);
    };

    /**
     * @brief Schedule this update for execution.
     */
    virtual void enqueueUpdate(Update *update) override
    {
      m_updatesToExecute.push(update);
    }

  private:

    //
    // Implementation details
    //

    //
    // Resource conflict detection and resolution
    //

    //
    // The pending queue is ordered by priority (numerically lowest first),
    // then by temporal order of insertion per priority level.
    //
    // A Node is initially inserted on the pending queue when it is eligible to
    // transition to EXECUTING, and it needs to acquire one or more resources.
    // It is removed when:
    //  - its conditions have changed and it is no longer eligible to execute;
    //  - it has acquired the mutexes and is transitioning to EXECUTING.
    //
    // At each step, each node in the pending queue is checked.
    // 

    // We know that the node is eligible to transition.
    // Is it a potential participant in a resource conflict?
    // Returns false if no chance of conflict,
    // true if potential for conflict must be evaluated before transition.

    bool resourceCheckRequired(Node *node)
    {
      if (node->getNextState() != EXECUTING_STATE)
        return false;
      return node->acquiresResources();
    }

    //! @brief Check whether a node on the pending queue should attempt to acquire resources.
    //!        Remove from pending queue if no longer eligible to execute.
    //!        Add to the state change queue if should transition to some other state.
    //! @return True if eligible for resource acquisition, false otherwise.
    bool resourceCheckEligible(Node *node)
    {
      switch (node->getQueueStatus()) {

      case QUEUE_PENDING_CHECK:
        // Resource(s) not released, so not eligible,
        // and node may not be eligible to execute any more
        if (!node->getDestState()) {
          // No longer transitioning at all - remove from pending queue
          removePendingNode(node);
        }
        else if (node->getNextState() != EXECUTING_STATE) {
          // Now transitioning to some other state
          // Remove from pending queue and add to state change queue
          removePendingNode(node);
          addStateChangeNode(node);
        }
        // else still eligible to transition to EXECUTING,
        // but resources not available
        node->setQueueStatus(QUEUE_PENDING);
        return false;

      case QUEUE_PENDING_TRY_CHECK:
        // Resource(s) were released,
        // but node may not be eligible to execute any more
        if (!node->getDestState()) {
          // No longer transitioning at all - remove from pending queue
          removePendingNode(node);
          return false;
        }
        else if (node->getNextState() != EXECUTING_STATE) {
          // Transitioning to some other state
          // Remove from pending queue and add to state change queue
          removePendingNode(node);
          addStateChangeNode(node);
          return false;
        }
        // else still transitioning to EXECUTING and some resource released -
        // give it a look
        return true;

      case QUEUE_PENDING_TRY:
        // Resource(s) were released, give it a look
        return true;

      case QUEUE_PENDING:
        // no change, ignore
        return false;

      default:
        checkError(ALWAYS_FAIL,
                   "Node " << node->getNodeId()
                   << " in pending queue with invalid queue status "
                   << node->getQueueStatus());
        return false;
      }
    }      

    // Only called if pending queue not empty
    void resolveResourceConflicts()
    {
      Node *priorityHead = m_pendingQueue.front();
      std::vector<Node *> priorityNodes;
      while (priorityHead) {
        // Gather nodes at same priority 
        int32_t thisPriority = priorityHead->getPriority();

        debugMsg("PlexilExec:step",
                 " processing resource reservations at priority " << thisPriority);

        Node *temp = priorityHead;
        do {
          if (resourceCheckEligible(temp)) 
            // Resource(s) were released, give it a look
            priorityNodes.push_back(temp);
          temp = temp->next();
        } while (temp && temp->getPriority() == thisPriority);

        // temp is at end of queue,
        // or pointing to node with higher (numerical) priority
        priorityHead = temp; // for next iteration

        debugMsg("PlexilExec:step",
                 ' ' << priorityNodes.size() << " nodes eligible to acquire resources");

        // Let each node try to acquire its resources.
        // Transition the ones that succeed.
        for (Node *n : priorityNodes) {
          if (n->tryResourceAcquisition()) {
            // Node can transition now
            debugMsg("PlexilExec:resolveResourceConflicts",
                     ' ' << n->getNodeId() << " succeeded");
            removePendingNode(n);
            addStateChangeNode(n);
          }
          else {
            // Can't get resources, so mark that node has been checked
            n->setQueueStatus(QUEUE_PENDING);
          }
        }

        // Done with this batch
        priorityNodes.clear();
      }
    }

    void performAssignments() 
    {
      condDebugMsg(!m_assignmentsToExecute.empty() || !m_assignmentsToRetract.empty(),
                   "PlexilExec:performAssignments", " performing "
                   << m_assignmentsToExecute.size() <<  " assignments and "
                   << m_assignmentsToRetract.size() << " retractions");
      for (Assignment *assn : m_assignmentsToExecute)
        assn->execute(m_listener);
      m_assignmentsToExecute.clear();
      for (Assignment *assn : m_assignmentsToRetract)
        assn->retract(m_listener);
      m_assignmentsToRetract.clear();
    }

    void executeOutboundQueue()
    {
      assertTrue_2(m_dispatcher,
                   "PlexilExec: attempt to execute without an ExternalInterface!");

      if (m_arbiter) {
        // Arbitrate commands to be executed
        LinkedQueue<CommandImpl> accepted, rejected;
        m_arbiter->arbitrateCommands(m_commandsToExecute, accepted, rejected);
        // Execute the ones which can be executed
        while (CommandImpl *cmd = accepted.front()) {
          accepted.pop();
          m_dispatcher->executeCommand(cmd);
        }
        // ... and reject the rest
        while (CommandImpl *cmd = rejected.front()) {
          rejected.pop();
          debugMsg("Test:testOutput", 
                   "Permission to execute " << cmd->getName()
                   << " has been denied by the resource arbiter."); // legacy message
          m_dispatcher->reportCommandArbitrationFailure(cmd);
        }
      }
      else {
        // Execute them all
        while (CommandImpl *cmd = m_commandsToExecute.front()) {
          m_commandsToExecute.pop();
          m_dispatcher->executeCommand(cmd);
        }
      }
      
      while (CommandImpl *cmd = m_commandsToAbort.front()) {
        m_commandsToAbort.pop();
        m_dispatcher->invokeAbort(cmd);
      }

      while (Update *upd = m_updatesToExecute.front()) {
        m_updatesToExecute.pop();
        m_dispatcher->executeUpdate(upd);
      }
    }

    //
    // Internal queue management
    //

    // N.B. A node can be in only one queue at a time.

    /**
     * @brief Dequeue a node from the candidate queue.
     * @return Pointer to the top node in the queue, or nullptr if queue empty.
     */
    Node *getCandidateNode() {
      Node *result = m_candidateQueue.front();
      if (!result)
        return nullptr;

      m_candidateQueue.pop();
      result->setQueueStatus(QUEUE_NONE);
      return result;
    }

    Node *getStateChangeNode() {
      Node *result = m_stateChangeQueue.front();
      if (!result)
        return nullptr;
    
      QueueStatus was = (QueueStatus) result->getQueueStatus();
      m_stateChangeQueue.pop();
      result->setQueueStatus(QUEUE_NONE);
      if (was == QUEUE_TRANSITION_CHECK)
        result->notifyChanged(this);
      return result;
    }
      
    void addStateChangeNode(Node *node) {
      switch (node->getQueueStatus()) {
      case QUEUE_NONE:   // normal case
        debugMsg("PlexilExec:step",
                 " adding " << node->getNodeId() << ' ' << node <<
                 " to state change queue");
        node->setQueueStatus(QUEUE_TRANSITION);
        m_stateChangeQueue.push(node);
        return;

        // Any other case should be considered an internal error

      case QUEUE_CHECK:   // shouldn't happen
        errorMsg("Cannot add node " << node->getNodeId() << ' ' << node
                 << " to transition queue while in candidate queue");
        return;

      case QUEUE_PENDING:
      case QUEUE_PENDING_CHECK:
      case QUEUE_PENDING_TRY_CHECK:
        errorMsg("PlexilExec::addStateChangeNode: Cannot add node "
                 << node->getNodeId() << ' ' << node
                 << " to transition queue while in pending queue");
        return;

      case QUEUE_TRANSITION:   // already in queue, shouldn't get here
      case QUEUE_TRANSITION_CHECK:  // already in queue, shouldn't get here
        errorMsg("PlexilExec::addStateChangeNode: Node "
                 << node->getNodeId() << ' ' << node
                 << " is already in transition queue");
        return;

      case QUEUE_DELETE:            // cannot possibly transition
        errorMsg("PlexilExec::addStateChangeNode: Node "
                 << node->getNodeId() << ' ' << node
                 << " cannot transition, is a finished root node awaiting deletion");
        return;

      default:                      // illegal or bogus value
        errorMsg("PlexilExec::addStateChangeNode: Invalid queue status "
                 << node->getQueueStatus()
                 << " for node " << node->getNodeId() << ' ' << node);
        return;
      }
    }

    void addPendingNode(Node *node)
    {
      debugMsg("PlexilExec:step",
               " adding " << node->getNodeId() << ' ' << node <<
               " to pending queue");
      node->setQueueStatus(QUEUE_PENDING_TRY);
      m_pendingQueue.insert(node);
    }

    // Should only happen in QUEUE_PENDING and QUEUE_PENDING_TRY.
    void removePendingNode(Node *node)
    {
      m_pendingQueue.remove(node);
      node->setQueueStatus(QUEUE_NONE);
      node->releaseResourceReservations();
    }

    Node *getFinishedRootNode() {
      Node *result = m_finishedRootNodes.front();
      if (!result)
        return nullptr;
    
      m_finishedRootNodes.pop();
      result->setQueueStatus(QUEUE_NONE);
      return result;
    }
  
    void addFinishedRootNode(Node *node) {
      switch (node->getQueueStatus()) {
      
      case QUEUE_CHECK: // seems plausible?
        m_candidateQueue.remove(node);
        // fall thru

      case QUEUE_NONE:
        debugMsg("PlexilExec:step",
                 " Marking " << node->getNodeId() << ' ' << node <<
                 " as a finished root node");
        node->setQueueStatus(QUEUE_DELETE);
        m_finishedRootNodes.push(node);
        return;

      case QUEUE_DELETE: // shouldn't happen, but harmless
        return;

      default:
        errorMsg("Root node " << node->getNodeId() << ' ' << node
                 << " is scheduled for deletion but is still in pending or state transition queue");
        return;
      }
    }

    std::string conditionCheckQueueStr() const {
      std::ostringstream retval;
      Node *node = m_candidateQueue.front();
      while (node) {
        retval << node->getNodeId() << ' ' << node << ' ';
        node = node->next();
      }
      return retval.str();
    }

    std::string stateChangeQueueStr() const {
      std::ostringstream retval;
      Node *node = m_stateChangeQueue.front();
      while (node) {
        retval << node->getNodeId() << ' ' << node << ' ';
        node = node->next();
      }
      return retval.str();
    }

    void printConditionCheckQueue() const
    {
#ifndef NO_DEBUG_MESSAGE_SUPPORT
      std::ostream &s = getDebugOutputStream();
      s << " Check queue: ";
      Node *node = m_candidateQueue.front();
      while (node) {
        s << node->getNodeId() << " ";
        node = node->next();
      }
      s << std::endl;
#endif
    }

    // TODO: add mutex, variable info
    void printPendingQueue() const
    {
#ifndef NO_DEBUG_MESSAGE_SUPPORT
      std::ostream &s = getDebugOutputStream();
      s << " Pending queue: ";
      Node *node = m_pendingQueue.front();
      while (node) {
        s << node->getNodeId() << " ";
        node = node->next();
      }
      s << std::endl;
#endif
    }

    void printStateChangeQueue() const
    {
#ifndef NO_DEBUG_MESSAGE_SUPPORT
      std::ostream &s = getDebugOutputStream();
      s << " State change queue: ";
      Node *node = m_stateChangeQueue.front();
      while (node) {
        s << node->getNodeId() << " ";
        node = node->next();
      }
      s << std::endl;
#endif
    }

  };

  // Public constructor
  PlexilExec *makePlexilExec()
  {
    return new PlexilExecImpl();
  }

}
