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
#include "ThreadSpawn.hh"
#include "timeval-utils.hh"

#include <iomanip>

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

  CommRelayBase *m_CommRelay;
  Agenda *m_Agenda;
  ResponseManagerMap *m_CmdToRespMgr;
  pthread_t m_SimulatorThread;
  bool m_Started;
  bool m_Stop;

public:

  SimulatorImpl(CommRelayBase *commRelay, ResponseManagerMap *map, Agenda *agenda)
    : m_TimingService(),
      m_LookupNowValueMap(),
      m_CommRelay(commRelay),
      m_Agenda(agenda),
      m_CmdToRespMgr(map),
      m_SimulatorThread((pthread_t) 0),
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

    // delete all the response managers
    for (ResponseManagerMap::iterator iter = m_CmdToRespMgr->begin();
         iter != m_CmdToRespMgr->end();
         ++iter)
      delete iter->second;

    debugMsg("Simulator:~Simulator",
             " shutting down with " << m_Agenda->size() << " responses pending");
    delete m_Agenda;
  }

  virtual void start()
  {
    threadSpawn(run, this, m_SimulatorThread);
  }

  virtual void stop()
  {
    debugMsg("Simulator:stop", " called");
    if (!m_Started)
      return;

    m_TimingService.stopTimer();

    if (m_Stop) {
      // we tried the gentle approach already -
      // take more drastic action
      if (m_SimulatorThread == pthread_self()) {
        errorMsg("Simulator:stop: Emergency stop!");
      }
      else {
        int pthread_errno = pthread_cancel(m_SimulatorThread);
        if (pthread_errno == ESRCH) {
          // no such thread to cancel, i.e. it's already dead
          m_Stop = false;
          m_Started = false;
          return;
        }
        else if (pthread_errno != 0) {
          errorMsg("Simulator:stop: fatal error: pthread_cancel returned " << pthread_errno);
        }

        // successfully canceled, wait for it to exit
        pthread_errno = pthread_join(m_SimulatorThread, NULL);
        if (pthread_errno == 0 || pthread_errno == ESRCH) {
          // thread joined or died before it could be joined
          // either way we succeeded
          m_Stop = false;
          m_Started = false;
        }
        else {
          errorMsg("Simulator:stop: fatal error: pthread_join returned " << pthread_errno);
        }
      }
    }
    else {
      // Usual case
      // stop the sim thread
      m_Stop = true;
      if (m_SimulatorThread != pthread_self()) {
        // Signal the thread to stop
        int pthread_errno = pthread_kill(m_SimulatorThread, SIGTERM);
        if (pthread_errno == 0) {
          // wait for thread to terminate
          pthread_errno = pthread_join(m_SimulatorThread, NULL);
          if (pthread_errno == 0 || pthread_errno == ESRCH) {
            // thread joined or died before it could be joined
            // either way we succeeded
            m_Stop = false;
            m_Started = false;
          }
          else {
            errorMsg("Simulator::stop: fatal error: pthread_join returned " << pthread_errno);
          }
        }
        else if (pthread_errno != ESRCH) {
          // thread is already dead
          m_Stop = false;
          m_Started = false;
        }
        else {
          errorMsg("Simulator::stop: fatal error: pthread_kill returned " << pthread_errno);
        }
      }
    }
  }

  static void* run(void* this_as_void_ptr)
  {
    Simulator* _this = reinterpret_cast<Simulator*>(this_as_void_ptr);
    _this->simulatorTopLevel();
    return NULL;
  }

  void simulatorTopLevel()
  {
    // if called directly from the main() thread,
    // record our thread ID
    if (m_SimulatorThread == (pthread_t) 0)
      m_SimulatorThread = pthread_self();
  
    assertTrue_2(m_SimulatorThread == pthread_self(),
                 "Internal error: simulatorTopLevel running in thread other than m_SimulatorThread");

    //
    // Initialize signal handling
    //
    if (!m_TimingService.defaultInitializeSignalHandling())
      return;

    m_Started = true;

    timeval now;
    gettimeofday(&now, NULL);
    debugMsg("Simulator:start", " at "
             << std::setiosflags(std::ios_base::fixed) 
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
               << std::setiosflags(std::ios_base::fixed)
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
        debugMsg("Simulator:simulatorTopLevel", " timing service received signal " << waitResult << ", exiting");
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
    m_SimulatorThread = (pthread_t) 0;
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
      return m_CmdToRespMgr->find(cmdName)->second;

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
    CommandResponseManager* msgMgr = iter->second;
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
             << std::setiosflags(std::ios_base::fixed) 
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
                 << std::setiosflags(std::ios_base::fixed) 
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
             << std::setiosflags(std::ios_base::fixed) 
             << timevalToDouble(now));

    //
    // Send every message with a scheduled time earlier than now.
    //
    while (!m_Agenda->empty()) {
      timeval responseTime;
      ResponseMessage* resp = m_Agenda->getNextResponse(responseTime);
      checkError(resp != NULL,
                 "Simulator:handleWakeup: Agenda returned NULL for getNextResponse")
        debugMsg("Simulator:handleWakeUp", " got response " << resp->getName()
                 << " with time of "
                 << responseTime.tv_sec << '.' << responseTime.tv_usec);
      if (responseTime > now)
        break;
    
      m_Agenda->pop();
      debugMsg("Simulator:handleWakeUp", " got past m_Agenda->pop()");
      if (resp->getMessageType() == MSG_TELEMETRY) {
        // Store the value for subsequent LookupNow requests
        m_LookupNowValueMap[resp->getName()] = resp->getValue();
      }
      m_CommRelay->sendResponse(resp);
      debugMsg("Simulator:handleWakeUp", " got past m_CommRelay->sendResponse()");
      debugMsg("Simulator:handleWakeUp", " Sent response");
      // delete resp; // handled by comm relay
    }

    debugMsg("Simulator:handleWakeUp", " done sending responses for now");

    //
    // Schedule next wakeup, if any
    //
    struct timeval nextWakeup = m_Agenda->nextResponseTime();
    if (nextWakeup.tv_sec != 0 && nextWakeup.tv_usec != 0) {
      debugMsg("Simulator:handleWakeUp",
               " Scheduling next wakeup at "
               << std::setiosflags(std::ios_base::fixed) 
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
