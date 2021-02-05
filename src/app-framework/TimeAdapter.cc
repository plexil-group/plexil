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

#include "plexil-config.h"  // TODO: check if redundant

#include "AdapterConfiguration.hh"
#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh" // REGISTER_ADAPTER() macro
#include "Configuration.hh"
#include "Debug.hh"
#include "Error.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"
#include "LookupHandler.hh"
#include "LookupReceiver.hh"
#include "State.hh"
#include "TimebaseFactory.hh"

#include <iomanip> // std::setprecision()
#include <memory>  // std::unique_ptr<>

//
// TODO: Document!!
//

//
// Issues:
// * Need experience with tick based timing
//

namespace PLEXIL
{

  //
  // Forward references
  //
  static void timeout(void *arg);

  struct TimeLookupHandler : public LookupHandler
  {
    TimeLookupHandler(Timebase *timebase)
      : m_timebase(timebase)
    {
    }
    
    virtual ~TimeLookupHandler() = default;

    //!
    // @brief Query the external system for the specified state, and
    //        return the value through the callback object.
    // @param state The State to look up.
    // @param rcvr Pointer to the LookupReceiver callback object.
    //
    virtual void lookupNow(const State & /* state */, LookupReceiver *rcvr) override
    {
      debugMsg("TimeLookupHandler:lookupNow", " called");
      rcvr->update(m_timebase->getTime());
    }

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
    virtual void setThresholds(const State & state, Real hi, Real lo) override
    {
      debugMsg("TimeLookupHandler:setThresholds",
               " requesting wakeup at " << std::setprecision(15) << hi);
      m_timebase->setTimer(hi);
    }

  private:

    //
    // Member variables
    //

    Timebase *m_timebase;
  };

  class TimeAdapter : public InterfaceAdapter
  {
  public:
    TimeAdapter(AdapterExecInterface &intf, AdapterConf *conf)
      : InterfaceAdapter(intf, conf),
        m_timebase()
    {
    }

    virtual ~TimeAdapter() = default;

    //!
    // @brief Construct the appropriate handler objects as specified in the
    //        configuration XML, and register them with the AdapterConfiguration
    //        instance.
    // @param config Pointer to the AdapterConfiguration interface registry.
    // @return true if successful, false otherwise.
    //
    virtual bool initialize(AdapterConfiguration *config) override
    {
      // Construct Timebase
      pugi::xml_node const tb_xml = getXml().child(InterfaceSchema::TIMEBASE_TAG);
      m_timebase.reset(makeTimebase(tb_xml, &timeout, (void *) this));
      config->registerLookupHandler(std::make_shared<TimeLookupHandler>(m_timebase.get()),
                                    "time");
      return true;
    }

    //!
    // @brief Start the interface.
    // @return true if successful, false otherwise.
    //
    virtual bool start() override
    {
      try {
        m_timebase->start();
        debugMsg("TimeAdapter:start", " complete");
        return true;
      } catch (const InterfaceError &e) {
        std::cerr << "ERROR: Starting timebase threw an exception:\n "
                  << e.what() << std::endl;
      }
      return false;
    }

    //! @brief Stop the interface.
    virtual void stop() override
    {
      try {
        m_timebase->stop(); 
        debugMsg("TimeAdapter:stop", " complete");
      } catch (const InterfaceError &e) {
        std::cerr << "ERROR: Stopping timebase threw an exception:\n "
                  << e.what() << std::endl;
      }
    }

    //!
    // @brief Get the timebase in use.
    // @return Pointer to the timebase (may be null).
    //
    Timebase *getTimebase()
    {
      return m_timebase.get();
    }

  private:

    //
    // Member variables
    //

    std::unique_ptr<Timebase> m_timebase;
  };

  //! @brief Wake up the Exec on a timer signal.
  //! @param arg Pointer to the TimeAdapter, cast as a void *.
  static void timeout(void *arg)
  {
    TimeAdapter *adpt = reinterpret_cast<TimeAdapter *>(arg);
    Timebase *tb = adpt->getTimebase();

    // Get the time
    double now = tb->getTime();
    debugMsg("TimeAdapter:timeout", " at " << std::setprecision(15) << now);

    // Check if timer went off too soon.
    double next = tb->getNextWakeup();
    if (next && now < next) {
      // Alarm went off too early. Hit the snooze button.
      debugMsg("TimeAdapter:timeout", " early wakeup, resetting");
      tb->setTimer(next); // possibility of reentrant call to this function
    }

    // Notify in any case
    adpt->getInterface().notifyOfExternalEvent();
  }

} // namespace PLEXIL

extern "C"
void initTimeAdapter()
{
  REGISTER_ADAPTER(PLEXIL::TimeAdapter, "Time");

  // Register timebase factories while we're at it
  initTimebaseFactories();
}
