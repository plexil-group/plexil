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

#ifndef EXEC_LISTENER_BASE_HH
#define EXEC_LISTENER_BASE_HH

#include "NodeTransition.hh"

#include <vector>

namespace PLEXIL
{

  // Forward references
  class Expression;
  class Value;

  //! \class ExecListenerBase
  //! \brief A stateless abstract base class for notifying the outside world
  //!        of events resulting from PLEXIL plan execution.
  //! \ingroup Exec-Core
  //!
  //! ExecListenerBase is the interface used by PlexilExec to report
  //! node state transitions and assignments.  It is meant to batch
  //! the events and report the entire batch to the outside world when
  //! the stepComplete() member function is called.
  //!
  //! \see ExecListenerHub
  //! \see LuvListener
  class ExecListenerBase
  {
  public: 

    //! \brief Virtual destructor.
    virtual ~ExecListenerBase() = default;

	//
	// API to Exec
	//

	//! \brief Notify that some nodes have changed state.
    //! \param Vector of NodeTransition instances.
    //! \note This is called synchronously from the outer loop of the Exec.
    //!        As such it should not block on I/O.
    virtual void notifyOfTransitions(std::vector<NodeTransition> const &transitions) = 0;

    //! \brief Notify that a variable assignment has been performed.
    //! \param dest The Expression being assigned to.
    //! \param destName A string naming the destination.
    //! \param value The value (as a generic Value) being assigned.
    //! \note This is called synchronously from the inner loop of the Exec.
    //!       Listeners should not do any I/O during this call.
    virtual void notifyOfAssignment(Expression const *dest,
                                    std::string const &destName,
                                    Value const &value) = 0;

    //! \brief Notify that a step is complete and the listener
    //!        may publish transitions and assignments.
    virtual void stepComplete(unsigned int cycleNum) = 0;

  };

}

#endif // EXEC_LISTENER_BASE_HH
