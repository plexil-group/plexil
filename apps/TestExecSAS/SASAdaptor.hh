#ifndef SAS_ADAPTOR_HH
#define SAS_ADAPTOR_HH

#include "ExecDefs.hh"
#include "InterfaceAdaptor.hh"
#include <pthread.h>
#include <map>
#include <lcm/lcm.h>

class LcmBaseImplSASExec;

struct ChangeLookupStruct
{
  ChangeLookupStruct(const PLEXIL::StateKey& _stateKey,
                     std::vector<double>& _prevValues,
                     const std::vector<double>& _tolerances)
    : stateKey(_stateKey), prevValues(_prevValues),
      tolerances(_tolerances) 
  {
  }
  ~ChangeLookupStruct(){}

  const PLEXIL::StateKey& getStateKey() const {return stateKey;}

  void setPreviousValues(std::vector<double>& _prevValues) 
  { 
    prevValues = _prevValues;
  }

  const std::vector<double>& getPreviousValues() const {return prevValues;}
  const std::vector<double>& getToleranceValues() const {return tolerances;}

  const PLEXIL::StateKey stateKey;
  std::vector<double> prevValues;
  const std::vector<double> tolerances;

};

class SASAdaptor : public PLEXIL::InterfaceAdaptor
{

public:
  SASAdaptor(PLEXIL::AdaptorExecInterface& execInterface);
  virtual ~SASAdaptor();

  virtual void lookupNow(const PLEXIL::StateKey& stateKey,
                         std::vector<double>& dest);

  virtual void registerChangeLookup(const PLEXIL::LookupKey& uniqueId,
                                    const PLEXIL::StateKey& stateKey,
                                    const std::vector<double>& tolerances);

  virtual void unregisterChangeLookup(const PLEXIL::LookupKey& uniqueId);

  virtual void executeCommand(const PLEXIL::LabelStr& name, const std::list<double>& args,
                              PLEXIL::ExpressionId dest, PLEXIL::ExpressionId ack);
  lcm_t* getLCM() {return m_lcm;}
  
  void postCommandResponse(const std::string& cmd, float value);
  void postTelemetryState(const std::string& cmd, unsigned int numOfValues,
                          const double* values);

private:
  lcm_t *m_lcm;  
  LcmBaseImplSASExec *m_lcmSASExec;
  pthread_t m_ThreadId;

  std::map<std::string, PLEXIL::ExpressionId> m_CommandToExpIdMap;
  std::map<std::string, std::vector<double> > m_StateToValueMap;
  std::map<std::string, ChangeLookupStruct> m_StateToChangeLookupMap;
  
};

#endif
