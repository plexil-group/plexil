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
#include "AdapterExecInterface.hh"
#include "Debug.hh"
#include "Error.hh"

namespace PLEXIL
{

  /**
   * @brief Constructor.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   */
  IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface)
    : InterfaceAdapter(execInterface)
  {
  }

  /**
   * @brief Constructor from configuration XML.
   * @param execInterface Reference to the parent AdapterExecInterface object.
   * @param xml A const pointer to the TiXmlElement describing this adapter
   */
  IpcAdapter::IpcAdapter(AdapterExecInterface& execInterface, 
			 const TiXmlElement * xml)
    : InterfaceAdapter(execInterface, xml)
  {
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
    // Initialize IPC

    // Connect to central

    return true;
  }

  /**
   * @brief Starts the adapter, possibly using its configuration data.  
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::start()
  {
    // Subscribe to messages

    return true;
  }

  /**
   * @brief Stops the adapter.  
   * @return true if successful, false otherwise.
   */
  bool IpcAdapter::stop()
  {
    // Unsubscribe from messages

    return true;
  }

  /**
   * @brief Resets the adapter.  
   * @return true if successful, false otherwise.
   * @note Adapters should provide their own methods.  The default method simply returns true.
   */
  bool IpcAdapter::reset()
  {
    // No-op (?)
    return true;
  }

  /**
   * @brief Shuts down the adapter, releasing any of its resources.
   * @return true if successful, false otherwise.
   * @note Adapters should provide their own methods.  The default method simply returns true.
   */
  bool IpcAdapter::shutdown()
  {
    // Disconnect from central

    return true;
  }

  /**
   * @brief Register one LookupOnChange.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param tolerances A vector of tolerances for the LookupOnChange.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::registerChangeLookup(const LookupKey& uniqueId,
					const StateKey& stateKey,
					const std::vector<double>& tolerances)
  {

  }

  /**
   * @brief Terminate one LookupOnChange.
   * @param uniqueId The unique ID of the lookup to be terminated.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::unregisterChangeLookup(const LookupKey& uniqueId)
  {
  }

  /**
   * @brief Register one LookupWithFrequency.
   * @param uniqueId The unique ID of this lookup.
   * @param stateKey The state key for this lookup.
   * @param lowFrequency The maximum interval in seconds between lookups.
   * @param highFrequency The minimum interval in seconds between lookups.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::registerFrequencyLookup(const LookupKey& uniqueId,
					   const StateKey& stateKey,
					   double lowFrequency, 
					   double highFrequency)
  {
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter::registerFrequencyLookup is not implemented");
  }

  /**
   * @brief Terminate one LookupWithFrequency.
   * @param uniqueId The unique ID of the lookup to be terminated.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::unregisterFrequencyLookup(const LookupKey& uniqueId)
  {
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter::unregisterFrequencyLookup is not implemented");
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
  }

  /**
   * @brief Execute a command with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the command.
   * @param ack The expression in which to store an acknowledgement of command transmission.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::executeCommand(const LabelStr& name,
				  const std::list<double>& args,
				  ExpressionId dest,
				  ExpressionId ack)
  {
  }

  /**
   * @brief Execute a function with the requested arguments.
   * @param name The LabelString representing the command name.
   * @param args The command arguments expressed as doubles.
   * @param dest The expression in which to store any value returned from the function.
   * @param ack The expression in which to store an acknowledgement of function transmission.
   * @note Derived classes may implement this method.  The default method causes an assertion to fail.
   */

  void IpcAdapter::executeFunctionCall(const LabelStr& name,
				       const std::list<double>& args,
				       ExpressionId dest,
				       ExpressionId ack)
  {
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter::executeFunctionCall is not implemented");
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
    assertTrueMsg(ALWAYS_FAIL, "IpcAdapter::invokeAbort is not implemented");
  }

  //
  // Implementation methods
  //

  /**
   * @brief Handler function as seen by IPC.
   */

  void IpcAdapter::messageHandler(MSG_INSTANCE rawMsg,
				  void * unmarshalledMsg,
				  void * this_as_void_ptr)
  {
    IpcAdapter* theAdapter = reinterpret_cast<IpcAdapter*>(this_as_void_ptr);
    assertTrueMsg(theAdapter != NULL,
		  "IpcAdapter::messageHandler: pointer to adapter instance is null!");

    const PlexilMsgBase* msgData = reinterpret_cast<const PlexilMsgBase*>(unmarshalledMsg);
    assertTrueMsg(msgData != NULL,
		  "IpcAdapter::messageHandler: pointer to message data is null!");

    theAdapter->handleIpcMessage(msgData);
    IPC_freeData(unmarshalledMsg);
  }

  /**
   * @brief Handler function as seen by adapter.
   */

  void IpcAdapter::handleIpcMessage(const PlexilMsgBase * msgData)
  {
    if (strcmp(msgData->senderUID, m_myUID) == 0)
      {
	debugMsg("IpcAdapter::handleIpcMessage", " ignoring my own outgoing message");
	return;
      }

    PlexilMsgType msgType = msgData->msgType;
    debugMsg("IpcAdapter::handleIpcMessage", " received message type = " << msgType);
    switch (msgType)
      {
	// End of data block
      case PlexilMsgType_NotifyExec:
	m_execInterface.notifyOfExternalEvent();
	break;

	// Command
      case PlexilMsgType_Command:
	break;

	// Message (similar to Command)
      case PlexilMsgType_Message:
	break;

	// LookupNow
	// May be followed by parameters
      case PlexilMsgType_LookupNow:

	// LookupOnChange
	// May be followed by parameters
      case PlexilMsgType_LookupOnChange:

	// AddPlan
	// Must be followed by a StringValue in either case
      case PlexilMsgType_AddPlan:
      case PlexilMsgType_AddPlanFile:

	// AddPlan
	// Must be followed by a StringValue in either case
      case PlexilMsgType_AddLibrary:
      case PlexilMsgType_AddLibraryFile:

	// Return value header - could be command or lookup
	// Must be followed by at least one value
      case PlexilMsgType_ReturnValues:

	// Values - could be parameters or return values
      case PlexilMsgType_NumericValue:
      case PlexilMsgType_StringValue:


	// Planner Update traffic
      case PlexilMsgType_PlannerUpdate:
      case PlexilMsgType_PairNumeric:
      case PlexilMsgType_PairString:

	// general NYI message
	assertTrueMsg(ALWAYS_FAIL,
		      "IpcAdapter::handleIpcMessage: received unimplemented message data type " << msgType);
	break;
			  
      default:
	assertTrueMsg(ALWAYS_FAIL,
		      "IpcAdapter::handleIpcMessage: received invalid message data type " << msgType);
	break;
      }

  }

}
