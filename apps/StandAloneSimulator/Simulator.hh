#ifndef SIMULATOR_HH
#define SIMULATOR_HH

#include <map>
#include "TimingService.hh"
#include "SimulatorScriptReader.hh"

class ResponseMessageManager;
class ResponseMessage;
class ResponseFactory;
class CommRelayBase;

#define CONVERT_TIMESPEC_TO_DOUBLE(t) static_cast<double>(t.tv_sec) + \
                                      static_cast<double>(t.tv_usec) / 1000000.0


class Simulator
{
public:
  Simulator(ResponseFactory* respFactory, 
            CommRelayBase* commRelay);
  ~Simulator();

  ResponseMessageManager* getResponseMessageManager(const std::string& cmdName) const;
  void registerResponseMessageManager(ResponseMessageManager* msgMgr);
  void handleWakeUp();
  bool readScript(const std::string& fName);
  ResponseFactory* getResponseFactory() const {return m_ResponseFactory;}
  
  void scheduleResponseForCommand(const std::string& command, int uniqueId);
private:
  Simulator(){};
  void sendResponse(const ResponseMessage* respMsg);
  
  
  std::map<const std::string, ResponseMessageManager*> m_CmdToRespMgr;
  std::multimap<timeval, ResponseMessage*> m_TimeToResp;

  ResponseFactory* m_ResponseFactory;
  CommRelayBase* m_CommRelay;
  TimingService m_TimingService;
  SimulatorScriptReader m_SimulatorScriptReader;
  pthread_mutex_t m_TimerMutex;
};

#endif // SIMULATOR_HH
