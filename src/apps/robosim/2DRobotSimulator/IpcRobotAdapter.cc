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

#include "defineIpcMessageTypes.h"

// ooid classes
#include "uuid.h"
#include "devrand.h"

#include <sys/time.h>

/**
 * @brief Constructor. Opens the connection and spawns a listener thread.
 */
IpcRobotAdapter::IpcRobotAdapter(const std::string& centralhost)
  : m_robots(),
    m_incompletes(),
    m_stateUIDMap(),
    m_thread(),
    m_myUID(),
    m_serial(0)
{
  initializeUID();

  debugMsg("IpcRobotAdapter:IpcRobotAdapter", 
	   " Connecting module " << m_myUID <<
	   " to central server at " << centralhost);

  // Initialize IPC
  // possibly redundant, but always safe
  IPC_initialize();

  // Connect to central
  assertTrueMsg(IPC_connectModule(m_myUID.c_str(), centralhost.c_str()) == IPC_OK,
		"IpcRobotAdapter: Unable to connect to the central server at " << centralhost);

  // Define the whole suite of message types,
  // as someone else may depend on this if we get to it first.
  assertTrueMsg(definePlexilIPCMessageTypes(),
		"IpcRobotAdapter: Unable to define IPC message types");

  // Spawn listener thread
  assertTrueMsg(threadSpawn((THREAD_FUNC_PTR)IPC_dispatch, NULL, m_thread),
		"IpcRobotAdapter constructor: Unable to spawn IPC dispatch thread");
  debugMsg("IpcRobotAdapter:IpcRobotAdapter", " spawned IPC dispatch thread");

  // Subscribe only to messages we care about
  IPC_RETURN_TYPE status = IPC_subscribeData(NUMERIC_VALUE_MSG, messageHandler, this);
  assertTrueMsg(status == IPC_OK,
		"IpcRobotAdapter: Error subscribing to " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
  status = IPC_subscribeData(STRING_VALUE_MSG, messageHandler, this);
  assertTrueMsg(status == IPC_OK,
		"IpcRobotAdapter: Error subscribing to " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);

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
 * @brief Initialize unique ID string
 */
void IpcRobotAdapter::initializeUID()
{
  kashmir::system::DevRand randomStream;
  kashmir::uuid_t uuid;
  randomStream >> uuid;
  std::ostringstream s;
  s << uuid;
  m_myUID = s.str();
  debugMsg("IpcRobotAdapter:initializeUID", " generated UUID " << m_myUID);
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
 * @brief Handler function as seen by IPC.
 */

void IpcRobotAdapter::messageHandler(MSG_INSTANCE rawMsg,
				  void * unmarshalledMsg,
				  void * this_as_void_ptr)
{
  IpcRobotAdapter* theRelay = reinterpret_cast<IpcRobotAdapter*>(this_as_void_ptr);
  assertTrueMsg(theRelay != NULL,
		"IpcRobotAdapter::messageHandler: pointer to instance is null!");

  const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*>(unmarshalledMsg);
  assertTrueMsg(msgData != NULL,
		"IpcRobotAdapter::messageHandler: pointer to message data is null!");

  theRelay->handleIpcMessage(msgData);
}

/**
 * @brief Handler function as seen by comm relay.
 */

void IpcRobotAdapter::handleIpcMessage(const PlexilMsgBase * msgData)
{
  if (strcmp(msgData->senderUID, m_myUID.c_str()) == 0)
    {
      debugMsg("IpcRobotAdapter:handleIpcMessage", " ignoring my own outgoing message");
      return;
    }

  PlexilMsgType msgType = (PlexilMsgType) msgData->msgType;
  debugMsg("IpcRobotAdapter:handleIpcMessage", " received message type = " << msgType);
  switch (msgType)
    {
      // Command is a PlexilStringValueMsg
      // Optionally followed by parameters
    case PlexilMsgType_Command:
      // Stash this and wait for the rest
      debugMsg("IpcRobotAdapter:handleIpcMessage", " processing as command");
      cacheMessageLeader(msgData);
      break;

      // LookupNow and LookupOnChange are PlexilStringValueMsg
      // Optionally followed by parameters
    case PlexilMsgType_LookupNow:
    case PlexilMsgType_LookupOnChange:
      // Stash this and wait for the rest
      cacheMessageLeader(msgData);
      break;

      // Values - could be parameters or return values
    case PlexilMsgType_NumericValue:
    case PlexilMsgType_StringValue:
      // Log with corresponding leader message, if any
      cacheMessageTrailer(msgData);
      break;

      //
      // below this line are not used by the simulator
      //

      // NotifyExec is a PlexilMsgBase
    case PlexilMsgType_NotifyExec:

      // AddPlan/AddPlanFile is a PlexilStringValueMsg
    case PlexilMsgType_AddPlan:
    case PlexilMsgType_AddPlanFile:

      // AddLibrary/AddLibraryFile is a PlexilStringValueMsg
    case PlexilMsgType_AddLibrary:
    case PlexilMsgType_AddLibraryFile:

      // ReturnValues is a PlexilReturnValuesMsg
      // Followed by 0 (?) or more values
    case PlexilMsgType_ReturnValues:

      // Message is a PlexilStringValueMsg
      // No parameters
    case PlexilMsgType_Message:

      // PlannerUpdate is a PlexilStringValueMsg
      // Followed by 0 (?) or more name/value pairs
    case PlexilMsgType_PlannerUpdate:

      // PlannerUpdate pairs
    case PlexilMsgType_PairNumeric:
    case PlexilMsgType_PairString:
      debugMsg("IpcRobotAdapter:handleIpcMessage", " ignoring message of type " << msgType);
      // free the message
      IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msgData->msgType)),
		   (void*) msgData);
      break;

      // clearly bogus data
    default:
      assertTrueMsg(ALWAYS_FAIL,
		    "IpcRobotAdapter::handleIpcMessage: received invalid message data type " << msgType);
      break;
    }
}

 
/**
 * @brief Cache start message of a multi-message sequence
 */
    
void IpcRobotAdapter::cacheMessageLeader(const PlexilMsgBase* msgData)
{
  IpcMessageId msgId(msgData->senderUID, msgData->serial);

  // Check that this isn't a duplicate header
  IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
  assertTrueMsg(it == m_incompletes.end(),
		"IpcRobotAdapter::cacheMessageLeader: internal error: found existing sequence for sender "
		<< msgData->senderUID << ", serial " << msgData->serial);

  if (msgData->count == 0)
    {
      debugMsg("IpcRobotAdapter:cacheMessageLeader", " no trailers, processing immediately");
      std::vector<const PlexilMsgBase*> msgVec(1, msgData);
      processMessageSequence(msgVec);
    }
  else
    {
      debugMsg("IpcRobotAdapter:cacheMessageLeader", " waiting for " << msgData->count << " trailing message(s)");
      m_incompletes[msgId] = std::vector<const PlexilMsgBase*>(1, msgData);
    }
}
 
/**
 * @brief Cache following message of a multi-message sequence
 */
    
void IpcRobotAdapter::cacheMessageTrailer(const PlexilMsgBase* msgData)
{
  IpcMessageId msgId(msgData->senderUID, msgData->serial);
  IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
  assertTrueMsg(it != m_incompletes.end(),
		"IpcRobotAdapter::cacheMessageTrailer: no existing sequence for sender "
		<< msgData->senderUID << ", serial " << msgData->serial);
  std::vector<const PlexilMsgBase*>& msgs = it->second;
  msgs.push_back(msgData);
  // Have we got them all?
  if (msgs.size() > msgs[0]->count)
    {
      processMessageSequence(msgs);
      m_incompletes.erase(it);
    }
}

/**
 * @brief Send a message sequence to the simulator and free the messages
 */
void IpcRobotAdapter::processMessageSequence(std::vector<const PlexilMsgBase*>& msgs)
{
  const PlexilMsgBase* leader = msgs[0];
  switch (leader->msgType)
    {
    case PlexilMsgType_Command:
      processCommand(msgs);
      break;

    case PlexilMsgType_LookupNow:
      processLookupNow(msgs);
      break;

      // these are ignored - telemetry updates provide the data to the exec
    case PlexilMsgType_LookupOnChange:
      debugMsg("IpcRobotAdapter:handleMessageSequence", " ignoring LookupOnChange request");
      break;

      // ignore these if they get through - we don't deal with them
    case PlexilMsgType_ReturnValues:
    case PlexilMsgType_PlannerUpdate:
      debugMsg("IpcRobotAdapter:handleMessageSequence", " ignoring sequence starting with type " << leader->msgType);
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL,
		    "IpcRobotAdapter::processMessageSequence: invalid leader message type " << leader->msgType);
    }

  // clean up
  for (size_t i = 0; i < msgs.size(); i++)
    {
      const PlexilMsgBase* msg = msgs[i];
      msgs[i] = NULL;
      IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msg->msgType)),
		   (void *) msg);
    }
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
		"IpcRobotAdapter::processCommand: name argument missing for command \"" << cmdName << "\"");
  assertTrueMsg(msgs[1]->msgType == PlexilMsgType_StringValue,
		"IpcRobotAdapter::processCommand: name argument for command \"" << cmdName << "\" is not a string");
  const std::string robotName(((const PlexilStringValueMsg*)msgs[1])->stringValue);
  NameToRobotMap::const_iterator it = m_robots.find(robotName);
  assertTrueMsg(it != m_robots.end(),
		"IpcRobotAdapter::processCommand: no robot named \"" << robotName << "\"");
  RobotBase* robot = it->second;
  assertTrueMsg(robot != NULL,
		"IpcRobotAdapter::processCommand: robot named \"" << robotName << "\" is null!");
  IpcMessageId transId = IpcMessageId(msgs[0]->senderUID, msgs[0]->serial);
  condDebugMsg(msgs[0]->count > 1,
	       "IpcRobotAdapter:processCommand",
	       "Ignoring " << msgs[0]->count << " argument(s)");
  sendReturnValues(transId, robot->processCommand(cmdName));
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

/**
 * @brief Deal with a LookupOnChange request
 */

// N.B. RoboSim does not implement LookupOnChange
void IpcRobotAdapter::processLookupOnChange(std::vector<const PlexilMsgBase*>& msgs)
{
  std::string stateName(((const PlexilStringValueMsg*)msgs[0])->stringValue);
  debugMsg("IpcRobotAdapter:lookupOnChange", " ignoring lookup request for " << stateName);
}
