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

#ifndef PLEXIL_STATE_CACHE_HH
#define PLEXIL_STATE_CACHE_HH

#include "State.hh"

namespace PLEXIL
{
  // Forward references
  class LookupReceiver;
  class StateCacheEntry;

  /**
   * @class StateCache
   * @brief Where the Exec's notion of external state is stored
   */
  class StateCache
  {
  public:
    virtual ~StateCache() = default;

    static StateCache &instance();

    //!
    // @brief Get the most recently cached value of the time.
    // @return The cached time value, as a double.
    //
    // @note The return type of this function may change when
    //       PLEXIL implements a real Date type.
    static double currentTime();

    //!
    // @brief Query the clock to get the time.
    // @return The actual time, as a double.
    //
    // @note The return type of this function may change when
    //       PLEXIL implements a real Date type.
    static double queryTime();

    /**
     * @brief Construct or find the cache entry for this state.
     * @param state The state being looked up.
     * @return Pointer to the StateCacheEntry for the state.
     * @note Return value can be presumed to be non-null.
     */
    virtual StateCacheEntry *ensureStateCacheEntry(State const &state) = 0;

    virtual LookupReceiver *getLookupReceiver(State const &state) = 0;

  protected:

    // Default constructor is only accessible to the implementation class
    StateCache() = default;

    // Time entry has to be initialized by the implementation constructor
    StateCacheEntry *m_timeEntry;

  private:

    // Unimplemented
    StateCache(StateCache const &) = delete;
    StateCache(StateCache &&) = delete;
    StateCache &operator=(StateCache const &) = delete;
    StateCache &operator=(StateCache &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_HH
