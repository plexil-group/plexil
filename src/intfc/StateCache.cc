/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "StateCache.hh"

#include "CachedValue.hh"
#include "Dispatcher.hh"
#include "Error.hh"
#include "Message.hh"
#include "State.hh"
#include "StateCacheEntry.hh"

#include <map>

namespace PLEXIL
{

  // Implementation of time query functions
  double StateCache::currentTime()
  {
    double result = 0.0;
    instance().ensureTimeEntry()->cachedValue()->getValue(result);
    return result;
  }

  double StateCache::queryTime()
  {
    // Update the cached value
    g_dispatcher->lookupNow(State::timeState(),
                            instance().ensureTimeEntry()->getLookupReceiver());
    // and return it
    return currentTime();
  }

  //! \class StateCacheImpl
  //! \brief Implements the StateCache API.
  class StateCacheImpl final : public StateCache
  {
    friend StateCache &StateCache::instance();

  private:

    //! \typedef EntryMap
    //! \brief Abbreviation for the type of the map structure.
    using EntryMap = std::map<State, std::unique_ptr<StateCacheEntry>>;

  public:

    //! \brief Virtual destructor.
    virtual ~StateCacheImpl() = default;

    //! Return the number of "macro steps" since this instance was constructed.
    //! @return The macro step count.
    virtual unsigned int getCycleCount() const
    {
      return m_cycleCount;
    }

    //! \brief Increment the Exec macro step count.
    virtual void incrementCycleCount()
    {
      ++m_cycleCount;
    }

    //! \brief Return the StateCacheEntry corresponding to the time state.
    //! \return Pointer to the entry.
    virtual StateCacheEntry *ensureTimeEntry()
    {
      if (!m_timeEntry) {
        // Initialize time state to 0
        m_timeEntry = ensureStateCacheEntry(State::timeState());
        m_timeEntry->updateValue(Value((Real) 0.0), m_cycleCount);
      }
      return m_timeEntry;
    }

    //! \brief Update the value for this state's Lookup.
    //! \param state The state.
    //! \param value The new value.
    virtual void lookupReturn(State const &state, Value const &value)
    {
      ensureStateCacheEntry(state)->updateValue(value, m_cycleCount);
    }

    //! \brief Construct or find the cache entry for this state.
    //! \param state The state being looked up.
    //! \return Pointer to the StateCacheEntry for the state.
    //! \note Return value can be presumed to be non-null.
    virtual StateCacheEntry *ensureStateCacheEntry(State const &state)
    {
      EntryMap::iterator iter = m_map.find(state);
      if (iter == m_map.end())
        iter = m_map.emplace(state, makeStateCacheEntry()).first;
      return iter->second.get();
    }

    //! \brief Get the object which should receive lookup result
    //!        notifications for this state.
    //! \param state Const reference to the State.
    //! \return Pointer to a LookupReceiver instance.
    virtual LookupReceiver *getLookupReceiver(State const &state)
    {
      return static_cast<LookupReceiver *>(ensureStateCacheEntry(state));
    }

    //
    // Message API to external interfaces
    //

    //! \brief Process a newly received Message.
    //! \param msg Const pointer to the new message.
    virtual void messageReceived(Message const *msg)
    {
      ensureStateCacheEntry(s_haveMessage)->update(true);
      ensureStateCacheEntry(s_peekAtMessage)->update(msg->message.name());
      ensureStateCacheEntry(s_peekAtMessageSender)->update(msg->sender);
    }

    //! \brief Perform the appropriate actions when the message queue
    //!        becomes empty.
    virtual void messageQueueEmpty()
    {
      ensureStateCacheEntry(s_haveMessage)->update(false);
      ensureStateCacheEntry(s_peekAtMessage)->setUnknown();
      ensureStateCacheEntry(s_peekAtMessageSender)->setUnknown();
    }

    //! \brief Accept an incoming message and associate it with the handle.
    //! \param msg Pointer to the message.  StateCache takes ownership of the message.
    //! \param handle String used as a handle for the message.
    virtual void assignMessageHandle(Message *msg, std::string const &handle)
    {
      Value handleValue(handle);
      ensureStateCacheEntry(State("MessageText", handleValue))->update(msg->message.name());
      size_t count = msg->message.parameterCount();
      ensureStateCacheEntry(State("MessageParameterCount", handleValue))
        ->updateValue(Value((Integer) count), m_cycleCount);
      for (size_t i = 0; i < count; ++i) {
        ensureStateCacheEntry(State("MessageParameter",
                                    handleValue,
                                    Value((Integer) i)))
          ->updateValue(msg->message.parameter(i), m_cycleCount);
      }
      ensureStateCacheEntry(State("MessageSender", handleValue))
        ->updateValue(Value(msg->sender), m_cycleCount);
      ensureStateCacheEntry(State("MessageArrived", handleValue))
        ->updateValue(Value(msg->timestamp), m_cycleCount);
      delete msg;
    }

    //! \brief Release the message handle, and clear the message data
    //!        associated with that handle.
    //! \param handle The handle being released.
    virtual void releaseMessageHandle(std::string const &handle)
    {
      // Need the parameter count to delete all the parameters
      Value handleValue(handle);
      EntryMap::iterator iter = m_map.find(State("MessageParameterCount", handleValue));
      if (iter == m_map.end())
        return; // not there, therefore already deleted or never existed

      Integer count;
      if (!iter->second->cachedValue()->getValue(count)) {
        // warn of internal error (NYI)
        return;
      }
      
      if (iter->second->hasRegisteredLookups()) {
        // BIG OOPS - can't delete these w/o leaving dangling pointers
        // warn (NYI)
        return;
      }

      m_map.erase(iter);
      for (Integer i = 0; i < count; ++i) {
        deleteStateCacheEntry(State("MessageParameter",
                                    handleValue,
                                    Value(i)));
      }
      deleteStateCacheEntry(State("MessageText", handleValue));
      deleteStateCacheEntry(State("MessageSender", handleValue));
      deleteStateCacheEntry(State("MessageArrived", handleValue));
    }

  private:

    //! \brief Default constructor.  Only accessible to StateCache::instance().
    StateCacheImpl()
      : m_map(),
        m_timeEntry(nullptr),
        m_cycleCount(1)
    {
    }

    //! \brief Get or construct a state cache entry for the given state.
    //! \param state Const reference to the state.
    //! \return Pointer to the entry.
    virtual StateCacheEntry *getStateCacheEntry(State const &state)
    {
      EntryMap::iterator iter = m_map.find(state);
      if (iter == m_map.end())
        iter = m_map.emplace(state, makeStateCacheEntry()).first;
      return iter->second.get();
    }

    //! \brief Delete the state cache entry for the named state.
    //! \param state Const reference to the state.
    void deleteStateCacheEntry(State const &state)
    {
      EntryMap::iterator iter = m_map.find(state);
      if (iter == m_map.end())
        return; // already deleted or never there
      if (iter->second->hasRegisteredLookups()) {
        // warn (NYI) and bail out
        return;
      }
      m_map.erase(iter);
    }

    // Unimplemented
    StateCacheImpl(StateCacheImpl const &) = delete;
    StateCacheImpl(StateCacheImpl &&) = delete;
    StateCacheImpl &operator=(StateCacheImpl const &) = delete;
    StateCacheImpl &operator=(StateCacheImpl &&) = delete;

    //! \brief The actual map.
    EntryMap m_map;

    //! \brief Pointer to the state cache entry for the time state.
    StateCacheEntry *m_timeEntry;

    //! \brief The Exec major cycle counter.
    unsigned int m_cycleCount;

    //
    // Static member variables for messaging
    //
    
    static const State s_haveMessage;
    static const State s_peekAtMessage;
    static const State s_peekAtMessageSender;
  };

  const State StateCacheImpl::s_haveMessage = State("HaveMessage");
  const State StateCacheImpl::s_peekAtMessage = State("PeekAtMessage");
  const State StateCacheImpl::s_peekAtMessageSender = State("PeekAtMessageSender");

  //! \brief Singleton accessor.
  StateCache &StateCache::instance()
  {
    static StateCacheImpl sl_instance;
    return static_cast<StateCache &>(sl_instance);
  }

} // namespace PLEXIL
