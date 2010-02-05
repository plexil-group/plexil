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

#include "Debug.hh"
#include "Error.hh"

#include <cerrno>
#include <iostream>

TimingService::TimerCallbackFn TimingService::s_Callback = NULL;
void* TimingService::s_CallbackArg = NULL;

TimingService::TimingService(TimerCallbackFn _callback, void* _callbackArg) 
  : m_TimerSetup(false)
{
  s_Callback = _callback;
  s_CallbackArg = _callbackArg;
  setupTimer();
}

TimingService::~TimingService()
{
  stopTimer();
}

void TimingService::setupTimer()
{
  struct sigaction sa;
  
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = &TimingService::timerHandler;
  sigaction(SIGALRM, &sa, &m_oldSigaction);
  
  m_Timer.it_interval.tv_sec = m_Timer.it_interval.tv_usec = 0;
  m_TimerSetup = true;
}

void TimingService::timerHandler (int signum)
{
  checkError(signum == SIGALRM,
	     "TimingService: Fatal error: Handler function called with wrong signal # " << signum);
  assertTrueMsg(s_Callback != NULL,
		"TimingService: Fatal error: Callback is null at wakeup!");
  (s_Callback)(s_CallbackArg);
}

bool TimingService::setTimer(const timeval& time)
{
  if (m_TimerSetup)
    {
      timeval cTime;
      gettimeofday(&cTime, NULL);
      m_Timer.it_value = time - cTime;
      
      if (m_Timer.it_value.tv_sec < 0 || m_Timer.it_value.tv_usec < 0)
        return true;
      
      int status = setitimer(ITIMER_REAL, &m_Timer, NULL);
      if (status == -1)
        {
          std::cerr << "TimingService::setTimer Error " << errno << " while setting timer." << std::endl;
          perror("Error for setitimer");
        }
      else
        {
          debugMsg("TimingService:setTimer",
		   " Set interval timer for: " << m_Timer.it_value.tv_sec << "(sec) "
		   << m_Timer.it_value.tv_usec << "(usec).");
        }
    }
  else
    std::cerr << "Error: TimingService::setTimer. Timer has not been set up." << std::endl;

  return false;
}

void TimingService::stopTimer()
{
  if (m_TimerSetup)
    {
      debugMsg("TimingService:stopTimer", " disabling timer interrupts");
      m_Timer.it_interval.tv_sec = m_Timer.it_interval.tv_usec = 0;
      int status = setitimer(ITIMER_REAL, &m_Timer, NULL);
      if (status == -1)
        {
          std::cerr << "TimingService:stopTimer Error " << errno << " while stopping timer." << std::endl;
          perror("Error for setitimer");
        }

      debugMsg("TimingService:stopTimer", " restoring old SIGALRM handler");
      sigaction(SIGALRM, &m_oldSigaction, NULL);
      m_TimerSetup = false;
    }
}
