#ifndef TIMING_SERVICE_HH
#define TIMING_SERVICE_HH

#include <sys/time.h>
#include <string.h>

class Simulator;

class TimingService
{
public:
  TimingService(Simulator* _callBack=NULL);
  ~TimingService();
  static void timerHandler (int signum);
  bool  setTimer(timeval time);

private:  
  void setupTimer(Simulator* _simulator);

  static Simulator* m_Simulator;
  bool m_TimerSetup;
  struct itimerval m_Timer;
};
#endif
