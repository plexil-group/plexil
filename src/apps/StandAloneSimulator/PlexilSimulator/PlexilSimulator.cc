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

#include "Simulator.hh"
#include "IpcCommRelay.hh"
#include "PlexilSimResponseFactory.hh"
#include "Debug.hh"
#include "ThreadSemaphore.hh"

#include <fstream>
#include <csignal>


PLEXIL::ThreadSemaphore doneSemaphore;
Simulator* _the_simulator_ = NULL;

void SIGINT_handler (int signum)
{
  assert (signum == SIGINT);
  debugMsg("PlexilSimulator", " Terminating simulator");
  if (_the_simulator_ != NULL)
    _the_simulator_->stop();
  doneSemaphore.post();
}

int main(int argc, char** argv)
{
  // defaults for command line args
  std::string commandScriptName("");
  std::string telemetryScriptName("");
  std::string centralhost("localhost:1381");
  std::string debugConfig("");

  std::string usage("Usage: PlexilSimulator -c <command script> -t <telemetry script> [-d <debug config file>] [-central <centralhost>]");

  for (int i = 1; i < argc; ++i)
    {
      if (strcmp(argv[i], "-c") == 0)
        commandScriptName = argv[++i];
      else if (strcmp(argv[i], "-t") == 0)
        telemetryScriptName = argv[++i];
      else if (strcmp(argv[i], "-d") == 0)
        debugConfig = argv[++i];
      else if (strcmp(argv[i], "-central") == 0)
        centralhost = argv[++i];
      else if (strcmp(argv[i], "-h") == 0)
        {
          std::cout << usage << std::endl;
          return 0;
        }
      else
	{
	  std::cout << "Unknown option '" 
		    << argv[i] 
		    << "'.  " 
		    << usage 
		    << std::endl;
	  return -1;
	}
    }

  if (commandScriptName.empty() && telemetryScriptName.empty())
    {
      std::cerr << "Error: no script(s) supplied\n" << usage << std::endl;
      return -1;
    }

  if (!debugConfig.empty())
    {
      std::ifstream dc(debugConfig.c_str());
      if (dc.fail())
	{
	  std::cerr << "Error: unable to open debug configuration file "
		    << debugConfig << std::endl;
	  return -1;
	}
      DebugMessage::setStream(std::cerr);
      if (!DebugMessage::readConfigFile(dc))
	{
	  std::cerr << "Error in debug configuration file " << debugConfig << std::endl;
	  return -1;
	}
    }

  debugMsg("PlexilSimulator",  
	   " Running with command script: " << commandScriptName
	   << " and telemetry script: " << telemetryScriptName);

  ResponseManagerMap mgrMap;
  {
    // These objects can go away as soon as we finish reading scripts.
    PlexilSimResponseFactory respFactory;
    SimulatorScriptReader rdr(mgrMap, respFactory);
    rdr.readCommandScript(commandScriptName);
    rdr.readTelemetryScript(telemetryScriptName);
  }
  
  {
    // Comm Relay has to be destroyed before we can nuke the simulator
    IpcCommRelay plexilRelay("RobotYellow", centralhost);
    _the_simulator_ = new Simulator(&plexilRelay, mgrMap);

    struct sigaction sa, previous_sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    //Register the handler for SIGINT.
    sa.sa_handler = SIGINT_handler;
    sigaction(SIGINT, &sa, &previous_sa);

    _the_simulator_->start();

    // wait here til we're interrupted
    doneSemaphore.wait();

    // Restore previous SIGINT handler
    sigaction(SIGINT, &previous_sa, NULL);
  }
  delete _the_simulator_;

  return 0;
}
