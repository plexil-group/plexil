#ifndef TIMING_SERVICE_HH
#define TIMING_SERVICE_HH

#include <sys/time.h>
#include <ace/Thread_Manager.h>
#include <ace/Reactor.h>

class Simulator;

class TimingService : public ACE_Event_Handler
{
public:
  TimingService(Simulator* sim=NULL);
  ~TimingService();
  void setTimer(timeval time);
  int handle_timeout(const ACE_Time_Value& tv, const void* timer_data_as_void);
private:
  Simulator* m_Simulator;
};

#endif // TIMING_SERVICE_HH
