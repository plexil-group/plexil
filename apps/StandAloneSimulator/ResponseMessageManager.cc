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
#include "ResponseMessageManager.hh"
#include "ResponseMessage.hh"
#include "ResponseBase.hh"
#include <iostream>

ResponseMessageManager::ResponseMessageManager(const std::string& id)
  : m_Identifier(id), m_Counter(1)
{
}

ResponseMessageManager::~ResponseMessageManager()
{
  std::cout << "~ResponseMessageManager of " << m_Identifier << std::endl;
  for(std::map<int, ResponseBase*>::iterator iter = m_CmdIdToResponse.begin();
      iter != m_CmdIdToResponse.end(); ++iter)
    delete iter->second;
}

void ResponseMessageManager::addResponse(int cmdIndex, ResponseBase* resp)
{
  // Make sure the command index had not been specified before.
  if (m_CmdIdToResponse.find(cmdIndex) == m_CmdIdToResponse.end())
    {
      m_CmdIdToResponse[cmdIndex] = resp;
    }
  else
    {
      std::cerr << "Warning: The command index entry has been repeated. Ignoring it."
                << std::endl;
      return;
    }

  if (cmdIndex == 0)
    m_DefaultResponse = resp;
  else
    m_CmdIdToResponse[cmdIndex] = resp;
}

ResponseMessage* ResponseMessageManager::getResponseMessages(timeval& tDelay)
{
  std::map<int, ResponseBase*>::iterator iter;
  ResponseBase* respBase;
  if ((iter = m_CmdIdToResponse.find(m_Counter)) == m_CmdIdToResponse.end())
    {
      std::cout << "ResponseMessageManager:getResponseMessages: " << "Using default." << std::endl;
  
      respBase = m_DefaultResponse;
    }
  else
    {
      std::cout << "ResponseMessageManager:getResponseMessages: " << "Using specific." << std::endl;
      respBase = iter->second;
    }

  std::cout << "ResponseMessageManager:getResponseMessages: " << m_Identifier
            << ", count: " << m_Counter << std::endl;
  tDelay = respBase->getDelay();
  ++m_Counter;

  return respBase->createResponseMessage();
}
