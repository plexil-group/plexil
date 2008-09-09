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
