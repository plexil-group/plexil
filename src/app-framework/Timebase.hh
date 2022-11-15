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

#ifndef PLEXIL_TIMEBASE_HH
#define PLEXIL_TIMEBASE_HH

#include <cstdint>
#include <functional>
#include <memory>

namespace PLEXIL
{

  //! \class Timebase
  //! \brief a An abstract base class specifying how the PLEXIL
  //! application framework interacts with platform timing services.

  //! Implementations of the Timebase class are expected to perform
  //! the following functions:
  //! - Provide access to the current time (typically "wall clock" time);
  //! - Call a user-supplied wakeup function at the requested times.

  //! A Timebase operates in one of two modes:
  //! - *Deadline* mode, in which the wakeup function is called only
  //!   in response to a setTimer() call.
  //! - *Tick* mode, in which the wakeup function is called repeatedly
  //!   at a fixed interval, and calls to setTimer() are ignored.

  //! Deadline mode is the default, for compatibility with previous
  //! PLEXIL implementations.

  //! In either mode, the wakeup function is generally called after
  //! the specified time plus a variable latency has elapsed.  On some
  //! platforms (notably macOS), the wakeup function may be called
  //! early, due to optimizations which combine timer events. The
  //! Timebase client should not rely on the wakeup function being
  //! called exactly as specified, but should check the time at which
  //! the call is performed, and act accordingly.

  //! The wakeup function may be called from a signal handler, from an
  //! OS timer queue, or from a separate thread in the calling
  //! application. In deadline mode, the wakeup function can be
  //! invoked from within a setTimer() call. The client application is
  //! responsible for avoiding deadlocks and related conflicts.

  class Timebase
  {
  public:

    //! \typedef WakeupFn
    //! \brief Type alias for a function of no arguments, returning void.
    using WakeupFn = std::function<void()>;

    //! \brief Convenience function. Gets the time from an existing timebase.
    //! \return Time in seconds, as a double. Returns 0 if there is no
    //!         existing timebase.
    static double queryTime();

    //! \brief Virtual destructor.
    virtual ~Timebase();

    //! \brief Get the current wall clock time.
    //! \return The time in seconds, as a double float.
    virtual double getTime() const = 0;

    //! \brief Set the interval between ticks.
    //! \param intvl The tick interval, in microseconds.
    //! \note A tick interval of 0 commands the Timebase to deadline mode;
    //!       i.e. the wakeup function will only be called for events
    //!       scheduled via the setTimer() member function.
    //! \note Deadline mode is the default, for backward compatibility purposes.
    virtual void setTickInterval(uint32_t intvl) = 0;

    //! \brief Get the tick interval.
    //! \return The tick interval, in microseconds.
    //! \note A tick interval of 0 means the Timebase is operating in deadline mode.
    virtual uint32_t getTickInterval() const = 0;

    //! \brief Start the timebase. Allows the Timebase to call the wakeup function.
    virtual void start() = 0;

    //! \brief Stop the timebase. The wakeup function will not be called after this call.
    virtual void stop() = 0;

    //! \brief Set the time of the next deadline wakeup event.
    //! \param d The desired wakeup time.
    //! \note May not be called before the start() member function has been called.
    //! \note The deadline may be ignored if the Timebase is configured as a tick-based service.
    //!       In this case the wakeup function will be called after every tick.
    //! \note While generally the wakeup function will be called after the requested time,
    //!       on some platforms (notably macOS) the wakeup function may be called
    //!       *before* the requested time.
    virtual void setTimer(double d) = 0;

    //! \brief Get the time of the most recently scheduled deadline wakeup.
    //! \return The scheduled wakeup time; 0 if no deadline wakeup has
    //!         ever been scheduled.
    //! \note The next-wakeup time is not cleared by a wakeup event.
    //! \note If the timebase is in tick mode, i.e. getTickInterval()
    //!       returns a non-zero value, this function will always
    //!       return 0.
    double getNextWakeup() const;

  protected:

    //! \brief Constructor.
    //! \param f A function of no arguments, returning void.
    //! \note Constructor is only accessible to derived classes.
    Timebase(WakeupFn const &f);

    //! \brief C-callable wrapper for a wakeup function.
    //! \param Pointer to a Timebase instance.
    static void timebaseWakeup(Timebase *tb);

    //
    // Static member variables
    //
        
    static Timebase *s_instance;  //!< Pointer to the existing instance, if any.

    //
    // Shared member variables
    //

    double    m_nextWakeup;     //!< The time of the next scheduled deadline wakeup, in seconds since the epoch.
    WakeupFn  m_wakeupFn;       //!< The wakeup function.
    uint32_t  m_interval_usec;  //!< Repeat interval in microseconds.
    bool      m_started;        //!< true if the start() method has been called, false otherwise.

  private:

    // Copy, move constructors, assignment operators unimplemented.
    Timebase(Timebase const &) = delete;
    Timebase(Timebase &&) = delete;
    Timebase &operator=(Timebase const &) = delete;
    Timebase &operator=(Timebase &&) = delete;

  };

} // namespace PLEXIL

//! Make the default timebase factories accessible.
extern "C"
void initTimebaseFactories();

#endif // PLEXIL_TIMEBASE_HH
