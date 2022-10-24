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

#include <cmath>  // fabs()
#include <memory>

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

  //! \class LookupImpl
  //! \brief Base implementation class for PLEXIL Lookup, LookupNow,
  //!        and LookupOnChange expressions.  Implements the
  //!        Expression API.
  class LookupImpl : public Lookup
  {
  protected:

    //
    // Member variables shared with derived class LookupOnChange
    //

    //! \brief The cached value of the Lookup's state.
    State m_cachedState;

    //! \brief The expression specifying the state name.
    Expression *m_stateName;

    //! \brief The parameters to the Lookup's state.
    ExprVec *m_paramVec;

    //! \brief This Lookup's entry in the state cache.
    StateCacheEntry* m_entry;

    //! \brief The declared return type of this Lookup;
    ValueType m_declaredType;

    //! \brief Whether the value of the Lookup is currently known.
    bool m_known;

    //! \brief Whether the Lookup's state is fully known.
    bool m_stateKnown;

    //! \brief Whether the Lookup's state is constant. Supports early
    //!        caching of state value.
    bool m_stateIsConstant;

    //! \brief If true, m_stateName is owned by the Lookup, and will
    //!        be deleted by the Lookup's destructor.
    bool m_stateNameIsGarbage;

    //! \brief Whether the Lookup is currently registered in the state
    //!        cache.
    bool m_isRegistered;

  public:

    //! \brief Constructor.
    //! \param stateName Pointer to a string valued Expression.
    //! \param stateNameIsGarbage If true, stateName will be deleted by the Lookup destructor.
    //! \param declaredType The expected type of the Lookup's value.
    //! \param paramVec Pointer to a vector of parameters.
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

    //! \brief Virtual destructor.
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
    // Listenable API
    //


    //! \brief Can this expression generate change notifications even
    //!        if none of its subexpressions change?
    //! \return True if the object can generate its own change
    //!         notifications, false if not.
    //! \note A Lookup's value can change independently of its
    //!       parameters.
    virtual bool isPropagationSource() const override
    {
      return true; // value may change independently of parameters
    }

    //
    // Propagator API
    //

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    //! \note Lookups must explicitly listen to their parameters,
    //!       because the lookup's value can change when the
    //!       parameters change.
    //! \note Wraps the Propagator method.
    virtual void addListener(ExpressionListener *l) override
    {
      if (!hasListeners()) {
        m_stateName->addListener(this);
        if (m_paramVec)
          m_paramVec->addListener(this);
      }
      Propagator::addListener(l);
    }

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    //! \note Wraps the Propagator method.
    virtual void removeListener(ExpressionListener *l) override
    {
      Propagator::removeListener(l);
      if (!hasListeners()) {
        if (m_paramVec)
          m_paramVec->removeListener(this);
        m_stateName->removeListener(this);
      }
    }

    //
    // Expression API
    //
    
    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override
    {
      return "LookupNow";
    }

    //! \brief Print the subexpressions of this expression to a stream.
    //! \param s Reference to the output stream.
    //! \see Expression::print
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

    //
    // Value access
    //

    virtual ValueType valueType() const override
    {
      if (m_entry && m_entry->valueType() != UNKNOWN_TYPE)
        return m_entry->valueType();
      return m_declaredType;
    }


    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    //! \note Delegated to the StateCacheEntry, if any.
    virtual bool isKnown() const override
    {
      if (!this->isActive() || !m_entry)
        return false;
      return m_entry->isKnown();
    }

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const override
    {
      if (!this->isActive() || !m_entry || !m_entry->cachedValue())
        return Value();
      return m_entry->cachedValue()->toValue();
    }

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &str) const override
    {
      str << this->toValue();
    }

    //! \brief Local macro to define Lookup::getValue() methods.
    //! \param _rtype_ The name of the method's result type.
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

    //! \brief Local macro to define Lookup::getValuePointer() methods.
    //! \param _rtype_ The name of the method's result type.
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

    //
    // API to state cache
    //

    //! \brief Notify this Lookup that its value has been updated.
    virtual void valueChanged() override
    {
      publishChange();
    }

    //! \brief Get this lookup's high and low thresholds.
    //! \param high Place to store the high threshold value.
    //! \param low Place to store the low threshold value.
    //! \return True if this lookup has active thresholds, false otherwise.
    //! \note The base class method always returns false.
    ///@{
    virtual bool getThresholds(Integer &high, Integer &low) const override
    {
      return false;
    }

    virtual bool getThresholds(Real &high, Real &low) const override
    {
      return false;
    }
    ///@}

  protected:

    //
    // Listenable API
    //

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      (oper)(m_stateName);
      if (m_paramVec)
        m_paramVec->doSubexprs(oper);
    }

    //
    // Notifier API
    // 

    //! \brief Perform any necessary actions to enter the active state.
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

    //! \brief Perform any necessary actions to enter the inactive state.
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

    //
    // Propagator API
    //

    //! \brief Perform whatever action is necessary when a change
    //!        notification is received.
    virtual void handleChange() override
    {
      debugMsg("Lookup:handleChange", ' ' << *this);
      if (handleChangeInternal())
        publishChange();
    }

    //
    // Shared behavior needed by LookupOnChange
    //

    //! \brief Get the State for this Lookup, if known.
    //! \param result The variable in which to store the result.
    //! \return True if fully known, false if not.
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
    
    //! \brief Handle a change notification.
    //! \return true if state changed, false otherwise
    bool handleChangeInternal()
    {
      debugMsg("LookupImpl:handleChangeInternal", " entered");

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

    //! \brief Ensure this lookup is registered with the state cache.
    void ensureRegistered()
    {
      if (m_isRegistered)
        return;
      debugMsg("Lookup:register", ' ' << m_cachedState);
      m_entry->registerLookup(m_cachedState, this);
      m_isRegistered = true;
    }

    //! \brief Unregister this lookup from the state cache.
    void unregister()
    {
      if (!m_isRegistered)
        return;
      debugMsg("Lookup:unregister", ' ' << m_cachedState);
      m_entry->unregisterLookup(m_cachedState, this);
      m_isRegistered = false;
    }

    //! \brief Invalidate the cached state value, by unregistering
    //! from the state cache.
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
  // I had hoped to define these classes entirely within the
  // LookupOnChange class definition, since they are only used within
  // LookupOnChange, but I was not able to define specializations of
  // member functions of a class template inside the parent class
  // definition.
  //
  // C++ sucks at information hiding.
  //
    
  //! \class ThresholdCache
  //! \brief Abstract base class for caching a LookupOnChange's
  //!        threshold values.
  class ThresholdCache
  {
  public:
    //! \brief Virtual destructor.
    virtual ~ThresholdCache() = default;

    //! \brief Check whether the tolerance value itself has changed.
    //! \param tolerance Tolerance expression.
    //! \return true if the tolerance expression's value changed,
    //!         false otherwise.
    virtual bool toleranceChanged(Expression const *tolerance) const = 0;

    //! \brief Check whether the current value is beyond the thresholds.
    //! \param entry Pointer to the value.
    //! \return True if exceeded, false otherwise.
    virtual bool thresholdsExceeded(CachedValue const *value) const = 0;

    //! \brief Set the thresholds based on the given cached value.
    //! \param value Pointer to the state cache entry.
    //! \param tolerance Pointer to the tolerance expression.
    virtual void setThresholds(CachedValue const *value, Expression const *tolerance) = 0;

    //! \brief Get the current thresholds.
    //! \param high Place to store the current high threshold.
    //! \param low Place to store the current low threshold.
    //! \note Default methods report an error.
    ///@{
    virtual void getThresholds(Integer & /* high */, Integer & /* low */) const
    {
      errorMsg("LookupOnChange:getThresholds: "
               "attempt to get Integer thresholds from non-Integer Lookup");
    }

    virtual void getThresholds(Real & /* high */, Real & /* low */) const
    {
      errorMsg("LookupOnChange:getThresholds: "
               "attempt to get Real thresholds from non-Real Lookup");
    }
    ///@}

  };

  //! \class ThresholdCacheImpl
  //! \brief Implementation class for Lookup threshold caching,
  //!        templated by value type of the threshold.
  //! \param NUM A numeric type name.
  template <typename NUM>
  class ThresholdCacheImpl : public ThresholdCache
  {
  public:

    //! \brief Default constructor.
    ThresholdCacheImpl()
      : m_wasKnown(false)
    {
    }

    //! \brief Virtual destructor.
    virtual ~ThresholdCacheImpl() = default;

    //! \brief Check whether the tolerance value itself has changed.
    //! \param tolerance Tolerance expression.
    //! \return true if the tolerance expression's value changed,
    //!         false otherwise.
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

    //! \brief Check whether the current value is beyond the thresholds.
    //! \param entry Pointer to the value cache entry.
    //! \return true if exceeded, false otherwise.
    //! \note Implementations are specialized by the template type parameter.
    virtual bool thresholdsExceeded(CachedValue const *value) const override;

    //! \brief Set the thresholds based on the given cached value.
    //! \param value Pointer to the state cache entry.
    //! \param tolerance Pointer to the tolerance expression.
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

  //! \brief Check whether the current value is beyond the thresholds
  //!        for Integer-valued Lookups.
  //! \param entry Pointer to the value cache entry.
  //! \return true if exceeded, false otherwise.
  template <>
  bool ThresholdCacheImpl<Integer>::thresholdsExceeded(CachedValue const *value) const
  {
    Integer currentValue;
    if (value->getValue(currentValue))
      return (currentValue >= m_high) || (currentValue <= m_low);
    // Current value is unknown
    return m_wasKnown; 
  }

  //! \brief Check whether the current value is beyond the thresholds
  //!        for Real-valued Lookups.
  //! \param entry Pointer to the value cache entry.
  //! \return true if exceeded, false otherwise.
  //! \note Implements comparison tolerance, to account for floating
  //!       point arithmetic accuracy issues.
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
    Real epsilon = fabs(currentValue) * 1e-13;
    if (m_high - currentValue < epsilon)
      return true;
    if (currentValue - m_low < epsilon)
      return true;
    return false;
  }

  //! \brief Construct the appropriate ThresholdCache instance for a
  //!        PLEXIL ValueType.
  //! \param typ The ValueType.
  //! \return Pointer to a ThresholdCacheImpl object.
  //! \note Inteprets UNKNOWN_TYPE as REAL_TYPE.
  //! \note Reports an error if there is no corresponding
  //!       ThresholdCacheImpl for the requested type.
  static ThresholdCache * ThresholdCacheFactory(ValueType typ)
  {
    switch (typ) {
    case INTEGER_TYPE:
      return new ThresholdCacheImpl<Integer>();

    case UNKNOWN_TYPE:
      warn("ThresholdCacheFactory: type unknown, defaulting to REAL");
      // drop thru

    case DATE_TYPE:
    case DURATION_TYPE:
    case REAL_TYPE:
      return new ThresholdCacheImpl<Real>();

    default:
      errorMsg("ThresholdCacheFactory: invalid or unimplemented type " << valueTypeName(typ));
      return nullptr;
    }
  }

  //! \class LookupOnChange
  //! \brief Represents a Lookup which ignores changes within the
  //!        provided tolerance.
  class LookupOnChange final : public LookupImpl
  {
  private:

    //
    // State local to LookupOnChange
    //

    //! \brief Threshold cache.
    std::unique_ptr<ThresholdCache> m_thresholds;

    //! \brief Pointer to the state cache entry, if any.
    std::unique_ptr<CachedValue> m_cachedValue;

    //! \brief Expression specifying the tolerance.  May be null.
    Expression *m_tolerance;

    //! \brief Whether to delete the tolerance expression when the
    //!        Lookup is deleted.
    bool m_toleranceIsGarbage;

  public:

    //! \brief Constructor.
    //! \param stateName Pointer to a string valued Expression.
    //! \param stateNameIsGarbage If true, the stateName expression will be deleted by the Lookup destructor.
    //! \param declaredType The expected type of the Lookup's value.
    //! \param tolerance Pointer to a string valued Expression.
    //! \param toleranceIsGarbage If true, the tolerance expression will be deleted by the Lookup destructor.
    //! \param paramVec Pointer to a vector of parameters.
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

    //! \brief Virtual destructor.
    virtual ~LookupOnChange()
    {
      if (m_toleranceIsGarbage)
        delete m_tolerance;
    }

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual const char *exprName() const override
    {
      return "LookupOnChange";
    }

    //
    // API to state cache
    //

    //! \brief Notify this Lookup that its value has been updated.
    //! \note May be called before lookup fully activated
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

    //! \brief Get this lookup's high and low thresholds.
    //! \param high Place to store the high threshold value.
    //! \param low Place to store the low threshold value.
    //! \return True if this lookup has active thresholds, false otherwise.
    ///@{
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
    ///@}

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    //! \note The value of a LookupOnChange may change when its
    //!       tolerance value changes.
    virtual void addListener(ExpressionListener *l) override
    {
      if (!hasListeners())
        m_tolerance->addListener(this);
      LookupImpl::addListener(l);
    }

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    virtual void removeListener(ExpressionListener *l) override
    {
      LookupImpl::removeListener(l);
      if (!hasListeners())
        m_tolerance->removeListener(this);
    }

    //! \brief Local macro for defining LookupOnChange::getValue() methods.
    //! \param _rtype_ The name of the result type.

    // Prefer locally cached value, as it may differ from state cache value.
#define DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(_rtype_)              \
    virtual bool getValue(_rtype_ &result) const override           \
    {                                                               \
      if (!this->isActive() || !m_entry || !m_entry->cachedValue()) \
        return false;                                               \
      if (m_cachedValue)                                            \
        return m_cachedValue->getValue(result);                     \
      if (m_entry->isKnown())                                       \
        return m_entry->cachedValue()->getValue(result);            \
      return false;                                                 \
    }

    // Explicit instantiations
    DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(Integer)
    DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD(Real)

#undef DEFINE_CHANGE_LOOKUP_GET_VALUE_METHOD

    //! \brief Get the value of this expression as a Value instance.
    //! \return The Value instance.
    virtual Value toValue() const override
    {
      if (!this->isActive() || !m_entry || !m_entry->cachedValue())
        return Value();
      // Use local cache, as the value cached locally may differ from
      // state cache value.
      if (m_cachedValue)
        return m_cachedValue->toValue();
      if (m_entry->isKnown())
        return m_entry->cachedValue()->toValue();
      return Value();
    }

  protected:

    //
    // Notifier API
    //

    //! \brief Perform any necessary actions to enter the active state.
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

    //! \brief Perform any necessary actions to enter the inactive state.
    virtual void handleDeactivate() override
    {
      debugMsg("LookupOnChange:handleDeactivate", " called");
      LookupImpl::handleDeactivate();
      m_tolerance->deactivate();
      // Clear m_thresholds and m_cachedValue if they are not already.
      if (m_thresholds) {
        m_thresholds.reset();
        m_cachedValue.reset();
      }
    }

    // TODO:
    //  Consider possibility lookup may not be fully activated yet.
    //  Consider possibility that tolerance has changed.

    //! \brief Perform whatever action is necessary when a change
    //!        notification is received.
    virtual void handleChange() override
    {
      debugMsg("LookupOnChange:handleChange", " called");
      if (updateInternal(LookupImpl::handleChangeInternal()))
        publishChange();
    }

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &oper) override
    {
      LookupImpl::doSubexprs(oper);
      (oper)(m_tolerance);
    }

    //! \brief Invalidate the cached state value, by unregistering
    //!       from the state cache, and resetting any internal state.
    //! \note Wrapper for base class method.
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

    //! \brief Update this LookupOnChange's internal state as appropriate.
    //! \param valueChanged Whether the state cache value has changed.
    bool updateInternal(bool valueChanged)
    {
      debugMsg("LookupOnChange:update",
               ' ' << this->m_cachedState << ", valueChanged = " << valueChanged);

      if (m_thresholds) {
        // Had the state name or parameters changed, m_thresholds
        // would have been deleted already.
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
                     << " threshold exceeded, propagating new value "
                     << val->toValue() << ", updating thresholds");
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

  };

  Expression *makeLookup(Expression *stateName,
                         bool stateNameIsGarbage,
                         ValueType declaredType,
                         ExprVec *paramVec)
  {
    return new LookupImpl(stateName, stateNameIsGarbage, declaredType, paramVec);
  }

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
