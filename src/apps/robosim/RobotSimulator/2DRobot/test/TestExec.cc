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

#include "../SSWGComm/SSWGClient.hh"
#include "../SSWGComm/SSWGCallbackHandler.hh"
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

int main()
{
  ClientMessageHandler cmh1("Client1");
  SSWGClient client;
  client.connectToServer("Client1", "127.0.0.1", 6164, &cmh1);

  while(1)
    {
      std::ostringstream strout;
      
      std::cout << "Enter command [R(ight), L(eft), U(p), D(own), S(tate)," 
                << " E(nergySensor), G(goalSensor), V(isibiltySensor)]: ";
      std::string cmd, command;
      std::cin >> cmd;
      
      if ((cmd == "R") || (cmd == "r"))
        command = "MoveRight";
      else if ((cmd == "L") || (cmd == "l"))
        command = "MoveLeft";
      else if ((cmd == "U") || (cmd == "u"))
        command = "MoveUp";
      else if ((cmd == "D") || (cmd == "d"))
        command = "MoveDown";
      else if ((cmd == "E") || (cmd == "e"))
        command = "QueryEnergySensor";
      else if ((cmd == "G") || (cmd == "g"))
        command = "QueryGoalSensor";
      else if ((cmd == "V") || (cmd == "v"))
        command = "QueryVisibilitySensor";
      else if ((cmd == "S") || (cmd == "s"))
        command = "QueryRobotState";
      else
        std::cout << "Unknown command" << std::endl;

      client.sendMessage(command, "RobotYellow");
    }

  return 0;
}

