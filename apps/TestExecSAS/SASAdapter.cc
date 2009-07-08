/* Copyright (c) 2006-2009, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SASAdapter.hh"
#include <iostream>
#include <vector>
#include <math.h>

#include <Expression.hh>
#include <PlexilExec.hh>
#include <Node.hh>
#include <Debug.hh>
#include <LabelStr.hh>
#include <CoreExpressions.hh>
#include <AdapterExecInterface.hh>
#include <StateCache.hh>
#include <tinyxml.h>
#include "ThreadSpawn.hh"
#include "genericCommand.h"

void spawnThreadForEachClient(void* args)
{
  std::cout << "Spawning the listening loop." << std::endl;
  SASAdapter* server = static_cast<SASAdapter*>(args);
  
  while (1)
    {
      lcm_handle(server->getLCM());
      usleep(500000);
    }
}

SASAdapter::SASAdapter(PLEXIL::AdapterExecInterface& execInterface) : 
  InterfaceAdapter(execInterface), m_lcm(NULL)
{
}

SASAdapter::SASAdapter(PLEXIL::AdapterExecInterface& execInterface,
		       const TiXmlElement * xml) : 
  InterfaceAdapter(execInterface, xml), m_lcm(NULL)
{
}

SASAdapter::~SASAdapter()
{
  shutdown();
}

/**
 * @brief Initializes the adapter, possibly using its configuration data.
 * @return true if successful, false otherwise.
 */
bool SASAdapter::initialize()
{
  // Default value for LCM provider
  const char* lcm_provider_spec = "udpm://";

  // Get provider from XML, if supplied
  const TiXmlElement* xml = this->getXml();
  if (this->getXml() != NULL) 
    {
      const char* provider = xml->Attribute("Provider");
      if (provider != NULL)
	{
	  lcm_provider_spec = provider;
	}
    }

  // Construct LCM
  debugMsg("SASAdapter:initialize", 
	   " Using LCM provider " << lcm_provider_spec);
  m_lcm = lcm_create(lcm_provider_spec);
  if (!m_lcm) 
    {
      debugMsg("SASAdapter:initialize", " Unable to create lcm.");
      return false;
    }
  debugMsg("SASAdapter:initialize", " Successfully created lcm.");

  m_lcmSASExec = new LcmBaseImplSASExec(m_lcm, this);
  if (threadSpawn((THREAD_FUNC_PTR)spawnThreadForEachClient, (void *)this,
                  m_ThreadId) != true)
    {
      std::cout << "Error spawing thread for the receiving socket." << std::endl;
      return false;
    }
  m_execInterface.defaultRegisterAdapter(getId());
  return true;
}

/**
 * @brief Starts the adapter, possibly using its configuration data.  
 * @return true if successful, false otherwise.
 */
bool SASAdapter::start()
{
  return true;
}

/**
 * @brief Stops the adapter.  
 * @return true if successful, false otherwise.
 */
bool SASAdapter::stop()
{
  return true;
}


/**
 * @brief Resets the adapter.  
 * @return true if successful, false otherwise.
 */
bool SASAdapter::reset()
{
  return true;
}

/**
 * @brief Shuts down the adapter, releasing any of its resources.
 * @return true if successful, false otherwise.
 */
bool SASAdapter::shutdown()
{
  debugMsg("SASAdapter:shutdown", " Deleting LCM SAS exec");
  delete m_lcmSASExec;
  m_lcmSASExec = NULL;
  if (m_lcm != NULL)
    {
      debugMsg("SASAdapter:shutdown", " Destroying LCM");
      lcm_destroy(m_lcm);
      m_lcm = NULL;
      std::cout << "Cancelling thread ...";
      pthread_cancel(m_ThreadId);
      pthread_join(m_ThreadId, NULL);
      std::cout << "done" << std::endl;
    }
  //TODO: clean up all the caches commands and states in maps.
  return true;
}

void SASAdapter::executeCommand(const PLEXIL::LabelStr& name,
                                const std::list<double>& args,
                                PLEXIL::ExpressionId dest, 
                                PLEXIL::ExpressionId ack)
{
  const PLEXIL::LabelStr& cmdName = name;
  
  if (m_lcm != NULL)
    {
      debugMsg("SASAdapter:executeCommand", 
               "Sending the following command to the stand alone simulator: "
               << cmdName.toString());
      genericCommand data;
      data.name = const_cast<char *>(cmdName.toString().c_str());
      //      driveCommand_publish(m_lcm, "DRIVECOMMAND", NULL);
      genericCommand_publish(m_lcm, "GENERICCOMMAND", &data);
    }
  else
    {
      debugMsg("SASAdapter:executeCommand", "m_lcm is NULL. Unable to post command.");
    }

  m_execInterface.handleValueChange
    (ack, PLEXIL::CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
  m_execInterface.notifyOfExternalEvent();

  m_CommandToExpIdMap[cmdName.toString()] = dest;
}

void SASAdapter::lookupNow(const PLEXIL::StateKey& stateKey,
                           std::vector<double>& dest)
{
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string n = name.toString();
  debugMsg("SASAdapter:lookupNow", "Looking up state: " << n);

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

      debugMsg("SASAdapter:lookupNow", "Found a cached state");
      assert(dest.size() == iter->second.size());
      dest = iter->second;

      //      if (m_StateToChangeLookupMap.find(n) == m_StateToChangeLookupMap.end())
      //  m_StateToValueMap.erase(iter);
    }
}

void SASAdapter::registerChangeLookup(const PLEXIL::LookupKey& uniqueId,
                                      const PLEXIL::StateKey& stateKey,
                                      const std::vector<double>& tolerances)
{
  PLEXIL::State state;
  this->getState(stateKey, state);
  const PLEXIL::LabelStr& name = state.first;
  std::string n = name.toString();

  debugMsg("SASAdapter:registerChangeLookup", "In change look up for " << n);

  std::map<std::string, ChangeLookupStruct>::iterator iter;

  if ((iter = m_StateToChangeLookupMap.find(n)) == m_StateToChangeLookupMap.end())
    {
      debugMsg("SASAdapter:registerChangeLookup", "The state " << n 
               << " has not already been registered for change lookup. Processing the new request");

      std::map<std::string, std::vector<double> >::iterator iter2;
      if ((iter2 = m_StateToValueMap.find(n)) != m_StateToValueMap.end())
        {
          debugMsg("SASAdapter:registerChangeLookup", "The newly registered state " << n 
               << " has a known telemetry value. Storing it.");
          
          m_StateToChangeLookupMap.insert(std::pair<std::string, 
                                          ChangeLookupStruct>(n, ChangeLookupStruct(stateKey, 
                                                                                    iter2->second, 
                                                                                    tolerances)));
        }
      else
        debugMsg("SASAdapter:registerChangeLookup", "The newly registered state " << n 
                 << " does not have a known telemetry value yet.");
    }
  else
    {
      debugMsg("SASAdapter:registerChangeLookup", "The state " << n 
               << " has already been registered for change lookup. Ignoring the new request");
    }
}

void SASAdapter::unregisterChangeLookup(const PLEXIL::LookupKey& uniqueId)
{
  debugMsg("SASAdapter:unregisterChangeLookup", "In unregister change look up");
  // TODO: What exactly needs to be done here?
}


void SASAdapter::postCommandResponse(const std::string& cmd, float value)
{
  debugMsg("SASAdapter:postCommandResponse", "Received a reponse for " << cmd);
  std::map<std::string, PLEXIL::ExpressionId>::iterator iter;
  
  if ((iter = m_CommandToExpIdMap.find(cmd)) != m_CommandToExpIdMap.end())
    {
      //PLEXIL::LabelStr(value).getKey()
      m_execInterface.handleValueChange(iter->second, value);
      m_CommandToExpIdMap.erase(iter);
    }
  m_execInterface.notifyOfExternalEvent();
}

void SASAdapter::postTelemetryState(const std::string& state, unsigned int numOfValues,
                                    const double* values)
{
  debugMsg("SASAdapter::postTelemetryState", "Received telemetry for " << state);
  std::vector<double> vect;
  for (unsigned int i = 0; i < numOfValues; ++i)
    vect.push_back(values[i]);

  m_StateToValueMap[state] = vect;
  bool changed = false;

  std::map<std::string, ChangeLookupStruct>::iterator iter;
  if ((iter = m_StateToChangeLookupMap.find(state)) != m_StateToChangeLookupMap.end())
    {
      debugMsg("SASAdapter:postTelemetryState", "The state " << state 
               << " has received a new telemetry value. Checking against the previous value.");
      const std::vector<double>& prev = iter->second.getPreviousValues();

      const std::vector<double>& tolerance = iter->second.getToleranceValues();

      if (prev.size() != 0)
        {
          // Check if values have changed
          assertTrueMsg((numOfValues == prev.size()),
                        "SASAdapter:postTelemetryState: Posted telemetry for state " << state 
                        << " is not the same length as expected");

          for (unsigned int i = 0; (i < numOfValues) && !changed; ++i)
            {
              debugMsg("SASAdapter:postTelemetryState", 
                       "prev: " << prev[i] << ", values: " << values[i] 
                       << ", tolerance: " << tolerance[i]);
              if (fabs(prev[i] - values[i]) >= tolerance[i])
                changed = true;
            }

          iter->second.setPreviousValues(vect);
        }
      else
        debugMsg("SASAdapter:postTelemetryState", "Not a known previous value to compute a change.");
    }

  if (changed)
    {
      debugMsg("SASAdapter::postTelemetryState", "The state has changed. Posting value");
      m_execInterface.handleValueChange(iter->second.getStateKey(), vect);
      m_execInterface.notifyOfExternalEvent();
    }

}


//
// Inner class implementation details
//

SASAdapter::LcmBaseImplSASExec::LcmBaseImplSASExec(lcm_t *lcm, SASAdapter* sasAdapter)
  : m_lcm(lcm), m_sasAdapter(sasAdapter)
{
  subscribeToMessages();
}

SASAdapter::LcmBaseImplSASExec::~LcmBaseImplSASExec()
{
  unsubscribeFromMessages();
}

void SASAdapter::LcmBaseImplSASExec::subscribeToMessages()
{
  genericRespSub = genericResponse_subscribe(m_lcm, "GENERICRESPONSE", 
					     &genericResponse_handler, m_sasAdapter);
  telDouble = telemetryDouble_subscribe(m_lcm, "TELEMETRYDOUBLE",
					&telemetryDouble_handler, m_sasAdapter);
}

void SASAdapter::LcmBaseImplSASExec::unsubscribeFromMessages()
{
  genericResponse_unsubscribe(m_lcm, genericRespSub);
  telemetryDouble_unsubscribe(m_lcm, telDouble);
}

void
SASAdapter::LcmBaseImplSASExec::genericResponse_handler (const lcm_recv_buf_t *rbuf, 
							 const char * channel, 
							 const genericResponse * msg,
							 void * user)
{
  SASAdapter* server = static_cast<SASAdapter*>(user);
  printf("Received a generic response\n");
  server->postCommandResponse(msg->name, msg->retValue[0]);
}

void
SASAdapter::LcmBaseImplSASExec::telemetryDouble_handler (const lcm_recv_buf_t *rbuf,
							 const char * channel, 
							 const telemetryDouble * msg,
							 void * user)
{
  SASAdapter* server = static_cast<SASAdapter*>(user);
  printf ("GOT some response for telemetry: %s %d %f\n", msg->state, msg->number, msg->values[0]);
  //  server->postTelemetryState(msg->state, msg->number, msg->values);
  server->postTelemetryState(msg->state, 1, msg->values);
}

