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

//
// Lookup interface classes and type definitions
//
// Interface implementors may choose to implement Lookups either via
// ordinary C++ functions, or via classes derived from LookupHandler.
//
// Many systems send state updates at regular intervals
// (e.g. telemetry).  In this case, the interface can call the member
// function AdapterExecInterface::handleValueChange() when new data
// arrives, and no lookup handlers are necessary. If a handler for
// LookupNow is provided, it should do nothing.
//
// LookupNow handlers are called in the PLEXIL Exec inner loop,
// therefore blocking is strongly discouraged.
//
// A single lookup handler, whether a function or a LookupHandler
// instance, may handle multiple state names.
//

#include "ValueType.hh" // PLEXIL::Integer, PLEXIL::Real

#include <functional>
#include <memory>

namespace PLEXIL
{

  // forward references
  class AdapterExecInterface;
  class LookupReceiver;
  class State;

  //
  // Type aliases for functions which perform actions for Lookups in PLEXIL
  //

  //*
  // @brief A LookupNowHandler function queries the external system
  // for the specified state, and returns the value through a callback
  // object (the LookupReceiver).
  //
  // @note As LookupNowHandler functions are called in the Exec inner loop,
  //       blocking is strongly discouraged.
  //
  // @see LookupReceiver
  //
  using LookupNowHandler = std::function<void(const State &state, LookupReceiver *rcvr)>;
  using LookupNowHandlerPtr = std::shared_ptr<LookupNowHandler>;

  //
  // The remaining functions are optional, but may be useful for optimizing
  // interface or PLEXIL Exec utilization.
  //

  //*
  // @brief The PLEXIL Exec calls a SetThresholds handler when the Exec
  // activates a LookupOnChange for the named state.  It tells the
  // interface that it need not send updates for new values within the
  // given bounds.
  //
  // @note SetThresholds handlers are only applicable to
  // numeric-valued Lookups.  Only the handler corresponding to the
  // Lookup's declared type should be implemented.
  //
  // @note This is primarily used for the Time state, to set alarms
  // for the next scheduled event.
  //
  using SetThresholdsHandlerReal =
    std::function<void(const State &state, Real hi, Real lo)>;
  using SetThresholdsHandlerRealPtr =
    std::shared_ptr<SetThresholdsHandlerReal>;

  using SetThresholdsHandlerInteger =
    std::function<void(const State &state, Integer hi, Integer lo)>;
  using SetThresholdsHandlerIntegerPtr =
    std::shared_ptr<SetThresholdsHandlerInteger>;

  //*
  // @brief The PLEXIL Exec calls a ClearThresholds handler when the Exec
  // no longer wishes to enforce thresholds on the named state.
  //
  using ClearThresholdsHandler = std::function<void(const State &state)>;
  using ClearThresholdsHandlerPtr = std::shared_ptr<ClearThresholdsHandler>;

  //!
  // @brief A base class representing the API of a lookup handler object.
  //
  // @note The default methods for all member functions are effectively no-ops,
  //       because doing nothing is a legal option for each member function.
  //       Default methods are defined in AdapterConfiguration.cc.
  //
  // @note If no lookupNow method is implemented, the interface is
  //       responsible for posting state updates via
  //       AdapterExecInterface::handleValueChange().
  //
  struct LookupHandler
  {
    // Constructor
    LookupHandler() = default;

    // Virtual destructor
    virtual ~LookupHandler() = default;

    //!
    // @brief Prepare the handler for plan execution.
    // @return True if initialization successful, false otherwise.
    //
    // @note If the same handler is registered for multiple state names.
    //       this member function will be called once for each state name.
    //
    // @note Default method simply returns true.
    //
    virtual bool initialize();

    //!
    // @brief Query the external system for the specified state, and
    //        return the value through the callback object.
    // @param state The State to look up.
    // @param rcvr Pointer to the LookupReceiver callback object.
    //             Call its update() or setUnknown() member function
    //             to return a value from the lookup.
    //
    // @see LookupReceiver::update
    // @see LookupReceiver::setUnknown
    //
    // @note The default method does nothing, optionally printing a debug message.
    //
    // @note This member function is called in the PLEXIL Exec inner
    // loop, therefore blocking is strongly discouraged.
    //
    virtual void lookupNow(const State &state, LookupReceiver *rcvr);

    //
    // The following member functions are optional, and the default
    // methods are no-ops which optionally print a debug message.
    //

    //!
    // @brief setThresholds() is called when the PLEXIL Exec activates
    //        a LookupOnChange for the named state, to notify the interface
    //        that the Exec is only interested in new values at or outside
    //        the given bounds.
    //
    // @param state The state on which the bounds are being established.
    // @param hi The value at or above which updates should be sent to the Exec.
    // @param lo The value at or below which updates should be sent to the Exec.
    //
    // @note setThresholds() methods are only applicable to
    //       numeric-valued Lookups.  Only the member function corresponding
    //       to the Lookup's declared type should be implemented.
    //
    // @note This is primarily used for the 'time' state, to set wakeups in
    //       a tickless system.
    //
    virtual void setThresholds(const State & state, Real hi, Real lo);
    virtual void setThresholds(const State & state, Integer hi, Integer lo);

    //!
    // @brief Tell the interface that thresholds are no longer in effect
    //        for this state.
    // @param state The state.
    //
    virtual void clearThresholds(const State& state);
  };

  using LookupHandlerPtr = std::shared_ptr<LookupHandler>;

}

#endif // ABSTRACT_LOOKUP_HANDLER_HH
