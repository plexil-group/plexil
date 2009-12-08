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
#include "PlexilCommRelay.hh"
#include "PlexilSimResponseFactory.hh"

#include <assert.h>
#include <signal.h>

bool done=false;

void SIGINT_handler (int signum)
{
  assert (signum == SIGINT);
  std::cout << "In SIGINT_handler. The simulator has been terminated." << std::endl;

  done = true;
}

int main(int argc, char** argv)
{
  std::string commandScriptName("NULL");
  std::string telemetryScriptName("NULL");
  std::string usage("Usage: plexilSimulator -c <command-script> -t <telemetry-script>");

   for (int i = 1; i < argc; ++i)
   {
      if (strcmp(argv[i], "-c") == 0)
        commandScriptName = argv[++i];
      else if (strcmp(argv[i], "-t") == 0)
        telemetryScriptName= argv[++i];
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

   std::cout << "Running the simulator with command script: " << commandScriptName
             << " and telemetry script: " << telemetryScriptName << std::endl;

  PlexilSimResponseFactory respFactory;
  
  PlexilCommRelay plexilRelay("RobotYellow");
  Simulator simulator(&respFactory, &plexilRelay);
  simulator.readScript(commandScriptName, telemetryScriptName);

  struct sigaction sa;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  //Register the handler for SIGINT.
  sa.sa_handler = SIGINT_handler;
  sigaction (SIGINT, &sa, 0);

  while(!done){;}

  return 0;
}
