/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#ifndef _H_StateCache
#define _H_StateCache

#include "ConstantMacros.hh"
#include "ExecDefs.hh"
#include "Expression.hh"

#include <map>
#include <set>

namespace PLEXIL 
{
  // Forward references
  class ExternalInterface;
  DECLARE_ID(ExternalInterface);

  class LookupDesc;
  DECLARE_ID(LookupDesc);

  class CacheEntry;
  DECLARE_ID(CacheEntry);
   
  /**
   * @brief The cache for state in the external world.  Handles
   *        updating multiple lookups for the same state.
   */
  class StateCache 
  {
  public:

    /**
     * @brief Utility function for stringifying a State.
     * @param state The state
     * @return A string usually of the form \<state name\>\((argument)*\)
     */

    static std::string toString(const State& state);

    StateCache();
    ~StateCache();

    const StateCacheId& getId() const {return m_id;}

    void setExternalInterface(ExternalInterfaceId& id)
    {
      m_interface = id;
    }

    inline ExternalInterfaceId& getExternalInterface()
    {
      return m_interface;
    }

    /**
     * FUNCTIONS FOR EXPRESSIONS TO CALL
     */

    /**
     * @brief Perform an immediate lookup on a value, and register it for updates 
     *        while it is active.
     *        If a value hasn't been gotten in this quiescence, a lookup will
     *        be performed on the outside world, otherwise the cached value
     *        is stored in the destination expressions.
     *
     * @param expr The Id of the LookupNow expression.
     * @param state The state being looked up.
     * @see handleQuiescenceStarted, handleQuiescenceEnded
     */
    void registerLookupNow(const ExpressionId& expr, const State& state);
         
    /**
     * @brief Un-register a lookup with the external world. Stop future updates of this expression.
     */
    void unregisterLookupNow(const ExpressionId& source);
         
    /**
     * @brief Register a change lookup with the external world.
     *        Performs an immediate lookup in manner of lookupNow.
     *
     * @param expr The Id of the LookupOnChange expression.
     * @param state The state being watched.
     * @param tolerance A tolerance beyond which the expression
     *                  should be informed of the change.
     */
    void registerChangeLookup(const ExpressionId& expr, 
                              const State& state, 
                              double tolerance);
         
    /**
     * @brief Un-register a change lookup with the external world
     *        (this may allow the functional layer to stop sending
     *        data, saving bandwidth.
     */
    void unregisterChangeLookup(const ExpressionId& source);
         
    /**
     * FUNCTIONS FOR THE EXTERNAL INTERFACE TO CALL
     */
         
    /**
     * @brief Update a state in the cache with a value from the
     *        external world.  Will cause updates of lookups on the
     *        state.  
     * @param state The state being updated 
     * @param value The new value for the state.
     * @note Apparently only used by the Exec regression tester and TestExec.
     */
    void updateState(const State& state, const Value& value);

    /**
     * @brief Get the last known value of the state.
     * @param state The state.
     * @return The value.
     * @note Returns UNKNOWN() if the state is not currently in the cache.
     * @note May only be called during quiescence.
     */
    const Value& getLastValue(const State& state);
         
    /**
     * @brief Put the cache in a state that is ready for lookup registration
     */
    void handleQuiescenceStarted();
         
    /**
     * @brief Put the cache in a state that is ready for state updates.
     */
    void handleQuiescenceEnded();
         
    /**
     * @brief Check whether we are in a quiescence cycle.
     * @return true if in quiescence cycle, false otherwise.
     */
    bool inQuiescence() const
    {
      return m_inQuiescence; 
    }
         
    /**
     * @brief Get the cache's current notion of the time.
     * @return The time.
     */
    double currentTime();
         
    /**
     * @brief Get the state used to identify time.
     * @return The state.
     */
    const State& getTimeState() const;
         
  private:
      
    /**
     * @brief Generate or find the cache entry for this state.
     * @param state The state being looked up.
     * @param entry Pointer to the CacheEntry for the state.
     */

    CacheEntryId ensureCacheEntry(const State& state);
         
    /**
     * @brief Update lookups on a given state with the given value.
     * @param entry Pointer to the CacheEntry for this state. 
     * @param value The new value.
     * @return True if the update moved the thresholds, false otherwise.
     */

    bool internalStateUpdate(const CacheEntryId& entry, const Value& value);
         
    /**
     * @brief Remove a lookup from internal data structures.
     * @param source The un-registered lookup.
     * @return the CacheEntryId corresponding to the removed lookup
     */
    CacheEntryId internalUnregisterLookup(const ExpressionId& source);

    /**
     * @brief Get the current time and update all subscribers.
     */
    void updateTimeState();

    typedef std::map<State, CacheEntryId> StateCacheMap;
    typedef std::map<ExpressionId, LookupDescId> ExpressionToLookupMap;

    StateCacheMap m_states; /*<! All data relevant to the cached states */
    ExpressionToLookupMap m_lookupsByExpression; /*<! A map from the lookup expressions to the interal lookup data structures*/
    StateCacheId m_id; /*<! The Id for this cache. */
    ExternalInterfaceId m_interface;  /*<! The Id of the external interface. */
    CacheEntryId m_timeEntry; /*<! Pointer to the time entry in the cache. */
    State m_timeState; /*<! The universal time state. */
    bool m_inQuiescence; /*<! Flag indicating whether or not the exec is quiescing. */
    int m_quiescenceCount; /*<! A count of the number of times handleQuiescenceStarted has been called.  Used for synchronization and looking up out-of-date values.*/
  };
}

#endif
