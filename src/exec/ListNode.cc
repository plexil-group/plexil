/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "ListNode.hh"

#include "BooleanOperators.hh"
#include "Debug.hh"
#include "Error.hh"
#include "Function.hh"
#include "NodeOperatorImpl.hh"

namespace PLEXIL
{

  //
  // Condition operators only used by ListNode
  //

  //! \class AllFinished
  //! \brief A specialized NodeOperator for ListNode which returns true
  //!        when all child nodes are in FINISHED node state.
  //! \see ListNode::specializedCreateConditionWrappers
  //! \ingroup Exec-Core
  class AllFinished : public NodeOperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~AllFinished() = default;

    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(AllFinished);

    //! \brief Calculate the function's value.
    //! \param result Reference to a Boolean variable.
    //! \param node Pointer to the node whose children are to be checked.
    //! \result True if the value is known, false otherwise.
    //! \note The result of this operator is always known.
    bool operator()(Boolean &result, NodeImpl const *node) const override
    {
      for (NodeImplPtr const &child : node->getChildren()) {
        if (child->getState() != FINISHED_STATE) {
          result = false;
          debugMsg("AllFinished", "result = false");
          return true;
        }
      }
      debugMsg("AllFinished", "result = true");
      result = true;
      return true; // always known
    }

    //! \brief Map the operator over the children of the node.
    //! \param node Pointer to a node.
    //! \param oper A functor of one parameter to map over the child nodes.
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const override
    {
      for (NodeImplPtr &child : node->getChildren())
        (oper)(child.get());
    }

  private:

    //! \brief Default constructor.
    //! \note Should only be called from instance() static member function.
    AllFinished()
      : NodeOperatorImpl<Boolean>("AllChildrenFinished")
    {
    }

    // Copy, move constructors, assignment operators not implemented.
    AllFinished(AllFinished const &) = delete;
    AllFinished(AllFinished &&) = delete;
    AllFinished &operator=(AllFinished const &) = delete;
    AllFinished &operator=(AllFinished &&) = delete;

  };

  //! \class AllWaitingOrFinished
  //! \brief A specialized NodeOperator for ListNode which returns true
  //!        when all child nodes are in either WAITING or FINISHED node state.
  //! \see ListNode::specializedCreateConditionWrappers
  //! \ingroup Exec-Core
  //! \ingroup Expression
  class AllWaitingOrFinished : public NodeOperatorImpl<Boolean>
  {
  public:

    //! \brief Virtual destructor.
    virtual ~AllWaitingOrFinished() = default;

    DECLARE_NODE_OPERATOR_STATIC_INSTANCE(AllWaitingOrFinished);

    //! \brief Calculate the function's value.
    //! \param result Reference to a Boolean variable.
    //! \param node Pointer to the node whose children are to be checked.
    //! \result True if the value is known, false otherwise.
    //! \note The result of this operator is always known.
    bool operator()(Boolean &result, NodeImpl const *node) const override
    {
      for (NodeImplPtr const &child : node->getChildren()) {
        switch (child->getState()) {
        case WAITING_STATE:
        case FINISHED_STATE:
          break;

        default:
          result = false;
          debugMsg("AllWaitingOrFinished", " result = false");
          return true;
        }
      }
      result = true;
      debugMsg("AllWaitingOrFinished", " result = true");
      return true; // always known
    }

    //! \brief Map the operator over the children of the node.
    //! \param node Pointer to a node.
    //! \param oper A functor of one parameter to map over the child nodes.
    void doPropagationSources(NodeImpl *node, ListenableUnaryOperator const &oper) const override
    {
      for (NodeImplPtr &child : node->getChildren())
        (oper)(child.get());
    }

  private:

    //! \brief Default constructor.
    //! \note Should only be called from instance() static member function.
    AllWaitingOrFinished()
      : NodeOperatorImpl<Boolean>("AllChildrenWaitingOrFinished")
    {
    }

    // Copy, move constructors, assignment operators not implemented.
    AllWaitingOrFinished(AllWaitingOrFinished const &) = delete;
    AllWaitingOrFinished(AllWaitingOrFinished &&) = delete;
    AllWaitingOrFinished &operator=(AllWaitingOrFinished const &) = delete;
    AllWaitingOrFinished &operator=(AllWaitingOrFinished &&) = delete;
  };

  //
  // ListNode member functions
  //

  ListNode::ListNode(char const *nodeId, NodeImpl *parent)
    : NodeImpl(nodeId, parent),
      m_actionCompleteFn(AllWaitingOrFinished::instance(), this),
      m_allFinishedFn(AllFinished::instance(), this)
  {
  }

  ListNode::ListNode(const std::string& type,
                     const std::string& name, 
                     NodeState state,
                     NodeImpl *parent)
    : NodeImpl(type, name, state, parent),
      m_actionCompleteFn(AllWaitingOrFinished::instance(), this),
      m_allFinishedFn(AllFinished::instance(), this)
  {
    checkError(type == LIST || type == LIBRARYNODECALL,
               "Invalid node type " << type << " for a ListNode");

    switch (m_state) {
    case EXECUTING_STATE:
      deactivatePostCondition();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      m_conditions[ancestorEndIdx]->activate();
      break;

    case FINISHING_STATE:
      activateAncestorExitInvariantConditions();
      activateActionCompleteCondition();
      activateExitCondition();
      activateInvariantCondition();
      activatePostCondition();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      m_conditions[ancestorEndIdx]->activate();
      break;

    case FAILING_STATE:
      activateActionCompleteCondition();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      m_conditions[ancestorEndIdx]->activate();
      break;

    default:
      break;
    }
  }

  NodeVariableMap const *ListNode::getChildVariableMap() const
  {
    if (m_variablesByName)
      return m_variablesByName.get();

    // Search ancestors for first in chain
    NodeImpl *n = m_parent;
    NodeVariableMap const *map = nullptr;
    while (n && !map) {
      map = n->getVariableMap();
      if (!map)
        n = n->m_parent;
    }
    return map;
  }

  // Create the ancestor end, ancestor exit, and ancestor invariant conditions required by children
  // This method is called after all user-spec'd conditions have been instantiated
  void ListNode::specializedCreateConditionWrappers()
  {
    // Not really a "wrapper", but this is best place to add it.
    m_conditions[actionCompleteIdx] = &m_actionCompleteFn;
    m_garbageConditions[actionCompleteIdx] = false;

    if (m_parent) {
      if (getExitCondition()) {
        if (getAncestorExitCondition()) {
          m_conditions[ancestorExitIdx] =
            makeFunction(BooleanOr::instance(),
                         getExitCondition(),
                         getAncestorExitCondition(),
                         false,
                         false);
          m_garbageConditions[ancestorExitIdx] = true;
        }
        else 
          m_conditions[ancestorExitIdx] = getExitCondition();
      }
      else 
        m_conditions[ancestorExitIdx] = getAncestorExitCondition(); // could be null

      if (getInvariantCondition()) {
        if (getAncestorInvariantCondition()) {
          m_conditions[ancestorInvariantIdx] =
            makeFunction(BooleanAnd::instance(),
                         getInvariantCondition(),
                         getAncestorInvariantCondition(), // from parent
                         false,
                         false);
          m_garbageConditions[ancestorInvariantIdx] = true;
        }
        else {
          m_conditions[ancestorInvariantIdx] = getInvariantCondition();
          m_garbageConditions[ancestorInvariantIdx] = false;
        }
      }
      else {
        m_conditions[ancestorInvariantIdx] = getAncestorInvariantCondition(); // could be null
        m_garbageConditions[ancestorInvariantIdx] = false;
      }

      // End is special
      if (getEndCondition()) {
        if (getAncestorEndCondition()) {
          m_conditions[ancestorEndIdx] =
            makeFunction(BooleanOr::instance(),
                         getEndCondition(),
                         getAncestorEndCondition(), // from parent
                         false,
                         false);
          m_garbageConditions[ancestorEndIdx] = true;
        }
        else {
          m_conditions[ancestorEndIdx] = getEndCondition();
          m_garbageConditions[ancestorEndIdx] = false;
        }
      }
      else {
        // No user-spec'd end condition - build one
        m_conditions[endIdx] = &m_allFinishedFn;
        m_garbageConditions[endIdx] = false;
        // *** N.B. ***
        // Normally ancestor-end is our end condition ORed with parent's ancestor-end.
        // But default all-children-finished end condition will always be false
        // when child evaluates ancestor-end.
        // See node state transition diagrams for proof.
        // Since false OR <anything> == <anything>,
        // just use parent's ancestor-end (which may be empty).
        m_conditions[ancestorEndIdx] = getAncestorEndCondition();
        m_garbageConditions[ancestorEndIdx] = false;
      }
    }
    else {
      // No parent - simply reuse existing conditions, if any
      m_conditions[ancestorExitIdx] = m_conditions[exitIdx]; // could be null
      m_conditions[ancestorInvariantIdx] = m_conditions[invariantIdx]; // could be null
      // End is special
      if (m_conditions[endIdx]) {
        // User-spec'd end condition doubles as ancestor-end
        m_conditions[ancestorEndIdx] = m_conditions[endIdx];
        m_garbageConditions[ancestorEndIdx] = false;
      }
      else {
        // No user-spec'd end condition - build one
        m_conditions[endIdx] = &m_allFinishedFn;
        m_garbageConditions[endIdx] = false;
        // *** N.B. ***
        // Normally for root nodes, ancestor-end is same as end. 
        // But default all-children-finished end condition will always be false
        // when child evaluates ancestor-end.
        // See node state transition diagrams for proof.
        // So if no parent and no user end condition, just leave ancestor-end empty.
        m_conditions[ancestorEndIdx] = nullptr;
        m_garbageConditions[ancestorEndIdx] = false;
      }
    }
  }

  ListNode::~ListNode()
  {
    debugMsg("ListNode:~ListNode", " destructor for " << m_nodeId);

    cleanUpConditions();

    this->cleanUpNodeBody(); // LibraryCallNode wraps this ListNode method
  }

  void ListNode::cleanUpConditions()
  {
    if (m_cleanedConditions)
      return;

    debugMsg("ListNode:cleanUpConditions", " for " << m_nodeId);

    cleanUpChildConditions();

    NodeImpl::cleanUpConditions();
  }

  void ListNode::cleanUpNodeBody()
  {
    if (m_cleanedBody)
      return;

    debugMsg("ListNode:cleanUpNodeBody", " for " << m_nodeId);

    // Delete children
    for (NodeImplPtr &child : m_children)
      delete (Node*) child.release();
    m_children.clear();
    m_cleanedBody = true;
  }

  void ListNode::cleanUpChildConditions()
  {
    debugMsg("ListNode:cleanUpChildConditions", " for " << m_nodeId);
    for (NodeImplPtr &child : m_children)
      child->cleanUpConditions();
    for (NodeImplPtr &child : m_children)
      child->cleanUpNodeBody();
  }

  NodeImpl const *ListNode::findChild(char const *childName) const
  {
    for (NodeImplPtr const &child : m_children)
      if (child->getNodeId() == childName)
        return child.get();
    return nullptr;
  }

  NodeImpl *ListNode::findChild(char const *childName)
  {
    for (NodeImplPtr &child : m_children)
      if (child->getNodeId() == childName)
        return child.get();
    return nullptr;
  }

  void ListNode::reserveChildren(size_t n)
  {
    m_children.reserve(n);
  }

  void ListNode::addChild(NodeImpl *node)
  {
    m_children.emplace_back(NodeImplPtr(node));
  }

  void ListNode::setState(PlexilExec *exec, NodeState newValue, double tym)
  {
    NodeImpl::setState(exec, newValue, tym);
    // Notify the children if the new state is one that they care about.
    switch (newValue) {
    case WAITING_STATE:
      for (NodeImplPtr &child : m_children)
        if (child->getState() == FINISHED_STATE)
          child->notify(exec);
      break;

    case EXECUTING_STATE:
    case FINISHED_STATE:
      for (NodeImplPtr &child : m_children)
        if (child->getState() == INACTIVE_STATE)
          child->notify(exec);
      break;

    default:
      break;
    }
  }

  //////////////////////////////////////
  //
  // Specialized state transition logic
  //
  //////////////////////////////////////

  //
  // EXECUTING
  //
  // Description and methods here are for NodeList and LibraryNodeCall only
  //
  // Legal predecessor states: WAITING
  // Conditions active: AncestorExit, AncestorInvariant, End, Exit, Invariant
  // Legal successor states: FAILING, FINISHING

  void ListNode::transitionToExecuting()
  {
    activateLocalVariables();

    // From WAITING, AncestorExit, AncestorInvariant, Exit are already active
    activateInvariantCondition();
    activateEndCondition();

    // These conditions are for the children.
    if (m_conditions[ancestorExitIdx])
      m_conditions[ancestorExitIdx]->activate();
    if (m_conditions[ancestorInvariantIdx])
      m_conditions[ancestorInvariantIdx]->activate();
    if (m_conditions[ancestorEndIdx])
      m_conditions[ancestorEndIdx]->activate();
  }

  bool ListNode::getDestStateFromExecuting()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor exit for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and ANCESTOR_EXIT_CONDITION true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Exit condition for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and EXIT_CONDITION true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor invariant for " << getNodeId() << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and ANCESTOR_INVARIANT_CONDITION false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Invariant for " << getNodeId() << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and INVARIANT_CONDITION false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = INVARIANT_CONDITION_FAILED;
        return true;
      }
    }

    if ((cond = getEndCondition()) && (!cond->getValue(temp) || !temp)) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "End for " << getNodeId() << ' ' << this << " is inactive.");
#endif
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> no change.");
      return false;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> FINISHING. List node and END_CONDITION true.");
    m_nextState = FINISHING_STATE;
    return true;
  }

  void ListNode::transitionFromExecuting(PlexilExec * /* exec */)
  {
    deactivateEndCondition();
    switch (m_nextState) {
      
    case FAILING_STATE:
      deactivateAncestorExitInvariantConditions(); 
      deactivateExitCondition();
      deactivateInvariantCondition();
      // fall through

    case FINISHING_STATE:
      // Both successor states will need this
      activateActionCompleteCondition();
      break;

    default:
      errorMsg("Attempting to transition NodeList/LibraryNodeCall "
               "from EXECUTING to invalid state "
               << nodeStateName(m_nextState));
      break;
    }
  }

  //
  // FINISHING
  //
  // State is only valid for NodeList and LibraryNodeCall nodes
  //
  // Legal predecessor states: EXECUTING
  // Conditions active: ActionComplete, AncestorExit, AncestorInvariant, Exit, Invariant, Post
  // Legal successor states: FAILING, ITERATION_ENDED

  void ListNode::transitionToFinishing()
  {
    activatePostCondition();
  }

  bool ListNode::getDestStateFromFinishing()
  {
    Expression *cond;
    bool temp;
    if ((cond = getAncestorExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor exit for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and ANCESTOR_EXIT_CONDITION true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = PARENT_EXITED;
        return true;
      }
    }

    if ((cond = getExitCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Exit condition for " << m_nodeId << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and EXIT_CONDITION true.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = INTERRUPTED_OUTCOME;
        m_nextFailureType = EXITED;
        return true;
      }
    }

    if ((cond = getAncestorInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Ancestor invariant for " << getNodeId() << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and ANCESTOR_INVARIANT_CONDITION false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = PARENT_FAILED;
        return true;
      }
    }

    if ((cond = getInvariantCondition())) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
      checkError(cond->isActive(),
                 "Invariant for " << getNodeId() << ' ' << this << " is inactive.");
#endif
      if (cond->getValue(temp) && !temp) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FAILING. List node and INVARIANT_CONDITION false.");
        m_nextState = FAILING_STATE;
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = INVARIANT_CONDITION_FAILED;
        return true;
      }
    }

    cond = getActionCompleteCondition();
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
    checkError(cond->isActive(),
               "Children waiting or finished for " << getNodeId() << ' ' << this
               << " is inactive.");
#endif
    cond->getValue(temp); // cannot be unknown, see above
    if (temp) {
      m_nextState = ITERATION_ENDED_STATE;
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> ITERATION_ENDED. List node and ALL_CHILDREN_WAITING_OR_FINISHED true.");
      if ((cond = getPostCondition()) && (!cond->getValue(temp) || !temp)) {
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
        checkError(cond->isActive(),
                   "ListNode::getDestStateFromFinishing: Post for " << m_nodeId << " is inactive.");
#endif
        m_nextOutcome = FAILURE_OUTCOME;
        m_nextFailureType = POST_CONDITION_FAILED;
      }
      else
        m_nextOutcome = SUCCESS_OUTCOME;
      return true;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> no change. List node, ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
    return false;
  }

  void ListNode::transitionFromFinishing(PlexilExec *exec)
  {
    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    switch (m_nextState) {

    case ITERATION_ENDED_STATE:
      // N.B. These are conditions for the children.
      if (m_conditions[ancestorExitIdx])
        m_conditions[ancestorExitIdx]->deactivate();
      if (m_conditions[ancestorInvariantIdx])
        m_conditions[ancestorInvariantIdx]->deactivate();
      if (m_conditions[ancestorEndIdx])
        m_conditions[ancestorEndIdx]->deactivate();
      // Local conditions
      deactivateActionCompleteCondition();
      deactivateExecutable(exec);
      activateAncestorEndCondition();
      break;

    case FAILING_STATE:
      deactivateAncestorExitInvariantConditions();
      break;

    default:
      errorMsg("Attempting to transition List node from FINISHING to invalid state "
               << nodeStateName(m_nextState));
    }
  }

  //
  // FAILING
  //
  // Description and methods here apply only to NodeList and LibraryNodeCall nodes
  //
  // Legal predecessor states: EXECUTING, FINISHING
  // Conditions active: ActionComplete
  // Legal successor states: FINISHED, ITERATION_ENDED

  void ListNode::transitionToFailing(PlexilExec * /* exec */)
  {
    // From EXECUTING: ActionComplete is already active (see transitionFromExecuting() above)
    // From FINISHING: ActionComplete is already active
  }

  bool ListNode::getDestStateFromFailing()
  {
    Expression *cond = getActionCompleteCondition();
#ifdef PARANOID_ABOUT_CONDITION_ACTIVATION
    checkError(cond->isActive(),
               "Children waiting or finished for " << getNodeId() << ' ' << this
               << " is inactive.");
#endif
    bool tempb;
    cond->getValue(tempb); // AllWaitingOrFinished is always known
    if (tempb) {
      if (this->getFailureType() == PARENT_EXITED) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. "
                 "List node, ALL_CHILDREN_WAITING_OR_FINISHED true and parent exited.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      if (this->getFailureType() == PARENT_FAILED) {
        debugMsg("Node:getDestState",
                 ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
                 << " -> FINISHED. "
                 "List node, ALL_CHILDREN_WAITING_OR_FINISHED true and parent failed.");
        m_nextState = FINISHED_STATE;
        return true;
      }
      debugMsg("Node:getDestState",
               ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
               << " -> ITERATION_ENDED. List node and "
               << (this->getFailureType() == EXITED ? "self-exited" : "self-failure."));
      m_nextState = ITERATION_ENDED_STATE;
      return true;
    }

    debugMsg("Node:getDestState",
             ' ' << m_nodeId << ' ' << this << ' ' << nodeStateName(m_state)
             << " -> no change.");
    return false;
  }

  void ListNode::transitionFromFailing(PlexilExec *exec)
  {
    // N.B. These are conditions for the children.
    if (m_conditions[ancestorExitIdx])
      m_conditions[ancestorExitIdx]->deactivate();
    if (m_conditions[ancestorInvariantIdx])
      m_conditions[ancestorInvariantIdx]->deactivate();
    if (m_conditions[ancestorEndIdx])
      m_conditions[ancestorEndIdx]->deactivate();

    deactivateActionCompleteCondition();
    deactivateExecutable(exec);

    switch (m_nextState) {

    case ITERATION_ENDED_STATE:
      activateAncestorExitInvariantConditions();
      activateAncestorEndCondition();
      break;

    case FINISHED_STATE:
      // nothing to do
      break;

    default:
      errorMsg("Attempting to transition NodeList/LibraryNodeCall node "
               "from FAILING to invalid state "
               << nodeStateName(m_nextState));
      break;
    }
  }


  // ***
  // *** END NODE STATE LOGIC ***
  // ***

  void ListNode::specializedActivate()
  {
    // Activate all children
    for (NodeImplPtr &child : m_children)
      child->activateNode();
  }

}
