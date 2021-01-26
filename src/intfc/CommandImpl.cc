/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "CommandOperatorImpl.hh"
#include "Assignable.hh"
#include "ExprVec.hh"
#include "ExternalInterface.hh"
#include "InterfaceError.hh"
#include "PlanError.hh"

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
    cleanUp();
  }

  void ResourceSpec::cleanUp()
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

  /**
   * @class CommandHandleKnown
   * @brief A CommandOperator that returns true if the command handle is known, false otherwise.
   */

  class CommandHandleKnown : public CommandOperatorImpl<Boolean>
  {
  public:
    ~CommandHandleKnown()
    {
    }
    
    DECLARE_COMMAND_OPERATOR_STATIC_INSTANCE(CommandHandleKnown)

    bool operator()(Boolean &result, CommandImpl const *command) const
    {
      result = (NO_COMMAND_HANDLE != command->getCommandHandle());
      return true;
    }

    void doPropagationSources(CommandImpl *command, ListenableUnaryOperator const &oper) const
    {
      (oper)(command->getAck());
    }

  private:

    CommandHandleKnown()
      : CommandOperatorImpl<Boolean>("CommandHandleKnown")
    {
    }

    // Disallow copy, assign
    CommandHandleKnown(CommandHandleKnown const &);
    CommandHandleKnown &operator=(CommandHandleKnown const &);
  };

  CommandImpl::CommandImpl(std::string const &nodeName)
    : m_next(nullptr),
      m_handleKnownFn(CommandHandleKnown::instance(), *this),
      m_ack(*this),
      m_abortComplete("abortComplete"),
      m_command(),
      m_nameExpr(nullptr),
      m_dest(nullptr),
      m_argVec(nullptr),
      m_resourceList(nullptr),
      m_resourceValueList(nullptr),
      m_commandHandle(NO_COMMAND_HANDLE),
      m_active(false),
      m_commandFixed(false),
      m_commandIsConstant(false),
      m_resourcesFixed(false),
      m_resourcesAreConstant(false),
      m_nameIsGarbage(false),
      m_destIsGarbage(false),
      m_checkedConstant(false)
  {
    m_ack.setName(nodeName);
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

    delete m_argVec;
    m_argVec = nullptr;

    if (m_destIsGarbage)
      delete m_dest;
    m_destIsGarbage = false;
    m_dest = nullptr;

    if (m_resourceList) {
      for (ResourceSpec &s : *m_resourceList)
        s.cleanUp();
      delete m_resourceList;
      m_resourceList = nullptr;
      delete m_resourceValueList;
      m_resourceValueList = nullptr;
    }
  }

  State const &CommandImpl::getCommand() const
  {
    assertTrue_1(m_commandIsConstant || m_commandFixed);
    return m_command;
  }

  std::string const &CommandImpl::getName() const
  {
    assertTrue_1(m_commandNameIsConstant || m_commandFixed);
    return m_command.name();
  }

  std::vector<Value> const &CommandImpl::getArgValues() const
  {
    assertTrue_1(m_commandIsConstant || m_commandFixed);
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

  const ResourceValueList &CommandImpl::getResourceValues() const
  {
    static ResourceValueList const sl_emptyList;

    assertTrue_1(m_resourcesFixed);
    if (m_resourceList)
      return *m_resourceValueList;
    return sl_emptyList;
  }

  void CommandImpl::setDestination(Expression *dest, bool isGarbage)
  {
    m_dest = dest;
    m_destIsGarbage = isGarbage;
  }

  void CommandImpl::setNameExpr(Expression *nameExpr, bool isGarbage)
  {
    m_nameExpr = nameExpr;
    m_nameIsGarbage = isGarbage;
  }

  void CommandImpl::setResourceList(ResourceList *lst)
  {
    if (m_resourceList && m_resourceList != lst) // unlikely, but...
      delete m_resourceList;

    m_resourceList = lst;
    m_resourcesAreConstant = false; // must check
  }

  void CommandImpl::setArgumentVector(ExprVec *vec)
  {
    delete m_argVec;
    m_argVec = vec;
    // TODO: check whether argument vector is constant
  }

  Expression *CommandImpl::getDest()
  {
    return m_dest;
  }

  bool CommandImpl::isCommandNameConstant() const
  {
    return m_nameExpr->isConstant();
  }

  bool CommandImpl::isCommandConstant() const
  {
    if (!isCommandNameConstant())
      return false;
    if (m_argVec)
      for (size_t i = 0; i < m_argVec->size(); ++i)
        if (!(*m_argVec)[i]->isConstant())
          return false;
    return true;
  }

  bool CommandImpl::areResourcesConstant() const
  {
    if (!m_resourceList)
      return true;

    for (ResourceSpec const &spec : *m_resourceList) {
      if (!spec.nameExp->isConstant()
          || !spec.priorityExp->isConstant()
          || (spec.lowerBoundExp && !spec.lowerBoundExp->isConstant())
          || (spec.upperBoundExp && !spec.upperBoundExp->isConstant())
          || (spec.releaseAtTermExp && !spec.releaseAtTermExp->isConstant()))
        return false;
    }

    return true;
  }

  void CommandImpl::fixValues() 
  {
    assertTrue_1(m_active);
    if (m_commandFixed)
      return;
    std::string const *name;
    m_nameExpr->getValuePointer(name);
    m_command.setName(*name);
    if (m_argVec) {
      size_t n = m_argVec->size();
      m_command.setParameterCount(n);
      for (size_t i = 0; i < n; ++i)
        m_command.setParameter(i, (*m_argVec)[i]->toValue());
    }
    m_commandFixed = true;
  }

  void CommandImpl::fixResourceValues()
  {
    check_error_1(m_active);
    if (!m_resourceList)
      return;
    if (m_resourcesFixed)
      return;
    
    size_t n = m_resourceList->size();
    if (!m_resourceValueList)
      m_resourceValueList = new ResourceValueList(n);

    for (size_t i = 0; i < n; ++i) {
      ResourceSpec const &spec = (*m_resourceList)[i];
      ResourceValue &resValue = (*m_resourceValueList)[i];
      checkPlanError(spec.nameExp->getValue(resValue.name),
                     "Command resource name expression has unknown or invalid value");
      checkPlanError(spec.priorityExp->getValue(resValue.priority),
                     "Command resource priority expression has unknown or invalid value");

      if (spec.lowerBoundExp) {
        checkPlanError(spec.lowerBoundExp->getValue(resValue.lowerBound),
                       "Command resource lower bound expression has unknown or invalid value");
      }
      else
        resValue.lowerBound = 1.0;

      if (spec.upperBoundExp) {
        checkPlanError(spec.upperBoundExp->getValue(resValue.upperBound),
                       "Command resource upper bound expression has unknown or invalid value");
      }
      else 
        resValue.upperBound = 1.0;

      if (spec.releaseAtTermExp) {
        checkPlanError(spec.releaseAtTermExp->getValue(resValue.releaseAtTermination),
                       "Command resource lower bound expression has unknown or invalid value");
      }
      else
        resValue.releaseAtTermination = true;
    }
    m_resourcesFixed = true;
  }

  void CommandImpl::activate()
  {
    check_error_1(!m_active);
    check_error_1(m_nameExpr);

    m_commandHandle = NO_COMMAND_HANDLE;
    m_ack.activate();
    m_abortComplete.activate();

    // Will always be false on first activation
    // and if relevant parameters are not constants
    if (!m_commandIsConstant)
      m_commandFixed = false;
    if (!m_resourcesAreConstant)
      m_resourcesFixed = false;

    if (m_dest)
      m_dest->activate();

    // If command fixed, these are already active
    if (!m_commandFixed) {
      m_nameExpr->activate();
      if (m_argVec)
        m_argVec->activate();
    }

    // If resources fixed, they are already active
    if (m_resourceList && !m_resourcesFixed)
      if (m_resourceList)
        for (ResourceSpec &res : *m_resourceList)
          res.activate();

    // Check for constancy at first activation
    if (!m_checkedConstant) {
      if ((m_commandNameIsConstant = isCommandNameConstant()))
        m_commandIsConstant = isCommandConstant(); // defaults to false
      if (m_resourceList)
        m_resourcesAreConstant = areResourcesConstant();
      else
        m_resourcesAreConstant = m_resourcesFixed = true; // because there aren't any
      m_checkedConstant = true;
    }

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

  void CommandImpl::deactivate() 
  {
    check_error_1(m_active);
    m_active = false;

    if (m_commandHandle != COMMAND_DENIED)
      g_interface->releaseResourcesForCommand(this);

    m_abortComplete.deactivate();
    m_ack.deactivate();

    if (m_dest)
      m_dest->deactivate();

    // Don't deactivate if resources are constant
    if (m_resourceList && !m_resourcesAreConstant) {
      if (!m_resourcesAreConstant) {
        for (ResourceSpec &res : *m_resourceList)
          res.deactivate();
        m_resourcesFixed = false;
      }
    }

    // Don't deactivate if command is constant
    if (!m_commandIsConstant) {
      m_nameExpr->deactivate();
      if (m_argVec)
        m_argVec->deactivate();
      m_commandFixed = false;
    }
  }

}
