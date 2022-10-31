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

#include "CommandImpl.hh"

#include "CommandOperator.hh"
#include "Assignable.hh"
#include "ExprVec.hh"
#include "InterfaceError.hh"
#include "PlanError.hh"
#include "ResourceArbiterInterface.hh"

namespace PLEXIL
{

  //
  // ResourceSpec implementation
  //

  void ResourceSpec::setNameExpression(Expression *exp, bool isGarbage)
  {
    nameExp = exp;
    nameIsGarbage = isGarbage;
  }

  void ResourceSpec::setPriorityExpression(Expression *exp, bool isGarbage)
  {
    priorityExp = exp;
    priorityIsGarbage = isGarbage;
  }

  void ResourceSpec::setLowerBoundExpression(Expression *exp, bool isGarbage)
  {
    lowerBoundExp = exp;
    lowerBoundIsGarbage = isGarbage;
  }

  void ResourceSpec::setUpperBoundExpression(Expression *exp, bool isGarbage)
  {
    upperBoundExp = exp;
    upperBoundIsGarbage = isGarbage;
  }

  void ResourceSpec::setReleaseAtTerminationExpression(Expression *exp, bool isGarbage)
  {
    releaseAtTermExp = exp;
    releaseIsGarbage = isGarbage;
  }

  ResourceSpec::~ResourceSpec()
  {
    if (nameIsGarbage)
      delete nameExp;
    nameIsGarbage = false;
    nameExp = nullptr;

    if (priorityIsGarbage)
      delete priorityExp;
    priorityIsGarbage = false;
    priorityExp = nullptr;

    if (lowerBoundIsGarbage)
      delete lowerBoundExp;
    lowerBoundIsGarbage = false;
    lowerBoundExp = nullptr;

    if (upperBoundIsGarbage)
      delete upperBoundExp;
    upperBoundIsGarbage = false;
    upperBoundExp = nullptr;

    if (releaseIsGarbage)
      delete releaseAtTermExp;
    releaseIsGarbage = false;
    releaseAtTermExp = nullptr;
  }

  bool ResourceSpec::isConstant() const
  {
    assertTrue_1(nameExp);
    if (!nameExp->isConstant())
      return false;
    if (priorityExp && !priorityExp->isConstant())
      return false;
    if (lowerBoundExp && !lowerBoundExp->isConstant())
      return false;
    if (upperBoundExp && !upperBoundExp->isConstant())
      return false;
    if (releaseAtTermExp && !releaseAtTermExp->isConstant())
      return false;
    return true;
  }

  void ResourceSpec::activate()
  {
    nameExp->activate();
    priorityExp->activate();
    if (lowerBoundExp)
      lowerBoundExp->activate();
    if (upperBoundExp)
      upperBoundExp->activate();
    if (releaseAtTermExp)
      releaseAtTermExp->activate();
  }

  void ResourceSpec::deactivate()
  {
    nameExp->deactivate();
    priorityExp->deactivate();
    if (lowerBoundExp)
      lowerBoundExp->deactivate();
    if (upperBoundExp)
      upperBoundExp->deactivate();
    if (releaseAtTermExp)
      releaseAtTermExp->deactivate();
  }

  //! \class CommandHandleKnown
  //! \brief A CommandOperator that returns true if the command handle
  //!        is known, false otherwise.
  class CommandHandleKnown : public CommandOperator
  {
  public:

    //! \brief Virtual destructor.
    virtual ~CommandHandleKnown() = default;
    
    //! \brief Singleton accessor, implemented as a static member function.
    //! \return Pointer to the const singleton.
    DECLARE_COMMAND_OPERATOR_STATIC_INSTANCE(CommandHandleKnown)

    //! \brief Get the type of the operator's return value.
    //! \return The ValueType.
    virtual ValueType valueType() const
    {
      return BOOLEAN_TYPE;
    }

    //! \brief Compute the result of applying this operator to the Command
    //!        and store it in the appropriately typed result variable.
    //! \param result Reference to the result variable.
    //! \param arg Const pointer to the Command.
    //! \return true if the result is known, false if not.
    virtual bool operator()(Boolean &result, CommandImpl const *arg) const
    {
      result = (NO_COMMAND_HANDLE != arg->getCommandHandle());
      return true;
    }

    //! \brief Is the value of this operator applied to this command known?
    //! \param command Const pointer to the Command.
    //! \return true if known, false if unknown.
    virtual bool isKnown(CommandImpl const *command) const
    {
      return true;
    }

    //! \brief Print the value of this operator applied to this command.
    //! \param s The output stream.
    //! \param command Const pointer to the Command.
    virtual void printValue(std::ostream &s, CommandImpl const *command) const
    {
      Boolean temp;
      ((*this)(temp, command)); // for effect
      PLEXIL::printValue(temp, s);
    }

    //! \brief Get the value of this operator applied to this command.
    //! \param command Const pointer to the command.
    //! \return The Value.
    virtual Value toValue(CommandImpl const *command) const
    {
      Boolean temp;
      ((*this)(temp, command)); // for effect
      return Value(temp);
    }

    void doPropagationSources(CommandImpl *command, ListenableUnaryOperator const &oper) const
    {
      (oper)(command->getAck());
    }

  private:

    //! \brief Default constructor, only accessible to singleton accessor function.
    CommandHandleKnown()
      : CommandOperator("CommandHandleKnown")
    {
    }

    // Disallow copy, assign
    CommandHandleKnown(CommandHandleKnown const &) = delete;
    CommandHandleKnown(CommandHandleKnown &&) = delete;
    CommandHandleKnown &operator=(CommandHandleKnown const &) = delete;
    CommandHandleKnown &operator=(CommandHandleKnown &&) = delete;
  };

  CommandImpl::CommandImpl(std::string const &nodeName)
    : m_handleKnownFn(CommandHandleKnown::instance(), *this),
      m_ack(*this, nodeName),
      m_abortComplete("abortComplete"),
      m_command(),
      m_resourceValueList(),
      m_next(nullptr),
      m_nameExpr(nullptr),
      m_dest(nullptr),
      m_argVec(),
      m_resourceList(),
      m_commandHandle(NO_COMMAND_HANDLE),
      m_active(false),
      m_checkedConstant(false),
      m_commandNameFixed(false),
      m_commandArgsFixed(false),
      m_resourcesFixed(false),
      m_commandNameIsConstant(false),
      m_commandArgsAreConstant(false),
      m_resourcesAreConstant(false),
      m_nameIsGarbage(false),
      m_destIsGarbage(false)
  {
  }

  CommandImpl::~CommandImpl() 
  {
    cleanUp();
  }

  void CommandImpl::cleanUp()
  {
    if (m_nameIsGarbage)
      delete m_nameExpr;
    m_nameIsGarbage = false;
    m_nameExpr = nullptr;

    m_argVec.reset();

    if (m_destIsGarbage)
      delete m_dest;
    m_destIsGarbage = false;
    m_dest = nullptr;

    m_resourceList.reset();
  }

  State const &CommandImpl::getCommand() const
  {
    assertTrue_1(m_commandNameFixed && m_commandArgsFixed);
    return m_command;
  }

  std::string const &CommandImpl::getName() const
  {
    assertTrue_1(m_commandNameFixed);
    return m_command.name();
  }

  std::vector<Value> const &CommandImpl::getArgValues() const
  {
    assertTrue_1(m_commandArgsFixed);
    return m_command.parameters();
  }

  bool CommandImpl::isReturnExpected() const
  {
    return (bool) m_dest;
  }

  CommandHandleValue CommandImpl:: getCommandHandle() const
  {
    return m_commandHandle;
  }

  ResourceValueList const &CommandImpl::getResourceValues() const
  {
    assertTrue_1(m_resourcesFixed);
    return m_resourceValueList;
  }

  void CommandImpl::setDestination(Expression *dest, bool isGarbage)
  {
    assertTrue_1(!m_checkedConstant);
    m_dest = dest;
    m_destIsGarbage = isGarbage;
  }

  void CommandImpl::setNameExpr(Expression *nameExpr, bool isGarbage)
  {
    assertTrue_1(!m_checkedConstant);
    m_nameExpr = nameExpr;
    m_nameIsGarbage = isGarbage;
  }

  void CommandImpl::setResourceList(ResourceSpecList *lst)
  {
    assertTrue_1(!m_checkedConstant);
    m_resourceList.reset(lst);
    m_resourcesAreConstant = false; // must check
  }

  void CommandImpl::setArgumentVector(ExprVec *vec)
  {
    assertTrue_1(!m_checkedConstant);
    m_argVec.reset(vec);
  }

  Expression *CommandImpl::getDest()
  {
    return m_dest;
  }

  void CommandImpl::checkConstant()
  {
    // Check name
    assertTrue_1(m_nameExpr);
    if (m_nameExpr->isConstant()) {
      m_commandNameIsConstant = true;
      fixCommandName();
    }

    // Check parameters
    m_commandArgsAreConstant = true;
    size_t nArgs = 0;
    if (m_argVec) {
      nArgs = m_argVec->size();
      // Check parameters 
      for (size_t i = 0; i < nArgs; ++i) {
        if (!(*m_argVec)[i]->isConstant()) {
          m_commandArgsAreConstant = false;
          break;
        }
      }
    }
    // Parameter list length for a command invocation cannot vary at
    // run time, so set it now
    m_command.setParameterCount(nArgs);
    if (m_commandArgsAreConstant)
      fixCommandArgs();

    // Check resource specs
    m_resourcesAreConstant = true;
    if (m_resourceList) {
      // Allocate resource value list now
      m_resourceValueList.resize(m_resourceList->size());
      // Check all specs for constancy
      for (ResourceSpec const &spec : *m_resourceList) {
        if (!spec.isConstant()) {
          m_resourcesAreConstant = false;
          break;
        }
      }
    }
    if (m_resourcesAreConstant)
      fixResourceValues();

    m_checkedConstant = true;
  }

  void CommandImpl::fixCommandName()
  {
    std::string const *name = nullptr;
    if (m_nameExpr->getValuePointer(name)) {
      m_command.setName(*name);
      m_commandNameFixed = true;
    }
    else {
      // Name is unknown - report plan error
      // TODO
    }
  }

  // Note that m_command.setParameterCount() was called in
  // checkConstant() above.
  void CommandImpl::fixCommandArgs()
  {
    if (m_argVec) {
      for (size_t i = 0; i < m_argVec->size(); ++i)
        m_command.setParameter(i, (*m_argVec)[i]->toValue());
    }
    m_commandArgsFixed = true;
  }

  void CommandImpl::fixResourceValues()
  {
    if (m_resourceList) {
      // m_resourceValueList was sized in checkConstant()
      for (size_t i = 0; i < m_resourceList->size(); ++i) {
        ResourceSpec const &spec = (*m_resourceList)[i];
        ResourceValue &resValue = m_resourceValueList[i];
        checkPlanError(spec.nameExp->getValue(resValue.name),
                       "Command resource name expression has unknown or invalid value");
        checkPlanError(spec.priorityExp->getValue(resValue.priority),
                       "Command resource priority expression has unknown or invalid value");

        if (spec.lowerBoundExp) {
          checkPlanError(spec.lowerBoundExp->getValue(resValue.lowerBound),
                         "Command resource lower bound expression has unknown or invalid value");
        }
        else {
          resValue.lowerBound = 1.0;
        }

        if (spec.upperBoundExp) {
          checkPlanError(spec.upperBoundExp->getValue(resValue.upperBound),
                         "Command resource upper bound expression has unknown or invalid value");
        }
        else {
          resValue.upperBound = 1.0;
        }

        if (spec.releaseAtTermExp) {
          checkPlanError(spec.releaseAtTermExp->getValue(resValue.releaseAtTermination),
                         "Command resource lower bound expression has unknown or invalid value");
        }
        else {
          resValue.releaseAtTermination = true;
        }
      }
    }
    m_resourcesFixed = true;
  }

  void CommandImpl::fixValues() 
  {
    check_error_1(m_active);

    if (!m_commandNameFixed)
      fixCommandName();
    if (!m_commandArgsFixed)
      fixCommandArgs();
    if (!m_resourcesFixed)
      fixResourceValues();
  }

  void CommandImpl::activate()
  {
    check_error_1(!m_active);

    m_commandHandle = NO_COMMAND_HANDLE;
    m_ack.activate();
    m_abortComplete.activate();

    // Check for constancy and set up internal data structures at
    // first activation.
    if (!m_checkedConstant) 
      checkConstant();

    // Activate any expressions which aren't constants,
    // and clear their fixed flags.
    if (!m_commandNameIsConstant) {
      m_commandNameFixed = false;
      m_nameExpr->activate();
    }
    if (!m_commandArgsAreConstant) {
      m_commandArgsFixed = false;
      m_argVec->activate();
    }
    if (!m_resourcesAreConstant) {
      m_resourcesFixed = false;
      for (ResourceSpec &res : *m_resourceList)
        res.activate();
    }

    // Activate the return value variable, if any.
    if (m_dest)
      m_dest->activate();

    m_active = true;
  }

  void CommandImpl::setCommandHandle(CommandHandleValue handle)
  {
    if (!m_active)
      return;
    checkInterfaceError(handle > NO_COMMAND_HANDLE && handle < COMMAND_HANDLE_MAX,
                        "Invalid command handle value");
    m_commandHandle = handle;
    m_ack.publishChange();
  }

  void CommandImpl::returnValue(Value const &val)
  {
    if (!m_active || !m_dest)
      return;
    m_dest->asAssignable()->setValue(val);
  }

  void CommandImpl::acknowledgeAbort(bool ack)
  {
    // Ignore late or erroneous acks
    if (!m_active)
      return;
    m_abortComplete.setValue(ack);
  }

  void CommandImpl::deactivate(ResourceArbiterInterface *arbiter)
  {
    check_error_1(m_active);
    m_active = false;

    if (m_commandHandle != COMMAND_DENIED && arbiter) // handle unit tests
      arbiter->releaseResourcesForCommand(this);

    m_abortComplete.deactivate();
    m_ack.deactivate();

    if (m_dest)
      m_dest->deactivate();

    // Deactivate any expressions activated earlier
    if (m_resourceList && !m_resourcesAreConstant) {
      if (!m_resourcesAreConstant) {
        for (ResourceSpec &res : *m_resourceList)
          res.deactivate();
        m_resourcesFixed = false;
      }
    }
    if (!m_commandNameIsConstant) {
      m_nameExpr->deactivate();
      m_commandNameFixed = false;
    }
    if (m_argVec && !m_commandArgsAreConstant) { 
        m_argVec->deactivate();
        m_commandArgsFixed = false;
    }
  }

}
