// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
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

#include "StateCacheEntry.hh"

#include "ArrayImpl.hh"
#include "CachedValue.hh"
#include "Debug.hh"
#include "Dispatcher.hh"
#include "Error.hh"
#include "Lookup.hh"
#include "State.hh"
#include "StateCache.hh"

#include <algorithm> // std::find

namespace PLEXIL
{

  //! \class StateCacheEntryImpl
  //! \brief Concrete implementation of the StateCacheEntry API.
  class StateCacheEntryImpl final : public StateCacheEntry
  {
  private:

    //! \brief Convenience typedef.
    using CachedValuePtr = std::unique_ptr<CachedValue>;

  public:

    //! \brief Default constructor.
    StateCacheEntryImpl()
      : m_value(),
        m_lowThreshold(),
        m_highThreshold()
    {
    }

    //! \brief Virtual destructor.
    virtual ~StateCacheEntryImpl() = default;

    //! \brief Return the type of the cached value.
    //! \return The value type.
    virtual ValueType const valueType() const
    {
      if (m_value)
        return m_value->valueType();
      return UNKNOWN_TYPE;
    }

    //! \brief Determine whether the cached value is known or unknown.
    //! \return true if known, false otherwise.
    virtual bool isKnown() const
    {
      if (m_value)
        return m_value->isKnown();
      return false;
    }

    //! \brief Determine if there are any Lookups currently registered
    //!        for this state.
    //! \return true if Lookups are currently registered, false otherwise.
    virtual bool hasRegisteredLookups() const
    {
      return !m_lookups.empty();
    }

    //! \brief Register a Lookup expression with this State.
    //! \param s Const reference to the state.
    //! \param l Pointer to the Lookup.
    virtual void registerLookup(State const &state, Lookup *lkup)
    {
      m_lookups.push_back(lkup);
      debugMsg("StateCacheEntry:registerLookup",
               ' ' << state << " now has " << m_lookups.size() << " lookups");
      // Update if stale
      if ((!m_value) || m_value->getTimestamp() < StateCache::instance().getCycleCount()) {
        debugMsg("StateCacheEntry:registerLookup", ' ' << state << " updating stale value");
        g_dispatcher->lookupNow(state, getLookupReceiver());
      }
    }

    //! \brief Remove the association between a Lookup expression and this State.
    //! \param s Const reference to the state.
    //! \param l Pointer to the Lookup.
    virtual void unregisterLookup(State const &state, Lookup *lkup)
    {
      debugMsg("StateCacheEntry:unregisterLookup", ' ' << state);

      if (m_lookups.empty())
        return; // can't possibly be registered

      // Somewhat likely to remove last item first, so check for that special case.
      // TODO: analyze to see if this is true!
      if (lkup == m_lookups.back())
        m_lookups.pop_back();
      else {
        std::vector<Lookup *>::iterator iter =
          std::find(m_lookups.begin(), m_lookups.end(), lkup);
        if (iter != m_lookups.end())
          m_lookups.erase(iter);
        else {
          debugMsg("StateCacheEntry:unregisterLookup", ' ' << state << " lookup not found")
            return;
        }
      }

      if (m_lookups.empty()) {
        debugMsg("StateCacheEntry:unregisterLookup",
                 ' ' << state << " no lookups remaining, unsubscribing");
        if (m_lowThreshold || m_highThreshold) {
          m_lowThreshold.reset();
          m_highThreshold.reset();
        }
      }
      else if (m_lowThreshold || m_highThreshold) {
        // Check whether thresholds should be updated
        debugMsg("StateCacheEntry:unregisterLookup",
                 ' ' << state << " updating thresholds from remaining "
                 << m_lookups.size() << " lookups");
        updateThresholds(state);
      }
    }

    //! \brief Update the LookupOnChange thresholds for the given state.
    //! \param s Const reference to the state.
    virtual void updateThresholds(State const &state)
    {
      // Survey lookups to determine if the thresholds
      // need to be established, changed, or deleted.
      ValueType vtype = m_value->valueType();
      bool hasThresholds = false;

      switch (vtype) {
      case INTEGER_TYPE:
        hasThresholds = integerUpdateThresholds(state);
        break;

        // FIXME: support non-Real date/duration types
      case DATE_TYPE:
      case DURATION_TYPE:

      case REAL_TYPE: {
        hasThresholds = realUpdateThresholds(state);
        break;
      }

      default:
        // this is a plan error
        warn("LookupOnChange: lookup value of type " << valueTypeName(vtype)
             << " does not allow a tolerance");
        return;
      }

      if (!hasThresholds) {
        debugMsg("StateCacheEntry:updateThresholds",
                 ' ' << state << " no change lookups remaining, clearing thresholds");
        m_lowThreshold.reset();
        m_highThreshold.reset();
      }
    }

    //! \brief Get the CachedValue instance associated with this entry.
    //! \return Const pointer to the CachedValue.
    //! \note Read access to the actual value is through the helper object.
    virtual CachedValue const *cachedValue() const
    {
      return m_value.get();
    }

    //! \brief Update the cache entry with the given new value.
    //! \param val Const reference to the new value.
    //! \note If the new value differs from the old, notifies all active lookups of the new value.
    //! \note Optimization for StateCache::lookupReturn()
    virtual void updateValue(Value const &val, unsigned int timestamp)
    {
      if (!ensureCachedValue(val.valueType()))
        return;
      if (m_value->update(timestamp, val))
        notify();
    }

    //
    // LookupReceiver API
    //

    //! \brief Assign a new value to this Lookup.
    //! \param val Const reference to the new Value.
    virtual void update(Value const &val)
    {
      if (!ensureCachedValue(val.valueType()))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), val))
        notify();
    }
    
    //! \brief Make the value of this Lookup unknown.
    virtual void setUnknown()
    {
      if (m_value && m_value->setUnknown(StateCache::instance().getCycleCount()))
        notify();
    }

    ///@{
    //! \brief Assign a new value of a specific type to this Lookup.
    //! \param val The new value.
    virtual void update(Boolean val)
    {
      if (!ensureCachedValue(BOOLEAN_TYPE))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), val))
        notify();
    }

    virtual void update(Integer val)
    {
      if (!ensureCachedValue(INTEGER_TYPE))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), val))
        notify();
    }

    virtual void update(Real val)
    {
      if (!ensureCachedValue(REAL_TYPE))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), val))
        notify();
    }
    ///@}

    ///@{
    //! \brief Assign a new string value to this Lookup.
    //! \param val The new value.
    virtual void update(String const &val)
    {
      if (!ensureCachedValue(STRING_TYPE))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), val))
        notify();
    }

    virtual void update(char const *val)
    {
      if (!ensureCachedValue(STRING_TYPE))
        return;
      if (m_value->update(StateCache::instance().getCycleCount(), String(val)))
        notify();
    }
    ///@}

    ///@{
    //! \brief Assign a new array value to this Lookup.
    //! \param ary The array.
    //! \param size The number of elements in the array.
    virtual void update(Boolean const ary[], size_t size)
    {
      std::vector<Boolean> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      BooleanArray array(v);
      updatePtr(&array);
    }

    virtual void update(Integer const ary[], size_t size)
    {
      std::vector<Integer> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      IntegerArray array(v);
      updatePtr(&array);
    }

    virtual void update(Real const ary[], size_t size)
    {
      std::vector<Real> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      RealArray array(v);
      updatePtr(&array);
    }

    virtual void update(String const ary[], size_t size)
    {
      std::vector<String> v(size);
      for (size_t i = 0; i < size; ++i)
        v[i] = ary[i];
      StringArray array(v);
      updatePtr(&array);
    }
    ///@}

    //
    // StateCacheEntry API
    //

    ///@{
    //! Update the cache entry with the given new value.
    //! @param valPtr The new value.
    //! @note Notifies all lookups of the new value.
    //! @note The caller is responsible for deleting the object pointed to upon return.
    virtual void updatePtr(String const *valPtr)
    {
      if (!ensureCachedValue(STRING_TYPE))
        return;
      if (m_value->updatePtr(StateCache::instance().getCycleCount(), valPtr))
        notify();
    }

    virtual void updatePtr(BooleanArray const *valPtr)
    {
      if (!ensureCachedValue(BOOLEAN_ARRAY_TYPE))
        return;
      if (m_value->updatePtr(StateCache::instance().getCycleCount(), valPtr))
        notify();
    }

    virtual void updatePtr(IntegerArray const *valPtr)
    {
      if (!ensureCachedValue(INTEGER_ARRAY_TYPE))
        return;
      if (m_value->updatePtr(StateCache::instance().getCycleCount(), valPtr))
        notify();
    }

    virtual void updatePtr(RealArray const *valPtr)
    {
      if (!ensureCachedValue(REAL_ARRAY_TYPE))
        return;
      if (m_value->updatePtr(StateCache::instance().getCycleCount(), valPtr))
        notify();
    }

    virtual void updatePtr(StringArray const *valPtr)
    {
      if (!ensureCachedValue(STRING_ARRAY_TYPE))
        return;
      if (m_value->updatePtr(StateCache::instance().getCycleCount(), valPtr))
        notify();
    }
    ///@}

  private:

    // Copy disallowed
    StateCacheEntryImpl(StateCacheEntryImpl const &orig) = delete;
    StateCacheEntryImpl(StateCacheEntryImpl &&orig) = delete;

    // Assign disallowed
    StateCacheEntryImpl &operator=(StateCacheEntryImpl const &) = delete;
    StateCacheEntryImpl &operator=(StateCacheEntryImpl &&) = delete;

    //
    // Internal functions
    //

    //! \brief Notify all subscribed Lookups of a change in value.
    void notify() const
    {
      for (Lookup *lkup : m_lookups)
        lkup->valueChanged();
    }

    //! \brief Ensure the state cache entry has a CachedValue object
    //!        of the appropriate type.  If there is no cached value
    //!        or it is a placeholder VoidCachedValue, construct the
    //!        appropriate CachedValue instance and return true.
    //! \return If there is a CachedValue, and it's of a compatible
    //!         type, return true.  Otherwise return false.
    bool ensureCachedValue(ValueType typ)
    {
      if (!m_value) {
        // Didn't exist before, simply construct the desired type
        // (will be a VoidCachedValue if typ is UNKNOWN_TYPE)
        m_value.reset(CachedValueFactory(typ));
        return true;
      }

      // Check that requested type is consistent with existing
      ValueType ctyp = m_value->valueType();
      if (ctyp == typ             // same type (should be usual case)
          || typ == UNKNOWN_TYPE) // caller doesn't know or care
        return true;
      if (ctyp == UNKNOWN_TYPE) {
        // Replace placeholder with correct type
        m_value.reset(CachedValueFactory(typ));
        return true;
      }
      if (typ == INTEGER_TYPE && isNumericType(ctyp)) // can store an integer in any numeric type
        return true;
      // Date, Duration are reals
      // FIXME implement a real time type
      if (typ == REAL_TYPE && (ctyp == DATE_TYPE || ctyp == DURATION_TYPE))
        return true;

      // Type mismatch
      // FIXME this is likely a plan or interface coding error, handle more gracefully
      debugMsg("StateCacheEntry:update",
               " requested type " << valueTypeName(typ)
               << " but existing value is type " << valueTypeName(ctyp));
      return false;
    }

    //! \brief Update all change lookup thresholds registered to an
    //!        Integer valued state cache entry.
    //! \param state Const reference to the State of the lookup.
    //! \return true if any of the Lookups had change thresholds,
    //!         false otherwise.
    bool integerUpdateThresholds(State const &state)
    {
      bool hasThresholds = false;
      Integer ihi, ilo;
      Integer newihi, newilo;
      for (Lookup *l : m_lookups) {
        if (l->getThresholds(newihi, newilo)) {
          if (hasThresholds) {
            if (newilo > ilo)
              ilo = newilo;
            if (newihi < ihi)
              ihi = newihi;
          }
          else {
            hasThresholds = true;
            ilo = newilo;
            ihi = newihi;
          }
        }
      }
      unsigned int timestamp = StateCache::instance().getCycleCount();
      if (hasThresholds) {
        debugMsg("StateCacheEntry:updateThresholds",
                 ' ' << state << " resetting thresholds " << ilo << ", " << ihi);
        if (!m_lowThreshold) {
          m_lowThreshold.reset(CachedValueFactory(INTEGER_TYPE));
          m_highThreshold.reset(CachedValueFactory(INTEGER_TYPE));
        }
        m_lowThreshold->update(timestamp, ilo);
        m_highThreshold->update(timestamp, ihi);
        g_dispatcher->setThresholds(state, ihi, ilo);
      }
      else if (m_lowThreshold) {
        // Had thresholds, but they're no longer in effect
        m_lowThreshold->setUnknown(timestamp);
        m_highThreshold->setUnknown(timestamp);
        g_dispatcher->clearThresholds(state);
      }
      return hasThresholds;
    }

    //! \brief Update all change lookup thresholds registered to an
    //!        Integer valued state cache entry.
    //! \param state Const reference to the State of the lookup.
    //! \return true if any of the Lookups had change thresholds,
    //!         false otherwise.
    bool realUpdateThresholds(State const &state)
    {
      bool hasThresholds = false;
      Real rhi, rlo;
      Real newrhi, newrlo;
      for (Lookup *l : m_lookups) {
        if (l->getThresholds(newrhi, newrlo)) {
          if (hasThresholds) {
            if (newrlo > rlo)
              rlo = newrlo;
            if (newrhi < rhi)
              rhi = newrhi;
          }
          else {
            hasThresholds = true;
            rlo = newrlo;
            rhi = newrhi;
          }
        }
      }
      unsigned int timestamp = StateCache::instance().getCycleCount();
      if (hasThresholds) {
        debugMsg("StateCacheEntry:updateThresholds",
                 ' ' << state << " setting thresholds " << rlo << ", " << rhi);
        if (!m_lowThreshold) {
          m_lowThreshold.reset(CachedValueFactory(REAL_TYPE));
          m_highThreshold.reset(CachedValueFactory(REAL_TYPE));
        }
        m_lowThreshold->update(timestamp, rlo);
        m_highThreshold->update(timestamp, rhi);
        g_dispatcher->setThresholds(state, rhi, rlo);
      }
      else if (m_lowThreshold) {
        // Had thresholds, but they're no longer in effect
        m_lowThreshold->setUnknown(timestamp);
        m_highThreshold->setUnknown(timestamp);
        g_dispatcher->clearThresholds(state);
      }
      return hasThresholds;
    }

    //
    // Member variables
    //

    //! \brief All the lookups registered on this state cache entry.
    std::vector<Lookup *> m_lookups;

    //! \brief Pointer to the value cache
    CachedValuePtr m_value;

    //! \brief Pointer to the highest low threshold currently in
    //!        effect.  May be null.
    CachedValuePtr m_lowThreshold;

    //! \brief Pointer to the lowest high threshold currently in
    //!        effect.  May be null.
    CachedValuePtr m_highThreshold;
  };

  std::unique_ptr<StateCacheEntry> makeStateCacheEntry()
  {
    return std::make_unique<StateCacheEntryImpl>();
  }

}
