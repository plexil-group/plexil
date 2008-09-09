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
#include "TimingService.hh"
#include "Simulator.hh"
#include <iostream>

static void* reactorEventLoop(void* arg)
{
  std::cout << "Starting reactor event loop for the local timing service." << std::endl;
  ACE_Reactor::instance()->run_reactor_event_loop();
  return NULL;
}


TimingService::TimingService(Simulator* sim) : m_Simulator(sim)
{
  if (-1 == ACE_Thread_Manager::instance()->spawn((ACE_THR_FUNC)reactorEventLoop))
    std::cerr << "TimingService::TimingService. Error while spawing thread for reactor loop" 
              << std::endl;
}

TimingService::~TimingService()
{
  ACE_Reactor::instance()->end_reactor_event_loop();
  std::cout << "Ended reactor event loop for the local timing service." << std::endl;
  ACE_Thread_Manager::instance()->suspend_all();
}

void TimingService::setTimer(timeval time)
{
  timeval currTime;
  gettimeofday(&currTime, NULL);
  time.tv_sec -= currTime.tv_sec;
  time.tv_usec -= currTime.tv_usec;
  std::cout << "TimingService:setTimer. Setting timer" << std::endl;
  ACE_Reactor::instance()->schedule_timer(this, 0, ACE_Time_Value(time));
}

int TimingService::handle_timeout(const ACE_Time_Value& tv, const void* timer_data_as_void)
{
  //  std::cout << "Got woken up" << std::endl;
  m_Simulator->handleWakeUp();
  return 0;
}
