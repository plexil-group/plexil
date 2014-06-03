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

  // TODO:
  // - potential for optimization in the case of constant states
  void Lookup::handleActivate()
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
      m_entry->registerLookup(this);
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
      if (m_stateKnown) {
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
    : Lookup(stateName, stateNameIsGarbage, params, paramsAreGarbage)
  {
  }

  template <typename T>
  LookupImpl<T>::~LookupImpl()
  {
  }

  // FIXME: determine whether we need to do a stale check here
  template <typename T>
  bool LookupImpl<T>::getValueImpl(T &result) const
  {
    if (!isActive())
      return false;
    if (Lookup::m_stateKnown)
      Lookup::m_entry->checkIfStale();
    if (!Lookup::m_known)
      return false;
    result = m_value;
    return true;
  }

  // FIXME: determine whether we need to do a stale check here
  template <typename T>
  bool LookupImpl<T>::getValuePointerImpl(T const *&ptr) const
  {
    if (!isActive())
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
  }

  template <typename T>
  void LookupImpl<T>::newValue(const T &val)
  {
    m_value = val;
    Lookup::m_timestamp = g_interface->getCycleCount();
    this->publishChange(this->getId());
  }

  template <typename T>
  template <typename U>
  void LookupImpl<T>::newValue(const U & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Lookup::newValue: type error");
  }

  template <>
  template <>
  void LookupImpl<double>::newValue(const int32_t &val)
  {
    m_value = (double) val;
    Lookup::m_timestamp = g_interface->getCycleCount();
    this->publishChange(this->getId());
  }

  //
  // LookupOnChange
  //

  // General case: tolerance not supported
  template <typename T>
  LookupOnChange<T>::LookupOnChange(ExpressionId const &stateName,
                                    bool stateNameIsGarbage,
                                    std::vector<ExpressionId> const &params,
                                    std::vector<bool> const &paramsAreGarbage,
                                    ExpressionId const &tolerance,
                                    bool toleranceIsGarbage)
    : LookupImpl<T>(stateName, stateNameIsGarbage, params, paramsAreGarbage),
      m_tolerance(ExpressionId::noId()),
      m_toleranceIsGarbage(false)
  {
    assertTrue_2(tolerance.isNoId(), "LookupOnChange constructor: type does not implement the tolerance option");
  }

  template <>
  LookupOnChange<int32_t>::LookupOnChange(ExpressionId const &stateName,
                                          bool stateNameIsGarbage,
                                          std::vector<ExpressionId> const &params,
                                          std::vector<bool> const &paramsAreGarbage,
                                          ExpressionId const &tolerance,
                                          bool toleranceIsGarbage)
    : LookupImpl<int32_t>(stateName, stateNameIsGarbage, params, paramsAreGarbage),
      m_tolerance(tolerance),
      m_toleranceIsGarbage(toleranceIsGarbage)
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
      m_toleranceIsGarbage(toleranceIsGarbage)
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

  // General case
  template <typename T>
  void LookupOnChange<T>::handleActivate()
  {
    Lookup::handleActivate();
  }

  // Numeric cases
  template <>
  void LookupOnChange<int32_t>::handleActivate()
  {
    if (m_tolerance.isId())
      m_tolerance->activate();
    Lookup::handleActivate();
  }

  template <>
  void LookupOnChange<double>::handleActivate()
  {
    if (m_tolerance.isId())
      m_tolerance->activate();
    Lookup::handleActivate();
  }

  // General case
  template <typename T>
  void LookupOnChange<T>::handleDeactivate()
  {
    Lookup::handleDeactivate();
  }

  // Numeric cases
  template <>
  void LookupOnChange<int32_t>::handleDeactivate()
  {
    Lookup::handleDeactivate();
    if (m_tolerance.isId())
      m_tolerance->deactivate();
  }

  template <>
  void LookupOnChange<double>::handleDeactivate()
  {
    Lookup::handleDeactivate();
    if (m_tolerance.isId())
      m_tolerance->deactivate();
  }

  // General case: always notify.
  template <typename T>
  void LookupOnChange<T>::newValue(const T &val)
  {
    LookupImpl<T>::newValue(val);
  }

  // Error case
  template <typename T>
  template <typename U>
  void LookupOnChange<T>::newValue(const U & /* val */)
  {
    assertTrue_2(ALWAYS_FAIL, "Lookup::newValue: type error");
  }

  // Integer
  template <>
  void LookupOnChange<int32_t>::newValue(const int32_t &val)
  {
    if (!m_known) {
      // Just Do It(tm)
      LookupImpl<int32_t>::newValue(val);
      return;
    }

    bool update = false;
    if (m_tolerance.isId()) {
      int32_t tolval;
      // TODO: negative check
      if (m_tolerance->getValue(tolval))
        // Evaluate whether value is enough different
        update = (abs(val - m_value) >= tolval);
      else
        // Tolerance value not known, presume 0
        update = (val != m_value);
    }
    if (update)
      LookupImpl<int32_t>::newValue(val);
  }
  

  // Real
  template <>
  void LookupOnChange<double>::newValue(const double &val)
  {
    if (!m_known) {
      // Just Do It(tm)
      LookupImpl<double>::newValue(val);
      return;
    }

    bool update = false;
    if (m_tolerance.isId()) {
      double tolval;
      // TODO: negative check
      if (m_tolerance->getValue(tolval))
        // Evaluate whether value is enough different
        update = (abs(val - m_value) >= tolval);
      else
        // Tolerance value not known, presume 0
        update = (val != m_value);
    }
    if (update)
      LookupImpl<double>::newValue(val);
  }
  
  template <>
  template <>
  void LookupOnChange<double>::newValue(const int32_t &val)
  {
    m_value = (double) val;
    Lookup::m_timestamp = g_interface->getCycleCount();
    this->publishChange(this->getId());
  }

} // namespace PLEXIL
