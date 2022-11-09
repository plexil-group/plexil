// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_STATE_CACHE_ENTRY_HH
#define PLEXIL_STATE_CACHE_ENTRY_HH

#include "LookupReceiver.hh"

#include <memory> // std::unique_ptr

namespace PLEXIL
{
  // Forward references
  class CachedValue;
  class Lookup;
  class State;
  class Value;

  //! \class StateCacheEntry
  //! \brief Stateless abstract base class which defines the external
  //!        API for a state cache entry, and value-type-independent
  //!        state and functionality.
  //! \ingroup External-Interface
  class StateCacheEntry : public LookupReceiver
  {
  public:

    //! \brief Get a pointer to a callback object for this state.
    //! \return Pointer to this as a pointer to LookupReceiver.
    LookupReceiver *getLookupReceiver()
    {
      return static_cast<LookupReceiver *>(this);
    }

    //! \brief Return the value type of this entry.
    //! \return The value type.
    virtual ValueType const valueType() const = 0;

    //! \brief Is the value of this entry known?
    //! \return true if known, false if unknown. 
    virtual bool isKnown() const = 0;

    //! \brief Determine if there are any Lookups currently registered
    //!        for this state.
    //! \return true if Lookups are currently registered, false otherwise.
    virtual bool hasRegisteredLookups() const = 0;

    //
    // API to Lookup
    //

    //! \brief Register a Lookup expression with this State.
    //! \param s Const reference to the state.
    //! \param l Pointer to the Lookup.
    virtual void registerLookup(State const &s, Lookup *l) = 0;

    //! \brief Remove the association between a Lookup expression and this State.
    //! \param s Const reference to the state.
    //! \param l Pointer to the Lookup.
    virtual void unregisterLookup(State const &s, Lookup *l) = 0;

    //! \brief Update the LookupOnChange thresholds for the given state.
    //! \param s Const reference to the state.
    virtual void updateThresholds(State const &s) = 0;

    //! \brief Get the CachedValue instance associated with this entry.
    //! \return Const pointer to the CachedValue.
    //! \note Read access to the actual value is through the helper object.
    virtual CachedValue const *cachedValue() const = 0;

    //! \brief Update the cache entry with the given new value.
    //! \param val Const reference to the new value.
    //! \note If the new value differs from the old, notifies all active lookups of the new value.
    //! \note Optimization for StateCache::lookupReturn()
    virtual void updateValue(Value const &val, unsigned int timestamp) = 0;

    ///@{
    //! Update the cache entry with the given new value.
    //! @param valPtr The new value.
    //! @note Notifies all lookups of the new value.
    //! @note The caller is responsible for deleting the object pointed to upon return.
    virtual void updatePtr(String const *valPtr) = 0;
    virtual void updatePtr(BooleanArray const *valPtr) = 0;
    virtual void updatePtr(IntegerArray const *valPtr) = 0;
    virtual void updatePtr(RealArray const *valPtr) = 0;
    virtual void updatePtr(StringArray const *valPtr) = 0;
    ///@}
  };

  //! \brief Construct a StateCacheEntry instance.
  //! \return Unique pointer to the new instance.
  std::unique_ptr<StateCacheEntry> makeStateCacheEntry();

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_ENTRY_HH
