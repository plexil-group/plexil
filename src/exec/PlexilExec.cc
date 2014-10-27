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

#include "PlexilExec.hh"

#include "Assignment.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecListenerBase.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "Node.hh"
#include "NodeConstants.hh"
#include "NodeFactory.hh"
#include "NodeTransition.hh"

#include <algorithm> // for find(), transform
#include <iterator> // for back_insert_iterator
#include <functional>
#include <time.h>

namespace PLEXIL 
{

  /**
   * @brief Comparator for ordering nodes that are in conflict.  Higher priority wins, but nodes already EXECUTING dominate.
   */
  struct NodeConflictComparator {
    bool operator() (Node const *x, Node const *y) const
    {
      return (x->getPriority() < y->getPriority());
    }
  };

  PlexilExec::PlexilExec()
    : ExecConnector(),
      m_listener(NULL),
      m_queuePos(0),
      m_finishedRootNodesDeleted(false)
  {}

  PlexilExec::~PlexilExec() 
  {
    // Every node on this list is also in m_plan
    m_finishedRootNodes.clear();
    for (std::list<Node *>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
      delete (Node*) (*it);
  }

  bool PlexilExec::addPlan(Node *root)
  {
    m_plan.push_back(root);
    root->activate();
    debugMsg("PlexilExec:addPlan",
             "Added plan: " << std::endl << root->toString());
    root->conditionChanged(); // redundant?
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
    checkError(!node->getParent(),
               "PlexilExec::markRootNodeFinished: Node \"" << node->getNodeId()
               << "\" is not a root node");
    checkError(node->getState() == FINISHED_STATE,
               "PlexilExec::markRootNodeFinished: node not in FINISHED state");
    // TODO: ensure all descendants in FINISHED state
    m_finishedRootNodes.push_back(node);
  }

  void PlexilExec::deleteFinishedPlans()
  {
    if (!m_finishedRootNodes.empty()) {
      for (std::vector<Node *>::iterator it = m_finishedRootNodes.begin();
           it != m_finishedRootNodes.end();
           ++it) {
        check_error_2(*it,
                      "PlexilExec::deleteFinishedPlans: null node in finished list");
        Node *node = *it;
        debugMsg("PlexilExec:deleteFinishedPlans",
                 " deleting node \"" << node->getNodeId() << "\"");
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
                      "PlexilExec::deleteFinishedPlan: Node \"" << node->getNodeId()
                      << "\" not found on active root node list");
        // Now safe to delete
        delete node;
      }
      m_finishedRootNodes.clear();
      m_finishedRootNodesDeleted = true;
    }
  }

  void PlexilExec::notifyNodeConditionChanged(Node *node)
  {
    debugMsg("PlexilExec:notifyNodeConditionChanged", " for node " << node->getNodeId());
    m_nodesToConsider.push(node);
  }

  bool PlexilExec::needsStep() const
  {
    return !m_nodesToConsider.empty();
  }

  //as a possible optimization, if we spend a lot of time searching through this list,
  //it should be faster to search the list backwards.
  void PlexilExec::handleConditionsChanged(Node *node, NodeState destState) 
  {
    debugMsg("PlexilExec:handleConditionsChanged",
             "Node " << node->getNodeId() << " had a relevant condition change.");

    debugMsg("PlexilExec:handleConditionsChanged",
             "Considering node '" << node->getNodeId() << "' for state transition.");
    if (node->getType() == NodeType_Assignment) {
      // Node can be in contention in either EXECUTING or FAILING 
      NodeState current = node->getState();
      switch (destState) {

      case EXECUTING_STATE: {
        // add it to contention consideration
        debugMsg("PlexilExec:handleConditionsChanged",
                 "Node '" << node->getNodeId() <<
                 "' is an assignment node that could be executing.  Adding it to the " <<
                 "resource contention list ");
        addToResourceContention(node);
        return;
      }

      case FAILING_STATE: // Is already in conflict set, and must be enqueued now
        debugMsg("PlexilExec:handleConditionsChanged",
                 "Node '" << node->getNodeId() <<
                 "' is an assignment node that is failing, and is already in the " <<
                 "resource contention list");
        m_variablesToRetract.push_back(node->getAssignmentVariable());
        break;

        // In addition to the obvious paths from EXECUTING,
        // the node could have been in WAITING and eligible for execution
        // but deferred to a higher priority node, then failed/exited/skipped
        // before it could execute.
      case ITERATION_ENDED_STATE:
      case FINISHED_STATE:
        if (current == EXECUTING_STATE
            || current == FAILING_STATE
            || current == WAITING_STATE) {
          debugMsg("PlexilExec:handleConditionsChanged",
                   "Node '" << node->getNodeId() <<
                   "' is an assignment node that is no longer possibly executing.  " <<
                   "Removing it from resource contention.");
          removeFromResourceContention(node);
        }
        break;

      default: // Is not in contention now, and not entering it either
        break;
      } // end switch (destState)
    }

    m_stateChangeQueue.push_back(node);
    debugMsg("PlexilExec:handleConditionsChanged",
             "Placing node '" << node->getNodeId() <<
             "' on the state change queue in position " << ++m_queuePos);
  }

  /**
   * @brief Schedule this assignment for execution.
   */
  void PlexilExec::enqueueAssignment(Assignment *assign)
  {
    m_assignmentsToExecute.push_back(assign);
  }

  /**
   * @brief Schedule this assignment for retraction.
   */
  void PlexilExec::enqueueAssignmentForRetraction(Assignment *assign)
  {
    m_assignmentsToRetract.push_back(assign);
  }

  // Assumes node is a valid ID and points to an Assignment node
  void PlexilExec::removeFromResourceContention(Node *node) 
  {
    Assignable *lhs = node->getAssignmentVariable();
    assertTrue_1(lhs && lhs->isAssignable());
    Assignable *exp = lhs->asAssignable();
    assertTrue_1(exp != NULL);
    exp = exp->getBaseVariable();
    assertTrue_1(exp != NULL);

    // Remove node from the variable's conflict set.
    VariableConflictSet &conflictNodes = exp->getConflictSet();
    conflictNodes.remove(node);

    // If deleted node was only one in conflict set,
    // remove variable from variable set.
    if (conflictNodes.empty()) {
      std::vector<Assignable *>::iterator varIt =
        std::find(m_resourceConflicts.begin(), m_resourceConflicts.end(), exp);
      if (varIt != m_resourceConflicts.end())
        m_resourceConflicts.erase(varIt);
    }
  }

  // Assumes node is a valid ID and points to an Assignment node whose next state is EXECUTING
  void PlexilExec::addToResourceContention(Node *node)
  {
    Assignable *lhs = node->getAssignmentVariable();
    assertTrue_1(lhs && lhs->isAssignable());
    Assignable *exp = lhs->asAssignable();
    assertTrue_1(exp != NULL);
    exp = exp->getBaseVariable();
    assertTrue_1(exp != NULL);

    debugMsg("PlexilExec:addToResourceContention",
             "Adding node '" << node->getNodeId() << "' to resource contention.");
    VariableConflictSet &conflictNodes = exp->getConflictSet();
    if (conflictNodes.empty())
      // add variable to list of conflicts
      m_resourceConflicts.push_back(exp);
    conflictNodes.push(node);
  }

  void PlexilExec::step(double startTime) 
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
    debugMsg("PlexilExec:cycle", "==>Start cycle " << cycleNum);

    // BEGIN QUIESCENCE LOOP
    do {
      // Size of m_nodesToConsider is upper bound on queue size
      m_stateChangeQueue.reserve(m_nodesToConsider.size());

      // Evaluate conditions of nodes reporting a change
      while (!m_nodesToConsider.empty()) {
        Node *candidate = m_nodesToConsider.front();
        m_nodesToConsider.pop();
        check_error_1(candidate);
        candidate->checkConditions(); // modifies m_stateChangeQueue, m_resourceConflicts
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
      std::vector<NodeTransition> transitionsToPublish;
      transitionsToPublish.reserve(m_stateChangeQueue.size());
      for (std::vector<Node *>::const_iterator it = m_stateChangeQueue.begin();
           it != m_stateChangeQueue.end();
           ++it) {
        Node *node = *it;
        debugMsg("PlexilExec:step",
                 "[" << cycleNum << ":" << stepCount << ":" << microStepCount <<
                 "] Transitioning node " << node->getNodeId()
                 << " from " << node->getStateName()
                 << " to " << nodeStateName(node->getNextState()));
        NodeState oldState = node->getState();
        node->transition(startTime);
        transitionsToPublish.push_back(NodeTransition(node, oldState));
        ++microStepCount;
      }

      // TODO: instrument high-water-mark of max nodes transitioned in this step

      // Synchronously notify transitioned nodes they may be eligible again
      for (std::vector<Node *>::const_iterator it = m_stateChangeQueue.begin();
           it != m_stateChangeQueue.end();
           ++it)
         (*it)->conditionChanged();

      // Publish the transitions
      // FIXME: Move call to listener outside of quiescence loop
      if (m_listener)
        m_listener->notifyOfTransitions(transitionsToPublish);

      // done with this batch
      m_stateChangeQueue.clear();

      ++stepCount;
    }
    while (m_assignmentsToExecute.empty()
           && m_assignmentsToRetract.empty()
           && g_interface->outboundQueueEmpty()
           && !m_nodesToConsider.empty());
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
    for (std::vector<Assignment *>::iterator it = m_assignmentsToExecute.begin();
         it != m_assignmentsToExecute.end();
         ++it) {
      Assignment *assn = *it;
      check_error_1(assn);
      assn->execute();
    }
    m_assignmentsToExecute.clear();
    for (std::vector<Assignment *>::iterator it = m_assignmentsToRetract.begin();
         it != m_assignmentsToRetract.end();
         ++it) {
      Assignment *assn = *it;
      check_error_1(assn);
      assn->retract();
    }
    m_assignmentsToRetract.clear();
    m_variablesToRetract.clear();
  }

  void PlexilExec::resolveResourceConflicts()
  {
    for (std::vector<Assignable *>::iterator it = m_resourceConflicts.begin();
         it != m_resourceConflicts.end();
         ++it)
      resolveVariableConflicts(*it);
  }

  /**
   * @brief Resolve conflicts for this variable.
   * @note Subroutine of resolveResourceConflicts() above.
   */
  void PlexilExec::resolveVariableConflicts(Assignable *var)
  {
    VariableConflictSet &conflictNodes = var->getConflictSet();
    checkError(!conflictNodes.empty(),
               "Resource conflict set for " << var->toString() << " is empty.");

    // Ignore any variables pending retraction
    for (std::vector<Assignable *>::const_iterator vit = m_variablesToRetract.begin();
         vit != m_variablesToRetract.end();
         ++vit) {
      if ((*vit)->getBaseVariable() == var) { // compare base variables for (e.g.) aliases, array refs
        debugMsg("PlexilExec:resolveResourceConflicts",
                 " Ignoring Assignments for variable '" << var->getName()
                 << "', which has a retraction pending");
        return;
      }
    }

    //we only have to look at all the nodes with the highest priority
    Node *nodeToExecute = NULL;
    NodeState destState = NO_NODE_STATE;
    size_t count = conflictNodes.front_count(); // # of nodes with same priority as top
    if (count == 1) {
      // Usual case (we hope) - make it simple
      nodeToExecute = dynamic_cast<Node *>(conflictNodes.front());
      destState = nodeToExecute->getNextState();
    }

    else {
      VariableConflictSet::iterator conflictIt = conflictNodes.begin(); 
      // Look at the destination states of all the nodes with equal priority
      for (size_t i = 0, conflictCounter = 0; i < count; ++i, ++conflictIt) {
        Node *node = dynamic_cast<Node *>(*conflictIt);
        NodeState dest = node->getNextState();

        // Found one that is scheduled for execution
        if (dest == EXECUTING_STATE || dest == FAILING_STATE)
          ++conflictCounter;
        else 
          // Internal error
          checkError(node->getState() == EXECUTING_STATE
                     || node->getState() == FAILING_STATE,
                     "Error: node '" << node->getNodeId()
                     << " is neither executing nor failing nor eligible for either, yet is in conflict map.");

        // If more than one node is scheduled for execution, we have a resource contention.
        // *** FIXME: This is a plan error. Find a non-fatal way to report this conflict!! ***
        checkError(conflictCounter < 2,
                   "Error: nodes '" << node->getNodeId() << "' and '"
                   << nodeToExecute->getNodeId() << "' are in contention over variable "
                   << var->toString() << " and have equal priority.");

        nodeToExecute = node;
        destState = dest;
      }
      assertTrue_1(nodeToExecute);
    }

    if (destState == EXECUTING_STATE || destState == FAILING_STATE) {
      m_stateChangeQueue.push_back(nodeToExecute);
      debugMsg("PlexilExec:resolveResourceConflicts",
               "Node '" << nodeToExecute->getNodeId()
               << "' has best priority.  Adding it to be executed in position "
               << ++m_queuePos);
    }
    else {
      condDebugMsg(nodeToExecute->getState() == EXECUTING_STATE
                   || nodeToExecute->getState() == FAILING_STATE,
                   "PlexilExec:resolveResourceConflicts",
                   "Node for " << var->getName() << " already executing.  Nothing to resolve.");
    }
  }


  std::string PlexilExec::stateChangeQueueStr() {
    std::ostringstream retval;
    for (std::vector<Node *>::const_iterator it = m_stateChangeQueue.begin();
         it != m_stateChangeQueue.end();
         ++it) {
      check_error_1(*it);
      retval << (*it)->getNodeId() << " ";
    }
    return retval.str();
  }
}
