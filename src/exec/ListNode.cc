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

#include "ListNode.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecConnector.hh"
#include "NodeFactory.hh"

#include <algorithm> // for find_if

namespace PLEXIL
{

  /**
   * @brief The constructor.  Will construct all conditions and child nodes.
   * @param node The PlexilNodeId for this node and all of its children.
   * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
   * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
   */
  ListNode::ListNode(const PlexilNodeId& node, 
                     const ExecConnectorId& exec, 
                     const NodeId& parent)
    : Node(node, exec, parent)
  {
    checkError(node->nodeType() == NodeType_NodeList || node->nodeType() == NodeType_LibraryNodeCall,
               "Invalid node type \"" << PlexilParser::nodeTypeString(node->nodeType())
               << "\" for a ListNode");

    // Instantiate child nodes, if any
    if (node->nodeType() == NodeType_NodeList) {
      debugMsg("Node:node", "Creating child nodes.");
      // XML parser should have checked for this
      checkError(Id<PlexilListBody>::convertable(node->body()),
                 "Node " << m_nodeId.toString() << " is a list node but doesn't have a " <<
                 "list body.");
      createChildNodes((PlexilListBody*) node->body()); // constructs default end condition
    }
  }

  /**
   * @brief Alternate constructor.  Used only by Exec test module.
   */
  ListNode::ListNode(const LabelStr& type,
                     const LabelStr& name, 
                     const NodeState state,
                     const ExecConnectorId& exec,
                     const NodeId& parent)
    : Node(type, name, state, exec, parent)
  {
    checkError(type == LIST() || type == LIBRARYNODECALL(),
               "Invalid node type \"" << type.toString() << "\" for a ListNode");

    switch (m_state) {
    case EXECUTING_STATE:
      deactivatePostCondition();
      m_conditions[ancestorEndIdx]->activate();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      break;

    case FINISHING_STATE:
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
      activateActionCompleteCondition();
      activateExitCondition();
      activateInvariantCondition();
      activatePostCondition();
      m_conditions[ancestorEndIdx]->activate();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      break;

    case FAILING_STATE:
      activateActionCompleteCondition();
      m_conditions[ancestorEndIdx]->activate();
      m_conditions[ancestorExitIdx]->activate();
      m_conditions[ancestorInvariantIdx]->activate();
      break;

    default:
      break;
    }
  }

  void ListNode::createChildNodes(const PlexilListBody* body) 
  {
    try {
      for (std::vector<PlexilNodeId>::const_iterator it = body->children().begin();
           it != body->children().end(); 
           ++it)
        m_children.push_back(NodeFactory::createNode(*it, m_exec, m_id));
    }
    catch (const Error& e) {
      debugMsg("Node:node", " Error creating child nodes: " << e);
      // Clean up 
      while (!m_children.empty()) {
        delete (Node*) m_children.back();
        m_children.pop_back();
      }
      // Rethrow so that outer error handler can deal with this as well
      throw;
    }
  }

  // N.B. The end condition constructed below can be overridden by the user
  void ListNode::createSpecializedConditions()
  {
    ExpressionId cond = (new AllChildrenWaitingOrFinishedCondition(m_children))->getId();
    cond->addListener(makeConditionListener(actionCompleteIdx));
    m_conditions[actionCompleteIdx] = cond;
    m_garbageConditions[actionCompleteIdx] = true;

    ExpressionId endCond = (new AllChildrenFinishedCondition(m_children))->getId();
    endCond->addListener(makeConditionListener(endIdx));
    m_conditions[endIdx] = endCond;
    m_garbageConditions[endIdx] = true;
  }

  void ListNode::specializedPostInitLate(const PlexilNodeId& node)
  {
    //call postInit on all children
    const PlexilListBody* body = (const PlexilListBody*) node->body();
    check_error(body != NULL);
    std::vector<NodeId>::iterator it = m_children.begin();
    std::vector<PlexilNodeId>::const_iterator pit = body->children().begin();   
    while (it != m_children.end() && pit != body->children().end()) {
      (*it++)->postInit(*pit++);
    }
    checkError(it == m_children.end() && pit == body->children().end(),
               "Node:postInit: mismatch between PlexilNode and list node children");
  }

  // Create the ancestor end, ancestor exit, and ancestor invariant conditions required by children
  void ListNode::createConditionWrappers()
  {
    if (m_parent) {
      m_conditions[ancestorEndIdx] =
        (new Disjunction(getAncestorEndCondition(), // from parent
                         false,
                         getEndCondition(),
                         false))->getId();
      m_conditions[ancestorExitIdx] =
        (new Disjunction(getAncestorExitCondition(), // from parent
                         false,
                         getExitCondition(),
                         false))->getId();
      m_conditions[ancestorInvariantIdx] =
        (new Conjunction(getAncestorInvariantCondition(), // from parent
                         false,
                         getInvariantCondition(),
                         false))->getId();
      m_garbageConditions[ancestorEndIdx] = true;
      m_garbageConditions[ancestorExitIdx] = true;
      m_garbageConditions[ancestorInvariantIdx] = true;
    }
    else {
      // Simply reuse existing conditions
      m_conditions[ancestorEndIdx] = m_conditions[endIdx];
      m_conditions[ancestorExitIdx] = m_conditions[exitIdx];
      m_conditions[ancestorInvariantIdx] = m_conditions[invariantIdx];
    }
  }

  /**
   * @brief Destructor.  Cleans up this entire part of the node tree.
   */
  ListNode::~ListNode()
  {
    debugMsg("ListNode:~ListNode", " destructor for " << m_nodeId.toString());

    cleanUpConditions();
    cleanUpNodeBody();
    // cleanUpVars(); // base destructor can handle this
  }

  void ListNode::cleanUpConditions() 
  {
    if (m_cleanedConditions)
      return;

    debugMsg("ListNode:cleanUpConditions", " for " << m_nodeId.toString());

    cleanUpChildConditions();

    // Defer to base class
    Node::cleanUpConditions();
  }

  void ListNode::cleanUpNodeBody()
  {
    debugMsg("ListNode:cleanUpNodeBody", " for " << m_nodeId.toString());
    // Delete children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it) {
      delete (Node*) (*it);
    }
    m_children.clear();
  }

  void ListNode::cleanUpChildConditions()
  {
    debugMsg("ListNode:cleanUpChildConditions", " for " << m_nodeId.toString());
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->cleanUpConditions();
  }

  class NodeIdEq {
  public:
    NodeIdEq(const double name) : m_name(name) {}
    bool operator()(const NodeId& node) {return node->getNodeId() == m_name;}
  private:
    double m_name;
  };

  NodeId ListNode::findChild(const LabelStr& childName) const
  {
    std::vector<NodeId>::const_iterator it =
      std::find_if(m_children.begin(), m_children.end(), NodeIdEq(childName));
    if (it == m_children.end())
      return NodeId::noId();
    return *it;
  }

  /**
   * @brief Sets the state variable to the new state.
   * @param newValue The new node state.
   * @note This method notifies the children of a change in the parent node's state.
   */
  void ListNode::setState(NodeState newValue)
  {
    Node::setState(newValue);
    // Notify the children if the new state is one that they care about.
    switch (newValue) {
    case WAITING_STATE:
      for (std::vector<NodeId>::iterator it = m_children.begin();
           it != m_children.end();
           ++it)
        if ((*it)->getState() == FINISHED_STATE)
          (*it)->conditionChanged();
      break;

    case EXECUTING_STATE:
    case FINISHED_STATE:
      for (std::vector<NodeId>::iterator it = m_children.begin();
           it != m_children.end();
           ++it)
        if ((*it)->getState() == INACTIVE_STATE)
          (*it)->conditionChanged();
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
    // From WAITING, AncestorExit, AncestorInvariant, Exit are active
    activateInvariantCondition();
    activateEndCondition();

    // These conditions are for the children.
    m_conditions[ancestorEndIdx]->activate();
    m_conditions[ancestorExitIdx]->activate();
    m_conditions[ancestorInvariantIdx]->activate();
  }

  NodeState ListNode::getDestStateFromExecuting()
  {
    checkError(isAncestorExitConditionActive(),
               "Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and ANCESTOR_EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isExitConditionActive(),
               "Exit condition for " << m_nodeId.toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and ANCESTOR_INVARIANT_CONDITION false.");
      return FAILING_STATE;
    }

    checkError(isInvariantConditionActive(),
               "Invariant for " << getNodeId().toString() << " is inactive.");
    if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and INVARIANT_CONDITION false.");
      return FAILING_STATE;
    }

    checkError(isEndConditionActive(),
               "End for " << getNodeId().toString() << " is inactive.");
    if (getEndCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FINISHING. List node and END_CONDITION true.");
      return FINISHING_STATE;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state.");
    return NO_NODE_STATE;
  }

  void ListNode::transitionFromExecuting(NodeState destState)
  {
    checkError(destState == FINISHING_STATE || destState == FAILING_STATE,
               "Attempting to transition NodeList/LibraryNodeCall from EXECUTING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
    }
    else if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
      getFailureTypeVariable()->setValue(FailureVariable::EXITED());
    }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
    }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
    }
      
    deactivateEndCondition();
    // Both successor states will need this
    activateActionCompleteCondition();

    if (destState == FAILING_STATE) {
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition(); 
      deactivateExitCondition();
      deactivateInvariantCondition();
    }

    // Defer deactivating local variables to FAILING or FINISHING.
    // deactivateExecutable();
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
    // activateActionCompleteCondition(); // see transitionFromExecuting() above
    activatePostCondition();
  }

  NodeState ListNode::getDestStateFromFinishing()
  {
    checkError(isAncestorExitConditionActive(),
               "Ancestor exit for " << m_nodeId.toString() << " is inactive.");
    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and ANCESTOR_EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isExitConditionActive(),
               "Exit condition for " << m_nodeId.toString() << " is inactive.");
    if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and EXIT_CONDITION true.");
      return FAILING_STATE;
    }

    checkError(isAncestorInvariantConditionActive(),
               "Ancestor invariant for " << getNodeId().toString() << " is inactive.");
    if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and ANCESTOR_INVARIANT_CONDITION false.");
      return FAILING_STATE;
    }

    checkError(isInvariantConditionActive(),
               "Invariant for " << getNodeId().toString() << " is inactive.");
    if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: FAILING. List node and INVARIANT_CONDITION false.");
      return FAILING_STATE;
    }

    checkError(isActionCompleteConditionActive(),
               "Children waiting or finished for " << getNodeId().toString() <<
               " is inactive.");
    if (getActionCompleteCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      debugMsg("Node:getDestState",
               " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. List node " <<
               "and ALL_CHILDREN_WAITING_OR_FINISHED true.");
      return ITERATION_ENDED_STATE;
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state. ALL_CHILDREN_WAITING_OR_FINISHED false or unknown.");
    return NO_NODE_STATE;
  }

  void ListNode::transitionFromFinishing(NodeState destState)
  {
    checkError(destState == ITERATION_ENDED_STATE ||
               destState == FAILING_STATE,
               "Attempting to transition List node from FINISHING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    if (getAncestorExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
        getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
        getFailureTypeVariable()->setValue(FailureVariable::PARENT_EXITED());
      }
    else if (getExitCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
        getOutcomeVariable()->setValue(OutcomeVariable::INTERRUPTED());
        getFailureTypeVariable()->setValue(FailureVariable::EXITED());
      }
    else if (getAncestorInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
        getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
        getFailureTypeVariable()->setValue(FailureVariable::PARENT_FAILED());
      }
    else if (getInvariantCondition()->getValue() == BooleanVariable::FALSE_VALUE()) {
        getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
        getFailureTypeVariable()->setValue(FailureVariable::INVARIANT_CONDITION_FAILED());
      }
    else if (getPostCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      getOutcomeVariable()->setValue(OutcomeVariable::SUCCESS());
    }
    else {
      getOutcomeVariable()->setValue(OutcomeVariable::FAILURE());
      getFailureTypeVariable()->setValue(FailureVariable::POST_CONDITION_FAILED());
    }

    deactivateExitCondition();
    deactivateInvariantCondition();
    deactivatePostCondition();

    if (destState == ITERATION_ENDED_STATE) {
      deactivateActionCompleteCondition();
      activateAncestorEndCondition();

      // N.B. These are conditions for the children.
      m_conditions[ancestorEndIdx]->deactivate();
      m_conditions[ancestorExitIdx]->deactivate();
      m_conditions[ancestorInvariantIdx]->deactivate();

      deactivateExecutable();
    }
    else { // FAILING
      deactivateAncestorExitCondition();
      deactivateAncestorInvariantCondition();
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

  void ListNode::transitionToFailing()
  {
    // From EXECUTING: ActionComplete active (see transitionFromExecuting() above)
    // From FINISHING: ActionComplete active
  }

  NodeState ListNode::getDestStateFromFailing()
  {
    checkError(isActionCompleteConditionActive(),
               "Children waiting or finished for " << getNodeId().toString() <<
               " is inactive.");

    if (getActionCompleteCondition()->getValue() == BooleanVariable::TRUE_VALUE()) {
      if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_EXITED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: FINISHED. "
                 <<"List node, ALL_CHILDREN_WAITING_OR_FINISHED true and parent exited.");
        return FINISHED_STATE;
      }
      else if (m_failureTypeVariable->getValue() == FailureVariable::PARENT_FAILED()) {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: FINISHED. List node and ALL_CHILDREN_WAITING_OR_FINISHED" <<
                 " true and parent failed.");
        return FINISHED_STATE;
      }
      else {
        debugMsg("Node:getDestState",
                 " '" << m_nodeId.toString() << "' destination: ITERATION_ENDED. List node and "
                 << (m_failureTypeVariable->getValue() == FailureVariable::EXITED() ? "self-exited" : "self-failure."));
        return ITERATION_ENDED_STATE;
      }
    }

    debugMsg("Node:getDestState",
             " '" << m_nodeId.toString() << "' destination: no state.");
    return NO_NODE_STATE;
  }

  void ListNode::transitionFromFailing(NodeState destState)
  {
    checkError(destState == ITERATION_ENDED_STATE ||
               destState == FINISHED_STATE,
               "Attempting to transition NodeList/LibraryNodeCall node from FAILING to invalid state '"
               << StateVariable::nodeStateName(destState).toString() << "'");

    deactivateActionCompleteCondition();

    // N.B. These are conditions for the children.
    m_conditions[ancestorEndIdx]->deactivate();
    m_conditions[ancestorExitIdx]->deactivate();
    m_conditions[ancestorInvariantIdx]->deactivate();

    if (destState == ITERATION_ENDED_STATE) {
      activateAncestorEndCondition();
      activateAncestorExitCondition();
      activateAncestorInvariantCondition();
    }

    deactivateExecutable();
  }


  // ***
  // *** END NODE STATE LOGIC ***
  // ***

  void ListNode::specializedActivate()
  {
    // Activate all children
    for (std::vector<NodeId>::iterator it = m_children.begin(); it != m_children.end(); ++it)
      (*it)->activate();
  }

}
