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
#include "ExecListener.hh"
#include "Node.hh"
#include "Expression.hh"
#include "CoreExpressions.hh"
#include "ExternalInterface.hh"
#include "Debug.hh"
#include "StateCache.hh"

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

  class RealExecConnector : public ExecConnector 
  {
  public:
    RealExecConnector(const PlexilExecId& exec) 
      : m_exec(exec)
    {}

    //
    // Methods simply forward to real PlexilExec
    //

    void handleConditionsChanged(const NodeId& node) 
    {
      m_exec->handleConditionsChanged(node);
    }

    void handleNeedsExecution(const NodeId& node) 
    {
      m_exec->handleNeedsExecution(node);
    }
    
    //const ExpressionId& findVariable(const LabelStr& name) {return m_exec->findVariable(name);}

    const StateCacheId& getStateCache() 
    {
      return m_exec->getStateCache();
    }

    const ExternalInterfaceId& getExternalInterface() 
    { 
      return m_exec->getExternalInterface(); 
    }

  private:
    PlexilExecId m_exec;
  };

  PlexilExec::PlexilExec(PlexilNodeId& plan)
    : m_id(this), m_cycleNum(0), m_queuePos(1),
      m_connector((new RealExecConnector(m_id))->getId()),
      m_cache((new StateCache())->getId()) {
    addPlan(plan, EMPTY_LABEL());
    //is it really this simple?
  }

  PlexilExec::PlexilExec()
    : m_id(this), m_cycleNum(0), m_queuePos(1),
      m_connector((new RealExecConnector(m_id))->getId()),
      m_cache((new StateCache())->getId())
  {}

  PlexilExec::~PlexilExec() {
    for(std::list<NodeId>::iterator it = m_plan.begin(); it != m_plan.end(); ++it)
      delete (Node*) (*it);
    delete (StateCache*) m_cache;
    m_id.remove();
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
  PlexilNodeId PlexilExec::getLibrary(const std::string& nodeName) const
  {
    checkError(!nodeName.empty(),
	       "PlexilExec::getLibrary: Node name is empty");
    for (std::vector<PlexilNodeId>::const_iterator it = m_libraries.begin();
		 it != m_libraries.end();
		 it++) {
      if (nodeName == (*it)->nodeId())
		return *it;
    }
    return PlexilNodeId::noId();
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
    checkError(!libNode->nodeId().empty(),
	       "Library node must have non-null node ID");
    m_libraries.push_back(libNode);
    debugMsg("PlexilExec:addLibrary",
	     "Added library node " << libNode->nodeId());
    publishAddLibrary(libNode);
  }

  // Add a plan

  bool PlexilExec::addPlan(PlexilNodeId& plan, const LabelStr& parent) {
    //currently parent is ignored!
    if (!plan->link(m_libraries)) {
      debugMsg("PlexilExec:addPlan", " library linking failed");
      return false;
    }

    // after this point any failures are likely to be fatal!
    //not actually quiesceing, but causing the new nodes to look at the current known world state
    m_cache->handleQuiescenceStarted();
    clock_t time1 = clock();
    NodeId root = (new Node(plan, m_connector))->getId();
    check_error(root.isValid());
    m_plan.push_back(root);
    root->postInit();
    debugMsg("PlexilExec:addPlan",
         "Added plan: " << std::endl << root->toString());
    debugMsg("Time", "Time to initialize plan: " << clock() - time1);
    publishAddPlan(plan, parent);
    root->checkConditions();
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

  //as a possible optimization, if we spend a lot of time searching through this list,
  //it should be faster to search the list backwards.
  void PlexilExec::handleConditionsChanged(const NodeId node) {
    check_error(node.isValid());
    debugMsg("PlexilExec:handleConditionsChanged",
	     "Node " << node->getNodeId().toString() << " had a relevant condition change.");

    NodeState destState = node->getDestState();
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
      m_stateChangeQueue.insert(std::pair<unsigned int, NodeId>(m_queuePos++, node));
    }
  }

  void PlexilExec::handleNeedsExecution(const NodeId node) {
    checkError(node->getState() == EXECUTING_STATE,
	       "Executive told to handle execution for node '" <<
	       node->getNodeId().toString() << "', but it's in state '" <<
	       node->getStateName().toString() << "'");
    debugMsg("PlexilExec:handleNeedsExecution",
	     "Storing action for node '" << node->getNodeId().toString() <<
	     "' of type '" << node->getType().toString() << 
             "' to be executed.");
    if(node->getType() == Node::ASSIGNMENT())
      m_assignmentsToExecute.push_back(node->getAssignment());
    else if(node->getType() == Node::COMMAND())
      m_commandsToExecute.push_back(node->getCommand());
    else if(node->getType() == Node::UPDATE())
      m_updatesToExecute.push_back(node->getUpdate());
  }

  void PlexilExec::removeFromResourceContention(const NodeId node) {
    check_error(node->getType() == Node::ASSIGNMENT());
    //this check may be too restrictive, since we'll probably use this method
    //to remove nodes when they get batch-executed.
    //check_error(node->getDestState() != EXECUTING_STATE);

    ExpressionId exp = node->getAssignmentVariable();

    check_error(exp.isValid());
    std::map<ExpressionId, std::multiset<NodeId, NodeConflictComparator> >::iterator conflict =
      m_resourceConflicts.find(exp);
    if(conflict == m_resourceConflicts.end())
      return;

    std::multiset<NodeId, NodeConflictComparator>& conflictSet = conflict->second;
    std::multiset<NodeId, NodeConflictComparator>::const_iterator conflictSetIter;

    if((conflictSetIter = conflictSet.find(node)) != conflictSet.end())
      {
        size_t count = conflictSet.count(node);
        //        debugMsg("PlexilExec:removeFromResourceContention", "There are " << count << " similar items in the"
        //                 << " set.");
        bool found = false;
        
        for (size_t i = 0; !found && (i < count); ++i, ++conflictSetIter)
          {
            if (node == *conflictSetIter)
              found = true;
          }
        if (found)
          {
            conflictSet.erase(node);
          }
      }
    if(conflictSet.empty())
      {
        m_resourceConflicts.erase(exp);
      }
  }

  void PlexilExec::addToResourceContention(const NodeId node) {
    check_error(node->getType() == Node::ASSIGNMENT());
    check_error(node->getDestState() == EXECUTING_STATE);

    ExpressionId exp = node->getAssignmentVariable();
    check_error(exp.isValid());

    debugMsg("PlexilExec:addToResourceContention",
	     "Adding node '" << node->getNodeId().toString() << "' to resource contention.");
    std::map<ExpressionId, std::multiset<NodeId, NodeConflictComparator> >::iterator resourceIt =
      m_resourceConflicts.find(exp);
    if(resourceIt == m_resourceConflicts.end()) {
      m_resourceConflicts.insert(std::make_pair(exp,
						std::multiset<NodeId, NodeConflictComparator>()));
      resourceIt = m_resourceConflicts.find(exp);
    }
    std::multiset<NodeId, NodeConflictComparator>& conflictSet = resourceIt->second;

    std::multiset<NodeId, NodeConflictComparator>::iterator conflictSetIter;

    //HACK.  maybe.  maybe not.
    if((conflictSetIter = conflictSet.find(node)) != conflictSet.end()) {

      // There could be multiple nodes with the same priority,
      size_t count = conflictSet.count(node);
      debugMsg("PlexilExec:addToResourceContention", "There are " << count << " similar items in the"
               << " set.");
      bool found = false;
      
      for (size_t i = 0; !found && (i < count); ++i, ++conflictSetIter)
        {
          if (node == *conflictSetIter)
            found = true;
        }
      if (found)
        {
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
    while(true) {
      resolveResourceConflicts();
      if(m_stateChangeQueue.empty())
	break;
      std::list<NodeId> transitioningNodes;
      std::transform(m_stateChangeQueue.begin(), m_stateChangeQueue.end(),
		     std::back_insert_iterator<std::list<NodeId> >(transitioningNodes),
		     select2nd<std::map<unsigned int, NodeId>::value_type>());
      m_stateChangeQueue.clear();

      std::ostringstream out;
      for(std::list<NodeId>::const_iterator it = transitioningNodes.begin();
	  it != transitioningNodes.end(); ++it) {
	out << (*it)->getNodeId().toString() << " ";
      }
      debugMsg("PlexilExec:step",
	       "[" << m_cycleNum << ":" << stepCount << "] State change queue: " << out.str());

      unsigned int microStepCount = 0;
      std::set<NodeId> transitionedNodes;

      //lock the nodes into their current transitioning states
      for(std::list<NodeId>::iterator it = transitioningNodes.begin();
	  it != transitioningNodes.end(); ++it) {
	NodeId node = *it;
	check_error(node.isValid());
	debugMsg("PlexilExec:lock",
		 "Locking node " << node->getNodeId().toString());
	node->lockConditions();
      }
      //transition them
      for(std::list<NodeId>::iterator it = transitioningNodes.begin();
	  it != transitioningNodes.end(); ++it) {
	NodeId node = *it;
	check_error(node.isValid());
	checkError(transitionedNodes.find(node) == transitionedNodes.end(),
		   "Node " << node->getNodeId().toString() <<
		   " already transitioned in this step.");
	debugMsg("PlexilExec:step",
		 "[" << m_cycleNum << ":" << stepCount << ":" << microStepCount <<
		 "] Transitioning node " << node->getNodeId().toString());
	NodeState oldState = node->getState();
	node->transition(quiescenceTime);
	publishTransition(oldState, node);
	transitionedNodes.insert(node);
	++microStepCount;
      }
      //unlock the nodes
      for(std::list<NodeId>::iterator it = transitioningNodes.begin();
	  it != transitioningNodes.end(); ++it) {
	NodeId node = *it;
	check_error(node.isValid());
	debugMsg("PlexilExec:unlock",
		 "Unlocking node " << node->getNodeId().toString());
	node->unlockConditions();
      }

      performAssignments();
      ++stepCount;
    }

    std::list<CommandId> commands(m_commandsToExecute.begin(), m_commandsToExecute.end());
    m_commandsToExecute.clear();
    getExternalInterface()->batchActions(commands);

    std::list<UpdateId> updates(m_updatesToExecute.begin(), m_updatesToExecute.end());
    m_updatesToExecute.clear();
    getExternalInterface()->updatePlanner(updates);

    debugMsg("PlexilExec:cycle", "==>End cycle " << m_cycleNum);
    for(std::list<NodeId>::const_iterator it = m_plan.begin(); it != m_plan.end(); ++it) {
      debugMsg("PlexilExec:printPlan", std::endl << (*it)->toString());
    }
    m_cache->handleQuiescenceEnded();
    //
    // *** END CRITICAL SECTION ***
    //
  }


  void PlexilExec::quiescenceLoop(const int depth, const unsigned int stop,
				  const double quiescenceTime) {
    resolveResourceConflicts();
    int stepNum = 0;
    if(!m_stateChangeQueue.empty() && m_stateChangeQueue.rbegin()->first > stop) {
      unsigned int nextStop = m_stateChangeQueue.rbegin()->first;
      std::set<NodeId> transitionedNodes;
      debugMsg("PlexilExec:step",
	       "[" << m_cycleNum << ":" << depth << "] Starting with stop index " << stop <<
	       " and state change queue: " << stateChangeQueueStr());
      std::map<unsigned int, NodeId>::iterator it = m_stateChangeQueue.end();
      --it;

      while(!m_stateChangeQueue.empty() && it->first > stop) {
	unsigned int pos = it->first;
	NodeId node = it->second;
	check_error(node.isValid());
	checkError(transitionedNodes.find(it->second) == transitionedNodes.end(),
		   "Node '" << node->getNodeId().toString() << "' (" << pos <<
		   ") already transitioned in this loop.");
	bool emptied = (it == m_stateChangeQueue.begin());
	if(!emptied)
	  --it;
	debugMsg("PlexilExec:step", "Erasing node at position: " << pos);
	m_stateChangeQueue.erase(pos);
	debugMsg("PlexilExec:step",
		 "[" << m_cycleNum << ":" << depth << ":" << stepNum << "]" <<
		 " Transitioning node '" << node->getNodeId().toString() << "' (" << pos <<
		 ")");
	NodeState oldState = node->getState();
	node->transition(quiescenceTime);
	publishTransition(oldState, node);
	transitionedNodes.insert(node);
	resolveResourceConflicts();
	stepNum++;
	if(emptied)
	  break;
      }
      performAssignments();
      quiescenceLoop(depth + 1, nextStop, quiescenceTime);
    }
  }

  void PlexilExec::performAssignments() 
  {
    for(std::vector<AssignmentId>::iterator it = m_assignmentsToExecute.begin();
	it != m_assignmentsToExecute.end(); ++it) 
      {
	AssignmentId assn = *it;
	check_error(assn.isValid());
	ExpressionId exp = assn->getDest();
	check_error(exp.isValid());
	double value = assn->getValue();
	debugMsg("Test:testOutput", "Assigning '" << assn->getDestName() <<
		 "' (" << exp->toString() << ") to " << Expression::valueToString(value));
	exp->setValue(value);
        publishAssignment(exp, assn->getDestName(), value);
	assn->getAck()->setValue(1);
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
    for(std::map<ExpressionId, std::multiset<NodeId, NodeConflictComparator> >::iterator it =
	  m_resourceConflicts.begin();	it != m_resourceConflicts.end(); ++it) {
      std::multiset<NodeId, NodeConflictComparator>& conflictSet = it->second;
      checkError(!conflictSet.empty(),
		 "Resource conflict set for " << it->first->toString() << " is empty.");

      //we only have to look at all the nodes with the highest priority
      std::multiset<NodeId, NodeConflictComparator>::iterator conflictIt = conflictSet.begin(); 
      size_t count = conflictSet.count(*conflictIt);

      NodeId nodeToExecute;

      // Look at the destination states of all the nodes with equal priority
      for (size_t i = 0, conflictCounter=0; i < count; ++i, ++conflictIt)
	{
	  NodeId node = *conflictIt;
      
	  check_error(node.isValid());
	  checkError(node->getState() == EXECUTING_STATE ||
		     node->getDestState() == EXECUTING_STATE,
		     "Error: node '" << node->getNodeId().toString() <<
		     " is neither executing nor is it eligible to do so, yet it is in the " <<
		     "conflict map.");

	  // Found one that is scheduled to for execution
	  if (node->getDestState() == EXECUTING_STATE)
	    ++conflictCounter;

	  // If more than one node is sceduled for execution, we have a resource contention.
	  checkError(conflictCounter < 2,
		     "Error: node '" << node->getNodeId().toString() << " and the node "
		     << nodeToExecute->getNodeId().toString() << " are in contention over variable "
		     << node->getAssignmentVariable()->toString() << " and have equal priority.");
	  nodeToExecute = node;
	}

      if(nodeToExecute->getDestState() == EXECUTING_STATE && inQueue(nodeToExecute) == -1) {
	debugMsg("PlexilExec:resolveResourceConflicts",
		 "Node '" << nodeToExecute->getNodeId().toString() <<
		 "' has best priority.  Adding it to be executed in position " << m_queuePos);
	m_stateChangeQueue.insert(std::pair<unsigned int, NodeId>(m_queuePos++, nodeToExecute));
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
    for(std::map<unsigned int, NodeId>::const_iterator it = m_stateChangeQueue.begin();
	it != m_stateChangeQueue.end(); ++it) {
      check_error(it->second.isValid());
      retval << "(" << it->first << ")'" << it->second->getNodeId().toString() << "' ";
    }
    return retval.str();
  }

  int PlexilExec::inQueue(const NodeId node) const {
    for(std::map<unsigned int, NodeId>::const_iterator it = m_stateChangeQueue.begin();
	it != m_stateChangeQueue.end(); ++it)
      if(it->second == node)
	return it->first;
    return -1;
  }

  void PlexilExec::publishTransition(NodeState prevState, const NodeId& node) {
    for(std::vector<ExecListenerId>::iterator it = m_listeners.begin(); it != m_listeners.end();
	++it) {
      ExecListenerId listener = *it;
      check_error(listener.isValid());
      (*it)->notifyOfTransition(prevState, node);
    }
  }

  void PlexilExec::publishAddLibrary(const PlexilNodeId& libNode) 
  {
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
	 it != m_listeners.end();
	 ++it)
      {
	ExecListenerId listener = *it;
	check_error(listener.isValid());
	(*it)->notifyOfAddLibrary(libNode);
      }
  }

  void PlexilExec::publishAddPlan(const PlexilNodeId& plan, const LabelStr& parent) 
  {
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
	 it != m_listeners.end();
	 ++it)
      {
	ExecListenerId listener = *it;
	check_error(listener.isValid());
	(*it)->notifyOfAddPlan(plan, parent);
      }
  }

  void PlexilExec::publishCommandReturn (const ExpressionId& dest,
                                         const std::string& destName,
                                         const double& value)
  {
    publishAssignment (dest, destName, value);
  }


  void PlexilExec::publishAssignment(const ExpressionId & dest,
                                     const std::string& destName,
                                     const double& value)
  {
    for (std::vector<ExecListenerId>::iterator it = m_listeners.begin();
	 it != m_listeners.end();
	 ++it)
      {
	ExecListenerId listener = *it;
	check_error(listener.isValid());
	(*it)->notifyOfAssignment(dest, destName, value);
      }
  }

  void PlexilExec::addListener(const ExecListenerId& listener) {
    check_error(std::find(m_listeners.begin(), m_listeners.end(),
			  listener) == m_listeners.end());
    m_listeners.push_back(listener);
  }

  void PlexilExec::removeListener(const ExecListenerId& listener) {
    std::vector<ExecListenerId>::iterator it =
      std::find(m_listeners.begin(), m_listeners.end(), listener);
    check_error(it != m_listeners.end());
    m_listeners.erase(it);
  }
}
