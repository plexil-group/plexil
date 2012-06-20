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

#include "Update.hh"
#include "BooleanVariable.hh"
#include "Debug.hh"
#include "ExpressionFactory.hh"
#include "Node.hh"
#include "PlexilPlan.hh"

namespace PLEXIL
{
  Update::Update(const NodeId& node, 
                 const PlexilUpdateId& updateProto)
    : m_id(this),
      m_source(node),
      m_pairs(),
      m_ack((new BooleanVariable(BooleanVariable::UNKNOWN()))->getId()),
      m_garbage() 
  {
    // Make ack variable pretty
    ((VariableImpl*) m_ack)->setName(node->getNodeId().toString() + " ack");

    if (updateProto.isValid()) {
      for (std::vector<std::pair<std::string, PlexilExprId> >::const_iterator it =
             updateProto->pairs().begin();
           it != updateProto->pairs().end();
           ++it) {
        LabelStr nameStr(it->first);
        debugMsg("Node:createUpdate", "Adding pair '" << nameStr.toString());
        PlexilExprId foo = it->second;
        bool wasCreated = false;
        ExpressionId valueExpr = 
          ExpressionFactory::createInstance(foo->name(),
                                            foo,
                                            (NodeConnectorId) node,
                                            wasCreated);
        check_error(valueExpr.isValid());
        if (wasCreated)
          m_garbage.push_back(valueExpr);
        m_pairs.insert(std::make_pair((double) nameStr, valueExpr));
      }
    }
  }

  Update::~Update() 
  {
    m_pairs.clear();
    for (std::vector<ExpressionId>::const_iterator it = m_garbage.begin(); 
         it != m_garbage.end();
         ++it)
      delete (Expression*) (*it);
    m_garbage.clear();
    delete (Expression*) m_ack;
    m_id.remove();
  }

  void Update::fixValues() 
  {
    for (ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      check_error(it->second.isValid());
      std::map<double, double>::iterator valuePairIt =
        m_valuePairs.find(it->first);
      if (valuePairIt == m_valuePairs.end()) {
        // new pair, safe to insert
        m_valuePairs.insert(std::make_pair(it->first, it->second->getValue()));
      }
      else {
        // recycle old pair
        valuePairIt->second = it->second->getValue();
      }
      debugMsg("Update:fixValues",
               " fixing pair '" << LabelStr(it->first).toString() << "', "
               << it->second->getValue());
    }
  }

  void Update::activate() 
  {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->activate();
    }
    m_ack->activate();
  }

  void Update::deactivate() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->deactivate();
    }
    m_ack->deactivate();
  }

  void Update::reset() {
    m_ack->reset();
  }

}
