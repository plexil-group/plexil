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

#ifndef PLEXIL_STATE_CACHE_ENTRY_HH
#define PLEXIL_STATE_CACHE_ENTRY_HH

#include "State.hh"

namespace PLEXIL
{
  class Lookup;

  // TODO:
  // - Determine how interface is accessed
  // - Determine how quiescence count is obtained

  /**
   * @class StateCacheEntry
   * @brief Base class which provides the external API for a state cache entry,
   *        and value-type-independent state and functionality.
   */
  class StateCacheEntry
  {
  public:
    // Public constructor
    static StateCacheEntry *factory(State const &state, ValueType vtype);

    virtual ~StateCacheEntry();

    // Utility
    ValueType valueType() const;

    // API to Lookup
    void registerLookup(Lookup *); // calls checkIfStale()
    void unregisterLookup(Lookup *);

    // Check whether the cached value is current, and trigger an update if needed.
    void checkIfStale();

    //
    // Callbacks for external interfaces and value queue
    //

    /**
     * @brief Set the state to unknown.
     * @note Notifies all lookups of the new status.
     */
    void setUnknown();

    /**
     * @brief Update the cache entry with the given new value.
     * @param val The new value.
     * @return True if correct type for lookup, false otherwise.
     * @note Notifies all lookups of the new value.
     * @note Default method ignores the call and returns false.
     * @note The caller is responsible for deleting the object pointed to upon return.
     */
    virtual bool update(bool const &val) = 0;
    virtual bool update(int32_t const &val) = 0;
    virtual bool update(double const &val) = 0;
    virtual bool update(std::string const &val) = 0;
    virtual bool updatePtr(std::string const *valPtr) = 0;
    virtual bool updatePtr(BooleanArray const *valPtr) = 0;
    virtual bool updatePtr(IntegerArray const *valPtr) = 0;
    virtual bool updatePtr(RealArray const *valPtr) = 0;
    virtual bool updatePtr(StringArray const *valPtr) = 0;

    // For convenience of TestExternalInterface, others
    virtual bool update(Value const &val) = 0;

  protected:
    // Internal functions
    StateCacheEntry(State const &, ValueType vtype);

    /**
     * @brief Call the interface to update a stale value.
     * @note The lookup function is expected to return its value via
     *       the appropriate update function above.
     */
    void callLookup();

    /**
     * @brief Notify all subscribers of a change in value.
     */
    void notify() const;

    /**
     * @brief Notify one subscriber of a change in value.
     * @param l The Lookup.
     */
    virtual void notifyLookup(Lookup *l) const = 0;

    // State shared with derived classes
    State const m_state;
    std::vector<Lookup *> m_lookups;
    unsigned int m_timestamp;
    ValueType m_valueType;
    bool m_cachedKnown;

  private:
    // Default, copy, assign disallowed
    StateCacheEntry();
    StateCacheEntry(StateCacheEntry const &);
    StateCacheEntry &operator=(StateCacheEntry const &);
  };

  // Another invocation of CRTP.
  // This class should never be seen in public.
  template <class IMPL>
  class StateCacheEntryShim : public StateCacheEntry
  {
  public:
    StateCacheEntryShim(State const &state, ValueType vtype)
      : StateCacheEntry(state, vtype)
    {
    }

    ~StateCacheEntryShim()
    {
    }

    inline bool update(bool const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(val);
    }
    inline bool update(int32_t const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(val);
    }
    inline bool update(double const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(val);
    }
    inline bool update(std::string const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(val);
    }
    inline bool updatePtr(std::string const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(valPtr);
    }
    inline bool updatePtr(BooleanArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(valPtr);
    }
    inline bool updatePtr(IntegerArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(valPtr);
    }
    inline bool updatePtr(RealArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(valPtr);
    }
    inline bool updatePtr(StringArray const *valPtr)
    {
      return static_cast<IMPL *>(this)->updatePtrImpl(valPtr);
    }
    inline bool update(Value const &val)
    {
      return static_cast<IMPL *>(this)->updateImpl(val);
    }
    
  };

  template <typename T>
  class StateCacheEntryImpl : public StateCacheEntryShim<StateCacheEntryImpl<T> >
  {
  public:
    StateCacheEntryImpl(State const &, ValueType vtype);
    ~StateCacheEntryImpl();

    bool updateImpl(T const &val);

    // Type conversion or invalid type
    template <typename U>
    bool updateImpl(U const &val);

    bool updateImpl(Value const &val);

    bool updatePtrImpl(T const *valPtr);

    // Type conversion or invalid type
    template <typename U>
    bool updatePtrImpl(U const *valPtr);

  protected:

    void notifyLookup(Lookup *l) const;

  private:
    // Default, copy, assign disallowed
    StateCacheEntryImpl();
    StateCacheEntryImpl(StateCacheEntryImpl const &);
    StateCacheEntryImpl &operator=(StateCacheEntryImpl const &);

    T m_cachedValue;
  };

} // namespace PLEXIL

#endif // PLEXIL_STATE_CACHE_ENTRY_HH
