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

#include "IpcAdapter.hh"
#include "ipc-data-formats.h"
#include "defineIpcMessageTypes.h"
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"
#include "CoreExpressions.hh"
#include "PlexilXmlParser.hh"
#include "StoredArray.hh"
#include "ThreadSpawn.hh"

#include "tinyxml.h"

// ooid classes
#include "uuid.h"
#include "devrand.h"

namespace PLEXIL
{

  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface),
      m_sem(),
      m_serial(0),
      m_pendingLookupSerial(0),
      m_pendingLookupDestination(NULL)
  {
    initializeUID();
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const pointer to the TiXmlElement describing this adapter
   */
  IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface, 
			 const TiXmlElement * xml)
    : InterfaceAdapter(execInterface, xml),
      m_sem(),
      m_serial(0),
      m_pendingLookupSerial(0),
      m_pendingLookupDestination(NULL)
  {
    initializeUID();
  }

  /**
   * @brief Initialize unique ID string
   */
  void IpcAdapter::initializeUID()
  {
    kashmir::system::DevRand randomStream;
    kashmir::uuid_t uuid;
    randomStream >> uuid;
    std::ostringstream s;
    s << uuid;
    m_myUID = s.str();
    debugMsg("IpcAdapter:initializeUID", " generated UUID " << m_myUID);
  }


  /**
   * @brief Destructor.
   */
  IpcAdapter::~IpcAdapter()
  {
  }

  //
  // API to ExecApplication
  //

  /**
   * @brief Initializes the adapter, possibly using its configuration data.
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::initialize()
  {
    // Get taskName, serverName from XML, if supplied
    const char* taskName = NULL;
    const char* serverName = NULL;

    const TiXmlElement* xml = this->getXml();
    if (xml != NULL) 
      {
	taskName = xml->Attribute("TaskName");
	serverName = xml->Attribute("Server");
      }

    // Use defaults if necessary
    if (taskName == NULL)
      {
	taskName = m_myUID.c_str();
      }
    if(serverName != NULL)
      {
	serverName = "localhost";
      }
    debugMsg("IpcAdapter:initialize", 
	     " Connecting module " << taskName <<
	     " to central server at " << serverName);

    // Initialize IPC
    // possibly redundant, but always safe
    IPC_initialize();

    // Connect to central
    assertTrueMsg(IPC_connectModule(taskName, serverName) == IPC_OK,
		  "IpcAdapter: Unable to connect to the central server at " << serverName);

    // Define messages
    assertTrueMsg(definePlexilIPCMessageTypes(),
		  "IpcAdapter: Unable to define IPC message types");

    // *** TODO: register lookup names for getting commands & msgs ***
    // Register with AdapterExecInterface
    m_execInterface.defaultRegisterAdapter(getId());

    debugMsg("IpcAdapter:initialize", " succeeded");
    return true;
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::start()
  {
    // Spawn listener thread
    assertTrueMsg(threadSpawn((THREAD_FUNC_PTR)IPC_dispatch, NULL, m_thread),
		  "IpcAdapter: Unable to spawn IPC dispatch thread");
    debugMsg("IpcAdapter:start", " spawned IPC dispatch thread");

    // Subscribe to messages
    IPC_RETURN_TYPE status;
    status = IPC_subscribeData(MSG_BASE, messageHandler, this);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error subscribing to " << MSG_BASE << " messages, IPC_errno = " << IPC_errno);
    status = IPC_subscribeData(RETURN_VALUE_MSG, messageHandler, this);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error subscribing to " << RETURN_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_subscribeData(NUMERIC_VALUE_MSG, messageHandler, this);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error subscribing to " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_subscribeData(STRING_VALUE_MSG, messageHandler, this);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error subscribing to " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    // *** TODO: implement receiving planner update
//    status = IPC_subscribeData(NUMERIC_PAIR_MSG, messageHandler, this);
//    assertTrueMsg(status == IPC_OK,
//		  "IpcAdapter: Error subscribing to " << NUMERIC_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
//    status = IPC_subscribeData(STRING_PAIR_MSG, messageHandler, this);
//    assertTrueMsg(status == IPC_OK,
//		  "IpcAdapter: Error subscribing to " << STRING_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);

    debugMsg("IpcAdapter:start", " succeeded");
    return true;
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::stop()
  {
    // Unsubscribe from messages
    IPC_RETURN_TYPE status;
    status = IPC_unsubscribe(MSG_BASE, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error unsubscribing from " << MSG_BASE << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(RETURN_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error unsubscribing from " << RETURN_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(NUMERIC_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error unsubscribing from " << NUMERIC_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    status = IPC_unsubscribe(STRING_VALUE_MSG, NULL);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter: Error unsubscribing from " << STRING_VALUE_MSG << " messages, IPC_errno = " << IPC_errno);
    // *** TODO: implement receiving planner update
//     status = IPC_unsubscribe(NUMERIC_PAIR_MSG, NULL);
//     assertTrueMsg(status == IPC_OK,
// 		  "IpcAdapter: Error unsubscribing from " << NUMERIC_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);
//     status = IPC_unsubscribe(STRING_PAIR_MSG, NULL);
//     assertTrueMsg(status == IPC_OK,
// 		  "IpcAdapter: Error unsubscribing from " << STRING_PAIR_MSG << " messages, IPC_errno = " << IPC_errno);

    // Cancel IPC dispatch thread
    int myErrno;
    assertTrueMsg((myErrno = pthread_cancel(m_thread)) == 0,
		  "IpcAdapter: error canceling IPC dispatch thread, errno = " << myErrno); 
    assertTrueMsg((myErrno = pthread_join(m_thread, NULL)) == 0,
		  "IpcAdapter: error joining IPC dispatch thread, errno = " << myErrno); 

    debugMsg("IpcAdapter:stop", " succeeded");
    return true;
  }

  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::reset()
  {
    // No-op (?)
    return true;
  }

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::shutdown()
  {
    // Disconnect from central
    IPC_disconnect();
    debugMsg("IpcAdapter:shutdown", " succeeded");
    return true;
  }

  /**
   * @brief Register one LookupOnChange.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param tolerances A vector of tolerances for the LookupOnChange.
   */

  void IpcAdapter::registerChangeLookup(const LookupKey& uniqueId,
					const StateKey& stateKey,
					const std::vector<double>& tolerances)
  {
    State state;
    assertTrueMsg(getState(stateKey, state),
		  "IpcAdapter::registerChangeLookup: Internal error: state not found!");
    LabelStr nameLabel(state.first);
    const std::vector<double>& params = state.second;
    size_t nParams = params.size();
    debugMsg("IpcAdapter:registerChangeLookup",
	     " for state " << nameLabel.toString()
	     << " with " << nParams << " parameters");

    if (hasPrefix(nameLabel.toString(), MESSAGE_PREFIX()))
      {
	// Set up to receive this message
	m_activeMessageListeners[nameLabel.toString().substr(MESSAGE_PREFIX().size())] = stateKey;
      }
    else if (hasPrefix(nameLabel.toString(), COMMAND_PREFIX()))
      {
	// Set up to receive this command
	m_activeCommandListeners[nameLabel.toString().substr(COMMAND_PREFIX().size())] = stateKey;
      }
    else if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX()))
      {
	// Set up to receive this lookup
	m_activeLookupListeners[nameLabel.toString().substr(LOOKUP_PREFIX().size())] = stateKey;
      }
    else if (hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX()))
      {
	// Set up to receive this lookup
	m_activeChangeLookupListeners[nameLabel.toString().substr(LOOKUP_ON_CHANGE_PREFIX().size())] = stateKey;
      }
    // *** TODO: implement receiving planner update
    else
      {
	// Send a request

	// Construct the messages
	// Leader
	uint32_t serial = getSerialNumber();
	struct PlexilStringValueMsg leader = {{PlexilMsgType_LookupOnChange,
					       nParams,
					       serial,
					       m_myUID.c_str()},
					      nameLabel.c_str()};
	// Construct parameter messages
	PlexilMsgBase* paramMsgs[nParams];
	for (size_t i = 0; i < nParams; i++)
	  {
	    double param = params[i];
	    PlexilMsgBase* paramMsg;
	    if (LabelStr::isString(param))
	      {
		// string
		struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
		strMsg->stringValue = LabelStr(param).c_str();
		paramMsg = (PlexilMsgBase*) strMsg;
		paramMsg->msgType = PlexilMsgType_StringValue;
	      }
	    else if (StoredArray::isKey(param))
	      {
		// array
		assertTrueMsg(ALWAYS_FAIL,
			      "IpcAdapter: Array values are not yet implemented");
	      }
	    else
	      {
		// number or Boolean
		struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
		numMsg->doubleValue = param;
		paramMsg = (PlexilMsgBase*) numMsg;
		paramMsg->msgType = PlexilMsgType_NumericValue;
	      }

	    // Fill in common fields
	    paramMsg->count = i;
	    paramMsg->serial = serial;
	    paramMsg->senderUID = m_myUID.c_str();
	    paramMsgs[i] = paramMsg;
	  }

	// Take care of bookkeeping
	m_changeLookups[serial] = stateKey;

	// Send the messages
	IPC_RETURN_TYPE status;
	status = IPC_publishData(STRING_VALUE_MSG, (void *) &leader);
	assertTrueMsg(status == IPC_OK,
		      "IpcAdapter::registerChangeLookup: IPC Error, IPC_errno = " << IPC_errno);
	for (size_t i = 0; i < nParams; i++)
	  {
	    status = IPC_publishData(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType), 
				     paramMsgs[i]);
	    assertTrueMsg(status == IPC_OK,
			  "IpcAdapter::registerChangeLookup: IPC Error, IPC_errno = " << IPC_errno);
	  }
	// free the parameter packets
	for (size_t i = 0; i < nParams; i++)
	  {
	    PlexilMsgBase* m = paramMsgs[i];
	    paramMsgs[i] = NULL;
	    if (m->msgType == PlexilMsgType_NumericValue)
	      delete (PlexilNumericValueMsg*) m;
	    else
	      delete (PlexilStringValueMsg*) m;
	  }
      }
  }

  /**
   * @brief Terminate one LookupOnChange.
   * @param uniqueId The unique ID of the lookup to be terminated.
   */

  void IpcAdapter::unregisterChangeLookup(const LookupKey& uniqueId)
  {
    debugMsg("IpcAdapter:unregisterChangeLookup", " entered");
    StateToLookupMap::const_iterator sit = this->findLookupKey(uniqueId);
    assertTrueMsg(sit != this->getAsynchLookupsEnd(),
		  "IpcAdapter::unregisterChangeLookup: internal error: no state key registered for lookup!");

    debugMsg("IpcAdapter:unregisterChangeLookup", " found state key");

    // Ignore this unless we're removing the last change lookup for this state
    if (sit->second.size() > 1)
      {
	debugMsg("IpcAdapter:unregisterChangeLookup",
		 " there are still active change lookups for this state, ignoring");

	return;
      }

    StateKey key = sit->first;
    State state;
    getState(key, state);
    const LabelStr& nameLabel = state.first;
    const std::string& name = nameLabel.toString();

    debugMsg("IpcAdapter:unregisterChangeLookup", " for " << name);

    if (hasPrefix(nameLabel.toString(), MESSAGE_PREFIX()))
      {
	// Stop looking for this message
	ActiveListenerMap::iterator it = m_activeMessageListeners.find(name.substr(MESSAGE_PREFIX().size()));
	assertTrueMsg(it != m_activeMessageListeners.end(),
		      "IpcAdapter::unregisterChangeLookup: internal error: can't find message \""
		      << name.substr(MESSAGE_PREFIX().size()) << "\"");
	m_activeMessageListeners.erase(it);
      }
    else if (hasPrefix(nameLabel.toString(), COMMAND_PREFIX()))
      {
	// Stop looking for this command
	ActiveListenerMap::iterator it = m_activeCommandListeners.find(name.substr(COMMAND_PREFIX().size()));
	assertTrueMsg(it != m_activeCommandListeners.end(),
		      "IpcAdapter::unregisterChangeLookup: internal error: can't find command \""
		      << name.substr(COMMAND_PREFIX().size()) << "\"");
	m_activeCommandListeners.erase(it);
      }
    else if (hasPrefix(nameLabel.toString(), LOOKUP_PREFIX()))
      {
	// Stop looking for this lookup
	ActiveListenerMap::iterator it = m_activeLookupListeners.find(name.substr(LOOKUP_PREFIX().size()));
	assertTrueMsg(it != m_activeLookupListeners.end(),
		      "IpcAdapter::unregisterChangeLookup: internal error: can't find lookup \""
		      << name.substr(LOOKUP_PREFIX().size()) << "\"");
	m_activeLookupListeners.erase(it);
      }
    else if (hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX()))
      {
	// Stop looking for this lookup
	ActiveListenerMap::iterator it =
	  m_activeChangeLookupListeners.find(name.substr(LOOKUP_ON_CHANGE_PREFIX().size()));
	assertTrueMsg(it != m_activeChangeLookupListeners.end(),
		      "IpcAdapter::unregisterChangeLookup: internal error: can't find change lookup \""
		      << name.substr(LOOKUP_ON_CHANGE_PREFIX().size()) << "\"");
	m_activeChangeLookupListeners.erase(it);
      }
    // *** TODO: implement receiving planner update
    else
      {
	// send a TerminateChangeLookup message with the serial #
	// of the original request
	debugMsg("IpcAdapter:unregisterChangeLookup", " sending TerminateChangeLookupMessage");
	IpcChangeLookupMap::iterator it = m_changeLookups.begin();
        while (it != m_changeLookups.end())
	  {
	    if (it->second == key)
	      break;
	    it++;
	  }
	assertTrueMsg(it != m_changeLookups.end(),
		      "IpcAdapter::unregisterChangeLookup: internal error: lookup not found");
	const uint32_t serial = it->first;
	PlexilMsgBase msg = {PlexilMsgType_TerminateChangeLookup,
			     0,
			     serial,
			     m_myUID.c_str()};
	IPC_RETURN_TYPE status;
	status = IPC_publishData(MSG_BASE,(void*) &msg);
	assertTrueMsg(status == IPC_OK,
		      "IpcAdapter::unregisterChangeLookup: IPC Error, IPC_errno = " << IPC_errno);

	// clean up table
	m_changeLookups.erase(it);
      }
    debugMsg("IpcAdapter:unregisterChangeLookup", " completed");
  }

  /**
   * @brief Perform an immediate lookup of the requested state.
   * @param stateKey The state key for this lookup.
   * @param dest A (reference to a) vector of doubles where the result is to be stored.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::lookupNow(const StateKey& stateKey,
			     std::vector<double>& dest)
  {
    State state;
    assertTrueMsg(getState(stateKey, state),
		  "IpcAdapter::lookupNow: Internal error: state not found!");
    LabelStr nameLabel(state.first);
    const std::vector<double>& params = state.second;
    size_t nParams = params.size();
    debugMsg("IpcAdapter:lookupNow",
	     " for state " << nameLabel.toString()
	     << " with " << nParams << " parameters");

    // filter out command, message, lookup lookups here - 
    // they only have significance to LookupOnChange
    // *** TODO: implement receiving (ignoring) planner update
    if (hasPrefix(nameLabel.toString(), MESSAGE_PREFIX())
	|| hasPrefix(nameLabel.toString(), COMMAND_PREFIX())
	|| hasPrefix(nameLabel.toString(), LOOKUP_PREFIX())
	|| hasPrefix(nameLabel.toString(), LOOKUP_ON_CHANGE_PREFIX()))
      {
	debugMsg("IpcAdapter:lookupNow",
		 " ignoring special state name \"" << nameLabel.toString() << "\"");
	return;
      }

    // Construct the messages
    // Leader
    uint32_t serial = getSerialNumber();
    struct PlexilStringValueMsg leader = {{PlexilMsgType_LookupNow,
					   nParams,
					   serial,
					   m_myUID.c_str()},
					  nameLabel.c_str()};
    // Construct parameter messages
    PlexilMsgBase* paramMsgs[nParams];
    for (size_t i = 0; i < nParams; i++)
      {
	double param = params[i];
	PlexilMsgBase* paramMsg;
	if (LabelStr::isString(param))
	  {
	    // string
	    struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
	    strMsg->stringValue = LabelStr(param).c_str();
	    paramMsg = (PlexilMsgBase*) strMsg;
	    paramMsg->msgType = PlexilMsgType_StringValue;
	  }
	else if (StoredArray::isKey(param))
	  {
	    // array
	    assertTrueMsg(ALWAYS_FAIL,
			  "IpcAdapter: Array values are not yet implemented");
	  }
	else
	  {
	    // number or Boolean
	    struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
	    numMsg->doubleValue = param;
	    paramMsg = (PlexilMsgBase*) numMsg;
	    paramMsg->msgType = PlexilMsgType_NumericValue;
	  }

	// Fill in common fields
	paramMsg->count = i;
	paramMsg->serial = serial;
	paramMsg->senderUID = m_myUID.c_str();
	paramMsgs[i] = paramMsg;
      }

    assertTrueMsg(m_pendingLookupSerial == 0,
		  "IpcAdapter::lookupNow: fatal error: reentrant call");

    // Send the messages
    m_pendingLookupSerial = serial;
    m_pendingLookupDestination = &dest;
    IPC_RETURN_TYPE status;
    status = IPC_publishData(STRING_VALUE_MSG, (void *) &leader);
    assertTrueMsg(status == IPC_OK,
		  "IpcAdapter::lookupNow: IPC Error, IPC_errno = " << IPC_errno);
    for (size_t i = 0; i < nParams; i++)
      {
	status = IPC_publishData(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType),
				 paramMsgs[i]);
	assertTrueMsg(status == IPC_OK,
		      "IpcAdapter::lookupNow: IPC Error, IPC_errno = " << IPC_errno);
      }

    // free the parameter packets
    for (size_t i = 0; i < nParams; i++)
      {
	PlexilMsgBase* m = paramMsgs[i];
	paramMsgs[i] = NULL;
	if (m->msgType == PlexilMsgType_NumericValue)
	  delete (PlexilNumericValueMsg*) m;
	else
	  delete (PlexilStringValueMsg*) m;
      }

    // Wait for results
    // *** TODO: check for error
    m_sem.wait();

    // Clean up
    m_pendingLookupSerial = 0;
    m_pendingLookupDestination = NULL;
  }


  /**
   * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
   * @param node The Node requesting the update.
   * @param valuePairs A map of <LabelStr key, value> pairs.
   * @param ack The expression in which to store an acknowledgement of completion.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::sendPlannerUpdate(const NodeId& node,
				     const std::map<double, double>& valuePairs,
				     ExpressionId ack)
  {
    assertTrueMsg(ALWAYS_FAIL,
		  "IpcAdapter: sendPlannerUpdate is not yet implemented");
  }

  /**
   * @brief Execute a command with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the command.
   * @param ack The expression in which to store an acknowledgement of command transmission.
   */

  void IpcAdapter::executeCommand(const LabelStr& name,
				  const std::list<double>& args,
				  ExpressionId dest,
				  ExpressionId ack)
  {
    // Check for SendMessage command
    if (name == SEND_MESSAGE_COMMAND())
      {
	// Check for one argument, the message
	assertTrueMsg(args.size() == 1,
		      "IpcAdapter: The SendMessage command requires exactly one argument");
	assertTrueMsg(LabelStr::isString(args.front()),
		      "IpcAdapter: The argument to the SendMessage command, " << args.front()
		      << ", is not a string");
	LabelStr theMessage(args.front());
	debugMsg("IpcAdapter:executeCommand",
		 " SendMessage(\"" << theMessage.c_str() << "\")");
	struct PlexilStringValueMsg packet = {{PlexilMsgType_Message,
					       0,
					       getSerialNumber(),
					       m_myUID.c_str()},
					      theMessage.c_str()};
	IPC_publishData(STRING_VALUE_MSG, (void *) &packet);
	// store ack
	m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
	m_execInterface.notifyOfExternalEvent();
	debugMsg("IpcAdapter:executeCommand", " message \"" << theMessage.c_str() << "\" sent.");
      }
    // Check for SendReturnValue command
    else if (name == SEND_RETURN_VALUE_COMMAND())
      {
	// Check for one argument, the message
	assertTrueMsg(args.size() >= 2,
		      "IpcAdapter: The SendReturnValue command requires at least two arguments.");

	// *** TODO: Implement!
	assertTrueMsg(LabelStr::isString(args.front()),
		      "IpcAdapter: The first argument to the SendReturnValue command, " << args.front()
		      << ", is not a string");
	LabelStr receiver(args.front());
	debugMsg("IpcAdapter:executeCommand",
		 " SendReturnValue(\"" << receiver.c_str() << "\")");
	struct PlexilStringValueMsg packet = {{PlexilMsgType_Message,
					       0,
					       getSerialNumber(),
					       m_myUID.c_str()},
					      receiver.c_str()};
	IPC_publishData(STRING_VALUE_MSG, (void *) &packet);
	// store ack
	m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SUCCESS().getKey());
	m_execInterface.notifyOfExternalEvent();
	debugMsg("IpcAdapter:executeCommand", " return value sent.");
      }
    else // general case
      {
	debugMsg("IpcAdapter:executeCommand", " for \"" << name.c_str()
		 << "\", destination expression is " << dest);
	uint32_t serial = getSerialNumber();
	size_t nParams = args.size();
	struct PlexilStringValueMsg cmdPacket = {{PlexilMsgType_Command,
						  nParams,
						  serial,
						  m_myUID.c_str()},
						 name.c_str()};
	// Construct parameter messages
	PlexilMsgBase* paramMsgs[nParams];
	unsigned int i = 0;
	for (std::list<double>::const_iterator it = args.begin();
	     it != args.end();
	     it++, i++)
	  {
	    double param = *it;
	    PlexilMsgBase* paramMsg;
	    if (LabelStr::isString(param))
	      {
		// string
		struct PlexilStringValueMsg* strMsg = new PlexilStringValueMsg();
		strMsg->stringValue = LabelStr(param).c_str();
		paramMsg = (PlexilMsgBase*) strMsg;
		paramMsg->msgType = PlexilMsgType_StringValue;
	      }
	    else if (StoredArray::isKey(param))
	      {
		// array
		assertTrueMsg(ALWAYS_FAIL,
			      "IpcAdapter: Array values are not yet implemented");
	      }
	    else
	      {
		// number or Boolean
		struct PlexilNumericValueMsg* numMsg = new PlexilNumericValueMsg();
		numMsg->doubleValue = param;
		paramMsg = (PlexilMsgBase*) numMsg;
		paramMsg->msgType = PlexilMsgType_NumericValue;
	      }

	    // Fill in common fields
	    paramMsg->count = i;
	    paramMsg->serial = serial;
	    paramMsg->senderUID = m_myUID.c_str();
	    paramMsgs[i] = paramMsg;
	  }

	// Send the messages
	// *** TODO: check for IPC errors ***
	IPC_RETURN_TYPE status;
	status = IPC_publishData(STRING_VALUE_MSG, (void *) &cmdPacket);
	assertTrueMsg(status == IPC_OK,
		      "IpcAdapter::executeCommand: IPC Error, IPC_errno = " << IPC_errno);
	for (size_t i = 0; i < nParams; i++)
	  {
	    status = IPC_publishData(msgFormatForType((PlexilMsgType) paramMsgs[i]->msgType),
				     paramMsgs[i]);
	    assertTrueMsg(status == IPC_OK,
			  "IpcAdapter::executeCommand: IPC Error, IPC_errno = " << IPC_errno);
	  }

	// log ack and return variables in case we get values for them
	m_pendingCommands[serial] = std::pair<ExpressionId, ExpressionId>(dest, ack);
	// store ack
	m_execInterface.handleValueChange(ack, CommandHandleVariable::COMMAND_SENT_TO_SYSTEM().getKey());
	m_execInterface.notifyOfExternalEvent();
	debugMsg("IpcAdapter:executeCommand", " command \"" << name.c_str() << "\" sent.");
      }
  }

  /**
   * @brief Abort the pending command with the supplied name and arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param ack The expression in which to store an acknowledgement of command abort.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::invokeAbort(const LabelStr& name, 
			       const std::list<double>& args, 
			       ExpressionId ack)
  {
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter::invokeAbort is not yet implemented");
  }

  //
  // Implementation methods
  //

  /**
   * @brief Handler function as seen by IPC.
   */

  // *** TODO: confirm correct message formats!! ***

  void IpcAdapter::messageHandler(MSG_INSTANCE rawMsg,
				  void * unmarshalledMsg,
				  void * this_as_void_ptr)
  {
    // Check whether the thread has been canceled before going any further
    pthread_testcancel();

    IpcAdapter* theAdapter = reinterpret_cast<IpcAdapter*>(this_as_void_ptr);
    assertTrueMsg(theAdapter != NULL,
		  "IpcAdapter::messageHandler: pointer to adapter instance is null!");

    const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*>(unmarshalledMsg);
    assertTrueMsg(msgData != NULL,
		  "IpcAdapter::messageHandler: pointer to message data is null!");

    theAdapter->handleIpcMessage(msgData);
  }

  /**
   * @brief Handler function as seen by adapter.
   */

  // *** TODO: confirm correct message formats!! ***

  void IpcAdapter::handleIpcMessage(const PlexilMsgBase * msgData)
  {
    if (strcmp(msgData->senderUID, m_myUID.c_str()) == 0)
      {
	debugMsg("IpcAdapter:handleIpcMessage", " ignoring my own outgoing message");
	return;
      }

    PlexilMsgType msgType = (PlexilMsgType) msgData->msgType;
    debugMsg("IpcAdapter:handleIpcMessage", " received message type = " << msgType);
    switch (msgType)
      {
	// NotifyExec is a PlexilMsgBase
      case PlexilMsgType_NotifyExec:

	// AddPlan/AddPlanFile is a PlexilStringValueMsg
      case PlexilMsgType_AddPlan:
      case PlexilMsgType_AddPlanFile:

 	// AddLibrary/AddLibraryFile is a PlexilStringValueMsg
      case PlexilMsgType_AddLibrary:
      case PlexilMsgType_AddLibraryFile:

	// In each case, simply send it to Exec and free the message
	enqueueMessage(msgData);
	break;

	// Command is a PlexilStringValueMsg
	// Optionally followed by parameters

      case PlexilMsgType_Command:
	// Stash this and wait for the rest
	debugMsg("IpcAdapter:handleIpcMessage", " processing as command");
	cacheMessageLeader(msgData);
	break;

	// Message is a PlexilStringValueMsg
	// No parameters

      case PlexilMsgType_Message:
	debugMsg("IpcAdapter:handleIpcMessage", " processing as message");
	handleMessageMessage((const PlexilStringValueMsg*) msgData);
	break;

	// LookupNow and LookupOnChange are PlexilStringValueMsg
	// Optionally followed by parameters

	// TODO: filter out commands/msgs we aren't prepared to handle
      case PlexilMsgType_LookupNow:
      case PlexilMsgType_LookupOnChange:
	// Stash this and wait for the rest
	cacheMessageLeader(msgData);
	break;

	// PlannerUpdate is a PlexilStringValueMsg
	// Followed by 0 (?) or more name/value pairs
      case PlexilMsgType_PlannerUpdate:
	// Stash this and wait for the rest
	cacheMessageLeader(msgData);
	break;

	// TelemetryValues is a PlexilStringValueMsg
	// Followed by 0 (?) or more values
      case PlexilMsgType_TelemetryValues:
	{
	  const PlexilStringValueMsg* tv = (const PlexilStringValueMsg*) msgData;
	  State state(LabelStr(tv->stringValue), std::vector<double>(0));
	  StateKey dummy;
	  // is this a state the exec knows about?
	  if (m_execInterface.findStateKey(state, dummy))
	    {
	      // stash this and wait for the rest
	      debugMsg("IpcAdapter:handleIpcMessage", " processing as telemetry value");
	      cacheMessageLeader(msgData);
	    }
	  else
	    {
	      debugMsg("IpcAdapter:handleIpcMessage",
		       " ignoring unknown state \"" << tv->stringValue << "\"");
	    }
	  break;
	}

	// ReturnValues is a PlexilReturnValuesMsg
	// Followed by 0 (?) or more values
      case PlexilMsgType_ReturnValues:
	{
	  // Only pay attention to our return values
	  debugMsg("IpcAdapter:handleIpcMessage", " processing as return value");
	  const PlexilReturnValuesMsg* returnLeader = (const PlexilReturnValuesMsg*) msgData;
	  if (strcmp(returnLeader->requesterUID, m_myUID.c_str()) == 0)
	    cacheMessageLeader(msgData);
	  break;
	}

	// Values - could be parameters or return values
      case PlexilMsgType_NumericValue:
      case PlexilMsgType_StringValue:

	// PlannerUpdate pairs
      case PlexilMsgType_PairNumeric:
      case PlexilMsgType_PairString:

	// Log with corresponding leader message
	cacheMessageTrailer(msgData);
	break;
			  
      default:
	assertTrueMsg(ALWAYS_FAIL,
		      "IpcAdapter::handleIpcMessage: received invalid message data type " << msgType);
	break;
      }

  }
 
  /**
   * @brief Cache start message of a multi-message sequence
   */

  // N.B. Presumes that messages are received in order.
  // Also presumes that any required filtering (e.g. on command name) has been done by the caller
    
  void IpcAdapter::cacheMessageLeader(const PlexilMsgBase* msgData)
  {
    IpcMessageId msgId(msgData->senderUID, msgData->serial);

    // Check that this isn't a duplicate header
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    assertTrueMsg(it == m_incompletes.end(),
		  "IpcAdapter::cacheMessageLeader: internal error: found existing sequence for sender "
		  << msgData->senderUID << ", serial " << msgData->serial);

    if (msgData->count == 0)
      {
	debugMsg("IpcAdapter:cacheMessageLeader", " count == 0, processing immediately");
	std::vector<const PlexilMsgBase*> msgVec(1, msgData);
	enqueueMessageSequence(msgVec);
      }
    else
      {
	debugMsg("IpcAdapter:cacheMessageLeader",
		 " storing leader with sender " << msgData->senderUID << ", serial " << msgData->serial
		 << ",\n expecting " << msgData->count << " values");
	m_incompletes[msgId] = std::vector<const PlexilMsgBase*>(1, msgData);
      }
  }


  /**
   * @brief Cache following message of a multi-message sequence
   */

  // N.B. Presumes that messages are received in order.
    
  void IpcAdapter::cacheMessageTrailer(const PlexilMsgBase* msgData)
  {
    IpcMessageId msgId(msgData->senderUID, msgData->serial);
    IncompleteMessageMap::iterator it = m_incompletes.find(msgId);
    if (it == m_incompletes.end())
      {
	debugMsg("IpcAdapter::cacheMessageTrailer",
		 " no existing sequence for sender "
		 << msgData->senderUID << ", serial " << msgData->serial << ", ignoring");
	return;
      }
    std::vector<const PlexilMsgBase*>& msgs = it->second;
    msgs.push_back(msgData);
    // Have we got them all?
    if (msgs.size() > msgs[0]->count)
      {
	enqueueMessageSequence(msgs);
	m_incompletes.erase(it);
      }
  }


  /**
   * @brief Send a single message to the Exec's queue and free the message
   */
  void IpcAdapter::enqueueMessage(const PlexilMsgBase* msgData)
  {
    assertTrueMsg(msgData != NULL,
		  "IpcAdapter::enqueueMessage: msgData is null");

    switch (msgData->msgType)
      {
      case PlexilMsgType_NotifyExec:
	m_execInterface.notifyOfExternalEvent();
	break;

	// AddPlan is a PlexilStringValueMsg
      case PlexilMsgType_AddPlan:
	{
	  const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
	  assertTrueMsg(stringMsg->stringValue != NULL,
			"IpcAdapter::enqueueMessage: AddPlan message contains null plan string");

	  // parse XML into node structure
	  PlexilXmlParser p;
	  try
	    {
	      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), false);
	      m_execInterface.handleAddPlan(root, EMPTY_LABEL());
	      // Always notify immediately when adding a plan
	      m_execInterface.notifyOfExternalEvent();
	    }
	  catch (const ParserException& e)
	    {
	      std::cerr << "Error parsing plan: \n" << e.what() << std::endl;
	    }
	}

	// AddPlanFile is a PlexilStringValueMsg
      case PlexilMsgType_AddPlanFile:
	{
	  const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
	  assertTrueMsg(stringMsg->stringValue != NULL,
			"IpcAdapter::enqueueMessage: AddPlanFile message contains null file name");

	  // parse XML into node structure
	  PlexilXmlParser p;
	  try
	    {
	      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), true);
	      m_execInterface.handleAddPlan(root, EMPTY_LABEL());
	      // Always notify immediately when adding a plan
	      m_execInterface.notifyOfExternalEvent();
	    }
	  catch (const ParserException& e)
	    {
	      std::cerr << "Error parsing plan from file: \n" << e.what() << std::endl;
	    }
	}
	break;

 	// AddLibrary is a PlexilStringValueMsg
      case PlexilMsgType_AddLibrary:
	{
	  const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
	  assertTrueMsg(stringMsg->stringValue != NULL,
			"IpcAdapter::enqueueMessage: AddLibrary message contains null library node string");

	  // parse XML into node structure
	  PlexilXmlParser p;
	  try
	    {
	      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), false);
	      m_execInterface.handleAddLibrary(root);
	    }
	  catch (const ParserException& e)
	    {
	      std::cerr << "Error parsing library node: \n" << e.what() << std::endl;
	    }
	}
	break;

 	// AddLibraryFile is a PlexilStringValueMsg
      case PlexilMsgType_AddLibraryFile:
	{
	  const PlexilStringValueMsg* stringMsg = (const PlexilStringValueMsg*) msgData;
	  assertTrueMsg(stringMsg->stringValue != NULL,
			"IpcAdapter::enqueueMessage: AddLibraryFile message contains null file name");

	  // parse XML into node structure
	  PlexilXmlParser p;
	  try
	    {
	      PlexilNodeId root = p.parse(std::string(stringMsg->stringValue), true);
	      m_execInterface.handleAddLibrary(root);
	    }
	  catch (const ParserException& e)
	    {
	      std::cerr << "Error parsing library file: \n" << e.what() << std::endl;
	    }
	}
	break;

      default:
	assertTrueMsg(ALWAYS_FAIL,
		      "IpcAdapter::enqueueMessage: invalid message type " << msgData->msgType);
      }

    // clean up
    IPC_freeData(IPC_msgFormatter(msgFormatForType((PlexilMsgType) msgData->msgType)),
		 (void*) msgData);
  }

  /**
   * @brief Process a PlexilMsgType_Message packet and free the message
   */
  void IpcAdapter::handleMessageMessage(const PlexilStringValueMsg* msgData)
  {
    assertTrueMsg(msgData != NULL,
		  "IpcAdapter::handleMessageMessage: msgData is null")
    assertTrueMsg(msgData->stringValue != NULL,
		  "IpcAdapter::handleMessageMessage: stringValue is null")
    ActiveListenerMap::const_iterator it = m_activeMessageListeners.find(msgData->stringValue);
    if (it != m_activeMessageListeners.end())
      {
	std::vector<double> valueVector(1, BooleanVariable::TRUE());
	const StateKey& key = it->second;
	m_execInterface.handleValueChange(key, valueVector);
	m_execInterface.notifyOfExternalEvent();
      }

    // clean up
    IPC_freeData(IPC_msgFormatter(STRING_VALUE_MSG), (void*) msgData);
  }


  /**
   * @brief Send a message sequence to the Exec's queue and free the messages
   */

  // *** TODO: implement! ***
  void IpcAdapter::enqueueMessageSequence(std::vector<const PlexilMsgBase*>& msgs)
  {
    const PlexilMsgBase* leader = msgs[0];
    switch (leader->msgType)
      {

      case PlexilMsgType_Command:
	break;

      case PlexilMsgType_LookupNow:
	break;

      case PlexilMsgType_LookupOnChange:
	break;

      case PlexilMsgType_TelemetryValues:
	handleTelemetryValuesSequence(msgs);
	break;

      case PlexilMsgType_ReturnValues:
	handleReturnValuesSequence(msgs);
	break;

      case PlexilMsgType_PlannerUpdate:
	break;

      default:
	assertTrueMsg(ALWAYS_FAIL,
		      "IpcAdapter::enqueueMessageSequence: invalid leader message type " << leader->msgType);
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
   * @brief Process a TelemetryValues message sequence
   */

  void IpcAdapter::handleTelemetryValuesSequence(std::vector<const PlexilMsgBase*>& msgs)
  {
    const PlexilStringValueMsg* tv = (const PlexilStringValueMsg*) msgs[0];
    State state(LabelStr(tv->stringValue), std::vector<double>(0));
    StateKey key;
    if (!m_execInterface.findStateKey(state, key))
      {
	debugMsg("IpcAdapter:handleTelemetryValuesSequence", 
		 " state \"" << tv->stringValue << "\" is unknown, ignoring");
	return;
      }

    size_t nValues = msgs[0]->count;
    std::vector<double> values(nValues);
    for (size_t i = 1; i <= nValues; i++)
      {
	if (msgs[i]->msgType == PlexilMsgType_NumericValue)
	  values[i-1] = ((const PlexilNumericValueMsg*) msgs[i])->doubleValue;
	else
	  values[i-1] =
	    LabelStr(((const PlexilStringValueMsg*) msgs[i])->stringValue).getKey();
      }
    m_execInterface.handleValueChange(key, values);
    m_execInterface.notifyOfExternalEvent();
  }

  /**
   * @brief Process a ReturnValues message sequence
   */

  void IpcAdapter::handleReturnValuesSequence(std::vector<const PlexilMsgBase*>& msgs)
  {
    const PlexilReturnValuesMsg* rv = (const PlexilReturnValuesMsg*) msgs[0];
    // Was this our request?
    if (strcmp(rv->requesterUID, m_myUID.c_str()) == 0)
      {
	size_t nValues = msgs[0]->count;
	if (rv->requestSerial == m_pendingLookupSerial)
	  {
	    // LookupNow for which we are awaiting data
	    debugMsg("IpcAdapter:handleReturnValuesSequence",
		     " processing value(s) for a pending LookupNow");
	    size_t nExpected = m_pendingLookupDestination->size();
	    assertTrueMsg(nExpected == 1,
			  "IpcAdapter::handleReturnValuesSequence: Exec expects " << nExpected
			  << " values; multiple return values for LookupNow not yet implemented");
	    (*m_pendingLookupDestination)[0] = parseReturnValues(msgs);
	    // *** TODO: check for error
	    m_sem.post();
	    return;
	  }

	IpcChangeLookupMap::const_iterator it = m_changeLookups.find(rv->requestSerial);
	if (it != m_changeLookups.end())
	  {
	    // Active LookupOnChange
	    debugMsg("IpcAdapter:handleReturnValuesSequence",
		     " processing value(s) for an active LookupOnChange");
	    const StateKey& key = it->second;
	    m_execInterface.handleValueChange(key, parseReturnValues(msgs));
	    m_execInterface.notifyOfExternalEvent();
	    return;
	  }

	PendingCommandsMap::iterator cit = m_pendingCommands.find(rv->requestSerial);
	if (cit != m_pendingCommands.end())
	  {
	    // It's a command return value or ack
	    ExpressionId& dest = cit->second.first;
	    ExpressionId& ack = cit->second.second;
	    size_t nValues = msgs[0]->count;
	    if (msgs[1]->count == MSG_COUNT_CMD_ACK)
	      {
		assertTrueMsg(nValues == 1, 
			      "IpcAdapter::handleReturnValuesSequence: command ack requires 1 value, received "
			      << nValues);
		debugMsg("IpcAdapter:handleReturnValuesSequence", 
			 " processing command acknowledgment for expression " << dest);
		m_execInterface.handleValueChange(ack, parseReturnValues(msgs));
		m_execInterface.notifyOfExternalEvent();
	      }
	    else if (dest.isId())
	      {
		debugMsg("IpcAdapter:handleReturnValuesSequence", 
			 " processing command return value for expression " << dest);
		m_execInterface.handleValueChange(dest, parseReturnValues(msgs));
		m_execInterface.notifyOfExternalEvent();
	      }
	    else
	      {
		debugMsg("IpcAdapter:handleReturnValuesSequence", 
			 " ignoring unwanted command return value");
	      }
	  }
	else
	  {
	    debugMsg("IpcAdapter:handleReturnValuesSequence",
		     " no lookup or command found for sequence");
	  }
      }
  }


  /**
   * @brief Given a sequence of messages, turn the trailers into a double value for the Exec.
   */
  double IpcAdapter::parseReturnValues(std::vector<const PlexilMsgBase*>& msgs)
  {
    size_t nValues = msgs[0]->count;
    if (nValues == 1)
      {
	debugMsg("IpcAdapter:handleReturnValuesSequence", 
		 " processing single return value");
	double value;
	if (msgs[1]->msgType == PlexilMsgType_NumericValue)
	  value = ((PlexilNumericValueMsg*) msgs[1])->doubleValue;
	else
	  value = LabelStr(((PlexilStringValueMsg*) msgs[1])->stringValue).getKey();
	return value;
      }
    else
      {
	// Convert sequence of values into an array
	debugMsg("IpcAdapter:parseReturnValues", 
		 " processing array of length " << nValues);
	bool isString = false;
	bool isNumeric = false;
	for (size_t i = 1; i <= nValues; i++)
	  {
	    if (msgs[i]->msgType == PlexilMsgType_StringValue)
	      {
		assertTrueMsg(!isNumeric, "IpcAdapter::parseReturnValues: array element types are not consistent");
		isString = true;
	      }
	    else
	      {
		assertTrueMsg(!isString, "IpcAdapter::parseReturnValues: array element types are not consistent");
		isNumeric = true;
	      }
	  }
	StoredArray ary(nValues, Expression::UNKNOWN());
	if (isString)
	  {
	    for (size_t i = 0; i < nValues; i++)
	      {
		assertTrueMsg(msgs[i+1]->msgType == PlexilMsgType_StringValue,
			      "IpcAdapter:handleReturnValueSequence: value is not a string");
		ary[i] = LabelStr(((PlexilStringValueMsg*) msgs[i+1])->stringValue).getKey();
	      }
	  }
	else
	  {
	    for (size_t i = 0; i < nValues; i++)
	      {
		assertTrueMsg(msgs[i+1]->msgType == PlexilMsgType_NumericValue,
			      "IpcAdapter:handleReturnValueSequence: value is not a number");
		ary[i] = ((PlexilNumericValueMsg*) msgs[i+1])->doubleValue;
	      }
	  }
	return ary.getKey();
      }
  }

  /**
   * @brief Get next serial number
   */
  uint32_t IpcAdapter::getSerialNumber()
  {
    m_serial++;
    // handle wraparound
    if (m_serial == 0)
      m_serial = 1;
    return m_serial;
  }


  //
  // Static member functions
  //

  /**
   * @brief Returns true if the string starts with the prefix, false otherwise.
   */
  bool IpcAdapter::hasPrefix(const std::string& s, const std::string& prefix)
  {
    if (s.size() < prefix.size())
	return false;
    return (0 == s.compare(0, prefix.size(), prefix));
  }
  
  /**
   * @brief Generate a transaction ID string combining the given UID and serial
   */
  std::string IpcAdapter::makeTransactionID(const std::string& uid, uint32_t serial)
  {
    std::ostringstream s;
    s << uid << TRANSACTION_ID_SEPARATOR_CHAR << serial;
    return s.str();
  }

    /**
     * @brief Given a transaction ID string, return the UID and the serial
     */
  void IpcAdapter::parseTransactionId(const std::string& transId, std::string& uidOut, uint32_t& serialOut)
  {
    std::string::size_type idx = transId.find(TRANSACTION_ID_SEPARATOR_CHAR);
    assertTrueMsg(idx != std::string::npos,
		  "parseTransactionId: string \"" << transId << "\" is not a valid transaction ID");
    uidOut = transId.substr(0, idx);
    std::istringstream s(transId.substr(idx + 1));
    s >> serialOut;
  }

}
