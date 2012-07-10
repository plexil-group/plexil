/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
#include "BooleanVariable.hh"
#include "CoreExpressions.hh"
#include "Node.hh"

namespace PLEXIL
{

  Command::Command(const ExpressionId nameExpr, 
                   const std::list<ExpressionId>& args,
                   const VariableId dest,
                   const LabelStr& dest_name,
                   const std::vector<ExpressionId>& garbage,
                   const ResourceList& resource,
                   const NodeId& parent)
    : m_id(this),
      m_node(parent),
      m_nameExpr(nameExpr),
      m_dest(dest),
      m_destName(dest_name),
      m_ack((new CommandHandleVariable(parent->getNodeId().toString() + " command_handle"))->getId()), 
      m_abortComplete((new BooleanVariable())->getId()),
      m_garbage(garbage),
      m_args(args),
      m_resourceList(resource)
  {
    ((VariableImpl*) m_abortComplete)->setName(parent->getNodeId().toString() + " abortComplete");
  }

  Command::~Command() {
    for (std::vector<ExpressionId>::const_iterator it = m_garbage.begin();
         it != m_garbage.end();
         ++it) {
      delete (Expression*) (*it);
    }
    delete (Expression*) m_ack;
    delete (Expression*) m_abortComplete;
    m_id.remove();
  }

  LabelStr Command::getName() const
  {
    return LabelStr(m_nameExpr->getValue()); 
  }

  void Command::fixValues() {
    m_argValues.clear();
    for (std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      m_argValues.push_back(expr->getValue());
    }
  }

  void Command::fixResourceValues()
  {
    m_resourceValuesList.clear();
    for(ResourceList::const_iterator resListIter = m_resourceList.begin();
        resListIter != m_resourceList.end();
        ++resListIter)
      {
        ResourceValues resValues;
        for(ResourceMap::const_iterator resIter = resListIter->begin();
            resIter != resListIter->end();
            ++resIter) {
          ExpressionId expr = resIter->second;
          check_error(expr.isValid());
          resValues[resIter->first] = expr->getValue();
        }
        m_resourceValuesList.push_back(resValues);
      }
  }

  //more error checking here
  void Command::activate() {
    m_nameExpr->activate();
    m_ack->activate();
    m_abortComplete->activate(); // redundant?
    if (m_dest != ExpressionId::noId())
      m_dest->activate();
    for (std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    for (ResourceList::const_iterator resListIter = m_resourceList.begin();
         resListIter != m_resourceList.end();
         ++resListIter) {
      for (ResourceMap::const_iterator resIter = resListIter->begin();
           resIter != resListIter->end();
           ++resIter) {
        ExpressionId expr = resIter->second;
        check_error(expr.isValid());
        expr->activate();
      }
    }
  }

  void Command::deactivate() {
    m_nameExpr->deactivate();
    m_ack->deactivate();
    m_abortComplete->deactivate(); // redundant?
    if (m_dest != ExpressionId::noId())
      m_dest->deactivate();
    for (std::list<ExpressionId>::iterator it = m_args.begin(); it != m_args.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
  }

  void Command::reset()
  {
    m_ack->reset();
    m_abortComplete->reset();
  }

  const std::string& Command::getDestName() const {
    return m_destName.toString();
  }

}
