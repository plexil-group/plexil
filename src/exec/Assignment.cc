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

#include "Assignment.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExecListenerBase.hh"
#include "PlexilExec.hh" // getExecListener()

namespace PLEXIL
{

  Assignment::Assignment(Assignable *lhs, 
                         Expression *rhs,
                         const bool deleteLhs, 
                         const bool deleteRhs,
                         const std::string &nodeId)
    : m_ack(),
      m_abortComplete(),
      m_rhs(rhs),
      m_dest(lhs),
      m_value(),
      m_deleteLhs(deleteLhs), m_deleteRhs(deleteRhs)
  {
    assertTrue_1(lhs != NULL);
    assertTrue_1(rhs != NULL);

    // Make ack variable pretty
    m_ack.setName(nodeId + " ack");
    m_abortComplete.setName(nodeId + " abortComplete");
  }

  Assignment::Assignment(const std::string &nodeId)
    : m_ack(),
      m_abortComplete(),
      m_rhs(NULL),
      m_dest(NULL),
      m_value(),
      m_deleteLhs(false),
      m_deleteRhs(false)
  {
    // Make ack variable pretty
    m_ack.setName(nodeId + " ack");
    m_abortComplete.setName(nodeId + " abortComplete");
  }

  Assignment::~Assignment() 
  {
    if (m_deleteLhs)
      delete m_dest;
    if (m_deleteRhs)
      delete m_rhs;
  }

  void Assignment::setVariable(Assignable *lhs, bool garbage)
  {
    assertTrue_1(lhs);
    assertTrue_2(!m_dest, "setVariable() on an Assignment with a variable");
    m_dest = lhs;
    m_deleteLhs = garbage;
  }

  void Assignment::setExpression(Expression *rhs, bool garbage)
  {
    assertTrue_1(rhs);
    assertTrue_2(!m_rhs, "setExpression() on an Assignment with an expression");
    m_rhs = rhs;
    m_deleteRhs = garbage;
  }

  void Assignment::fixValue() 
  {
    m_dest->saveCurrentValue();
    m_value = m_rhs->toValue();
  }

  void Assignment::activate() 
  {
    assertTrue_2(m_dest && m_rhs,
                 "Attempt to activate uninitialized Assignment");
    m_rhs->activate();
    m_dest->activate();
  }

  void Assignment::deactivate() 
  {
    m_rhs->deactivate();
    m_dest->deactivate();
    m_value = Value(); // make unknown
  }

  void Assignment::execute()
  {
    debugMsg("Test:testOutput", "Assigning " << m_dest->toString() << " to " << m_value);
    m_dest->setValue(m_value);
    m_ack.setValue(true);
    ExecListenerBase *l = g_exec->getExecListener();
    if (l)
      l->notifyOfAssignment(m_dest, m_dest->getName(), m_value);
  }

  void Assignment::retract()
  {
    debugMsg("Test:testOutput",
             "Restoring previous value of " << m_dest->toString());
    m_dest->restoreSavedValue();
    m_abortComplete.setValue(true);
    ExecListenerBase *l = g_exec->getExecListener();
    if (l)
      l->notifyOfAssignment(m_dest, m_dest->getName(), m_dest->getSavedValue());
  }

  void Assignment::reset()
  {
    m_ack.reset();
    m_abortComplete.reset();
    m_value = Value(); // set unknown
  }

}
