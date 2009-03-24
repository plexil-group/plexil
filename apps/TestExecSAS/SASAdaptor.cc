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
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "ThreadSpawn.hh"

// C wrapper to call a C++ method. Used while spawning a thread.
void spawnThreadForEachClient(void* args)
{
  
  std::cout << "Spawing the listening loop." << std::endl;
  SASAdaptor* server = static_cast<SASAdaptor*>(args);

  bool done = false;
  ServerSocket ss(server->getListeningPortNumber());
  if (ss.accept(ss))
    {
      while(!done)
        {
          std::string msg;
          try
            {
              ss >> msg;
              server->readMessage(msg);
            }
          catch (SocketException se)
            {
              done = true;
              std::cout << "spawnThreadForEachClient: " << se.description() << std::endl;
            }
        }
    }
}



SASAdaptor::SASAdaptor(PLEXIL::AdaptorExecInterface& execInterface, const std::string& host, int sendingPort,
                       int receivingPort) : InterfaceAdaptor(execInterface), m_ListeningPort(receivingPort)
{
  try 
    {
      m_ClientSocket = new ClientSocket(host, sendingPort);
    }
  catch (SocketException se)
    {
      m_ClientSocket = NULL;
      std::cout << "SASAdaptor::SASAdaptor(): Setting up client(sender): " 
                << se.description() << std::endl;
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
  delete m_ClientSocket;
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
  
  if (m_ClientSocket != NULL) 
    {
      debugMsg("SASAdaptor:executeCommand", 
               "Sending the following command to the stand alone simulator: "
               << cmdName.toString());

      *m_ClientSocket << cmdName.toString();
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
  
  std::map<std::string, int>::iterator iter;
  if (n == "time")
    {
      returnValue = 0.0;
      dest[0] = returnValue;
    }
  else if ((iter = m_StateToValueMap.find(n)) != m_StateToValueMap.end())
    {

      debugMsg("SASAdaptor:lookupNow", "Found a cached state");
      dest[0] = iter->second;
      m_StateToValueMap.erase(iter);
    }
}

void SASAdaptor::readMessage(const std::string& msg)
{
  debugMsg("SASAdaptor:readMessage", 
           "Got the following response from the stand alone simulator: " << msg);

  int msgType;
  char name[256];
  float value;
  // parse the msg;
  sscanf(msg.c_str(),"%d %s %f", &msgType, name, &value);

  std::cout << "msgType: " << msgType << ", name: " << name
            << ", value: " << value << std::endl;
  
  // determine of the msg is command response or telemetry
  // if command response look up the expression id and post
  // else if telemetry, create a <state_name, value> pair.

  if (msgType == 0)
    {
      std::map<std::string, PLEXIL::ExpressionId>::iterator iter;

      if ((iter = m_CommandToExpIdMap.find(name)) != m_CommandToExpIdMap.end())
        {
          //PLEXIL::LabelStr(value).getKey()
          m_execInterface.handleValueChange(iter->second, value);
          m_CommandToExpIdMap.erase(iter);
        }
      m_execInterface.notifyOfExternalEvent();
    }
  else if (msgType == 1)
    {
      m_StateToValueMap[name] = static_cast<int>(value);
    }

}


