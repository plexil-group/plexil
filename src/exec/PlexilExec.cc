/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

namespace PLEXIL {
  bool NodeConflictComparator::operator() (NodeId x, NodeId y) const {

    check_error(x->getType() == Node::ASSIGNMENT());
    check_error(y->getType() == Node::ASSIGNMENT());
    return (x->getPriority() < y->getPriority() ? true : false);
  }

  PlexilExec::PlexilExec(PlexilNodeId& plan)
    : ExecConnector(),
	  m_id(this, ExecConnector::getId()),
	  m_cycleNum(0), m_queuePos(1),
      m_cache((new StateCache())->getId()) {
    addPlan(plan, EMPTY_LABEL());
    //is it really this simple?
  }

  PlexilExec::PlexilExec()
    : ExecConnector(),
	  m_id(this, ExecConnector::getId()),
	  m_cycleNum(0), m_queuePos(1),
      m_cache((new StateCache())->getId())
  {}

  PlexilExec::~PlexilExec() 
  {
    for(std::list<NodeId>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
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

  // *** To do:
  //  - check node name for duplicates, replace old?

  void PlexilExec::addLibraryNode(const PlexilNodeId& libNode) {
	checkError(libNode.isId(), 
			   "PlexilExec::addLibraryNode: Invalid library node pointer");
    checkError(!libNode->nodeId().empty(), 
			   "PlexilExec::addLibraryNode: Library node ID value is empty");
    m_libraries[libNode->nodeId()] = libNode;
    debugMsg("PlexilExec:addLibrary",
			 "Added library node \"" << libNode->nodeId() << "\"");
	if (m_listener.isValid())
	  m_listener->notifyOfAddLibrary(libNode);
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
	if (m_listener.isValid())
	  m_listener->notifyOfAddPlan(plan, parent);
    root->conditionChanged();
    m_cache->handleQuiescenceEnded();

    return true;
  }

  /**
   * @brief Queries whether all plans are finished.
   * @return true if all finished, false otherwise.
   */
  bool PlexilExec::allPlansFinished() const
  {
    // If we're in a quiescence cycle, presume some plan is active.
    if (m_cache->inQuiescence())
      return false;

    bool result = false; // return value in the event no plan has been received

    for (std::list<NodeId>::const_iterator planit = m_plan.begin();
         planit != m_plan.end();
         planit++)
      {
        NodeId root = *planit;
        if (root->getState() == FINISHED_STATE)
          result = true;
        else
          return false; // some node is not finished
      }
    return result;
  }

  void PlexilExec::notifyNodeConditionChanged(NodeId node)
  {
    debugMsg("PlexilExec:notifyNodeConditionChanged", " for node " << node->getNodeId().toString());
    m_nodesToConsider.push_back(node);
  }

  bool PlexilExec::needsStep() const
  {
	return !m_nodesToConsider.empty();
  }

  //as a possible optimization, if we spend a lot of time searching through this list,
  //it should be faster to search the list backwards.
  void PlexilExec::handleConditionsChanged(const NodeId& node, NodeState destState) 
  {
    check_error(node.isValid());
    debugMsg("PlexilExec:handleConditionsChanged",
			 "Node " << node->getNodeId().toString() << " had a relevant condition change.");

    if (destState == NO_NODE_STATE) {
      debugMsg("PlexilExec:handleConditionsChanged",
			   "Node '" << node->getNodeId().toString() <<
			   "' was previously eligible to transition but isn't now.");
      int idx = inQueue(node);
      if (idx != -1) {
		debugMsg("PlexilExec:handleConditionsChanged",
				 "Removing " << node->getNodeId().toString() <<
				 " from the state change queue.");
		m_stateChangeQueue.erase(idx);
      }
      else {
		if ((node->getType() == Node::ASSIGNMENT()) &&
			node->getState() != EXECUTING_STATE) {
		  debugMsg("PlexilExec:handleConditionsChanged",
				   "Removing node from resource contention.");
		  removeFromResourceContention(node);
		}
      }
      return;
    }
    debugMsg("PlexilExec:handleConditionsChanged",
			 "Considering node '" << node->getNodeId().toString() << "' for state transition.");
    if (node->getType() == Node::ASSIGNMENT()) {
      if (destState == EXECUTING_STATE) {
		//if it's an assignment node and it's eligible to execute
		//add it to contention consideration
		debugMsg("PlexilExec:handleConditionsChanged",
				 "Node '" << node->getNodeId().toString() <<
				 "' is an assignment node that could be executing.  Adding it to the " <<
				 "resource contention list ");
		addToResourceContention(node);
		//remove it from the state change queue
		int it = inQueue(node);
		if(it != -1) {
		  debugMsg("PlexilExec:handleConditionsChanged",
				   "Node '" << node->getNodeId().toString() <<
				   "' is in the state change queue.  Removing it.");
		  m_stateChangeQueue.erase(it);
		}
		return;
      }
      else {
		debugMsg("PlexilExec:handleConditionsChanged",
				 "Node '" << node->getNodeId().toString() <<
				 "' is an assignment node that is no longer possibly executing.  " <<
				 "Removing it from resource contention.");
		removeFromResourceContention(node);
      }
    }

    if (inQueue(node) == -1) {
      debugMsg("PlexilExec:handleConditionsChanged",
			   "Placing node '" << node->getNodeId().toString() <<
			   "' on the state change queue in position " << m_queuePos);
      m_stateChangeQueue[m_queuePos++] = NodeTransition(node, destState);
    }
  }

  /**
   * @brief Schedule this assignment for execution.
   */
  void PlexilExec::enqueueAssignment(const AssignmentId& assign)
  {
      m_assignmentsToExecute.push_back(assign);
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
    //this check may be too restrictive, since we'll probably use this method
    //to remove nodes when they get batch-executed.
    //check_error(node->getDestState() != EXECUTING_STATE);

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
	  conflictIt++;
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
      m_resourceConflicts.insert(std::make_pair(exp, VariableConflictSet()));
      resourceIt = m_resourceConflicts.find(exp);
    }

    VariableConflictSet& conflictSet = resourceIt->second;
    VariableConflictSet::iterator conflictSetIter;
    //HACK.  maybe.  maybe not.
    if ((conflictSetIter = conflictSet.find(node)) != conflictSet.end()) {

      // There could be multiple nodes with the same priority,
      size_t count = conflictSet.count(node);
	  // Removing this invalidates the regression tests. Bugger.
      debugMsg("PlexilExec:addToResourceContention", "There are " << count << " similar items in the"
               << " set.");
      bool found = false;
      for (size_t i = 0; !found && (i < count); ++i, ++conflictSetIter) {
		if (node == *conflictSetIter) {
		  found = true;
		  break;
		}
	  }
      if (found) {
		debugMsg("PlexilExec:addToResourceContention",
				 "Skipping node '" << node->getNodeId().toString() <<
				 "' because it's already in the set.");
		return;
	  }
    }
    conflictSet.insert(node);
  }

  // Used in step() below.
  // Replaces the STL template of the same name; code is copied from the GNU STL implementation.

  template <class _Pair>
  struct select2nd : public std::unary_function<_Pair,
						typename _Pair::second_type>
    {
      typename _Pair::second_type&
      operator()(_Pair& __x) const
      { return __x.second; }

      const typename _Pair::second_type&
      operator()(const _Pair& __x) const
      { return __x.second; }
    };

  //interesting test case:  one node assigns to a variable.  another node only executes when the first is executing and also assigns to that variable
  void PlexilExec::step() {
    //
    // *** BEGIN CRITICAL SECTION ***
    //
    m_cache->handleQuiescenceStarted();
    m_cycleNum++;

    double quiescenceTime = m_cache->currentTime();
    debugMsg("PlexilExec:cycle", "==>Start cycle " << m_cycleNum);

    unsigned int stepCount = 0;

	// BEGIN QUIESCENCE LOOP
    while(true) {
      do {
		// Evaluate conditions of nodes reporting a change
		std::vector<NodeId> changedNodes(m_nodesToConsider);
		m_nodesToConsider.clear();
		for (std::vector<NodeId>::iterator nodeIt = changedNodes.begin();
			 nodeIt != changedNodes.end();
			 nodeIt++)
		  (*nodeIt)->checkConditions(); // modifies m_stateChangeQueue, m_resourceConflicts
      }
      while (!m_nodesToConsider.empty());

	  // Sort out (literally) Assignment nodes by priority
      resolveResourceConflicts();

      if(m_stateChangeQueue.empty())
		break; // exit quiescence loop
      std::list<NodeTransition> transitioningNodes;
      std::transform(m_stateChangeQueue.begin(), m_stateChangeQueue.end(),
					 std::back_insert_iterator<std::list<NodeTransition> >(transitioningNodes),
					 select2nd<StateChangeQueue::value_type>());
      m_stateChangeQueue.clear();

      std::ostringstream out;
      for(std::list<NodeTransition>::const_iterator it = transitioningNodes.begin();
		  it != transitioningNodes.end(); ++it) {
		out << it->node->getNodeId().toString() << " ";
      }
      debugMsg("PlexilExec:step",
			   "[" << m_cycleNum << ":" << stepCount << "] State change queue: " << out.str());

      unsigned int microStepCount = 0;
      std::set<NodeId> transitionedNodes;

      //lock the nodes into their current transitioning states
      for(std::list<NodeTransition>::iterator it = transitioningNodes.begin();
		  it != transitioningNodes.end(); ++it) {
		const NodeId& node = it->node;
		check_error(node.isValid());
		debugMsg("PlexilExec:lock",
				 "Locking node " << node->getNodeId().toString());
		node->lockConditions();
      }

	  // Initialize transition report list
	  std::vector<NodeTransition> transitionsToPublish;
	  transitionsToPublish.reserve(transitioningNodes.size());

      //transition them
      for (std::list<NodeTransition>::iterator it = transitioningNodes.begin();
		   it != transitioningNodes.end();
		   ++it) {
		const NodeId& node = it->node;
		check_error(node.isValid());
		checkError(transitionedNodes.find(node) == transitionedNodes.end(),
				   "Node " << node->getNodeId().toString() <<
				   " already transitioned in this step.");
		debugMsg("PlexilExec:step",
				 "[" << m_cycleNum << ":" << stepCount << ":" << microStepCount <<
				 "] Transitioning node " << node->getNodeId().toString());
		NodeState oldState = node->getState();
		node->transition(it->state, quiescenceTime);
		transitionsToPublish.push_back(NodeTransition(node, oldState));
		transitionedNodes.insert(node);
		++microStepCount;
      }
	  // publish the transitions
	  if (m_listener.isValid())
		m_listener->notifyOfTransitions(transitionsToPublish);

      //unlock the nodes
      for(std::list<NodeTransition>::iterator it = transitioningNodes.begin();
		  it != transitioningNodes.end(); ++it) {
		const NodeId& node = it->node;
		check_error(node.isValid());
		debugMsg("PlexilExec:unlock",
				 "Unlocking node " << node->getNodeId().toString());
		node->unlockConditions();
      }

      ++stepCount;
    }
	// END QUIESCENCE LOOP

	performAssignments();

    std::list<CommandId> commands(m_commandsToExecute.begin(), m_commandsToExecute.end());
    m_commandsToExecute.clear();
    getExternalInterface()->batchActions(commands);

    std::list<UpdateId> updates(m_updatesToExecute.begin(), m_updatesToExecute.end());
    m_updatesToExecute.clear();
    getExternalInterface()->updatePlanner(updates);

    debugMsg("PlexilExec:cycle", "==>End cycle " << m_cycleNum);
    for(std::list<NodeId>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
      debugMsg("PlexilExec:printPlan", std::endl << **it);
    }
    m_cache->handleQuiescenceEnded();
    //
    // *** END CRITICAL SECTION ***
    //
  }

  void PlexilExec::performAssignments() 
  {
    for(std::vector<AssignmentId>::iterator it = m_assignmentsToExecute.begin();
	it != m_assignmentsToExecute.end(); ++it) {
	  AssignmentId assn = *it;
	  check_error(assn.isValid());
	  assn->execute();
	  if (m_listener.isValid())
		m_listener->notifyOfAssignment(assn->getDest(), assn->getDestName(), assn->getValue());
	}
    m_assignmentsToExecute.clear();
  }

  //essential algorithm (given Nodes a and b) :
  //if conflicts(a, b)
  // if a.state == EXECUTING
  //  put b on pending list
  //  return a
  // else if b.state == EXECUTING
  //  put a on pending list
  //  return b
  // else if a.priority < b.priority
  //  put b on pending list
  //  return a
  // else if a.priority > b.priority
  //  put a on pending list
  //  return b
  // else
  //  error
  void PlexilExec::resolveResourceConflicts() {
    for(VariableConflictMap::iterator it = m_resourceConflicts.begin();	it != m_resourceConflicts.end(); ++it) {
      VariableConflictSet& conflictSet = it->second;
      checkError(!conflictSet.empty(),
		 "Resource conflict set for " << it->first->toString() << " is empty.");

      //we only have to look at all the nodes with the highest priority
      VariableConflictSet::iterator conflictIt = conflictSet.begin(); 
      size_t count = conflictSet.count(*conflictIt);

      NodeId nodeToExecute;

      // Look at the destination states of all the nodes with equal priority
      for (size_t i = 0, conflictCounter=0; i < count; ++i, ++conflictIt)
	{
	  NodeId node = *conflictIt;
      
	  check_error(node.isValid());

	  // Found one that is scheduled to for execution
	  if (node->getDestState() == EXECUTING_STATE)
	    ++conflictCounter;
	  else 
		checkError(node->getState() == EXECUTING_STATE,
				   "Error: node '" << node->getNodeId().toString() <<
				   " is neither executing nor is it eligible to do so, yet it is in the " <<
				   "conflict map.");

	  // If more than one node is scheduled for execution, we have a resource contention.
	  checkError(conflictCounter < 2,
				 "Error: nodes '" << node->getNodeId().toString() << "' and '"
				 << nodeToExecute->getNodeId().toString() << "' are in contention over variable "
				 << node->getAssignmentVariable()->getBaseVariable()->toString() << " and have equal priority.");
	  nodeToExecute = node;
	}

      if(nodeToExecute->getDestState() == EXECUTING_STATE && inQueue(nodeToExecute) == -1) {
	debugMsg("PlexilExec:resolveResourceConflicts",
		 "Node '" << nodeToExecute->getNodeId().toString() <<
		 "' has best priority.  Adding it to be executed in position " << m_queuePos);
	m_stateChangeQueue[m_queuePos++] = NodeTransition(nodeToExecute, EXECUTING_STATE);
      }
      else {
	condDebugMsg(nodeToExecute->getState() == EXECUTING_STATE,
		     "PlexilExec:resolveResourceConflicts",
		     "Node for " << it->first << " already executing.  Nothing to resolve.");
	condDebugMsg(inQueue(nodeToExecute) != -1,
		     "PlexilExec:resolveResourceConflicts",
		     "Node for " << it->first << " already in state change queue.  Nothing " <<
		     "to resolve.");
	continue;
      }
    }
  }

  //   const ExpressionId& PlexilExec::findVariable(const LabelStr& name) {
  //     debugMsg("PlexilExec:findVariable",
  // 	     "Searching for variable '" << name.toString() << "' in the entire plan.");
  //     for(std::list<NodeId>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
  //       NodeId node = *it;
  //       check_error(node.isValid());
  //       debugMsg("PlexilExec:findVariable",
  // 	       "Searching for variable in plan tree rooted at " <<
  // 	       node->getNodeId().toString());
  //       const ExpressionId& var = (*it)->findVariable(name, false);
  //       if(var.isId())
  // 	return var;
  //     }
  //     return ExpressionId::noId();
  //   }

  std::string PlexilExec::stateChangeQueueStr() {
    std::ostringstream retval;
    for(StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
	it != m_stateChangeQueue.end(); ++it) {
      check_error(it->second.node.isValid());
      retval << "(" << it->first << ")'" << it->second.node->getNodeId().toString() << "' ";
    }
    return retval.str();
  }

  int PlexilExec::inQueue(const NodeId node) const {
    for(StateChangeQueue::const_iterator it = m_stateChangeQueue.begin();
	it != m_stateChangeQueue.end(); ++it)
      if(it->second.node == node)
	return it->first;
    return -1;
  }

  void PlexilExec::publishCommandReturn(const ExpressionId& dest,
										const std::string& destName,
										const double& value)
  {
	if (m_listener.isValid())
	  m_listener->notifyOfAssignment(dest, destName, value);
  }

  // Convenience method for backward compatibility
  void PlexilExec::addListener(const ExecListenerBaseId& listener) 
  {
    check_error(m_listener.isValid());
    m_listener->addListener(listener);
  }

  // Convenience method for backward compatibility
  void PlexilExec::removeListener(const ExecListenerBaseId& listener) 
  {
    check_error(m_listener.isValid());
    m_listener->removeListener(listener);
  }
}
