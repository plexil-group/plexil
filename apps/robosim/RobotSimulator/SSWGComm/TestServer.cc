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

#include "SSWGServer.hh"
#include <signal.h>
#include <iostream>
#include <unistd.h>
#include <assert.h>

/* Signal handler for SIGINT. */
void SIGINT_handler (int signum)
{
#if defined(UNIX)
  assert (signum == SIGINT);
#endif
  std::cout << "In SIGINT_handler. Communication server has been terminated" << std::endl;
}

int main(int argc, char**argv)
{
  // Parser command line option

  int echoMessage = 0;
  int portNumber = 6164;

  for (int i = 1; i < argc; i++)
    {
      if ((strcmp(argv[i], "-h") == 0) ||
          (strcmp(argv[i], "-help") == 0))
        {
          std::cout << "Usage:  " << argv[0] << " -e[choMessages] <low, medium, high; default=low>"
                    << " -p[ortNumber] <int; default=6164>" << std::endl;
          exit(0);
        }
      else if (!strcmp(argv[i], "-e"))
        {
          std::string oLevel(argv[++i]);
          if (oLevel == "low")
            echoMessage = SSWGCOMM_LOW;
          else if (oLevel == "medium")
            echoMessage = SSWGCOMM_MEDIUM;
          else if (oLevel == "high")
            echoMessage = SSWGCOMM_HIGH;
          else 
            {
              std::cout << "Unknown echo mode, try -help for options." << std::endl;
              exit(0);
            }
        }
      else if (!strcmp(argv[i], "-p"))
        portNumber = atoi(argv[++i]);
      else 
        {
          std::cout << "Unknown command line option, try -help for options." << std::endl;
          exit(0);
        }
    }

  std::cout << "Command line arguments: echoMessages: " << echoMessage
            << ", portNumber: " << portNumber << std::endl;
  
  struct sigaction sa;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;

  //Register the handler for SIGINT.
  sa.sa_handler = SIGINT_handler;
  sigaction (SIGINT, &sa, 0);
  SSWGServer g_Server(echoMessage);
  g_Server.acceptConnections(portNumber);
  
  std::cout << "Server cleaning up.." << std::endl;
  sleep(3);
  return 0;
}
