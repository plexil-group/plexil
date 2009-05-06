/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "ExecDefs.hh"

/*
  ExecIntf::lookupNow() -> StateCache::lookupNow() ->
  ExecIntf::performLookup -> ExecIntf::updateState ->
  StateCache::updateState
*/
namespace PLEXIL 
{
  // Forward references
  class ExternalInterface;
  typedef Id<ExternalInterface> ExternalInterfaceId;

  namespace Cache 
  {
    class Lookup;
    typedef Id<Lookup> LookupId;
      
    class Lookup 
    {
    public:
      Lookup(const ExpressionId& _source, const Expressions& _dest, 
             const StateKey& key);
      virtual ~Lookup();
      LookupId& getId(){return m_id;}
            
      ExpressionId source;
      Expressions dest;
      StateKey state;
    private:
      LookupId m_id;
            
    };
      
    class ChangeLookup : public Lookup 
    {
    public:
      ChangeLookup(const ExpressionId& _source, const Expressions& _dest,
                   const StateKey& key, 
                   const std::vector<double>& _tolerances);
      std::vector<double> tolerances;
      std::vector<double> previousValues;
    };
      
    class FrequencyLookup : public Lookup 
    {
    public:
      FrequencyLookup(const ExpressionId& _source, 
                      const Expressions& _dest, const StateKey& key,
                      const double _lowFreq, const double _highFreq);
      double lowFreq, highFreq, lastTime;
    };
  }
   
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
         
    /**
     * @brief Utility function for stringifying a vector of values,
     *        some of which may be double-ed LabelStrs.
     *
     * @param values The vector of values
     * @return A comma-delimited string
     */

    static std::string toString(const std::vector<double>& values);

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
     * FUNCTIONS FOR THE EXECUTIVE TO CALL
     */
         
    /**
     * @brief Perform an immediate lookup on a value.  If a value
     *        hasn't been gotten in this quiescence, a lookup will
     *        be performed on the outside world, otherwise the
     *        cached value is returned.
     *
     * @param source The Id of the LookupNow expression.
     * @param dest A vector of the expressions into which the
     *             values returned by the lookup will be stored.
     * @param state The state being looked up.
     * @see handleQuiescenceStarted, handleQuiescenceEnded
     */
    void lookupNow(const ExpressionId& source, Expressions& dest,
                   const State& state);
         
    /**
     * @brief Register a change lookup with the external world.
     *        Performs an immediate lookup in manner of lookupNow.
     *
     * @param source The Id of the LookupOnChange expression.
     * @param dest A vector of the expressions into which the
     *        values returned by the lookup will be stored.
     * @param state The state being watched.
     * @param tolerances A vector of the tolerances beyond which
     *                   the destinations should be informed of the
     *                   change.
     */
    void registerChangeLookup(const ExpressionId& source, 
                              Expressions& dest, const State& state, 
                              const std::vector<double>& tolerances);
         
    /**
     * @brief Register a change lookup with the external world.
     *        Performs an immediate lookup in manner of lookupNow.
     *        @param source The Id of the LookupOnChange
     *        expression.  @param dest A vector of the expressions
     *        into which the values returned by the lookup will be
     *        stored.  @param state The state being watched.
     *        @param lowFreq The longest time the lookup should
     *        wait between updates before setting UNKNOWN.  @param
     *        highFreq The least time the lookup will wait between
     *        updates.
     */
    void registerFrequencyLookup(const ExpressionId& source, 
                                 Expressions& dest, const State& state,
                                 const double lowFreq, 
                                 const double highFreq);
         
    /**
     * @brief Un-register a change lookup with the external world
     *        (this may allow the functional layer to stop sending
     *        data, saving bandwidth.
     */
    void unregisterChangeLookup(const ExpressionId& source);
         
    /**
     * @brief Un-register a frequency lookup with the external
     *        world (this may allow the functional layer to stop
     *        sending data, saving bandwidth.
     */
    void unregisterFrequencyLookup(const ExpressionId& source);
         
    /**
     * FUNCTIONS FOR THE EXTERNALINTERFACE TO CALL
     */
         
    /**
     * @brief Update a state in the cache with values in the
     *        external world.  Will cause updates of lookups on the
     *        state.  @param key The key of the state being updated
     *        (primarily to save bandwidth) @param values The new
     *        values for the state.
     */
    void updateState(const StateKey& key, const std::vector<double>& values);
         
         
    /**
     * @brief Update a state in the cache with values in the
     *        external world.  Will cause updates of lookups on the
     *        state.  @param state The state being updated @param
     *        values The new values for the state.
     */
    void updateState(const State& state, const std::vector<double>& values);
         
         
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
         
    /**
     * @brief Get the state key used to identify time.
     * @return The key.
     */
    const StateKey& getTimeStateKey() const;
         
    /**
     * @brief Get a unique key for a state, creating a new key for a new state.
     * @param state The state.
     * @param key The key.
     * @return True if a new key had to be generated.
     */
    bool keyForState(const State& state, StateKey& key);
         
    /**
     * @brief Get (a copy of) the State for this StateKey.
     * @param key The key to look up.
     * @param state The state associated with the key.
     * @return True if the key is found, false otherwise.
     */
    bool stateForKey(const StateKey& key, State& state) const;
         
         
  protected:
  private:
         
    /**
     * @brief Update lookups on a given state with the given
     *        values.  Also expires late frequency lookups.
     *
     * @param key The key for the state.
     * @param values The values for the update.
     */

    bool internalStateUpdate(const StateKey& key, 
                             const std::vector<double>& values);
         
    /**
     * @brief Conditionally update a change lookup if its
     *        tolerances are exceeded.
     *
     * @param lookup The lookup to possibly update.
     * @param values The values to update to.
     */

    bool updateChangeLookup(Cache::ChangeLookup* lookup, 
                            const std::vector<double>& values);
         
    /**
     * @brief Conditionally update a frequency lookup if the
     *        current time is within its limits.  If the update is
     *        too late, the destination expressions are set to
     *        UNKNOWN.  @param lookup The lookup to possibly
     *        update.  @param values The values to update to.
     *        @param time The current time.
     */
    bool updateFrequencyLookup(Cache::FrequencyLookup* lookup, 
                               const std::vector<double>& values,
                               const double& time);
         
    /**
     * @brief Remove a lookup from internal data structures.
     * @param source The un-registered lookup.
     */
    void internalUnregisterLookup(const ExpressionId& source);
         
    /**
     * @brief Set all expired frequency lookups (lookups that
     *        haven't gotten an update in the last lowFreq time
     *        units) to UNKNOWN.
     */
    //void expireFrequencyLookups();
         
    /**
     * @brief Compute the magnitude of the difference between x and y.
     * @param x First value.
     * @param y Second value.
     * @return The magnitude of the difference.  If both x and y
     *         are UNKNOWN, the magnitude is 0.  If exactly one of
     *         them is UNKNOWN, the magnitude is inf.  Otherwise,
     *         it's abs(x - y).
     */
    double differenceMagnitude(const double x, const double y) const;
         
    StateCacheId m_id; /*<! The Id for this cache. */
    ExternalInterfaceId m_interface;  /*<! The Id of the external interface. */
    bool m_inQuiescence; /*<! Flag indicating whether or not the exec is quiescing. */
    int m_quiescenceCount; /*<! A count of the number of times handleQuiescenceStarted has been called.  Used for synchronization and looking up out-of-date values.*/
    StateKey m_timeState; /*<! The key for the universal time state. */
    std::map<StateKey, std::pair<State, int> > m_states; /*<! A map of state keys to a pair of a state and the last quiescence the state was updated. */
    std::map<State, StateKey> m_keysByState; /*<! A map of states to their generated keys. */
    std::map<StateKey, std::vector<double> > m_values; /*<! A map of StateKeys to the latest values gotten for those states. */
    std::multimap<StateKey, Cache::LookupId> m_lookups; /*<! A map from StateKeys to the lookups on those states. */
    std::map<ExpressionId, Cache::LookupId> m_lookupsByExpression; /*<! A map from the lookup expressions to the interal lookup data structures*/
  };
}

#endif
