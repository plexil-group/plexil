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

#include "NodeVariableMap.hh"

#include <cstddef>
#include <cstdlib>

namespace PLEXIL
{
  typedef SimpleMap<char const *, Expression *, VariableComp> BaseMap;

  NodeVariableMap::NodeVariableMap(NodeVariableMap *parentMap)
    : BaseMap(),
      m_parentMap(parentMap)
  {
  }

  NodeVariableMap::~NodeVariableMap()
  {
    clear();
  }

  void NodeVariableMap::setParentMap(NodeVariableMap *parent)
  {
    m_parentMap = parent;
  }

  void NodeVariableMap::clear()
  {
    // Delete all the key strings we've copied
    for (MapVector::iterator it = BaseMap::m_vector.begin();
         it != BaseMap::m_vector.end();
         ++it) {
      free(const_cast<char *>(it->first));
      it->first = NULL;
    }
  }

  Expression *NodeVariableMap::findVariable(char const *name)
  {
    iterator it = find(name);
    if (it != end())
      return it->second;
    // Iteratively search ancestors for this name
    for (NodeVariableMap *ancestor = m_parentMap;
         ancestor;
         ancestor = ancestor->m_parentMap) {
      it = ancestor->find(name);
      if (it != ancestor->end())
        return it->second;
    }
    return NULL;
  }
   
  // Copy key on insert
  BaseMap::iterator 
  NodeVariableMap::insertEntry(BaseMap::iterator it,
                               char const * const &k,
                               Expression * const &v)
  {
    return BaseMap::m_vector.insert(it, BaseMap::MapEntry(strdup(k), v));
  }


} // namespace PLEXIL
