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

#include "StateCacheEntry.hh"

#include "ArrayImpl.hh"
#include "Error.hh"
#include "ExternalInterface.hh"
#include "Lookup.hh"

namespace PLEXIL
{
  StateCacheEntry::StateCacheEntry(State const &state, ValueType vtype)
    : m_state(state),
      m_timestamp(0),
      m_valueType(vtype),
      m_cachedKnown(false)
  {
  }

  StateCacheEntry::~StateCacheEntry()
  {
  }

  ValueType StateCacheEntry::valueType() const
  {
    return m_valueType;
  }

  void StateCacheEntry::registerLookup(Lookup *l)
  {
    bool unsubscribed = !m_lookups.empty();
    m_lookups.push_back(l);
    if (unsubscribed)
      g_interface->subscribe(m_state);

    if (m_timestamp < g_interface->getCycleCount())
      // Is stale. Get current value; new lookup will get notified.
      g_interface->lookupNow(m_state, *this);
    else 
      // Value is current, tell the new lookup.
      this->notifyLookup(l); // may be redundant
  }

  void StateCacheEntry::unregisterLookup(Lookup *l)
  {
    // Most likely to remove last item first, so check for that special case.
    if (l == m_lookups.back()) {
      m_lookups.pop_back();
      if (m_lookups.empty()) {
        g_interface->unsubscribe(m_state);
      }
      return;
    }

    // Standard C++ library sucks. 
    // Should be able to do this in reverse order without gimmicks.
    // For now, KISS.
    for (std::vector<Lookup *>::iterator it = m_lookups.begin();
         it != m_lookups.end();
         ++it) {
      if (l == *it) {
        m_lookups.erase(it);
        // TODO (?): Delete self if none left??
        return;
      }
    }
    // N.B. no error if not found
  }

  void StateCacheEntry::setUnknown()
  {
    m_cachedKnown = false;
    m_timestamp = g_interface->getCycleCount();
    notifyUnknown();
  }

  void StateCacheEntry::checkIfStale()
  {
    if (m_timestamp < g_interface->getCycleCount())
      g_interface->lookupNow(m_state, *this);
  }

  void StateCacheEntry::notify() const
  {
    for (std::vector<Lookup *>::const_iterator it = m_lookups.begin();
         it != m_lookups.end();
         ++it)
      this->notifyImpl(*it);
  }

  void StateCacheEntry::notifyUnknown() const
  {
    for (std::vector<Lookup *>::const_iterator it = m_lookups.begin();
         it != m_lookups.end();
         ++it)
      (*it)->setUnknown();
  }

  StateCacheEntry *StateCacheEntry::factory(State const &state, ValueType vtype)
  {
    switch (vtype) {
    case BOOLEAN_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<bool>(state, vtype));

    case INTEGER_TYPE:
      return static_cast<StateCacheEntry *>(new NumericStateCacheEntry<int32_t>(state, vtype));

    case REAL_TYPE:
    case DATE_TYPE: // FIXME
    case DURATION_TYPE: // FIXME
      return static_cast<StateCacheEntry *>(new NumericStateCacheEntry<double>(state, vtype));

    case STRING_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<std::string>(state, vtype));

    case BOOLEAN_ARRAY_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<BooleanArray>(state, vtype));

    case INTEGER_ARRAY_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<IntegerArray>(state, vtype));

    case REAL_ARRAY_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<RealArray>(state, vtype));

    case STRING_ARRAY_TYPE:
      return static_cast<StateCacheEntry *>(new StateCacheEntryImpl<StringArray>(state, vtype));

    default:
      assertTrue_2(ALWAYS_FAIL, "StateCacheEntry::factory: Invalid or unimplemented value type");
      return NULL;
    }
  }

  //
  // Typed implementation
  //

  template <typename T>
  StateCacheEntryImpl<T>::StateCacheEntryImpl(State const &state, ValueType vtype)
    : StateCacheEntryShim<StateCacheEntryImpl<T> >(state, vtype)
  {
  }

  template <typename T>
  StateCacheEntryImpl<T>::~StateCacheEntryImpl()
  {
  }

  template <typename T>
  void StateCacheEntryImpl<T>::registerChangeLookupImpl(Lookup * /* l */,
                                                        int32_t /* tolerance */)
  {
    assertTrue_2(ALWAYS_FAIL, "LookupOnChange not implemented for this type");
  }

  template <typename T>
  void StateCacheEntryImpl<T>::registerChangeLookupImpl(Lookup * /* l */,
                                                        double /* tolerance */)
  {
    assertTrue_2(ALWAYS_FAIL, "LookupOnChange not implemented for this type");
  }

  template <typename T>
  bool StateCacheEntryImpl<T>::updateImpl(T const &val)
  {
    if (!StateCacheEntry::m_cachedKnown
        || m_cachedValue != val) {
      m_cachedValue = val;
      StateCacheEntry::m_cachedKnown = true;
      StateCacheEntry::m_timestamp = g_interface->getCycleCount();
      StateCacheEntry::notify();
    }
    return true;
  }

  // Default wrong-type method.
  template <typename T>
  template <typename U>
  bool StateCacheEntryImpl<T>::updateImpl(U const & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "StateCacheEntry::update: Type error");
    return false;
  }

  // Type conversion method.
  template <>
  template <>
  bool StateCacheEntryImpl<double>::updateImpl(int32_t const &val)
  {
    return this->updateImpl((double) val);
  }

  // From Value
  // Default case
  template <typename T>
  bool StateCacheEntryImpl<T>::updateImpl(Value const &val)
  {
    T const *valPtr;
    if (val.getValuePointer(valPtr))
      return this->updatePtrImpl(valPtr);
    else {
      StateCacheEntry::setUnknown();
      return true;
    }
  }

  // Scalar types
  template <>
  bool StateCacheEntryImpl<bool>::updateImpl(Value const &val)
  {
    bool nativeVal;
    if (val.getValue(nativeVal))
      return this->updateImpl(nativeVal);
    else {
      StateCacheEntry::setUnknown();
      return true;
    }
  }

  template <>
  bool StateCacheEntryImpl<int32_t>::updateImpl(Value const &val)
  {
    int32_t nativeVal;
    if (val.getValue(nativeVal))
      return this->updateImpl(nativeVal);
    else {
      StateCacheEntry::setUnknown();
      return true;
    }
  }

  template <>
  bool StateCacheEntryImpl<double>::updateImpl(Value const &val)
  {
    double nativeVal;
    if (val.getValue(nativeVal))
      return this->updateImpl(nativeVal);
    else {
      StateCacheEntry::setUnknown();
      return true;
    }
  }

  template <typename T>
  bool StateCacheEntryImpl<T>::updatePtrImpl(T const *ptr)
  {
    if (!StateCacheEntry::m_cachedKnown
        || m_cachedValue != *ptr) {
      m_cachedValue = *ptr;
      StateCacheEntry::m_cachedKnown = true;
      StateCacheEntry::m_timestamp = g_interface->getCycleCount();
      StateCacheEntry::notify();
    }
    return true;
  }

  // Default wrong-type method.
  template <typename T>
  template <typename U>
  bool StateCacheEntryImpl<T>::updatePtrImpl(U const * /* ptr */)
  {
    assertTrue_2(ALWAYS_FAIL, "StateCacheEntry::updatePtr: Type error");
    return false;
  }

  template <typename T>
  void StateCacheEntryImpl<T>::notifyLookup(Lookup *l) const
  {
    if (this->m_cachedKnown)
      l->newValue(m_cachedValue);
    else
      l->setUnknown();
  }

  template <typename T>
  void StateCacheEntryImpl<T>::notifyImpl(Lookup *l) const
  {
    assertTrue_2(this->m_cachedKnown, "StateCacheEntryImpl::notifyImpl called when unknown");
    l->newValue(m_cachedValue);
  }

  template <typename NUM>
  NumericStateCacheEntry<NUM>::NumericStateCacheEntry(State const &state, ValueType vtype)
    : StateCacheEntryImpl<NUM>(state, vtype)
  {
  }

  template <typename NUM>
  NumericStateCacheEntry<NUM>::~NumericStateCacheEntry()
  {
  }

  // Same type
  template <typename NUM>
  void NumericStateCacheEntry<NUM>::registerChangeLookupImpl(Lookup *l, NUM tolerance)
  {
    // Is the state already subscribed?
    bool unsubscribed = this->m_lookups.empty();
    if (unsubscribed) { 
      this->m_lookups.push_back(l);
      g_interface->subscribe(this->m_state);
    }

    if (unsubscribed || !this->m_timestamp < g_interface->getCycleCount())
      g_interface->lookupNow(this->m_state, static_cast<StateCacheEntry &>(*this));

    // Can't set thresholds if we don't have a current value
    if (this->m_cachedKnown) {
      NUM low = this->m_cachedValue - tolerance;
      NUM high = this->m_cachedValue + tolerance;

      if (m_changeLookups.empty()) {
        // No previous active change lookups
        m_lowThreshold = low;
        m_highThreshold = high;
        g_interface->setThresholds(this->m_state,
                                   m_lowThreshold, 
                                   m_highThreshold);
        m_tolerance = tolerance;
      }
      else {
        // Thresholds already active - narrow them
        bool changed = false;
        if (low > m_lowThreshold) {
          m_lowThreshold = low;
          changed = true;
        }
        if (high < m_highThreshold) {
          m_highThreshold = high;
          changed = true;
        }
        g_interface->setThresholds(this->m_state,
                                   m_lowThreshold, 
                                   m_highThreshold);
        if (tolerance < m_tolerance)
          m_tolerance = tolerance;
      }
    }
  }

  // Type mismatch
  template <typename NUM>
  template <typename CVT>
  void NumericStateCacheEntry<NUM>::registerChangeLookupImpl(Lookup * /* l */, CVT /* tolerance */)
  {
    assertTrue_2(ALWAYS_FAIL, "LookupOnChange: Invalid or unsupported tolerance type for this state")
  }

  // Valid conversions
  template <>
  template <>
  void NumericStateCacheEntry<double>::registerChangeLookupImpl(Lookup *l,  int32_t tolerance)
  {
    this->registerChangeLookupImpl(l, (double) tolerance);
  }

  template <typename NUM>
  void NumericStateCacheEntry<NUM>::unregisterLookup(Lookup *l)
  {
    if (!m_changeLookups.empty()) {
      // FIXME: tail-first search would likely be more optimal
      for (std::vector<Lookup *>::iterator it = m_changeLookups.begin();
           it != m_changeLookups.end();
           ++it) {
        if (l == *it) {
          m_changeLookups.erase(it);
          break;
        }
      }
    }
    StateCacheEntry::unregisterLookup(l);
  }

  template <typename NUM>
  bool NumericStateCacheEntry<NUM>::updateImpl(NUM const &val)
  {
    if (!m_changeLookups.empty()) {
      // Reset thresholds if appropriate
      if (!this->m_cachedKnown || val > m_highThreshold || val < m_lowThreshold) {
        m_lowThreshold = val - m_tolerance;
        m_highThreshold = val + m_tolerance;
        g_interface->setThresholds(this->m_state,
                                   m_lowThreshold,
                                   m_highThreshold);
      }
    }
    // Go on to notify
    return StateCacheEntryImpl<NUM>::updateImpl(val);
  }

  //
  // Explicit instantiation (possibly redundant with factory above)
  //

  template class StateCacheEntryImpl<bool>;
  template class StateCacheEntryImpl<std::string>;
  template class StateCacheEntryImpl<BooleanArray>;
  template class StateCacheEntryImpl<IntegerArray>;
  template class StateCacheEntryImpl<RealArray>;
  template class StateCacheEntryImpl<StringArray>;

  template class NumericStateCacheEntry<int32_t>;
  template class NumericStateCacheEntry<double>;

}
