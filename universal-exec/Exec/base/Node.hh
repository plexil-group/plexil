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

#include "CommonDefs.hh"
#include "ExecDefs.hh"
#include "PlexilPlan.hh"
#include "LabelStr.hh"

#include <list>
#include <map>
#include <set>

namespace PLEXIL {

   typedef std::map<double, ExpressionId> ExpressionMap;

  class NodeConnector {
  public:
    NodeConnector() : m_id(this) {}
    virtual ~NodeConnector() {m_id.remove();}
    const NodeConnectorId& getId() const {return m_id;}
    virtual const ExpressionId& findVariable(const PlexilVarRef* ref) = 0;
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

    static const std::set<double>& ALL_CONDITIONS();

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
    DECLARE_STATIC_CLASS_CONST(LabelStr, FUNCTION, "FunctionCall");
    DECLARE_STATIC_CLASS_CONST(LabelStr, REQUEST, "Request");
    DECLARE_STATIC_CLASS_CONST(LabelStr, EMPTY, "Empty");

    /**
     * @brief The constructor.  Will construct all conditions and child nodes.
     * @param node The PlexilNodeId for this node and all of its children.
     * @param exec The executive (used for notifying the executive that a node is eligible for state transition or execution).
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    Node(const PlexilNodeId& node, const ExecConnectorId& exec, const NodeId& parent = NodeId::noId());

    Node(const LabelStr& type, const LabelStr& name, const LabelStr& state, const bool skip, const bool start, const bool pre,
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
    const LabelStr& getDestState();

    /**
     * @brief Gets the current state of this node.
     * @return the current node state.
     */
    const LabelStr getState();

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

    FunctionCallId& getFunctionCall();

    void checkConditions();

    std::string toString(const unsigned int indent = 0);

    /**
     * @brief Looks up a variable by reference.
     */
//    const ExpressionId& _findVariable(const PlexilVarId& var);

    const ExpressionId& findVariable(const PlexilVarRef* ref);

    const ExpressionId& findVariable(const LabelStr& name);

    ExecConnectorId& getExec() {return m_exec;}

    ExpressionId& getCondition(const LabelStr& name);

    double getAcknowledgementValue() const;

    void activatePair(const LabelStr& name);

    void deactivatePair(const LabelStr& name);

    bool pairActive(const LabelStr& name);

  protected:
    friend class PlexilExec;
    friend class InternalCondition;
    friend class StateComputer;
    friend class TransitionHandler;

    //create conditions, assignments, and commands.  We have to do this late because they could refer to internal variables of other nodes.
    void postInit();
    void commonInit();
  private:

    void createCommand(const PlexilCommandBody* body);

    void createAssignment(const PlexilAssignmentBody* body);

    void createUpdate(const PlexilUpdateBody* body);

    void createFunctionCall(const PlexilFunctionCallBody* funcCall);

    void createConditions(const std::map<std::string, PlexilExprId>& conds);

    void createChildNodes(const PlexilListBody* body);

    void createLibraryNode(const PlexilNodeId& node);

    void testLibraryNodeParameters(const PlexilNodeId& libNode, 
                                   const std::vector<PlexilVarRef*>& interfaceVars,
                                   PlexilAliasMap& aliases);

    void createDeclaredVars(const std::vector<PlexilVarId>& vars);

    void getVarsFromInterface(const PlexilInterfaceId& intf);

    void getVarsFromParent();

    void execute();

    void reset();

    void abort();

    void lockConditions();

    void unlockConditions();

    void deactivateExecutable();

    void cleanUpConditions();

    void cleanUpVars();

    const ExpressionId& getInternalVariable(const LabelStr& name);

    /**
     * @brief Sets the default variables for the conditions and establishes the internal conditions that are dependent on parent conditions
     *
     */
    void setConditionDefaults();

    NodeId m_id; /*<! The Id for this node*/
    ExecConnectorId m_exec; /*<! The executive (to notify it about condition changes and whether it needs to be executed)*/
    NodeId m_parent; /*<! The parent of this node.*/
    NodeConnectorId m_connector;
    PlexilNodeId m_node;
    bool sl_called, m_cleanedConditions, m_cleanedVars, m_transitioning;
    LabelStr m_lastQuery;
    double m_priority; /*<! The priority of this node */
    LabelStr m_nodeId;  /*<! the NodeId from the xml.*/
    LabelStr m_nodeType; /*<! The node type (either directly from the Node element or determined by the sub-elements.*/
    std::map<double, ExpressionId> m_variablesByName; /*<! Locally declared variables or references to variables gotten through an interface.
							Should there be an expression type for handling 'in' variables (i.e. a wrapper that fails on setValue)?
							I'll stick all variables in here, just to be safe.*/
    std::list<ExpressionId> m_localVariables; /*<! Variables created in this node*/
    std::map<double, ExpressionId> m_conditionsByName; /*<! The condition expressions.*/
    std::map<double, ExpressionListenerId> m_listenersByName; /*<! Listeners on the various condition expressions.  This allows us to turn them on/off when appropriate*/
    /*std::pair<ExpressionId, ExpressionId> m_assignment;*/ /*<! The assignment to be performed.  m_assignment.first->setValue(m_assignment.second->getValue()), essentially.
							      Since assignment is an external behavior now, we may just return this pair and batch it out to the external interface.*/
    AssignmentId m_assignment;
    CommandId m_command; /*<! The command to be performed. */
    UpdateId m_update;
    FunctionCallId m_functionCall;
    ExpressionId m_ack; /*<! The destination for acknowledgement of the command/assignment.  DON'T FORGET TO RESET THIS VALUE IN REPEAT-UNTILs! */
    std::list<NodeId> m_children; /*<! Child nodes.*/
    std::set<double> m_garbage;
    static unsigned int anonynode;
    ExpressionId m_extraEndCond;
    ExpressionId m_interruptEndCond;
    ExpressionId m_conjunctCondition;
    ExpressionId m_allCommandHandleValues;
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

  class Command {
  public:
    Command(const ExpressionId nameExpr, const std::list<ExpressionId>& args, 
            const ExpressionId dest, const ExpressionId ack,
	    const std::list<ExpressionId>& garbage,
            const std::vector<std::map<std::string, ExpressionId> >& resource);
    ~Command();
    CommandId& getId() {return m_id;}
    const LabelStr& getName();
    ExpressionId& getDest() {return m_dest;}
    ExpressionId& getAck() {return m_ack;}
    const std::list<double>& getArgValues() {return m_argValues;}
    const std::vector<std::map<std::string, double> >& getResourceValues() const 
    {return m_resourceValues;}
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
    ExpressionId m_ack;
    std::list<ExpressionId> m_garbage;
    std::list<double> m_argValues;
    std::vector<std::map<std::string, ExpressionId> > m_resourceList;
    std::vector<std::map<std::string, double> > m_resourceValues;
  };

  class Update {
  public:
    Update(const NodeId& node, const std::map<double, ExpressionId>& pairs, const ExpressionId ack,
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
    std::map<double, ExpressionId> m_pairs;
    ExpressionId m_ack;
    std::list<ExpressionId> m_garbage;
    std::map<double, double> m_valuePairs;
  };

  class FunctionCall {
  public:
    FunctionCall(const ExpressionId nameExpr, const std::list<ExpressionId>& args, const ExpressionId dest, const ExpressionId ack,
                 const std::list<ExpressionId>& garbage);
    ~FunctionCall();
    FunctionCallId& getId() {return m_id;}
    const LabelStr& getName();
    ExpressionId& getDest() {return m_dest;}
    ExpressionId& getAck() {return m_ack;}
    const std::list<double>& getArgValues() {return m_argValues;}
    void activate();
    void deactivate();
  protected:
    friend class Node;
    void fixValues();
  private:
    FunctionCallId m_id;
    LabelStr m_name;
    ExpressionId m_nameExpr;
    std::list<ExpressionId> m_args;
    ExpressionId m_dest;
    ExpressionId m_ack;
    std::list<ExpressionId> m_garbage;
    std::list<double> m_argValues;
  };

}

#endif
