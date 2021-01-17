/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "ValueType.hh"

namespace PLEXIL
{
  // Forward references
  class CachedValue;
  class Lookup;
  class LookupReceiver;
  class State;
  class Value;

  //!
  // @class StateCacheEntry
  // @brief Provides the external API for a state cache entry,
  //        and value-type-independent state and functionality.
  //
  class StateCacheEntry
  {
  public:
    virtual ~StateCacheEntry() = default;

    // Utility
    virtual ValueType const valueType() const = 0;
    virtual bool isKnown() const = 0;

    // Callback object for interface implementations
    virtual LookupReceiver *getLookupReceiver() = 0;

    // Safety check before deleting entry
    virtual bool hasRegisteredLookups() const = 0;

    // API to Lookup
    virtual void registerLookup(State const &s, Lookup *l) = 0;
    virtual void unregisterLookup(State const &s, Lookup *l) = 0;
    virtual void updateThresholds(State const &s) = 0;

    // Read access to the actual value is through the helper object.
    virtual CachedValue const *cachedValue() const = 0;

    //!
    // @brief Set the state to unknown.
    // @note Notifies all lookups of the new status.
    //
    virtual void setUnknown() = 0;

    //!
    // @brief Update the cache entry with the given new value.
    // @param val The new value.
    // @note Notifies all lookups of the new value.
    // @note The caller is responsible for deleting the object pointed to upon return.
    //
    virtual void update(Boolean const &val) = 0;
    virtual void update(Integer const &val) = 0;
    virtual void update(Real const &val) = 0;
    virtual void update(String const &val) = 0;
    virtual void updatePtr(String const *valPtr) = 0;
    virtual void updatePtr(BooleanArray const *valPtr) = 0;
    virtual void updatePtr(IntegerArray const *valPtr) = 0;
    virtual void updatePtr(RealArray const *valPtr) = 0;
    virtual void updatePtr(StringArray const *valPtr) = 0;

    // For convenience of TestExternalInterface, others
    virtual void update(Value const &val) = 0;

  protected:
    // Constructor is only accessible to the implementation class
    StateCacheEntry() = default;

  private:
    // Copy disallowed
    StateCacheEntry(StateCacheEntry const &orig) = delete;
    StateCacheEntry(StateCacheEntry &&orig) = delete;

    // Assign disallowed
    StateCacheEntry &operator=(StateCacheEntry const &) = delete;
    StateCacheEntry &operator=(StateCacheEntry &&) = delete;
  };

  StateCacheEntry *makeStateCacheEntry();

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_ENTRY_HH
