/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "CommandResponseManager.hh"
#include "ResponseMessage.hh"
#include "GenericResponse.hh"

#include "Debug.hh"
#include "Error.hh"

CommandResponseManager::CommandResponseManager(const std::string& id)
  : m_Identifier(id), 
    m_Counter(1)
{
}

CommandResponseManager::~CommandResponseManager()
{
  for (std::map<int, const GenericResponse*>::iterator iter = m_CmdIdToResponse.begin();
       iter != m_CmdIdToResponse.end(); 
       ++iter)
    delete iter->second;
}

const std::string &CommandResponseManager::getIdentifier() const
{
  return m_Identifier;
}

const GenericResponse *CommandResponseManager::getDefaultResponse()
{
  return m_DefaultResponse;
}

void CommandResponseManager::addResponse(GenericResponse* resp, int cmdIndex)
{
  // Make sure the command index had not been specified before.
  if (m_CmdIdToResponse.find(cmdIndex) != m_CmdIdToResponse.end())
    {
      debugMsg("CommandResponseManager:addResponse",
           "Warning for " << m_Identifier
                << ": Command index " << cmdIndex
           << " has been repeated. Ignoring it.");
      delete resp;
      return;
    }

  if (cmdIndex == 0) {
    m_DefaultResponse = resp;
  }
  else
    m_CmdIdToResponse[cmdIndex] = resp;
}

const GenericResponse* CommandResponseManager::getResponses(timeval& tDelay)
{
  IndexResponseMap::iterator iter;
  const GenericResponse* respBase;
  if ((iter = m_CmdIdToResponse.find(m_Counter)) == m_CmdIdToResponse.end()) {
    debugMsg("CommandResponseManager:getResponses",
             " for " << m_Identifier << ": Getting default response");
    respBase = m_DefaultResponse;
  }
  else {
    debugMsg("CommandResponseManager:getResponses",
             " for " << m_Identifier << ": Using response for index " << m_Counter);
    respBase = iter->second;
  }
  debugMsg("CommandResponseManager:getResponses",
           " " << m_Identifier << ", count: " << m_Counter);
  ++m_Counter;

  // This shouldn't happen, but check anyway just in case
  assertTrueMsg(respBase != NULL,
                "CommandResponseManager::getResponses: Internal error: No response found for \""
                << m_Identifier << "\"");

  if (respBase->numberOfResponses > 0) {
    tDelay = respBase->delay;
    return respBase;
  }
  return NULL;
}
