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
#include "PlexilCommRelay.hh"
#include "Simulator.hh"
#include "ResponseMessage.hh"
#include <iostream>
#include <sstream>
#include "ThreadSpawn.hh"
#include <ServerSocket.h>
#include <ClientSocket.h>

// C wrapper to call a C++ method. Used while spawning a thread.
void spawnThreadForEachClient(void* args)
{

  std::cout << "Spawning the listening loop." << std::endl;
  PlexilCommRelay* server = static_cast<PlexilCommRelay*>(args);

  // Now we know that the universal exec is up, we can try and connect
  // to its listening port.

  bool done = false;
  ServerSocket ss(server->getListeningPortNumber());
  if (ss.accept(ss))
    {
      while(!done)
        {
          std::string msg;
          try
            {
              ss >> msg;
              server->receivedMessage(msg);
            }
          catch (SocketException se)
            {
              done = true;
              std::cout << "spawnThreadForEachClient: " << se.description() << std::endl;
            }
        }
    }
}

PlexilCommRelay::PlexilCommRelay(const std::string& _host,
                                 int sendingPort, int listeningPort)
  : CommRelayBase(_host), m_ClientSocket(NULL), m_HostName(_host), m_SendingPort(sendingPort), 
    m_ListeningPort(listeningPort)
{
  if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForEachClient, (void *)this,
                  m_ThreadId) != true)
    {
      std::cout << "Error spawning thread for the receiving socket." << std::endl;
    }

  // Try connecting to the listening port universal exec. It may fail if the exec is still not
  // up. We will try again after the exec sends a message.
  //  this->connectToUniversalExec();
}

PlexilCommRelay::~PlexilCommRelay()
{
  std::cout << "Cancelling thread ...";
  pthread_cancel(m_ThreadId);
  pthread_join(m_ThreadId, NULL);
  std::cout << "done" << std::endl;
}

void PlexilCommRelay::connectToUniversalExec()
{
  std::cout << "Trying to connect to the client socket" << std::endl;
  try 
    {
      m_ClientSocket = new ClientSocket(m_HostName, m_SendingPort);
      std::cout << "Succeeded in connecting to the sending socket." << std::endl;
    }
  catch (SocketException se)
    {
      m_ClientSocket = NULL;
      std::cout << "PlexilCommRelay::connectToUniversalExec(): Setting up client(sender): " 
                << se.description() << std::endl;
    }
}

void PlexilCommRelay::receivedMessage (const std::string& msg)
{
  std::cout << "\n\nPlexilCommRelay:: got something: " << msg << std::endl;
  
  //if (m_ClientSocket) *m_ClientSocket << "Here is your response.";
  m_Simulator->scheduleResponseForCommand(msg, 0);
}

void PlexilCommRelay::sendResponse(const ResponseMessage* respMsg)
{
  // See if a listener is up.
  if (getClientSocket() == NULL)
    connectToUniversalExec();
  
  std::cout << "PlexilCommRelay::sendResponse Sending message: " << respMsg->contents
            << std::endl;
  timeval currTime;
  gettimeofday(&currTime, NULL);
  std::cout << "PlexilCommRelay::sendResponse. Current time: " 
            << currTime.tv_sec << std::endl;

  std::ostringstream str;
  str << respMsg->messageType;
  
  std::string retValue = str.str() + " " + respMsg->name + " " + respMsg->contents;
  if (m_ClientSocket != NULL) *m_ClientSocket << retValue;

}
