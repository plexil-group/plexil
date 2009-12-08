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

#ifndef ROBO_SIM_INTERFACE_ADAPTER_HH
#define ROBO_SIM_INTERFACE_ADAPTER_HH

#include <vector>
#include <list>
#include <string>
#include "InterfaceAdapter.hh"
#include "SSWGCallbackHandler.hh"
#include "SSWGClient.hh"

class RoboSimInterfaceAdapter : public PLEXIL::InterfaceAdapter, public SSWGCallbackHandler
{
public:
  RoboSimInterfaceAdapter(PLEXIL::AdapterExecInterface& execInterface,
			  const std::string& _name = "RoboSimExec",
                          const std::string& ipAddress = "127.0.0.1", 
			  int portNumber=6164);
  ~RoboSimInterfaceAdapter();
  
  virtual void lookupNow(const PLEXIL::StateKey& stateKey,
                         std::vector<double>& dest);
  
  virtual void executeCommand(const PLEXIL::LabelStr& name,
                              const std::list<double>& args,
                              PLEXIL::ExpressionId dest,
                              PLEXIL::ExpressionId ack);
  virtual void executeFunctionCall(const PLEXIL::LabelStr& name,
                                   const std::list<double>& args,
                                   PLEXIL::ExpressionId dest,
                                   PLEXIL::ExpressionId ack);
  virtual void receivedMessage (const std::string& sender,
                                const std::string& msg);

private:

  std::vector<double> extractMessageContents(std::string& cmdName,
                                             const std::string& msg);

  std::string m_Name;
  SSWGClient m_SSWGClient;
  bool m_Connected;
  std::map<std::string, PLEXIL::ExpressionId> m_CommandMap;
  std::map<std::string, PLEXIL::ExpressionId>::iterator m_CommandMapIter;
  std::map<std::string, std::vector<double> > m_LookupMap;
  std::map<std::string, std::vector<double> >::iterator m_LookupMapIter;
  std::vector<std::string> m_MoveDirectionVector;
};

#endif
