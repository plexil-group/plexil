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

#ifndef _H_NodeStateManager
#define _H_NodeStateManager

#include "LabelStr.hh"
#include "ExecDefs.hh"

#include <map>
#include <set>

namespace PLEXIL {

  class StateComputer;
  typedef Id<StateComputer> StateComputerId;

  class TransitionHandler;
  typedef Id<TransitionHandler> TransitionHandlerId;

  class NodeStateManager;
  typedef Id<NodeStateManager> NodeStateManagerId;

  class StateComputer {
  public:
    StateComputer() : m_id(this) {}
    virtual ~StateComputer() {m_id.remove();}
    const StateComputerId& getId() const {return m_id;}
    virtual const LabelStr& getDestState(NodeId& node) = 0;
  protected:
  private:
    StateComputerId m_id;
  };

  class TransitionHandler {
  public:
    TransitionHandler() : m_id(this) {}
    virtual ~TransitionHandler(){m_id.remove();}
    const TransitionHandlerId& getId() const {return m_id;}
    /**
     * @brief Handle the node exiting this state.
     */
    virtual void transitionFrom(NodeId& node, const LabelStr& destState) = 0;
    /**
     * @brief Handle the node entering this state.
     */
    virtual void transitionTo(NodeId& node, const LabelStr& destState) = 0;
  protected:
    static void activatePair(NodeId& node, const LabelStr& name);
    static void deactivatePair(NodeId& node, const LabelStr& name);
    static void deactivateExecutable(NodeId& node);
    static void handleExecution(NodeId& node);
    static void handleReset(NodeId& node);
    static void handleAbort(NodeId& node);
    static bool checkConditions(const NodeId& node, const std::set<double>& active);
  private:
    TransitionHandlerId m_id;
  };

  class NodeStateManager {
  public:
    static void registerStateManager(const LabelStr& nodeType, const NodeStateManagerId manager);
    static NodeStateManagerId& getStateManager(const LabelStr& nodeType);

    NodeStateManager();
    ~NodeStateManager();
    const NodeStateManagerId& getId() const {return m_id;}
    const LabelStr& getDestState(NodeId& node);
    bool canTransition(NodeId& node);
    void transition(NodeId& node);
    void addStateComputer(const LabelStr& fromState, const StateComputerId& cmp);
    void addTransitionHandler(const LabelStr& fromState, const TransitionHandlerId& trans);

  private:
    NodeStateManagerId m_id;
    std::map<double, StateComputerId> m_stateComputers;
    std::map<double, TransitionHandlerId> m_transitionHandlers;

    static std::map<double, NodeStateManagerId>& registeredManagers();
  };

  class StateComputerError : public StateComputer {
  public:
    StateComputerError() : StateComputer() {}
    const LabelStr& getDestState(NodeId& node);
  };

  class TransitionHandlerError : public TransitionHandler {
  public:
    TransitionHandlerError() : TransitionHandler() {}
    void transitionTo(NodeId& node, const LabelStr& destState);
    void transitionFrom(NodeId& node, const LabelStr& destState);
  };

#define REGISTER_STATE_MANAGER(CLASS, TYPE) {NodeStateManager::registerStateManager(LabelStr(#TYPE), (new CLASS())->getId());}
}

#endif
