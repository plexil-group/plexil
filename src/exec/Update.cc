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

#include "Update.hh"
#include "Debug.hh"
#include "Expression.hh"
#include "Node.hh"

namespace PLEXIL
{

  Update::Update(const NodeId& node, 
				 const ExpressionMap& pairs,
				 const VariableId ack,
				 const std::list<ExpressionId>& garbage)
    : m_id(this), m_source(node), m_pairs(pairs), m_ack(ack), m_garbage(garbage) 
  {
  }

  Update::~Update() 
  {
    for(std::list<ExpressionId>::const_iterator it = m_garbage.begin(); 
		it != m_garbage.end();
		++it)
      delete (Expression*) (*it);
    m_id.remove();
  }

  void Update::fixValues() 
  {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end();
		++it) {
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

  void Update::activate() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->activate();
    }
  }

  void Update::deactivate() {
    for(ExpressionMap::iterator it = m_pairs.begin(); it != m_pairs.end(); ++it) {
      it->second->deactivate();
    }
  }

}
