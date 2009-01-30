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

#include <iostream>
#include <LabelStr.hh>
#include <StoredArray.hh>

#include "RoboSimInterfaceAdaptor.hh"
#include "CoreExpressions.hh"
#include "AdaptorExecInterface.hh"


RoboSimInterfaceAdaptor::RoboSimInterfaceAdaptor(PLEXIL::AdaptorExecInterface& execInterface,
						 const std::string& _name,
                                                 const std::string& ipAddress, 
                                                 int portNumber)
  : InterfaceAdaptor(execInterface),
    m_Name(_name)
{
  m_Connected = m_SSWGClient.connectToServer(m_Name, ipAddress, portNumber, this);
  if (!m_Connected) std::cerr << "The executive is unable to connect to the "
                              << "communication server" << std::endl;
  else
    std::cout << "Successfully connected to server." << std::endl;

  m_MoveDirectionVector.push_back("MoveUp");
  m_MoveDirectionVector.push_back("MoveRight");
  m_MoveDirectionVector.push_back("MoveDown");
  m_MoveDirectionVector.push_back("MoveLeft");
}

RoboSimInterfaceAdaptor::~RoboSimInterfaceAdaptor()
{
}
  
void RoboSimInterfaceAdaptor::lookupNow(const PLEXIL::StateKey& stateKey,
                                        std::vector<double>& dest)
{
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdaptor", "Received LookupNow for " 
           << nStr);
  dest.resize(1);

  if ((m_LookupMapIter = m_LookupMap.find(nStr)) != m_LookupMap.end())
    {
      // The state is available at this time
      std::vector<double> retValues = m_LookupMapIter->second;
      if (retValues.size() > 1)
        {
          // return as an array
          PLEXIL::StoredArray retArray(retValues.size(), retValues);
          dest[0] = retArray.getKey();
        }
      else
        {
          dest[0] = retValues[0];
        }
      m_LookupMap.erase(m_LookupMapIter);
    }
  else
    {
      // The state is not available at this time
      debugMsg("RoboSimInterfaceAdaptor", "State not available for " << nStr);
      dest[0] = PLEXIL::Expression::UNKNOWN();
    }
}
  
void RoboSimInterfaceAdaptor::executeCommand(const PLEXIL::LabelStr& name,
                                             const std::list<double>& args,
                                             PLEXIL::ExpressionId dest,
                                             PLEXIL::ExpressionId ack)
{
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdaptor", "Received executeCommand for " 
           << nStr);
  bool failed = false;
  if (nStr == "Move")
    {
      int direction = static_cast<int>(*(args.begin()));
      if ((direction >= 0) && (direction < static_cast<int>(m_MoveDirectionVector.size())))
        nStr = m_MoveDirectionVector[direction];
      else
        {
          debugMsg("RoboSimInterfaceAdaptor", "The specified move direction is not "
                   << "valid. The command will fail");
          failed = true;
        }
    }

  if (m_Connected && !failed) 
    {  
      m_CommandMap[nStr] = dest;
      m_SSWGClient.sendMessage(nStr, "RobotYellow");
    }

  m_execInterface.handleValueChange
    (ack, (!m_Connected || failed) ? PLEXIL::CommandHandleVariable::COMMAND_FAILED() :
     PLEXIL::CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
  m_execInterface.notifyOfExternalEvent();
}

void RoboSimInterfaceAdaptor::executeFunctionCall(const PLEXIL::LabelStr& name,
                                                  const std::list<double>& args,
                                                  PLEXIL::ExpressionId dest,
                                                  PLEXIL::ExpressionId ack)
{
  std::string nStr = name.toString();
  debugMsg("RoboSimInterfaceAdaptor", "Received functionCall for " 
           << nStr);
  std::list<double>::const_iterator argIter = args.begin();

  double minVal = (*argIter);
  ++argIter;
  double  maxVal = (*argIter);
  int res = static_cast<int>((minVal + (maxVal - minVal) * 
                              (static_cast<double>(rand()) / 
                               static_cast<double>(RAND_MAX))) + 0.5);
  
  m_execInterface.handleValueChange
    (ack, PLEXIL::BooleanVariable::TRUE());
  m_execInterface.handleValueChange (dest, res);
  m_execInterface.notifyOfExternalEvent();
}

void RoboSimInterfaceAdaptor::receivedMessage (const std::string& sender,
                                               const std::string& msg)
{
  debugMsg("RoboSimInterfaceAdaptor", "got something back from " << sender 
           << " who says: " << msg);

  std::string cmdName;
  std::vector<double> retValues = extractMessageContents(cmdName, msg);

  if ((m_CommandMapIter = m_CommandMap.find(cmdName)) != m_CommandMap.end())
    {
      if (retValues.size() > 1)
        {
          // return as an array
          PLEXIL::StoredArray retArray(retValues.size(), retValues);
          m_execInterface.handleValueChange
            (m_CommandMapIter->second, retArray.getKey());
        }
      else
        {
          m_execInterface.handleValueChange
            (m_CommandMapIter->second, retValues[0]);
        }
      m_CommandMap.erase(m_CommandMapIter);
    }
  else 
    {
      debugMsg("RoboSimInterfaceAdaptor" , "Received an unsolicited reply for " 
               << cmdName << ". Ignoring it.");
      return;
    }
  
  m_execInterface.notifyOfExternalEvent();
}

std::vector<double>
RoboSimInterfaceAdaptor::extractMessageContents(std::string& cmdName,
                                                const std::string& msg)
{
  std::vector<double> retValues;
  std::string delimiter(",");
  std::string::size_type lastPos = msg.find_first_not_of(delimiter, 0);
  std::string::size_type pos = msg.find_first_of(delimiter, lastPos);
  cmdName = msg.substr(lastPos, pos - lastPos);
  bool done = false;
  while(!done)
    {
      lastPos = msg.find_first_not_of(delimiter, pos);
      pos = msg.find_first_of(delimiter, lastPos);
      if ((std::string::npos == pos) && (std::string::npos == lastPos))
        {
          done = true;
        }
      else
        {
          retValues.push_back(atof(msg.substr(lastPos, pos - lastPos).c_str()));
        }
    }
  debugMsg("RoboSimInterfaceAdaptor", "Extracting the following values for "
           << cmdName << ", retValue size: " << retValues.size());
  for (unsigned int i = 0; i < retValues.size(); ++i)
    debugMsg("RoboSimInterfaceAdaptor", retValues[i]);

  return retValues;
}
