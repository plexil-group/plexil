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

#include "LookupReceiver.hh"

namespace PLEXIL
{
  // Forward references
  class CachedValue;
  class Lookup;
  class State;
  class Value;

  //! @class StateCacheEntry
  //! Provides the external API for a state cache entry, and
  //! value-type-independent state and functionality.
  class StateCacheEntry : public LookupReceiver
  {
  public:
    virtual ~StateCacheEntry() = default;

    // Callback object for interface implementations
    LookupReceiver *getLookupReceiver()
    {
      return static_cast<LookupReceiver *>(this);
    }

    // Utility
    virtual ValueType const valueType() const = 0;
    virtual bool isKnown() const = 0;

    // Safety check before deleting entry
    virtual bool hasRegisteredLookups() const = 0;

    // API to Lookup
    virtual void registerLookup(State const &s, Lookup *l) = 0;
    virtual void unregisterLookup(State const &s, Lookup *l) = 0;
    virtual void updateThresholds(State const &s) = 0;

    // Read access to the actual value is through the helper object.
    // Only Lookup and StateCache should use this member function.
    virtual CachedValue const *cachedValue() const = 0;

    //! Update with the given value and timestamp.
    //! @param val The new value.
    //! @param timestamp The cycle count at the time of update.
    //! @note Optimization for StateCache::lookupReturn()
    virtual void updateValue(Value const &val, unsigned int timestamp) = 0;

    //! Update the cache entry with the given new value.
    //! @param valPtr The new value.
    //! @note Notifies all lookups of the new value.
    //! @note The caller is responsible for deleting the object pointed to upon return.
    virtual void updatePtr(String const *valPtr) = 0;
    virtual void updatePtr(BooleanArray const *valPtr) = 0;
    virtual void updatePtr(IntegerArray const *valPtr) = 0;
    virtual void updatePtr(RealArray const *valPtr) = 0;
    virtual void updatePtr(StringArray const *valPtr) = 0;
  };

  StateCacheEntry *makeStateCacheEntry();

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_ENTRY_HH
