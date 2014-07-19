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
#include "ExternalInterface.hh"
#include "ResourceArbiterInterface.hh"

namespace PLEXIL
{

  Command::Command(Expression *nameExpr, 
                   std::vector<Expression *> const &args,
                   std::vector<Expression *> const &garbage,
                   Assignable *dest,
                   const ResourceList &resource,
                   std::string const &nodeName)
    : m_ack(*this),
      m_abortComplete(),
      m_nameExpr(nameExpr),
      m_dest(dest),
      m_garbage(garbage),
      m_args(args),
      m_resourceList(resource),
      m_commandHandle(NO_COMMAND_HANDLE),
      m_fixed(false),
      m_resourceFixed(false),
      m_active(false)
  {
    m_ack.setName(nodeName + " commandHandle");
    m_abortComplete.setName(nodeName + " abortComplete");
  }

  Command::~Command() {
    for (std::vector<Expression *>::const_iterator it = m_garbage.begin();
         it != m_garbage.end();
         ++it) {
      delete (*it);
    }
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

  const ResourceValuesList &Command::getResourceValues() const
  {
    assertTrue_1(m_resourceFixed);
    return m_resourceValuesList;
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

    std::vector<Value> vals;
    vals.reserve(m_args.size());
    for (std::vector<Expression *>::const_iterator it = m_args.begin();
         it != m_args.end();
         ++it)
      vals.push_back((*it)->toValue());

    m_command = State(*name, vals);
    m_fixed = true;
  }

  void Command::fixResourceValues()
  {
    assertTrue_1(m_active && !m_resourceFixed);
    m_resourceValuesList.clear();
    for(ResourceList::const_iterator resListIter = m_resourceList.begin();
        resListIter != m_resourceList.end();
        ++resListIter)
      {
        ResourceValues resValues;
        for(ResourceMap::const_iterator resIter = resListIter->begin();
            resIter != resListIter->end();
            ++resIter) {
          Expression *expr = resIter->second;
          resValues[resIter->first] = expr->toValue();
        }
        m_resourceValuesList.push_back(resValues);
      }
    m_resourceFixed = true;
  }

  // more error checking here
  void Command::activate()
  {
    assertTrue_1(!m_active);
    m_nameExpr->activate();
    m_ack.activate();
    m_abortComplete.activate();
    if (m_dest)
      m_dest->activate();
    for (std::vector<Expression *>::iterator it = m_args.begin(); it != m_args.end(); ++it)
      (*it)->activate();
    for (ResourceList::const_iterator resListIter = m_resourceList.begin();
         resListIter != m_resourceList.end();
         ++resListIter) {
      for (ResourceMap::const_iterator resIter = resListIter->begin();
           resIter != resListIter->end();
           ++resIter)
        resIter->second->activate();
    }
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
      g_interface->getResourceArbiter()->releaseResourcesForCommand(m_command.name());
    for (ResourceList::const_iterator resListIter = m_resourceList.begin();
         resListIter != m_resourceList.end();
         ++resListIter) {
      for (ResourceMap::const_iterator resIter = resListIter->begin();
           resIter != resListIter->end();
           ++resIter)
        resIter->second->deactivate();
    }
    for (std::vector<Expression *>::iterator it = m_args.begin(); it != m_args.end(); ++it)
      (*it)->deactivate();
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
