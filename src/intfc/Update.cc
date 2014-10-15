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

#include "Update.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExternalInterface.hh"
#include "NodeConnector.hh"

namespace PLEXIL
{

  Update::Update(NodeConnector *node)
    : m_source(node),
      m_ack(),
      m_garbage(),
      m_pairs()
  {
    // Make ack variable pretty
    m_ack.setName(node->getNodeId() + " ack");
  }

  Update::~Update() 
  {
    m_valuePairs.clear();
    m_pairs.clear();
    for (std::vector<Expression *>::const_iterator it = m_garbage.begin(); 
         it != m_garbage.end();
         ++it)
      delete (*it);
    m_garbage.clear();
  }

  void Update::reservePairs(size_t n)
  {
    m_pairs.grow(n);
    m_valuePairs.grow(n);
  }

  bool Update::addPair(std::string const &name, Expression *exp, bool garbage)
  {
    debugMsg("Update:addPair", " name = \"" << name << "\", exp = " << *exp);
    if (m_pairs.find(name) != m_pairs.end())
      return false;
    m_pairs[name] = exp;
    if (garbage)
      m_garbage.push_back(exp);
    return true;
  }

  void Update::fixValues()
  {
    for (PairExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      check_error_1(it->second);
      m_valuePairs[it->first] = it->second->toValue();
      debugMsg("Update:fixValues",
               " fixing pair \"" << it->first << "\", "
               << *it->second << " = " << it->second->toValue());
    }
  }

  void Update::activate() 
  {
    for (PairExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it)
      it->second->activate();
    m_ack.activate();
  }

  void Update::execute()
  {
    assertTrue_1(m_ack.isActive());
    fixValues();
    g_interface->enqueueUpdate(this);
  }

  void Update::acknowledge(bool ack)
  {
    if (!m_ack.isActive())
      return; // ignore if not executing
    m_ack.setValue(ack);
  }

  void Update::deactivate()
  {
    assertTrue_1(m_ack.isActive());
    for(PairExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it)
      it->second->deactivate();
    m_ack.deactivate();
  }

  void Update::reset() 
  {
    m_ack.reset();
  }

}
