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

#include "Calculable.hh"

namespace PLEXIL
{
  Calculable::Calculable()
    : Mutable(),
      m_listener(*this)
  {
  }

  Calculable::~Calculable()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i) {
      m_subexpressions[i]->removeListener(m_listener.getId());
      if (m_garbage[i])
        delete (Expression *) m_subexpressions[i];
    }
    m_subexpressions.clear();
  }

  void Calculable::addSubexpression(const ExpressionId & exp, bool isGarbage)
  {
    // TODO: figure out when we can omit adding listener to subexps
    // For now play it safe
    exp->addListener(m_listener.getId());
    m_subexpressions.push_back(exp);
    m_garbage.push_back(exp);
  }

  void Calculable::finalize()
  {
  }

  void Calculable::handleActivate()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i)
      m_subexpressions[i]->activate();
  }

  void Calculable::handleDeactivate()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i)
      m_subexpressions[i]->deactivate();
  }


} // namespace PLEXIL
