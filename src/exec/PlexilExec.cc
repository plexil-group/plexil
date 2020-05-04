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
#include "Error.hh"
#include "ExecListenerBase.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "LinkedQueue.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "VariableConflictSet.hh"

namespace PLEXIL 
{

  // Initialization of global variable
  PlexilExec *g_exec = NULL;

  class PlexilExecImpl:
    public PlexilExec
  {
  private:

    // Private member variables

    LinkedQueue<Node> m_candidateQueue;    /*<! Nodes whose conditions have changed and may be eligible to transition. */
    LinkedQueue<Node> m_stateChangeQueue;  /*<! Nodes awaiting state transition.*/
    LinkedQueue<Node> m_finishedRootNodes; /*<! Root nodes which are no longer eligible to execute. */
    LinkedQueue<Assignment> m_assignmentsToExecute;
    LinkedQueue<Assignment> m_assignmentsToRetract;
    std::list<Node *> m_plan; /*<! The root of the plan.*/
    std::vector<Expression *> m_variablesToRetract; /*<! Set of variables with assignments to be retracted due to node failures */
    ExecListenerBase *m_listener;
    VariableConflictSet *m_resourceConflicts; /*<! Linked list of variable assignment contention sets. */
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
        m_assignmentsToExecute(),
        m_assignmentsToRetract(),
        m_plan(),
        m_variablesToRetract(),
        m_listener(NULL),
        m_resourceConflicts(NULL),
        m_queuePos(0),
        m_finishedRootNodesDeleted(false)
    {}

    virtual ~PlexilExecImpl() 
    {
      // Every node on this list is also in m_plan
      m_finishedRootNodes.clear();
      for (std::list<Node *>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
        delete (Node*) (*it);

      while (m_resourceConflicts) {
        VariableConflictSet *temp = m_resourceConflicts;
        m_resourceConflicts = m_resourceConflicts->next();
        delete temp;
      }
    }

    void setExecListener(ExecListenerBase *l)
    {
      m_listener = l;
    }

    ExecListenerBase *getExecListener()
    {
      return m_listener;
    }

    /**
     * @brief Get the list of active plans.
     */
    std::list<Node *> const &getPlans() const
    {
      return m_plan;
    }

    bool addPlan(Node *root)
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

    bool allPlansFinished() const
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

    void markRootNodeFinished(Node *node)
    {
      checkError(node,
                 "PlexilExec::markRootNodeFinished: node pointer is invalid");
      addFinishedRootNode(node);
    }

    void deleteFinishedPlans()
    {
      while (!m_finishedRootNodes.empty()) {
        Node *node = m_finishedRootNodes.front();
        m_finishedRootNodes.pop();
        debugMsg("PlexilExec:deleteFinishedPlans",
                 " deleting node " << node->getNodeId() << ' ' << node);
        // Remove from active plan
        bool found = false;
        for (std::list<Node *>::iterator pit = m_plan.begin();
             pit != m_plan.end();
             ++pit) {
          if (*pit == node) {
            found = true;
            m_plan.erase(pit);
            break;
          }
        }
        assertTrueMsg(found,
                      "PlexilExec::deleteFinishedPlan: Node " << node->getNodeId()
                      << ' ' << node << " not found on active root node list");
        // Now safe to delete
        delete node;
      }
      m_finishedRootNodesDeleted = true;
    }

    bool needsStep() const
    {
      return !m_candidateQueue.empty();
    }

    /**
     * @brief Schedule this assignment for execution.
     */
    void enqueueAssignment(Assignment *assign)
    {
      m_assignmentsToExecute.push(assign);
    }

    /**
     * @brief Schedule this assignment for retraction.
     */
    void enqueueAssignmentForRetraction(Assignment *assign)
    {
      m_assignmentsToRetract.push(assign);
    }

    /**
     * @brief Removes a node from consideration for resource contention.  This is usually because some condition has changed that makes the node no longer
     * eligible for execution.
     * @param node The assignment node.
     */

    void removeFromResourceContention(Node *node) 
    {
      Expression *exp = node->getAssignmentVariable();
      assertTrue_1(exp);
      exp = exp->asAssignable()->getBaseVariable();
      assertTrue_1(exp);

      // Remove node from the variable's conflict set.
      VariableConflictSet *conflictNodes = getConflictSet(exp);
      if (!conflictNodes) {
        debugMsg("PlexilExec:removeFromResourceContention",
                 " no conflict set found for variable " << *exp);
        return; // not found
      }

      debugMsg("PlexilExec:removeFromResourceContention",
               " removing node " << node->getNodeId() << ' ' << node
               << " from contention for variable " << *exp);
      conflictNodes->remove(node);

      // If deleted node was only one in conflict set,
      // remove variable from conflicts list.
      if (conflictNodes->empty()) {
        debugMsg("PlexilExec:removeFromResourceContention",
                 " node " << node->getNodeId() << ' ' << node
                 << " was only node assigning " << *exp << ", removing variable from contention");
        if (m_resourceConflicts == conflictNodes)
          // First on list, just point past it
          m_resourceConflicts = m_resourceConflicts->next();
        else {
          // Delete from middle or end
          VariableConflictSet *prev = m_resourceConflicts;
          VariableConflictSet *curr = prev->next();
          while (curr) {
            if (curr == conflictNodes) {
              prev->setNext(curr->next());
              break;
            }
            prev = curr;
            curr = curr->next();
          }
          // didn't find it
          assertTrueMsg(curr, "Internal error: Active conflict set not on active list");
        }
        // give it back
        VariableConflictSet::release(conflictNodes);
      }
    }

    /**
     * @brief Adds a node to consideration for resource contention.  The node must be an assignment node and it must be eligible to transition to EXECUTING.
     * @param node The assignment node.
     */

    void addToResourceContention(Node *node)
    {
      Expression *exp = node->getAssignmentVariable();
      assertTrue_1(exp);
      exp = exp->asAssignable()->getBaseVariable();
      assertTrue_1(exp);

      debugMsg("PlexilExec:addToResourceContention",
               "Adding node " << node->getNodeId() << ' ' << node << " to resource contention.");
      VariableConflictSet *conflict = ensureConflictSet(exp);
      conflict->push(node);
    }

    void step(double startTime) 
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

      // BEGIN QUIESCENCE LOOP
      do {
        debugMsg("PlexilExec:step",
                 "[" << cycleNum << ":" << stepCount << "] Check queue: "
                 << conditionCheckQueueStr());

        // Size of m_candidateQueue is upper bound on queue size
        // Evaluate conditions of nodes reporting a change
        while (!m_candidateQueue.empty()) {
          Node *candidate = getCandidateNode();
          debugMsg("Node:checkConditions",
                   "Checking condition change for node " << candidate->getNodeId() << ' ' << candidate);
          if (candidate->getDestState()) { // sets node's next state
            debugMsg("Node:checkConditions",
                     "Can (possibly) transition to " << nodeStateName(candidate->getNextState()));
            handleConditionsChanged(candidate);
          }
        }

        // Sort Assignment nodes by priority
        resolveResourceConflicts();

        if (m_stateChangeQueue.empty())
          break; // nothing to do, exit quiescence loop

        debugMsg("PlexilExec:step",
                 "[" << cycleNum << ":" << stepCount << "] State change queue: "
                 << stateChangeQueueStr());

        unsigned int microStepCount = 0;

        // Transition the nodes
        while (!m_stateChangeQueue.empty()) {
          Node *node = getStateChangeNode();
          debugMsg("PlexilExec:step",
                   "[" << cycleNum << ":" << stepCount << ":" << microStepCount <<
                   "] Transitioning node " << node->getNodeId() << ' ' << node
                   << " from " << nodeStateName(node->getState())
                   << " to " << nodeStateName(node->getNextState()));
          NodeState oldState = node->getState();
          node->transition(startTime); // may put node on m_candidateQueue or m_finishedRootNodes
          if (m_listener)
            m_listener->notifyNodeTransition(node, oldState, node->getState());
          ++microStepCount;
        }

        // TODO: instrument high-water-mark of max nodes transitioned in this step

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
      for (std::list<Node *>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
        debugMsg("PlexilExec:printPlan", std::endl << **it);
      }
      //
      // *** END CRITICAL SECTION ***
      //
    }

    void addCandidateNode(Node *node)
    {
      debugMsg("PlexilExec:notifyNodeConditionChanged",
               " for node " << node->getNodeId() << ' ' << node);
      m_candidateQueue.push(node);
    }

  private:

    //
    // Implementation details
    //
    
    /**
     * @brief Prepare for a potential state transition of this eligible node.
     * @param node Pointer to the node.
     */

    void handleConditionsChanged(Node *node)
    {
      debugMsg("PlexilExec:handleConditionsChanged",
               " Considering node " << node->getNodeId() << ' ' << node << " for state transition.");

      NodeState destState = node->getNextState();
      if (node->getType() == NodeType_Assignment) {
        // Node can be in contention in either EXECUTING or FAILING 
        switch (destState) {

        case EXECUTING_STATE: {
          // add it to contention consideration
          debugMsg("PlexilExec:handleConditionsChanged",
                   " Node " << node->getNodeId() << ' ' << node <<
                   " is an assignment node that could be executing. Adding it to the resource contention list");
          addToResourceContention(node);
          return;
        }

        case FAILING_STATE: // Is already in conflict set, and must be enqueued now
          debugMsg("PlexilExec:handleConditionsChanged",
                   " Node " << node->getNodeId() << ' ' << node <<
                   " is an assignment node that is failing, and is already in the resource contention list");
          m_variablesToRetract.push_back(node->getAssignmentVariable());
          break;

          // In addition to the obvious paths from EXECUTING,
          // the node could have been in WAITING and eligible for execution
          // but deferred to a higher priority node, then failed/exited/skipped
          // before it could execute.
        case ITERATION_ENDED_STATE:
        case FINISHED_STATE:
          switch (node->getState()) {
          case EXECUTING_STATE:
          case FAILING_STATE:
          case WAITING_STATE:
            debugMsg("PlexilExec:handleConditionsChanged",
                     " Node " << node->getNodeId() << ' ' << node <<
                     " is an assignment node that is no longer possibly executing. Removing it from resource contention.");
            removeFromResourceContention(node);
            break;

          default:
            break;
          }
          break;

        default: // Is not in contention now, and not entering it either
          break;
        } // end switch (destState)
      }

      addStateChangeNode(node);
    }

    //
    // Variable conflict sets
    //

    VariableConflictSet *getConflictSet(Expression *a)
    {
      VariableConflictSet *result = m_resourceConflicts;
      while (result) {
        if (result->getVariable() == a)
          return result;
        result = result->next();
      }
      return NULL;
    }

    VariableConflictSet *ensureConflictSet(Expression *a)
    {
      VariableConflictSet *result = m_resourceConflicts;
      while (result) {
        if (result->getVariable() == a) {
          debugMsg("PlexilExec:ensureConflictSet",
                   " returning existing conflict set for " << *a);
          return result; // found it
        }
        result = result->next();
      }

      // Not found
      result = VariableConflictSet::allocate();
      result->setNext(m_resourceConflicts);
      result->setVariable(a);
      m_resourceConflicts = result;
      debugMsg("PlexilExec:ensureConflictSet",
               " created new conflict set for " << *a);
      return result;
    }

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

    /**
     * @brief Resolve conflicts among potentially executing assignment variables.
     */

    void resolveResourceConflicts()
    {
      VariableConflictSet *c = m_resourceConflicts;
      while (c) {
        resolveVariableConflicts(c);
        c = c->next();
      }
    }

    /**
     * @brief Resolve conflicts for this variable.
     * @note Subroutine of resolveResourceConflicts() above.
     */
    void resolveVariableConflicts(VariableConflictSet *conflict)
    {
      Expression const *var = conflict->getVariable();
      assertTrue_1(var);
      checkError(!conflict->empty(),
                 "Resource conflict set for " << conflict->getVariable()->toString() << " is empty.");

      // Ignore any variables pending retraction
      for (std::vector<Expression *>::const_iterator vit = m_variablesToRetract.begin();
           vit != m_variablesToRetract.end();
           ++vit) {
        if ((*vit)->asAssignable()->getBaseVariable() == var->asAssignable()->getBaseVariable()) { // compare base variables for (e.g.) aliases, array refs
          debugMsg("PlexilExec:resolveResourceConflicts",
                   " Ignoring Assignments for variable " << var->getName()
                   << ", which has a retraction pending");
          return;
        }
      }

      // Only look at nodes with the highest priority
      Node *nodeToExecute = NULL;
      NodeState destState = NO_NODE_STATE;
      size_t count = conflict->front_count(); // # of nodes with best priority
      debugMsg("PlexilExec:resolveResourceConflicts",
               ' ' << count << " Assignment node(s) with best priority for variable " << var->getName());
      if (count == 1) {
        // Usual case (we hope) - make it simple
        Node *node = conflict->front();
        NodeState dest = node->getNextState();
        if (dest == NO_NODE_STATE
            && node->getState() == WAITING_STATE) { // other cases? EXECUTING_STATE?
          // A node was eligible to transition in a previous cycle but is no longer,
          // possibly due to a retraction
          removeFromResourceContention(node);
        }
        else {
          nodeToExecute = node;
          destState = node->getNextState();
        }
      }
      else {
        size_t conflictCounter = 0;
        VariableConflictSet::iterator conflictIt = conflict->begin(); 
        // Look at the destination states of all the nodes with equal priority
        for (size_t i = 0; i < count; ++i) {
          Node *node = *conflictIt;
          NodeState dest = node->getNextState();

          if (dest == NO_NODE_STATE
              && node->getState() == WAITING_STATE) { // other cases? EXECUTING_STATE?
            // A node was eligible to transition in a previous cycle but is no longer,
            // possibly due to a retraction
            // Remove from conflict set without invalidating conflictIt
            bool atBegin = (conflictIt == conflict->begin());
            if (!atBegin)
              --conflictIt; // back up to previous
            removeFromResourceContention(node);
            if (atBegin)
              conflictIt = conflict->begin();
            else
              ++conflictIt; // step forward past deleted
            continue;
          }
          else if (dest != EXECUTING_STATE && dest != FAILING_STATE) {
            errorMsg("Error: unexpected node " << node->getNodeId() << ' ' << node
                     << " state " << nodeStateName(node->getState())
                     << " eligible to transition to " << nodeStateName(dest)
                     << " in conflict map.");
            ++conflictIt;
            continue;
          }

          // Got a live one
          ++conflictCounter;
          ++conflictIt;

          // If more than one node is scheduled for execution, we have a resource contention.
          // N.B.: If this message triggers, nodeToExecute has been set in a previous iteration
          // *** FIXME: This is a plan error. Find a non-fatal way to handle this conflict!! ***
          checkError(conflictCounter < 2,
                     "Error: nodes " << node->getNodeId() << ' ' << node << " and "
                     << nodeToExecute->getNodeId() << ' ' << nodeToExecute
                     << " are in contention over variable "
                     << var->toString() << " and have equal priority.");

          nodeToExecute = node;
          destState = dest;
        }
      }

      if (!nodeToExecute) {
        // FIXME - If top priority nodes were removed from conflict list, are there more?
        debugMsg("PlexilExec:resolveResourceConflicts",
                 " No eligible Assignment nodes for " << var->getName());
        return;
      }

      if (destState == EXECUTING_STATE || destState == FAILING_STATE) {
        debugMsg("PlexilExec:resolveResourceConflicts",
                 " Node " << nodeToExecute->getNodeId() << ' ' << nodeToExecute
                 << " has best priority.");
        addStateChangeNode(nodeToExecute);
      }
      else {
        condDebugMsg(nodeToExecute->getState() == EXECUTING_STATE
                     || nodeToExecute->getState() == FAILING_STATE,
                     "PlexilExec:resolveResourceConflicts",
                     " Node for " << var->getName() << " already executing.  Nothing to resolve.");
      }
    }

    //
    // Internal queue management
    //

    // N.B. A node can be in only one queue at a time.

    /**
     * @brief Dequeue a node from the candidate queue.
     * @return Pointer to the top node in the queue, or NULL if queue empty.
     */
    Node *getCandidateNode() {
      Node *result = m_candidateQueue.front();
      if (!result)
        return NULL;

      m_candidateQueue.pop();
      result->setQueueStatus(QUEUE_NONE);
      return result;
    }

    Node *getStateChangeNode() {
      Node *result = m_stateChangeQueue.front();
      if (!result)
        return NULL;
    
      QueueStatus was = (QueueStatus) result->getQueueStatus();
      m_stateChangeQueue.pop();
      result->setQueueStatus(QUEUE_NONE);
      if (was == QUEUE_TRANSITION_CHECK)
        result->notifyChanged();
      return result;
    }

    void addStateChangeNode(Node *node) {
      switch (node->getQueueStatus()) {
      case QUEUE_NONE:
        debugMsg("PlexilExec:addStateChangeNode",
                 " Placing node " << node->getNodeId() << ' ' << node <<
                 " on the state change queue in position " << ++m_queuePos);
        node->setQueueStatus(QUEUE_TRANSITION);
        m_stateChangeQueue.push(node);
        return;

      case QUEUE_CHECK:             // shouldn't happen
        errorMsg("Cannot add node " << node->getNodeId() << ' ' << node
                 << " to transition queue, is still in candidate queue");
        return;

      case QUEUE_TRANSITION:        // already in queue, nothing to do
        debugMsg("PlexilExec:addStateChangeNode",
                 " node " << node->getNodeId() << ' ' << node << " is already in transition queue, ignoring");
        return;

      case QUEUE_TRANSITION_CHECK:  // already in queue, nothing to do
        debugMsg("PlexilExec:addStateChangeNode",
                 " node " << node->getNodeId() << ' ' << node
                 << " is already in transition queue AND scheduled for check queue, ignoring");
        return;

      case QUEUE_DELETE:            // cannot possibly transition
        errorMsg("Cannot add node " << node->getNodeId() << ' ' << node
                 << " to transition queue, is finished root node pending deletion");
        return;
      }
    }

    Node *getFinishedRootNode() {
      Node *result = m_finishedRootNodes.front();
      if (!result)
        return NULL;
    
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

  };

  // Public constructor
  PlexilExec *makePlexilExec()
  {
    return new PlexilExecImpl();
  }

}
