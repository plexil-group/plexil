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

#ifndef PLEXIL_COMMAND_IMPL_HH
#define PLEXIL_COMMAND_IMPL_HH

#include "Command.hh"
#include "CommandFunction.hh"
#include "CommandHandleVariable.hh"
#include "SimpleBooleanVariable.hh"

#include <memory> // std::unique_ptr

namespace PLEXIL
{

  // Forward references
  class ExprVec;
  class ResourceArbiterInterface;

  //! \struct ResourceValue
  //! \brief A structure to represent fixed resource values.  Used by
  //!        Command and ResourceArbiterInterface.
  struct ResourceValue final
  {
    //! \brief The resource name.
    std::string name;

    //! \brief The lower bound of the resource.
    //! \deprecated Resource lower bound will be removed in a future PLEXIL release.
    double lowerBound;

    //! \brief The upper bound of the resource.
    double upperBound;

    //! \brief The priority of the resource.
    //! \deprecated Resource priority will be removed in a future PLEXIL release.
    int32_t priority;

    //! \brief Whether the resource is returned when the command has
    //!        completed.
    bool releaseAtTermination;
  };

  using ResourceValueList = std::vector<ResourceValue>;

  //! \class ResourceSpec
  //! \brief Internal representation for a resource specification.
  //! \note Used only in CommandImpl class, but exposed to parser
  class ResourceSpec final
  {
  public:
    //! \brief Default constructor.
    ResourceSpec() = default;

    //! \brief Move constructor.
    ResourceSpec(ResourceSpec &&) = default;

    //! \brief Move assignment operator.
    ResourceSpec &operator=(ResourceSpec &&) = default;

    //! \brief Destructor.
    ~ResourceSpec();

    //! \brief Are all expressions associated with this spec constants?
    //! \return true if all are constant, false if any are not.
    bool isConstant() const;

    //! \brief Activate the expressions associated with this
    //!        ResourceSpec.
    void activate();

    //! \brief Deactivate the expressions associated with this
    //!        ResourceSpec.
    void deactivate();

    //
    // Plan reader access
    //

    //! \brief Set the resource name expression.
    //! \param e Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted with
    //!        the ResourceSpec.
    void setNameExpression(Expression *e, bool isGarbage);

    //! \brief Set the priority expression.
    //! \param e Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted with
    //!        the ResourceSpec.
    //! \deprecated Resource priority will be removed in a future PLEXIL release.
    void setPriorityExpression(Expression *e, bool isGarbage);

    //! \brief Set the resource lower bound expression.
    //! \param e Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted with
    //!        the ResourceSpec.
    //! \deprecated Resource lower bound will be removed in a future PLEXIL release.
    void setLowerBoundExpression(Expression *e, bool isGarbage);

    //! \brief Set the resource upper bound expression.
    //! \param e Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted with
    //!        the ResourceSpec.
    void setUpperBoundExpression(Expression *e, bool isGarbage);

    //! \brief Set the resource release-at-termination expression.
    //! \param e Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted with
    //!        the ResourceSpec.
    void setReleaseAtTerminationExpression(Expression *e, bool isGarbage);

    //! \brief The expression giving the resource name.
    Expression *nameExp;

    //! \brief The expression giving the resource priority.
    //! \deprecated Resource priority will be removed in a future PLEXIL release.
    Expression *priorityExp;

    //! \brief The expression giving the resource lower bound.
    //! \deprecated Resource lower bound will be removed in a future PLEXIL release.
    Expression *lowerBoundExp;

    //! \brief The expression giving the resource upper bound.
    Expression *upperBoundExp;

    //! \brief The expression giving the resource
    //!        release-at-termination flag.
    Expression *releaseAtTermExp;

  private:

    // Copy constructor, assignment not implemented
    ResourceSpec(ResourceSpec const &) = delete;
    ResourceSpec &operator=(ResourceSpec const &) = delete;

    ///@{
    //! \brief If true, the destructor should delete the associated
    //!        expression; if false, the expression is owned by another
    //!        object.
    bool nameIsGarbage;
    bool priorityIsGarbage;
    bool lowerBoundIsGarbage;
    bool upperBoundIsGarbage;
    bool releaseIsGarbage;
    ///@}

  };

  using ResourceSpecList = std::vector<ResourceSpec>;

  //! \class CommandImpl
  //! \brief The implementation class for PLEXIL commands.
  class CommandImpl final : public Command
  {
    friend class CommandHandleVariable;

  public:

    //! \brief Constructor.
    //! \param nodeName The name of the CommandNode which owns this
    //!        command.
    CommandImpl(std::string const &nodeName);

    //! \brief Virtual destructor.
    virtual ~CommandImpl();

    //
    // Command API
    //

    //! \brief Accessor for a command's name and parameters, as a
    //!        State instance.
    //! \return Const reference to the State.
    virtual State const &getCommand() const;

    //! \brief Accessor for the command's name.
    //! \return Const reference to the name as a std::string.
    virtual std::string const &getName() const;

    //! \brief Accessor for the command's parameters, if any.
    //! \return Const reference to a std::vector of Value instances.
    virtual std::vector<Value> const &getArgValues() const;

    //! \brief Is this command expected to return a value?
    //! \return true if a return value is expected, false if not.
    //! \note For the benefit of TestExec.
    virtual bool isReturnExpected() const;

    //! \brief Get the list of fixed resource values for the command.
    //! \return Const reference to the resource list.
    ResourceValueList const &getResourceValues() const;

    //! \brief Get the current value of the command handle (status)
    //!        variable.
    //! \return The value.
    CommandHandleValue getCommandHandle() const;

    //! \brief Get the expression which is to receive the return value
    //!        from this command.
    //! \return Pointer to the Expression.  May be null.
    Expression *getDest();

    //! \brief Get the command handle variable from this command.
    //! \return Pointer to the Expression.
    Expression *getAck() {return &m_ack;}

    //! \brief Get the abort-complete variable from this command.
    //! \return Pointer to the Expression.
    Expression *getAbortComplete() {return &m_abortComplete;}

    //! \brief Get the command-handle-known function from this command.
    //! \return Pointer to the Expression.
    Expression *getCommandHandleKnownFn() { return &m_handleKnownFn; }

    //! \brief Is this command active?
    //! \return true if active, false if not.
    bool isActive() const { return m_active; }

    //
    // Interface to plan parser
    //

    //! \brief Set the variable to receive the command's return value.
    //! \param dest Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted by
    //!                  the command's destructor.
    //! \note It is an error to call this method after the command has
    //!       been activated.
    void setDestination(Expression *dest, bool isGarbage);

    //! \brief Set the command name expression.
    //! \param dest Pointer to the Expression.
    //! \param isGarbage If true, the expression will be deleted by
    //!                  the command's destructor.
    //! \note It is an error to call this method after the command has
    //!       been activated.
    void setNameExpr(Expression *nameExpr, bool isGarbage);

    //! \brief Set the command's argument vector.
    //! \param dest Pointer to the vector.
    //! \note Ownership of the vector is transferred to the command instance.
    //! \note It is an error to call this method after the command has
    //!       been activated.
    void setArgumentVector(ExprVec *vec);

    //! \brief Set the command's resource list.
    //! \param dest Pointer to the ResourceList.
    //! \note Ownership of the list is transferred to the command instance.
    //! \note It is an error to call this method after the command has
    //!       been activated.
    void setResourceList(ResourceSpecList *l);

    //
    // Interface to CommandNode
    //

    //! \brief Activate all the expressions associated with the
    //!        command.  Perform one-time-only initializations if
    //!        required.
    //! \note It is an error to call this method when the command is
    //!       active.
    void activate();

    //! \brief Fix the values of all expressions prior to execution.
    //! \note It is an errror to call this method when the command is
    //!       inactive.
    void fixValues();
    
    //! \brief Deactivate all the expressions associated with the
    //!        command.  Report deactivation to the resource arbiter,
    //!        if given.
    //! \param arbiter Pointer to the resource arbiter.  May be null.
    //! \note It is an errror to call this method when the command is
    //!       inactive.
    void deactivate(ResourceArbiterInterface *arbiter);

    //! \brief Delete any "garbage" objects associated with this
    //!        command.
    //! \note Called by CommandNode::cleanUpNodeBody().
    void cleanUp();

    //
    // Interface to ExternalInterface
    //

    //! \brief Receive a return value from the external interface.
    //! \param val Const reference to the return value.
    void returnValue(Value const &val);

    //! \brief Receive a command handle (status) value from the
    //!        external interface.
    //! \param handle The return value.
    void setCommandHandle(CommandHandleValue handle);

    //! \brief Receive acknowledgement of a command abort from the
    //!        external interface.
    //! \param ack The acknowledgement value; true indicates
    //!            successful abort.
    void acknowledgeAbort(bool ack);

    //
    // LinkedQueue item API
    //

    //! \brief Get the next command in a LinkedQueue.
    //! \return Pointer to the next command.  May be null.
    CommandImpl *next() const
    {
      return m_next;
    }

    //! \brief Get the pointer to the next-item pointer in a
    //!        LinkedQueue.
    //! \return Pointer to the next-item pointer.  Will never be null.
    CommandImpl **nextPtr()
    {
      return &m_next;
    }

  private:

    // Default, copy move constructors, assignment operators
    // deliberately not implemented
    CommandImpl() = delete;
    CommandImpl(CommandImpl const &) = delete;
    CommandImpl(CommandImpl &&) = delete;
    CommandImpl& operator=(CommandImpl const &) = delete;
    CommandImpl& operator=(CommandImpl &&) = delete;

    //
    // Helpers
    //

    //! \brief At first activation, check all the expressions
    //!        associated with this command for constancy, and perform
    //!        internal initializations.
    void checkConstant();

    //! \brief Evaluate the command name expression and set the command name.
    void fixCommandName();

    //! \brief Evaluate the command parameter values and set them.
    void fixCommandArgs();

    //! \brief Evaluate the command's resource values.
    void fixResourceValues();

    //
    // Member variables
    //

    //! \brief The command-handle-known function.
    CommandFunction m_handleKnownFn;

    //! \brief The command handle expression.
    CommandHandleVariable m_ack;

    //! \brief The abort-complete variable.
    SimpleBooleanVariable m_abortComplete;

    //! \brief The command's name and parameters, represented as a
    //!        State instance.
    //! \note Only valid when the command is active and has been
    //!       fixed, or if isCommandConstant() returns true.
    State m_command;

    //! \brief The vector of ResourceValue instances.  Only valid when
    //!        m_resourcesFixed is true.
    ResourceValueList m_resourceValueList;

    //! \brief Pointer to the next CommandImpl in a LinkedQueue.
    CommandImpl *m_next;

    //! \brief The name expression.
    Expression *m_nameExpr;

    //! \brief The expression to receive the return value.  May be
    //!        null.
    Expression *m_dest;

    //! \brief Pointer to the vector of parameter expressions.  May be
    //!        null.
    std::unique_ptr<ExprVec> m_argVec;

    //! \brief Pointer to the vector of ResourceSpec instances.  May
    //!        be null.
    std::unique_ptr<ResourceSpecList> m_resourceList;

    //! \brief The current command handle value.  Referenced by m_ack.
    CommandHandleValue m_commandHandle;

    //! \brief true when the command is active, false at all other times.
    bool m_active;

    //! \brief Have all the expressions associated with this command
    //!        been checked for constants?  Initialized to false.
    bool m_checkedConstant;

    //! \brief true when the command's name is fixed.
    bool m_commandNameFixed;

    //! \brief true when the command's parameter values are fixed.
    bool m_commandArgsFixed;

    //! \brief True when the command's resource values are fixed.
    bool m_resourcesFixed;

    //! \brief True if the command's parameter expressions are all constants.
    bool m_commandArgsAreConstant;

    //! \brief True if the command name expression is a constant.
    bool m_commandNameIsConstant;

    //! \brief True if the command's resource specification
    //!        expressions are all constants.
    bool m_resourcesAreConstant;

    //! \brief If true, m_nameExpr will be deleted by the command's
    //!        destructor.
    bool m_nameIsGarbage;

    //! \brief If true, m_dest will be deleted by the command's
    //!        destructor.
    bool m_destIsGarbage;
  };

} // namespace PLEXIL

#endif // PLEXIL_COMMAND_IMPL_HH
