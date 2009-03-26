#include "SASAdaptor.hh"
#include <iostream>
#include <vector>

#include <Expression.hh>
#include <PlexilExec.hh>
#include <Node.hh>
#include <Debug.hh>
#include <LabelStr.hh>
#include <CoreExpressions.hh>
#include <AdaptorExecInterface.hh>
#include <StateCache.hh>
#include "ThreadSpawn.hh"
#include "driveCommand.h"
#include "LcmBaseImplSASExec.hh"

void spawnThreadForEachClient(void* args)
{

  std::cout << "Spawning the listening loop." << std::endl;
  SASAdaptor* server = static_cast<SASAdaptor*>(args);
  
  while (1)
    {
      lcm_handle(server->getLCM());
      usleep(500000);
    }
}

SASAdaptor::SASAdaptor(PLEXIL::AdaptorExecInterface& execInterface) : 
  InterfaceAdaptor(execInterface), m_lcm(NULL)
{
  m_lcm = lcm_create("udpm://");
  m_lcmSASExec = new LcmBaseImplSASExec(m_lcm, this);
  if (!m_lcm) 
    {
      debugMsg("SASAdaptor:SASAdaptor", "Unable to create lcm.");
    }
  else
    {
      debugMsg("SASAdaptor:SASAdaptor", "Successfully created lcm.");
    }

  if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForEachClient, (void *)this,
                  m_ThreadId) != true)
    {
      std::cout << "Error spawing thread for the receiving socket." << std::endl;
    }
  else
    {
      ;
    }

}

SASAdaptor::~SASAdaptor()
{
  delete m_lcmSASExec;
  lcm_destroy(m_lcm);
  std::cout << "Cancelling thread ...";
  pthread_cancel(m_ThreadId);
  pthread_join(m_ThreadId, NULL);
  std::cout << "done" << std::endl;

}

void SASAdaptor::executeCommand(const PLEXIL::LabelStr& name,
                                const std::list<double>& args,
                                PLEXIL::ExpressionId dest, 
                                PLEXIL::ExpressionId ack)
{
  const PLEXIL::LabelStr& cmdName = name;
  
  if (m_lcm != NULL)
    {
      debugMsg("SASAdaptor:executeCommand", 
               "Sending the following command to the stand alone simulator: "
               << cmdName.toString());
      driveCommand_publish(m_lcm, "DRIVECOMMAND", NULL);
    }
  else
    {
      debugMsg("SASAdaptor:executeCommand", "m_lcm is NULL. Unable to post command.");
    }

  m_execInterface.handleValueChange
    (ack, PLEXIL::CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
  m_execInterface.notifyOfExternalEvent();

  m_CommandToExpIdMap[cmdName.toString()] = dest;
}

void SASAdaptor::lookupNow(const PLEXIL::StateKey& stateKey,
                           std::vector<double>& dest)
{
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string n = name.toString();
  debugMsg("SASAdaptor:lookupNow", "Looking up state: " << n);

  double returnValue = 0.0;
  dest.resize(1);
  
  std::map<std::string, std::vector<double> >::iterator iter;
  if (n == "time")
    {
      returnValue = 0.0;
      dest[0] = returnValue;
    }
  else if ((iter = m_StateToValueMap.find(n)) != m_StateToValueMap.end())
    {

      debugMsg("SASAdaptor:lookupNow", "Found a cached state");
      assert(dest.size() == iter->second.size());
      dest = iter->second;
      m_StateToValueMap.erase(iter);
    }
}

void SASAdaptor::postCommandResponse(const std::string& cmd,
                                     float value)
{
  std::map<std::string, PLEXIL::ExpressionId>::iterator iter;
  
  if ((iter = m_CommandToExpIdMap.find(cmd)) != m_CommandToExpIdMap.end())
    {
      //PLEXIL::LabelStr(value).getKey()
      m_execInterface.handleValueChange(iter->second, value);
      m_CommandToExpIdMap.erase(iter);
    }
  m_execInterface.notifyOfExternalEvent();
}

void SASAdaptor::postTelemetryState(const std::string& state, int numOfValues,
                                    const double* values)
{
  std::vector<double> vect;
  for (int i = 0; i < numOfValues; ++i)
    vect.push_back(values[i]);

  m_StateToValueMap[state] = vect;
}




