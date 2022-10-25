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
// Handling queries and commands from the PLEXIL Exec to the outside world
//

#ifndef PLEXIL_DISPATCHER_HH
#define PLEXIL_DISPATCHER_HH

#include "ValueType.hh"

namespace PLEXIL
{

  // Forward declarations
  class Command;
  class LookupReceiver;
  class State;
  class Update;

  //! \class Dispatcher
  //! \brief Stateless abstract base class for requests/commands from
  //!        the PLEXIL Exec to the outside world.
  class Dispatcher
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Dispatcher() = default;

    //
    // API to Lookup
    //

    //! \brief Perform an immediate lookup on an existing state.
    //! \param state The state.
    //! \param receiver Callback object to receive the lookup result.
    //! \note Value is returned via methods on the LookupReceiver callback.
    virtual void lookupNow(State const &state, LookupReceiver *receiver) = 0;

    //! \brief Advise the interface of the current thresholds to use when reporting this state.
    //! \param state The state.
    //! \param hi The upper threshold, at or above which to report changes.
    //! \param lo The lower threshold, at or below which to report changes.
    //! \note This is a kludge, mostly used for the 'time' state, to
    //!       schedule wakeups in tickless systems.
    virtual void setThresholds(const State& state, Real hi, Real lo) = 0;
    virtual void setThresholds(const State& state, Integer hi, Integer lo) = 0;

    //! \brief Tell the interface that thresholds are no longer in effect
    //!        for this state.
    //! \param state The state.
    virtual void clearThresholds(const State& state) = 0;

    //
    // API to Exec
    //

    //! \brief Delegate this command for execution.
    //! \param cmd The command.
    virtual void executeCommand(Command *cmd) = 0;

    //! \brief Report a command arbitration failure in the appropriate
    //!        way for the application.
    //! \param cmd The rejected Command.
    virtual void reportCommandArbitrationFailure(Command *cmd) = 0;

    //! \brief Delegate this command to be aborted.
    //! \param cmd The command.
    virtual void invokeAbort(Command *cmd) = 0;

    //! \brief Delegate this update for execution.
    //! \param update The update.
    virtual void executeUpdate(Update *update) = 0;

  }; // class Dispatcher

  //! Global variable pointing to the Dispatcher instance
  extern Dispatcher *g_dispatcher;

} // namespace PLEXIL

#endif // PLEXIL_DISPATCHER_HH
