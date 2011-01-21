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

#include "NodeStateManager.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Node.hh"
#include "Utils.hh"

namespace PLEXIL {

  std::map<double, NodeStateManagerId>& NodeStateManager::registeredManagers() 
  {
    static std::map<double, NodeStateManagerId>* sl_map = NULL;
    if (sl_map == NULL)
      sl_map = new std::map<double, NodeStateManagerId>();

    return *sl_map;
  }

  void TransitionHandler::handleExecution(NodeId& node) {node->execute();}
  void TransitionHandler::handleReset(NodeId& node) {node->reset();}
  void TransitionHandler::handleAbort(NodeId& node) {node->abort();}
  void TransitionHandler::deactivateExecutable(NodeId& node) {node->deactivateExecutable();}

  bool TransitionHandler::checkConditions(const NodeId& /* node */,
					  const std::set<double>& active) {
    std::vector<double> inactive(Node::ALL_CONDITIONS().size() - active.size());
    // *** FIXME: FINISH THIS ***
    return true;
  }

  void NodeStateManager::registerStateManager(const LabelStr& nodeType, const NodeStateManagerId manager) {
    check_error(manager.isValid());
    checkError(registeredManagers().find(nodeType) == registeredManagers().end(), "State manager already registered for node type '" << nodeType.toString() << "'");
    registeredManagers().insert(std::pair<double, NodeStateManagerId>(nodeType, manager));
  }

  NodeStateManagerId& NodeStateManager::getStateManager(const LabelStr& nodeType) {
    checkError(registeredManagers().find(nodeType) != registeredManagers().end(), "No state manager registered for node type '" << nodeType.toString() << "'");
    return registeredManagers().find(nodeType)->second;
  }

  NodeStateManager::NodeStateManager()
    : m_id(this) 
  {
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      m_stateComputers[s] = (new StateComputerError())->getId();
      m_transitionHandlers[s] = (new TransitionHandlerError())->getId();
    }
  }

  NodeStateManager::~NodeStateManager() {
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      StateComputerId cmp = m_stateComputers[s];
      m_stateComputers[s] = StateComputerId::noId();
      delete (StateComputer*) cmp;
    }
    for (size_t s = INACTIVE_STATE; s < NODE_STATE_MAX; s++) {
      TransitionHandlerId cmp = m_transitionHandlers[s];
      m_transitionHandlers[s] = TransitionHandlerId::noId();
      delete (TransitionHandler*) cmp;
    }
    m_id.remove();
  }

  NodeState NodeStateManager::getDestState(NodeId& node) {
    check_error(node.isValid());
    const StateComputerId& comp = m_stateComputers[node->getState()];
    checkError(comp.isValid(),
	       "Invalid state computer for node '" << node->getNodeId().toString() << " for state " << node->getStateName().toString());
    return comp->getDestState(node);
  }

  // This method is currently used only by exec-test-module.
  // Its logic has been absorbed into transition() below to avoid redundant calls to getDestState().
  bool NodeStateManager::canTransition(NodeId& node) {
    check_error(node.isValid());
    NodeState toState(getDestState(node));
    return toState != NO_NODE_STATE && toState != node->getState();
  }

  // Inline canTransition() to save a bunch of work.
  // Can skip the NodeId valid check because getDestState() does it for us.
  void NodeStateManager::transition(NodeId& node) {
    NodeState destState(getDestState(node));
    checkError(destState != NO_NODE_STATE
	       && destState != node->getState(),
	       "Attempted to transition node " << node->getNodeId().toString() <<
	       " when it is ineligible.");

    const TransitionHandlerId& fromHandler = m_transitionHandlers[node->getState()];
    checkError(fromHandler.isValid(),
	       "Invalid transition handler for node " << node->getNodeId().toString() <<
	       " from state " << node->getStateName().toString());

    const TransitionHandlerId& toHandler = m_transitionHandlers[destState];
    checkError(toHandler.isValid(),
	       "Invalid transition handler for node " << node->getNodeId().toString() <<
	       " to state " << StateVariable::nodeStateName(destState).toString());

    debugMsg("NodeStateManager:transition",
	     "(" << getId() << ")" << node->getNodeId().toString() << ": " <<
	     node->getStateName().toString() << " -> " << StateVariable::nodeStateName(destState).toString());
    fromHandler->transitionFrom(node, destState);
    toHandler->transitionTo(node, destState);
  }

  void NodeStateManager::addStateComputer(NodeState fromState,
					  const StateComputerId& cmp) {
    check_error(cmp.isValid());
    check_error(fromState < NODE_STATE_MAX);
    
    StateComputerId oldCmp = m_stateComputers[fromState];
    if (oldCmp.isId()) {
      debugMsg("NodeStateManager:addStateComputer", 
	       getId() << " Replacing state computer for state "
	       << StateVariable::nodeStateName(fromState).toString());
      delete (StateComputer*) oldCmp;
      m_stateComputers[fromState] = cmp;
    }
    else {
      debugMsg("NodeStateManager:addStateComputer", 
	       getId() << " Adding new state computer for state "
	       << StateVariable::nodeStateName(fromState).toString());
      m_stateComputers[fromState] = cmp;
    }
  }

  void NodeStateManager::addTransitionHandler(NodeState fromState, 
					      const TransitionHandlerId& trans) {
    check_error(trans.isValid());
    check_error(fromState < NODE_STATE_MAX);

    TransitionHandlerId oldTrans = m_transitionHandlers[fromState];
    if (oldTrans.isId()) {
      debugMsg("NodeStateManager:addTransitionHandler", getId()
	       << " Replacing transition handler for state " << StateVariable::nodeStateName(fromState).toString());
      delete (TransitionHandler*) oldTrans;
      m_transitionHandlers[fromState] = trans;
    }
    else {
      debugMsg("NodeStateManager:addTransitionHandler", getId()
	       << " Adding new transition handler for state " << StateVariable::nodeStateName(fromState).toString());
      m_transitionHandlers[fromState] = trans;
    }
  }

  NodeState StateComputerError::getDestState(NodeId& node) {
    checkError(ALWAYS_FAIL, "Attempted to compute destination state for node " << node->getNodeId().toString() << " of type " << node->getType());
    return NO_NODE_STATE;
  }

  void TransitionHandlerError::transitionTo(NodeId& node, NodeState /* destState */) {
    checkError(ALWAYS_FAIL,
	       "Attempted to transition node '" << node->getNodeId().toString() <<
	       "' of type " << node->getType().toString());
  }

  void TransitionHandlerError::transitionFrom(NodeId& node, NodeState /* destState */) {
    checkError(ALWAYS_FAIL,
	       "Attempted to transition node '" << node->getNodeId().toString() <<
	       "' of type " << node->getType().toString());
  }
}
