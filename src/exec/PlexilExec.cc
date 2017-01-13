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

#include "PlexilExec.hh"

#include "Assignable.hh"
#include "Assignment.hh"
#include "Debug.hh"
#include "ExecListenerBase.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "Mutex.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "NodeFactory.hh"
#include "PlanError.hh"

#include <algorithm> // for find(), transform
#include <iterator> // for back_insert_iterator
#include <functional>
#include <time.h>

namespace PLEXIL 
{

  // Comparison function for pending queue
  bool PlexilExec::PendingCompare::operator() (Node const &x, Node const &y) const
  {
    return (x.getPriority() < y.getPriority());
  }

  PlexilExec::PlexilExec()
    : ExecConnector(),
      m_candidateQueue(),
      m_stateChangeQueue(),
      m_finishedRootNodes(),
      m_assignmentsToExecute(),
      m_assignmentsToRetract(),
      m_plan(),
      m_transitionsToPublish(),
      m_listener(NULL),
      m_queuePos(0),
      m_finishedRootNodesDeleted(false)
  {
  }

  PlexilExec::~PlexilExec() 
  {
    // Every node on this list is also in m_plan
    m_finishedRootNodes.clear();
    for (std::list<Node *>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
      delete (Node*) (*it);
  }

  /**
   * @brief Get the list of active plans.
   */
  std::list<Node *> const &PlexilExec::getPlans() const
  {
    return m_plan;
  }

  bool PlexilExec::addPlan(Node *root)
  {
    m_plan.push_back(root);
    root->activate();
    debugMsg("PlexilExec:addPlan",
             "Added plan: " << std::endl << root->toString());
    root->notifyChanged(); // redundant?
    return true;
  }

  /**
   * @brief Queries whether all plans are finished.
   * @return true if all finished, false otherwise.
   */

  bool PlexilExec::allPlansFinished() const
  {
    bool result = m_finishedRootNodesDeleted; // return value in the event no plan is active

    for (std::list<Node *>::const_iterator planit = m_plan.begin();
         planit != m_plan.end();
         ++planit)
      {
        Node *root = *planit;
        if (root->getState() == FINISHED_STATE)
          result = true;
        else
          return false; // some node is not finished
      }
    return result;
  }

  void PlexilExec::markRootNodeFinished(Node *node)
  {
    checkError(node,
               "PlexilExec::markRootNodeFinished: node pointer is invalid");
    addFinishedRootNode(node);
  }

  void PlexilExec::deleteFinishedPlans()
  {
    while (!m_finishedRootNodes.empty()) {
      Node *node = m_finishedRootNodes.front();
      m_finishedRootNodes.pop();
      debugMsg("PlexilExec:deleteFinishedPlans",
               " deleting node \"" << node->getNodeId() << "\"");
      m_plan.remove(node);
      delete node;
    }
    m_finishedRootNodesDeleted = true;
  }

  bool PlexilExec::needsStep() const
  {
    return !m_candidateQueue.empty();
  }

  // Local helper function
  // Returns true if no assignment in list references same variable
  static bool checkAssignment(Assignment const *assign,
                              LinkedQueue<Assignment> &list)
  {
    Expression const *base = assign->getDest()->asAssignable()->getBaseVariable();
    return !list.find_if([base](Assignment const *a)
                         {return base == a->getDest()->asAssignable()->getBaseVariable();});
  }

  /**
   * @brief Schedule this assignment for execution.
   */
  void PlexilExec::enqueueAssignment(Assignment *assign)
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
  void PlexilExec::enqueueAssignmentForRetraction(Assignment *assign)
  {
    checkPlanError(checkAssignment(assign, m_assignmentsToRetract),
                   "Multiple retractions scheduled for variable "
                   << assign->getDest()->asAssignable()->getBaseVariable()->getName());
    checkPlanError(checkAssignment(assign, m_assignmentsToExecute),
                   "Assignment and retraction scheduled for same variable "
                   << assign->getDest()->asAssignable()->getBaseVariable()->getName());
    m_assignmentsToRetract.push(assign);
  }

  //
  // The pending queue is ordered by priority (numerically lowest first),
  // then by temporal order of insertion per priority level.
  //
  // A Node is initially inserted on the pending queue when it is eligible to
  // transition to EXECUTING, and it needs to acquire one or more mutexes.
  // It is removed when:
  //  - its conditions have changed and it is no longer eligible to execute;
  //  - it has acquired the mutexes and is transitioning to EXECUTING.
  //
  // At each step, each node in the pending queue is checked.
  // 

  // We know that the node is eligible to transition.
  // Is it a potential participant in a resource conflict?
  
  bool PlexilExec::checkResourceConflicts(Node *node)
  {
    if (node->getState() != WAITING_STATE
        || node->getNextState() != EXECUTING_STATE)
      return false; // not a candidate to acquire mutexes

    std::vector<Mutex *> const *usingVec = node->getUsingMutexes();
    if (!usingVec)
      return false; // no conflicts possible

    // add it to contention consideration
    debugStmt("PlexilExec:checkResourceConflicts",
              {
                std::ostream &s = getDebugOutputStream();
                s << "[PlexilExec:checkResourceConflicts] Node "
                  << node->getNodeId()
                  << " is eligible to execute and depends on mutex(es) ";
                for (Mutex *m : *usingVec)
                  s << m->getName() << ' ';
                s << std::endl;
              });
    debugMsg("PlexilExec:addToContention",
             " Adding node " << node->getNodeId() << " to pending queue.");
    addPendingNode(node);
    return true;
  }

  // Find eligible nodes which can obtain mutexes, and enqueue them
  // Order is lowest (numeric) priority first, then first-in-first-out
  void PlexilExec::resolveResourceConflicts()
  {
    Node *n = m_pendingQueue.front();
    while (n) {
      // Do we need to recalculate node state?
      QueueStatus qs = n->getQueueStatus();
      if (qs == QUEUE_PENDING_CHECK
          || qs == QUEUE_PENDING_TRY_CHECK) {
        qs = n->conditionsChecked();   // clear check-pending "flag" and update status
        bool canTransition = n->getDestState();
        if (n->getNextState() != EXECUTING_STATE) {
          // No longer eligible, remove from queue
          Node *temp = n;
          n = n->next();
          removePendingNode(temp);
            
          debugMsg("PlexilExec:resolveResourceConflicts",
                   " node " << temp->getNodeId()
                   << " is no longer eligible to execute, removing from pending queue");
          if (canTransition) {
            // Can transition, just not to EXECUTING
            addStateChangeNode(temp);
            // Preserve old debug output
            debugMsg("PlexilExec:handleConditionsChanged",
                     "Placing node '" << temp->getNodeId() <<
                     "' on the state change queue in position " << ++m_queuePos);
          }
          continue;
        }
      }

      // Evaluate whether mutex(es) can be acquired
      if (qs == QUEUE_PENDING_TRY) {
        // Check whether mutexes are available
        std::vector<Mutex *> const *mutexes = n->getUsingMutexes();
        bool eligible = true;
        for (Mutex const *m : *mutexes) {
          if (m->getHolder()) {
            // Check that holder isn't node's ancestor
            Node const *ancestor = n;
            Node const *holder = m->getHolder();
            do {
              checkPlanError(holder != ancestor,
                             "Error: Node " << n->getNodeId()
                             << " attempted to acquire mutex " << m->getName()
                             << ", which is held by node's ancestor "
                             << ancestor->getNodeId());
            } while ((ancestor = ancestor->getParent()));

            debugMsg("PlexilExec:resolveResourceConflicts",
                     " node " << n->getNodeId()
                     << " with priority " << n->getPriority()
                     << " cannot execute because mutex " << m->getName()
                     << " is held by node " << holder->getNodeId());
            eligible = false;
            break; // from inner loop
          } // end mutex in use
        } // end inner loop

        if (eligible) {
          // Remove from pending queue and move to state change queue
          Node *temp = n;
          n = n->next();
          removePendingNode(temp);

          // Grab mutexes and enqueue it for execution
          for (Mutex *m : *mutexes)
            m->acquire(temp);
          addStateChangeNode(temp);
          debugMsg("PlexilExec:resolveResourceConflicts",
                   " node '" << temp->getNodeId()
                   << "' with priority " << temp->getPriority()
                   << " queued for state transition");
          debugMsg("PlexilExec:step",
                   " mutex(es) acquired, placing node " << temp->getNodeId()
                   << " on the state change queue in position " << ++m_queuePos);
          continue;
        }
        else {
          // TODO check for deadlock, priority inversion, etc.
          debugMsg("PlexilExec:step",
                   " unable to acquire mutex(es) for node " << n->getNodeId()
                   << ", leaving in pending queue");
          n->setQueueStatus(QUEUE_PENDING);
        }
      }

      // Look at next
      n = n->next();
    } // end while
  }

  void PlexilExec::step(double startTime) 
  {
    //
    // *** BEGIN CRITICAL SECTION ***
    //

    // State change queue had better be empty when we get here!
    checkError(m_stateChangeQueue.empty(),
               "PlexilExec::step: State change queue not empty at entry");

    unsigned int stepCount = 0;
#ifndef NO_DEBUG_MESSAGE_SUPPORT
    // Only used in debugMsg calls
    unsigned int cycleNum = g_interface->getCycleCount();
#endif

    debugMsg("PlexilExec:cycle", "==>Start cycle " << cycleNum);

    // BEGIN QUIESCENCE LOOP
    do {
      // Preserve old format
      debugStmt("PlexilExec:step",
                {
                  getDebugOutputStream() << "[PlexilExec:step]["
                                         << cycleNum << ":" << stepCount << "]";
                  printConditionCheckQueue();
                });

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
          if (!checkResourceConflicts(candidate)) { // may place node on pending queue
            // The node is eligible to transition now
            addStateChangeNode(candidate);
            // Preserve old debug output
            debugMsg("PlexilExec:handleConditionsChanged",
                     "Placing node '" << candidate->getNodeId() <<
                     "' on the state change queue in position " << ++m_queuePos);
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

      // Transition the nodes
      m_transitionsToPublish.reserve(m_stateChangeQueue.size());
      while (!m_stateChangeQueue.empty()) {
        Node *node = getStateChangeNode();
        debugMsg("PlexilExec:step",
                 "[" << cycleNum << ":" << stepCount << ":" << microStepCount <<
                 "] Transitioning node " << node->getNodeId()
                 << " from " << nodeStateName(node->getState())
                 << " to " << nodeStateName(node->getNextState()));
        NodeState oldState = node->getState();
        node->transition(startTime); // may put node on m_candidateQueue or m_finishedRootNodes
        m_transitionsToPublish.push_back(NodeTransition(node, oldState));
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

    debugMsg("PlexilExec:cycle", "==>End cycle " << cycleNum);
    for (std::list<Node *>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
      debugMsg("PlexilExec:printPlan", std::endl << **it);
    }
    //
    // *** END CRITICAL SECTION ***
    //
  }

  void PlexilExec::performAssignments() 
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
  }

  //
  // Internal queue management
  //

  // N.B. A node can be in only one queue at a time.

  void PlexilExec::addCandidateNode(Node *node)
  {
    // Preserve old debug output
    debugMsg("PlexilExec:notifyNodeConditionChanged", " for node " << node->getNodeId());
    node->setQueueStatus(QUEUE_CHECK);
    m_candidateQueue.push(node);
  }

  Node *PlexilExec::getCandidateNode() {
    Node *result = m_candidateQueue.front();
    if (!result)
      return nullptr;
    
    m_candidateQueue.pop();
    result->conditionsChecked(); // mark it as dequeued
    return result;
  }

  void PlexilExec::addPendingNode(Node *node)
  {
    node->setQueueStatus(QUEUE_PENDING_TRY);
    m_pendingQueue.insert(node);
    for (Mutex *m : *node->getUsingMutexes())
      m->addWaitingNode(node);
  }

  // Should only happen in QUEUE_PENDING and QUEUE_PENDING_TRY.
  void PlexilExec::removePendingNode(Node *node)
  {
    m_pendingQueue.remove(node);
    node->setQueueStatus(QUEUE_NONE);
    for (Mutex *m : *node->getUsingMutexes())
      m->removeWaitingNode(node);
  }

  Node *PlexilExec::getStateChangeNode() {
    Node *result = m_stateChangeQueue.front();
    if (!result)
      return nullptr;
    
    QueueStatus was = result->getQueueStatus();
    m_stateChangeQueue.pop();
    result->setQueueStatus(QUEUE_NONE);
    if (was == QUEUE_TRANSITION_CHECK)
      addCandidateNode(result);
    return result;
  }

  void PlexilExec::addStateChangeNode(Node *node) {
    switch (node->getQueueStatus()) {
    case QUEUE_NONE:    // normal case
      node->setQueueStatus(QUEUE_TRANSITION);
      m_stateChangeQueue.push(node);
      return;

    case QUEUE_CHECK:   // shouldn't happen (?)
      assertTrueMsg(ALWAYS_FAIL,
                    "PlexilExec::addStateChangeNode: Node "
                    << node->getNodeId() << " already in check queue");
      return;

    case QUEUE_TRANSITION:        // already in queue, shouldn't get here
    case QUEUE_TRANSITION_CHECK:  // already in queue, shouldn't get here
      assertTrueMsg(ALWAYS_FAIL,
                    "PlexilExec::addStateChangeNode: " << node->getNodeId()
                    << " is already in transition queue");
      return;

    default:                      // illegal or bogus value
      assertTrueMsg(ALWAYS_FAIL,
                    "PlexilExec::addStateChangeNode: Invalid queue status "
                    << node->getQueueStatus()
                    << " for node " << node->getNodeId());
      return;
    }
  }

  Node *PlexilExec::getFinishedRootNode() {
    Node *result = m_finishedRootNodes.front();
    if (!result)
      return NULL;
    
    m_finishedRootNodes.pop();
    result->setQueueStatus(QUEUE_NONE);
    return result;
  }
  
  void PlexilExec::addFinishedRootNode(Node *node) {
    switch (node->getQueueStatus()) {
      
    case QUEUE_CHECK: // seems plausible??
      m_candidateQueue.remove(node);
      // fall thru

    case QUEUE_NONE:
      node->setQueueStatus(QUEUE_DELETE);
      m_finishedRootNodes.push(node);
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

  void PlexilExec::printConditionCheckQueue() const
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
  void PlexilExec::printPendingQueue() const
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

  void PlexilExec::printStateChangeQueue() const
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

}
