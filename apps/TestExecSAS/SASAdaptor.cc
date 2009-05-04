#include "SASAdaptor.hh"
#include <iostream>
#include <vector>
#include <math.h>

#include <Expression.hh>
#include <PlexilExec.hh>
#include <Node.hh>
#include <Debug.hh>
#include <LabelStr.hh>
#include <CoreExpressions.hh>
#include <AdaptorExecInterface.hh>
#include <StateCache.hh>
#include "ThreadSpawn.hh"
#include "genericCommand.h"
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

  //TODO: clean up all the caches commands and states in maps.
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
      genericCommand data;
      data.name = const_cast<char *>(cmdName.toString().c_str());
      //      driveCommand_publish(m_lcm, "DRIVECOMMAND", NULL);
      genericCommand_publish(m_lcm, "GENERICCOMMAND", &data);
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

      //      if (m_StateToChangeLookupMap.find(n) == m_StateToChangeLookupMap.end())
      //  m_StateToValueMap.erase(iter);
    }
}

void SASAdaptor::registerChangeLookup(const PLEXIL::LookupKey& uniqueId,
                                      const PLEXIL::StateKey& stateKey,
                                      const std::vector<double>& tolerances)
{
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string n = name.toString();

  debugMsg("SASAdaptor:registerChangeLookup", "In change look up for " << n);

  std::map<std::string, ChangeLookupStruct>::iterator iter;

  if ((iter = m_StateToChangeLookupMap.find(n)) == m_StateToChangeLookupMap.end())
    {
      debugMsg("SASAdaptor:registerChangeLookup", "The state " << n 
               << " has not already been registered for change lookup. Processing the new request");

      std::map<std::string, std::vector<double> >::iterator iter2;
      if ((iter2 = m_StateToValueMap.find(n)) != m_StateToValueMap.end())
        {
          debugMsg("SASAdaptor:registerChangeLookup", "The newly registered state " << n 
               << " has a known telemetry value. Storing it.");
          
          m_StateToChangeLookupMap.insert(std::pair<std::string, 
                                          ChangeLookupStruct>(n, ChangeLookupStruct(stateKey, 
                                                                                    iter2->second, 
                                                                                    tolerances)));
        }
      else
        debugMsg("SASAdaptor:registerChangeLookup", "The newly registered state " << n 
                 << " does not have a known telemetry value yet.");
    }
  else
    {
      debugMsg("SASAdaptor:registerChangeLookup", "The state " << n 
               << " has already been registered for change lookup. Ignoring the new request");
    }
}

void SASAdaptor::unregisterChangeLookup(const PLEXIL::LookupKey& uniqueId)
{
  debugMsg("SASAdaptor:unregisterChangeLookup", "In unregister change look up");
  // TODO: What exactly needs to be done here?
}


void SASAdaptor::postCommandResponse(const std::string& cmd, float value)
{
  debugMsg("SASAdaptor:postCommandResponse", "Received a reponse for " << cmd);
  std::map<std::string, PLEXIL::ExpressionId>::iterator iter;
  
  if ((iter = m_CommandToExpIdMap.find(cmd)) != m_CommandToExpIdMap.end())
    {
      //PLEXIL::LabelStr(value).getKey()
      m_execInterface.handleValueChange(iter->second, value);
      m_CommandToExpIdMap.erase(iter);
    }
  m_execInterface.notifyOfExternalEvent();
}

void SASAdaptor::postTelemetryState(const std::string& state, unsigned int numOfValues,
                                    const double* values)
{
  debugMsg("SASAdaptor::postTelemetryState", "Received telemetry for " << state);
  std::vector<double> vect;
  for (unsigned int i = 0; i < numOfValues; ++i)
    vect.push_back(values[i]);

  m_StateToValueMap[state] = vect;
  bool changed = false;

  std::map<std::string, ChangeLookupStruct>::iterator iter;
  if ((iter = m_StateToChangeLookupMap.find(state)) != m_StateToChangeLookupMap.end())
    {
      debugMsg("SASAdaptor:postTelemetryState", "The state " << state 
               << " has received a new telemetry value. Checking against the previous value.");
      const std::vector<double>& prev = iter->second.getPreviousValues();

      const std::vector<double>& tolerance = iter->second.getToleranceValues();

      if (prev.size() != 0)
        {
          // Check if values have changed
          assertTrueMsg((numOfValues == prev.size()),
                        "SASAdaptor:postTelemetryState: Posted telemetry for state " << state 
                        << " is not the same length as expected");

          for (unsigned int i = 0; (i < numOfValues) && !changed; ++i)
            {
              debugMsg("SASAdaptor:postTelemetryState", 
                       "prev: " << prev[i] << ", values: " << values[i] 
                       << ", tolerance: " << tolerance[i]);
              if (fabs(prev[i] - values[i]) >= tolerance[i])
                changed = true;
            }

          iter->second.setPreviousValues(vect);
        }
      else
        debugMsg("SASAdaptor:postTelemetryState", "Not a known previous value to compute a change.");
    }

  if (changed)
    {
      debugMsg("SASAdaptor::postTelemetryState", "The state has changed. Posting value");
      m_execInterface.handleValueChange(iter->second.getStateKey(), vect);
      m_execInterface.notifyOfExternalEvent();
    }
}




