// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
//

#include "Lookup.hh"

#include "CachedValue.hh"
#include "Debug.hh"
#include "ExprVec.hh"
#include "PlanError.hh"
#include "State.hh"
#include "StateCacheEntry.hh"
#include "StateCache.hh"
#include "ValueType.hh"

#include <memory>

#if defined(HAVE_CMATH)
#include <cmath>  // fabs()
#elif defined(HAVE_MATH_H)
#include <math.h> // fabs()
#endif

namespace PLEXIL
{

  // OPEN QUESTIONS -
  // - Registry for Lookup, Command param/return types
  //
  // FORMERLY OPEN QUESTIONS -
  // - Local cache for last value? (Yes, for now; revisit when we can profile)

  //
  // Lookup use cases
  //
  // LookupNow
  //  - external i/f queried on demand synchronously
  //  - may be active for more than one Exec cycle,
  //    so could be updated by external events
  //
  // LookupOnChange
  //  - grab from external i/f or state cache at initial activation
  //  - data updates triggered by interface
  //  - frequently active for many Exec cycles
  //

  class LookupImpl : public Lookup
  {
  protected:

    //
    // Member variables shared with derived class LookupOnChange
    //

    State m_cachedState;
    Expression *m_stateName;
    ExprVec *m_paramVec;
    StateCacheEntry* m_entry;
    ValueType m_declaredType;
    bool m_known;
    bool m_stateKnown;
    bool m_stateIsConstant; // allows early caching of state value
    bool m_stateNameIsGarbage;
    bool m_isRegistered;

  public:

    LookupImpl(Expression *stateName,
               bool stateNameIsGarbage,
               ValueType declaredType,
               ExprVec *paramVec)
      : Lookup(),
        m_stateName(stateName),
        m_paramVec(paramVec),
        m_entry(nullptr),
        m_declaredType(declaredType),
        m_known(false),
        m_stateKnown(false),
        m_stateIsConstant(true),
        m_stateNameIsGarbage(stateNameIsGarbage),
        m_isRegistered(false)
    {
      if (!m_stateName->isConstant())
        m_stateIsConstant = false;

      if (m_paramVec) {
        bool parmsAreConstant = true;
        for (size_t i = 0; i < m_paramVec->size(); ++i)
          if (!(*m_paramVec)[i]->isConstant())
            parmsAreConstant = false;
        if (!parmsAreConstant)
          m_stateIsConstant = false;
      }
    
      // If all expressions are constants, cache state now
      if (m_stateIsConstant) {
        checkPlanError(getState(m_cachedState),
                       "Error in Lookup: State is constant "
                       "but state name or some parameter is unknown");
        m_stateKnown = true;
      }
    }

    virtual ~LookupImpl()
    {
      if (m_entry) {
        unregister();
        m_entry = nullptr;
      }
      delete m_paramVec;
      if (m_stateNameIsGarbage)
        delete m_stateName;
    }

    //
    // Standard Expression API
    //
    
    virtual bool isAssignable() const override
    {
      return false;
    }
    
    virtual const char *exprName() const override
    {
      return "LookupNow";
    }

    virtual void printValue(std::ostream &str) const override
    {
      str << this->toValue();
    }

    virtual void printSubexpressions(std::ostream &str) const override
    {
      str << " name " << *m_stateName;
      if (m_paramVec) {
        str << " params";
        for (size_t i = 0; i < m_paramVec->size(); ++i)
          str << ' ' << *(*m_paramVec)[i];
      }
      str << ' ';
    }

    // Lookups must explicitly listen to their parameters,
    // because the lookup value changes when the params change.
    virtual void addListener(ExpressionListener *l) override
    {
      if (!hasListeners()) {
        m_stateName->addListener(this);
        if (m_paramVec)
          m_paramVec->addListener(this);
      }
      Notifier::addListener(l);
    }

    virtual void removeListener(ExpressionListener *l) override
    {
      Notifier::removeListener(l);
      if (!hasListeners()) {
        if (m_paramVec)
          m_paramVec->removeListener(this);
        m_stateName->removeListener(this);
      }
    }

    /**
     * @brief Query whether this expression is a source of change events.
     * @return True if the value may change independently of any subexpressions, false otherwise.
     */
    virtual bool isPropagationSource() const override
    {
      return true; // value changes independently of parameters
    }

    //
    // Value access
    //

    virtual ValueType valueType() const override
    {
      if (m_entry && m_entry->valueType() != UNKNOWN_TYPE)
        return m_entry->valueType();
      return m_declaredType;
    }

    // Delegated to the StateCacheEntry in every case

    virtual bool isKnown() const override
    {
      if (!this->isActive() || !m_entry)
        return false;
      return m_entry->isKnown();
    }

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The expression value is not copied if the return value is false.
     */

    // Local macro

#define DEFINE_LOOKUP_GET_VALUE_METHOD(_rtype_)                 \
    virtual bool getValue(_rtype_ &result) const override       \
    {                                                           \
      if (!isActive() || !m_entry || !m_entry->cachedValue())   \
        return false;                                           \
      return m_entry->cachedValue()->getValue(result);          \
    }

    DEFINE_LOOKUP_GET_VALUE_METHOD(Boolean)
    DEFINE_LOOKUP_GET_VALUE_METHOD(Integer)
    DEFINE_LOOKUP_GET_VALUE_METHOD(Real)
    DEFINE_LOOKUP_GET_VALUE_METHOD(String)

    // Uncomment if any of these are ever required
    // Falls back to Expression::getValue(_rtype_) methods
    // DEFINE_LOOKUP_GET_VALUE_METHOD(NodeState)
    // DEFINE_LOOKUP_GET_VALUE_METHOD(NodeOutcome)
    // DEFINE_LOOKUP_GET_VALUE_METHOD(FailureType)
    // DEFINE_LOOKUP_GET_VALUE_METHOD(CommandHandleValue)

#undef DEFINE_LOOKUP_GET_VALUE_METHOD

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     * @note The pointer is not copied if the return value is false.
     */

    // Local macro
#define DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(_rtype_)                 \
    virtual bool getValuePointer(_rtype_ const *&ptr) const override    \
    {                                                                   \
      if (!isActive() || !m_entry || !m_entry->cachedValue())           \
        return false;                                                   \
      return m_entry->cachedValue()->getValuePointer(ptr);              \
    }

    // Explicit instantiations
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(String)
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(Array)
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(BooleanArray)
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(IntegerArray)
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(RealArray)
    DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD(StringArray)

#undef DEFINE_LOOKUP_GET_VALUE_POINTER_METHOD

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override
    {
      if (!this->isActive() || !m_entry || !m_entry->cachedValue())
        return Value();
      return m_entry->cachedValue()->toValue();
    }

    //
    // API to state cache
    //

    //!
    // @brief Notify this Lookup that its value has been updated.
    //
    virtual void valueChanged() override
    {
      publishChange();
    }

    //!
    // @brief Get this lookup's high and low thresholds.
    // @param high Place to store the high threshold value.
    // @param low Place to store the low threshold value.
    // @return True if this lookup has active thresholds, false otherwise.
    // @note The base class method always returns false.
    //
    virtual bool getThresholds(Integer &high, Integer &low) const override
    {
      return false;
    }

    virtual bool getThresholds(Real &high, Real &low) const override
    {
      return false;
    }

  protected:

    //
    // NotifierImpl API
    // 

    virtual void handleActivate() override
    {
      debugMsg("Lookup:handleActivate", " called");
      // Activate all subexpressions
      m_stateName->activate();
      if (m_paramVec)
        m_paramVec->activate();

      // Compute current state and cache it
      if (!m_stateIsConstant)
        m_stateKnown = getState(m_cachedState);
      if (!m_entry && m_stateKnown) {
        m_entry =
          StateCache::instance().ensureStateCacheEntry(m_cachedState);
        assertTrue_2(m_entry, "Lookup::handleActivate: Failed to get state cache entry");
      }
      if (m_entry)
        ensureRegistered();
    }

    virtual void handleDeactivate() override
    {
      // Dectivate all subexpressions
      m_stateName->deactivate();
      if (m_paramVec)
        m_paramVec->deactivate();

      if (m_stateKnown)
        unregister();

      // Preserve cache entry if state is known constant
      if (!m_stateIsConstant)
        m_entry = nullptr;
    }
      
    virtual void handleChange() override
    {
      debugMsg("Lookup:handleChange", ' ' << *this);
      if (handleChangeInternal())
        publishChange();
    }

    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      (oper)(m_stateName);
      if (m_paramVec)
        m_paramVec->doSubexprs(oper);
    }

    //
    // Shared behavior needed by LookupOnChange
    //

    /**
     * @brief Get the State for this Lookup, if known.
     * @param result The variable in which to store the result.
     * @return True if fully known, false if not.
     */
    bool getState(State &result) const
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
    
    // Return true if state changed, false otherwise
    bool handleChangeInternal()
    {
      State newState;
      bool oldKnown = m_stateKnown;
      m_stateKnown = getState(newState);
      bool stateChanged = oldKnown != m_stateKnown;
      if (m_stateKnown) {
        if (oldKnown && newState != m_cachedState) {
          this->invalidateOldState();
          stateChanged = true;
        }
        m_cachedState = newState;
        m_entry =
          StateCache::instance().ensureStateCacheEntry(m_cachedState);
        assertTrue_2(m_entry, "Lookup::handleChange: Failed to get state cache entry");
        ensureRegistered();
      }
      else if (oldKnown)
        // state used to be known, isn't any longer
        this->invalidateOldState();
      return stateChanged;
    }

    // Register this lookup with the state cache (?)
    void ensureRegistered()
    {
      if (m_isRegistered)
        return;
      debugMsg("Lookup:register", ' ' << m_cachedState);
      m_entry->registerLookup(m_cachedState, this);
      m_isRegistered = true;
    }

    void unregister()
    {
      if (!m_isRegistered)
        return;
      debugMsg("Lookup:unregister", ' ' << m_cachedState);
      m_entry->unregisterLookup(m_cachedState, this);
      m_isRegistered = false;
    }

    // called before updating state to new value
    virtual void invalidateOldState()
    {
      unregister();
      m_entry = nullptr;
    }

  private:
    // Unimplemented
    LookupImpl() = delete;
    LookupImpl(LookupImpl const &) = delete;
    LookupImpl(LookupImpl &&) = delete;
    LookupImpl &operator=(LookupImpl const &) = delete;
    LookupImpl &operator=(LookupImpl &&) = delete;
  };


  //
  // LookupOnChange implementation details
  //

  // I wanted to define these classes entirely within the LookupOnChange
  // class definition, since they are only used within LookupOnChange, 
  // but I was not able to define specializations of member functions
  // of a class template inside the parent class definition.
  // C++ sucks at information hiding.

  class ThresholdCache
  {
  public:
    ThresholdCache() = default;
    virtual ~ThresholdCache() = default;

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


    /**
     * @brief Get the current thresholds.
     * @param high Place to store the current high threshold.
     * @param low Place to store the current low threshold.
     * @note Default methods.
     */

    virtual void getThresholds(Integer & /* high */, Integer & /* low */) const
    {
      errorMsg("LookupOnChange:getThresholds: "
               "attempt to get Integer thresholds from non-Integer");
    }

    virtual void getThresholds(Real & /* high */, Real & /* low */) const
    {
      errorMsg("LookupOnChange:getThresholds: "
               "attempt to get Real thresholds from non-Real");
    }

  };

  template <typename NUM>
  class ThresholdCacheImpl : public ThresholdCache
  {
  public:
    ThresholdCacheImpl()
      : ThresholdCache(),
        m_wasKnown(false)
    {
    }

    ~ThresholdCacheImpl() = default;

    virtual bool toleranceChanged(Expression const *tolerance) const override
    {
      check_error_1(tolerance); // paranoid check
      NUM newTol;
      if (tolerance->getValue(newTol)) {
        if (newTol < 0)
          newTol = -newTol;
        debugMsg("ThresholdCache:toleranceChanged",
                 " returning " << (newTol != m_tolerance));
        return newTol != m_tolerance;
      }
      // Tolerance is unknown, default to 0
      newTol = 0;
      debugMsg("ThresholdCache:toleranceChanged",
               " tolerance is unknown, returning " << true);
      return true;
    }

    // See below for implementations of this method.
    virtual bool thresholdsExceeded(CachedValue const *value) const override;

    virtual void setThresholds(CachedValue const *value, Expression const *tolerance) override
    {
      debugMsg("ThresholdCache:setThresholds", " entered");
      check_error_1(value); // paranoid check
      NUM base, tol;
      if (tolerance->getValue(tol)) {
        if (tol < 0)
          tol = -tol;
        debugMsg("ThresholdCache:setThresholds", " tolerance is " << tol);
      }
      else {
        tol = 0;
        debugMsg("ThresholdCache:setThresholds", " tolerance is unknown");
      }
      if (value->getValue(base)) {
        m_tolerance = tol;
        m_low = base - tol;
        m_high = base + tol;
        m_wasKnown = true;
        debugMsg("ThresholdCache:setThresholds",
                 " setting thresholds to [" << m_low << ", " << m_high << ']');
      }
      else {
        m_wasKnown = false;
        debugMsg("ThresholdCache:setThresholds", " lookup value is unknown");
      }
    }

    virtual void getThresholds(NUM &high, NUM &low) const override
    {
      high = m_high;
      low = m_low;
    }

  private:
    NUM m_low;
    NUM m_high;
    NUM m_tolerance;
    bool m_wasKnown;
  };

  // Threshold check for Integer-valued lookups
  template <>
  bool ThresholdCacheImpl<Integer>::thresholdsExceeded(CachedValue const *value) const
  {
    Integer currentValue;
    if (value->getValue(currentValue))
      return (currentValue >= m_high) || (currentValue <= m_low);
    // Current value is unknown
    return m_wasKnown; 
  }

  // Threshold check for Real-valued lookups
  // Covers up a horde of sins, notably timers returning early (!)
  template <>
  bool ThresholdCacheImpl<Real>::thresholdsExceeded(CachedValue const *value) const
  {
    check_error_1(value); // paranoid check
    Real currentValue;
    if (!(value->getValue(currentValue)))
      // now unknown, was it last time?
      return m_wasKnown;

    // Is known from here down
    if (!m_wasKnown)
      return true; // was unknown

    if ((currentValue >= m_high) || (currentValue <= m_low))
      return true;

    // Put guard bands around thresholds
    Real epsilon = fabs(currentValue) * 1e-13; // on the order of 150 usec for time
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
      return new ThresholdCacheImpl<Integer>();

    case UNKNOWN_TYPE:
      warn("ThresholdCacheFactory: type unknown, defaulting to REAL");
      // drop thru

      // FIXME: Implement for non-Real date, duration types
    case DATE_TYPE:
    case DURATION_TYPE:

    case REAL_TYPE:
      return new ThresholdCacheImpl<Real>();

    default:
      errorMsg("ThresholdCacheFactory: invalid or unimplemented type");
      return nullptr;
    }
  }

  //!
  // @class LookupOnChange
  // @brief Represents a Lookup which ignores changes within
  //        the provided tolerance.
  //

  class LookupOnChange final : public LookupImpl
  {
  private:

    //
    // State local to LookupOnChange
    //

    std::unique_ptr<ThresholdCache> m_thresholds;
    std::unique_ptr<CachedValue> m_cachedValue;
    Expression *m_tolerance;
    bool m_toleranceIsGarbage;

  public:

    LookupOnChange(Expression *stateName,
                   bool stateNameIsGarbage,
                   ValueType declaredType,
                   Expression *tolerance,
                   bool toleranceIsGarbage,
                   ExprVec *paramVec)
      : LookupImpl(stateName, stateNameIsGarbage, declaredType, paramVec),
        m_thresholds(),
        m_cachedValue(),
        m_tolerance(tolerance),
        m_toleranceIsGarbage(toleranceIsGarbage)
    {
    }

    virtual ~LookupOnChange()
    {
      if (m_toleranceIsGarbage)
        delete m_tolerance;
    }

    virtual const char *exprName() const override
    {
      return "LookupOnChange";
    }

    //
    // API to state cache
    //

    // May be called before lookup fully activated
    virtual void valueChanged() override
    {
      if (!this->isActive()) {
        debugMsg("LookupOnChange:valueChanged", ' ' << m_cachedState << ": not active, ignoring");
        return;
      }
      if (updateInternal(true)) {
        debugMsg("LookupOnChange:valueChanged", ' ' << m_cachedState << ": notifying listeners");
        publishChange();
      }
      else {
        debugMsg("LookupOnChange:valueChanged", ' ' << m_cachedState << ": no change");
      }
    }

    virtual bool getThresholds(Integer &high, Integer &low) const override
    {
      if (!this->isActive()) {
        debugMsg("LookupOnChange:getThresholds",
                 ' ' << m_cachedState << ": not active, returning false");
        return false;
      }
      if (!m_thresholds) {
        debugMsg("LookupOnChange:getThresholds",
                 ' ' << m_cachedState << ": no thresholds, returning false");
        return false;
      }
      assertTrueMsg(valueType() == INTEGER_TYPE,
                    "getThresholds: calling Integer method on non-Integer lookup");
      debugMsg("LookupOnChange:getThresholds",
               ' ' << m_cachedState << ": thresholds are ["
               << low << ", " << high << "], returning true");
      m_thresholds->getThresholds(high, low);
      return true;
    }

    virtual bool getThresholds(Real &high, Real &low) const override
    {
      if (!this->isActive()) {
        debugMsg("LookupOnChange:getThresholds",
                 ' ' << m_cachedState << ": not active, returning false");
        return false;
      }
      if (!m_thresholds) {
        debugMsg("LookupOnChange:getThresholds",
                 ' ' << m_cachedState << ": no thresholds, returning false");
        return false;
      }
      assertTrueMsg(valueType() == REAL_TYPE,
                    "getThresholds: calling Real method on non-Real lookup");
      m_thresholds->getThresholds(high, low);
      debugMsg("LookupOnChange:getThresholds",
               ' ' << m_cachedState << ": thresholds are ["
               << low << ", " << high << "], returning true");
      return true;
    }

    // Change lookups must explicitly listen to their tolerance,
    // because the lookup value can change when the tolerance changes.
    virtual void addListener(ExpressionListener *l) override
    {
      if (!hasListeners())
        m_tolerance->addListener(this);
      LookupImpl::addListener(l);
    }

    virtual void removeListener(ExpressionListener *l) override
    {
      LookupImpl::removeListener(l);
      if (!hasListeners())
        m_tolerance->removeListener(this);
    }

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The expression value is not copied if the return value is false.
     */

    // Local macro

    // Use local cache if we have a tolerance, as it may differ from state cache value

#define DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(_rtype_)              \
    virtual bool getValue(_rtype_ &result) const override           \
    {                                                               \
      if (!this->isActive() || !m_entry || !m_entry->cachedValue()) \
        return false;                                               \
      else if (m_cachedValue)                                       \
        return m_cachedValue->getValue(result);                     \
      else if (m_entry->isKnown())                                  \
        return m_entry->cachedValue()->getValue(result);            \
      return false;                                                 \
    }

    // Explicit instantiations
    DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(Integer)
    DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(Real)

#undef DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override
    {
      if (!this->isActive() || !m_entry || !m_entry->cachedValue())
        return Value();
      // Use local cache if we have a tolerance, as it may differ from state cache value
      else if (m_cachedValue)
        return m_cachedValue->toValue();
      else if (m_entry->isKnown())
        return m_entry->cachedValue()->toValue();
      return Value();
    }

  protected:

    //
    // NotifierImpl API
    //

    virtual void handleActivate() override
    {
      check_error_1(m_tolerance); // paranoid check
      debugMsg("LookupOnChange:handleActivate", " called");
      LookupImpl::handleActivate(); // may register lookup if state known,
      // may cause calls to handleChange(), valueChanged()
      m_tolerance->activate();  // may cause calls to handleChange()
      updateInternal(true);     // may cause redundant notifications
      if (this->isKnown())
        publishChange();
    }

    // TODO: Optimization opportunity if state is known to be constant

    // Sets m_thresholds and m_cachedValue to null if they are not already.

    virtual void handleDeactivate() override
    {
      debugMsg("LookupOnChange:handleDeactivate", " called");
      LookupImpl::handleDeactivate();
      m_tolerance->deactivate();
      if (m_thresholds) {
        m_thresholds.reset();
        m_cachedValue.reset();
      }
    }

    // Consider possibility that tolerance has changed.
    // Consider possibility lookup may not be fully activated yet.
    virtual void handleChange() override
    {
      debugMsg("LookupOnChange:handleChange", " called");
      if (updateInternal(LookupImpl::handleChangeInternal()))
        publishChange();
    }

    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      LookupImpl::doSubexprs(oper);
      (oper)(m_tolerance);
    }

    // Wrapper for base class method
    // Sets m_thresholds and m_cachedValue to null if they are not already
    virtual void invalidateOldState() override
    {
      LookupImpl::invalidateOldState();
      if (m_thresholds) {
        m_thresholds.reset();
        m_cachedValue.reset();
      }
    }

  private:
    // Prohibit default constructor, copy, assign
    LookupOnChange() = delete;
    LookupOnChange(LookupOnChange const &) = delete;
    LookupOnChange(LookupOnChange &&) = delete;
    LookupOnChange &operator=(LookupOnChange const &) = delete;
    LookupOnChange &operator=(LookupOnChange &&) = delete;

    // Called at: (1) activation (2) subexpression change (3) value change
    // Returns true if event should trigger notification, false otherwise.
    // Updated state and cache entry should be valid if state is known.
    bool updateInternal(bool valueChanged)
    {
      debugMsg("LookupOnChange:update",
               ' ' << this->m_cachedState << ", valueChanged = " << valueChanged);

      if (m_thresholds) {
        // Had the state changed, m_thresholds would have been deleted already.
        // Therefore state is unchanged.
        // m_cachedState, m_entry, and m_cachedValue are valid.
        if (m_tolerance->isKnown()) {
          // Check whether the thresholds have changed
          if (m_thresholds->toleranceChanged(m_tolerance)) {
            debugMsg("LookupOnChange:update",
                     ' ' << this->m_cachedState
                     << " tolerance changed, updating thresholds");
            m_thresholds->setThresholds(m_cachedValue.get(), m_tolerance);
          }

          // Has the (possibly updated) threshold been exceeded?
          CachedValue const *val = this->m_entry->cachedValue();
          if (m_thresholds->thresholdsExceeded(val)) {
            // TODO? Check that value hasn't changed type
            debugMsg("LookupOnChange:update",
                     ' ' << this->m_cachedState
                     << " threshold exceeded, propagating new value and updating thresholds");
            // Should be only place m_cachedValue is reassigned
            *m_cachedValue = *val;
            m_thresholds->setThresholds(val, m_tolerance);
            m_entry->updateThresholds(m_cachedState);
            return true;
          }
          debugMsg("LookupOnChange:update",
                   ' ' << this->m_cachedState << " value changed but within tolerances");
          return false; // value or threshold updated, but value within tolerances
        }
        else {
          debugMsg("LookupOnChange:update",
                   ' ' << this->m_cachedState
                   << " thresholds set but tolerance is now unknown; deleting thresholds");
          m_thresholds.reset();
          // *** Do we need to delete m_cachedValue here??? ***
          // Is case of tolerance becoming unknown independent of state/value
          // worth optimizing?
          m_cachedValue.reset();
          // Tell the cache entry about it
          m_entry->updateThresholds(m_cachedState);
          return valueChanged; // ??
        }
      }
      else if (this->m_entry && this->m_entry->isKnown() && m_tolerance->isKnown()) {
        // State, lookup value, and tolerance are all known,
        // but no thresholds have been set yet.
        // Cache current value and establish them.
        CachedValue const *val = this->m_entry->cachedValue();
        debugMsg("LookupOnChange:update",
                 ' ' << this->m_cachedState << " constructing initial threshold");
        m_thresholds.reset(ThresholdCacheFactory(this->m_entry->valueType()));
        // m_cachedValue is likely null
        if (m_cachedValue)
          *m_cachedValue = *val;
        else
          m_cachedValue.reset(val->clone()); // should be the usual case
        m_thresholds->setThresholds(val, m_tolerance);
        m_entry->updateThresholds(m_cachedState);
      }

      return valueChanged;
    }

    // Unique member data

    // Presumes both lookup value and tolerance value are known.

  };


  // Construct a Lookup expression.
  Expression *makeLookup(Expression *stateName,
                         bool stateNameIsGarbage,
                         ValueType declaredType,
                         ExprVec *paramVec)
  {
    return new LookupImpl(stateName, stateNameIsGarbage, declaredType, paramVec);
  }

  // Construct a LookupOnChange expression.
  Expression *makeLookupOnChange(Expression *stateName,
                                 bool stateNameIsGarbage,
                                 ValueType declaredType,
                                 Expression *tolerance,
                                 bool toleranceIsGarbage,
                                 ExprVec *paramVec)
  {
    return new LookupOnChange(stateName, stateNameIsGarbage, declaredType,
                              tolerance, toleranceIsGarbage, paramVec);
  }

} // namespace PLEXIL
