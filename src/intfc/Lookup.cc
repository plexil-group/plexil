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

#include "Lookup.hh"

#include "ArrayImpl.hh"
#include "CachedValue.hh"
#include "Error.hh"
#include "ExternalInterface.hh" // for timestamp access
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"

#include <cmath> // for abs()

namespace PLEXIL
{
  Lookup::Lookup(ExpressionId const &stateName,
                 bool stateNameIsGarbage,
                 std::vector<ExpressionId> const &params,
                 std::vector<bool> const &paramsAreGarbage)
    : m_params(params),
      m_garbage(paramsAreGarbage),
      m_stateName(stateName),
      m_entry(NULL),
      m_known(false),
      m_stateKnown(false),
      m_stateIsConstant(false),
      m_stateNameIsGarbage(stateNameIsGarbage)
  {
    assertTrue_2(stateName.isId(), "Lookup constructor: Null state name expression");
    if (!m_stateName->isConstant())
      m_stateName->addListener(getId());
    assertTrue_2(params.size() == paramsAreGarbage.size(),
                 "Lookup constructor: Parameter vector and garbage vector differ in length");
    for (size_t i = 0; i < params.size(); ++i) {
      if (!m_params[i]->isConstant())
        m_params[i]->addListener(getId());
    }
    // TODO: If all expressions are constants, can cache state now
  }

  Lookup::~Lookup()
  {
    if (m_entry) {
      m_entry->unregisterLookup(this);
      m_entry = NULL;
    }
    for (size_t i = 0; i < m_params.size(); ++i) {
      if (!m_params[i]->isConstant())
        m_params[i]->removeListener(getId());
      if (m_garbage[i])
        delete (Expression *) m_params[i];
    }
    if (!m_stateName->isConstant())
      m_stateName->removeListener(getId());
    if (m_stateNameIsGarbage)
      delete (Expression *) m_stateName;
  }

  bool Lookup::isAssignable() const
  {
    return false;
  }

  bool Lookup::isConstant() const
  {
    return false;
  }

  char const *Lookup::exprName() const
  {
    return "LookupNow";
  }

  // punt
  void Lookup::printValue(std::ostream &s) const
  {
    s << this->toValue();
  }

  void Lookup::handleActivate()
  {
    activateInternal();
    if (m_entry)
      m_entry->registerLookup(this);
  }

  // TODO:
  // - potential for optimization in the case of constant states
  void Lookup::activateInternal()
  {
    // Activate all subexpressions
    m_stateName->activate();
    for (size_t i = 0; i < m_params.size(); ++i)
      m_params[i]->activate();

    // Compute current state and cache it
    m_stateKnown = getState(m_cachedState);
    if (m_stateKnown) {
      m_entry =
        StateCacheMap::instance().ensureStateCacheEntry(m_cachedState);
      assertTrue_2(m_entry != NULL, "Lookup::handleActivate: Failed to get state cache entry");
    }
  }

  // TODO:
  // - potential for optimization in the case of constant states
  void Lookup::handleDeactivate()
  {
    // Dectivate all subexpressions
    m_stateName->deactivate();
    for (size_t i = 0; i < m_params.size(); ++i)
      m_params[i]->deactivate();

    if (m_stateKnown)
      m_entry->unregisterLookup(this);
    m_entry = NULL;
  }

  void Lookup::handleChange(ExpressionId src)
  {
    handleChangeInternal(src);
  }

  bool Lookup::handleChangeInternal(ExpressionId src)
  {
    State newState;
    bool oldKnown = m_stateKnown;
    m_stateKnown = getState(newState);
    bool stateChanged = (oldKnown != m_stateKnown);
    if (!m_stateKnown) {
      if (oldKnown) {
        // state used to be known, isn't any longer
        m_entry->unregisterLookup(this);
        m_entry = NULL;
      }
    }
    else { // state now known
      if (oldKnown && newState != m_cachedState) {
        m_entry->unregisterLookup(this);
        m_entry = NULL;
        stateChanged = true;
      }
      m_cachedState = newState;
      m_entry =
        StateCacheMap::instance().ensureStateCacheEntry(m_cachedState);
      assertTrue_2(m_entry != NULL, "Lookup::handleChange: Failed to get state cache entry");
      m_entry->registerLookup(this);
    }
    if (stateChanged)
      this->publishChange(src);
    return stateChanged;
  }

  const ValueType Lookup::valueType() const
  {
    if (!m_entry)
      return UNKNOWN_TYPE;
    else
      return m_entry->valueType();
  }

  bool Lookup::getState(State &result) const
  {
    std::string name;
    if (!m_stateName->getValue(name))
      return false;
    std::vector<Value> args(m_params.size());
    for (size_t i = 0; i < m_params.size(); ++i)
      if (!(args[i] = m_params[i]->toValue()).isKnown())
        return false;
    result = State(name, args);
    return true;
  }

  bool Lookup::isKnown() const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->isKnown();
  }

  bool Lookup::getValue(bool &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValue(result);
  }

  // Not implemented for this set of internal types
  bool Lookup::getValue(uint16_t & /*result */) const
  {
    assertTrue_2(ALWAYS_FAIL, "Lookup::getValue: type error");
    return false;
  }

  bool Lookup::getValue(int32_t &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else {
      return m_entry->cachedValue()->getValue(result);
    }
  }

  bool Lookup::getValue(double &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else {
      return m_entry->cachedValue()->getValue(result);
    }
  }

  bool Lookup::getValue(std::string &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValue(result);
  }

  bool Lookup::getValuePointer(std::string const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(Array const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(RealArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(StringArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  /**
   * @brief Get the value of this expression as a Value instance.
   * @return The Value instance.
   */
  Value Lookup::toValue() const
  {
    if (!this->isActive() || !m_entry)
      return Value();
    else
      return m_entry->cachedValue()->toValue();
  }

  // Callback from external interface
  void Lookup::valueChanged()
  {
    this->publishChange(this->getId());
  }

  //
  // LookupOnChange
  //

  // Internal class used only by LookupOnChange
  class ThresholdCache
  {
  public:
    ThresholdCache()
      : m_known(false),
        m_toleranceKnown(false)
    {
    }

    virtual ~ThresholdCache()
    {
    }

    /**
     * @brief Check whether the threshold value itself has changed.
     * @param tolerance Tolerance expression.
     * @return True if changed.
     */
    bool thresholdChanged(ExpressionId tolerance)
    {
      // Always trigger on known-to-unknown or unknown-to-known transition
      bool oldKnown = m_toleranceKnown;
      if ((m_toleranceKnown = tolerance->isKnown()) != oldKnown)
        return true;
      if (m_toleranceKnown)
        return checkTolerance(tolerance);
      return false;
    }

    /**
     * @brief Check whether the current value in the state cache is beyond the thresholds.
     * @param entry Pointer to state cache entry.
     * @return True if exceeded, false otherwise.
     */
    bool thresholdsExceeded(StateCacheEntry const *entry)
    {
      // Always trigger on known-to-unknown or unknown-to-known transition
      bool oldKnown = m_known;
      if ((m_known = entry->isKnown()) != oldKnown)
        return true;
      if (!m_toleranceKnown)
        return true; // always
      return check(entry);
    }

    /**
     * @brief Set the thresholds based on the current value in the state cache.
     * @param entry Pointer to state cache entry.
     */
    void setThresholds(StateCacheEntry const *entry, ExpressionId tolerance)
    {
      if (!tolerance->isKnown()) {
        m_toleranceKnown = false;
        return;
      }
      if ((m_known = entry->isKnown()))
        this->set(entry, tolerance);
    }

    // Reset to unknown so next update causes notification.
    void reset()
    {
      m_known = false;
    }

  protected:
    // Delegated to implementation classes
    virtual bool checkTolerance(ExpressionId tolerance) = 0;
    virtual bool check(StateCacheEntry const *entry) const = 0;
    virtual void set(StateCacheEntry const *entry, ExpressionId tolerance) = 0;

    bool m_known; // Whether the value and thresholds were known at last check.
    bool m_toleranceKnown; // Whether tolerance was known at last check.
  };

  template <typename IMPL>
  class ThresholdCacheShim : public ThresholdCache
  {
  public:
    ThresholdCacheShim()
      : ThresholdCache()
    {
    }

    virtual ~ThresholdCacheShim()
    {
    }

    bool checkTolerance(ExpressionId tolerance)
    {
      return static_cast<IMPL *>(this)->checkToleranceImpl(tolerance);
    }

    bool check(StateCacheEntry const *entry) const
    {
      return static_cast<IMPL const *>(this)->checkImpl(entry);
    }

    void set(StateCacheEntry const *entry, ExpressionId tolerance)
    {
      m_known = entry->isKnown();
      m_toleranceKnown = tolerance->isKnown();
      if (m_known && m_toleranceKnown)
        static_cast<IMPL *>(this)->setImpl(entry, tolerance);
    }

  protected:
    virtual bool checkToleranceImpl(ExpressionId tolerance) = 0;
    virtual bool checkImpl(StateCacheEntry const *entry) const = 0;
    virtual void setImpl(StateCacheEntry const *entry, ExpressionId tolerance) = 0;
  };

  template <typename NUM>
  class ThresholdCacheImpl : public ThresholdCacheShim<ThresholdCacheImpl<NUM> >
  {
  public:
    ThresholdCacheImpl()
      : ThresholdCacheShim<ThresholdCacheImpl<NUM> >()
    {
    }

    ~ThresholdCacheImpl()
    {
    }

    // Only called if tolerance is and was known.
    // Returns true if tolerance changed; also resets thresholds in that case.
    bool checkToleranceImpl(ExpressionId tolerance)
    {
      NUM newTol;
      tolerance->getValue(newTol);
      if (newTol < 0)
        newTol = -newTol;
      if (newTol == m_tolerance)
        return false;
      // Calculate new thresholds based on last value
      m_high = m_high - m_tolerance + newTol;
      m_low = m_low + m_tolerance - newTol;
      m_tolerance = newTol;
      return true;
    }

    // Only called if was known and is known now.
    bool checkImpl(StateCacheEntry const *entry) const
    {
      NUM currentValue;
      entry->cachedValue()->getValue(currentValue);
      return (currentValue >= m_high) || (currentValue <= m_low);
    }

    // Only called if both current value and tolerance known.
    void setImpl(StateCacheEntry const *entry, ExpressionId tolerance)
    {
      NUM curr, tol;
      entry->cachedValue()->getValue(curr);
      tolerance->getValue(tol);
      if (tol < 0)
        tol = -tol;
      m_tolerance = tol;
      m_low = curr - tol;
      m_high = curr + tol;
    }

  private:
    NUM m_low;
    NUM m_high;
    NUM m_tolerance;
  };

  static ThresholdCache * ThresholdCacheFactory(ValueType typ)
  {
    switch (typ) {
    case INTEGER_TYPE:
      return new ThresholdCacheImpl<int32_t>();

    case UNKNOWN_TYPE:
      warn("ThresholdCacheFactory: type unknown, defaulting to REAL");
      // drop thru

      // FIXME: Implement for non-double date, duration types
    case DATE_TYPE:
    case DURATION_TYPE:

    case REAL_TYPE:
      return new ThresholdCacheImpl<double>();

    default:
      assertTrue_2(ALWAYS_FAIL, "ThresholdCacheFactory: invalid or unimplemented type");
      return NULL;
    }
  }

  LookupOnChange::LookupOnChange(ExpressionId const &stateName,
                                 bool stateNameIsGarbage,
                                 std::vector<ExpressionId> const &params,
                                 std::vector<bool> const &paramsAreGarbage,
                                 ExpressionId const &tolerance,
                                 bool toleranceIsGarbage)
    : Lookup(stateName, stateNameIsGarbage, params, paramsAreGarbage),
      m_thresholds(NULL),
      m_cachedValue(NULL),
      m_tolerance(tolerance),
      m_toleranceIsGarbage(toleranceIsGarbage)
  {
    // Check that tolerance is of compatible type
    assertTrue_2(tolerance.isId(),
                 "LookupOnChange constructor: no tolerance expression supplied");
    assertTrue_2(isNumericType(tolerance->valueType()),
                 "LookupOnChange constructor: tolerance expression is not numeric");
    if (!m_tolerance->isConstant())
      m_tolerance->addListener(getId());
  }

  LookupOnChange::~LookupOnChange()
  {
    delete m_thresholds;
    delete m_cachedValue;
    if (!m_tolerance->isConstant())
      m_tolerance->removeListener(getId());
    if (m_toleranceIsGarbage)
      delete (Expression *) m_tolerance;
  }

  char const *LookupOnChange::exprName() const
  {
    return "LookupOnChange";
  }

  void LookupOnChange::handleActivate()
  {
    this->activateInternal();
    m_tolerance->activate();
    if (this->m_entry) {
      // State is known
      if (m_tolerance->isKnown())
        this->m_entry->registerChangeLookup(static_cast<Lookup *>(this),
                                            m_tolerance);
      else
        this->m_entry->registerLookup(static_cast<Lookup *>(this));
      // Value should have been looked up, but may still be unknown
      if (!m_thresholds)
        m_thresholds = ThresholdCacheFactory(this->m_entry->valueType());
      // Initialize or update value cache
      updateInternal();
    } // end state is known
  }

  void LookupOnChange::handleDeactivate()
  {
    Lookup::handleDeactivate();
    m_tolerance->deactivate();
    if (m_thresholds)
      m_thresholds->reset();
  }

  // Consider possibility that tolerance has changed.
  // Consider possibility lookup may not be fully activated yet.
  void LookupOnChange::handleChange(ExpressionId src)
  {
    if (Lookup::handleChangeInternal(src)) {
      // State changed
      if (m_thresholds)
        m_thresholds->reset();
    }
    if (m_thresholds) {
      if (m_thresholds->thresholdChanged(m_tolerance)) {
        // Tolerance changed, resubscribe if necessary and check thresholds 
        if (m_tolerance->isKnown())
          this->m_entry->registerChangeLookup(static_cast<Lookup *>(this), 
                                              m_tolerance);
        // THIS IS MISLEADING - value of lookup may not have changed
        if (m_thresholds->thresholdsExceeded(this->m_entry)) {
          updateInternal();
          this->publishChange(src);
        }
      }
    }
    else if (m_tolerance->isKnown()
             && this->m_stateKnown
             && this->m_entry->isKnown()) {
      // Tolerance became known, and we have a value for state,
      // so set thresholds
      m_thresholds = ThresholdCacheFactory(this->m_entry->valueType());
      m_thresholds->setThresholds(this->m_entry, m_tolerance);
      this->m_entry->registerChangeLookup(static_cast<Lookup *>(this), 
                                          m_tolerance);
    }
  }

  void LookupOnChange::valueChanged()
  {
    bool publish = true; // publish by default
    if (m_thresholds) {
      publish = m_thresholds->thresholdsExceeded(this->m_entry);
      if (publish)
        updateInternal();
    }
    if (publish)
      this->publishChange(this->getId());
  }

  void LookupOnChange::updateInternal()
  {
    if (!this->m_entry) {
      // *** TEMP DEBUG ***
      warn("LookupOnChange::updateInternal: no state cache entry");
      return;
    }
    CachedValue const *val = this->m_entry->cachedValue();
    if (val) {
      if (m_cachedValue) {
        if (*m_cachedValue == *val)
          return; // value unchanged
        if (m_cachedValue->valueType() == val->valueType())
          *m_cachedValue = *val;
        else {
          // Type changed (possibly from UNKNOWN_TYPE)

          // *** TEMP DEBUG ***
          warn("Lookup value type changed from " << valueTypeName(m_cachedValue->valueType())
               << " to " << valueTypeName(val->valueType()));

          delete m_cachedValue;
          m_cachedValue = val->clone();
        }
      }
      else
        m_cachedValue = val->clone();
    }
    // Value changed, set thresholds
    m_thresholds->setThresholds(this->m_entry, m_tolerance);
    if (m_tolerance->isKnown())
      this->m_entry->registerChangeLookup(static_cast<Lookup *>(this), 
                                          m_tolerance);
  }

  bool LookupOnChange::getValue(int32_t &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else if (m_cachedValue)
      return m_cachedValue->getValue(result);
    else if (m_entry->isKnown())
      return m_entry->cachedValue()->getValue(result);
    else
      return false;
  }

  bool LookupOnChange::getValue(double &result) const
  {
    if (!this->isActive() || !m_entry)
      return false;
    else if (m_cachedValue)
      return m_cachedValue->getValue(result);
    else if (m_entry->isKnown())
      return m_entry->cachedValue()->getValue(result);
    else
      return false;
  }

  /**
   * @brief Get the value of this expression as a Value instance.
   * @return The Value instance.
   */
  Value LookupOnChange::toValue() const
  {
    if (!this->isActive() || !m_entry)
      return Value();
    else if (m_cachedValue)
      return m_cachedValue->toValue();
    else if (m_entry->isKnown())
      return m_entry->cachedValue()->toValue();
    else
      return Value();
  }

} // namespace PLEXIL
