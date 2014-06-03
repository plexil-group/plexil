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

#ifndef PLEXIL_LOOKUP_HH
#define PLEXIL_LOOKUP_HH

#include "ExpressionImpl.hh"
#include "NotifierImpl.hh"
#include "State.hh"

namespace PLEXIL
{

  // Forward references
  class StateCacheEntry;

  // OPEN QUESTIONS -
  // - Registry for Lookup, Command param/return types
  //
  // FORMERLY OPEN QUESTIONS -
  // - Local cache for last value? (Yes, for now; revisit when we can profile)
  // - Access to Exec "globals":
  //  StateCacheMap instance - via singleton pattern
  //  cycle count (timestamp) - via g_exec global var

  //
  // Lookup use cases
  //
  // LookupNow
  //  - external i/f queried on demand synchronously
  //  - may be active for more than one Exec cycle, so updates possible
  //
  // LookupOnChange
  //  - grab from external i/f or state cache at initial activation
  //  - data updates via supply push
  //  - frequently active for many Exec cycles
  //

  class Lookup : public NotifierImpl
  {
  public:
    Lookup(ExpressionId const &stateName,
           bool stateNameIsGarbage,
           std::vector<ExpressionId> const &params,
           std::vector<bool> const &paramsAreGarbage);
    virtual ~Lookup();

    // Standard Expression API
    bool isAssignable() const;
    bool isConstant() const;

    // Common behavior required by NotifierImpl
    void handleActivate();
    void handleDeactivate();
    void handleChange(ExpressionId src);

    //
    // API to external interface
    //

    /**
     * @brief Set the value of this entry to UNKNOWN and notify all registered lookups.
     */
    void setUnknown();

    /**
     * @brief Update the entry with a new value and notify all registered lookups.
     * @param val The new value.
     * @note Implemented by LookupImpl below.
     */
    virtual void newValue(bool const &val) = 0;
    virtual void newValue(int32_t const &val) = 0;
    virtual void newValue(double const &val) = 0;
    virtual void newValue(std::string const &val) = 0;
    virtual void newValue(BooleanArray const &val) = 0;
    virtual void newValue(IntegerArray const &val) = 0;
    virtual void newValue(RealArray const &val) = 0;
    virtual void newValue(StringArray const &val) = 0;

    // Utility

    /**
     * @brief Get the state for this Lookup, if known.
     * @param result The place to store the State.
     * @return True if fully known, false if not.
     */
    bool getState(State &result) const; 

    /**
     * @brief Get the quiescence count (i.e. timestamp) for the most recent update.
     * @return The timestamp.
     */
    unsigned int getTimestamp() const;

  protected:

    // Behavior delegated to implementation classes
    virtual void makeUnknown() = 0;
    
    // Member variables shared with implementation classes
    State m_cachedState;
    std::vector<ExpressionId> m_params;
    std::vector<bool> m_garbage;
    ExpressionId m_stateName;
    StateCacheEntry* m_entry; // TODO opportunity to use refcounted ptr?
    unsigned int m_timestamp;
    bool m_known;
    bool m_stateKnown;
    bool m_stateIsConstant; // allows early caching of state value
    bool m_stateNameIsGarbage;

  private:
  };

  // Functionality common to all Lookups, but parceled out by type
  template <typename T>
  class LookupImpl : public Lookup, public ExpressionImpl<T>
  {
  public:
    LookupImpl(ExpressionId const &stateName,
               bool stateNameIsGarbage,
               std::vector<ExpressionId> const &params,
               std::vector<bool> const &paramsAreGarbage);
    ~LookupImpl();

    bool isKnown() const; // has to be here, not on Lookup

    bool getValueImpl(T &result) const;
    bool getValuePointerImpl(T const *&ptr) const;

    // API to external interface, exec
    void newValue(const T &val);
    // Error/type conversion
    template <typename U>
    void newValue(const U &val);

    // API to Lookup base
    void makeUnknown();

  private:
    // Prohibit copy, assign
    LookupImpl(LookupImpl const &);
    LookupImpl &operator=(LookupImpl const &);

  protected:
    T m_value;
  };

  template <typename T>
  class LookupOnChange : public LookupImpl<T>
  {
  public:
    LookupOnChange(ExpressionId const &stateName,
                   bool stateNameIsGarbage,
                   std::vector<ExpressionId> const &params,
                   std::vector<bool> const &paramsAreGarbage,
                   ExpressionId const &tolerance = ExpressionId::noId(),
                   bool toleranceIsGarbage = false);
    ~LookupOnChange();

    // API to external interface, exec
    void newValue(const T &val);
    // Error/type conversion
    template <typename U>
    void newValue(const U &val);

    // Wrappers around LookupImpl methods
    void handleActivate();
    void handleDeactivate();
    void handleChange(ExpressionId exp);

  protected:

  private:
    // Prohibit default, copy, assign
    LookupOnChange();
    LookupOnChange(const LookupOnChange &);
    LookupOnChange &operator=(const LookupOnChange &);

    // Unique member data
    ExpressionId m_tolerance;
    bool m_toleranceIsGarbage;
  };

} // namespace PLEXIL

#endif // PLEXIL_LOOKUP_HH
