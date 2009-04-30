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
#include "LcmBaseImpl.hh"

// C wrapper to call a C++ method. Used while spawning a thread.
void spawnThreadForEachClient(void* args)
{

  std::cout << "Spawning the listening loop." << std::endl;
  PlexilCommRelay* server = static_cast<PlexilCommRelay*>(args);

  while (1)
    {
      lcm_handle(server->getLCM());
      usleep(500000);
    }
}

PlexilCommRelay::PlexilCommRelay(const std::string& _host)
  : CommRelayBase(_host), m_lcm(NULL)
{
  m_lcm = lcm_create("udpm://");
  m_lcmBaseImpl = new LcmBaseImpl(m_lcm, this);
  if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForEachClient, (void *)this,
                  m_ThreadId) != true)
    {
      std::cout << "Error spawning thread for the receiving socket." << std::endl;
    }
}

PlexilCommRelay::~PlexilCommRelay()
{
  delete m_lcmBaseImpl;
  lcm_destroy(m_lcm);
  std::cout << "Cancelling thread ...";
  pthread_cancel(m_ThreadId);
  pthread_join(m_ThreadId, NULL);
  std::cout << "done" << std::endl;
}

void PlexilCommRelay::receivedMessage (const std::string& msg)
{
  std::cout << "\n\nPlexilCommRelay:: got something: " << msg << std::endl;
  
  m_Simulator->scheduleResponseForCommand(msg, 0);
}

void PlexilCommRelay::sendResponse(const ResponseMessage* respMsg)
{
  
  std::cout << "\nPlexilCommRelay::sendResponse Sending message: " << respMsg->contents
            << " of type " << respMsg->messageType << " for: " << respMsg->name << std::endl;
  timeval currTime;
  gettimeofday(&currTime, NULL);
  std::cout << "PlexilCommRelay::sendResponse. Current time: " 
            << currTime.tv_sec << std::endl;
  
  m_lcmBaseImpl->sendMessage(respMsg->name, respMsg->contents, respMsg->messageType);
}
