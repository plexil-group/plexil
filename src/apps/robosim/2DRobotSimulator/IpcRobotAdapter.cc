/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#include "IpcRobotAdapter.hh"
#include "RobotBase.hh"

#include "Debug.hh"
#include "Error.hh"
#include "ThreadSpawn.hh"


// ooid classes
#include "uuid.h"
#include "devrand.h"

#include <sys/time.h>

/**
 * @brief Constructor. Opens the connection and spawns a listener thread.
 */
IpcRobotAdapter::IpcRobotAdapter(const std::string& centralhost)
  : m_robots(),
    m_stateUIDMap(),
    m_ipcFacade(),
    m_listener(*this)
{
  assertTrueMsg(m_ipcFacade.initilize(m_ipcFacade.getUID(), centralhost) == IPC_OK,
      "IpcRobotAdapter: Unable to initilize ipc to central server at " << centralhost);

  // Spawn listener thread
  assertTrueMsg(m_ipcFacade.start() == IPC_OK,
		"IpcRobotAdapter constructor: Unable to start IPC dispatch thread");

  // Subscribe only to messages we care about
  m_ipcFacade.subscribe(m_listener, PlexilMsgType_Command);
  m_ipcFacade.subscribe(m_listener, PlexilMsgType_LookupNow);
  debugMsg("IpcRobotAdapter:IpcRobotAdapter", " succeeded");
}

/**
 * @brief Destructor. Shuts down the listener thread and closes the connection.
 */
IpcRobotAdapter::~IpcRobotAdapter()
{
    // Unsubscribe from messages
    IPC_RETURN_TYPE status = IPC_unsubscribe(NUMERIC_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcRobotAdapter: Error unsubscribing from " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(STRING_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcRobotAdapter: Error unsubscribing from " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);

    // Cancel IPC dispatch thread
    int myErrno;
    assertTrueMsg((myErrno = pthread_cancel(m_thread)) == 0,
		  "IpcRobotAdapter: error canceling IPC dispatch thread, errno = " << myErrno); 
    assertTrueMsg((myErrno = pthread_join(m_thread, NULL)) == 0,
		  "IpcRobotAdapter: error joining IPC dispatch thread, errno = " << myErrno); 

    debugMsg("IpcRobotAdapter:~IpcRobotAdapter", " complete");
}

/**
 * @brief Make the adapter aware of the robot.
 */
void IpcRobotAdapter::registerRobot(const std::string& name, 
				    RobotBase* robot)
{
  NameToRobotMap::const_iterator it = m_robots.find(name);
  assertTrueMsg(it == m_robots.end(),
		"Robot name conflict for \"" << name << "\"");
  m_robots[name] = robot;
}

/**
 * @brief Send a response from the sim back to the UE.
 */

// *** TODO: isolate this method from the format of the response base!

void IpcRobotAdapter::sendReturnValues(const IpcMessageId& requestId,
				       const std::vector<double>& values)
{
  // Get the response message
  // Format the leader
  PlexilMsgBase* leader = NULL;
  debugMsg("IpcRobotAdapter:sendReturnValues",
	   " sending " << values.size() << " return value(s)");
  PlexilReturnValuesMsg* rvMsg = new PlexilReturnValuesMsg();
  rvMsg->header.msgType = (uint16_t) PlexilMsgType_ReturnValues;
  rvMsg->requestSerial = requestId.second;
  rvMsg->requesterUID = requestId.first.c_str();
  leader = reinterpret_cast<PlexilMsgBase*>(rvMsg);

  // fill in common fields
  leader->count = values.size();
  uint32_t leaderSerial = ++m_serial;
  leader->serial = leaderSerial;
  leader->senderUID= m_myUID.c_str(); 

  // Format the value message(s)
  PlexilNumericValueMsg* valueMsgs[values.size()];
  for (size_t i = 0; i < values.size(); i++)
    {
      PlexilNumericValueMsg* valueMsg = new PlexilNumericValueMsg();
      valueMsg->header.msgType = (uint16_t) PlexilMsgType_NumericValue;
      valueMsg->header.count = (uint16_t) i;
      valueMsg->header.serial = leaderSerial;
      valueMsg->header.senderUID = m_myUID.c_str();
      valueMsg->doubleValue = values[i];
      valueMsgs[i] = valueMsg;
    }

  // Send them
  debugMsg("IpcRobotAdapter:sendReturnValues", " sending leader");
  IPC_RETURN_TYPE status;
  status = IPC_publishData(RETURN_VALUE_MSG, (void *) leader);
  assertTrueMsg(status == IPC_OK,
		"IpcRobotAdapter::sendReturnValues: IPC Error, IPC_errno = " << IPC_errno);
  debugMsg("IpcRobotAdapter:sendReturnValues", " sending " << values.size() << " values");
  for (size_t i = 0; i < values.size(); i++)
    {
      status = IPC_publishData(NUMERIC_VALUE_MSG, (void *) valueMsgs[i]);
      assertTrueMsg(status == IPC_OK,
		    "IpcRobotAdapter::sendReturnValues: IPC Error, IPC_errno = " << IPC_errno);
    }

  // free the leader
  if (leader->msgType == PlexilMsgType_ReturnValues)
    delete reinterpret_cast<PlexilReturnValuesMsg*>(leader);
  else if (leader->msgType == PlexilMsgType_TelemetryValues)
    delete reinterpret_cast<PlexilStringValueMsg*>(leader);

  // free the value packets
  for (size_t i = 0; i < values.size(); i++)
    {
      delete valueMsgs[i];
    }

  debugMsg("IpcRobotAdapter:sendReturnValues", " completed");
}

/**
 * @brief Send a command to RoboSim
 */

// N.B. RoboSim commands take one argument, the robot name.
// Additional arguments are ignored.
void IpcRobotAdapter::processCommand(std::vector<const PlexilMsgBase*>& msgs)
{
  const std::string cmdName(((const PlexilStringValueMsg*)msgs[0])->stringValue);
  assertTrueMsg(msgs[0]->count >= 1,
		"IpcRobotAdapter::processCommand: robot name argument missing for command \"" << cmdName << "\"");
  assertTrueMsg(msgs[1]->msgType == PlexilMsgType_StringValue,
		"IpcRobotAdapter::processCommand: robot name argument for command \"" << cmdName << "\" is not a string");
  checkError(msgs.size() >= 2,
	     "IpcRobotAdapter::processCommand: internal error: not enough arguments to \""
	     << cmdName << "\" command");
  const std::string robotName(((const PlexilStringValueMsg*)msgs[1])->stringValue);
  NameToRobotMap::const_iterator it = m_robots.find(robotName);
  assertTrueMsg(it != m_robots.end(),
		"IpcRobotAdapter::processCommand: no robot named \"" << robotName << "\"");
  RobotBase* robot = it->second;
  assertTrueMsg(robot != NULL,
		"IpcRobotAdapter::processCommand: robot named \"" << robotName << "\" is null!");
  IpcMessageId transId = IpcMessageId(msgs[0]->senderUID, msgs[0]->serial);
  double parameter = 0.0;
  // Check for missing parameter
  if (cmdName == "Move")
    {
      assertTrueMsg(msgs[0]->count >= 2,
		    "IpcRobotAdapter::processCommand: missing required direction argument to \""
		    << cmdName << "\" command");
      checkError(msgs.size() >= 3,
		 "IpcRobotAdapter::processCommand: internal error: not enough arguments to \""
		 << cmdName << "\" command");
      assertTrueMsg(msgs[2]->msgType == PlexilMsgType_NumericValue,
		    "IpcRobotAdapter::processCommand: direction argument for command \"" << cmdName << "\" is not a number");
      parameter = ((const PlexilNumericValueMsg*)msgs[2])->doubleValue;
      condDebugMsg(msgs[0]->count > 2,
		   "IpcRobotAdapter:processCommand",
		   "Ignoring " << msgs[0]->count - 2 << " argument(s)");
    }
  else
    {
      condDebugMsg(msgs[0]->count > 1,
		   "IpcRobotAdapter:processCommand",
		   "Ignoring " << msgs[0]->count - 1 << " argument(s)");
    }
  std::vector<double>& ret_values = robot->processCommand(cmdName, parameter);
  m_ipcFacade.publishReturnValues(transId, std::list<double>(ret_values.begin(), ret_values.end()));
}

/**
 * @brief Deal with a LookupNow request
 */

// N.B. RoboSim does not implement LookupNow
void IpcRobotAdapter::processLookupNow(std::vector<const PlexilMsgBase*>& msgs)
{
  std::string stateName(((const PlexilStringValueMsg*)msgs[0])->stringValue);
  debugMsg("IpcRobotAdapter:lookupNow", " ignoring lookup request for " << stateName);
}


IpcRobotAdapter::MessageListener::MessageListener(IpcRobotAdapter& adapter) :
  m_adapter(adapter) {
}
IpcRobotAdapter::MessageListener::~MessageListener() {
}
void IpcRobotAdapter::MessageListener::ReceiveMessage(const std::vector<const PlexilMsgBase*>& msgs) {
  const PlexilMsgBase* leader = msgs[0];
  switch (leader->msgType)
    {
    case PlexilMsgType_Command:
      m_adapter.processCommand(msgs);
      break;

    case PlexilMsgType_LookupNow:
      m_adapter.processLookupNow(msgs);
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL,
            "IpcRobotAdapter::ReceiveMessage: invalid leader message type " << leader->msgType);
    }
}
