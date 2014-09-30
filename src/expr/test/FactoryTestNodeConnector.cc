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

#include "FactoryTestNodeConnector.hh"

#include "PlexilExpr.hh"
#include "Expression.hh"

// *** TEMP DEBUG ***
#include <iostream>

namespace PLEXIL
{
  FactoryTestNodeConnector::FactoryTestNodeConnector()
    : NodeConnector()
  {
  }

  FactoryTestNodeConnector::~FactoryTestNodeConnector()
  {
    TestVariableMap::iterator it = m_variableMap.begin();
    while (it != m_variableMap.end()) {
      // *** TEMP DEBUG ***
      std::cout << "Deleting " << it->first << std::endl;
      // *** END TEMP DEBUG ***
      delete it->second;
      m_variableMap.erase(it);
      it = m_variableMap.begin();
    }
    m_variableMap.clear();
  }

  Expression *FactoryTestNodeConnector::findVariable(const PlexilVarRef* ref)
  {
    return this->findVariable(ref->varName(), false);
  }

  Expression *FactoryTestNodeConnector::findVariable(const std::string & name,
                                                     bool ignored)
  {
    TestVariableMap::const_iterator it = m_variableMap.find(name);
    if (it != m_variableMap.end())
      return it->second;
    else
      return NULL;
  }

  Node *FactoryTestNodeConnector::findNodeRef(PlexilNodeRef const * /* nodeRef */)
  {
    return NULL;
  }

  std::string const &FactoryTestNodeConnector::getNodeId() const
  {
    static std::string sl_empty;
    return sl_empty;
  }

  Node const *FactoryTestNodeConnector::findChild(const std::string& childName) const
  {
    return NULL;
  }
   
  Node *FactoryTestNodeConnector::findChild(const std::string& childName)
  {
    return NULL;
  }

  Node *FactoryTestNodeConnector::getParent()
  {
    return NULL;
  }

  Node const *FactoryTestNodeConnector::getParent() const
  {
    return NULL;
  }

  void FactoryTestNodeConnector::storeVariable(const std::string & name, Expression *var)
  {
    TestVariableMap::iterator it = m_variableMap.find(name);
    if (it != m_variableMap.end()) {
      it->second = var; // replace existing
    }
    else 
      m_variableMap.insert(std::pair<std::string, Expression *>(name, var));
  }

} // namespace PLEXIL
