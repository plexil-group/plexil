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

#include "plexil-config.h"

#include "Simulator.hh"

#include "Agenda.hh"
#include "CommandResponseManager.hh"
#include "CommRelayBase.hh"
#include "GenericResponse.hh"
#include "ResponseMessage.hh"
#include "SimulatorScriptReader.hh"
#include "TimingService.hh"

#include "Debug.hh"
#include "Error.hh"
#include "timeval-utils.hh"

#include <iomanip>
#include <memory>
#include <thread>

#if defined(HAVE_CERRNO)
#include <cerrno>
#elif defined(HAVE_ERRNO_H)
#include <errno.h>
#endif

using PLEXIL::Value;

class SimulatorImpl : public Simulator
{
private:

  //
  // Member data
  //

  TimingService m_TimingService;

  typedef std::map<std::string, Value> NameValueMap;
  NameValueMap m_LookupNowValueMap;

  CommRelayBase *m_CommRelay; // owned by the application
  std::unique_ptr<Agenda> m_Agenda;
  std::unique_ptr<ResponseManagerMap> m_CmdToRespMgr;
  std::thread m_SimulatorThread;
  bool m_Started;
  bool m_Stop;

public:

  SimulatorImpl(CommRelayBase *commRelay, ResponseManagerMap *map, Agenda *agenda)
    : m_TimingService(),
      m_LookupNowValueMap(),
      m_CommRelay(commRelay),
      m_Agenda(agenda),
      m_CmdToRespMgr(map),
      m_SimulatorThread(),
      m_Started(false),
      m_Stop(false)
  {
    m_CommRelay->registerSimulator(this);
  }

  // In case someone is foolish enough to derive from this class...
  virtual ~SimulatorImpl()
  {
    // Shut down anything that may be running
    stop();
    debugMsg("Simulator:~Simulator",
             " shutting down with " << m_Agenda->size() << " responses pending");
  }

  virtual void start()
  {
    assertTrue_2(!m_Started,
                 "StandAloneSimulator::start: simulatorTopLevel already running");
    m_SimulatorThread = std::thread([this]() -> void { this->simulatorTopLevel(); });
    debugMsg("Simulator:start", " top level thread started");
  }

  virtual void stop()
  {
    if (!m_Started)
      return;

    debugMsg("Simulator:stop", " called");
    m_TimingService.stopTimer();

    // Tell the sim thread to stop and wait for it to join
    m_Stop = true;
    m_SimulatorThread.join();
    debugMsg("Simulator:stop", " succeeded");
  }

  // *** BEWARE ***
  // This can be called directly from main() or run as a thread;
  // see examples/robosim/RoboSimSimulator/RoboSimSimulator.cc
  // for an example of the latter.
  void simulatorTopLevel()
  {
    //
    // Initialize signal handling
    //
    if (!m_TimingService.defaultInitializeSignalHandling())
      return;

    m_Started = true;

    timeval now;
    gettimeofday(&now, NULL);
    debugMsg("Simulator:start", " at "
             << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
             << timevalToDouble(now));

    // Schedule initial telemetry responses
    m_Agenda->setSimulatorStartTime(now);
  
    //
    // Set the timer for the first event, if any
    //

    timeval firstWakeup;
    firstWakeup.tv_sec = firstWakeup.tv_usec = 0;
    if (!m_Agenda->empty())
      firstWakeup = m_Agenda->nextResponseTime();

    if (firstWakeup.tv_sec != 0 || firstWakeup.tv_usec != 0) {
      debugMsg("Simulator:start",
               " scheduling first event at "
               << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
               << timevalToDouble(firstWakeup));
      scheduleNextResponse(firstWakeup);
    }

    //
    // Handle timer events until interrupted
    //

    while (!m_Stop) {
      // wait for next timer wakeup
      int waitResult = m_TimingService.wait();
      if (waitResult != 0) {
        // received some other signal
        debugMsg("Simulator:simulatorTopLevel",
                 " timing service received signal " << waitResult << ", exiting");
        break;
      }

      // check for exit request
      if (m_Stop) {
        debugMsg("Simulator:simulatorTopLevel", " stop request received");
        break;
      }

      handleWakeUp();
    }

    //
    // Clean up
    //
    m_TimingService.stopTimer();
    m_TimingService.restoreSignalHandling();

    m_Stop = false;
    m_Started = false;

    debugMsg("Simulator:simulatorTopLevel", " cleaning up");
  }

  void scheduleResponseForCommand(const std::string& command,
                                  void* uniqueId)
  {
    timeval time;
    gettimeofday(&time, NULL);
    debugMsg("Simulator:scheduleResponseForCommand",
             " for : " << command);
    bool valid = constructNextResponse(command, uniqueId, time, MSG_COMMAND);
    if (valid)
      scheduleNextResponse(time);
    else
      debugMsg("Simulator:scheduleResponseForCommand",
               "Not a command that needs a response.");
  }

  /**
   * @brief Get the current value of the named state.
   * @param stateName The state name to which we are responding.
   * @return Pointer to a const ResponseMessage object, or NULL.
   */
  ResponseMessage* getLookupNowResponse(const std::string& stateName, void* uniqueId) const
  {
    NameValueMap::const_iterator it = m_LookupNowValueMap.find(stateName);
    if (it == m_LookupNowValueMap.end())
      return NULL; // Name not known or no value established yet
    return new ResponseMessage(stateName, it->second, MSG_LOOKUP, uniqueId);
  }

private:

  //
  // Implementation details
  //
  
  CommandResponseManager* getCommandResponseManager(const std::string& cmdName) const
  {
    if (m_CmdToRespMgr->find(cmdName) != m_CmdToRespMgr->end())
      return m_CmdToRespMgr->find(cmdName)->second.get();

    return NULL;
  }

  bool constructNextResponse(const std::string& command,
                             void* uniqueId, 
                             timeval& time,
                             int type)
  {
    ResponseManagerMap::const_iterator iter  = m_CmdToRespMgr->find(command);
    if (iter == m_CmdToRespMgr->end()) {
      debugMsg("Simulator:constructNextResponse",
               " No response manager for \"" << command << "\", ignoring.");
      return false;
    }
    CommandResponseManager* msgMgr = iter->second.get();
    timeval tDelay;
    const GenericResponse* respBase = msgMgr->getResponses(tDelay);
    if (respBase == NULL) {
      debugMsg("Simulator:constructNextResponse",
               " No more responses for \"" << command << "\"");
      return false;
    }
  
    ResponseMessage* respMsg =
      new ResponseMessage(respBase->name, respBase->value, type, uniqueId);
    scheduleMessage(tDelay, respMsg);
  
    return true;
  }

  /**
   * @brief Schedules a message to be sent at a future time.
   * @param time The absolute time at which to send the message.
   * @param msg The message to be sent.
   */

  void scheduleMessage(const timeval& delay, ResponseMessage* msg)
  {
    timeval now;
    gettimeofday(&now, NULL);
    timeval eventTime = now + delay;
    debugMsg("Simulator:scheduleMessage",
             " scheduling message at "
             << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
             << timevalToDouble(eventTime));
    m_Agenda->scheduleResponse(eventTime, msg);
  }

  void scheduleNextResponse(const timeval& time)
  {
    timeval nextEvent;
    m_TimingService.getTimer(nextEvent);
    if ((nextEvent.tv_sec == 0 && nextEvent.tv_usec == 0)
        || time < nextEvent) {
      // Schedule timer
      debugMsg("Simulator:scheduleNextResponse", " Scheduling a timer");
      bool timeIsFuture = m_TimingService.setTimer(time);
      if (timeIsFuture) {
        debugMsg("Simulator:scheduleNextResponse",
                 " Timer set for "
                 << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
                 << timevalToDouble(time));
      }
      else {
        debugMsg("Simulator:scheduleNextResponse", " Immediate response required");
        handleWakeUp();
      }
    }
    else {
      debugMsg("Simulator:scheduleNextResponse",
               " A wakeup has already been scheduled for an earlier time.");
    }
  }

  void handleWakeUp()
  {
    timeval now;
    gettimeofday(&now, NULL);
    debugMsg("Simulator:handleWakeUp",
             " entered at "
             << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
             << timevalToDouble(now));

    //
    // Send every message with a scheduled time earlier than now.
    //
    while (!m_Agenda->empty()) {
      timeval const &responseTime = m_Agenda->nextResponseTime();
      debugMsg("Simulator:handleWakeUp", " next response at "
               << responseTime.tv_sec << '.' << responseTime.tv_usec);
      if (responseTime > now)
        break;
    
      ResponseMessage *resp = m_Agenda->popResponse();
      checkError(resp,
                 "Simulator:handleWakeup: Agenda::popResponse returned null");
      if (resp->getMessageType() == MSG_TELEMETRY) {
        // Store the value for subsequent LookupNow requests
        m_LookupNowValueMap[resp->getName()] = resp->getValue();
      }
      debugMsg("Simulator:handleWakeUp", " sending response "
               << resp->getName() << " value " << resp->getValue()
               << " scheduled for "
               << responseTime.tv_sec << '.' << responseTime.tv_usec);

      m_CommRelay->sendResponse(resp); // comm relay will delete resp
      debugMsg("Simulator:handleWakeUp", " Sent response");
    }

    debugMsg("Simulator:handleWakeUp", " done sending responses for now");

    //
    // Schedule next wakeup, if any
    //
    struct timeval nextWakeup = m_Agenda->nextResponseTime();
    if (nextWakeup.tv_sec != 0 && nextWakeup.tv_usec != 0) {
      debugMsg("Simulator:handleWakeUp",
               " Scheduling next wakeup at "
               << std::setiosflags(std::ios_base::fixed) << std::setprecision(6)
               << timevalToDouble(nextWakeup));
      scheduleNextResponse(nextWakeup);
    }
    debugMsg("Simulator:handleWakeUp", " completed");
  }

};

Simulator *makeSimulator(CommRelayBase* commRelay, ResponseManagerMap *map, Agenda *agenda)
{
  return new SimulatorImpl(commRelay, map, agenda);
}
