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

#include "InterfaceAdapter.hh"
#include <ipc.h>

// Forward declaration
struct PlexilMsgBase;

namespace PLEXIL
{
  class IpcAdapter:
    public InterfaceAdapter
  {
  public:

    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    IpcAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml A const pointer to the TiXmlElement describing this adapter
     * @note The instance maintains a shared pointer to the TiXmlElement.
     */
    IpcAdapter(AdapterExecInterface& execInterface, 
	       const TiXmlElement * xml);

    /**
     * @brief Destructor.
     */
    virtual ~IpcAdapter();

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods.  The default method simply returns true.
     */
    virtual bool reset();

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods.  The default method simply returns true.
     */
    virtual bool shutdown();

    /**
     * @brief Register one LookupOnChange.
     * @param uniqueId The unique ID of this lookup.
     * @param stateKey The state key for this lookup.
     * @param tolerances A vector of tolerances for the LookupOnChange.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void registerChangeLookup(const LookupKey& uniqueId,
				      const StateKey& stateKey,
				      const std::vector<double>& tolerances);

    /**
     * @brief Terminate one LookupOnChange.
     * @param uniqueId The unique ID of the lookup to be terminated.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void unregisterChangeLookup(const LookupKey& uniqueId);

    /**
     * @brief Register one LookupWithFrequency.
     * @param uniqueId The unique ID of this lookup.
     * @param stateKey The state key for this lookup.
     * @param lowFrequency The maximum interval in seconds between lookups.
     * @param highFrequency The minimum interval in seconds between lookups.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void registerFrequencyLookup(const LookupKey& uniqueId,
					 const StateKey& stateKey,
					 double lowFrequency, 
					 double highFrequency);

    /**
     * @brief Terminate one LookupWithFrequency.
     * @param uniqueId The unique ID of the lookup to be terminated.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void unregisterFrequencyLookup(const LookupKey& uniqueId);

    /**
     * @brief Perform an immediate lookup of the requested state.
     * @param stateKey The state key for this lookup.
     * @param dest A (reference to a) vector of doubles where the result is to be stored.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void lookupNow(const StateKey& stateKey,
			   std::vector<double>& dest);

    /**
     * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
     * @param node The Node requesting the update.
     * @param valuePairs A map of <LabelStr key, value> pairs.
     * @param ack The expression in which to store an acknowledgement of completion.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void sendPlannerUpdate(const NodeId& node,
				   const std::map<double, double>& valuePairs,
				   ExpressionId ack);

    /**
     * @brief Execute a command with the requested arguments.
     * @param name The LabelString representing the command name.
     * @param args The command arguments expressed as doubles.
     * @param dest The expression in which to store any value returned from the command.
     * @param ack The expression in which to store an acknowledgement of command transmission.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void executeCommand(const LabelStr& name,
				const std::list<double>& args,
				ExpressionId dest,
				ExpressionId ack);

    /**
     * @brief Execute a function with the requested arguments.
     * @param name The LabelString representing the command name.
     * @param args The command arguments expressed as doubles.
     * @param dest The expression in which to store any value returned from the function.
     * @param ack The expression in which to store an acknowledgement of function transmission.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void executeFunctionCall(const LabelStr& name,
				     const std::list<double>& args,
				     ExpressionId dest,
				     ExpressionId ack);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param name The LabelString representing the command name.
     * @param args The command arguments expressed as doubles.
     * @param ack The expression in which to store an acknowledgement of command abort.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    virtual void invokeAbort(const LabelStr& name, 
			     const std::list<double>& args, 
			     ExpressionId ack);

  private:

    // Deliberately unimplemented
    IpcAdapter();
    IpcAdapter(const IpcAdapter &);
    IpcAdapter & operator=(const IpcAdapter &);

    //
    // Implementation methods
    //

    /**
     * @brief Handler function as seen by IPC.
     */

    static void messageHandler(MSG_INSTANCE rawMsg,
			       void * unmarshalledMsg,
			       void * this_as_void_ptr);

    /**
     * @brief Handler function as seen by adapter.
     */

    void handleIpcMessage(const PlexilMsgBase * msgData);

    

    //
    // Private data types
    //

    //* brief Unique identifier of a message sequence
    typedef std::pair<std::string, uint32_t> IpcMessageId;

    //* brief Cache of not-yet-complete message sequences
    typedef std::map<IpcMessageId, std::vector<???> > IncompleteMessageMap;

    //
    // Member variables
    //

    IncompleteMessageMap m_incompletes;

    //* @brief Cache of incomplete received message data
    

    //* @brief Unique ID of this adapter instance
    const char* m_myUID;

  };

}
