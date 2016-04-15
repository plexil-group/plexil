/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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
#include "CachedValue.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExternalInterface.hh"
#include "Lookup.hh"
#include "State.hh"

#include <algorithm> // std::find

namespace PLEXIL
{
  StateCacheEntry::StateCacheEntry()
    : m_value(NULL)
  {
  }

  // Copy constructor, used ONLY by StateCacheMap
  // Will throw an exception if called with an entry which has a value or lookups
  StateCacheEntry::StateCacheEntry(StateCacheEntry const &orig)
    : m_value(NULL)
  {
    assertTrue_1(!orig.m_value && orig.m_lookups.empty());
  }

  StateCacheEntry::~StateCacheEntry()
  {
    delete m_value;
  }

  ValueType const StateCacheEntry::valueType() const
  {
    if (m_value)
      return m_value->valueType();
    else
      return UNKNOWN_TYPE;
  }

  bool StateCacheEntry::isKnown() const
  {
    if (m_value)
      return m_value->isKnown();
    else
      return false;
  }

  void StateCacheEntry::registerLookup(State const &s, Lookup *l)
  {
    bool unsubscribed = m_lookups.empty();
    m_lookups.push_back(l);
    if (unsubscribed)
      g_interface->subscribe(s);
    // Update if stale
    if ((!m_value) || m_value->getTimestamp() < g_interface->getCycleCount())
      g_interface->lookupNow(s, *this);
  }

  void StateCacheEntry::unregisterLookup(State const &s, Lookup *l)
  {
    if (m_lookups.empty())
      return; // can't possibly be registered

    // Somewhat likely to remove last item first, so check for that special case.
    // TODO: analyze to see if this is true!
    if (l == m_lookups.back())
      m_lookups.pop_back();
    else {
      std::vector<Lookup *>::iterator it =
        std::find(m_lookups.begin(), m_lookups.end(), l);
      if (it != m_lookups.end())
        m_lookups.erase(it);
      else
        return; // not found
    }

    if (m_lookups.empty())
      g_interface->unsubscribe(s);
  }

  void StateCacheEntry::setThresholds(State const &s, Expression const *tolerance)
  {
    // Check for valid tolerance
    if (!tolerance->isKnown()) {
      debugMsg("StateCacheEntry:setThresholds", " tolerance value unknown, ignoring");
      return;
    }
    ValueType ttype = tolerance->valueType();
    assertTrueMsg(isNumericType(ttype),
                  "LookupOnChange with invalid tolerance type "
                  << valueTypeName(tolerance->valueType()));

    // Can only set thresholds if we know the current value.
    if (!m_value->isKnown()) {
      debugMsg("StateCacheEntry:setThresholds", " lookup value unknown, ignoring");
      return;
    }

    ValueType vtype = m_value->valueType();
    assertTrueMsg(isNumericType(vtype),
                  "LookupOnChange: lookup value of type " << valueTypeName(vtype)
                  << " does not allow a tolerance");
    switch (ttype) {
    case INTEGER_TYPE: {
      int32_t itol;
      tolerance->getValue(itol); // known to be known
      if (itol == 0)
        return;
      if (itol < 0)
        itol = -itol;
      if (vtype == INTEGER_TYPE) {
        int32_t curr;
        m_value->getValue(curr); // known to be known
        g_interface->setThresholds(s, curr + itol, curr - itol);
      }
      // FIXME: add support for non-double date/duration types
      else {
        double rtol = (double) itol;
        double rcurr;
        m_value->getValue(rcurr); // known to be known
        g_interface->setThresholds(s, rcurr + rtol, rcurr - rtol);
      }
      break;
    }

      // FIXME: support non-double date/duration types
    case DATE_TYPE:
    case DURATION_TYPE:

    case REAL_TYPE: {
      // FIXME later if this proves to be a problem
      assertTrue_2(vtype != INTEGER_TYPE,
                   "LookupOnChange: integer state values must have integer tolerances");
      double rtol;
      tolerance->getValue(rtol);
      if (rtol == 0)
        return;
      if (rtol < 0)
        rtol = -rtol;
      double rcurr;
      m_value->getValue(rcurr); // known to be known
      g_interface->setThresholds(s, rcurr + rtol, rcurr - rtol);
      break;
    }

    default:
      assertTrueMsg(ALWAYS_FAIL,
                    "LookupOnChange internal error: invalid/unimplemented tolerance type "
                    << valueTypeName(ttype));
      break;
    }
  }

  CachedValue const *StateCacheEntry::cachedValue() const
  {
    return m_value;
  }

  bool StateCacheEntry::ensureCachedValue(ValueType v)
  {
    if (m_value) {
      // Check that requested type is consistent with existing
      ValueType ct = m_value->valueType();
      if (ct == v) // usual case, we hope
        return true;
      if (v == UNKNOWN_TYPE) // caller doesn't know or care
        return true;
      if (ct == UNKNOWN_TYPE) {
        // Replace placeholder with correct type
        delete m_value;
        m_value = CachedValueFactory(v);
        return true;
      }
      if (v == INTEGER_TYPE && isNumericType(ct)) // can store an integer in any numeric type
        return true;
      // FIXME implement a real time type
      if (v == REAL_TYPE && (ct == DATE_TYPE || ct == DURATION_TYPE)) // date, duration are real
        return true;

      // Type mismatch
      // FIXME this is likely a plan or interface coding error, handle more gracefully
      debugMsg("StateCacheEntry:update",
               " requested type " << valueTypeName(v)
               << " but existing value is type " << valueTypeName(ct));
      return false;
    }
    else {
      m_value = CachedValueFactory(v);
      return true;
    }
  }

  void StateCacheEntry::setUnknown()
  {
    if (m_value) {
      if (m_value->setUnknown(g_interface->getCycleCount()))
        notify();
    }
  }

  void StateCacheEntry::update(bool const &val)
  {
    if (!ensureCachedValue(BOOLEAN_TYPE))
      return;
    if (m_value->update(g_interface->getCycleCount(), val))
      notify();
  }

  void StateCacheEntry::update(int32_t const &val)
  {
    if (!ensureCachedValue(INTEGER_TYPE))
      return;
    if (m_value->update(g_interface->getCycleCount(), val))
      notify();
  }

  void StateCacheEntry::update(double const &val)
  {
    if (!ensureCachedValue(REAL_TYPE))
      return;
    if (m_value->update(g_interface->getCycleCount(), val))
      notify();
  }

  void StateCacheEntry::update(std::string const &val)
  {
    if (!ensureCachedValue(STRING_TYPE))
      return;
    if (m_value->update(g_interface->getCycleCount(), val))
      notify();
  }

  void StateCacheEntry::update(Value const &val)
  {
    if (!ensureCachedValue(val.valueType()))
      return;
    if (m_value->update(g_interface->getCycleCount(), val))
      notify();
  }

  void StateCacheEntry::updatePtr(std::string const *valPtr)
  {
    if (!ensureCachedValue(STRING_TYPE))
      return;
    if (m_value->updatePtr(g_interface->getCycleCount(), valPtr))
      notify();
  }

  void StateCacheEntry::updatePtr(BooleanArray const *valPtr)
  {
    if (!ensureCachedValue(BOOLEAN_ARRAY_TYPE))
      return;
    if (m_value->updatePtr(g_interface->getCycleCount(), valPtr))
      notify();
  }

  void StateCacheEntry::updatePtr(IntegerArray const *valPtr)
  {
    if (!ensureCachedValue(INTEGER_ARRAY_TYPE))
      return;
    if (m_value->updatePtr(g_interface->getCycleCount(), valPtr))
      notify();
  }

  void StateCacheEntry::updatePtr(RealArray const *valPtr)
  {
    if (!ensureCachedValue(REAL_ARRAY_TYPE))
      return;
    if (m_value->updatePtr(g_interface->getCycleCount(), valPtr))
      notify();
  }

  void StateCacheEntry::updatePtr(StringArray const *valPtr)
  {
    if (!ensureCachedValue(STRING_ARRAY_TYPE))
      return;
    if (m_value->updatePtr(g_interface->getCycleCount(), valPtr))
      notify();
  }

  void StateCacheEntry::notify() const
  {
    for (std::vector<Lookup *>::const_iterator it = m_lookups.begin();
         it != m_lookups.end();
         ++it)
      (*it)->valueChanged();
  }

}
