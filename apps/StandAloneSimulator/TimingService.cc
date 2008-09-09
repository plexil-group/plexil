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
