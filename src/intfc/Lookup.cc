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

#include "Lookup.hh"

#include "ArrayImpl.hh"
#include "CachedValue.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExprVec.hh"
#include "ExternalInterface.hh" // for timestamp access
#include "StateCacheEntry.hh"
#include "StateCacheMap.hh"

#include <cmath> // for abs()

namespace PLEXIL
{
  Lookup::Lookup(Expression *stateName,
                 bool stateNameIsGarbage,
                 ExprVec *paramVec)
    : m_stateName(stateName),
      m_paramVec(paramVec),
      m_entry(NULL),
      m_known(false),
      m_stateKnown(false),
      m_stateIsConstant(true),
      m_stateNameIsGarbage(stateNameIsGarbage)
  {
    assertTrue_2(stateName, "Lookup constructor: Null state name expression");
    if (!m_stateName->isConstant()) {
      m_stateName->addListener(this);
      m_stateIsConstant = false;
    }
    if (m_paramVec) {
      m_paramVec->addListener(this);
      for (size_t i = 0; i < m_paramVec->size(); ++i)
        if (!(*m_paramVec)[i]->isConstant())
          m_stateIsConstant = false;
    }
    
    // TODO: If all expressions are constants, can cache state now
  }

  Lookup::~Lookup()
  {
    if (m_entry) {
      m_entry->unregisterLookup(m_cachedState, this);
      m_entry = NULL;
    }
    if (m_paramVec) {
      m_paramVec->removeListener(this);
      delete m_paramVec;
    }
    if (!m_stateName->isConstant())
      m_stateName->removeListener(this);
    if (m_stateNameIsGarbage)
      delete m_stateName;
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

  void Lookup::printSubexpressions(std::ostream &s) const
  {
    s << " name " << *m_stateName;
    if (m_paramVec) {
      s << " params";
      for (size_t i = 0; i < m_paramVec->size(); ++i)
        s << ' ' << *(*m_paramVec)[i];
    }
    s << ' ';
  }

  void Lookup::handleActivate()
  {
    // Activate all subexpressions
    m_stateName->activate();
    if (m_paramVec)
      m_paramVec->activate();

    // Compute current state and cache it
    if (!m_stateIsConstant || !m_entry) {
      m_stateKnown = getState(m_cachedState);
      if (m_stateKnown) {
        m_entry =
          StateCacheMap::instance().ensureStateCacheEntry(m_cachedState);
        assertTrue_2(m_entry != NULL, "Lookup::handleActivate: Failed to get state cache entry");
      }
    }
    if (m_entry)
      m_entry->registerLookup(m_cachedState, this);
  }

  void Lookup::handleDeactivate()
  {
    // Dectivate all subexpressions
    m_stateName->deactivate();
    if (m_paramVec)
      m_paramVec->deactivate();

    if (m_stateKnown)
      m_entry->unregisterLookup(m_cachedState, this);

    // Preserve cache entry if state is known constant
    if (!m_stateIsConstant)
      m_entry = NULL;
  }

  // Called whenever state name or parameter changes
  void Lookup::handleChange(Expression const *src)
  {
    if (handleChangeInternal(src))
      this->publishChange(src);
  }

  // Return true if state changed, false otherwise
  bool Lookup::handleChangeInternal(Expression const *src)
  {
    State newState;
    bool oldKnown = m_stateKnown;
    m_stateKnown = getState(newState);
    bool stateChanged = (oldKnown != m_stateKnown);
    if (!m_stateKnown) {
      if (oldKnown) {
        // state used to be known, isn't any longer
        m_entry->unregisterLookup(m_cachedState, this);
        m_entry = NULL;
      }
    }
    else { // state now known
      if (oldKnown && newState != m_cachedState) {
        m_entry->unregisterLookup(m_cachedState, this);
        m_entry = NULL;
        stateChanged = true;
      }
      m_cachedState = newState;
      m_entry =
        StateCacheMap::instance().ensureStateCacheEntry(m_cachedState);
      assertTrue_2(m_entry != NULL, "Lookup::handleChange: Failed to get state cache entry");
      m_entry->registerLookup(m_cachedState, this);
    }
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
    result.setName(name);
    if (m_paramVec) {
      size_t n = m_paramVec->size();
      result.setParameterCount(n);
      for (size_t i = 0; i < n; ++i) {
        Value temp = (*m_paramVec)[i]->toValue();
        if (!temp.isKnown())
          return false;
        result.setParameter(i, temp);
      }
    }
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
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
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
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else {
      return m_entry->cachedValue()->getValue(result);
    }
  }

  bool Lookup::getValue(double &result) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else {
      return m_entry->cachedValue()->getValue(result);
    }
  }

  bool Lookup::getValue(std::string &result) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValue(result);
  }

  bool Lookup::getValuePointer(std::string const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(Array const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(BooleanArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(IntegerArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(RealArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return false;
    else
      return m_entry->cachedValue()->getValuePointer(ptr);
  }

  bool Lookup::getValuePointer(StringArray const *&ptr) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
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
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return Value();
    else
      return m_entry->cachedValue()->toValue();
  }

  // Callback from external interface
  void Lookup::valueChanged()
  {
    this->publishChange(this);
  }

  //
  // LookupOnChange
  //

  // Internal class used only by LookupOnChange
  // Presumes both lookup value and tolerance value are known.
  class ThresholdCache
  {
  public:
    ThresholdCache()
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
    virtual bool toleranceChanged(Expression const *tolerance) const = 0;

    /**
     * @brief Check whether the current value is beyond the thresholds.
     * @param entry Pointer to the value.
     * @return True if exceeded, false otherwise.
     */
    virtual bool thresholdsExceeded(CachedValue const *value) const = 0;

    /**
     * @brief Set the thresholds based on the given cached value.
     * @param entry Pointer to state cache entry.
     */
    virtual void setThresholds(CachedValue const *value, Expression const *tolerance) = 0;
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

    bool toleranceChanged(Expression const *tolerance) const
    {
      return static_cast<IMPL const *>(this)->checkToleranceImpl(tolerance);
    }

    bool thresholdsExceeded(CachedValue const *value) const
    {
      return static_cast<IMPL const *>(this)->checkImpl(value);
    }

    void setThresholds(CachedValue const *value, Expression const *tolerance)
    {
      static_cast<IMPL *>(this)->setImpl(value, tolerance);
    }

  protected:
    virtual bool checkToleranceImpl(Expression const *tolerance) const = 0;
    virtual bool checkImpl(CachedValue const *value) const = 0;
    virtual void setImpl(CachedValue const *value, Expression const *tolerance) = 0;
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

    bool checkToleranceImpl(Expression const *tolerance) const
    {
      check_error_1(tolerance); // paranoid check
      NUM newTol;
      assertTrue_2(tolerance->getValue(newTol),
                   "LookupOnChange: internal error: tolerance unknown");
      if (newTol < 0)
        newTol = -newTol;
      return newTol != m_tolerance;
    }

    // This implementation is appropriate for integers.
    // See below for floating point types.
    bool checkImpl(CachedValue const *value) const
    {
      check_error_1(value); // paranoid check
      NUM currentValue;
      assertTrue_2(value->getValue(currentValue),
                   "LookupOnChange: internal error: lookup value unknown");
      return (currentValue >= m_high) || (currentValue <= m_low);
    }

    void setImpl(CachedValue const *value, Expression const *tolerance)
    {
      debugMsg("LookupOnChange:setThresholds", " entered");
      check_error_1(value); // paranoid check
      check_error_1(tolerance); // paranoid check
      NUM base, tol;
      assertTrue_2(value->getValue(base),
                   "LookupOnChange: internal error: lookup value unknown");
      assertTrue_2(tolerance->getValue(tol),
                   "LookupOnChange: internal error: tolerance unknown");
      if (tol < 0)
        tol = -tol;
      m_tolerance = tol;
      m_low = base - tol;
      m_high = base + tol;
    }

  private:
    NUM m_low;
    NUM m_high;
    NUM m_tolerance;
  };

  // Separate check for double-valued lookups
  // Covers up a horde of sins, notably timers returning early (!)
  template <>
  bool ThresholdCacheImpl<double>::checkImpl(CachedValue const *value) const
  {
    check_error_1(value); // paranoid check
    double currentValue;
    assertTrue_2(value->getValue(currentValue),
		 "LookupOnChange: internal error: lookup value unknown");
    if ((currentValue >= m_high) || (currentValue <= m_low))
      return true;

    // Put guard bands around thresholds
    double epsilon = fabs(currentValue) * 1e-13; // on the order of 150 usec for time
    if (m_high - currentValue < epsilon)
      return true;
    if (currentValue - m_low < epsilon)
      return true;
    return false;
  }

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

  LookupOnChange::LookupOnChange(Expression *stateName,
                                 bool stateNameIsGarbage,
                                 Expression *tolerance,
                                 bool toleranceIsGarbage,
                                 ExprVec *paramVec)
    : Lookup(stateName, stateNameIsGarbage, paramVec),
      m_thresholds(NULL),
      m_cachedValue(NULL),
      m_tolerance(tolerance),
      m_toleranceIsGarbage(toleranceIsGarbage)
  {
    debugMsg("LookupOnChange", " constructor");
    // Check that tolerance is of compatible type
    assertTrue_2(tolerance,
                 "LookupOnChange constructor: no tolerance expression supplied");
    assertTrue_2(isNumericType(tolerance->valueType()),
                 "LookupOnChange constructor: tolerance expression is not numeric");
    if (!m_tolerance->isConstant())
      m_tolerance->addListener(this);
  }

  LookupOnChange::~LookupOnChange()
  {
    delete m_thresholds;
    delete m_cachedValue;
    if (!m_tolerance->isConstant())
      m_tolerance->removeListener(this);
    if (m_toleranceIsGarbage)
      delete m_tolerance;
  }

  char const *LookupOnChange::exprName() const
  {
    return "LookupOnChange";
  }

  void LookupOnChange::handleActivate()
  {
    debugMsg("LookupOnChange:handleActivate", " called");
    Lookup::handleActivate(); // may register lookup if state known,
                              // may cause calls to handleChange(), valueChanged()
    m_tolerance->activate();  // may cause calls to handleChange()
    if (updateInternal(true)) // may cause redundant notifications
      this->publishChange(this);
  }

  // TODO: Optimization opportunity if state is known to be constant
  void LookupOnChange::handleDeactivate()
  {
    debugMsg("LookupOnChange:handleDeactivate", " called");
    Lookup::handleDeactivate();
    m_tolerance->deactivate();
    delete m_thresholds;
    m_thresholds = NULL;
    delete m_cachedValue;
    m_cachedValue = NULL;
  }

  // Consider possibility that tolerance has changed.
  // Consider possibility lookup may not be fully activated yet.
  void LookupOnChange::handleChange(Expression const *src)
  {
    bool stateChanged = Lookup::handleChangeInternal(src);
    if (stateChanged && m_thresholds) {
      // State changed, thresholds & value cache are now invalid
      delete m_thresholds;
      m_thresholds = NULL;
      delete m_cachedValue;
      m_cachedValue = NULL;
    }
    if (updateInternal(false) || stateChanged)
      this->publishChange(src);
  }

  // May be called before lookup fully activated
  void LookupOnChange::valueChanged()
  {
    if (!this->isActive()) {
      debugMsg("LookupOnChange:valueChanged", " for " << m_cachedState << " not active, ignoring");
      return;
    }
    if (updateInternal(true)) {
      debugMsg("LookupOnChange:valueChanged", " for " << m_cachedState << ": notifying listeners");
      this->publishChange(this);
    }
    else {
      debugMsg("LookupOnChange:valueChanged", " for " << m_cachedState << ": no change");
    }
  }

  // Call if something has changed - could be state, tolerance, or value
  // Returns true if event should trigger notification, false otherwise.
  bool LookupOnChange::updateInternal(bool valueChanged)
  {
    debugMsg("LookupOnChange:update", " changed = " << valueChanged);
    if (this->m_entry && this->m_entry->isKnown() && m_tolerance->isKnown()) {
      CachedValue const *val = this->m_entry->cachedValue();
      // If no threshold, establish one and cache current value
      if (!m_thresholds) {
        m_thresholds = ThresholdCacheFactory(this->m_entry->valueType());
        m_cachedValue = val->clone();
        m_thresholds->setThresholds(val, m_tolerance);
        m_entry->setThresholds(m_cachedState, m_tolerance);
        return true;
      }
      else {
        // Have previous value and thresholds
        // Check whether the thresholds have changed
        if (m_thresholds->toleranceChanged(m_tolerance))
          m_thresholds->setThresholds(m_cachedValue, m_tolerance);

        // Has the (possibly updated) threshold been exceeded?
        if (m_thresholds->thresholdsExceeded(val)) {
          // TODO? Check that value hasn't changed type
          *m_cachedValue = *val;
          m_thresholds->setThresholds(val, m_tolerance);
          m_entry->setThresholds(m_cachedState, m_tolerance);
          return true;
        }
        else
          return false; // value or threshold updated, but value within tolerances
      }
    }

    // State, value, or tolerance unknown; delete cache and threshold
    if (m_thresholds) {
      delete m_thresholds;
      m_thresholds = NULL;
      delete m_cachedValue;
      m_cachedValue = NULL;
      return true; // was known, is no longer
    }
    return valueChanged;
  }

  bool LookupOnChange::getValue(int32_t &result) const
  {
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
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
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
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
    if (!this->isActive() || !m_entry || !m_entry->cachedValue())
      return Value();
    else if (m_cachedValue)
      return m_cachedValue->toValue();
    else if (m_entry->isKnown())
      return m_entry->cachedValue()->toValue();
    else
      return Value();
  }

} // namespace PLEXIL
