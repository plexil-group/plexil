/* Copyright (c) 2006-2017, Universities Space Research Association (USRA).
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

#include <memory> // std::unique_ptr

// Take care of annoying VxWorks macro
#undef UPDATE

// *** For debug use only ***
// Uncomment this if we don't trust the condition activation/deactivation logic
// #define PARANOID_ABOUT_CONDITION_ACTIVATION 1

namespace PLEXIL {

  // Forward references
  class Mutex;
  class NodeTimepointValue;
  class NodeVariableMap;

  //
  // QueueStatus - Used by PlexilExec and Node classes for queue management
  // There are four queues:
  //  - the candidate queue, scheduled for condition evaluation;
  //  - the pending queue, eligible for execution and waiting on one or more mutexes;
  //  - the transition queue, nodes in the process of state transition; and
  //  - the deletion queue, top level nodes which have run and are no longer active.
  //
  // A node can be in at most one queue at any time.
  // A node in the candidate queue cannot be re-added before its conditions are evaluated.
  // A node in the pending queue can have a condition check scheduled, or be notified
  //  that a mutex it's waiting on has been released, or both.
  //
  enum QueueStatus : uint8_t {
    QUEUE_NONE = 0,          // not in any queue
    QUEUE_CHECK,             // in check-conditions queue
    QUEUE_PENDING_TRY,       // just added, or waiting for a mutex which was just released
    QUEUE_PENDING_TRY_CHECK, // just added, or waiting for a mutex which was just released
                             // AND check-conditions requested
    QUEUE_PENDING,           // waiting for a mutex
    QUEUE_PENDING_CHECK,     // waiting for a mutex AND check-conditions requested
    QUEUE_TRANSITION,        // in state transition queue
    QUEUE_TRANSITION_CHECK,  // in state transition queue AND check-conditions requested
    QUEUE_DELETE             // no longer eligible to transition
  };

  

  /**
   * @brief The class representing a Node in the plan--either a list of sub-Nodes, an assignment, or a command execution.
   * There is a possible refactoring here, breaking the three node types into subclasses.  Unfortunately, the XML format doesn't
   * currently support that.
   * There is a fair amount of work to be done here.  For instance, implementing the entire state transition graph and
   * responses to the various conditions changing value, accessors for the node information and the conditions, there should
   * be error checking in all of the state transitions for node types (FAILING, for instance, can only be occupied by list nodes).
   */
  class Node :
    public NodeConnector,
    public ExpressionListener
  {
  public:
    static char const * const ALL_CONDITIONS[];

    // N.B.: These need to match the order of ALL_CONDITIONS above
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

    /**
     * @brief The constructor.
     * @param nodeId The name of this node.
     * @param parent The parent of this node (used for the ancestor conditions and variable lookup).
     */
    Node(char const *nodeId, Node *parent = NULL);

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
    // LinkedQueue API
    //

    Node *next() const
    {
      return m_next;
    }

    Node **nextPtr()
    {
      return &m_next;
    }

    //
    // NodeConnector API to expressions
    //

    /**
     * @brief Looks up a variable by name.
     */
    Expression *findVariable(char const *name);

    /**
     * @brief Find the named variable in this node, ignoring its ancestors.
     * @param name Name of the variable.
     * @return The variable, or NULL if not found.
     */
    Expression *findLocalVariable(char const *name);

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
     */
    Integer getPriority() const
    {
      return m_priority;
    }

    /**
     * @brief Set the node's priority.
     * @param prio The priority.
     * @note Should only be used by plan parser and unit tests.
     */
    void setPriority(Integer prio)
    {
      m_priority = prio;
    }

    //
    // ExpressionListener API
    //

    virtual void notifyChanged();

    /**
     * @brief Gets the destination state of this node, were it to transition, based on the values of various conditions.
     * @return True if the node is eligible to transition, false otherwise.
     * @note Sets m_nextState, m_nextOutcome, m_nextFailureType as a side effect.
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
     * @brief Gets the time at which this node entered its current state.
     * @return Time value as a double.
     * @note Used by GanttListener and PlanDebugListener.
     */
    double getCurrentStateStartTime() const;

    // Used by plan parser
    
    // May return NULL.
    // Used by plan analyzer and plan parser module test only.
    const std::vector<std::unique_ptr<Expression> > *getLocalVariables() const
    { return m_localVariables; }

    // May return NULL.
    // Used by GanttListener.
    NodeVariableMap const *getVariableMap() const { return m_variablesByName; }

    // Pre-allocate local variable vector, variable map.
    void allocateVariables(size_t n);

    std::vector<Mutex *> const *getUsingMutexes() const { return m_usingMutexes; }

    // Pre-allocate mutex vector.
    void allocateMutexes(size_t n);
    void allocateUsingMutexes(size_t n);

    // Add a mutex.
    void addMutex(Mutex *m);
    void addUsingMutex(Mutex *m);

    /**
     * @brief Looks up a mutex by name. Searches ancestors and globals.
     * @param name Name of the mutex.
     * @return The mutex, or NULL if not found.
     */
    Mutex *findMutex(char const *name);

    /**
     * @brief Find the named mutex in this node, ignoring its ancestors
     *        and any global declarations.
     * @param name Name of the mutex.
     * @return The mutex, or NULL if not found.
     */
    Mutex *findLocalMutex(char const *name);

    /**
     * @brief Notify that a mutex on which we're pending is now available.
     */
    void notifyMutexAvailable();

    /**
     * @brief Mark the node as eligible for recheck of conditions.
     * @return true if it should be added to candidate queue, false otherwise
     */
    bool scheduleCheckConditions();

    /**
     * @brief Clear the check-conditions "flag".
     * @return The resulting QueueStatus.
     */
    QueueStatus conditionsChecked();

    virtual std::vector<Node *>& getChildren();
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

    // For use of plan parser.
    Expression *ensureTimepoint(NodeState st, bool isEnd);

    /**
     * @brief Accessor for an assignment node's assigned variable.
     */
    virtual Expression *getAssignmentVariable() const 
    {
      return NULL;
    }

    /**
     * @brief Gets the type of this node.
     * @return The type of this node.
     * @note Empty node method.
     */
    virtual PlexilNodeType getType() const
    {
      return NodeType_Empty;
    }

    virtual Node const *findChild(char const *childName) const;
    virtual Node *findChild(char const *childName);

    //
    // For convenience of PlexilExec queue management
    //

    QueueStatus getQueueStatus() const
    {
      return m_queueStatus;
    }

    void setQueueStatus(QueueStatus newval)
    {
      m_queueStatus = newval;
    }

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

    // Used internally, also by LuvListener. Non-const variant is protected.
    Expression const *getCondition(size_t idx) const;

    //
    // Utilities for plan parser and analyzer
    //

    /**
     * @brief Add a named "variable" to the node, to be deleted with the node.
     * @param name The name
     * @param var The expression to associate with the name.
     *            It will be deleted when the node is deleted.
     * @return true if successful, false if name is a duplicate
     */
    bool addLocalVariable(char const *name, Expression *var);

    /**
     * @brief Add a condition expression to the node.
     * @param cname The name of the condition.
     * @param cond The expression.
     * @param isGarbage True if the expression should be deleted with the node.
     */
    void addUserCondition(char const *cname, Expression *cond, bool isGarbage);

    /**
     * @brief Construct any internal conditions now that the node is complete.
     */
    void finalizeConditions();
    
    // Public only for plan analyzer
    static char const *getConditionName(size_t idx);

  protected:
    friend class LibraryCallNode;
    friend class ListNode;

    friend class PlexilExec;
    friend class InternalCondition;

    friend class FailureVariable;
    friend class OutcomeVariable;
    friend class StateVariable;

    // Abstracts out the issue of where the condition comes from.
    Expression *getCondition(size_t idx);

    static ConditionIndex getConditionIndex(char const *cName);

    // Only used by Node, ListNode, LibraryCallNode.
    virtual NodeVariableMap const *getChildVariableMap() const;

    void commonInit();

    // Called from the transition handler
    void execute();
    void reset();
    virtual void abort();
    void deactivateExecutable();

    // Variables
    void activateLocalVariables();
    void deactivateLocalVariables();

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
    virtual void specializedCreateConditionWrappers();
    virtual void specializedActivate();
    virtual void specializedHandleExecution();
    virtual void specializedDeactivateExecutable();

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
    // Common state
    //

    Node        *m_next;                /*!< For LinkedQueue<Node> */
    QueueStatus  m_queueStatus;         /*!< Which exec queue the node is in, if any. */
    NodeState    m_state;               /*!< The current state of the node. */
    NodeOutcome  m_outcome;             /*!< The current outcome. */
    FailureType  m_failureType;         /*!< The current failure. */

    bool         m_pad;                 /*!< Ensure 4-byte alignment */
    NodeState    m_nextState;           /*!< The state calculated by getDestState() the last time checkConditions() was called. */
    NodeOutcome  m_nextOutcome;         /*!< The pending outcome. */
    FailureType  m_nextFailureType;     /*!< The pending failure. */

    Node *m_parent;                              /*!< The parent of this node.*/
    Expression *m_conditions[conditionIndexMax]; /*!< The condition expressions. */
 
    std::vector<std::unique_ptr<Expression> > *m_localVariables; /*!< Variables created in this node. */
    std::vector<std::unique_ptr<Mutex> > *m_localMutexes; /*!< Mutexes created in this node. */
    std::vector<Mutex *> *m_usingMutexes; /*!< Mutexes required by this node. */
    StateVariable m_stateVariable;
    OutcomeVariable m_outcomeVariable;
    FailureVariable m_failureTypeVariable;
    NodeVariableMap *m_variablesByName; /*!< Locally declared variables or references to variables gotten through an interface. */
    std::string m_nodeId;  /*!< the NodeId from the xml.*/

  private:
    
    // Node transition history trace
    double m_currentStateStartTime;
    NodeTimepointValue *m_timepoints;

  protected:

    Integer m_priority;

    // Housekeeping details
    bool m_garbageConditions[conditionIndexMax]; /*!< Flags for conditions to delete. */
    bool m_cleanedConditions, m_cleanedVars, m_cleanedBody;

  private:

    void createConditionWrappers();

    // These should only be called from transition().
    void setNodeOutcome(NodeOutcome o);
    void transitionFrom();
    void transitionTo(double tym); // FIXME
    void logTransition(double time, NodeState newState);

    // Subfunction of deactivateExecutable()
    void releaseMutexes();

    //
    // Internal versions
    //

    void printVariables(std::ostream& stream, const unsigned int indent = 0) const;
  };

  std::ostream& operator<<(std::ostream& strm, const Node& node);

}

#endif
