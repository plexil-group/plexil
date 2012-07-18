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

#ifndef _H_Node
#define _H_Node

#include "ConstantMacros.hh"
#include "ExecDefs.hh"
#include "Expression.hh"
#include "LabelStr.hh"
#include "NodeConnector.hh"
#include "PlexilPlan.hh"
#include "generic_hash_map.hh"

#include <list>
#include <map>
#include <set>
#include <vector>

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL {

  typedef PLEXIL_HASH_MAP(double, VariableId) VariableMap;

  /**
   * @brief The class representing a Node in the plan--either a list of sub-Nodes, an assignment, or a command execution.
   * There is a possible refactoring here, breaking the three node types into subclasses.  Unfortunately, the XML format doesn't
   * currently support that.
   * There is a fair amount of work to be done here.  For instance, implementing the entire state transition graph and
   * responses to the various conditions changing value, accessors for the node information and the conditions, there should
   * be error checking in all of the state transitions for node types (FAILING, for instance, can only be occupied by list nodes).
   */
  class Node : public NodeConnector
  {
  public:
    //condition names
    DECLARE_STATIC_CLASS_CONST(LabelStr, SKIP_CONDITION, "SkipCondition"); /*!< The name for the node's skip condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, START_CONDITION, "StartCondition"); /*!< The name for the node's start condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, END_CONDITION, "EndCondition"); /*!< The name for the node's end condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, EXIT_CONDITION, "ExitCondition"); /*!< The name for the node's exit condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, INVARIANT_CONDITION, "InvariantCondition"); /*!< The name for the node's invariant condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, PRE_CONDITION, "PreCondition"); /*!< The name for the node's pre-condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, POST_CONDITION, "PostCondition"); /*!< The name for the node's post-condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, REPEAT_CONDITION, "RepeatCondition"); /*!< The name for the node's repeat condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, ANCESTOR_INVARIANT_CONDITION, "AncestorInvariantCondition"); /*!< The name for the node's ancestor-invariant
                                                                                                        condition (parent.invariant && parent.ancestor-invariant).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, ANCESTOR_END_CONDITION, "AncestorEndCondition"); /*!< The name for the ancestor-end condition
                                                                                            (parent.end || parent.ancestor-end). */
    DECLARE_STATIC_CLASS_CONST(LabelStr, ANCESTOR_EXIT_CONDITION, "AncestorExitCondition"); /*!< The name for the ancestor-exit condition
                                                                                            (parent.exit || parent.ancestor-exit). */

    DECLARE_STATIC_CLASS_CONST(LabelStr, ACTION_COMPLETE, "ActionCompleteCondition"); /*!< The name for the action-complete condition. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, ABORT_COMPLETE, "AbortCompleteCondition"); /*!< The name for the abort-complete condition. */

    static const std::vector<double>& ALL_CONDITIONS();

    //in-built variable names
    DECLARE_STATIC_CLASS_CONST(LabelStr, STATE, "state");
    DECLARE_STATIC_CLASS_CONST(LabelStr, OUTCOME, "outcome");
    DECLARE_STATIC_CLASS_CONST(LabelStr, FAILURE_TYPE, "failure_type");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_HANDLE, "command_handle");

    //node types
    DECLARE_STATIC_CLASS_CONST(LabelStr, ASSIGNMENT, "Assignment");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND, "Command");
    DECLARE_STATIC_CLASS_CONST(LabelStr, LIST, "NodeList");
    DECLARE_STATIC_CLASS_CONST(LabelStr, LIBRARYNODECALL, "LibraryNodeCall");
    DECLARE_STATIC_CLASS_CONST(LabelStr, UPDATE, "Update");
    DECLARE_STATIC_CLASS_CONST(LabelStr, EMPTY, "Empty");

    static const LabelStr& nodeTypeToLabelStr(PlexilNodeType nodeType);

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    Node(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent = NodeId::noId());

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    Node(const LabelStr& type,
         const LabelStr& name,
         const NodeState state,
         const ExecConnectorId& exec = ExecConnectorId::noId(),
         const NodeId& parent = NodeId::noId());

    /**
     * @brief Destructor.  Cleans up this entire part of the node tree.
     */
    virtual ~Node();

    //
    // NodeConnector API to expressions
    //

    /**
     * @brief Looks up a variable by reference.
     */
    const VariableId& findVariable(const PlexilVarRef* ref);

    /**
     * @brief Looks up a variable by name.
     */
    virtual const VariableId& findVariable(const LabelStr& name, bool recursive = false);

    const ExecConnectorId& getExec() const { return m_exec; }

    const NodeId& getNode() const { return m_id; }

    const ExecListenerHubId& getExecListenerHub() const;


    // create conditions, assignments, and commands.
    // We have to do this late because they could refer to internal variables of other nodes.
    void postInit(const PlexilNodeId& node);

    // Make the node active.
    virtual void activate();

    const NodeId& getId() const {return m_id;}
        
    /**
     * @brief Accessor for the NodeId as it was written in the XML.
     * @return This node's node id.
     */
    const LabelStr& getNodeId(){return m_nodeId;}

    /**
     * @brief Accessor for the Node's parent.
     * @return This node's parent.
     */
    NodeId& getParent() {return m_parent; }

    /**
     * @brief Ask whether this node can transition now.
     * @note This member function is used only by exec-test-module.
     */
    bool canTransition();

    /**
     * @brief Commit a state transition based on the statuses of various conditions.
     * @note See the various state graphs.
     */
    void transition(NodeState destState, const double time = 0.0);

    /**
     * @brief Handle the node exiting its current state.
     */
    void transitionFrom(NodeState destState);

    /**
     * @brief Handle the node entering this new state.
     */
    void transitionTo(NodeState destState);

    /**
     * @brief Accessor for the priority of a node.  The priority is used to resolve resource conflicts.
     * @return the priority of this node.
     * @note Default method; only assignment nodes care about priority.
     */
    virtual double getPriority() const {return WORST_PRIORITY;}

    /**
     * @brief Gets the destination state of this node, were it to transition, based on the values of various conditions.
     * @return The destination state.
     */
    NodeState getDestState();

    /**
     * @brief Gets the name of the current state of this node.
     * @return the current node state name as a LabelStr const reference.
     */
    const LabelStr& getStateName() const;

    /**
     * @brief Gets the current state of this node.
     * @return the current node state as a double (LabelStr key).
     */
    NodeState getState() const;

    /**
     * @brief Sets the state variable to the new state.
     * @param newValue The new node state.
     * @note Virtual so it can be overridden by ListNode wrapper method.
     */
    virtual void setState(NodeState newValue);

    /**
     * @brief Gets the time at which this node entered its current state.
     * @return Time value as a double.
     */
    double getCurrentStateStartTime() const;

    //Isaac
    double getCurrentStateEndTime() const;

    //Isaac - get local variables ExpressionMap
    const VariableMap& getLocalVariablesByName() { return m_variablesByName; }
    
    //Isaac - get local variables
    const std::vector<VariableId> & getLocalVariables() { return m_localVariables; }

    //Isaac - get children
    virtual const std::vector<NodeId>& getChildren() const;

    /**
     * @brief Gets the state variable representing the state of this node.
     * @return the state variable.
     */
    const VariableId& getStateVariable() const { return m_stateVariable; }

    const LabelStr getOutcome();
    const VariableId& getOutcomeVariable() const { return m_outcomeVariable; }

    const LabelStr getFailureType();
    const VariableId& getFailureTypeVariable() const { return m_failureTypeVariable; }

    /**
     * @brief Accessor for an assignment node's assigned variable.
     */
    virtual const VariableId& getAssignmentVariable() const 
    {
      return VariableId::noId();
    }

    /**
     * @brief Gets the type of this node (node list, assignment, or command).
     * @return The type of this node.
     */
    const LabelStr& getType() const {return m_nodeType;}

    /**
     * @brief Notifies the node that one of its conditions has changed.
     */
    void conditionChanged();

    /**
     * @brief Evaluates the conditions to see if the node is eligible to transition.
     */
    void checkConditions();

    std::string toString(const unsigned int indent = 0);
    void print(std::ostream& stream, const unsigned int indent = 0) const;

    // Condition accessors
    // These are public only to appease the module test

    // These conditions belong to the parent node.
    const ExpressionId& getAncestorEndCondition() const               { return getCondition(ancestorEndIdx); }
    const ExpressionId& getAncestorExitCondition() const              { return getCondition(ancestorExitIdx); }
    const ExpressionId& getAncestorInvariantCondition() const         { return getCondition(ancestorInvariantIdx); }

    // User conditions
    const ExpressionId& getSkipCondition() const                      { return m_conditions[skipIdx]; }
    const ExpressionId& getStartCondition() const                     { return m_conditions[startIdx]; }
    const ExpressionId& getEndCondition() const                       { return m_conditions[endIdx]; }
    const ExpressionId& getExitCondition() const                      { return m_conditions[exitIdx]; }
    const ExpressionId& getInvariantCondition() const                 { return m_conditions[invariantIdx]; }
    const ExpressionId& getPreCondition() const                       { return m_conditions[preIdx]; }
    const ExpressionId& getPostCondition() const                      { return m_conditions[postIdx]; }
    const ExpressionId& getRepeatCondition() const                    { return m_conditions[repeatIdx]; }
    // These are for specialized node types
    const ExpressionId& getActionCompleteCondition() const            { return m_conditions[actionCompleteIdx]; }
    const ExpressionId& getAbortCompleteCondition() const             { return m_conditions[abortCompleteIdx]; }

    // Activate a condition
    // These are public only to appease the module test

    // These are special because parent owns the condition expression
    void activateAncestorEndCondition()
    {
      if (m_listeners[ancestorEndIdx].isId())
        m_listeners[ancestorEndIdx]->activate(); 
    }
    void activateAncestorExitCondition()
    {
      if (m_listeners[ancestorExitIdx].isId())
        m_listeners[ancestorExitIdx]->activate(); 
    }
    void activateAncestorInvariantCondition()         
    {
      if (m_listeners[ancestorEndIdx].isId())
        m_listeners[ancestorInvariantIdx]->activate(); 
    }

    // User conditions
    void activateSkipCondition()                      { activatePair(skipIdx); }
    void activateStartCondition()                     { activatePair(startIdx); }
    void activateEndCondition()                       { activatePair(endIdx); }
    void activateExitCondition()                      { activatePair(exitIdx); }
    void activateInvariantCondition()                 { activatePair(invariantIdx); }
    void activatePreCondition()                       { activatePair(preIdx); }
    void activatePostCondition()                      { activatePair(postIdx); }
    void activateRepeatCondition()                    { activatePair(repeatIdx); }
    // These are for specialized node types
    void activateActionCompleteCondition()            { activatePair(actionCompleteIdx); }
    void activateAbortCompleteCondition()             { activatePair(abortCompleteIdx); }

    // Test whether a condition is active
    // These are public only to appease the module test

    // These are special because parent owns the condition expression
    bool isAncestorEndConditionActive()               { return pairActive(ancestorEndIdx); }
    bool isAncestorExitConditionActive()              { return pairActive(ancestorExitIdx); }
    bool isAncestorInvariantConditionActive()         { return pairActive(ancestorInvariantIdx); }

    // User conditions
    bool isSkipConditionActive()                      { return pairActive(skipIdx); }
    bool isStartConditionActive()                     { return pairActive(startIdx); }
    bool isEndConditionActive()                       { return pairActive(endIdx); }
    bool isExitConditionActive()                      { return pairActive(exitIdx); }
    bool isInvariantConditionActive()                 { return pairActive(invariantIdx); }
    bool isPreConditionActive()                       { return pairActive(preIdx); }
    bool isPostConditionActive()                      { return pairActive(postIdx); }
    bool isRepeatConditionActive()                    { return pairActive(repeatIdx); }
    // These are for specialized node types
    bool isActionCompleteConditionActive()            { return pairActive(actionCompleteIdx); }
    bool isAbortCompleteConditionActive()             { return pairActive(abortCompleteIdx); }

    // Should only be used by LuvListener.
    const ExpressionId& getCondition(const LabelStr& name) const;

  protected:
    friend class LibraryCallNode;
    friend class ListNode;

    friend class PlexilExec;
    friend class InternalCondition;

    // N.B.: These need to match the order of ALL_CONDITIONS()
    enum ConditionIndex {
      // Conditions on parent
      // N.B. Ancestor end and ancestor invariant MUST come before
      // end and invariant, respectively, because the former depend
      // on the latter and must be cleaned up first.
      ancestorEndIdx = 0,
      ancestorExitIdx,
      ancestorInvariantIdx,
      // User specified conditions
      skipIdx,
      startIdx,
      endIdx,
      exitIdx,
      invariantIdx,
      preIdx,
      postIdx,
      repeatIdx,
      // For all but Empty nodes
      actionCompleteIdx,
      // For all but Empty and Update nodes
      abortCompleteIdx,

      conditionIndexMax
    };

    // Abstracts out the issue of where the condition comes from.
    const ExpressionId& getCondition(size_t idx) const;

    ExpressionListenerId makeConditionListener(size_t idx);

    static size_t getConditionIndex(const LabelStr& cName);
    static LabelStr getConditionName(size_t idx);

    virtual NodeId findChild(const LabelStr& childName) const;

    void commonInit();

    // Called from the transition handler
    void execute();
    void reset();
    virtual void abort();
    virtual void deactivateExecutable();

    // Deactivate a condition

    // These are special because parent owns the condition expression
    void deactivateAncestorEndCondition()               
    {
      if (m_listeners[ancestorEndIdx].isId())
        m_listeners[ancestorEndIdx]->deactivate(); 
    }
    void deactivateAncestorExitCondition()               
    {
      if (m_listeners[ancestorExitIdx].isId())
        m_listeners[ancestorExitIdx]->deactivate(); 
    }
    void deactivateAncestorInvariantCondition()         
    {
      if (m_listeners[ancestorEndIdx].isId())
        m_listeners[ancestorInvariantIdx]->deactivate(); 
    }

    // User conditions
    void deactivateSkipCondition()                      { deactivatePair(skipIdx); }
    void deactivateStartCondition()                     { deactivatePair(startIdx); }
    void deactivateEndCondition()                       { deactivatePair(endIdx); }
    void deactivateExitCondition()                      { deactivatePair(exitIdx); }
    void deactivateInvariantCondition()                 { deactivatePair(invariantIdx); }
    void deactivatePreCondition()                       { deactivatePair(preIdx); }
    void deactivatePostCondition()                      { deactivatePair(postIdx); }
    void deactivateRepeatCondition()                    { deactivatePair(repeatIdx); }
    // These are for specialized node types
    void deactivateActionCompleteCondition()            { deactivatePair(actionCompleteIdx); }
    void deactivateAbortCompleteCondition()             { deactivatePair(abortCompleteIdx); }

    // Specific behaviors for derived classes
    virtual void specializedPostInit(const PlexilNodeId& node);
    virtual void specializedPostInitLate(const PlexilNodeId& node);
    virtual void createSpecializedConditions();
    virtual void createConditionWrappers();
    virtual void specializedActivate();
    virtual void specializedActivateInternalVariables();
    virtual void specializedHandleExecution();
    virtual void specializedDeactivateExecutable();
    virtual void specializedReset();

    // *** are these const? ***
    virtual NodeState getDestStateFromInactive();
    virtual NodeState getDestStateFromWaiting();
    virtual NodeState getDestStateFromExecuting();
    virtual NodeState getDestStateFromFinishing();
    virtual NodeState getDestStateFromFinished();
    virtual NodeState getDestStateFromFailing();
    virtual NodeState getDestStateFromIterationEnded();

    virtual void transitionFromInactive(NodeState toState);
    virtual void transitionFromWaiting(NodeState toState);
    virtual void transitionFromExecuting(NodeState toState);
    virtual void transitionFromFinishing(NodeState toState);
    virtual void transitionFromFinished(NodeState toState);
    virtual void transitionFromFailing(NodeState toState);
    virtual void transitionFromIterationEnded(NodeState toState);

    virtual void transitionToInactive();
    virtual void transitionToWaiting();
    virtual void transitionToExecuting();
    virtual void transitionToFinishing();
    virtual void transitionToFinished();
    virtual void transitionToFailing();
    virtual void transitionToIterationEnded(); 

    // Phases of destructor
    // Not useful if called from base class destructor!
    virtual void cleanUpConditions();
    virtual void cleanUpVars();
    virtual void cleanUpNodeBody();

    // Printing utility
    virtual void printCommandHandle(std::ostream& stream, const unsigned int indent) const;

    //
    // Common state
    //
    NodeId m_id; /*!< The Id for this node*/
    NodeId m_parent; /*!< The parent of this node.*/
    ExecConnectorId m_exec; /*!< The executive (to notify it about condition changes and whether it needs to be executed) */
    LabelStr m_nodeId;  /*!< the NodeId from the xml.*/
    LabelStr m_nodeType; /*!< The node type (either directly from the Node element or determined by the sub-elements. */
    VariableMap m_variablesByName; /*!< Locally declared variables or references to variables gotten through an interface. */
    std::vector<double>* m_sortedVariableNames; /*!< Convenience for printing. */
    std::vector<VariableId> m_localVariables; /*!< Variables created in this node. */
    ExpressionId m_conditions[conditionIndexMax]; /*!< The condition expressions. */
    ExpressionListenerId m_listeners[conditionIndexMax]; /*!< Listeners on the various condition expressions.  This allows us to turn them on/off when appropriate. */
    VariableId m_startTimepoints[NO_NODE_STATE]; /*!< Timepoint start variables indexed by state. */
    VariableId m_endTimepoints[NO_NODE_STATE]; /*!< Timepoint end variables indexed by state. */
    VariableId m_stateVariable;
    VariableId m_outcomeVariable;
    VariableId m_failureTypeVariable;
    NodeState m_state; /*!< The actual state of the node. */
    NodeState m_lastQuery; /*!< The state returned by getDestState() the last time checkConditions() was called. */
    bool m_garbageConditions[conditionIndexMax]; /*!< Flags for conditions to delete. */
    bool m_postInitCalled, m_cleanedConditions, m_cleanedVars, m_checkConditionsPending;

  private:

    void createConditions(const std::map<std::string, PlexilExprId>& conds);

    void createDeclaredVars(const std::vector<PlexilVarId>& vars);

    void getVarsFromInterface(const PlexilInterfaceId& intf);
    VariableId getInVariable(const PlexilVarRef* varRef, bool parentIsLibCall);
    VariableId getInOutVariable(const PlexilVarRef* varRef, bool parentIsLibCall);

    void lockConditions();

    void unlockConditions();

    // Make the node's internal variables active.
    virtual void activateInternalVariables();

    // Deactivate the local variables
    void deactivateLocalVariables();

    /**
     * @brief Perform whatever action is necessary for execution.
     */
    virtual void handleExecution();

    const VariableId& getInternalVariable(const LabelStr& name) const;

    //
    // Internal versions
    //

    bool pairActive(size_t idx);

    void activatePair(size_t idx);

    void deactivatePair(size_t idx);

    /**
     * @brief Sets the default variables for the conditions and establishes the internal conditions that are dependent on parent conditions
     *
     */
    void setConditionDefaults();

    static const std::vector<double>& START_TIMEPOINT_NAMES();
    static const std::vector<double>& END_TIMEPOINT_NAMES();

    void printVariables(std::ostream& stream, const unsigned int indent = 0) const;
    void ensureSortedVariableNames() const;

  };

  std::ostream& operator<<(std::ostream& strm, const Node& node);

  class ConditionChangeListener : public ExpressionListener 
  {
  public:
    ConditionChangeListener(Node& node, size_t condIdx);
    void notifyValueChanged(const ExpressionId& /* expression */);

  private:
    // Deliberately unimplemented
    ConditionChangeListener();
    ConditionChangeListener(const ConditionChangeListener&);
    ConditionChangeListener& operator=(const ConditionChangeListener&);

    Node& m_node;
    LabelStr m_cond;
  };

}

#endif
