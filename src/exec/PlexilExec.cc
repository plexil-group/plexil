/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
#include "CoreExpressions.hh" // for StateVariable::nodeStateName
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExecListenerHub.hh"
#include "Expression.hh"
#include "ExternalInterface.hh"
#include "Node.hh"
#include "NodeFactory.hh"
#include "StateCache.hh"
#include "Variable.hh"

#include <algorithm> // for find(), transform
#include <iterator> // for back_insert_iterator
#include <map>
#include <functional>
#include <time.h>

namespace PLEXIL 
{
  bool NodeConflictComparator::operator() (NodeId x, NodeId y) const 
  {
    check_error(x->getType() == Node::ASSIGNMENT());
    check_error(y->getType() == Node::ASSIGNMENT());
    return (x->getPriority() < y->getPriority() ? true : false);
  }

  PlexilExec::PlexilExec(PlexilNodeId& plan)
    : ExecConnector(),
      m_id(this, ExecConnector::getId()),
      m_cache((new StateCache())->getId()),
      m_cycleNum(0), m_queuePos(0)
  {
    addPlan(plan, EMPTY_LABEL());
    //is it really this simple?
  }

  PlexilExec::PlexilExec()
    : ExecConnector(),
      m_id(this, ExecConnector::getId()),
      m_cache((new StateCache())->getId()),
      m_cycleNum(0), m_queuePos(0),
      m_finishedRootNodesDeleted(false)
  {}

  PlexilExec::~PlexilExec() 
  {
    // Every node on this list is also in m_plan
    m_finishedRootNodes.clear();
    for (std::list<NodeId>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
      delete (Node*) (*it);
    delete (StateCache*) m_cache;
    m_id.removeDerived(ExecConnector::getId());
  }

  /**
   * @brief Set the ExternalInterface instance used by this Exec.
   * @param id The Id of the ExternalInterface instance.
   */
  void PlexilExec::setExternalInterface(ExternalInterfaceId& id)
  {
    m_interface = id;
    m_cache->setExternalInterface(id);
  }

  /**
   * @brief Retrieves the named library node if it is present.
   * @param nodeName The name of the library node.
   * @return The library node, or noId() if not found.
   */
  const PlexilNodeId PlexilExec::getLibrary(const std::string& nodeName) const
  {
    checkError(!nodeName.empty(),
               "PlexilExec::getLibrary: Node name is empty");
    std::map<std::string, PlexilNodeId>::const_iterator it = 
      m_libraries.find(nodeName);
    if (it == m_libraries.end()) {
      debugMsg("PlexilExec:getLibrary", " library node \"" << nodeName << "\" not found");
      return PlexilNodeId::noId();
    }
    debugMsg("PlexilExec:getLibrary", " found library node \"" << nodeName << "\"");
    return it->second;
  }

  /**
   * @brief Queries whether the named library node is loaded.
   * @param nodeName The name of the library node.
   * @return True if the node is already defined, false otherwise.
   */
  bool PlexilExec::hasLibrary(const std::string& nodeName) const {
    return getLibrary(nodeName).isId();
  }

  // Add a new library node

  void PlexilExec::addLibraryNode(const PlexilNodeId& libNode) 
  {
    checkError(libNode.isValid(), 
               "PlexilExec::addLibraryNode: Invalid library node pointer");
    const std::string& nodeName = libNode->nodeId();
    checkError(!nodeName.empty(), 
               "PlexilExec::addLibraryNode: Library node ID value is empty");

    // Check for previous
    PlexilNodeId oldNode;
    if (m_libraries.find(nodeName) != m_libraries.end()) {
      oldNode = m_libraries[nodeName];
    }

    m_libraries[nodeName] = libNode;
    debugMsg("PlexilExec:addLibrary",
             "Added library node \"" << nodeName
             << (oldNode.isId() ? "\", deleting previous" : "\""));
    if (m_listener.isId())
      m_listener->notifyOfAddLibrary(libNode);

    // Delete previous
    if (oldNode.isId()) {
      delete (PlexilNode*) oldNode;
    }
  }

  // Add a plan
  // Currently parent is ignored

  bool PlexilExec::addPlan(PlexilNodeId& plan, const LabelStr& parent) 
  {
    // Try to link any library calls
    if (!plan->link(m_libraries)) {
      debugMsg("PlexilExec:addPlan", " library linking failed");
      return false;
    }

    // Try to construct the node, 
    // and catch any errors that may occur
    NodeId root;
    bool wasThrowEnabled = Error::throwEnabled();
    try {
      if (!wasThrowEnabled)
        Error::doThrowExceptions();
      root = NodeFactory::createNode(plan, ExecConnector::getId());
      check_error(root.isValid());
      root->postInit(plan);
    }
    catch (const Error& e) {
      if (!wasThrowEnabled)
        Error::doNotThrowExceptions();
      debugMsg("PlexilExec:addPlan", " failed: " << e);
      return false;
    }
    if (!wasThrowEnabled)
      Error::doNotThrowExceptions();

    // after this point any failures are likely to be fatal!
    //not actually quiesceing, but causing the new nodes to look at the current known world state
    m_cache->handleQuiescenceStarted();
    m_plan.push_back(root);
    root->activate();
    debugMsg("PlexilExec:addPlan",
             "Added plan: " << std::endl << root->toString());
    if (m_listener.isId())
      m_listener->notifyOfAddPlan(plan, parent);
    root->conditionChanged();
    m_cache->handleQuiescenceEnded();

    return true;
  }

  /**
   * @brief Queries whether all plans are finished.
   * @return true if all finished, false otherwise.
   */

  // FIXME: this will return false if a finished plan has been deleted
  bool PlexilExec::allPlansFinished() const
  {
    // If we're in a quiescence cycle, presume some plan is active.
    if (m_cache->inQuiescence())
      return false;

    bool result = m_finishedRootNodesDeleted; // return value in the event no plan is active

    for (std::list<NodeId>::const_iterator planit = m_plan.begin();
         planit != m_plan.end();
         ++planit)
      {
        NodeId root = *planit;
        if (root->getState() == FINISHED_STATE)
          result = true;
        else
          return false; // some node is not finished
      }
    return result;
  }

  void PlexilExec::markRootNodeFinished(const NodeId& node)
  {
    checkError(node.isValid(),
               "PlexilExec::markRootNodeFinished: node pointer is invalid");
    checkError(node->getParent().isNoId(),
               "PlexilExec::markRootNodeFinished: Node \"" << node->getNodeId().toString()
               << "\" is not a root node");
    checkError(node->getState() == FINISHED_STATE,
               "PlexilExec::markRootNodeFinished: node not in FINISHED state");
    // TODO: ensure all descendants in FINISHED state
    m_finishedRootNodes.push_back(node);
  }

  void PlexilExec::deleteFinishedPlans()
  {
    if (!m_finishedRootNodes.empty()) {
      for (std::vector<NodeId>::iterator it = m_finishedRootNodes.begin();
           it != m_finishedRootNodes.end();
           ++it) {
        checkError(it->isValid(),
                   "PlexilExec::deleteFinishedPlans: attempt to delete node at invalid pointer");
        NodeId node = *it;
        debugMsg("PlexilExec:deleteFinishedPlans",
                 " deleting node \"" << node->getNodeId().toString() << "\"");
        // Remove from active plan
        bool found = false;
        for (std::list<NodeId>::iterator pit = m_plan.begin();
             pit != m_plan.end();
             ++pit) {
          if (*pit == node) {
            found = true;
            m_plan.erase(pit);
            break;
          }
        }
        assertTrueMsg(found,
                      "PlexilExec::deleteFinishedPlan: Node \"" << node->getNodeId().toString()
                      << "\" not found on active root node list");
        // Now safe to delete
        delete (Node*) node;
      }
      m_finishedRootNodes.clear();
      m_finishedRootNodesDeleted = true;
    }
  }

  void PlexilExec::notifyNodeConditionChanged(NodeId node)
  {
    debugMsg("PlexilExec:notifyNodeConditionChanged", " for node " << node->getNodeId().toString());
    m_nodesToConsider.push(node);
  }

  bool PlexilExec::needsStep() const
  {
    return !m_nodesToConsider.empty();
  }

  //as a possible optimization, if we spend a lot of time searching through this list,
  //it should be faster to search the list backwards.
  void PlexilExec::handleConditionsChanged(const NodeId& node, NodeState destState) 
  {
    debugMsg("PlexilExec:handleConditionsChanged",
             "Node " << node->getNodeId().toString() << " had a relevant condition change.");

    if (destState == NO_NODE_STATE) {
      // Node not eligible to transition
      debugMsg("PlexilExec:handleConditionsChanged",
               "Node '" << node->getNodeId().toString() <<
               "' was previously eligible to transition but isn't now.");
      return;
    }
    debugMsg("PlexilExec:handleConditionsChanged",
             "Considering node '" << node->getNodeId().toString() << "' for state transition.");
    if (node->getType() == Node::ASSIGNMENT()) {
      // Node can be in contention in either EXECUTING or FAILING 
      NodeState current = node->getState();
      switch (destState) {

      case EXECUTING_STATE: {
        // add it to contention consideration
        debugMsg("PlexilExec:handleConditionsChanged",
                 "Node '" << node->getNodeId().toString() <<
                 "' is an assignment node that could be executing.  Adding it to the " <<
                 "resource contention list ");
        addToResourceContention(node);
        return;
      }

      case FAILING_STATE: // Is already in conflict set, and must be enqueued now
        debugMsg("PlexilExec:handleConditionsChanged",
                 "Node '" << node->getNodeId().toString() <<
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
                   "Node '" << node->getNodeId().toString() <<
                   "' is an assignment node that is no longer possibly executing.  " <<
                   "Removing it from resource contention.");
          removeFromResourceContention(node);
        }
        break;

      default: // Is not in contention now, and not entering it either
        break;
      } // end switch (destState)
    }

    m_stateChangeQueue.push_back(NodeTransition(node, destState));
    debugMsg("PlexilExec:handleConditionsChanged",
             "Placing node '" << node->getNodeId().toString() <<
             "' on the state change queue in position " << ++m_queuePos);
  }

  /**
   * @brief Schedule this assignment for execution.
   */
  void PlexilExec::enqueueAssignment(const AssignmentId& assign)
  {
    m_assignmentsToExecute.push_back(assign);
  }

  /**
   * @brief Schedule this assignment for retraction.
   */
  void PlexilExec::enqueueAssignmentForRetraction(const AssignmentId& assign)
  {
    m_assignmentsToRetract.push_back(assign);
  }

  /**
   * @brief Schedule this command for execution.
   */
  void PlexilExec::enqueueCommand(const CommandId& cmd)
  {
    m_commandsToExecute.push_back(cmd);
  }

  /**
   * @brief Schedule this update for execution.
   */
  void PlexilExec::enqueueUpdate(const UpdateId& update)
  {
    m_updatesToExecute.push_back(update);
  }

  // Assumes node is a valid ID and points to an Assignment node
  void PlexilExec::removeFromResourceContention(const NodeId& node) 
  {
    VariableId exp = node->getAssignmentVariable()->getBaseVariable();
    check_error(exp.isValid());

    VariableConflictMap::iterator conflict = m_resourceConflicts.find(exp);
    if (conflict == m_resourceConflicts.end())
      return;

    // Remove node from the conflict set.
    // If there are multiple nodes with the same priority, find() may not point us at the one we want
    VariableConflictSet& conflictSet = conflict->second;
    VariableConflictSet::iterator conflictIt = conflictSet.find(node);
    if (conflictIt == conflictSet.end())
      return; // already removed

    if ((*conflictIt) != node) {
      bool found = false;
      ++conflictIt;
      while (conflictIt != conflictSet.end()
             && (*conflictIt)->getPriority() == node->getPriority()) {
        if (*conflictIt == node) {
          found = true;
          break;
        }
      }
      if (!found)
        return;
    }
    // Found it, delete it
    conflictSet.erase(conflictIt);

    if (conflict->second.empty())
      m_resourceConflicts.erase(exp);
  }

  // Assumes node is a valid ID and points to an Assignment node whose next state is EXECUTING
  void PlexilExec::addToResourceContention(const NodeId& node) {
    VariableId exp = node->getAssignmentVariable()->getBaseVariable();
    check_error(exp.isValid());

    debugMsg("PlexilExec:addToResourceContention",
             "Adding node '" << node->getNodeId().toString() << "' to resource contention.");
    VariableConflictMap::iterator resourceIt = m_resourceConflicts.find(exp);
    if (resourceIt == m_resourceConflicts.end()) {
      // No conflict set for this variable, so create one with this node
      std::pair<VariableConflictMap::iterator, bool> insertResult =
        m_resourceConflicts.insert(std::make_pair(exp, VariableConflictSet()));
      insertResult.first->second.insert(node);
      return;
    }

    VariableConflictSet& conflictSet = resourceIt->second;

    // Check to prevent redundant insertion
    VariableConflictSet::iterator conflictSetIter;
    if ((conflictSetIter = conflictSet.find(node)) != conflictSet.end()) {
      // Check only nodes with the same priority
      size_t count = conflictSet.count(node);
      debugMsg("PlexilExec:addToResourceContention",
               "There are " << count << " similar items in the set.");
      for (size_t i = 0; i < count; ++i, ++conflictSetIter) {
        if (node == *conflictSetIter) {
          debugMsg("PlexilExec:addToResourceContention",
                   "Skipping node '" << node->getNodeId().toString() <<
                   "' because it's already in the set.");
          return;
        }
      }
    }

    // OK to insert
    conflictSet.insert(node);
  }

  void PlexilExec::step() 
  {
    //
    // *** BEGIN CRITICAL SECTION ***
    //
    m_cache->handleQuiescenceStarted();

    // Queue had better be empty when we get here!
    checkError(m_stateChangeQueue.empty(), "State change queue not empty at entry");

    unsigned int stepCount = 0;
    ++m_cycleNum;
    debugMsg("PlexilExec:cycle", "==>Start cycle " << m_cycleNum);

    double quiescenceTime = m_cache->currentTime();

    // BEGIN QUIESCENCE LOOP
    do {
      // Size of m_nodesToConsider is upper bound on queue size
      m_stateChangeQueue.reserve(m_nodesToConsider.size());

      // Evaluate conditions of nodes reporting a change
      while (!m_nodesToConsider.empty()) {
        NodeId candidate = m_nodesToConsider.front();
        m_nodesToConsider.pop();
        // If we have to check node validity, do it here, instead of redundantly below.
        check_error(candidate.isValid());
        candidate->checkConditions(); // modifies m_stateChangeQueue, m_resourceConflicts
      }

      // Sort Assignment nodes by priority
      resolveResourceConflicts();

      if (m_stateChangeQueue.empty())
        break; // nothing to do, exit quiescence loop

      debugMsg("PlexilExec:step",
               "[" << m_cycleNum << ":" << stepCount << "] State change queue: "
               << stateChangeQueueStr());

      unsigned int microStepCount = 0;

      // Lock the nodes into their current transitioning states
      for (StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
           it != m_stateChangeQueue.end();
           ++it) {
        const NodeId& node = it->node;
        debugMsg("PlexilExec:lock",
                 "Locking node " << node->getNodeId().toString());
        node->lockConditions();
      }

      // Transition the nodes
      std::vector<NodeTransition> transitionsToPublish;
      transitionsToPublish.reserve(m_stateChangeQueue.size());
      for (StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
           it != m_stateChangeQueue.end();
           ++it) {
        const NodeId& node = it->node;
        debugMsg("PlexilExec:step",
                 "[" << m_cycleNum << ":" << stepCount << ":" << microStepCount <<
                 "] Transitioning node " << node->getNodeId().toString()
                 << " from " << node->getStateName().toString()
                 << " to " << StateVariable::nodeStateName(it->state).toString());
        NodeState oldState = node->getState();
        node->transition(it->state, quiescenceTime);
        transitionsToPublish.push_back(NodeTransition(node, oldState));
        ++microStepCount;
      }

      // TODO: instrument high-water-mark of max nodes transitioned in this step

      // Publish the transitions
      if (m_listener.isId())
        m_listener->notifyOfTransitions(transitionsToPublish);

      // Unlock the nodes
      for (StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
           it != m_stateChangeQueue.end();
           ++it) {
        const NodeId& node = it->node;
        debugMsg("PlexilExec:unlock",
                 "Unlocking node " << node->getNodeId().toString());
        node->unlockConditions();
      }

      // done with this batch
      m_stateChangeQueue.clear();

      ++stepCount;
    }
    while (m_assignmentsToExecute.empty()
           && m_assignmentsToRetract.empty()
           && m_commandsToExecute.empty()
           && m_updatesToExecute.empty()
           && !m_nodesToConsider.empty());
    // END QUIESCENCE LOOP

    // Perform side effects

    performAssignments();

    std::list<CommandId> commands(m_commandsToExecute.begin(), m_commandsToExecute.end());
    m_commandsToExecute.clear();
    getExternalInterface()->batchActions(commands);

    std::list<UpdateId> updates(m_updatesToExecute.begin(), m_updatesToExecute.end());
    m_updatesToExecute.clear();
    getExternalInterface()->updatePlanner(updates);

    debugMsg("PlexilExec:cycle", "==>End cycle " << m_cycleNum);
    for (std::list<NodeId>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
      debugMsg("PlexilExec:printPlan", std::endl << **it);
    }
    m_cache->handleQuiescenceEnded();
    //
    // *** END CRITICAL SECTION ***
    //
  }

  void PlexilExec::performAssignments() 
  {
    debugMsg("PlexilExec:performAssignments",
             " performing " << m_assignmentsToExecute.size() <<  " assignments and "
             << m_assignmentsToRetract.size() << " retractions");
    for (std::vector<AssignmentId>::iterator it = m_assignmentsToExecute.begin();
         it != m_assignmentsToExecute.end();
         ++it) {
      AssignmentId assn = *it;
      check_error(assn.isValid());
      assn->execute();
    }
    m_assignmentsToExecute.clear();
    for (std::vector<AssignmentId>::iterator it = m_assignmentsToRetract.begin();
         it != m_assignmentsToRetract.end();
         ++it) {
      AssignmentId assn = *it;
      check_error(assn.isValid());
      assn->retract();
    }
    m_assignmentsToRetract.clear();
    m_variablesToRetract.clear();
  }

  void PlexilExec::resolveResourceConflicts()
  {
    for (VariableConflictMap::iterator it = m_resourceConflicts.begin(); it != m_resourceConflicts.end(); ++it)
      resolveVariableConflicts(it->first, it->second);
  }

  /**
   * @brief Resolve conflicts for this variable.
   * @note Subroutine of resolveResourceConflicts() above.
   */
  void PlexilExec::resolveVariableConflicts(const VariableId& var,
                                            const VariableConflictSet& conflictSet)
  {
    // Ignore any variables pending retraction
    // Grumble... Why doesnt std:;vector<T> have a find() member fn?
    for (std::vector<VariableId>::const_iterator vit = m_variablesToRetract.begin();
         vit != m_variablesToRetract.end();
         ++vit) {
      if (*vit == var){
        debugMsg("PlexilExec:resolveResourceConflicts",
                 " Ignoring Assignments for variable '" << var->getName()
                 << "', which has a retraction pending");
        return;
      }
    }

    checkError(!conflictSet.empty(),
               "Resource conflict set for " << var->toString() << " is empty.");

    //we only have to look at all the nodes with the highest priority
    VariableConflictSet::iterator conflictIt = conflictSet.begin(); 
    size_t count = conflictSet.count(*conflictIt); // # of nodes with same priority as top

    NodeId nodeToExecute;
    NodeState destState = NO_NODE_STATE;

    // Look at the destination states of all the nodes with equal priority
    for (size_t i = 0, conflictCounter = 0; i < count; ++i, ++conflictIt) {
      NodeId node = *conflictIt;
      NodeState dest = node->getDestState();

      check_error(node.isValid());

      // Found one that is scheduled for execution
      if (dest == EXECUTING_STATE || dest == FAILING_STATE)
        ++conflictCounter;
      else 
        // Internal error
        checkError(node->getState() == EXECUTING_STATE
                   || node->getState() == FAILING_STATE,
                   "Error: node '" << node->getNodeId().toString()
                   << " is neither executing nor failing nor eligible for either, yet is in conflict map.");

      // If more than one node is scheduled for execution, we have a resource contention.
      // *** FIXME: This is a plan error. Find a non-fatal way to report this conflict!! ***
      checkError(conflictCounter < 2,
                 "Error: nodes '" << node->getNodeId().toString() << "' and '"
                 << nodeToExecute->getNodeId().toString() << "' are in contention over variable "
                 << var->toString() << " and have equal priority.");

      nodeToExecute = node;
      destState = dest;
    }

    if (destState == EXECUTING_STATE || destState == FAILING_STATE) {
      m_stateChangeQueue.push_back(NodeTransition(nodeToExecute, destState));
      debugMsg("PlexilExec:resolveResourceConflicts",
               "Node '" << nodeToExecute->getNodeId().toString()
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
    for (StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
         it != m_stateChangeQueue.end();
         ++it) {
      check_error(it->node.isValid());
      retval << it->node->getNodeId().toString() << " ";
    }
    return retval.str();
  }

  // Convenience method for backward compatibility
  void PlexilExec::addListener(const ExecListenerBaseId& listener) 
  {
    check_error(m_listener.isValid());
    check_error(listener.isValid());
    m_listener->addListener(listener);
  }

  // Convenience method for backward compatibility
  void PlexilExec::removeListener(const ExecListenerBaseId& listener) 
  {
    check_error(m_listener.isValid());
    check_error(listener.isValid());
    m_listener->removeListener(listener);
  }
}
