/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef ABSTRACT_LOOKUP_HANDLER_HH
#define ABSTRACT_LOOKUP_HANDLER_HH

#include "plexil-stdint.h" // int32_t

//
// Type definitions for Lookup interface functions and classes
//

namespace PLEXIL
{

  // forward references
  class AdapterExecInterface;
  class State;
  class StateCacheEntry;

  //
  // Type aliases for functions which perform actions for Lookups in PLEXIL
  //
  // Interface implementors can choose to implement Lookups as either
  // ordinary C++ functions, or as LookupHandler derived classes.
  //
  // Many systems send state updates at regular intervals
  // (e.g. telemetry).  In this case, the interface can call the
  // member function AdapterExecInterface::handleValueChange() when
  // new data arrives, and the LookupNow handler should do nothing.
  //
  // LookupNow handlers are called in the PLEXIL Exec inner loop,
  // therefore blocking is strongly discouraged.
  //
  // A single lookup handler, whether a function or a LookupHandler
  // instance, may handle multiple state names.
  //

  //*
  //
  // @brief A LookupNowHandler function queries the external system
  // for the specified state, and updates the cache entry.
  //
  // @see StateCacheEntry::update
  // @see StateCacheEntry::updatePtr
  // @see StateCacheEntry::setUnknown
  //

  typedef void (*LookupNowHandler)(const State &state, StateCacheEntry &cacheEntry);

  //
  // The remaining functions are optional, but may be useful for optimizing
  // interface or PLEXIL Exec utilization.
  //

  //* 
  //
  // @brief The PLEXIL Exec calls the SubscribeHandler to notify the
  // interface that the Exec is interested in updates for this state.
  //
  // @note SubscribeHandler and UnsubscribeHandler should be
  // registered as a pair.
  //
  // @see AdapterExecInterface::handleValueChange
  //

  typedef void (*SubscribeHandler)(const State &state, AdapterExecInterface *intf);

  //* 
  //
  // @brief The PLEXIL Exec calls the UnsubscribeHandler to notify the
  // interface that the Exec is no longer interested in updates for
  // this state.
  //
  // @note SubscribeHandler and UnsubscribeHandler should be
  // registered as a pair.
  //
  
  typedef void (*UnsubscribeHandler)(const State &state);

  //*
  //
  // @brief The PLEXIL Exec calls a SetThresholds handler when the Exec
  // activates a LookupOnChange for the named state.  It tells the
  // interface that it need not send updates for new values within the
  // given bounds.
  //
  // @note SetThresholds handlers will only be called on a state which
  // is currently subscribed.
  //
  // @note SetThresholds handlers are only applicable to
  // numeric-valued Lookups.  Only the handler corresponding to the
  // Lookup's declared type should be implemented.
  //
  // @note This is primarily used for the Time state, to set alarms
  // for the next scheduled event.
  //

  typedef void (*SetThresholdsHandlerReal)(const State &state, double hi, double lo);
  typedef void (*SetThresholdsHandlerInteger)(const State &state, int32_t hi, int32_t lo);

  //*
  //
  // @brief An extensible base class representing the API of a lookup
  // handler object.
  //
  // @note The default methods for all member functions are no-ops,
  // because doing nothing is a legal option for each member function.
  //
  // @note If the lookupNow method is empty, the interface is
  // responsible for posting state updates via
  // AdapterExecInterface::handleValueChange().
  //

  class LookupHandler
  {
  public:

    // Constructor
    LookupHandler()
    {
    }

    // Virtual destructor
    virtual ~LookupHandler()
    {
    }

    //*
    //
    // @brief Query the external system for the specified state, and
    // update the given state cache entry.
    // @param state The State to look up.
    // @param cacheEntry The StateCacheEntry for the given State.
    // Call its update(), updatePtr(), or setUnknown() member
    // function as appropriate.
    //
    // @see StateCacheEntry::update
    // @see StateCacheEntry::updatePtr
    // @see StateCacheEntry::setUnknown
    //
    // @note This member function is called in the PLEXIL Exec inner
    // loop, therefore blocking is strongly discouraged.
    //

    virtual void lookupNow(const State & /* state */,
                           StateCacheEntry & /* cacheEntry */)
    {
    }

    //
    // The following member functions are optional, and the default
    // methods are no-ops, as shown here.
    //

    //*
    //
    // @brief The subscribe() method notifies the interface that the
    // PLEXIL Exec is interested in updates for this state.
    //
    // @param state The State being subscribed to.
    // @param intf Pointer to the AdapterExecInterface, through which
    // updates in the state's value can be sent.
    //
    // @see AdapterExecInterface::handleValueChange
    //

    virtual void subscribe(const State & /* state */, AdapterExecInterface * /* intf */)
    {
    }

    //*
    //
    // @brief The unsubscribe() method notifies the interface that the
    // PLEXIL Exec is no longer interested in updates for this state.
    // @param state The State formerly subscribed to.
    //
    
    virtual void unsubscribe(const State & /* state */)
    {
    }

    //*
    //
    // @brief setThresholds() is called when the PLEXIL Exec activates
    // a LookupOnChange for the named state, to notify the interface
    // that the Exec is not interested in new values within the given
    // bounds.
    //
    // @param state The state on which the bounds are being
    // established.
    // @param hi The value above which updates should be sent to the
    // Exec.
    // @param lo The value below which updates should be sent to the
    // Exec.
    //
    // @note setThresholds() will only be called on a state which is
    // currently subscribed.
    //
    // @note setThresholds() methods are only applicable to
    // numeric-valued Lookups.  Only the member function corresponding
    // to the Lookup's declared type should be implemented.
    //
    // @note This is primarily used for the Time state, to set alarms
    // for the next scheduled event.
    //

    virtual void setThresholds(const State & /* state */,
                               double /* hi */,
                               double /* lo */)
    {
    }

    virtual void setThresholds(const State & /* state */,
                               int32_t /* hi */,
                               int32_t /* lo */)
    {
    }
  };

}

#endif // ABSTRACT_LOOKUP_HANDLER_HH
