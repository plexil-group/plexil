#ifndef COMM_RELAY_BASE_HH
#define COMM_RELAY_BASE_HH

#include <string>
class ResponseMessage;
class Simulator;

class CommRelayBase
{
public:
  CommRelayBase(const std::string& id) : m_Identifier(id) {}
  virtual ~CommRelayBase(){}
  void registerSimulator(Simulator* sim)
  {
    m_Simulator = sim;
  }
  virtual void sendResponse(const ResponseMessage* respMsg) = 0;

protected:
  const std::string m_Identifier;
  Simulator* m_Simulator;
};

#endif // COMM_RELAY_BASE_HH
