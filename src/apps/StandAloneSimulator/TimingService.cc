/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
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
#include "timeval-utils.hh"
#include "Simulator.hh"
#include <signal.h>
#include <iostream>

Simulator* TimingService::m_Simulator=NULL;

TimingService::TimingService(Simulator* _m_Simulator) : m_TimerSetup(false)
{
  setupTimer(_m_Simulator);
}

TimingService::~TimingService()
{
}

void TimingService::setupTimer(Simulator* _m_Simulator)
{
  struct sigaction sa;
  
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &TimingService::timerHandler;
  sigaction (SIGALRM, &sa, NULL);
  
  m_Timer.it_interval.tv_sec = m_Timer.it_interval.tv_usec = 0;
  m_Simulator = _m_Simulator;
  m_TimerSetup = true;
}

void TimingService::timerHandler (int signum)
{
  m_Simulator->handleWakeUp();
}

bool TimingService::setTimer(const timeval& time)
{
  if (m_TimerSetup)
    {
      timeval cTime;
      gettimeofday(&cTime, NULL);
      
      // compute (time - cTime).
      
      m_Timer.it_value = time - cTime;
      
      if (m_Timer.it_value.tv_sec < 0 || m_Timer.it_value.tv_usec < 0)
        return true;
      
      int status = setitimer (ITIMER_REAL, &m_Timer, NULL);
      if (status == -1)
        {
          std::cerr << "TimingService::setTimer Error while setting timer." << std::endl;
          perror("Error for setitimer");
        }
      else
        {
          std::cout << "Set interval timer for: " << m_Timer.it_value.tv_sec << "(sec) "
                    << m_Timer.it_value.tv_usec << "(usec)." << std::endl;
        }
    }
  else
    std::cerr << "Error: TimingService::setTimer. Timer has not been setup." << std::endl;

  return false;
}
