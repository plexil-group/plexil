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

#ifndef PLEXIL_TIMEBASE_HH
#define PLEXIL_TIMEBASE_HH

#include "plexil-stdint.h" // intptr_t

namespace PLEXIL
{

  //
  // An abstract base class implementing timer functionality.
  //

  // Common signature of both POSIX signal handlers and Dispatch handlers
  using WakeupFn = void (*)(void *);

  class Timebase
  {
  public:
    virtual ~Timebase() = default;

    virtual double getTime() const = 0;

    virtual double getNextWakeup() const
    {
      return m_nextWakeup;
    }

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void setTimer(double d) = 0;

  protected:

    // Constructor only accessible to derived classes
    Timebase(WakeupFn f, intptr_t arg)
      : m_nextWakeup(0),
        m_wakeupFn(f),
        m_wakeupArg(arg)
    {
    }

    double m_nextWakeup;
    WakeupFn m_wakeupFn;
    intptr_t m_wakeupArg;
  };

} // namespace PLEXIL

#endif // PLEXIL_TIMEBASE_HH
