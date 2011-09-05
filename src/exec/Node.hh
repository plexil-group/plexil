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

#ifndef _H_Node
#define _H_Node

#include "ConstantMacros.hh"
#include "ExecDefs.hh"
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
    DECLARE_STATIC_CLASS_CONST(LabelStr, SKIP_CONDITION, "SkipCondition"); /*!< The name for the node's skip condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, START_CONDITION, "StartCondition"); /*!< The name for the node's start condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, END_CONDITION, "EndCondition"); /*!< The name for the node's end condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, INVARIANT_CONDITION, "InvariantCondition"); /*!< The name for the node's invariant condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, PRE_CONDITION, "PreCondition"); /*!< The name for the node's pre-condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, POST_CONDITION, "PostCondition"); /*!< The name for the node's post-condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, REPEAT_CONDITION, "RepeatCondition"); /*<! The name for the node's repeat condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, ANCESTOR_INVARIANT_CONDITION, "AncestorInvariantCondition"); /*!< The name for the node's ancestor-invariant
													condition (parent.invariant && parent.ancestor-invariant).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, ANCESTOR_END_CONDITION, "AncestorEndCondition"); /*<! The name for the ancestor-end condition
											    (parent.end || parent.ancestor-end).*/

    DECLARE_STATIC_CLASS_CONST(LabelStr, PARENT_EXECUTING_CONDITION, "ParentExecutingCondition"); /*<! The name for the ancestor-executing condition
												    (checked in state INACTIVE, transitions to state WAITING)*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, PARENT_FINISHED_CONDITION, "ParentFinishedCondition"); /*<! The name for the ancestor-executing condition
												    (checked in state INACTIVE, transitions to state WAITING)*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, CHILDREN_WAITING_OR_FINISHED, "AllChildrenWaitingOrFinishedCondition"); /*<! The name for the node's all-children-waiting-or-finished condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, ABORT_COMPLETE, "AbortCompleteCondition"); /*<! The name for the command-abort-complete condition.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, PARENT_WAITING_CONDITION, "ParentWaitingCondition");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_HANDLE_RECEIVED_CONDITION, "CommandHandleReceivedCondition");

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
    Node(const LabelStr& type, const LabelStr& name, const NodeState state,
	 const bool skip, const bool start, const bool pre,
	 const bool invariant, const bool post, const bool end, const bool repeat,
	 const bool ancestorInvariant, const bool ancestorEnd, const bool parentExecuting,
	 const bool childrenFinished, const bool commandAbort, const bool parentWaiting,
	 const bool parentFinished, const bool cmdHdlRcvdCondition,
	 const ExecConnectorId& exec = ExecConnectorId::noId());

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
	virtual void transitionFrom(NodeState destState);

    /**
     * @brief Handle the node entering this new state.
     */
	virtual void transitionTo(NodeState destState);

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
     */
    void setState(NodeState newValue);

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
    ExpressionId& getSkipCondition()                      { return m_conditions[skipIdx]; }
    ExpressionId& getStartCondition()                     { return m_conditions[startIdx]; }
    ExpressionId& getEndCondition()                       { return m_conditions[endIdx]; }
    ExpressionId& getInvariantCondition()                 { return m_conditions[invariantIdx]; }
    ExpressionId& getPreCondition()                       { return m_conditions[preIdx]; }
    ExpressionId& getPostCondition()                      { return m_conditions[postIdx]; }
    ExpressionId& getRepeatCondition()                    { return m_conditions[repeatIdx]; }
    ExpressionId& getAncestorInvariantCondition()         { return m_conditions[ancestorInvariantIdx]; }
    ExpressionId& getAncestorEndCondition()               { return m_conditions[ancestorEndIdx]; }
    ExpressionId& getParentExecutingCondition()           { return m_conditions[parentExecutingIdx]; }
    ExpressionId& getChildrenWaitingOrFinishedCondition() { return m_conditions[childrenWaitingOrFinishedIdx]; }
    ExpressionId& getAbortCompleteCondition()             { return m_conditions[abortCompleteIdx]; }
    ExpressionId& getParentWaitingCondition()             { return m_conditions[parentWaitingIdx]; }
    ExpressionId& getParentFinishedCondition()            { return m_conditions[parentFinishedIdx]; }
    ExpressionId& getCommandHandleReceivedCondition()     { return m_conditions[commandHandleReceivedIdx]; }

	// Activate a condition
	// These are public only to appease the module test
    void activateSkipCondition()                      { return activatePair(skipIdx); }
    void activateStartCondition()                     { return activatePair(startIdx); }
    void activateEndCondition()                       { return activatePair(endIdx); }
    void activateInvariantCondition()                 { return activatePair(invariantIdx); }
    void activatePreCondition()                       { return activatePair(preIdx); }
    void activatePostCondition()                      { return activatePair(postIdx); }
    void activateRepeatCondition()                    { return activatePair(repeatIdx); }
    void activateAncestorInvariantCondition()         { return activatePair(ancestorInvariantIdx); }
    void activateAncestorEndCondition()               { return activatePair(ancestorEndIdx); }
    void activateParentExecutingCondition()           { return activatePair(parentExecutingIdx); }
    void activateChildrenWaitingOrFinishedCondition() { return activatePair(childrenWaitingOrFinishedIdx); }
    void activateAbortCompleteCondition()             { return activatePair(abortCompleteIdx); }
    void activateParentWaitingCondition()             { return activatePair(parentWaitingIdx); }
    void activateParentFinishedCondition()            { return activatePair(parentFinishedIdx); }
    void activateCommandHandleReceivedCondition()     { return activatePair(commandHandleReceivedIdx); }

	// Should only be used by LuvListener.
    ExpressionId& getCondition(const LabelStr& name);

  protected:
	friend class LibraryCallNode;
	friend class ListNode;

    friend class PlexilExec;
    friend class InternalCondition;

    // N.B.: These need to match the order of ALL_CONDITIONS()
    enum {
	  // User specified conditions
      skipIdx = 0,
      startIdx,
      endIdx,
      invariantIdx,
      preIdx,
      postIdx,
      repeatIdx,
	  // Internal conditions for all nodes
      ancestorInvariantIdx,
      ancestorEndIdx,
      parentExecutingIdx,
      parentWaitingIdx,
      parentFinishedIdx,
	  // Only for list or library call nodes
      childrenWaitingOrFinishedIdx,
	  // Only for command nodes
      abortCompleteIdx,
      commandHandleReceivedIdx,

      conditionIndexMax
    } conditionIndex;

    static unsigned int getConditionIndex(const LabelStr& cName);
    static LabelStr getConditionName(unsigned int idx);
	virtual NodeId findChild(const LabelStr& childName) const;

    void commonInit();

	// Called from the transition handler
    void execute();
    void reset();
    virtual void abort();
    virtual void deactivateExecutable();

	// Deactivate a condition
    void deactivateSkipCondition()                      { return deactivatePair(skipIdx); }
    void deactivateStartCondition()                     { return deactivatePair(startIdx); }
    void deactivateEndCondition()                       { return deactivatePair(endIdx); }
    void deactivateInvariantCondition()                 { return deactivatePair(invariantIdx); }
    void deactivatePreCondition()                       { return deactivatePair(preIdx); }
    void deactivatePostCondition()                      { return deactivatePair(postIdx); }
    void deactivateRepeatCondition()                    { return deactivatePair(repeatIdx); }
    void deactivateAncestorInvariantCondition()         { return deactivatePair(ancestorInvariantIdx); }
    void deactivateAncestorEndCondition()               { return deactivatePair(ancestorEndIdx); }
    void deactivateParentExecutingCondition()           { return deactivatePair(parentExecutingIdx); }
    void deactivateChildrenWaitingOrFinishedCondition() { return deactivatePair(childrenWaitingOrFinishedIdx); }
    void deactivateAbortCompleteCondition()             { return deactivatePair(abortCompleteIdx); }
    void deactivateParentWaitingCondition()             { return deactivatePair(parentWaitingIdx); }
    void deactivateParentFinishedCondition()            { return deactivatePair(parentFinishedIdx); }
    void deactivateCommandHandleReceivedCondition()     { return deactivatePair(commandHandleReceivedIdx); }

	// Test whether a condition is active
    bool isSkipConditionActive()                      { return pairActive(skipIdx); }
    bool isStartConditionActive()                     { return pairActive(startIdx); }
    bool isEndConditionActive()                       { return pairActive(endIdx); }
    bool isInvariantConditionActive()                 { return pairActive(invariantIdx); }
    bool isPreConditionActive()                       { return pairActive(preIdx); }
    bool isPostConditionActive()                      { return pairActive(postIdx); }
    bool isRepeatConditionActive()                    { return pairActive(repeatIdx); }
    bool isAncestorInvariantConditionActive()         { return pairActive(ancestorInvariantIdx); }
    bool isAncestorEndConditionActive()               { return pairActive(ancestorEndIdx); }
    bool isParentExecutingConditionActive()           { return pairActive(parentExecutingIdx); }
    bool isChildrenWaitingOrFinishedConditionActive() { return pairActive(childrenWaitingOrFinishedIdx); }
    bool isAbortCompleteConditionActive()             { return pairActive(abortCompleteIdx); }
    bool isParentWaitingConditionActive()             { return pairActive(parentWaitingIdx); }
    bool isParentFinishedConditionActive()            { return pairActive(parentFinishedIdx); }
    bool isCommandHandleReceivedConditionActive()     { return pairActive(commandHandleReceivedIdx); }

	// Specific behaviors for derived classes
	virtual void specializedPostInit(const PlexilNodeId& node);
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

	virtual void printCommandHandle(std::ostream& stream, const unsigned int indent, bool always = false) const;

	// Phases of destructor
	// Not useful if called from base class destructor!
    virtual void cleanUpConditions();
    virtual void cleanUpVars();
	virtual void cleanUpChildConditions();
	virtual void cleanUpNodeBody();

	//
	// Common state
	//
    NodeId m_id; /*<! The Id for this node*/
    NodeId m_parent; /*<! The parent of this node.*/
    ExecConnectorId m_exec; /*<! The executive (to notify it about condition changes and whether it needs to be executed) */
    LabelStr m_nodeId;  /*<! the NodeId from the xml.*/
    LabelStr m_nodeType; /*<! The node type (either directly from the Node element or determined by the sub-elements. */
    VariableMap m_variablesByName; /*<! Locally declared variables or references to variables gotten through an interface. */
	std::vector<double>* m_sortedVariableNames; /*<! Convenience for printing. */
    std::vector<VariableId> m_localVariables; /*<! Variables created in this node. */
    ExpressionId m_conditions[conditionIndexMax]; /*<! The condition expressions. */
    ExpressionListenerId m_listeners[conditionIndexMax]; /*<! Listeners on the various condition expressions.  This allows us to turn them on/off when appropriate. */
    VariableId m_startTimepoints[NODE_STATE_MAX]; /*<! Timepoint start variables indexed by state. */
    VariableId m_endTimepoints[NODE_STATE_MAX]; /*<! Timepoint end variables indexed by state. */
    VariableId m_stateVariable;
	VariableId m_outcomeVariable;
	VariableId m_failureTypeVariable;
    NodeState m_state; /*<! The actual state of the node. */
    NodeState m_lastQuery; /*<! The state of the node the last time checkConditions() was called. */
	bool m_garbageConditions[conditionIndexMax]; /*<! Flags for conditions to delete. */
    bool m_postInitCalled, m_cleanedConditions, m_cleanedVars, m_transitioning, m_checkConditionsPending;

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

    bool pairActive(unsigned int idx);

    void activatePair(unsigned int idx);

    void deactivatePair(unsigned int idx);

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


}

#endif
