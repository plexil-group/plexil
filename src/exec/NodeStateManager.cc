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

//   void StateComputer::activatePair(NodeId& node, const LabelStr& name) {node->activatePair(name);}
//   void StateComputer::deactivatePair(NodeId& node, const LabelStr& name) {node->deactivatePair(name);}

  void TransitionHandler::activatePair(NodeId& node, const LabelStr& name) {node->activatePair(name);}
  void TransitionHandler::deactivatePair(NodeId& node, const LabelStr& name) {node->deactivatePair(name);}
  void TransitionHandler::handleExecution(NodeId& node) {node->execute();}
  void TransitionHandler::handleReset(NodeId& node) {node->reset();}
  void TransitionHandler::handleAbort(NodeId& node) {node->abort();}
  void TransitionHandler::deactivateExecutable(NodeId& node) {node->deactivateExecutable();}

  bool TransitionHandler::checkConditions(const NodeId& /* node */,
					  const std::set<double>& active) {
    std::vector<double> inactive(Node::ALL_CONDITIONS().size() - active.size());
    //FINISH THIS
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

  NodeStateManager::NodeStateManager() : m_id(this) {
    for(std::set<double>::const_iterator it = StateVariable::ALL_STATES().begin(); it != StateVariable::ALL_STATES().end(); ++it) {
      addStateComputer(*it, (new StateComputerError())->getId());
      addTransitionHandler(*it, (new TransitionHandlerError())->getId());
    }
  }

  NodeStateManager::~NodeStateManager() {
    cleanup(m_stateComputers);
    cleanup(m_transitionHandlers);
    m_id.remove();
  }

  const LabelStr& NodeStateManager::getDestState(NodeId& node) {
    check_error(node.isValid());
    std::map<double, StateComputerId>::iterator it = m_stateComputers.find(node->getState());
    checkError(it != m_stateComputers.end(),
	       "No state computer for node '" << node->getNodeId().toString() << "' from state '" << node->getState().toString() << "'");
    checkError(it->second.isValid(),
	       "Invalid state computer for node '" << node->getNodeId().toString() << " for state " << node->getState().toString());
    return it->second->getDestState(node);
  }

  // This method is currently used only by exec-test-module.
  // Its logic has been absorbed into transition() below to avoid redundant calls to getDestState().
  bool NodeStateManager::canTransition(NodeId& node) {
    check_error(node.isValid());
    LabelStr toState = getDestState(node);
    return toState != StateVariable::UNKNOWN() && toState != StateVariable::NO_STATE() && toState != node->getState();
  }

  // Inline canTransition() to save a bunch of work.
  // Can skip the NodeId valid check because getDestState() does it for us.
  void NodeStateManager::transition(NodeId& node) {
    LabelStr destState = getDestState(node);
    checkError(destState != StateVariable::UNKNOWN()
	       && destState != StateVariable::NO_STATE()
	       && destState != node->getState(),
	       "Attempted to transition node " << node->getNodeId().toString() <<
	       " when it is ineligible.");

    std::map<double, TransitionHandlerId>::iterator fromIt =
      m_transitionHandlers.find(node->getState());
    checkError(fromIt != m_transitionHandlers.end(),
	       "No transition handler for node " << node->getNodeId().toString() <<
	       " from state " << node->getState().toString());
    checkError(fromIt->second.isValid(),
	       "Invalid transition handler for node " << node->getNodeId().toString() <<
	       " from state " << node->getState().toString());

    std::map<double, TransitionHandlerId>::iterator toIt =
      m_transitionHandlers.find(destState);
    checkError(toIt != m_transitionHandlers.end(),
	       "No transition handler for node " << node->getNodeId().toString() <<
	       " to state " << destState.toString());
    checkError(fromIt->second.isValid(),
	       "Invalid transition handler for node " << node->getNodeId().toString() <<
	       " to state " << destState.toString());

    debugMsg("NodeStateManager:transition",
	     "(" << getId() << ")" << node->getNodeId().toString() << ": " <<
	     node->getState().toString() << " -> " << destState.toString());
    fromIt->second->transitionFrom(node, destState);
    toIt->second->transitionTo(node, destState);
  }

  void NodeStateManager::addStateComputer(const LabelStr& fromState,
					  const StateComputerId& cmp) {
    check_error(cmp.isValid());
    std::map<double, StateComputerId>::iterator it = m_stateComputers.find(fromState);
    if(it != m_stateComputers.end()) {
      debugMsg("NodeStateManager:addStateComputer", 
	       getId() << " Replacing state computer for state " << fromState.toString());
      delete (StateComputer*) it->second;
      it->second = cmp;
    }
    else {
      debugMsg("NodeStateManager:addStateComputer", 
	       getId() << " Adding new state computer for state " << fromState.toString());
      m_stateComputers.insert(std::pair<double, StateComputerId>(fromState, cmp));
    }
  }

  void NodeStateManager::addTransitionHandler(const LabelStr& fromState, 
					      const TransitionHandlerId& trans) {
    check_error(trans.isValid());
    std::map<double, TransitionHandlerId>::iterator it = m_transitionHandlers.find(fromState);
    if(it != m_transitionHandlers.end()) {
      debugMsg("NodeStateManager:addTransitionHandler", getId() << " Replacing transition handler for state " << fromState.toString());
      delete (TransitionHandler*) it->second;
      it->second = trans;
    }
    else {
      debugMsg("NodeStateManager:addTransitionHandler", getId() << " Adding new transition handler for state " << fromState.toString());
      m_transitionHandlers.insert(std::pair<double, TransitionHandlerId>(fromState, trans));
    }
  }

  const LabelStr& StateComputerError::getDestState(NodeId& node) {
    checkError(ALWAYS_FAIL, "Attempted to compute destination state for node " << node->getNodeId().toString() << " of type " << node->getType());
    return StateVariable::UNKNOWN_STR();
  }

  void TransitionHandlerError::transitionTo(NodeId& node, const LabelStr& /* destState */) {
    checkError(ALWAYS_FAIL,
	       "Attempted to transition node '" << node->getNodeId().toString() <<
	       "' of type " << node->getType().toString());
  }

  void TransitionHandlerError::transitionFrom(NodeId& node, const LabelStr& /* destState */) {
    checkError(ALWAYS_FAIL,
	       "Attempted to transition node '" << node->getNodeId().toString() <<
	       "' of type " << node->getType().toString());
  }
}
