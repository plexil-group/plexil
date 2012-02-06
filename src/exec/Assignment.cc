/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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
#include "BooleanVariable.hh"
#include "Debug.hh"

namespace PLEXIL
{

  Assignment::Assignment(const VariableId lhs, 
						 const ExpressionId rhs,
						 const VariableId ack, 
						 const LabelStr& lhsName, 
                         const bool deleteLhs, 
						 const bool deleteRhs)
    : m_id(this),
	  m_dest(lhs),
	  m_ack(ack), 
	  m_rhs(rhs),
      m_destName(lhsName),
      m_value(Expression::UNKNOWN()),
      m_deleteLhs(deleteLhs), m_deleteRhs(deleteRhs)
  {
  }

  Assignment::~Assignment() 
  {
    if (m_deleteLhs)
      delete (Variable*) m_dest;
    if (m_deleteRhs)
      delete (Expression*) m_rhs;
    m_id.remove();
  }

  void Assignment::fixValue() 
  {
    m_value = m_rhs->getValue();
  }

  void Assignment::activate() 
  {
    m_rhs->activate();
    m_dest->activate();
  }

  void Assignment::deactivate() 
  {
    m_rhs->deactivate();
    m_dest->deactivate();
  }

  void Assignment::execute()
  {
	check_error(m_dest.isValid());
	debugMsg("Test:testOutput", "Assigning '" << m_destName.toString() <<
			 "' (" << m_dest->toString() << ") to " << Expression::valueToString(m_value));
	m_dest->setValue(m_value);
	m_ack->setValue(BooleanVariable::TRUE_VALUE());
  }

  const std::string& Assignment::getDestName() const 
  {
    return m_destName.toString();
  }

}
