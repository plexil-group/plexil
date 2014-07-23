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

#ifndef PLEXIL_STATE_CACHE_ENTRY_HH
#define PLEXIL_STATE_CACHE_ENTRY_HH

#include "State.hh"

namespace PLEXIL
{
  class CachedValue;
  class Expression;
  class Lookup;

  /**
   * @class StateCacheEntry
   * @brief Provides the external API for a state cache entry,
   *        and value-type-independent state and functionality.
   */
  class StateCacheEntry
  {
  public:
    StateCacheEntry(State const &state);
    StateCacheEntry(StateCacheEntry const &); // needed by StateCacheMap for creation

    virtual ~StateCacheEntry();

    State const &state() const { return m_state; }

    // Utility
    bool operator<(StateCacheEntry const &y) const;
    ValueType const valueType() const;
    bool isKnown() const;

    // API to Lookup
    void registerLookup(Lookup *); // calls updateIfStale()
    virtual void unregisterLookup(Lookup *);

    // Can be called multiple times
    void setThresholds(Expression const *tolerance);

    // Read access to the actual value is through the helper object.
    CachedValue const *cachedValue() const;

    /**
     * @brief Set the state to unknown.
     * @note Notifies all lookups of the new status.
     */
    void setUnknown();

    /**
     * @brief Update the cache entry with the given new value.
     * @param val The new value.
     * @note Notifies all lookups of the new value.
     * @note The caller is responsible for deleting the object pointed to upon return.
     */
    void update(unsigned int timestamp, bool const &val);
    void update(unsigned int timestamp, int32_t const &val);
    void update(unsigned int timestamp, double const &val);
    void update(unsigned int timestamp, std::string const &val);
    void updatePtr(unsigned int timestamp, std::string const *valPtr);
    void updatePtr(unsigned int timestamp, BooleanArray const *valPtr);
    void updatePtr(unsigned int timestamp, IntegerArray const *valPtr);
    void updatePtr(unsigned int timestamp, RealArray const *valPtr);
    void updatePtr(unsigned int timestamp, StringArray const *valPtr);

    // For convenience of TestExternalInterface, others
    void update(unsigned int timestamp, Value const &val);

  protected:
    // Internal functions

    /**
     * @brief Notify all subscribers of a change in value.
     */
    void notify() const;

    /*
     * @brief Check if the cached value is current, and update if required.
     */
    void updateIfStale();

    // Return true if entry type is compatible with requested, false if not.
    bool ensureCachedValue(ValueType v = UNKNOWN_TYPE);

  private:
    // Default, assign disallowed
    StateCacheEntry();
    StateCacheEntry &operator=(StateCacheEntry const &);

    State const m_state;
    std::vector<Lookup *> m_lookups;
    CachedValue *m_value;
  };

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_ENTRY_HH
