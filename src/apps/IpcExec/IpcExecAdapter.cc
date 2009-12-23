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

#include "IpcExecAdapter.hh"
#include <iostream>
#include <vector>
#include <math.h>
#include <string>

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
#include <stdlib.h>
#include <pthread.h>


//<IP info>
#include <netdb.h>
#include <arpa/inet.h>
//</IP Info>

extern "C" {
#include "ipc.h"
#include "ipcMsgFormats.h"
}

using namespace PLEXIL;

std::string IpcExecAdapter::MessageIndicator = "MESSAGE__"; //Get from an XML file
std::string IpcExecAdapter::UniqueIDDivide = "$";
int IpcExecAdapter::MessageIndicatorLength = MessageIndicator.size();
std::vector<double> IpcExecAdapter::emptyArgListVector;
std::vector<double> IpcExecAdapter::trueVector(1, 1.0);

IpcExecAdapter::IpcExecAdapter(AdapterExecInterface& execInterface) : 
  InterfaceAdapter(execInterface)
{
}

IpcExecAdapter::IpcExecAdapter(AdapterExecInterface& execInterface,
				   const TiXmlElement * xml) : 
  InterfaceAdapter(execInterface, xml)
{
}

IpcExecAdapter::~IpcExecAdapter()
{
  shutdown();
}

/**
 * @brief Initializes the adapter, possibly using its configuration data.
 * @return true if successful, false otherwise.
 */
bool IpcExecAdapter::initialize()
{
  // IPC connection info
  // Default connection values for IPC 

  const char* ipc_taskName = "defaultModule";
  // taskName is used only for message logging purposes, and needs not be unique.

  const char* ipc_serverName = "localhost";
  // serverName is the  machine name where the central server is running.


  //***** Should probably add the ability to get the MessageIdentifier from here as well?

  // Get taskName, serverName from XML, if supplied
  const TiXmlElement* xml = this->getXml();

  if (this->getXml() != NULL) 
    {
      const char* taskName = xml->Attribute("TaskName");
      const char* serverName = xml->Attribute("Server");
      if (taskName != NULL)
	{
	  ipc_taskName = taskName;
	}
      if(serverName != NULL)
        {
	  ipc_serverName = serverName;
        }
    }
  
  //Get my Unique ID
  myUniqueID = getMyIP() + ":" + getMyPID();

  //Construct IPC
  debugMsg("IpcExecAdapter:initialize", 
	   " Using IPC taskName" << ipc_taskName <<
	   " Using IPC serverName" << ipc_serverName);

  // possibly redundant, but always safe
  IPC_initialize();

  IPC_connectModule(ipc_taskName, ipc_serverName);

  if(IPC_isConnected() != 1){ //Not connected to the central server

    assertTrueMsg(false,
		  "IpcExecAdapter: Unable to connect to the central server. ");

    return false;
  }

  // For dynamic updating of available executives
  // IPC_subscribeConnect(newExecConnectHandler, this);

  // Sets the number of simultaneous messages the central server is able to send
  IPC_setCapacity(4);


  //You need to define the messages and formats that the IpcExecAdapter needs in
  //order to publish and subscribe to messages.

  //defineformat - Only need to do this for non IPC provided formats

  // Check if format is already defined; if not, define it
  if (!IPC_isMsgDefined(STRMSG))
    {
      IPC_defineMsg(STRMSG, IPC_VARIABLE_LENGTH, STRMSG_FORMAT); //Publishes Strings
    }

  m_execInterface.defaultRegisterAdapter(getId());

  return true;
}

/**
 * @brief Starts the adapter, possibly using its configuration data.  
 * @return true if successful, false otherwise.
 */
bool IpcExecAdapter::start()
{
  //What types of data the IpcExecAdapter subscribes to.
  IPC_subscribe(STRMSG, strMsgHandler, this);
 
 
  if (threadSpawn((THREAD_FUNC_PTR)IPC_dispatch, (void *)this,
                  m_ThreadId))
    {
      debugMsg("IpcExecAdapter:start", " created IPC thread " << m_ThreadId);
    }
  else
    {
      std::cerr << "Error spawing IPC dispatch thread" << std::endl;
      return false;
    }
  
  return true;
}

/**
 * @brief Stops the adapter.  
 * @return true if successful, false otherwise.
 */
bool IpcExecAdapter::stop()
{
  pthread_cancel(m_ThreadId);
  pthread_join(m_ThreadId, NULL); //waits until m_ThreadId is terminated before continuing

  //If we subscribe to connections (in Initialize()) then we need to unsubscribe here.
  //IPC_unsubscribeConnect();
  
  return true;
}

/**
 * @brief Resets the adapter.  
 * @return true if successful, false otherwise.
 */
bool IpcExecAdapter::reset()
{
  start();
  return true;
}

/**
 * @brief Shuts down the adapter, releasing any of its resources.
 * @return true if successful, false otherwise.
 */
bool IpcExecAdapter::shutdown()
{
  //Deallocate whatever memory we've allocated.
  IPC_disconnect();
  return true;
}

void IpcExecAdapter::executeCommand(const LabelStr& name,
				      const std::list<double>& args,
				      ExpressionId dest, 
				      ExpressionId ack)
{
  const LabelStr& cmdName = name;
  const std::string& cmdString = name.toString();
  
  assertTrueMsg(IPC_isConnected(),
		"IpcExecAdapter::executeCommand: IPC not connected");
  debugMsg("IpcExecAdapter:executeCommand", 
	   " Sending command \""
	   << cmdString
	   << "\" to simulator");
  
  // Beginning of Get Data & Publish 
  for (std::list<double>::const_iterator it = args.begin(); it != args.end(); it++)
    {
    
      //Check to see if cmdString is "SendMessage" or "SendCommand"
      if (cmdString == "SendMessage")
	{
      
	  //If the element in the list of doubles actually represents a string...
	  if (LabelStr::isString(*it))
	    {
	      std::string ourMessage = LabelStr::LabelStr(*it).toString();

	      //Can remove the "myUniqueID + UniqueIDDivide" once a universal IPC PLEXIL message format is created.
	      //Because there is a specific varaible in the message format for it.
	      std::string finalString = myUniqueID + UniqueIDDivide +  MessageIndicator + ourMessage;
	      // Must pass a POINTER to the char* pointer!
	      char* finalCString = const_cast<char*>(finalString.c_str());

	      // For debugging:
	      //std::cout << "Publishing \"" << finalString << "\", length is " << strlen(finalString.c_str()) << std::endl; 

	      // Must pass a POINTER to the string!
	      IPC_publishData(STRMSG, &finalCString);
	    }
	  else
	    {
	      //More Debug Info..
	      //std::cout << *it 
	      //	  << " represents something OTHER than a string.\n"
	      //	  << std::endl;
	
	      //The element represnts something other than a string. However we can't be entirely sure 100% of the time.
	      //We can tell if it's a Real number, or an Array of Real numbers.
	      //We can tell if it's some Integer (other than 1 or 0), or an array of Integers - [1,0].
	      //We CANNOT tell whether [1,0] represent Boolean values or Integer values.
	    }
	}
      else if (cmdString == "SendCommand")
	{
	  //Do something for commands here.
	  assertTrueMsg(ALWAYS_FAIL, "IpcExecAdapter::executeCommand: \"SendCommand\" command not yet implemented");
	}
      else 
	{
	  // Other commands implemented
	  assertTrueMsg(ALWAYS_FAIL, "IpcExecAdapter::executeCommand: command \"" << cmdString << "\" is unimplemented");
	}
    }
  
  // End of Get Data & Publish
 
  m_execInterface.handleValueChange
    (ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM());
  m_execInterface.notifyOfExternalEvent();
  
  m_CommandToExpIdMap[cmdName.toString()] = dest;
}

void IpcExecAdapter::lookupNow(const StateKey& stateKey,
				 std::vector<double>& dest)
{
  State state;
  this->getState(stateKey, state);
  const LabelStr& name = state.first;
  std::string n = name.toString();
  debugMsg("IpcExecAdapter:lookupNow", " Looking up state: " << n);
  
}

void IpcExecAdapter::registerChangeLookup(const LookupKey& uniqueId,
					    const StateKey& stateKey,
					    const std::vector<double>& tolerances)
{
  //Everything is taken care-of with the m_asynchLookups map and the strMsgHandler.

  State state;
  this->getState(stateKey, state);
  const LabelStr& name = state.first;
  std::string n = name.toString();

  //Debugging
  //std::cout << "LookupOnChange(" << n << ")" << std::endl;
  debugMsg("IpcExecAdapter:registerChangeLookup", " for " << n);
  
}

void IpcExecAdapter::unregisterChangeLookup(const LookupKey& uniqueId)
{
  debugMsg("IpcExecAdapter:unregisterChangeLookup", " In unregister change look up");
  // TODO: What exactly needs to be done here?
}



//Helper Functions

// Based loosely on examples from...
// http://www.daniweb.com/code/post968089.html#
// http://rabbit.eng.miami.edu/info/functions/internet.html#gethostbynamesample1
std::string IpcExecAdapter::getMyIP(){
  //Need to modify if you want IPv6 Compatibility
#define MAX_HOST_LEN 512

  char host_buf[MAX_HOST_LEN];
  gethostname(host_buf, MAX_HOST_LEN);

  hostent * record = gethostbyname(host_buf);

  if (record==NULL){
    //Throw a real error too.
    return "getMyIP() Error";
  }

  char *myIP;
  in_addr * address=(in_addr * )record->h_addr;
  myIP = inet_ntoa(* address); //inet_ntoa() is deprecated... replace with inet_ntop()

  return (std::string)myIP;
}

std::string IpcExecAdapter::getMyPID(){ 
  pid_t currentExecId = getpid();
  char myStrPID[256];
  pid_t myPID = currentExecId;
   
  sprintf(myStrPID, "%d", currentExecId);

  return (std::string)myStrPID;
}

 
// IPC_Handlers

// Dynamically adding new exec connections to a list?
// static void newExecConnectHandler (const char *moduleName, void *clientData)
// {
//   IpcExecAdapter *myAdapter = (IpcExecAdapter*) clientData;
//   // invoke IpcExecAdapter methods here

//   IPC_isModuleConnected(moduleName);
// }
  
void IpcExecAdapter::strMsgHandler (MSG_INSTANCE msgRef, BYTE_ARRAY callData,
				      void *this_as_void_ptr)
{
  IpcExecAdapter* myAdapter = reinterpret_cast<IpcExecAdapter*>(this_as_void_ptr);
      
  assertTrueMsg((myAdapter != 0),
		"IpcExecAdapter::strMsgHandler: Pointer to adapter is null");
  
  myAdapter->handleMsg(msgRef, callData);
}


void IpcExecAdapter::handleMsg(MSG_INSTANCE msgRef, BYTE_ARRAY callData)
{
  STRMSG_TYPE strMsg;
  
  IPC_unmarshallData(IPC_msgInstanceFormatter(msgRef), callData,
		     &strMsg, sizeof(strMsg));
  
  std::string strWUniqueID(strMsg);
  
  // Should probably check to see if the "UniqueIDDivide" is in the string before we do this in the future.
  // Then again, in the future the UniqueID stuff will be as an individual variable in the
  // IPC PLEXIL Message Data Structure... so it doesn't really matter.

  int endOfUniqueID = (int)strWUniqueID.find(UniqueIDDivide);

  if(strWUniqueID.substr(0, endOfUniqueID) == myUniqueID)
    {
      //This is the data that I published... I don't care about it.
    }
  else
    {
      //Our original message, without the UniqueID or UniqueIDDivide.
      std::string str1 = strWUniqueID.substr(endOfUniqueID + 1, strWUniqueID.length());
    
      //Data we care about because we didn't send it.
    
      if((str1.substr(0, MessageIndicatorLength)) == MessageIndicator){
      
	State myState(LabelStr(str1).getKey(),emptyArgListVector);
      
	//Now get the Key associated to the state "myState"
      
	StateKey key;
	//Can't use the bool return as our lookup b/c if the key has been created before
	//but nolonger exists in the lookup table; then it would fail.
	m_execInterface.keyForState(myState,key);

	std::map<StateKey, std::set<LookupKey> >::const_iterator it = findLookupKey(key);
	if (it != getAsynchLookupsEnd())
	  {
	    // For debugging
	    // std::cerr << "Submitting \"" << str1 << "\" to Exec" << std::endl;
	    m_execInterface.handleValueChange(key, trueVector);
	    m_execInterface.notifyOfExternalEvent();
	  }
	else{
	  //Message wasn't in our lookup table
	}
      
	//Debugging
	//printf("Receiving %s \"%s\" \n", IPC_msgInstanceName(msgRef), strMsg);
      }
      else{
	//Message doesn't begin with our MessageIndicator

	//This is where we check if the message begins with some other indicator (COMMAND__, etc.)

	//Debugging
	//printf("Receiving %s \"%s\" \n", IPC_msgInstanceName(msgRef), strMsg);
      }
    
      IPC_freeByteArray(callData);
    
    }
}
