/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include <memory>

class CommandResponseManagerImpl final : public CommandResponseManager
{
private:
  // Local typedef
  using IndexResponseMap = std::map<int, std::unique_ptr<GenericResponse> >;

  // Member variables
  IndexResponseMap m_CmdIdToResponse;
  const std::string m_Identifier;
  std::unique_ptr<GenericResponse> m_DefaultResponse;
  int m_Counter;

public:
  CommandResponseManagerImpl(const std::string& id)
    : CommandResponseManager(),
      m_Identifier(id), 
      m_DefaultResponse(),
      m_Counter(1)
  {
  }

  virtual ~CommandResponseManagerImpl() = default;

  const std::string &getIdentifier() const
  {
    return m_Identifier;
  }

  const GenericResponse *getDefaultResponse()
  {
    return m_DefaultResponse.get();
  }

  void addResponse(GenericResponse* resp, int cmdIndex)
  {
    // Make sure the command index had not been specified before.
    if (m_CmdIdToResponse.find(cmdIndex) != m_CmdIdToResponse.end()) {
        warn("CommandResponseManager::addResponse "
             << m_Identifier << ": Command index " << cmdIndex
             << " has been repeated. Ignoring it.");
        delete resp;
        return;
      }

    if (cmdIndex == 0) 
      m_DefaultResponse.reset(resp);
    else
      m_CmdIdToResponse.emplace(cmdIndex, resp);
  }

  const GenericResponse* getResponses(timeval& tDelay)
  {
    IndexResponseMap::iterator iter;
    const GenericResponse* respBase;
    if ((iter = m_CmdIdToResponse.find(m_Counter)) == m_CmdIdToResponse.end()) {
      debugMsg("CommandResponseManager:getResponses",
               " for " << m_Identifier << ": Getting default response");
      respBase = m_DefaultResponse.get();
    }
    else {
      debugMsg("CommandResponseManager:getResponses",
               " for " << m_Identifier << ": Using response for index " << m_Counter);
      respBase = iter->second.get();
    }
    debugMsg("CommandResponseManager:getResponses",
             " " << m_Identifier << ", count: " << m_Counter);
    ++m_Counter;

    // This shouldn't happen, but check anyway just in case
    assertTrueMsg(respBase != nullptr,
                  "getResponses: Internal error: No response found for \""
                  << m_Identifier << "\"");

    if (respBase->numberOfResponses > 0) {
      tDelay = respBase->delay;
      return respBase;
    }
    return nullptr;
  }

}; // class CommandResponseManagerImpl

CommandResponseManager *makeCommandResponseManager(std::string const &name)
{
  return new CommandResponseManagerImpl(name);
}
