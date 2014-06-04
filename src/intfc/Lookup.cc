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
      m_timestamp(0),
      m_known(false),
      m_stateKnown(false),
      m_stateIsConstant(false),
      m_stateNameIsGarbage(stateNameIsGarbage)
  {
    assertTrue_2(params.size() == paramsAreGarbage.size(),
                 "Lookup constructor: Parameter vector and garbage vector differ in length");
    // TODO: check whether all expressions are constants
  }

  Lookup::~Lookup()
  {
    if (m_entry) {
      m_entry->unregisterLookup(this);
      m_entry = NULL;
    }
    for (size_t i = 0; i < m_params.size(); ++i)
      if (m_garbage[i])
        delete (Expression *) m_params[i];
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
        StateCacheMap::instance().ensureStateCacheEntry(m_cachedState, this->valueType());
      assertTrue_2(m_entry != NULL, "Lookup::handleActivate: Failed to get state cache entry");
    }
  }

  void Lookup::handleDeactivate()
  {
    // Dectivate all subexpressions
    m_stateName->deactivate();
    for (size_t i = 0; i < m_params.size(); ++i)
      m_params[i]->deactivate();

    if (m_stateKnown)
      m_entry->unregisterLookup(this);
    m_entry = NULL;
    this->makeUnknown();
  }

  void Lookup::handleChange(ExpressionId src)
  {
    State newState;
    bool stateKnown = getState(newState);
    if (!stateKnown) {
      if (m_stateKnown) {
        // state used to be known, isn't any longer
        m_entry->unregisterLookup(this);
        m_entry = NULL;
        setUnknown();
      }
      m_stateKnown = false;
    }
    else { // state now known
      if (m_stateKnown && newState != m_cachedState) {
        m_entry->unregisterLookup(this);
        m_entry = NULL;
      }
      m_stateKnown = true;
      m_cachedState = newState;
      m_entry =
        StateCacheMap::instance().ensureStateCacheEntry(m_cachedState, this->valueType());
      assertTrue_2(m_entry != NULL, "Lookup::handleChange: Failed to get state cache entry");
      m_entry->registerLookup(this); // will cause value update
    }
  }

  void Lookup::setUnknown()
  {
    bool wasKnown = m_known;
    m_known = false;
    m_timestamp = g_interface->getCycleCount();
    if (wasKnown)
      this->publishChange(this->getId());
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

  unsigned int Lookup::getTimestamp() const
  {
    return m_timestamp;
  }

  //
  // LookupImpl
  //

  template <typename T>
  LookupImpl<T>::LookupImpl(ExpressionId const &stateName,
                            bool stateNameIsGarbage,
                            std::vector<ExpressionId> const &params,
                            std::vector<bool> const &paramsAreGarbage)
    : LookupShim<LookupImpl<T> >(stateName, stateNameIsGarbage, params, paramsAreGarbage)
  {
  }

  template <typename T>
  LookupImpl<T>::~LookupImpl()
  {
  }

  template <typename T>
  bool LookupImpl<T>::getValueImpl(T &result) const
  {
    if (!this->isActive())
      return false;
    if (Lookup::m_stateKnown)
      Lookup::m_entry->checkIfStale();
    if (!Lookup::m_known)
      return false;
    result = m_value;
    return true;
  }

  template <typename T>
  bool LookupImpl<T>::getValuePointerImpl(T const *&ptr) const
  {
    if (!this->isActive())
      return false;
    if (Lookup::m_stateKnown)
      Lookup::m_entry->checkIfStale();
    if (!Lookup::m_known)
      return false;
    ptr = &m_value;
    return true;
  }

  template <typename T>
  bool LookupImpl<T>::isKnown() const
  {
    return Lookup::m_known;
  }

  template <typename T>
  void LookupImpl<T>::makeUnknown()
  {
    // Erase previous value
    m_value = T();
    Lookup::m_known = false;
  }

  template <typename T>
  void LookupImpl<T>::newValueImpl(const T &val)
  {
    if (!this->isActive())
      return;
    m_value = val;
    Lookup::m_known = true;
    Lookup::m_timestamp = g_interface->getCycleCount();
    this->publishChange(this->getId());
  }

  template <typename T>
  template <typename U>
  void LookupImpl<T>::newValueImpl(const U & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Lookup::newValue: type error");
  }

  template <>
  template <>
  void LookupImpl<double>::newValueImpl(const int32_t &val)
  {
    if (!this->isActive())
      return;
    m_value = (double) val;
    Lookup::m_timestamp = g_interface->getCycleCount();
    this->publishChange(this->getId());
  }

  //
  // LookupOnChange
  //

  // NOTE: Generic template constructor not implemented, so attempts to call it
  // can be prevented at link time.

  template <>
  LookupOnChange<int32_t>::LookupOnChange(ExpressionId const &stateName,
                                          bool stateNameIsGarbage,
                                          std::vector<ExpressionId> const &params,
                                          std::vector<bool> const &paramsAreGarbage,
                                          ExpressionId const &tolerance,
                                          bool toleranceIsGarbage)
    : LookupImpl<int32_t>(stateName, stateNameIsGarbage, params, paramsAreGarbage),
      m_tolerance(tolerance),
      m_cachedTolerance(0),
      m_toleranceIsGarbage(toleranceIsGarbage),
      m_toleranceKnown(tolerance.isId())
  {
    // Check that tolerance (if supplied) is of compatible type
    if (tolerance.isId()) {
      assertTrue_2(INTEGER_TYPE == tolerance->valueType(),
                   "LookupOnChange<Integer> constructor: tolerance expression is not Integer type");
    }
  }

  template <>
  LookupOnChange<double>::LookupOnChange(ExpressionId const &stateName,
                                         bool stateNameIsGarbage,
                                         std::vector<ExpressionId> const &params,
                                         std::vector<bool> const &paramsAreGarbage,
                                         ExpressionId const &tolerance,
                                         bool toleranceIsGarbage)
    : LookupImpl<double>(stateName, stateNameIsGarbage, params, paramsAreGarbage),
      m_tolerance(tolerance),
      m_cachedTolerance(0),
      m_toleranceIsGarbage(toleranceIsGarbage),
      m_toleranceKnown(tolerance.isId())
  {
    // Check that tolerance (if supplied) is of compatible type
    if (tolerance.isId()) {
      ValueType vtype = tolerance->valueType();
      assertTrue_2(INTEGER_TYPE == vtype || REAL_TYPE == vtype,
                   "LookupOnChange<Real> constructor: tolerance expression has non-numeric type");
    }
  }

  template <typename T>
  LookupOnChange<T>::~LookupOnChange()
  {
    if (m_toleranceIsGarbage)
      delete (Expression *) m_tolerance;
  }

  template <typename T>
  char const *LookupOnChange<T>::exprName() const
  {
    return "LookupOnChange";
  }

  template <typename T>
  void LookupOnChange<T>::handleActivate()
  {
    if (m_tolerance.isId()) {
      m_tolerance->activate();
      m_toleranceKnown = m_tolerance->getValue(m_cachedTolerance);
      if (m_toleranceKnown && m_cachedTolerance < 0)
        m_cachedTolerance = -m_cachedTolerance;
    }
    Lookup::activateInternal();
    if (Lookup::m_entry) {
      if (m_toleranceKnown && m_cachedTolerance != 0) 
        Lookup::m_entry->registerChangeLookup(static_cast<Lookup *>(this),
                                              m_cachedTolerance);
      else
        Lookup::m_entry->registerLookup(static_cast<Lookup *>(this));
    }
  }

  template <typename T>
  void LookupOnChange<T>::handleDeactivate()
  {
    Lookup::handleDeactivate();
    if (m_tolerance.isId()) {
      m_tolerance->deactivate();
      m_toleranceKnown = false;
    }
  }

  // Consider possibility that tolerance has changed. 
  template <typename T>
  void LookupOnChange<T>::handleChange(ExpressionId src)
  {
    Lookup::handleChange(src); // could change states on us!
    if (m_tolerance.isId()) {
      // Check whether tolerance changed
      T oldTolerance = m_cachedTolerance;
      bool wasKnown = m_toleranceKnown;
      m_toleranceKnown = m_tolerance->getValue(m_cachedTolerance);
      if (m_toleranceKnown) {
        // Ensure it's positive
        if (m_cachedTolerance < 0)
          m_cachedTolerance = -m_cachedTolerance;
        // Has tolerance gotten tighter?
        if (wasKnown && m_cachedTolerance < oldTolerance) {
          // Check to see if we should update
          if (this->m_entry->isStale(Lookup::m_timestamp)) {
            this->m_entry->notifyLookup(this); // will update us if newer available
          }
        }
      }
      else if (wasKnown) { // and not currently known
        // Tolerance effectively now 0, get latest available value
        if (this->m_entry->isStale(Lookup::m_timestamp))
          this->m_entry->notifyLookup(this); // will update us if newer available
      }
    }
  }

  // Return true if the value exceeds the tolerance.
  static bool checkTolerance(int32_t newVal, int32_t oldVal, int32_t tolerance)
  {
    return (tolerance <= abs(newVal - oldVal));
  }

  static bool checkTolerance(double newVal, double oldVal, double tolerance)
  {
    return (tolerance <= fabs(newVal - oldVal));
  }

  template <typename T>
  void LookupOnChange<T>::newValueImpl(const T &val)
  {
    if (!this->isActive())
      return;
    bool update = !this->m_known || m_tolerance.isNoId() || !m_toleranceKnown;
    if (this->m_known && m_tolerance.isId() && m_toleranceKnown)
      // Evaluate whether value is enough different
      update = checkTolerance(val, this->m_value, m_cachedTolerance);
    if (update)
      LookupImpl<T>::newValueImpl(val);
  }
  
  // Type conversion
  template <>
  template <>
  void LookupOnChange<double>::newValueImpl(const int32_t &val)
  {
    this->newValueImpl((double) val);
  }

  // Error case
  template <typename T>
  template <typename U>
  void LookupOnChange<T>::newValueImpl(const U & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Lookup::newValue: type error");
  }

  //
  // Explicit instantiation
  //

  template class LookupImpl<bool>;
  template class LookupImpl<int32_t>;
  template class LookupImpl<double>;
  template class LookupImpl<std::string>;
  template class LookupImpl<BooleanArray>;
  template class LookupImpl<IntegerArray>;
  template class LookupImpl<RealArray>;
  template class LookupImpl<StringArray>;

  // LookupOnChange only implemented for numeric types
  template class LookupOnChange<int32_t>;
  template class LookupOnChange<double>;

} // namespace PLEXIL
