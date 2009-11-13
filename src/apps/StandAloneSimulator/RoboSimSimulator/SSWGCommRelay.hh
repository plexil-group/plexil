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
#ifndef SSWG_COMM_RELAY_HH
#define SSWG_COMM_RELAY_HH

#include "CommRelayBase.hh"
#include "SSWGCallbackHandler.hh"
#include "SSWGClient.hh"
#include <map>
#include <string>

class SSWGCommRelay : public CommRelayBase, public SSWGCallbackHandler
{
public:
  SSWGCommRelay(const std::string& name,
                const std::string& ipAddress = "127.0.0.1", int portNumber=6164);
  ~SSWGCommRelay();
  virtual void receivedMessage (const std::string& sender,
                                const std::string& msg);
  virtual void sendResponse(const ResponseMessage* respMsg);
private:
  bool m_Connected;
  int m_UniqueId;
  SSWGClient m_SSWGClient;
  std::map<int, std::string> m_UniqueIdToSender;
  std::map<int, std::string> m_UniqueIdToCommand;
};

#endif // SSWG_COMM_RELAY_HH
