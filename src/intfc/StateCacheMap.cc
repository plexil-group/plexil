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

#include "StateCacheMap.hh"

#include "StateCacheEntry.hh"

namespace PLEXIL
{
  StateCacheMap::StateCacheMap()
  {
    // Initialize time state to 0
    ensureStateCacheEntry(State::timeState(), DATE_TYPE)->update(0);
  }

  StateCacheMap::~StateCacheMap()
  {
    // Delete all StateCacheEntry instances
    EntryMap::iterator it = m_map.begin();
    while (it != m_map.end()) {
      delete it->second;
      m_map.erase(it);
      it = m_map.begin();
    }
  }

  StateCacheMap &StateCacheMap::instance()
  {
    static StateCacheMap sl_instance;
    return sl_instance;
  }

  StateCacheEntry *StateCacheMap::ensureStateCacheEntry(State const &state,
                                                        ValueType vtype)
  {
    EntryMap::iterator it = m_map.find(state);
    // FIXME: check value type
    if (it == m_map.end())
      it = m_map.insert(EntryPair(state, StateCacheEntry::factory(state, vtype))).first;
    return it->second;
  }

  StateCacheEntry *StateCacheMap::findStateCacheEntry(State const &state)
  {
    EntryMap::iterator it = m_map.find(state);
    if (it != m_map.end())
      return it->second;
    else
      return NULL;
  }

  void StateCacheMap::removeStateCacheEntry(State const &state)
  {
    EntryMap::iterator it = m_map.find(state);
    if (it == m_map.end())
      return;
    StateCacheEntry *entry = it->second;
    m_map.erase(it);
    delete entry;
  }

} // namespace PLEXIL
