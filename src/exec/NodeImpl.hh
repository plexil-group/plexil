// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef NODE_IMPL_HH
#define NODE_IMPL_HH

//! \def PARANOID_ABOUT_CONDITION_ACTIVATION
//! \brief When set, enables additional debugging checks on the node
//!        state transition logic.
// #define PARANOID_ABOUT_CONDITION_ACTIVATION 1

#include "Node.hh"
#include "NodeVariables.hh"
#include "Notifier.hh"

#include <memory> // std::unique_ptr

namespace PLEXIL
{

  // Forward declarations
  class Mutex;
  class NodeImpl;
  class NodeTimepointValue;
  class NodeVariableMap;

  // Type aliases
  using ExpressionPtr = std::unique_ptr<Expression>;
  using MutexPtr = std::unique_ptr<Mutex>;
  using NodeImplPtr = std::unique_ptr<NodeImpl>;
  using NodeTimepointValuePtr = std::unique_ptr<NodeTimepointValue>;
  using NodeVariableMapPtr = std::unique_ptr<NodeVariableMap>;

  //! \class NodeImpl
  //! \brief The innards shared between node implementation classes,
  //!        the XML parser, and external interfaces; also the
  //!        implementation class for empty nodes.
  class NodeImpl :
    public Node,
    public Notifier
  {
  public:

    //! \brief The worst possible node priority.  Used as the default
    //!        when no priority is specified.
    //! \note This used to be 100000000, which somehow gets printed as
    //!       scientific notation in XML and subsequently doesn't
    //!       parse correctly.
    //! \bug Check whether this note still applies.
    static constexpr int32_t WORST_PRIORITY = 100000;

    //! \brief An array of the names of all conditions.
    //! \see NodeImpl::ConditionIndex
    static char const * const ALL_CONDITIONS[];

    //! \enum ConditionIndex
    //! \brief An enumeration of every implemented node condition,
    //!        including internal conditions which cannot be specified
    //!        by users.  Used to index the node's internal array of
    //!        conditions
    //! \note The ordering of the enumeration values must remain
    //!       consistent with the order of NodeImpl::ALL_CONDITIONS.
    //! \see NodeImpl::ALL_CONDITIONS
    enum ConditionIndex
      {
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

        // Internal condition for all but Empty nodes
        actionCompleteIdx,

        // Internal condition for all but Empty and Update nodes
        abortCompleteIdx,

        conditionIndexMax,
      };

    //! \brief Primary constructor.
    //! \param nodeId The name of this node, as a const pointer to character string.
    //! \param parent (Optional) Pointer to the parent Node, if any.  Defaults to null.
    NodeImpl(char const *nodeId, NodeImpl *parent = nullptr);

    //! \brief Alternate constructor used only by unit tests.  Sets
    //!        the node's state to a known value for state transition
    //!        testing.
    //! \param type The name of the node type.
    //! \param name The node ID.
    //! \param state The initial NodeState value.
    //! \param parent (Optional) Pointer to the parent Node, if any.  Defaults to null.
    NodeImpl(const std::string& type,
             const std::string& name,
             NodeState state,
             NodeImpl *parent = nullptr);

    //! \brief Virtual destructor
    virtual ~NodeImpl();

    //
    // NodeConnector API
    //

    //! \brief Get the node's name.
    //! \return Const reference to the name string.
    virtual std::string const &getNodeId() const override
    {
      return m_nodeId;
    }

    //! \brief Look up a declared variable by name.
    //! \param name Name of the variable, as a const pointer to character string.
    //! \return Pointer to the variable, as an expression.  Will be null if not found.
    virtual Expression *findVariable(char const *name) override;

    //! \brief Notify the node that a resource on which it is pending has become available.
    virtual void notifyResourceAvailable() override;

    //
    // ExpressionListener API
    //
    
    //! \brief Notify this object of a change.
    virtual void notifyChanged() override;

    //
    // Listenable API
    //

    //! \brief Query whether this object is active.
    //! \return true if active, false if not.
    //! \note Overrides Notifier method.
    virtual bool isActive() const override
    {
      return true;
    }

    //! \brief Make this object active if it is not already.
    //! \note Overrides the Notifier method.
    virtual void activate() override
    {
    }

    //! \brief Request that this object become inactive if it is not already.
    //! \note Overrides the Notifier method.
    virtual void deactivate() override
    {
    }
    
    //! \brief Can this expression generate change notifications even
    //!        if none of its subexpressions change?
    //! \return True if the object can generate its own change
    //!         notifications, false if not.
    //! \note The NodeImpl method always returns true.
    virtual bool isPropagationSource() const override
    {
      return true;
    }

    //
    // Accessors of static node state
    //

    //! \brief Get the type of this node.
    //! \return The PlexilNodeType value.
    //! \brief This default method implements the Emoty node behavior.
    //!        Derived classes must override it.
    virtual PlexilNodeType getType() const override
    {
      return NodeType_Empty;
    }

    //! \brief Get the priority of this node.
    //! \return The priority.
    virtual int32_t getPriority() const override
    {
      return m_priority;
    }

    //! \brief Get the parent of this node.
    //! \return Const pointer to the parent Node, if any.
    virtual Node const *getParent() const override
    {
      return dynamic_cast<Node const *>(m_parent);
    }

    //
    // Accessors of dynamic node state
    //

    //! \brief Get the current state of this node.
    //! \return The current NodeState value.
    virtual NodeState getState() const override;

    //! \brief Get the outcome of this node.
    //! \return The NodeOutcome value.
    virtual NodeOutcome getOutcome() const override;

    //! \brief Get the failure type of this node.
    //! \return The FailureType value.
    virtual FailureType getFailureType() const override;

    //
    // Queue management API
    //
    
    //! \brief Get the pointer to the next item in the queue.
    //! \return The pointer.
    //! \see LinkedQueue
    virtual Node *next() const override
    {
      return static_cast<Node *>(m_next);
    }

    //! \brief Get the pointer to the pointer to the next item in the queue.
    //! \return The pointer.
    //! \see LinkedQueue
    virtual Node **nextPtr() override
    {
      return static_cast<Node **>(&m_next);
    }
        
    //! \brief Get the current queue status of this Node.
    //! \return The status value.
    virtual QueueStatus getQueueStatus() const override
    {
      return m_queueStatus;
    }

    //! \brief Update the queue status of this Node.
    //! \param newval The new status value.
    virtual void setQueueStatus(QueueStatus newval) override
    {
      m_queueStatus = newval;
    }

    //
    // Node state transition API
    //

    //! \brief Make the node and its expressions active, i.e. eligible
    //! for condition checks.
    virtual void activateNode() override;

    //! \brief Notify the node that something has changed, to make it
    //!        eligible for condition checking.
    //! \param exec Pointer to the PlexilExec instance.
    virtual void notifyChanged(PlexilExec *exec) override;

    //! \brief Calculate the next state of this node, were it to transition,
    //!        based on the values of its conditions.
    //! \return True if the new destination state is different from
    //!         the last check, false otherwise.
    virtual bool getDestState() override;

    //! \brief Get the previously calculated next state of this node.
    //! \return The cached next node state value.
    virtual NodeState getNextState() const override
    {
      return (NodeState) m_nextState;
    }

    //! \brief Commit a pending node state transition.
    //! \param exec Pointer to the PlexilExec instance.
    //! \param time The time of the transition.
    void transition(PlexilExec *exec, double time = 0.0) override;

    //
    // Resource conflict API
    //

    //! Does this node need to acquire resources before it can execute?
    //! @return true if resources must be acquired, false otherwise.
    virtual bool acquiresResources() const override;

    //! \brief Get an Assignment node's assigned variable.
    //! \return Pointer to an Assignable.  If node is not an
    //!         AssignmentNode, will be null.
    //! \note This default method always returns null.
    virtual Assignable *getAssignmentVariable() const override 
    {
      return nullptr;
    }

    //! \brief Attempt to reserve the resources needed by the node.
    //!        If the attempt fails, add the node to the resources'
    //!        wait lists.
    //! \return true if reservation was successful, false if not.
    virtual bool tryResourceAcquisition() override;

    //! \brief Remove the node from the pending queues of any
    //!        resources it was trying to acquire.
    virtual void releaseResourceReservations() override;

    //
    // Printed representation
    //

    //! \brief Print a representation this node to an output stream.
    //! \param stream The output stream.
    //! \param indent (Optional) The number of spaces to indent; default is 0.
    virtual void print(std::ostream& stream, const unsigned int indent = 0) const override;

    //! \brief Return a string with a printed representation of this node.
    //! \param indent (Optional) The number of spaces to indent; default is 0.
    virtual std::string toString(const unsigned int indent = 0) const override;

    //
    // Member functions unique to this class and its derivatives
    //

    //! \brief Set priority of a node.
    //! \param prio The new priority.
    //! \note Should only be used by the plan parser.
    void setPriority(int32_t prio)
    {
      m_priority = prio;
    }

    //! \brief Get a pointer to the node's parent NodeImpl.
    //! \return Pointer to the parent node.
    NodeImpl *getParentNode()
    {
      return m_parent;
    }

    //! \brief Get a const pointer to the node's parent NodeImpl.
    //! \return Const pointer to the parent node.
    NodeImpl const *getParentNode() const
    {
      return m_parent;
    }

    //! \brief Update the node state.
    //! \param exec The Exec to notify of the change.
    //! \param newValue The new node state.
    //! \param tym The time of transition.
    //! \note This member function is virtual, so it can be overridden
    //!       by ListNode wrapper method.
    //! \note Should only be used by node implementation classes and
    //!       unit tests.
    virtual void setState(PlexilExec *exec, NodeState newValue, double tym);

    //! \brief Set the node's failure type.
    //! \param f The new FailureType value.
    //! \note Should only be used by NodeImpl::transition() and exec unit tests.
    void setNodeFailureType(FailureType f);

    //! \brief Get the time at which this node entered its current state.
    //! \return The time value, as a double (Real) value.
    //! \note Used by PlanDebugListener.
    double getCurrentStateStartTime() const;

    //! \brief Get the time at which this node entered a particular state.
    //! \param state The NodeState value.
    //! \return The time value, as a double (Real) value.  If the node
    //!          has not yet entered the requested state, returns -DBL_MAX.
    //! \note Used by PlanDebugListener.
    double getStateStartTime(NodeState state) const;

    //! \brief Find the named variable in this node, ignoring its ancestors.
    //! \param name Name of the variable, as a pointer to const character string.
    //! \return Pointer to the variable.  Will be null if no variable with that name was declared in this node.
    //! \note Used only by the plan parser and its unit tests.
    Expression *findLocalVariable(char const *name);
  
    //! \brief Get the children of this node.
    //! \return Reference to a vector of pointers to the children.
    //! \note Only ListNode and LibraryCallNode have chidren.
    //! \note The default method returns an empty vector.
    virtual std::vector<NodeImplPtr> &getChildren();

    //! \brief Get the children of this node.
    //! \return Const reference to a vector of pointers to the children.
    //! \note Only ListNode and LibraryCallNode have chidren.
    //! \note The default method returns an empty vector.
    virtual const std::vector<NodeImplPtr> &getChildren() const;

    //! \brief Find a child node of this node with the given name.
    //! \param childName The name, as a pointer to const character string.
    //! \return Const pointer to the child node; null if no child with that name exists.
    //! \note Only ListNode and LibraryCallNode have chidren.
    //! \note The default method always returns null.
    virtual NodeImpl const *findChild(char const *childName) const;

    //! \brief Find a child node of this node with the given name.
    //! \param childName The name, as a pointer to const character string.
    //! \return Pointer to the child; null if no child with that name exists.
    //! \note Only ListNode and LibraryCallNode have chidren.
    //! \note The default method always returns null.
    virtual NodeImpl *findChild(char const *childName);

    //
    // Utilities for plan parser and analyzer
    //

    //! \brief Reserve space in the node's variable vector and variable map.
    //! \param n The number of entries to reserve.
    void allocateVariables(size_t nVars);

    //! \brief Add a named "variable" to the node, to be deleted with the node.
    //! \param name The name as a pointer to const character string.
    //! \param var Pointer to the expression to associate with the name.
    //!            It will be deleted when the node is deleted.
    //! \return true if successful, false if name is a duplicate
    bool addLocalVariable(char const *name, Expression *var);

    //! \brief Pre-allocate the node's vector of declared mutexes to the requested size.
    //! \param n The size.
    //! \note This is an optimization for use by the plan parser.
    void allocateMutexes(size_t n);

    //! \brief Pre-allocate the vector of mutexes used by this node to
    //! the requested size.
    //! \param n The size.
    //! \note This is an optimization for use by the plan parser.
    void allocateUsingMutexes(size_t n);

    //! \brief Add this mutex to the mutexes declared by the node.
    //! \param m Pointer to the mutex.
    void addMutex(Mutex *m);

    //! \brief Add this mutex to the mutexes used by the node.
    //! \param m Pointer to the mutex.
    void addUsingMutex(Mutex *m);

    //! \brief Look up a mutex by name.  Searches ancestors and globals.
    //! \param name Name of the mutex, as a const pointer to character string.
    //! \return The mutex.  Will be null if no mutex with the given
    //!         name is in scope for the node.
    Mutex *findMutex(char const *name) const;

    //! \brief Get the map of variables referenced in this node.
    //! \return Const pointer to the map.  May be null.
    NodeVariableMap const *getVariableMap() const
    {
      return m_variablesByName.get();
    }

    //! \brief Add a condition expression to the node.
    //! \param cname The name of the condition as a pointer to const character string.
    //! \param cond Pointer to the expression.
    //! \param isGarbage True if the expression should be deleted with the node, false otherwise.
    void addUserCondition(char const *cname, Expression *cond, bool isGarbage);

    //! \brief Construct any internal conditions now that the node is complete.
    void finalizeConditions();
    
    //! \brief Given a ConditionIndex value, get the name of the condition.
    //! \param idx The ConditionIndex value.
    //! \return Const pointer to the name, as a character string.
    //!         Will be null if the index is not associated with a
    //!         condition name.
    //! \note Used by various debugging utilities.
    static char const *getConditionName(size_t idx);

    //! \brief Get a pointer to the node's state variable.
    //! \return Pointer to the variable.
    //! \note Only used by plan parser.
    Expression *getStateVariable()
    {
      return &m_stateVariable;
    }

    //! \brief Get a pointer to the node's outcome variable.
    //! \return Pointer to the variable.
    //! \note Only used by plan parser.
    Expression *getOutcomeVariable()
    {
      return &m_outcomeVariable;
    }

    //! \brief Get a pointer to the node's failure type variable.
    //! \return Pointer to the variable.
    //! \note Only used by plan parser.
    Expression *getFailureTypeVariable()
    {
      return &m_failureTypeVariable;
    }

    //! \brief Get a pointer to the variable representing the named
    //!        node state transition timepoint.
    //! \param st The desired NodeState.
    //! \param isEnd true if the end timepoint is desired, false if
    //!              the start timepoint is desired.
    //! \return Pointer to the timepoint variable.  Will be
    //!         constructed if it does not already exist.
    //! \note Only used by plan parser.
    Expression *ensureTimepoint(NodeState st, bool isEnd);

    //! \brief Get the node's locally declared variables.
    //! \return Const pointer to the vector of variable pointers.
    //! \note Used by plan parser, plan analyzer, and parser unit tests.
    const std::vector<ExpressionPtr> *getLocalVariables() const
    {
      return m_localVariables.get();
    }

    // Condition accessors
    // These are public only to appease the module test

    ///@{
    //! \brief Accessor for the named ancestor condition.
    //! \return Pointer to the condition expression.  May be null.
    //! \note The condition expression is owned by the parent node.
    Expression *getAncestorEndCondition()               { return getCondition(ancestorEndIdx); }
    Expression *getAncestorExitCondition()              { return getCondition(ancestorExitIdx); }
    Expression *getAncestorInvariantCondition()         { return getCondition(ancestorInvariantIdx); }
    ///@}

    ///@{
    //! \brief Accessor for the named user condition.
    //! \return Pointer to the condition expression.  May be null.
    Expression *getSkipCondition()                      { return m_conditions[skipIdx]; }
    Expression *getStartCondition()                     { return m_conditions[startIdx]; }
    Expression *getEndCondition()                       { return m_conditions[endIdx]; }
    Expression *getExitCondition()                      { return m_conditions[exitIdx]; }
    Expression *getInvariantCondition()                 { return m_conditions[invariantIdx]; }
    Expression *getPreCondition()                       { return m_conditions[preIdx]; }
    Expression *getPostCondition()                      { return m_conditions[postIdx]; }
    Expression *getRepeatCondition()                    { return m_conditions[repeatIdx]; }
    ///@}


    ///@{
    //! \brief Accessor for the named user condition.
    //! \return Pointer to the condition expression.  May be null.
    Expression *getActionCompleteCondition()            { return m_conditions[actionCompleteIdx]; }
    Expression *getAbortCompleteCondition()             { return m_conditions[abortCompleteIdx]; }
    ///#}

    //! \brief Get the condition indicated by the index.
    //! \param idx A valid ConditionIndex value.
    //! \return Const pointer to the requested condition.  May be null.
    //! \note Used by LuvListener and other debug utilities.
    //! \note The non-const variant of this member function is protected.
    Expression const *getCondition(size_t idx) const;

  protected:

    friend class ListNode;

    //! \brief Get the condition indicated by the index.
    //! \param idx A valid ConditionIndex value.
    //! \return Pointer to the requested condition.  May be null.
    Expression *getCondition(size_t idx);

    //! \brief Get the map of variables accessible to children of this node.
    //! \return Const pointer to the map.  May return null.
    //! \note This default method always returns null.
    virtual NodeVariableMap const *getChildVariableMap() const;

    //! \brief Perform common initializations used by both constructors.
    void commonInit();

    //! \brief Execute the node.
    //! \param exec The PlexilExec.
    void execute(PlexilExec *exec);

    //! \brief Reset the node to its initial state.
    void reset();

    //! \brief Deactivate any resources used in executing the node.
    //! \param exec The PlexilExec.
    void deactivateExecutable(PlexilExec *exec);

    //! \brief Activate all local variables in preparation for
    //! executing the node.
    void activateLocalVariables();

    //! \brief Deactivate all local variables.
    void deactivateLocalVariables();

    // Activate conditions

    //! \brief Activate the ancestor-end condition for the node, if any.
    //! \note The node's parent owns the condition expression.
    void activateAncestorEndCondition();

    //! \brief Activate the ancestor-exit and ancestor-invariant
    //!        condition for the node, if any.
    //! \note The node's parent owns the condition expression.
    void activateAncestorExitInvariantConditions();

    //! \brief Activate the node's precondition, skip condition, and
    //! start condition, if any.
    void activatePreSkipStartConditions();

    //! \brief Activate the node's end condition, if any.
    void activateEndCondition();

    //! \brief Activate the node's exit condition, if any.
    void activateExitCondition();

    //! \brief Activate the node's invariant condition, if any.
    void activateInvariantCondition();

    //! \brief Activate the node's postcondition, if any.
    void activatePostCondition();

    //! \brief Activate the node's repeat condition, if any.
    void activateRepeatCondition();

    //! \brief Activate the node's action-complete condition, if any.
    void activateActionCompleteCondition();

    //! \brief Activate the node's abort-complete condition, if any.
    void activateAbortCompleteCondition();

    // Deactivate a condition

    //! \brief Deactivate the ancestor-end condition for the node, if any.
    //! \note The node's parent owns the condition expression.
    void deactivateAncestorEndCondition();

    //! \brief Deactivate the ancestor-exit and ancestor-invariant
    //!        condition for the node, if any.
    //! \note The node's parent owns the condition expression.
    void deactivateAncestorExitInvariantConditions();

    //! \brief Deactivate the node's precondition, skip condition, and
    //! start condition, if any.
    void deactivatePreSkipStartConditions();

    //! \brief Deactivate the node's end condition, if any.
    void deactivateEndCondition();

    //! \brief Deactivate the node's exit condition, if any.
    void deactivateExitCondition();

    //! \brief Deactivate the node's invariant condition, if any.
    void deactivateInvariantCondition();

    //! \brief Deactivate the node's postcondition, if any.
    void deactivatePostCondition();

    //! \brief Deactivate the node's repeat condition, if any.
    void deactivateRepeatCondition();

    //! \brief Deactivate the node's action-complete condition, if any.
    void deactivateActionCompleteCondition();

    //! \brief Deactivate the node's abort-complete condition, if any.
    void deactivateAbortCompleteCondition();

    // Specific behaviors for derived classes

    //! \brief Create any "wrapper" condition expressions required by the node type.
    virtual void specializedCreateConditionWrappers();

    //! \brief Perform any specialized activation actions required by the node type.
    virtual void specializedActivate();

    //! \brief Perform execution as required by the node type.
    //! \param exec Pointer to the PlexilExec.
    virtual void specializedHandleExecution(PlexilExec *exec);

    //! \brief Perform any specialized activation actions required by the node type.
    //! \param exec Pointer to the PlexilExec.
    virtual void specializedDeactivateExecutable(PlexilExec *exec);

    //
    // State transition implementation methods
    //
    // Non-virtual member functions are common to all node types.
    // Virtual members are specialized by node type.
    //

    //! \brief Compute the next state from the INACTIVE state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    bool getDestStateFromInactive();

    //! \brief Compute the next state from the WAITING state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    bool getDestStateFromWaiting();

    //! \brief Compute the next state from the EXECUTING state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    virtual bool getDestStateFromExecuting();

    //! \brief Compute the next state from the FINISHING state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    virtual bool getDestStateFromFinishing();

    //! \brief Compute the next state from the FINISHED state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    bool getDestStateFromFinished();

    //! \brief Compute the next state from the FAILING state.  Set
    //!        m_nextState, m_nextOutcome, m_nextFailureType as a side
    //!        effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    virtual bool getDestStateFromFailing();

    //! \brief Compute the next state from the ITERATION_ENDED state.
    //!        Set m_nextState, m_nextOutcome, m_nextFailureType as a
    //!        side effect.
    //! \return true if the new destination state is different from the last check, false otherwise.
    bool getDestStateFromIterationEnded();

    //
    // Transition out of the named current state.
    //

    //! \brief Transition out of the INACTIVE state.
    void transitionFromInactive();

    //! \brief Transition out of the WAITING state.
    void transitionFromWaiting();

    //! \brief Transition out of the EXECUTING state.
    virtual void transitionFromExecuting(PlexilExec *exec);

    //! \brief Transition out of the FINISHING state.
    virtual void transitionFromFinishing(PlexilExec *exec);

    //! \brief Transition out of the FINISHED state.
    void transitionFromFinished();

    //! \brief Transition out of the FAILING state.
    virtual void transitionFromFailing(PlexilExec *exec);

    //! \brief Transition out of the ITERATION_ENDED state.
    void transitionFromIterationEnded();

    //
    // Transition into the named next state.
    //

    //! \brief Transition into the INACTIVE state.
    void transitionToInactive();

    //! \brief Transition into the WAITING state.
    void transitionToWaiting();

    //! \brief Transition into the EXECUTING state.
    virtual void transitionToExecuting();

    //! \brief Transition into the FINISHING state.
    virtual void transitionToFinishing();

    //! \brief Transition into the FINISHED state.
    virtual void transitionToFinished();

    //! \brief Transition into the FAILING state.
    virtual void transitionToFailing(PlexilExec *exec);

    //! \brief Transition into the ITERATION_ENDED state.
    virtual void transitionToIterationEnded(); 

    //
    // Destructor helper member functions
    //

    //! \brief Delete any unique expressions used by the node's conditions.
    //! \warning Not useful if called from the base class destructor.
    virtual void cleanUpConditions();

    //! \brief Delete any unique expressions used by the node's body.
    //! \warning Not useful if called from the base class destructor.
    virtual void cleanUpNodeBody();

    //! \brief Delete all local variables allocated by this node.
    void cleanUpVars();

    //
    // Printing utility
    //

    //! \brief Print the node's command handle (acknowledgement) value, if any.
    //! \param stream The output stream.
    //! \indent indent The number of spaces to indent.
    virtual void printCommandHandle(std::ostream& stream, const unsigned int indent) const;

    //
    // Common state
    //

    Node        *m_next;                //!< For LinkedQueue<Node>
    QueueStatus  m_queueStatus;         //!< Which exec queue the node is in, if any.
    NodeState    m_state;               //!< The current state of the node.
    NodeOutcome  m_outcome;             //!< The current outcome.
    FailureType  m_failureType;         //!< The current failure.

    bool         m_pad; // to ensure 8 byte alignment
    NodeState    m_nextState;           //!< The state returned by getDestState() the last time checkConditions() was called.
    NodeOutcome  m_nextOutcome;         //!< The pending outcome.
    FailureType  m_nextFailureType;     //!< The pending failure.

    NodeImpl    *m_parent;                          //!< The parent of this node.*/
    Expression  *m_conditions[conditionIndexMax]; //!< The condition expressions.
 
    std::unique_ptr<std::vector<ExpressionPtr>> m_localVariables;  //!< Variables created in this node.
    std::unique_ptr<std::vector<MutexPtr>>      m_localMutexes;    //!< Mutexes created in this node.
    std::unique_ptr<std::vector<Mutex *>>       m_usingMutexes;    //!< Mutexes to be acquired by this node.

    StateVariable    m_stateVariable;
    OutcomeVariable  m_outcomeVariable;
    FailureVariable  m_failureTypeVariable;

    NodeVariableMapPtr m_variablesByName;  //!< Locally declared variables or references to variables gotten through an interface.
    std::string        m_nodeId;           //!< The NodeId.
    int32_t            m_priority;         //!< The priority.

  private:
    
    //
    // Node transition history trace
    //

    double m_currentStateStartTime;        //!< The time of the last node state transition.
    NodeTimepointValuePtr m_timepoints;    //!< Pointer to the structure holding the state transition history.

  protected:

    //
    // Housekeeping details
    //

    bool m_garbageConditions[conditionIndexMax]; //!< Flags indicating which conditions to delete.
    bool m_cleanedBody;                          //!< true if node body has been cleaned up, false otherwise.
    bool m_cleanedConditions;                    //!< true if node conditions have been cleaned up, false otherwise.
    bool m_cleanedVars;                          //!< true if node variables have been cleaned up, false otherwise.

  private:

    //! \brief Create any required "wrapper" expressions around user specified conditions.
    //! \see NodeImpl::specializedCreateConditionWrappers
    void createConditionWrappers();

    //! \brief Set the node's outcome.
    //! \param o The new NodeOutcome value.
    void setNodeOutcome(NodeOutcome o);
    
    //! \brief Log the transition from the previous state to the specified state.
    //! \param time The time to log for the transition.
    //! \param newState The state being transitioned into.
    void logTransition(double time, NodeState newState);

    //
    // Debug utilities
    //

    //! \brief Print the node's local variables and their values.
    //! \param stream The output stream.
    //! \param indent (Optional) The number of spaces to indent.
    //! \note Used for debug printing.
    void printVariables(std::ostream& stream, const unsigned int indent = 0) const;

    //! \brief Print the node's mutexes and their current status.
    //! \param stream The output stream.
    //! \param indent (Optional) The number of spaces to indent.
    //! \note Used for debug printing.
    void printMutexes(std::ostream& stream, const unsigned int indent = 0) const;

  };

}

#endif // NODE_IMPL_HH
