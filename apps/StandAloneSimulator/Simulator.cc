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
  return (CONVERT_TIMESPEC_TO_DOUBLE(t1) < CONVERT_TIMESPEC_TO_DOUBLE(t1)) ?
    true : false;
}

Simulator::Simulator(ResponseFactory* respFactory, 
                     CommRelayBase* commRelay) : 
  m_ResponseFactory(respFactory), m_CommRelay(commRelay),
  m_TimingService(TimingService(this)), m_SimulatorScriptReader(this)
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

bool Simulator::readScript(const std::string& fName)
{
  return m_SimulatorScriptReader.readScript(fName);
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
  std::map<const std::string, ResponseMessageManager*>::iterator iter;

  if ((iter = m_CmdToRespMgr.find(command)) == m_CmdToRespMgr.end())
    {
      std::cerr << "Simulator::scheduleResponseForMessage. The received command: "
                << command << " does not "
                << "have a registered response manager. Ignoring it." << std::endl;
      return;
    }
  ResponseMessageManager* msgMgr = iter->second;
  timeval tDelay;
  ResponseMessage* respMsg = msgMgr->getResponseMessages(tDelay);
  respMsg->id = uniqueId;
  timeval currTime;
  gettimeofday(&currTime, NULL);
  currTime.tv_sec += tDelay.tv_sec;
  currTime.tv_usec += tDelay.tv_usec;

  //  std::cout << "tDelay: " << tDelay.tv_sec << std::endl;
  MutexGuard mg(&m_TimerMutex);
  if ((tDelay.tv_sec != 0) || (tDelay.tv_usec != 0))
    {
      // Check if the new time already exists in the map
      if(m_TimeToResp.find(currTime) == m_TimeToResp.end())
        {
          // Schedule timer
          std::cout << "Simulator::scheduleResponseForMessage. Scheduling a timer" << std::endl;
          m_TimingService.setTimer(currTime);
        }
      m_TimeToResp.insert(std::pair<timeval, ResponseMessage*>(currTime, respMsg));
    }
  else
    {
      // send a response right away.
      m_CommRelay->sendResponse(respMsg);
    }
}

void Simulator::handleWakeUp()
{
  MutexGuard mg(&m_TimerMutex);
  std::cout << "Simulator::handleWakeUp" << std::endl;
  int count = m_TimeToResp.count(m_TimeToResp.begin()->first);
  //  std::cout << "count: " << count << std::endl;
  for (std::multimap<timeval, ResponseMessage*>::iterator iter = m_TimeToResp.begin();
       count > 0; ++iter, --count)
    {
      ResponseMessage* respMsg = iter->second;
      m_CommRelay->sendResponse(respMsg);
      delete respMsg;
      m_TimeToResp.erase(iter);
    }
}
