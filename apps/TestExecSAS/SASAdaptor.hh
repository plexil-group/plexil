#ifndef SAS_ADAPTOR_HH
#define SAS_ADAPTOR_HH

#include "ExecDefs.hh"
#include "InterfaceAdaptor.hh"
#include <pthread.h>
#include <map>
#include <lcm/lcm.h>

class LcmBaseImplSASExec;

class SASAdaptor : public PLEXIL::InterfaceAdaptor
{

public:
  SASAdaptor(PLEXIL::AdaptorExecInterface& execInterface);
  virtual ~SASAdaptor();

  virtual void lookupNow(const PLEXIL::StateKey& stateKey,
                         std::vector<double>& dest);

  virtual void executeCommand(const PLEXIL::LabelStr& name, const std::list<double>& args,
                              PLEXIL::ExpressionId dest, PLEXIL::ExpressionId ack);
  lcm_t* getLCM() {return m_lcm;}
  
  void postCommandResponse(const std::string& cmd, float value);
  void postTelemetryState(const std::string& cmd, int numOfValues,
                          const double* values);

private:
  lcm_t *m_lcm;  
  LcmBaseImplSASExec *m_lcmSASExec;
  pthread_t m_ThreadId;

  std::map<std::string, PLEXIL::ExpressionId> m_CommandToExpIdMap;
  std::map<std::string, std::vector<double> > m_StateToValueMap;
  
};

#endif
