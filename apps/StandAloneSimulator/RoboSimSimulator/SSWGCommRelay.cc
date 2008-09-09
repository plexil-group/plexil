#include "SSWGCommRelay.hh"
#include "Simulator.hh"
#include "ResponseMessage.hh"

SSWGCommRelay::SSWGCommRelay(const std::string& name,
                             const std::string& ipAddress, int portNumber)
  : CommRelayBase(name), m_UniqueId(0)
{
  m_Connected = m_SSWGClient.connectToServer(name, ipAddress, 
                                             portNumber, this);
  if (!m_Connected) std::cerr << "The executive is unable to connect to the "
                              << "communication server" << std::endl;
  else
    std::cout << "Successfully connected to server." << std::endl;
}

SSWGCommRelay::~SSWGCommRelay()
{
}

void SSWGCommRelay::receivedMessage (const std::string& sender,
                                     const std::string& msg)
{
  std::cout << "\n\nSSWGCommRelay:: got something back from " << sender
            << " who says: " << msg << std::endl;
  
  m_UniqueIdToSender[m_UniqueId] = sender;
  m_UniqueIdToCommand[m_UniqueId] = msg;
  m_Simulator->scheduleResponseForCommand(msg, m_UniqueId);
  ++m_UniqueId;
}

void SSWGCommRelay::sendResponse(const ResponseMessage* respMsg)
{
  std::cout << "Sending message: " << respMsg->contents
            << std::endl;
  
  if (m_Connected) 
    m_SSWGClient.sendMessage(m_UniqueIdToCommand[respMsg->id] + "," + respMsg->contents, 
                             m_UniqueIdToSender[respMsg->id]);
}
