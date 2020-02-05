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

#include "PlexilExec.hh"

#include "Assignable.hh"
#include "Assignment.hh"
#include "Debug.hh"
#include "ExecListenerBase.hh"
#include "ExternalInterface.hh"
#include "LinkedQueue.hh"
#include "Mutex.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "PlanError.hh"

#include "lifecycle-utils.h"

#include <algorithm> // std::find

namespace PLEXIL 
{

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

  class VariableConflictSet final
  {
  private:

    static VariableConflictSet *s_freeList;

    VariableConflictSet *m_next;
    Assignable const *m_variable;
    PriorityQueue<Node, PriorityCompare> m_nodes;

    // Constructor is private; use allocate() instead.
    VariableConflictSet()
      : m_next(nullptr),
        m_variable(nullptr)
    {
    }

    // Not implemented
    VariableConflictSet(VariableConflictSet const &) = delete;
    VariableConflictSet(VariableConflictSet &) = delete;
    VariableConflictSet &operator=(VariableConflictSet const &) = delete;
    VariableConflictSet &operator=(VariableConflictSet &&) = delete;

  public:
    typedef LinkedQueue<Node>::const_iterator const_iterator;
    typedef LinkedQueue<Node>::iterator iterator;

    ~VariableConflictSet() = default;

    Assignable const *getVariable() const
    {
      return m_variable;
    }
      
    void setVariable(Assignable const *a)
    {
      m_variable = a;
    }

    size_t size() const // self-explanatory
    {
      return m_nodes.size();
    }

    bool empty() const  // self-explanatory
    {
      return m_nodes.empty();
    }

    VariableConflictSet *next() const
    {
      return m_next;
    }

    void setNext(VariableConflictSet *nxt)
    {
      m_next = nxt;
    }

    // insert unique in (weakly) sorted order
    void push(Node *node)
    {
      m_nodes.insert(node);
    }

    // access the element with lowest priority which was inserted first
    Node *front()
    {
      return m_nodes.front();
    }
      
    // delete the indicated element (no error if not there)
    void remove(Node *node)
    {
      m_nodes.remove(node);
    }

    // how many have same priority as front element
    size_t front_count() const
    {
      return m_nodes.front_count();
    }

    iterator begin()
    {
      return m_nodes.begin();
    }
    const_iterator begin() const
    {
      return m_nodes.begin();
    }

    iterator end()
    {
      return m_nodes.end();
    }
    const_iterator end() const
    {
      return m_nodes.end();
    }

    // Allocate an instance from the free list, or construct a new one.
    static VariableConflictSet *allocate()
    {
      // One-time initialization
      static bool sl_initialized = false;
      if (!sl_initialized) {
        s_freeList = nullptr;
        plexilAddFinalizer(&cleanupVariableConflictSets);
        sl_initialized = true;
      }
        
      if (!s_freeList)
        return new VariableConflictSet();

      VariableConflictSet *result = s_freeList;
      s_freeList = s_freeList->m_next;
      result->m_next = nullptr;
      return result;
    }

    // Return an instance to the freelist.
    static void release(VariableConflictSet *v)
    {
      v->m_next = s_freeList;
      v->m_variable = nullptr;
      v->m_nodes.clear();
      s_freeList = v;
    }

    static void cleanupVariableConflictSets()
    {
      while (s_freeList) {
        VariableConflictSet *temp = s_freeList;
        s_freeList = s_freeList->next();
        delete temp;
      }
    }

  };

  VariableConflictSet *VariableConflictSet::s_freeList = nullptr;

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
    std::vector<Expression *> m_variablesToRetract; /*<! Set of variables with assignments to be retracted due to node failures */

    std::list<Node *> m_plan; /*<! The root of the plan.*/
    std::vector<NodeTransition> m_transitionsToPublish;
    ExecListenerBase *m_listener;
    unsigned int m_queuePos;
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
        m_variablesToRetract(),
        m_plan(),
        m_listener(nullptr),
        m_queuePos(0),
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
      m_variablesToRetract.clear();

      for (std::list<Node *>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
        delete (Node*) (*it);
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
    virtual std::list<Node *> const &getPlans() const override
    {
      return m_plan;
    }

    virtual bool addPlan(Node *root) override
    {
      m_plan.push_back(root);
      debugMsg("PlexilExec:addPlan",
               "Added plan: \n" << root->toString());
      root->notifyChanged(); // make sure root is considered first
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

      for (Node const *root : m_plan) {
        if (root->getState() == FINISHED_STATE)
          result = true;
        else
          return false; // some node is not finished
      }
      return result;
    }

    virtual void deleteFinishedPlans() override
    {
      while (!m_finishedRootNodes.empty()) {
        Node *node = m_finishedRootNodes.front();
        m_finishedRootNodes.pop();
        debugMsg("PlexilExec:deleteFinishedPlans",
                 " deleting node " << node->getNodeId() << ' ' << node);
        m_plan.remove(node);
        delete node;
      }
      m_finishedRootNodesDeleted = true;
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

      unsigned int stepCount = 0;
#ifndef NO_DEBUG_MESSAGE_SUPPORT
      // Only used in debugMsg calls
      unsigned int cycleNum = g_interface->getCycleCount();
#endif

      debugMsg("PlexilExec:cycle", " ==>Start cycle " << cycleNum);

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
        // Preserve old format
        debugStmt("PlexilExec:step",
                  {
                    getDebugOutputStream() << "[PlexilExec:step]["
                                           << cycleNum << ":" << stepCount << "]";
                    printConditionCheckQueue();
                  });

        // Size of m_candidateQueue is upper bound on queue size
        // Evaluate conditions of nodes reporting a change
        while (!m_candidateQueue.empty()) {
          Node *candidate = getCandidateNode();
          bool canTransition = candidate->getDestState(); // sets node's next state
          // Preserve old debug output
          condDebugMsg(canTransition,
                       "Node:checkConditions",
                       " Can (possibly) transition to "
                       << nodeStateName(candidate->getNextState()));
          if (canTransition) {
            // Preserve old debug output
            debugMsg("PlexilExec:handleConditionsChanged",
                     "Node " << candidate->getNodeId()
                     << " had a relevant condition change.");
            debugMsg("PlexilExec:handleConditionsChanged",
                     "Considering node '" << candidate->getNodeId()
                     << "' for state transition.");
            if (!resourceCheckRequired(candidate)) {
              // The node is eligible to transition now
              addStateChangeNode(candidate);
              // Preserve old debug output
              debugMsg("PlexilExec:handleConditionsChanged",
                       "Placing node '" << candidate->getNodeId() <<
                       "' on the state change queue in position " << ++m_queuePos);
            }
            else {
              // Possibility of conflict - set it aside to evaluate as a batch
              addPendingNode(candidate);
            }
          }
          // else false alarm, wait for next notification
        }

        // See if any on the pending queue are eligible
        condDebugStmt(!m_pendingQueue.empty(),
                      "PlexilExec:step",
                      {
                        getDebugOutputStream() << "[PlexilExec:step]["
                                               << cycleNum << ":" << stepCount << "]";
                        printPendingQueue();
                      });
        resolveResourceConflicts();

        if (m_stateChangeQueue.empty())
          break; // nothing to do, exit quiescence loop

        debugStmt("PlexilExec:step",
                  {
                    getDebugOutputStream() << "[PlexilExec:step]["
                                           << cycleNum << ":" << stepCount << "]";
                    printStateChangeQueue();
                  });

        unsigned int microStepCount = 0;

        // Reserve space for the transitions to be published
        if (m_listener)
          m_transitionsToPublish.reserve(m_stateChangeQueue.size());

        // Transition the nodes
        while (!m_stateChangeQueue.empty()) {
          Node *node = getStateChangeNode();
          debugMsg("PlexilExec:step",
                   "[" << cycleNum << ":" << stepCount << ":" << microStepCount <<
                   "] Transitioning node " << node->getNodeId() << ' ' << node
                   << " from " << nodeStateName(node->getState())
                   << " to " << nodeStateName(node->getNextState()));
          node->transition(startTime); // may put node on m_candidateQueue or m_finishedRootNodes
          if (m_listener)
            m_transitionsToPublish.emplace_back(NodeTransition(node,
                                                               node->getState(),
                                                               node->getNextState()));
          ++microStepCount;
        }

        // TODO: instrument high-water-mark of max nodes transitioned in this step

        // Publish the transitions
        // FIXME: Move call to listener outside of quiescence loop
        if (m_listener)
          m_listener->notifyOfTransitions(m_transitionsToPublish);
        m_transitionsToPublish.clear();

        // done with this batch
        ++stepCount;
      }
      while (m_assignmentsToExecute.empty()
             && m_assignmentsToRetract.empty()
             && g_interface->outboundQueueEmpty()
             && !m_candidateQueue.empty());
      // END QUIESCENCE LOOP
      // Perform side effects
      g_interface->incrementCycleCount();
      performAssignments();
      g_interface->executeOutboundQueue();
      if (m_listener)
        m_listener->stepComplete(cycleNum);

      debugMsg("PlexilExec:cycle", " ==>End cycle " << cycleNum);
      for (Node const *node: m_plan)
        debugMsg("PlexilExec:printPlan", std::endl << *node);

      //
      // *** END CRITICAL SECTION ***
      //
    }

    virtual void addCandidateNode(Node *node) override
    {
      debugMsg("PlexilExec:notifyNodeConditionChanged",
               " for node " << node->getNodeId() << ' ' << node);
      m_candidateQueue.push(node);
    }

    /**
     * @brief Schedule this assignment for execution.
     */
    virtual void enqueueAssignment(Assignment *assign) override
    {
      checkPlanError(checkAssignment(assign, m_assignmentsToExecute),
                     "Multiple assignments scheduled for variable "
                     << assign->getDest()->asAssignable()->getBaseVariable()->getName());
      checkPlanError(checkAssignment(assign, m_assignmentsToRetract),
                     "Assignment and retraction scheduled for same variable "
                     << assign->getDest()->asAssignable()->getBaseVariable()->getName());
      m_assignmentsToExecute.push(assign);
    }

    /**
     * @brief Schedule this assignment for retraction.
     */
    virtual void enqueueAssignmentForRetraction(Assignment *assign) override
    {
      checkPlanError(checkAssignment(assign, m_assignmentsToRetract),
                     "Multiple retractions scheduled for variable "
                     << assign->getDest()->asAssignable()->getBaseVariable()->getName());
      checkPlanError(checkAssignment(assign, m_assignmentsToExecute),
                     "Assignment and retraction scheduled for same variable "
                     << assign->getDest()->asAssignable()->getBaseVariable()->getName());
      m_assignmentsToRetract.push(assign);
    }

    virtual void markRootNodeFinished(Node *node) override
    {
      checkError(node,
                 "PlexilExec::markRootNodeFinished: node pointer is invalid");
      addFinishedRootNode(node);
    }

  private:

    //
    // Implementation details
    //

    // Local helper function
    // Returns true if no assignment in list references same variable
    static bool checkAssignment(Assignment const *assign,
                                LinkedQueue<Assignment> &list)
    {
      Expression const *base = assign->getDest()->asAssignable()->getBaseVariable();
      return !list.find_if([base](Assignment const *a)
                           {return base == a->getDest()->asAssignable()->getBaseVariable();});
    }

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
      if (node->getType() == NodeType_Assignment)
        return true;
      if (node->getUsingMutexes())
        return true;
      return false;
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
        // else still transitioning to EXECUTING, but resources not available
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

    // Reserve the resource(s)
    // If resource(s) busy, leave node on pending queue
    void tryResourceAcquisition(Node *node)
    {
      // Mutexes first
      std::vector<Mutex *> const *uses = node->getUsingMutexes();
      bool success = true;
      if (uses) {
        for (Mutex *m : *uses)
          success = m->acquire(node) && success;
      }

      // Variables next
      if (node->getType() == NodeType_Assignment) {
        Assignable *var = node->getAssignmentVariable()->asAssignable();
        if (success) {
          // Try to reserve the variable
          success = var->reserve(node);
        }
        else {
          // Add it to variable's queue
          var->addWaitingNode(node);
        }
      }

      if (success) {
        // Node can transition now
        removePendingNode(node);
        addStateChangeNode(node);
      }
      else {
        // If we couldn't get all the resources, release the mutexes we got
        // and set pending status
        for (Mutex *m : *uses)
          if (node == m->getHolder()) {
            m->release();
            m->addWaitingNode(node);
          }
        node->setQueueStatus(QUEUE_PENDING);
      }
    }

    void resolveResourceConflicts()
    {
      if (m_pendingQueue.empty())
        return;

      Node *priorityHead = m_pendingQueue.front();
      std::vector<Node *> priorityNodes;
      while (priorityHead) {
        // Gather nodes at same priority 
        priorityNodes.clear();
        int32_t thisPriority = priorityHead->getPriority();
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

        if (priorityNodes.size() > 1) {
          // Must check for variable conflicts
          // TODO

        }

        // Acquire the resources and transition the remaining nodes, if possible
        for (Node *n : priorityNodes)
          tryResourceAcquisition(n);
      }
    }

    //
    // Assignment variable conflict detection and resolution
    //

    // bool checkVariableConflicts(Node *node)
    // {
    //   // Can't have a variable conflict if it's not an Assignment node.
    //   if (node->getType() != NodeType_Assignment)
    //     return false;

    //   NodeState destState = node->getNextState();
    //   // Node can be in contention in either EXECUTING or FAILING 
    //   switch (destState) {

    //   case EXECUTING_STATE:
    //     // add it to contention consideration
    //     debugMsg("PlexilExec:handleConditionsChanged",
    //              " Node " << node->getNodeId() << ' ' << node <<
    //              " is an assignment node that could be executing. Adding it to the resource contention list");
    //     // *** FIXME ***
    //     addToVariableContention(node);
    //     return true;

    //   case FAILING_STATE: // Is already in conflict set, and must be enqueued now
    //     debugMsg("PlexilExec:handleConditionsChanged",
    //              " Node " << node->getNodeId() << ' ' << node <<
    //              " is an assignment node that is failing, and is already in the resource contention list");
    //     m_variablesToRetract.push_back(node->getAssignmentVariable());
    //     return false;

    //     // In addition to the obvious paths from EXECUTING,
    //     // the node could have been in WAITING and eligible for execution
    //     // but deferred to a higher priority node, then failed/exited/skipped
    //     // before it could execute.
    //   case ITERATION_ENDED_STATE:
    //   case FINISHED_STATE:
    //     switch (node->getState()) {
    //     case EXECUTING_STATE:
    //     case FAILING_STATE:
    //     case WAITING_STATE:
    //       debugMsg("PlexilExec:handleConditionsChanged",
    //                " Node " << node->getNodeId() << ' ' << node <<
    //                " is an assignment node that is no longer possibly executing. Removing it from resource contention.");
    //       // *** FIXME ***
    //       removeFromVariableContention(node);
    //       break;

    //     default:
    //       break;
    //     }

    //   default: // Is not in contention now, and not entering it either
    //     break;
    //   } // end switch (destState)
    //   return false;
    // }

    /**
     * @brief Adds a node to consideration for resource contention.  The node must be an assignment node and it must be eligible to transition to EXECUTING.
     * @param node The assignment node.
     */

    // void addToVariableContention(Node *node)
    // {
    //   Expression *exp = node->getAssignmentVariable();
    //   assertTrue_1(exp);
    //   exp = exp->asAssignable()->getBaseVariable();
    //   assertTrue_1(exp);

    //   debugMsg("PlexilExec:addToResourceContention",
    //            "Adding node " << node->getNodeId() << ' ' << node << " to resource contention.");
    //   VariableConflictSet *conflict = ensureConflictSet(exp);
    //   conflict->push(node);
    // }

    /**
     * @brief Removes a node from consideration for resource contention.  This is usually because some condition has changed that makes the node no longer
     * eligible for execution.
     * @param node The assignment node.
     */

    // void removeFromVariableContention(Node *node) 
    // {
    //   Expression *exp = node->getAssignmentVariable();
    //   assertTrue_1(exp);
    //   exp = exp->asAssignable()->getBaseVariable();
    //   assertTrue_1(exp);

    //   // Remove node from the variable's conflict set.
    //   VariableConflictSet *conflictNodes = getConflictSet(exp);
    //   if (!conflictNodes) {
    //     debugMsg("PlexilExec:removeFromResourceContention",
    //              " no conflict set found for variable " << *exp);
    //     return; // not found
    //   }

    //   debugMsg("PlexilExec:removeFromResourceContention",
    //            " removing node " << node->getNodeId() << ' ' << node
    //            << " from contention for variable " << *exp);
    //   conflictNodes->remove(node);

    //   // If deleted node was only one in conflict set,
    //   // remove variable from conflicts list.
    //   if (conflictNodes->empty()) {
    //     debugMsg("PlexilExec:removeFromResourceContention",
    //              " node " << node->getNodeId() << ' ' << node
    //              << " was only node assigning " << *exp << ", removing variable from contention");
    //     if (m_resourceConflicts == conflictNodes)
    //       // First on list, just point past it
    //       m_resourceConflicts = m_resourceConflicts->next();
    //     else {
    //       // Delete from middle or end
    //       VariableConflictSet *prev = m_resourceConflicts;
    //       VariableConflictSet *curr = prev->next();
    //       while (curr) {
    //         if (curr == conflictNodes) {
    //           prev->setNext(curr->next());
    //           break;
    //         }
    //         prev = curr;
    //         curr = curr->next();
    //       }
    //       // didn't find it
    //       assertTrueMsg(curr, "Internal error: Active conflict set not on active list");
    //     }
    //     // give it back
    //     VariableConflictSet::release(conflictNodes);
    //   }
    // }

    /**
     * @brief Resolve conflicts among potentially executing assignment variables.
     */

    // void resolveAllVariableConflicts()
    // {
    //   VariableConflictSet *c = m_resourceConflicts;
    //   while (c) {
    //     resolveVariableConflicts(c);
    //     c = c->next();
    //   }
    // }

    /**
     * @brief Resolve conflicts for this variable.
     * @note Subroutine of resolveAllVariableConflicts() above.
     */
    // void resolveVariableConflicts(VariableConflictSet *conflict)
    // {
    //   Expression const *var = conflict->getVariable();
    //   assertTrue_1(var);
    //   checkError(!conflict->empty(),
    //              "Resource conflict set for " << conflict->getVariable()->toString() << " is empty.");

    //   // Ignore any variables pending retraction
    //   for (std::vector<Expression *>::const_iterator vit = m_variablesToRetract.begin();
    //        vit != m_variablesToRetract.end();
    //        ++vit) {
    //     if ((*vit)->asAssignable()->getBaseVariable() == var->asAssignable()->getBaseVariable()) { // compare base variables for (e.g.) aliases, array refs
    //       debugMsg("PlexilExec:resolveResourceConflicts",
    //                " Ignoring Assignments for variable " << var->getName()
    //                << ", which has a retraction pending");
    //       return;
    //     }
    //   }

    //   // Only look at nodes with the highest priority
    //   Node *nodeToExecute = nullptr;
    //   NodeState destState = NO_NODE_STATE;
    //   size_t count = conflict->front_count(); // # of nodes with best priority
    //   debugMsg("PlexilExec:resolveResourceConflicts",
    //            ' ' << count << " Assignment node(s) with best priority for variable " << var->getName());
    //   if (count == 1) {
    //     // Usual case (we hope) - make it simple
    //     Node *node = conflict->front();
    //     NodeState dest = node->getNextState();
    //     if (dest == NO_NODE_STATE
    //         && node->getState() == WAITING_STATE) { // other cases? EXECUTING_STATE?
    //       // A node was eligible to transition in a previous cycle but is no longer,
    //       // possibly due to a retraction
    //       removeFromVariableContention(node);
    //     }
    //     else {
    //       nodeToExecute = node;
    //       destState = node->getNextState();
    //     }
    //   }
    //   else {
    //     size_t conflictCounter = 0;
    //     VariableConflictSet::iterator conflictIt = conflict->begin(); 
    //     // Look at the destination states of all the nodes with equal priority
    //     for (size_t i = 0; i < count; ++i) {
    //       Node *node = *conflictIt;
    //       NodeState dest = node->getNextState();

    //       if (dest == NO_NODE_STATE
    //           && node->getState() == WAITING_STATE) { // other cases? EXECUTING_STATE?
    //         // A node was eligible to transition in a previous cycle but is no longer,
    //         // possibly due to a retraction
    //         // Remove from conflict set without invalidating conflictIt
    //         bool atBegin = (conflictIt == conflict->begin());
    //         if (!atBegin)
    //           --conflictIt; // back up to previous
    //         removeFromVariableContention(node);
    //         if (atBegin)
    //           conflictIt = conflict->begin();
    //         else
    //           ++conflictIt; // step forward past deleted
    //         continue;
    //       }
    //       else if (dest != EXECUTING_STATE && dest != FAILING_STATE) {
    //         errorMsg("Error: unexpected node " << node->getNodeId() << ' ' << node
    //                  << " state " << nodeStateName(node->getState())
    //                  << " eligible to transition to " << nodeStateName(dest)
    //                  << " in conflict map.");
    //         ++conflictIt;
    //         continue;
    //       }

    //       // Got a live one
    //       ++conflictCounter;
    //       ++conflictIt;

    //       // If more than one node is scheduled for execution, we have a resource contention.
    //       // N.B.: If this message triggers, nodeToExecute has been set in a previous iteration
    //       // *** FIXME: This is a plan error. Find a non-fatal way to handle this conflict!! ***
    //       checkError(conflictCounter < 2,
    //                  "Error: nodes " << node->getNodeId() << ' ' << node << " and "
    //                  << nodeToExecute->getNodeId() << ' ' << nodeToExecute
    //                  << " are in contention over variable "
    //                  << var->toString() << " and have equal priority.");

    //       nodeToExecute = node;
    //       destState = dest;
    //     }
    //   }

    //   if (!nodeToExecute) {
    //     // FIXME - If top priority nodes were removed from conflict list, are there more?
    //     debugMsg("PlexilExec:resolveResourceConflicts",
    //              " No eligible Assignment nodes for " << var->getName());
    //     return;
    //   }

    //   if (destState == EXECUTING_STATE || destState == FAILING_STATE) {
    //     debugMsg("PlexilExec:resolveResourceConflicts",
    //              " Node " << nodeToExecute->getNodeId() << ' ' << nodeToExecute
    //              << " has best priority.");
    //     addStateChangeNode(nodeToExecute);
    //   }
    //   else {
    //     condDebugMsg(nodeToExecute->getState() == EXECUTING_STATE
    //                  || nodeToExecute->getState() == FAILING_STATE,
    //                  "PlexilExec:resolveResourceConflicts",
    //                  " Node for " << var->getName() << " already executing.  Nothing to resolve.");
    //   }
    // }

    //
    // Variable conflict sets
    //

    // VariableConflictSet *getConflictSet(Expression *a)
    // {
    //   VariableConflictSet *result = m_resourceConflicts;
    //   while (result) {
    //     if (result->getVariable() == a)
    //       return result;
    //     result = result->next();
    //   }
    //   return nullptr;
    // }

    // VariableConflictSet *ensureConflictSet(Expression *a)
    // {
    //   VariableConflictSet *result = m_resourceConflicts;
    //   while (result) {
    //     if (result->getVariable() == a) {
    //       debugMsg("PlexilExec:ensureConflictSet",
    //                " returning existing conflict set for " << *a);
    //       return result; // found it
    //     }
    //     result = result->next();
    //   }

    //   // Not found
    //   result = VariableConflictSet::allocate();
    //   result->setNext(m_resourceConflicts);
    //   result->setVariable(a);
    //   m_resourceConflicts = result;
    //   debugMsg("PlexilExec:ensureConflictSet",
    //            " created new conflict set for " << *a);
    //   return result;
    // }

    void performAssignments() 
    {
      debugMsg("PlexilExec:performAssignments",
               " performing " << m_assignmentsToExecute.size() <<  " assignments and "
               << m_assignmentsToRetract.size() << " retractions");
      while (!m_assignmentsToExecute.empty()) {
        Assignment *assn = m_assignmentsToExecute.front();
        m_assignmentsToExecute.pop();
        assn->execute();
      }
      while (!m_assignmentsToRetract.empty()) {
        Assignment *assn = m_assignmentsToRetract.front();
        m_assignmentsToRetract.pop();
        assn->retract();
      }
      m_variablesToRetract.clear();
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
        result->notifyChanged();
      return result;
    }

    void addStateChangeNode(Node *node) {
      switch (node->getQueueStatus()) {
      case QUEUE_NONE:   // normal case
        debugMsg("PlexilExec:addStateChangeNode",
                 " Placing node " << node->getNodeId() << ' ' << node <<
                 " on the state change queue in position " << ++m_queuePos);
        node->setQueueStatus(QUEUE_TRANSITION);
        m_stateChangeQueue.push(node);
        return;

      case QUEUE_CHECK:   // shouldn't happen
        errorMsg("Cannot add node " << node->getNodeId() << ' ' << node
                 << " to transition queue, is still in candidate queue");
        return;

      case QUEUE_TRANSITION:   // already in queue, shouldn't get here
        debugMsg("PlexilExec:addStateChangeNode",
                 " node " << node->getNodeId() << ' ' << node << " is already in transition queue, ignoring");
        return;

      case QUEUE_TRANSITION_CHECK:  // already in queue, shouldn't get here
        debugMsg("PlexilExec:addStateChangeNode",
                 " node " << node->getNodeId() << ' ' << node
                 << " is already in transition queue AND scheduled for check queue, ignoring");
        return;

      case QUEUE_DELETE:            // cannot possibly transition
        errorMsg("Cannot add node " << node->getNodeId() << ' ' << node
                 << " to transition queue, is finished root node pending deletion");
        return;

      default:                      // illegal or bogus value
        assertTrueMsg(ALWAYS_FAIL,
                      "PlexilExec::addStateChangeNode: Invalid queue status "
                      << node->getQueueStatus()
                      << " for node " << node->getNodeId());
        return;
      }
    }

    void addPendingNode(Node *node)
    {
      node->setQueueStatus(QUEUE_PENDING_TRY);
      m_pendingQueue.insert(node);
      for (Mutex *m : *node->getUsingMutexes())
        m->addWaitingNode(node);
    }

    // Should only happen in QUEUE_PENDING and QUEUE_PENDING_TRY.
    void removePendingNode(Node *node)
    {
      m_pendingQueue.remove(node);
      node->setQueueStatus(QUEUE_NONE);
      for (Mutex *m : *node->getUsingMutexes())
        m->removeWaitingNode(node);
      if (node->getType() == NodeType_Assignment)
        node->getAssignmentVariable()->asAssignable()->removeWaitingNode(node);
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
        node->setQueueStatus(QUEUE_DELETE);
        m_finishedRootNodes.push(node);
        return;

      case QUEUE_TRANSITION:
      case QUEUE_TRANSITION_CHECK:
        errorMsg("Root node " << node->getNodeId() << ' ' << node
                 << " is eligible for deletion but is still in state transition queue");
        return;

      case QUEUE_DELETE: // shouldn't happen, but harmless
        return;
      default:
        assertTrueMsg(ALWAYS_FAIL,
                      "Root node " << node->getNodeId()
                      << " is eligible for deletion but is still in pending or state transition queue");
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

    // TODO: add mutex info
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
