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
#include "PlexilPlan.hh"
#include "generic_hash_map.hh"

#include <list>
#include <map>
#include <set>

// Take care of annoying VxWorks macro
#undef UPDATE

namespace PLEXIL {

  // forward reference
  class NodeStateManager;
  typedef Id<NodeStateManager> NodeStateManagerId;

  typedef PLEXIL_HASH_MAP(double, ExpressionId) ExpressionMap;

  class NodeConnector {
  public:
    NodeConnector() : m_id(this) {}
    virtual ~NodeConnector() {m_id.remove();}
    const NodeConnectorId& getId() const {return m_id;}
    virtual const ExpressionId& findVariable(const PlexilVarRef* ref) const = 0;
    virtual const ExpressionId& findVariable(const LabelStr& name) const = 0;
    virtual const NodeId& getNode() const = 0;
    virtual const ExecConnectorId& getExec() = 0;
  protected:
  private:
    NodeConnectorId m_id;
  };

  /**
   * @brief The class representing a Node in the plan--either a list of sub-Nodes, an assignment, or a command execution.
   * There is a possible refactoring here, breaking the three node types into subclasses.  Unfortunately, the XML format doesn't
   * currently support that.
   * There is a fair amount of work to be done here.  For instance, implementing the entire state transition graph and
   * responses to the various conditions changing value, accessors for the node information and the conditions, there should
   * be error checking in all of the state transitions for node types (FAILING, for instance, can only be occupied by list nodes).
   */
  class Node {
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
    DECLARE_STATIC_CLASS_CONST(LabelStr, REQUEST, "Request");
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

    const NodeId& getId() const {return m_id;}

    /**
     * @brief Accessor for PlexilNode.
     * @return This node's contained PlexilNode
     */

    const PlexilNodeId & getPlexilNode()
    {
       return m_node;
    }
        
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
     * @brief Commit a state transition based on the statuses of various conditions.  See the various state graphs.
     */
    void transition(const double time = 0.0);

    /**
     * @brief Accessor for the assigned variable.
     */
    const ExpressionId& getAssignmentVariable() const;

    /**
     * @brief Accessor for the priority of a node.  The priority is used to resolve resource conflicts.
     * @return the priority of this node.
     */
    double getPriority() const {return m_priority;}

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

    /**
     * @brief Gets the state variable representing the state of this node.
     * @return the state variable.
     */
    const ExpressionId& getStateVariable();

    const LabelStr getOutcome();
    const ExpressionId& getOutcomeVariable();

    
    const LabelStr getFailureType();
    const ExpressionId& getFailureTypeVariable();

    const LabelStr getCommandHandle();
    const ExpressionId& getCommandHandleVariable();

    /**
     * @brief Gets the type of this node (node list, assignment, or command).
     * @return The type of this node.
     */
    const LabelStr& getType() {return m_nodeType;}

    /**
     * @brief Gets the assignment to be performed by this node.  Evaluates the RHS.
     * @return The <destination, value> pair.
     */
    AssignmentId& getAssignment();

    /**
     * @brief Gets the command to be executed by this node.  The arguments are evaluated by the external interface.
     * @return The structure representing the command
     */
    CommandId& getCommand();

    UpdateId& getUpdate();

    /**
     * @brief Notifies the node that one of its conditions has changed.
     */
    void conditionChanged();

    /**
     * @brief Evaluates the conditions to see if the node is eligible to transition.
     */
    void checkConditions();

    std::string toString(const unsigned int indent = 0);

    /**
     * @brief Looks up a variable by reference.
     */
//    const ExpressionId& _findVariable(const PlexilVarId& var);

    const ExpressionId& findVariable(const PlexilVarRef* ref) const;

    const ExpressionId& findVariable(const LabelStr& name) const;

    const ExecConnectorId& getExec() {return m_exec;}

    // Condition accessors
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

	// Should only be used by LuvListener.
    ExpressionId& getCondition(const LabelStr& name);

    double getAcknowledgementValue() const;

	// Called from the transition handler
    void execute();
    void reset();
    void abort();
    void deactivateExecutable();

	// Activate a condition
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

  protected:
    friend class PlexilExec;
    friend class InternalCondition;
    friend class StateComputer;
    friend class TransitionHandler;

    //create conditions, assignments, and commands.  We have to do this late because they could refer to internal variables of other nodes.
    void postInit();
    void commonInit();

  private:

    // N.B.: These need to match the order of ALL_CONDITIONS()
    enum {
      skipIdx = 0,
      startIdx,
      endIdx,
      invariantIdx,
      preIdx,
      postIdx,
      repeatIdx,
      ancestorInvariantIdx,
      ancestorEndIdx,
      parentExecutingIdx,
      childrenWaitingOrFinishedIdx,
      abortCompleteIdx,
      parentWaitingIdx,
      parentFinishedIdx,
      commandHandleReceivedIdx,

      conditionIndexMax
    } conditionIndex;

    static unsigned int getConditionIndex(const LabelStr& cName);
    LabelStr getConditionName(unsigned int idx);

    void createCommand(const PlexilCommandBody* body);

    void createAssignment(const PlexilAssignmentBody* body);

    void createUpdate(const PlexilUpdateBody* body);

    void createConditions(const std::map<std::string, PlexilExprId>& conds);

    void createChildNodes(const PlexilListBody* body);

    void createLibraryNode(const PlexilNodeId& node);

    void testLibraryNodeParameters(const PlexilNodeId& libNode, 
                                   const std::vector<PlexilVarRef*>& interfaceVars,
                                   PlexilAliasMap& aliases);

    void createDeclaredVars(const std::vector<PlexilVarId>& vars);

    void getVarsFromInterface(const PlexilInterfaceId& intf);

    void getVarsFromParent();

    void lockConditions();

    void unlockConditions();

    void cleanUpConditions();

    void cleanUpVars();

    const ExpressionId& getInternalVariable(const LabelStr& name);

    // Listener accessors
    ExpressionListenerId& getSkipListener()                      { return m_listeners[skipIdx]; }
    ExpressionListenerId& getStartListener()                     { return m_listeners[startIdx]; }
    ExpressionListenerId& getEndListener()                       { return m_listeners[endIdx]; }
    ExpressionListenerId& getInvariantListener()                 { return m_listeners[invariantIdx]; }
    ExpressionListenerId& getPreListener()                       { return m_listeners[preIdx]; }
    ExpressionListenerId& getPostListener()                      { return m_listeners[postIdx]; }
    ExpressionListenerId& getRepeatListener()                    { return m_listeners[repeatIdx]; }
    ExpressionListenerId& getAncestorInvariantListener()         { return m_listeners[ancestorInvariantIdx]; }
    ExpressionListenerId& getAncestorEndListener()               { return m_listeners[ancestorEndIdx]; }
    ExpressionListenerId& getParentExecutingListener()           { return m_listeners[parentExecutingIdx]; }
    ExpressionListenerId& getChildrenWaitingOrFinishedListener() { return m_listeners[childrenWaitingOrFinishedIdx]; }
    ExpressionListenerId& getAbortCompleteListener()             { return m_listeners[abortCompleteIdx]; }
    ExpressionListenerId& getParentWaitingListener()             { return m_listeners[parentWaitingIdx]; }
    ExpressionListenerId& getParentFinishedListener()            { return m_listeners[parentFinishedIdx]; }
    ExpressionListenerId& getCommandHandleReceivedListener()     { return m_listeners[commandHandleReceivedIdx]; }

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

    NodeId m_id; /*<! The Id for this node*/
    ExecConnectorId m_exec; /*<! The executive (to notify it about condition changes and whether it needs to be executed)*/
    NodeId m_parent; /*<! The parent of this node.*/
    NodeConnectorId m_connector;
    PlexilNodeId m_node;
    bool m_postInitCalled, m_cleanedConditions, m_cleanedVars, m_transitioning, m_checkConditionsPending;
    double m_priority; /*<! The priority of this node */
    LabelStr m_nodeId;  /*<! the NodeId from the xml.*/
    LabelStr m_nodeType; /*<! The node type (either directly from the Node element or determined by the sub-elements.*/
    NodeStateManagerId m_stateManager; /*<! The state manager for this node type. */
    ExpressionMap m_variablesByName; /*<! Locally declared variables or references to variables gotten through an interface.
							Should there be an expression type for handling 'in' variables (i.e. a wrapper that fails on setValue)?
							I'll stick all variables in here, just to be safe.*/
    std::list<ExpressionId> m_localVariables; /*<! Variables created in this node*/
    ExpressionId m_startTimepoints[NODE_STATE_MAX]; /*<! Timepoint start variables indexed by state. */
    ExpressionId m_endTimepoints[NODE_STATE_MAX]; /*<! Timepoint end variables indexed by state. */
    ExpressionId m_conditions[conditionIndexMax]; /*<! The condition expressions.*/
    ExpressionListenerId m_listeners[conditionIndexMax]; /*<! Listeners on the various condition expressions.  This allows us to turn them on/off when appropriate*/
    std::set<unsigned int> m_garbageConditions; /*<! Indices of conditions to be cleaned up. */
    AssignmentId m_assignment;
    CommandId m_command; /*<! The command to be performed. */
    UpdateId m_update;
    ExpressionId m_ack; /*<! The destination for acknowledgement of the command/assignment.  DON'T FORGET TO RESET THIS VALUE IN REPEAT-UNTILs! */
    std::list<NodeId> m_children; /*<! Child nodes.*/
    std::set<double> m_garbage; /*<! Expression names (conditions, internal variables, timepoint variables) to be cleaned up. */
    ExpressionId m_extraEndCond;
    ExpressionId m_interruptEndCond;
    ExpressionId m_conjunctCondition;
    ExpressionId m_allCommandHandleValues;
    ExpressionId m_stateVariable; /*<! Expression copy of the actual state of the node. */
    NodeState m_state; /*<! The actual state of the node. */
    NodeState m_lastQuery; /*<! The state of the node the last time checkConditions() was called. */
  };

  class Assignment {
  public:
    Assignment(const ExpressionId lhs, const ExpressionId rhs, const ExpressionId ack,
	       const LabelStr& lhsName, const bool deleteLhs, const bool deleteRhs);
    ~Assignment();
    AssignmentId& getId() {return m_id;}
    ExpressionId& getDest() {return m_lhs;}
    ExpressionId& getAck() {return m_ack;}
    double getValue(){return m_value;}
    void activate();
    void deactivate();
    const std::string& getDestName();
  protected:
    friend class Node;
    void fixValue();
  private:
    AssignmentId m_id;
    ExpressionId m_lhs, m_rhs, m_ack;
    double m_value;
    LabelStr m_destName;
    bool m_deleteLhs, m_deleteRhs;
  };

  // *** TODO: replace ResourceMap and ResourceValues with structs or classes
  typedef std::map<std::string, ExpressionId> ResourceMap;
  typedef std::vector<ResourceMap> ResourceList;
  typedef std::map<std::string, double> ResourceValues;
  typedef std::vector<ResourceValues> ResourceValuesList;

  class Command {
  public:
    Command(const ExpressionId nameExpr, 
			const std::list<ExpressionId>& args, 
            const ExpressionId dest,
            const LabelStr& dest_name,
			const ExpressionId ack,
			const std::list<ExpressionId>& garbage,
            const ResourceList& resource);
    ~Command();

    CommandId& getId() {return m_id;}
    const LabelStr& getName();
    ExpressionId& getDest() {return m_dest;}
    ExpressionId& getAck() {return m_ack;}
    const std::list<double>& getArgValues() {return m_argValues;}
    const ResourceValuesList& getResourceValues() const {return m_resourceValuesList;}
    const std::string& getDestName();

    void activate();
    void deactivate();

  protected:
    friend class Node;

    void fixValues();
    void fixResourceValues();

  private:
    CommandId m_id;
    LabelStr m_name;
    ExpressionId m_nameExpr;
    std::list<ExpressionId> m_args;
    ExpressionId m_dest;
    LabelStr m_destName;
    ExpressionId m_ack;
    std::list<ExpressionId> m_garbage;
    std::list<double> m_argValues;
    ResourceList m_resourceList;
    ResourceValuesList m_resourceValuesList;
  };

  class Update {
  public:
    Update(const NodeId& node, const ExpressionMap& pairs, const ExpressionId ack,
	   const std::list<ExpressionId>& garbage);
    ~Update();
    UpdateId& getId() {return m_id;}
    ExpressionId& getAck() {return m_ack;}
    const std::map<double, double>& getPairs() {return m_valuePairs;}
    void activate();
    void deactivate();
    const NodeId& getSource() {return m_source;}
  protected:
    friend class Node;
    void fixValues();
  private:
    UpdateId m_id;
    NodeId m_source;
    ExpressionMap m_pairs;
    ExpressionId m_ack;
    std::list<ExpressionId> m_garbage;
    std::map<double, double> m_valuePairs;
  };

}

#endif
