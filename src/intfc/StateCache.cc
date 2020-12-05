/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "StateCache.hh"

#include "CachedValue.hh"
#include "Error.hh"
#include "ExternalInterface.hh"
#include "State.hh"
#include "StateCacheEntry.hh"

#include <map>
#include <memory>

namespace PLEXIL
{

  // Implementation of time query functions
  double StateCache::currentTime()
  {
    double result = 0.0;
    instance().m_timeEntry->cachedValue()->getValue(result);
    return result;
  }

  double StateCache::queryTime()
  {
    // Update the cached value
    g_interface->lookupNow(State::timeState(),
                           instance().m_timeEntry->getLookupReceiver());
    // and return it
    return currentTime();
  }

  class StateCacheImpl final : public StateCache
  {
  private:

    using EntryMap = std::map<State, std::unique_ptr<StateCacheEntry> >;
    EntryMap m_map;

  public:

    StateCacheImpl()
      : StateCache(),
        m_map()
    {
      // Initialize time state to 0
      m_timeEntry = ensureStateCacheEntry(State::timeState());
      m_timeEntry->update((double) 0);
    }

    virtual ~StateCacheImpl() = default;

    virtual StateCacheEntry *ensureStateCacheEntry(State const &state)
    {
      EntryMap::iterator iter = m_map.find(state);
      if (iter == m_map.end())
        iter = m_map.emplace(state, makeStateCacheEntry()).first;
      return iter->second.get();
    }

    virtual LookupReceiver *getLookupReceiver(State const &state)
    {
      return ensureStateCacheEntry(state)->getLookupReceiver();
    }

  private:

    // Unimplemented
    StateCacheImpl(StateCacheImpl const &) = delete;
    StateCacheImpl(StateCacheImpl &&) = delete;
    StateCacheImpl &operator=(StateCacheImpl const &) = delete;
    StateCacheImpl &operator=(StateCacheImpl &&) = delete;

  };

  StateCache &StateCache::instance()
  {
    static StateCacheImpl sl_instance;
    return static_cast<StateCache &>(sl_instance);
  }

} // namespace PLEXIL
