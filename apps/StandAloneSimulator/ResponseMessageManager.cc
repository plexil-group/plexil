#include "ResponseMessageManager.hh"
#include "ResponseMessage.hh"
#include "ResponseBase.hh"
#include <iostream>

ResponseMessageManager::ResponseMessageManager(const std::string& id)
  : m_Identifier(id), m_Counter(0)
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
      respBase = m_DefaultResponse;
    }
  else
    {
      respBase = iter->second;
    }

  std::cout << "ResponseMessageManager:getResponseMessages: " << m_Identifier
            << ", count: " << m_Counter << std::endl;
  tDelay = respBase->getDelay();
  ++m_Counter;
  return respBase->createResponseMessage();
}
