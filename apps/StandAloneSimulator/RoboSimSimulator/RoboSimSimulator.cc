#include "SimulatorScriptReader.hh"
#include "Simulator.hh"
#include "RoboSimResponseFactory.hh"
#include "SSWGCommRelay.hh"

#include <assert.h>

bool done=false;

void SIGINT_handler (int signum)
{
  assert (signum == SIGINT);
  std::cout << "In SIGINT_handler. The simulator has been terminated." << std::endl;

  done = true;
}

int main(int argc, char** argv)
{
  RoboSimResponseFactory respFactory;
  SSWGCommRelay sswgRelay("RobotYellow");
  Simulator simulator(&respFactory, &sswgRelay);

  simulator.readScript("Test.script");

  struct sigaction sa;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  //Register the handler for SIGINT.
  sa.sa_handler = SIGINT_handler;
  sigaction (SIGINT, &sa, 0);

  while(!done){;}

  return 0;
}
