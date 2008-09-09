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
#include "SSWGCommRelay.hh"
#include "Simulator.hh"
#include "ResponseMessage.hh"

SSWGCommRelay::SSWGCommRelay(const std::string& name,
                             const std::string& ipAddress, int portNumber)
  : CommRelayBase(name), m_UniqueId(0)
{
  m_Connected = m_SSWGClient.connectToServer(name, ipAddress, 
                                             portNumber, this);
  if (!m_Connected) std::cerr << "The executive is unable to connect to the "
                              << "communication server" << std::endl;
  else
    std::cout << "Successfully connected to server." << std::endl;
}

SSWGCommRelay::~SSWGCommRelay()
{
}

void SSWGCommRelay::receivedMessage (const std::string& sender,
                                     const std::string& msg)
{
  std::cout << "\n\nSSWGCommRelay:: got something back from " << sender
            << " who says: " << msg << std::endl;
  
  m_UniqueIdToSender[m_UniqueId] = sender;
  m_UniqueIdToCommand[m_UniqueId] = msg;
  m_Simulator->scheduleResponseForCommand(msg, m_UniqueId);
  ++m_UniqueId;
}

void SSWGCommRelay::sendResponse(const ResponseMessage* respMsg)
{
  std::cout << "Sending message: " << respMsg->contents
            << std::endl;
  
  if (m_Connected) 
    m_SSWGClient.sendMessage(m_UniqueIdToCommand[respMsg->id] + "," + respMsg->contents, 
                             m_UniqueIdToSender[respMsg->id]);
}
