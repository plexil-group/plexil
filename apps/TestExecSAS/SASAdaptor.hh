#ifndef SAS_ADAPTOR_HH
#define SAS_ADAPTOR_HH

#include "ExecDefs.hh"
#include "InterfaceAdaptor.hh"
#include <pthread.h>
#include <map>

class ClientSocket;
class ServerSocket;

class SASAdaptor : public PLEXIL::InterfaceAdaptor
{

public:
  SASAdaptor(PLEXIL::AdaptorExecInterface& execInterface, const std::string& host="localhost", int sendingPort=6165,
                      int receivingPort=6166);
  virtual ~SASAdaptor();

  virtual void lookupNow(const PLEXIL::StateKey& stateKey,
                         std::vector<double>& dest);

  virtual void executeCommand(const PLEXIL::LabelStr& name, const std::list<double>& args,
                              PLEXIL::ExpressionId dest, PLEXIL::ExpressionId ack);
  void readMessage(const std::string& msg);
  int getListeningPortNumber() const {return m_ListeningPort;}

private:
  ClientSocket* m_ClientSocket;
  pthread_t m_ThreadId;
  int m_ListeningPort;
  
  std::map<std::string, PLEXIL::ExpressionId> m_CommandToExpIdMap;
  std::map<std::string, int> m_StateToValueMap;
};

#endif
