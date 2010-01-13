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

#include "SSWGClient.hh"
#include "SSWGServer.hh"
#include "SSWGCallbackHandler.hh"
#include "ThreadSpawn.hh"
#include <unistd.h>
#include <sstream>

class ClientMessageHandler : public SSWGCallbackHandler
{
public:
	ClientMessageHandler(const std::string& name) : m_Name(name){}
	~ClientMessageHandler(){}
	virtual void receivedMessage (const std::string& sender,
                                      const std::string& msg)
	{
		std::cout << m_Name << ": In callback Msg received: " << msg << std::endl;
	}
private:
	const std::string m_Name;
};

void testClient()
{
	ClientMessageHandler cmh1("Client1");
	SSWGClient client;
	client.connectToServer("Client1", "127.0.0.1", 6165, &cmh1);
	std::string base = "Count at Client1: ";
	int count = 1;
	while(1)
	{
		std::ostringstream strout;
		strout << base << count;
		client.sendMessage(strout.str(), "Client2");
		sleep(1);
		++count;
	}

}

void testClient2()
{
	ClientMessageHandler cmh2("Client2");
	SSWGClient client;
	client.connectToServer("Client2", "127.0.0.1", 6165, &cmh2);
	std::string base = "Count at Client2: ";
	int count = 100;
	while(1)
	{
		std::ostringstream strout;
		strout << base << count;
		client.sendMessage(strout.str(), "Client1");
		sleep(1);
		++count;
	}
}

void testServer()
{
	SSWGServer server(SSWGCOMM_HIGH);
	server.acceptConnections(6165);
}

int main()
{
  std::cout << "In the main function" << std::endl;
  pthread_t threadId1, threadId2, threadId3;
  threadSpawn((THREAD_FUNC_PTR)testServer, NULL, threadId1);
  sleep(1);
  threadSpawn((THREAD_FUNC_PTR)testClient, NULL, threadId2);
  sleep(1);
  threadSpawn((THREAD_FUNC_PTR)testClient2, NULL, threadId3);
  
  sleep(30);
  std::cout << "End of main." << std::endl;
  return 1;
}
