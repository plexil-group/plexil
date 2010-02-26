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

#include "IpcCommRelay.hh"
#include "defineIpcMessageTypes.h"
#include "ResponseMessage.hh"
#include "GenericResponse.hh"
#include "Simulator.hh"

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
IpcCommRelay::IpcCommRelay(const std::string& id, const std::string& centralhost)
  : CommRelayBase(id),
    m_incompletes(),
    m_stateUIDMap(),
    m_thread(),
    m_myUID(),
    m_serial(0)
{
  initializeUID();

  debugMsg("IpcCommRelay:IpcCommRelay", 
	   " Connecting module " << m_myUID <<
	   " to central server at " << centralhost);

  // Initialize IPC
  // possibly redundant, but always safe
  IPC_initialize();

  // Connect to central
  assertTrueMsg(IPC_connectModule(m_myUID.c_str(), centralhost.c_str()) == IPC_OK,
		"IpcCommRelay: Unable to connect to the central server at " << centralhost);

  // Define the whole suite of message types,
  // as someone else may depend on this if we get to it first.
  assertTrueMsg(definePlexilIPCMessageTypes(),
		"IpcCommRelay: Unable to define IPC message types");

  // Spawn listener thread
  assertTrueMsg(threadSpawn((THREAD_FUNC_PTR)IPC_dispatch, NULL, m_thread),
		"IpcCommRelay constructor: Unable to spawn IPC dispatch thread");
  debugMsg("IpcCommRelay:IpcCommRelay", " spawned IPC dispatch thread");

  // Subscribe only to messages we care about
  IPC_RETURN_TYPE status = IPC_subscribeData(NUMERIC_VALUE_MSG, messageHandler, this);
  assertTrueMsg(status == IPC_OK,
		"IpcCommRelay: Error subscribing to " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
  status = IPC_subscribeData(STRING_VALUE_MSG, messageHandler, this);
  assertTrueMsg(status == IPC_OK,
		"IpcCommRelay: Error subscribing to " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);

  debugMsg("IpcCommRelay:IpcCommRelay", " succeeded");
}

/**
 * @brief Destructor. Shuts down the listener thread and closes the connection.
 */
IpcCommRelay::~IpcCommRelay()
{
    // Unsubscribe from messages
    IPC_RETURN_TYPE status = IPC_unsubscribe(NUMERIC_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcCommRelay: Error unsubscribing from " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(STRING_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcCommRelay: Error unsubscribing from " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);

    // Cancel IPC dispatch thread
    int myErrno;
    assertTrueMsg((myErrno = pthread_cancel(m_thread)) == 0,
		  "IpcCommRelay: error canceling IPC dispatch thread, errno = " << myErrno); 
    assertTrueMsg((myErrno = pthread_join(m_thread, NULL)) == 0,
		  "IpcCommRelay: error joining IPC dispatch thread, errno = " << myErrno); 

    debugMsg("IpcCommRelay:~IpcCommRelay", " complete");
}

/**
 * @brief Initialize unique ID string
 */
void IpcCommRelay::initializeUID()
{
  kashmir::system::DevRand randomStream;
  kashmir::uuid_t uuid;
  randomStream >> uuid;
  std::ostringstream s;
  s << uuid;
  m_myUID = s.str();
  debugMsg("IpcCommRelay:initializeUID", " generated UUID " << m_myUID);
}

/**
 * @brief Send a response from the sim back to the UE.
 */

// *** TODO: isolate this method from the format of the response base!

void IpcCommRelay::sendResponse(const ResponseMessage* respMsg)
{
  // Get the response message
  const GenericResponse* gr = dynamic_cast<const GenericResponse*>(respMsg->getResponseBase());
  assertTrueMsg(gr != NULL,
		"IpcCommRelay::sendResponse: invalid ResponseBase object");
  const std::vector<double>& values = gr->getReturnValue();

  // Format the leader
  PlexilMsgBase* leader = NULL;
  switch (respMsg->getMessageType())
    {
    case MSG_COMMAND:
    case MSG_LOOKUP:
      {
	// Return values message
	debugMsg("IpcCommRelay:sendResponse",
		 " sending " << values.size() << " return value(s) for "
		 << ((respMsg->getMessageType() == MSG_COMMAND) ? "command" : "lookup")
		 << " \"" << respMsg->getName() << "\"");
	const IpcMessageId* transId = static_cast<const IpcMessageId*>(respMsg->getId());
	assertTrueMsg(transId != NULL,
		      "sendResponse: internal error: null transaction ID for command");
	PlexilReturnValuesMsg* rvMsg = new PlexilReturnValuesMsg();
	rvMsg->header.msgType = (uint16_t) PlexilMsgType_ReturnValues;
	rvMsg->requestSerial = transId->second;
	rvMsg->requesterUID = transId->first.c_str();
	leader = reinterpret_cast<PlexilMsgBase*>(rvMsg);
      }
      break;

    case MSG_TELEMETRY:
      {
	// Telemetry values message
	debugMsg("IpcCommRelay:sendResponse",
		 " sending telemetry message for \"" << respMsg->getName() << "\"");
	PlexilStringValueMsg* tvMsg = new PlexilStringValueMsg();
	tvMsg->header.msgType = (uint16_t) PlexilMsgType_TelemetryValues;
	tvMsg->stringValue = respMsg->getName().c_str();
	leader = reinterpret_cast<PlexilMsgBase*>(tvMsg);
      }
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL,
		    "IpcCommRelay::sendResponse: invalid message type " << respMsg->getMessageType());
    }
  // safety check before we go any further!
  checkError(leader != NULL,
	     "IpcCommRelay::sendResponse: internal error: leader message is null!");

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
  debugMsg("IpcCommRelay:sendResponse", " sending leader");
  IPC_RETURN_TYPE status;
  status = IPC_publishData((respMsg->getMessageType() == MSG_TELEMETRY ? STRING_VALUE_MSG : RETURN_VALUE_MSG),
			   (void *) leader);
  assertTrueMsg(status == IPC_OK,
		"IpcCommRelay::sendResponse: IPC Error, IPC_errno = " << IPC_errno);
  debugMsg("IpcCommRelay:sendResponse", " sending " << values.size() << " values");
  for (size_t i = 0; i < values.size(); i++)
    {
      status = IPC_publishData(NUMERIC_VALUE_MSG, (void *) valueMsgs[i]);
      assertTrueMsg(status == IPC_OK,
		    "IpcCommRelay::sendResponse: IPC Error, IPC_errno = " << IPC_errno);
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

  // free the ResponseMessage and associated
  if (respMsg->getMessageType() == MSG_COMMAND)
    delete static_cast<IpcMessageId*>(respMsg->getId());
  delete respMsg;
  debugMsg("IpcCommRelay:sendResponse", " completed");
}

/**
 * @brief Handler function as seen by IPC.
 */

void IpcCommRelay::messageHandler(MSG_INSTANCE rawMsg,
				  void * unmarshalledMsg,
				  void * this_as_void_ptr)
{
    IpcCommRelay* theRelay = reinterpret_cast<IpcCommRelay*>(this_as_void_ptr);
    assertTrueMsg(theRelay != NULL,
		  "IpcCommRelay::messageHandler: pointer to instance is null!");

    const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*>(unmarshalledMsg);
    assertTrueMsg(msgData != NULL,
		  "IpcCommRelay::messageHandler: pointer to message data is null!");

    theRelay->handleIpcMessage(msgData);
}

/**
 * @brief Handler function as seen by comm relay.
 */

void IpcCommRelay::handleIpcMessage(const PlexilMsgBase * msgData)
{
  if (strcmp(msgData->senderUID, m_myUID.c_str()) == 0)
    {
      debugMsg("IpcCommRelay::handleIpcMessage", " ignoring my own outgoing message");
      return;
    }

  PlexilMsgType msgType = (PlexilMsgType) msgData->msgType;
  debugMsg("IpcCommRelay:handleIpcMessage", " received message type = " << msgType);
  switch (msgType)
    {
      // Command is a PlexilStringValueMsg
      // Optionally followed by parameters
    case PlexilMsgType_Command:
      // Stash this and wait for the rest
      debugMsg("IpcCommRelay:handleIpcMessage", " processing as command");
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
      debugMsg("IpcCommRelay:handleIpcMessage", " ignoring message of type " << msgType);
      // free the message
      IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msgData->msgType)),
		   (void*) msgData);
      break;

      // clearly bogus data
    default:
      assertTrueMsg(ALWAYS_FAIL,
		    "IpcCommRelay::handleIpcMessage: received invalid message data type " << msgType);
      break;
    }
}

 
/**
 * @brief Cache start message of a multi-message sequence
 */
    
void IpcCommRelay::cacheMessageLeader(const PlexilMsgBase* msgData)
{
    IpcMessageId msgId(msgData->senderUID, msgData->serial);

    // Check that this isn't a duplicate header
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    assertTrueMsg(it == m_incompletes.end(),
		  "IpcCommRelay::cacheMessageLeader: internal error: found existing sequence for sender "
		  << msgData->senderUID << ", serial " << msgData->serial);

    if (msgData->count == 0)
      {
	debugMsg("IpcCommRelay:cacheMessageLeader", " no trailers, processing immediately");
	std::vector<const PlexilMsgBase*> msgVec(1, msgData);
	processMessageSequence(msgVec);
      }
    else
      {
	debugMsg("IpcCommRelay:cacheMessageLeader", " waiting for " << msgData->count << " trailing message(s)");
	m_incompletes[msgId] = std::vector<const PlexilMsgBase*>(1, msgData);
      }
}
 
/**
 * @brief Cache following message of a multi-message sequence
 */
    
void IpcCommRelay::cacheMessageTrailer(const PlexilMsgBase* msgData)
{
    IpcMessageId msgId(msgData->senderUID, msgData->serial);
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    assertTrueMsg(it != m_incompletes.end(),
		  "IpcCommRelay::cacheMessageTrailer: no existing sequence for sender "
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
void IpcCommRelay::processMessageSequence(std::vector<const PlexilMsgBase*>& msgs)
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
      debugMsg("IpcCommRelay:handleMessageSequence", " ignoring LookupOnChange request");
      break;

      // ignore these if they get through - we don't deal with them
    case PlexilMsgType_ReturnValues:
    case PlexilMsgType_PlannerUpdate:
      debugMsg("IpcCommRelay:handleMessageSequence", " ignoring sequence starting with type " << leader->msgType);
      break;

    default:
      assertTrueMsg(ALWAYS_FAIL,
		    "IpcCommRelay::processMessageSequence: invalid leader message type " << leader->msgType);
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
 * @brief Send a command to the simulator
 */
void IpcCommRelay::processCommand(std::vector<const PlexilMsgBase*>& msgs)
{
  std::string cmdName(((const PlexilStringValueMsg*)msgs[0])->stringValue);
  IpcMessageId* transId = new IpcMessageId(msgs[0]->senderUID, msgs[0]->serial);
  m_Simulator->scheduleResponseForCommand(cmdName, static_cast<void*>(transId));
}

/**
 * @brief Deal with a LookupNow request
 */
void IpcCommRelay::processLookupNow(std::vector<const PlexilMsgBase*>& msgs)
{
  std::string stateName(((const PlexilStringValueMsg*)msgs[0])->stringValue);
  debugMsg("IpcCommRelay:lookupNow", " for " << stateName);
  if (msgs[0]->count != 0)
    debugMsg("IpcCommRelay:lookupNow",
	     " ignoring parameters for state \"" << stateName << "\"");
  IpcMessageId* transId = new IpcMessageId(msgs[0]->senderUID, msgs[0]->serial);
  ResponseMessage* response = m_Simulator->getLookupNowResponse(stateName, static_cast<void*>(transId));
  if (response != NULL)
    {
      // Simply send the response
      debugMsg("IpcCommRelay:lookupNow", " sending response for " << stateName);
    }
  else
    {
      // Create a bogus response that returns 0 values (i.e. unknown)
      debugMsg("IpcCommRelay:lookupNow", " " << stateName << " not found, returning UNKNOWN");
      static GenericResponse gr(std::vector<double>(0, 0.0));
      response = new ResponseMessage(&gr, static_cast<void*>(transId), MSG_LOOKUP);
    }
  // Simply send the response
  sendResponse(response); // deletes response
}

/**
 * @brief Deal with a LookupOnChange request
 */
void IpcCommRelay::processLookupOnChange(std::vector<const PlexilMsgBase*>& msgs)
{
  assertTrueMsg(ALWAYS_FAIL, "processLookupOnChange is not yet implemented");
}
