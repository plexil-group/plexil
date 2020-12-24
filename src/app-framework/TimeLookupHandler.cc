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

//
// **** WORK IN PROGRESS ****
//

#include "TimeLookupHandler.hh"

#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "ExternalInterface.hh" // g_interface
#include "InterfaceError.hh"
#include "State.hh"
#include "StateCacheEntry.hh"
#ifdef PLEXIL_WITH_THREADS
#include "ThreadSpawn.hh"
#endif

#include <iomanip>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

#if defined(HAVE_CLOCK_GETTIME)

#if defined(HAVE_CTIME)
#include <ctime>
#elif defined(HAVE_TIME_H)
#include <time.h>
#endif
#include "timespec-utils.hh"

#elif defined(HAVE_GETTIMEOFDAY)

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#include "timeval-utils.hh"

#endif

namespace PLEXIL
{

  class TimeLookupHandlerImpl : public TimeLookupHandler
  {
  public:

    TimeLookupHandlerImpl()
    {
    }
    
    virtual ~TimeLookupHandlerImpl() = default;

    virtual bool initialize() override
    {
    }

    virtual void lookupNow(const State &state, LookupReceiver *rcvr) override
    {
    }

    virtual void setThresholds(const State & state, Real hi, Real lo)
    {
    }

  };

}
