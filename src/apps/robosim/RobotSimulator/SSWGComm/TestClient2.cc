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
#include "SSWGCallbackHandler.hh"
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
	  static int count2=0;
          std::cout << m_Name << ": In callback Msg received: " << msg << std::endl;
          std::cout << "Message count: " << count2++ << std::endl;
	}
private:
	const std::string m_Name;
};

int main()
{
	ClientMessageHandler cmh2("Client2");
	SSWGClient client;
        client.connectToServer("Client2", "127.0.0.1", 6164, &cmh2);
        //client.connectToServer("Client2", "143.232.66.72", 6165, &cmh2); // alder
	std::string base = "Count at Client2: ";
	int count = 100;
	while(1)
	{
		std::ostringstream strout;
		strout << base << count;
		client.sendMessage(strout.str(), "Client1");
                //usleep(100000);
		sleep(1);
		++count;
	}
        return 1;
}
