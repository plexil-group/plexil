#include "TimingService.hh"
#include "Simulator.hh"
#include <signal.h>
#include <iostream>


timeval operator- (const timeval& t1, const timeval& t2)
{
  timeval time;
  
  time.tv_sec = t1.tv_sec - t2.tv_sec;

  if (t1.tv_usec >= t2.tv_usec)
    {
      time.tv_usec = t1.tv_usec - t2.tv_usec;
    }
  else
    {
      time.tv_sec -= 1;
      time.tv_usec = 
        static_cast<long>(((static_cast<double>((t1.tv_usec) * ONE_MILLIONTH) + 1.0) -
                           (static_cast<double>((t2.tv_usec) * ONE_MILLIONTH))) / ONE_MILLIONTH);
    }

  return time;
}

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

bool TimingService::setTimer(timeval time)
{
  if (m_TimerSetup)
    {
      timeval cTime;
      gettimeofday(&cTime, NULL);
      
      // compute (time - cTime).
      
      m_Timer.it_value = time - cTime;
      
      if (m_Timer.it_value.tv_sec < 0)
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
