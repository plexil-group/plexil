// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    // Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    // Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    // Neither the name of the Universities Space Research Association nor the
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

#ifndef PLEXIL_TIME_LOOKUP_HANDLER_HH
#define PLEXIL_TIME_LOOKUP_HANDLER_HH

#include "LookupHandler.hh"
#include "LookupReceiver.hh"

namespace PLEXIL
{

  class TimeLookupHandler : public LookupHandler
  {
  public:
    TimeLookupHandler();
    virtual ~TimeLookupHandler();

    //!
    // @brief Prepare the handler for plan execution.
    // @return True if initialization successful, false otherwise.
    //
    virtual bool initialize() override;

    //*
    // @brief Query the external system for the specified state, and
    // return the result via a callback object.
    // @param state The State to look up.
    // @param rcvr Pointer to the callback object
    //
    virtual void lookupNow(const State &state, LookupReceiver *rcvr) override;

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
    virtual void setThresholds(const State & state, Real hi, Real lo);

  };
}

#endif // PLEXIL_TIME_LOOKUP_HANDLER_HH
