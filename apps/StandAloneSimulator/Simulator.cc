/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
#include <sys/time.h>
#include <iostream>

#include "Simulator.hh"
#include "ResponseMessage.hh"
#include "ResponseMessageManager.hh"
#include "CommRelayBase.hh"
#include "MutexGuard.hh"

bool operator< (const timeval& t1, const timeval& t2)
{
  return ((t1.tv_sec < t2.tv_sec) || 
          ((t1.tv_sec == t2.tv_sec) && (t1.tv_usec < t2.tv_usec))) ? true : false;
}

timeval operator+ (const timeval& t1, const timeval& t2)
{
  timeval time;
  long usecL = t1.tv_usec + t2.tv_usec;
  double usecD = (static_cast<double>(usecL)) * ONE_MILLIONTH;
  
  time.tv_sec = t1.tv_sec + t2.tv_sec;
  if (usecD > 1.0)
    {
      time.tv_sec += 1;
      time.tv_usec = static_cast<long>((usecD - 1.0) / ONE_MILLIONTH);
    }
  else
    time.tv_usec = static_cast<long>((usecD) / ONE_MILLIONTH);

  return time;
}


Simulator::Simulator(ResponseFactory* respFactory, 
                     CommRelayBase* commRelay) : 
  m_ResponseFactory(respFactory), m_CommRelay(commRelay),
  m_TimingService(this), m_SimulatorScriptReader(this), m_TimerScheduled(false)
{
  m_CommRelay->registerSimulator(this);
  pthread_mutex_init(&m_TimerMutex, NULL);
}

Simulator::~Simulator()
{
  // delete all the registered response managers
  for(std::map<const std::string, ResponseMessageManager*>::iterator iter = m_CmdToRespMgr.begin();
      iter != m_CmdToRespMgr.end(); ++iter)
    delete iter->second;

  std::cout << "Destructing the Simulator object: There are " << m_TimeToResp.size()
            << " responses pending" << std::endl;
  pthread_mutex_destroy(&m_TimerMutex);
}

bool Simulator::readScript(const std::string& fName,
                           const std::string& fNameTelemetry)
{
  bool status = m_SimulatorScriptReader.readCommandScript(fName);
  if (fNameTelemetry != "NULL") 
    status &= m_SimulatorScriptReader.readTelemetryScript(fNameTelemetry);
  return status;
}

ResponseMessageManager* Simulator::getResponseMessageManager(const std::string& cmdName) const
{
  if (m_CmdToRespMgr.find(cmdName) != m_CmdToRespMgr.end())
    return m_CmdToRespMgr.find(cmdName)->second;

  return NULL;
}

void Simulator::registerResponseMessageManager(ResponseMessageManager* msgMgr)
{
  m_CmdToRespMgr[msgMgr->getIdentifier()] = msgMgr;
}

void Simulator::scheduleResponseForCommand(const std::string& command,
                                           int uniqueId)
{
  timeval time;
  std::cout << "Simulator::scheduleResponseForCommand for command: " << command 
            << std::endl;
  bool valid = constructNextResponse(command, uniqueId, time, MSG_COMMAND);
  if (valid) scheduleNextResponse(time);
}

void Simulator::scheduleResponseForTelemetry(const std::string& state)
{
  timeval time;
  std::cout << "Simulator::scheduleResponseForCommand for telemetry: " << state 
            << std::endl;
  bool valid = constructNextResponse(state, INT_MAX, time, MSG_TELEMETRY);
  if (valid) scheduleNextResponse(time);
}

bool Simulator::constructNextResponse(const std::string& command,
                                      int uniqueId, timeval& time,
                                      int type)
{
  std::map<const std::string, ResponseMessageManager*>::iterator iter;

  if ((iter = m_CmdToRespMgr.find(command)) == m_CmdToRespMgr.end())
    {
      std::cerr << "Simulator::constructNextResponse. The received command: "
                << command << " does not "
                << "have a registered response manager. Ignoring it." << std::endl;
      return false;
    }
  ResponseMessageManager* msgMgr = iter->second;
  timeval tDelay;
  ResponseMessage* respMsg = msgMgr->getResponseMessages(tDelay);
  respMsg->id = uniqueId;
  respMsg->name = command;
  respMsg->messageType = type;
  timeval currTime;
  gettimeofday(&currTime, NULL);
  std::cout << "Simulator::constructNextResponse. Current time: " 
            << currTime.tv_sec << std::endl;
  
  time = currTime + tDelay;

  std::cout << "tDelay: " << tDelay.tv_sec << ", time: " << time.tv_sec << std::endl;

  {
    MutexGuard mg(&m_TimerMutex);
    m_TimeToResp.insert(std::pair<timeval, ResponseMessage*>(time, respMsg));
  }
  
  return true;
}

/*
  By passing the time as argument we can avoid yet another mutex lock in this
  function.
 */
void Simulator::scheduleNextResponse(timeval time)
{
  bool immediateResp = false;

  if((!m_TimerScheduled) || (time < m_TimerScheduledTime))
    {
      // Schedule timer
      std::cout << "Simulator::scheduleResponseForMessage. Scheduling a timer" << std::endl;
      immediateResp = m_TimingService.setTimer(time);
      if (immediateResp) std::cout << "Immediate response required." << std::endl;
      m_TimerScheduled = true;
      m_TimerScheduledTime = time;
    }
  else
    {
      std::cout << "A wakeup call had already been scheduled for an earlier time." 
                << std::endl;
    }
  
  if (immediateResp) handleWakeUp();
}

void Simulator::handleWakeUp()
{
  std::cout << "Simulator::handleWakeUp" << std::endl;
  bool scheduleTimer=false;
  timeval time;
    
  {
    MutexGuard mg(&m_TimerMutex);
    int count = m_TimeToResp.count(m_TimeToResp.begin()->first);
    //std::cout << "count in the timer list map: " << count 
    //	      << ", total size: " << m_TimeToResp.size() << std::endl;
    for (std::multimap<timeval, ResponseMessage*>::iterator iter = m_TimeToResp.begin();
	 count > 0; ++iter, --count)
      {
	ResponseMessage* respMsg = iter->second;
	m_CommRelay->sendResponse(respMsg);
	std::cout << "Simulator::handleWakeUp. Sent response." << std::endl;
	delete respMsg;
	m_TimeToResp.erase(iter);
      }
    
    m_TimerScheduled = false;
    if (m_TimeToResp.size() > 0)
      {
	std::multimap<timeval, ResponseMessage*>::iterator iter = m_TimeToResp.begin();
	time = iter->first;
	scheduleTimer = true;
      }
  }

  if (scheduleTimer) scheduleNextResponse(time);
}

timeval Simulator::convertDoubleToTimeVal(double timeD)
{
  timeval time;
  time.tv_sec = static_cast<long>(timeD);
  time.tv_usec = static_cast<long>((timeD - static_cast<double>(time.tv_sec)) / ONE_MILLIONTH);
  
  return time;
}
