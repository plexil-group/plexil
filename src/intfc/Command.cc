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

#include "Command.hh"
#include "Error.hh"
#include "ExprVec.hh"
#include "ExternalInterface.hh"
#include "ResourceArbiterInterface.hh"

namespace PLEXIL
{

  //
  // ResourceSpec member functions
  //

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

  Command::Command(std::string const &nodeName)
    : m_ack(*this),
      m_abortComplete(),
      m_command(),
      m_garbage(),
      m_resourceList(),
      m_resourceValueList(),
      m_nameExpr(NULL),
      m_dest(NULL),
      m_argVec(NULL),
      m_commandHandle(NO_COMMAND_HANDLE),
      m_fixed(false),
      m_resourceFixed(false),
      m_active(false)
  {
    m_ack.setName(nodeName + " commandHandle");
    m_abortComplete.setName(nodeName + " abortComplete");
  }

  Command::~Command() 
  {
    for (std::vector<Expression *>::const_iterator it = m_garbage.begin();
         it != m_garbage.end();
         ++it)
      delete (*it);
    if (m_argVec)
      delete m_argVec;
  }

  void Command::setDestination(Assignable *dest, bool isGarbage)
  {
    assertTrue_1(dest);
    m_dest = dest;
    if (isGarbage)
      m_garbage.push_back(dest);
  }

  void Command::setNameExpr(Expression *nameExpr, bool isGarbage)
  {
    assertTrue_1(!m_nameExpr);
    assertTrue_1(nameExpr);
    m_nameExpr = nameExpr;
    if (isGarbage)
      m_garbage.push_back(nameExpr);
  }

  ResourceList &Command::getResourceList()
  {
    return m_resourceList;
  }

  void Command::addGarbageExpression(Expression *exp)
  {
    m_garbage.push_back(exp);
  }

  void Command::setArgumentVector(ExprVec *vec)
  {
    if (m_argVec)
      delete m_argVec;
    m_argVec = vec;
  }

  State const &Command::getCommand() const
  {
    assertTrue_1(m_fixed);
    return m_command;
  }

  std::string const &Command::getName() const
  {
    assertTrue_1(m_fixed);
    return m_command.name();
  }

  std::vector<Value> const &Command::getArgValues() const
  {
    assertTrue_1(m_fixed);
    return m_command.parameters();
  }

  const ResourceValueList &Command::getResourceValues() const
  {
    assertTrue_1(m_resourceFixed);
    return m_resourceValueList;
  }

  Expression *Command::getDest()
  {
    if (m_dest)
      return m_dest;
    else
      return NULL;
  }

  void Command::fixValues() 
  {
    assertTrue_1(m_active && !m_fixed);
    std::string const *name;
    m_nameExpr->getValuePointer(name);
    m_command.setName(*name);
    
    if (m_argVec) {
      size_t n = m_argVec->size();
      m_command.setParameterCount(n);
      for (size_t i = 0; i < n; ++i)
        m_command.setParameter(i, (*m_argVec)[i]->toValue());
    }

    m_fixed = true;
  }

  void Command::fixResourceValues()
  {
    assertTrue_1(m_active && !m_resourceFixed);
    m_resourceValueList.clear();
    size_t n = m_resourceList.size();
    m_resourceValueList.resize(n);
    for (size_t i = 0; i < n; ++i) {
      ResourceSpec const &spec = m_resourceList[i];
      ResourceValue &resValue = m_resourceValueList[i];
      assertTrue_2(spec.nameExp->getValue(resValue.name),
                   "Resource name expression has unknown or invalid value");
      assertTrue_2(spec.priorityExp->getValue(resValue.priority),
                   "Resource priority expression has unknown or invalid value");

      if (spec.lowerBoundExp) {
        assertTrue_2(spec.lowerBoundExp->getValue(resValue.lowerBound),
                     "Resource lower bound expression has unknown or invalid value");
      }
      else
        resValue.lowerBound = 1.0;

      if (spec.upperBoundExp) {
        assertTrue_2(spec.upperBoundExp->getValue(resValue.upperBound),
                     "Resource upper bound expression has unknown or invalid value");
      }
      else 
        resValue.upperBound = 1.0;

      if (spec.releaseAtTermExp) {
        assertTrue_2(spec.releaseAtTermExp->getValue(resValue.releaseAtTermination),
                     "Resource lower bound expression has unknown or invalid value");
      }
      else
        resValue.releaseAtTermination = true;
    }
    m_resourceFixed = true;
  }

  // more error checking here
  void Command::activate()
  {
    assertTrue_1(!m_active);
    assertTrue_1(m_nameExpr);
    m_nameExpr->activate();
    m_ack.activate();
    m_abortComplete.activate();
    if (m_dest)
      m_dest->activate();
    if (m_argVec)
      m_argVec->activate();
    for (ResourceList::iterator resListIter = m_resourceList.begin();
         resListIter != m_resourceList.end();
         ++resListIter)
      resListIter->activate();
    m_active = true;
  }

  void Command::execute()
  {
    assertTrue_1(m_active);
    fixValues();
    fixResourceValues();
    g_interface->enqueueCommand(this);
  }

  void Command::setCommandHandle(CommandHandleValue handle)
  {
    if (!m_active)
      return;
    assertTrue_1(handle > NO_COMMAND_HANDLE && handle < COMMAND_HANDLE_MAX);
    m_commandHandle = handle;
    m_ack.valueChanged();
  }

  void Command::returnValue(Value const &val)
  {
    if (!m_active || !m_dest)
      return;
    m_dest->setValue(val);
  }

  void Command::abort()
  {
    assertTrue_1(m_active);
    // Handle stupid unit test
    if (g_interface) {
      g_interface->abortCommand(this);
    }
  }

  void Command::acknowledgeAbort(bool ack)
  {
    // Ignore late or erroneous acks
    if (!m_active)
      return;
    m_abortComplete.setValue(ack);
  }

  void Command::deactivate() 
  {
    assertTrue_1(m_active);
    m_active = false;
    if (m_commandHandle != COMMAND_DENIED)
      g_interface->getResourceArbiter()->releaseResourcesForCommand(this);
    for (ResourceList::iterator resListIter = m_resourceList.begin();
         resListIter != m_resourceList.end();
         ++resListIter)
      resListIter->deactivate();
    if (m_argVec)
      m_argVec->deactivate();
    if (m_dest)
      m_dest->deactivate();
    m_abortComplete.deactivate();
    m_ack.deactivate();
    m_nameExpr->deactivate();
  }

  void Command::reset()
  {
    m_commandHandle = NO_COMMAND_HANDLE;
    m_abortComplete.reset();
    // TODO: optimize in case name & args are constants (a common case)
    m_fixed = m_resourceFixed = m_active = false;
  }

}
