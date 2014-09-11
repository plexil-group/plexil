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

#ifndef _H_Node
#define _H_Node

#include "Expression.hh"
#include "NodeConnector.hh"
#include "NodeVariables.hh"
#include "PlexilNodeType.hh"
#include "PlexilPlan.hh"
#include "generic_hash_map.hh"

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL {

  typedef PLEXIL_HASH_MAP(std::string, Expression *) VariableMap;

  // NOTE: this used to be 100000000, which somehow gets printed as
  // scientific notation in XML and doesn't parse correctly.  
  const double WORST_PRIORITY = 100000;

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
    DECLARE_STATIC_CLASS_CONST(std::string, SKIP_CONDITION, "SkipCondition"); /*!< The name for the node's skip condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, START_CONDITION, "StartCondition"); /*!< The name for the node's start condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, END_CONDITION, "EndCondition"); /*!< The name for the node's end condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, EXIT_CONDITION, "ExitCondition"); /*!< The name for the node's exit condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, INVARIANT_CONDITION, "InvariantCondition"); /*!< The name for the node's invariant condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, PRE_CONDITION, "PreCondition"); /*!< The name for the node's pre-condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, POST_CONDITION, "PostCondition"); /*!< The name for the node's post-condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, REPEAT_CONDITION, "RepeatCondition"); /*!< The name for the node's repeat condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, ANCESTOR_INVARIANT_CONDITION, "AncestorInvariantCondition"); /*!< The name for the node's ancestor-invariant
                                                                                                        condition (parent.invariant && parent.ancestor-invariant).*/
    DECLARE_STATIC_CLASS_CONST(std::string, ANCESTOR_END_CONDITION, "AncestorEndCondition"); /*!< The name for the ancestor-end condition
                                                                                            (parent.end || parent.ancestor-end). */
    DECLARE_STATIC_CLASS_CONST(std::string, ANCESTOR_EXIT_CONDITION, "AncestorExitCondition"); /*!< The name for the ancestor-exit condition
                                                                                            (parent.exit || parent.ancestor-exit). */

    DECLARE_STATIC_CLASS_CONST(std::string, ACTION_COMPLETE, "ActionCompleteCondition"); /*!< The name for the action-complete condition. */
    DECLARE_STATIC_CLASS_CONST(std::string, ABORT_COMPLETE, "AbortCompleteCondition"); /*!< The name for the abort-complete condition. */

    static const std::vector<std::string>& ALL_CONDITIONS();

    //in-built variable names
    DECLARE_STATIC_CLASS_CONST(std::string, STATE, "state");
    DECLARE_STATIC_CLASS_CONST(std::string, OUTCOME, "outcome");
    DECLARE_STATIC_CLASS_CONST(std::string, FAILURE_TYPE, "failure_type");
    DECLARE_STATIC_CLASS_CONST(std::string, COMMAND_HANDLE, "command_handle");

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    Node(PlexilNode const *node, Node *parent = NULL);

    /**
     * @brief Alternate constructor.  Used only by Exec test module.
     */
    Node(const std::string& type,
         const std::string& name,
         NodeState state,
         Node *parent = NULL);

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
    Expression *findVariable(const PlexilVarRef* ref);

    /**
     * @brief Looks up a variable by name.
     */
    virtual Expression *findVariable(const std::string& name, bool recursive = false);

    Node *findNodeRef(PlexilNodeRef const *nodeRef);

    // create conditions, assignments, and commands.
    // We have to do this late because they could refer to internal variables of other nodes.
    void postInit(PlexilNode const *node);

    // Make the node active.
    void activate();
        
    /**
     * @brief Accessor for the NodeId as it was written in the XML.
     * @return This node's node id.
     */
    std::string const &getNodeId() const { return m_nodeId; }

    /**
     * @brief Accessor for the Node's parent.
     * @return This node's parent.
     */
    Node *getParent() {return m_parent; }
    Node const *getParent() const {return m_parent; }

    /**
     * @brief Commit a pending state transition based on the statuses of various conditions.
     * @param time The time of the transition.
     */
    void transition(double time = 0.0); // FIXME - need a better representation

    /**
     * @brief Accessor for the priority of a node.  The priority is used to resolve resource conflicts.
     * @return the priority of this node.
     * @note Default method; only assignment nodes care about priority.
     */
    virtual double getPriority() const {return WORST_PRIORITY;}

    /**
     * @brief Gets the destination state of this node, were it to transition, based on the values of various conditions.
     * @return True if the new destination state is different from the last check, false otherwise.
     * @note Sets m_nextState, m_nextOutcome, m_nextFailureType as a side effect.
     * @note External only for convenience of unit tests.
     */
    bool getDestState();

    /**
     * @brief Gets the previously calculated destination state of this node.
     * @return The destination state.
     * @note Should only be called by PlexilExec::resolveVariableConflicts() and unit tests.
     */
    NodeState getNextState() const 
    {
      return (NodeState) m_nextState;
    }

    /**
     * @brief Gets the name of the current state of this node.
     * @return the current node state name as a const reference to string.
     */
    std::string const &getStateName() const;

    /**
     * @brief Gets the current state of this node.
     * @return the current node state as a NodeState (enum) value.
     */
    NodeState getState() const;

    /**
     * @brief Sets the state variable to the new state.
     * @param newValue The new node state.
     * @note Virtual so it can be overridden by ListNode wrapper method.
     */
    virtual void setState(NodeState newValue, double tym); // FIXME

    // Transition helpers
    // Public so transition tests can use it.
    void setNodeFailureType(FailureType f);

    /**
     * @brief Get the time of the named transition.
     * @param state The state.
     * @param isEnd True if requesting end time of state, false for start.
     * @param result Place to store the requested time.
     * @return True if requested time is known, false otherwise.
     * @note If unknown, result is not modified.
     */
    bool getStateTransitionTime(NodeState state, bool isEnd, double &result) const; // FIXME

    /**
     * @brief Get a const pointer to the time of the named transition.
     * @param state The state.
     * @param isEnd True if requesting end time of state, false for start.
     * @param ptr Place to store the pointer to the time.
     * @return True if requested time is known, false otherwise.
     * @note If unknown, ptr is not modified.
     */
    bool getStateTransitionTimePointer(NodeState state, bool isEnd, double const *&ptr) const; // FIXME

    /**
     * @brief Gets the time at which this node entered its current state.
     * @return Time value as a double.
     */
    double getCurrentStateStartTime() const;

    //Isaac
    double getCurrentStateEndTime() const;

    //Isaac - get local variables
    const VariableMap& getLocalVariablesByName() { return m_variablesByName; }
    
    //Isaac - get local variables
    const std::vector<Expression *> & getLocalVariables() { return m_localVariables; }

    //Isaac - get children
    virtual const std::vector<Node *>& getChildren() const;

    /**
     * @brief Gets the state variable representing the state of this node.
     * @return the state variable.
     */
    Expression *getStateVariable() { return &m_stateVariable; }

    NodeOutcome getOutcome() const;
    Expression *getOutcomeVariable() { return &m_outcomeVariable; }

    FailureType getFailureType() const;
    Expression *getFailureTypeVariable() { return &m_failureTypeVariable; }

    /**
     * @brief Accessor for an assignment node's assigned variable.
     */
    virtual Assignable *getAssignmentVariable() const 
    {
      return NULL;
    }

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     */
    PlexilNodeType getType() const {return m_nodeType;}

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
    Expression *getAncestorEndCondition()               { return getCondition(ancestorEndIdx); }
    Expression *getAncestorExitCondition()              { return getCondition(ancestorExitIdx); }
    Expression *getAncestorInvariantCondition()         { return getCondition(ancestorInvariantIdx); }

    // User conditions
    Expression *getSkipCondition()                      { return m_conditions[skipIdx]; }
    Expression *getStartCondition()                     { return m_conditions[startIdx]; }
    Expression *getEndCondition()                       { return m_conditions[endIdx]; }
    Expression *getExitCondition()                      { return m_conditions[exitIdx]; }
    Expression *getInvariantCondition()                 { return m_conditions[invariantIdx]; }
    Expression *getPreCondition()                       { return m_conditions[preIdx]; }
    Expression *getPostCondition()                      { return m_conditions[postIdx]; }
    Expression *getRepeatCondition()                    { return m_conditions[repeatIdx]; }
    // These are for specialized node types
    Expression *getActionCompleteCondition()            { return m_conditions[actionCompleteIdx]; }
    Expression *getAbortCompleteCondition()             { return m_conditions[abortCompleteIdx]; }

    // Test whether a condition is active
    // These are public only to appease the module test

    // These are special because parent owns the condition expression
    bool isAncestorEndConditionActive()               { return getAncestorEndCondition()->isActive(); }
    bool isAncestorExitConditionActive()              { return getAncestorExitCondition()->isActive(); }
    bool isAncestorInvariantConditionActive()         { return getAncestorInvariantCondition()->isActive(); }

    // User conditions
    bool isSkipConditionActive()                      { return m_conditions[skipIdx]->isActive(); }
    bool isStartConditionActive()                     { return m_conditions[startIdx]->isActive(); }
    bool isEndConditionActive()                       { return m_conditions[endIdx]->isActive(); }
    bool isExitConditionActive()                      { return m_conditions[exitIdx]->isActive(); }
    bool isInvariantConditionActive()                 { return m_conditions[invariantIdx]->isActive(); }
    bool isPreConditionActive()                       { return m_conditions[preIdx]->isActive(); }
    bool isPostConditionActive()                      { return m_conditions[postIdx]->isActive(); }
    bool isRepeatConditionActive()                    { return m_conditions[repeatIdx]->isActive(); }
    // These are for specialized node types
    bool isActionCompleteConditionActive()            { return m_conditions[actionCompleteIdx]->isActive(); }
    bool isAbortCompleteConditionActive()             { return m_conditions[abortCompleteIdx]->isActive(); }

    // Should only be used by LuvListener.
    Expression *getCondition(const std::string& name);

    // NodeFactory::createNode for the module test needs this to be public.
    void activateInternalVariables();

  protected:
    friend class LibraryCallNode;
    friend class ListNode;

    friend class PlexilExec;
    friend class InternalCondition;

    friend class FailureVariable;
    friend class OutcomeVariable;
    friend class StateVariable;

    // N.B.: These need to match the order of ALL_CONDITIONS()
    enum ConditionIndex {
      // Conditions on parent
      // N.B. Ancestor end/exit/invariant MUST come before
      // end/exit/invariant, respectively, because the former depend
      // on the latter and must be cleaned up first.
      ancestorExitIdx = 0,
      ancestorInvariantIdx,
      ancestorEndIdx,
      // User specified conditions
      skipIdx,
      startIdx,
      preIdx,
      exitIdx,
      invariantIdx,
      endIdx,
      postIdx,
      repeatIdx,
      // For all but Empty nodes
      actionCompleteIdx,
      // For all but Empty and Update nodes
      abortCompleteIdx,

      conditionIndexMax
    };

    // Abstracts out the issue of where the condition comes from.
    Expression *getCondition(size_t idx);
    Expression const *getCondition(size_t idx) const;

    void removeConditionListener(size_t idx);

    static size_t getConditionIndex(const std::string& cName);
    static const std::string& getConditionName(size_t idx);

    virtual Node const *findChild(const std::string& childName) const;
    virtual Node *findChild(const std::string& childName);

    void commonInit();

    // Called from the transition handler
    void execute();
    void reset();
    virtual void abort();
    virtual void deactivateExecutable();

    // Activate conditions

    // These are special because parent owns the condition expression
    void activateAncestorEndCondition();
    void activateAncestorExitInvariantConditions();

    // User conditions
    void activatePreSkipStartConditions();
    void activateEndCondition();
    void activateExitCondition();
    void activateInvariantCondition();
    void activatePostCondition();
    void activateRepeatCondition();
    // These are for specialized node types
    void activateActionCompleteCondition();
    void activateAbortCompleteCondition();

    // Deactivate a condition

    // These are special because parent owns the condition expression
    void deactivateAncestorEndCondition();
    void deactivateAncestorExitInvariantConditions();

    // User conditions
    void deactivatePreSkipStartConditions();
    void deactivateEndCondition();
    void deactivateExitCondition();
    void deactivateInvariantCondition();
    void deactivatePostCondition();
    void deactivateRepeatCondition();
    // These are for specialized node types
    void deactivateActionCompleteCondition();
    void deactivateAbortCompleteCondition();

    // Specific behaviors for derived classes
    virtual void specializedPostInit(PlexilNode const *node);
    virtual void specializedPostInitLate(PlexilNode const *node);
    virtual void createConditionWrappers();
    virtual void specializedActivate();
    virtual void specializedHandleExecution();
    virtual void specializedDeactivateExecutable();
    virtual void specializedReset();

    //
    // State transition implementation methods
    //
    // Non-virtual member functions are common to all node types.
    // Virtual members are specialized by node type.
    //

    // getDestStateFrom...
    // Return true if the new destination state is different from the last check, false otherwise.
    // Set m_nextState, m_nextOutcome, m_nextFailureType as a side effect.
    bool getDestStateFromInactive();
    bool getDestStateFromWaiting();
    virtual bool getDestStateFromExecuting();
    virtual bool getDestStateFromFinishing();
    bool getDestStateFromFinished();
    virtual bool getDestStateFromFailing();
    bool getDestStateFromIterationEnded();

    //
    // Transition out of the named current state.
    void transitionFromInactive();
    void transitionFromWaiting();
    virtual void transitionFromExecuting();
    virtual void transitionFromFinishing();
    void transitionFromFinished();
    virtual void transitionFromFailing();
    void transitionFromIterationEnded();

    void transitionToInactive();
    void transitionToWaiting();
    virtual void transitionToExecuting();
    virtual void transitionToFinishing();
    void transitionToFinished();
    virtual void transitionToFailing();
    void transitionToIterationEnded(); 

    // Phases of destructor
    // Not useful if called from base class destructor!
    virtual void cleanUpConditions();
    void cleanUpVars();
    virtual void cleanUpNodeBody();

    // Printing utility
    virtual void printCommandHandle(std::ostream& stream, const unsigned int indent) const;

    // Node state limit
    virtual NodeState nodeStateMax() const { return FINISHED_STATE; } // empty node method

    //
    // Listener class
    //

    class ConditionChangeListener : public ExpressionListener 
    {
    public:
      ConditionChangeListener(Node& node);
      ~ConditionChangeListener();

      void notifyChanged(Expression const *src);

    private:

      // Deliberately unimplemented
      ConditionChangeListener();
      ConditionChangeListener(const ConditionChangeListener&);
      ConditionChangeListener& operator=(const ConditionChangeListener&);

      Node& m_node;
    };

    //
    // Common state
    //
    Node *m_parent; /*!< The parent of this node.*/
    // Listener for the various condition expressions.
    ConditionChangeListener m_listener;
    std::string m_nodeId;  /*!< the NodeId from the xml.*/
 
    // Expressions
    VariableMap m_variablesByName; /*!< Locally declared variables or references to variables gotten through an interface. */
    std::vector<std::string>* m_sortedVariableNames; /*!< Convenience for printing. */
    std::vector<Expression *> m_localVariables; /*!< Variables created in this node. */
    Expression *m_conditions[conditionIndexMax]; /*!< The condition expressions. */
    StateVariable m_stateVariable;
    OutcomeVariable m_outcomeVariable;
    FailureVariable m_failureTypeVariable;

    // Node transition history trace
    // Records the state and the time it was entered
    double m_transitionTimes[NODE_STATE_MAX]; /*!< The times of each node transition since activation. */
    uint16_t m_transitionStates[NODE_STATE_MAX]; /*!< The sequence of states since activation. */
    uint16_t m_traceIdx; /*!< The index of the next entry into the transition history tables. */

    // Current state
    uint16_t m_state; /*!< The current state of the node. */
    uint16_t m_nextState; /*!< The state returned by getDestState() the last time checkConditions() was called. */
    uint16_t m_outcome;
    uint16_t m_nextOutcome;
    uint16_t m_failureType;
    uint16_t m_nextFailureType;

    // Housekeeping details
    bool m_garbageConditions[conditionIndexMax]; /*!< Flags for conditions to delete. */
    bool m_postInitCalled, m_cleanedConditions, m_cleanedVars, m_checkConditionsPending;
    PlexilNodeType m_nodeType; /*!< The node type. */

  private:

    void logTransition(double time, NodeState newState);

    void createConditions(const std::vector<std::pair<PlexilExpr *, std::string> >& conds);

    void createDeclaredVars(const std::vector<PlexilVar *>& vars);

    void getVarsFromInterface(PlexilInterface const *intf);
    Expression *getInVariable(PlexilVarRef const *varRef, bool parentIsLibCall);
    Assignable *getInOutVariable(PlexilVarRef const *varRef, bool parentIsLibCall);

    // These 3 should only be called from transition().
    void setNodeOutcome(NodeOutcome o);
    void transitionFrom();
    void transitionTo(double tym); // FIXME

    void activateLocalVariables();
    void deactivateLocalVariables();

    Expression *getInternalVariable(const std::string& name);
    Expression const *getInternalVariable(const std::string& name) const;

    //
    // Internal versions
    //

    void printVariables(std::ostream& stream, const unsigned int indent = 0) const;
    void ensureSortedVariableNames() const;

    // Cleanup
    static void purgeAllConditions();

    // Storage for static "constants"
    static std::vector<std::string>* s_allConditions;
  };

  std::ostream& operator<<(std::ostream& strm, const Node& node);

}

#endif
