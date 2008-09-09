#ifndef RESPONSE_MESSAGE_MANAGER_HH
#define RESPONSE_MESSAGE_MANAGER_HH

#include <string>
#include <map>

class ResponseBase;
class ResponseMessage;

class ResponseMessageManager
{
public:
  ResponseMessageManager(const std::string& id);
  ~ResponseMessageManager();

  const std::string& getIdentifier() const {return m_Identifier;}
  void addResponse(int cmdIndex, ResponseBase* resp);
  ResponseMessage* getResponseMessages(timeval& tDelay);

private:
  const std::string m_Identifier;
  int m_Counter;
  std::map<int, ResponseBase*> m_CmdIdToResponse;
  ResponseBase* m_DefaultResponse;
};
#endif // RESPONSE_MESSAGE_MANAGER_HH
