/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#ifndef _H_InterfaceManager
#define _H_InterfaceManager

#include "ExternalInterface.hh"
#include "AdapterExecInterface.hh"
#include "PlexilPlan.hh"
#include "ValueQueue.hh"

// STL
#include <set>
#include <vector>

// Forward reference in global namespace
class TiXmlElement;

namespace PLEXIL 
{
  class AdapterConfiguration;

  // Forward references
  class ExecApplication;

  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class AdapterConfiguration;
  typedef Id<AdapterConfiguration> AdapterConfigurationId;

  class InterfaceManager;
  typedef Id<InterfaceManager> InterfaceManagerId;

  class ResourceArbiterInterface;
  typedef Id<ResourceArbiterInterface> ResourceArbiterInterfaceId;

  class ExecController;
  typedef Id<ExecController> ExecControllerId;

  class ExecListenerHub;
  typedef Id<ExecListenerHub> ExecListenerHubId;

  /**
   * @brief A concrete derived class implementing the APIs of the
   *        ExternalInterface and AdapterExecInterface classes.
   * @details The InterfaceManager class is responsible for keeping track
   *          of all the external interfaces used by the PlexilExec.  It
   *          maintains a queue of messages for the Exec to process.  Interface
   *          instantiation, initialization, startup, stopping, shutdown, and
   *          deallocation are all handled by the InterfaceManager instance.
   * @note Supersedes the old ThreadedExternalInterface class.
   */
  class InterfaceManager :
    public ExternalInterface,
    public AdapterExecInterface
  {
  public:

    /**
     * @brief Constructor. A new DefaultAdapterConfiguration is used as the AdapterConfiguration.
     * @param app The ExecApplication instance to which this object belongs.
     */
    InterfaceManager(ExecApplication & app);

    /**
     * @brief Destructor.
     */
    virtual ~InterfaceManager();

    /**
     * @brief Get internal ID pointer.
     */
    const InterfaceManagerId& getInterfaceManagerId() const
    {
      return m_interfaceManagerId;
    }

    /**
     * @brief Get parent application.
     */
    inline ExecApplication& getApplication() const
    {
      return m_application;
    }

    /**
     * @brief Get adapterConfiguration.
     */
    inline AdapterConfigurationId getAdapterConfig() const
    {
      return m_adapterConfig;
    }
	
	/**
	 * @brief Set the Exec.
	 */
    virtual void setExec(const PlexilExecId& exec);

    //
    // API for all related objects
    //

    /**
     * @brief Associate an arbitrary object with a string.
     * @param name The string naming the property.
     * @param thing The property value as an untyped pointer.
     */
    virtual void setProperty(const std::string& name, void * thing);

    /**
     * @brief Fetch the named property.
     * @param name The string naming the property.
     * @return The property value as an untyped pointer.
     */
    virtual void* getProperty(const std::string& name);


    //
    // API for ExecApplication
    //

	/**
	 * @brief Get the search path for library nodes.
	 * @return A reference to the library search path.
	 */
	const std::vector<std::string>& getLibraryPath() const;

	/**
	 * @brief Get the search path for plan files.
	 * @return A reference to the plan search path.
	 */
	const std::vector<std::string>& getPlanPath() const;

	/**
	 * @brief Add the specified directory name to the end of the library node loading path.
	 * @param libdir The directory name.
	 */
	void addLibraryPath(const std::string& libdir);

	/**
	 * @brief Add the specified directory names to the end of the library node loading path.
	 * @param libdirs The vector of directory names.
	 */
	void addLibraryPath(const std::vector<std::string>& libdirs);

	/**
	 * @brief Add the specified directory name to the end of the plan loading path.
	 * @param libdir The directory name.
	 */
	void addPlanPath(const std::string& libdir);

	/**
	 * @brief Add the specified directory names to the end of the plan loading path.
	 * @param libdirs The vector of directory names.
	 */
	void addPlanPath(const std::vector<std::string>& libdirs);

    /**
     * @brief Constructs interface adapters from the provided XML.
     * @param configXml The XML element used for interface configuration.
	 * @return true if successful, false otherwise.
     */
    bool constructInterfaces(const TiXmlElement * configXml);

    /**
     * @brief Add an externally constructed interface adapter.
     * @param adapter The adapter ID.
     */
    void addInterfaceAdapter(const InterfaceAdapterId& adapter);

    /**
     * @brief Add an externally constructed ExecListener.
     * @param listener The ExecListener ID.
     */
    void addExecListener(const ExecListenerId& listener);

    /**
     * @brief Performs basic initialization of the interface and all adapters.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize();

    /**
     * @brief Prepares the interface and adapters for execution.
     * @return true if successful, false otherwise.
     */
    virtual bool start();

    /**
     * @brief Halts all interfaces.
     * @return true if successful, false otherwise.
     */
    virtual bool stop();

    /**
     * @brief Resets the interface prior to restarting.
     * @return true if successful, false otherwise.
     */
    virtual bool reset();

    /**
     * @brief Shuts down the interface.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown();
    
    /**
     * @brief Updates the state cache from the items in the queue.
     * @return True if the exec needs to be stepped, false otherwise.
     * @note Should only be called with exec locked by the current thread.
     */
    bool processQueue();

	/**
	 * @brief Insert a mark in the value queue.
	 * @return The sequence number of the mark.
	 */
	unsigned int markQueue()
	{
	  return m_valueQueue.mark();
	}

	/**
	 * @brief Get the sequence number of the most recently processed mark.
	 * @return The sequence number, 0 if no marks have yet been processed.
	 */
	unsigned int getLastMark() const
	{
	  return m_lastMark;
	}

    //
    // API for exec
    //
    
    /**
     * @brief Delete any entries in the queue.
     */
    void resetQueue();

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
	 * @return The current value of the state or UNKNOWN().
     */
    double lookupNow(const State& state);

	/**
	 * @brief Inform the interface that it should report changes in value of this state.
	 * @param state The state.
	 */
	void subscribe(const State& state);

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     */
	void unsubscribe(const State& state);

	/**
	 * @brief Advise the interface of the current thresholds to use when reporting this state.
	 * @param state The state.
	 * @param hi The upper threshold, at or above which to report changes.
	 * @param lo The lower threshold, at or below which to report changes.
	 */
	void setThresholds(const State& state, double hi, double lo);

    //this batches the set of actions from quiescence completion.  calls PlexilExecutive::step() at the end
    //assignments must be performed first.
    //though it only takes a list of commands as an argument at the moment, it will eventually take function calls and the like
    //so the name remains "batchActions"
    void batchActions(std::list<CommandId>& commands);
    void updatePlanner(std::list<UpdateId>& updates);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmdName The LabelString representing the command name.
     * @param cmdArgs The command arguments expressed as doubles.
     * @param cmdAck The acknowledgment of the pending command
     * @param abrtAck The expression in which to store an acknowledgment of command abort.
     * @note Derived classes may implement this method.  The default method causes an assertion to fail.
     */

    void invokeAbort(const LabelStr& cmdName, const std::list<double>& cmdArgs, ExpressionId abrtAck, ExpressionId cmdAck);

    double currentTime();


    //
    // API to interface adapters
    //

    /**
     * @brief Register the given interface adapter based on its configuration XML.  
     * @param adapter The interface adapter to handle this command.
     */

    virtual void defaultRegisterAdapter(InterfaceAdapterId adapter);

    /**
     * @brief Register the given interface adapter for this command.  
     Returns true if successful.  Fails and returns false 
     iff the command name already has an adapter registered.
     * @param commandName The command to map to this adapter.
     * @param intf The interface adapter to handle this command.
     */
    bool registerCommandInterface(const LabelStr & commandName,
				  InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for lookups to this state.
     Returns true if successful.  Fails and returns false 
     if the state name already has an adapter registered.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     */
    bool registerLookupInterface(const LabelStr & stateName,
        const InterfaceAdapterId& intf);

    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false 
              iff an adapter is already registered.
     * @param intf The interface adapter to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for all lookups and commands
     which do not have a specific adapter.  Returns true if successful.
     Fails and returns false if there is already a default adapter registered.
     * @param intf The interface adapter to use as the default.
     */
    bool setDefaultInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for lookups.
              This interface will be used for all lookups which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default lookup adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultLookupInterface(InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter as the default for commands.
              This interface will be used for all commands which do not have
	      a specific adapter.  
              Returns true if successful.
	      Fails and returns false if there is already a default command adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultCommandInterface(InterfaceAdapterId intf);

    /**
     * @brief Retract registration of the previous interface adapter for this command.  
     * @param commandName The command.
     */
    void unregisterCommandInterface(const LabelStr & commandName);

    /**
     * @brief Retract registration of the previous interface adapter for this state.
     * @param stateName The state name.
     */
    void unregisterLookupInterface(const LabelStr & stateName);

    /**
     * @brief Retract registration of the previous interface adapter for planner updates.
     */
    void unregisterPlannerUpdateInterface();

    /**
     * @brief Retract registration of the previous default interface adapter.
     */
    void unsetDefaultInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for commands.
     */
    void unsetDefaultCommandInterface();

    /**
     * @brief Retract registration of the previous default interface adapter for lookups.
     */
    void unsetDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for this command, whether 
     specifically registered or default. May return NoId().
     * @param commandName The command.
     */
    InterfaceAdapterId getCommandInterface(const LabelStr & commandName);

    /**
     * @brief Return the current default interface adapter for commands.
              May return NoId().
     */
    InterfaceAdapterId getDefaultCommandInterface();

    /**
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NoId().
     * @param stateName The state.
     */
    InterfaceAdapterId getLookupInterface(const LabelStr & stateName);

    /**
     * @brief Return the current default interface adapter for lookups.
              May return NoId().
     */
    InterfaceAdapterId getDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for planner updates,
              whether specifically registered or default. May return NoId().
     */
    InterfaceAdapterId getPlannerUpdateInterface();

    /**
     * @brief Return the current default interface adapter. May return NoId().
     */
    InterfaceAdapterId getDefaultInterface();

    /**
     * @brief Register the given resource arbiter interface for all commands
     Returns true if successful.
     Fails and returns false if there is already an interface registered.
     * @param raIntf The resource arbiter interface to use.
     */
    bool setResourceArbiterInterface(ResourceArbiterInterfaceId raIntf);

    /**
     * @brief Retract registration of the previous resource arbiter interface.
     */
    void unsetResourceArbiterInterface();

    /**
     * @brief Return the current resource arbiter interface. May return NoId().
     */
    ResourceArbiterInterfaceId getResourceArbiterInterface() const {return m_raInterface;}

    /**
     * @brief Notify of the availability of a new value for a lookup.
     * @param state The state for the new value.
     * @param value The new value.
     */
    void handleValueChange(const State& state, double value);

    /**
     * @brief Notify of the availability of (e.g.) a command return or acknowledgement.
     * @param exp The expression whose value is being returned.
     * @param value The new value of the expression.
     */
    void handleValueChange(const ExpressionId & exp, double value);

    /**
     * @brief Tells the external interface to expect a return value from this command.
     Use handleValueChange() to actually return the value.
     * @param dest The expression whose value will be returned.
     * @param name The command whose value will be returned.
     * @param params The parameters associated with this command.
     */
    void registerCommandReturnValue(ExpressionId dest,
				    const LabelStr & name,
				    const std::list<double> & params);

    /**
     * @brief Notify the external interface that this previously registered expression
     should not wait for a return value.
     * @param dest The expression whose value was to be returned.
     */
    void unregisterCommandReturnValue(ExpressionId dest);

    /**
     * @brief Notify the executive of a new plan.
     * @param planXml The TinyXML representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @return False if the plan references unloaded libraries, true otherwise.
     * @note This is deprecated, use the PlexilNodeId variant instead.
     */
    bool handleAddPlan(TiXmlElement * planXml,
		       const LabelStr& parent)
      throw(ParserException);

    /**
     * @brief Notify the executive of a new plan.
     * @param planStruct The PlexilNode representation of the new plan.
     * @param parent The node which is the parent of the new node.
     * @return False if the plan references unloaded libraries, true otherwise.
     */
    bool handleAddPlan(PlexilNodeId planStruct,
                       const LabelStr& parent);

    /**
     * @brief Notify the executive of a new library node.
     * @param planStruct The PlexilNode representation of the new library node.
     */
    void handleAddLibrary(PlexilNodeId planStruct);

    /**
     * @brief Determine whether the named library is loaded.
     * @return True if loaded, false otherwise.
     */
    bool isLibraryLoaded(const std::string& libName) const;

    /**
     * @brief Notify the executive that it should run one cycle.
    */
    void notifyOfExternalEvent();

    /**
     * @brief Run the exec and wait until all events in the queue have been processed.
	 */
	void notifyAndWaitForCompletion();

    StateCacheId getStateCache() const;

    //
    // Utility accessors
    //

    // *** Is this actually needed?? ***
    inline PlexilExecId getExec() const
    { 
      return m_exec; 
    }

    /**
     * @brief Clears the interface adapter registry.
     */
    void clearAdapterRegistry();


  protected:

    //
    // Internal functionality
    //

	// Executes a command.
	void executeCommand(CommandId cmd);

    // rejects a command due to non-availability of resources
    void rejectCommand(const LabelStr& name,
			const std::list<double>& args,
			ExpressionId dest,
			ExpressionId ack);

    /**
     * @brief Removes the adapter and deletes it iff nothing refers to it.
     */
    void deleteIfUnknown(InterfaceAdapterId intf);

    friend class AdapterConfiguration;

  private:

    // Deliberately unimplemented
    InterfaceManager();
    InterfaceManager(const InterfaceManager &);
    InterfaceManager & operator=(const InterfaceManager &);

    /**
     * @brief update the resoruce arbiter interface that an ack or return value
     * has been received so that resources can be released.
     * @param ackOrDest The expression id for which a value has been posted.
     */
    void releaseResourcesAtCommandTermination(ExpressionId ackOrDest);

    /**
     * @brief Deletes the given adapter
     * @return true if the given adapter existed and was deleted. False if not found
     */
    bool deleteAdapter(InterfaceAdapterId intf);

    //
    // Internal types and classes
    //

    //
    // State cache
    //
    // The state cache is always updated by the exec thread.  Older 
    // implementations could have been updated by another thread, which
    // caused race conditions in the quiescence loop.
    //

    bool expressionToState(const ExpressionId & exp, State & key) const;
    bool updateStateListener(const ExpressionId & dest, double value);

    //
    // Private member variables
    //

    //* ID as an interface manager
    InterfaceManagerId m_interfaceManagerId;

    //* Parent object
    ExecApplication& m_application;
    //* Adapter Config
    AdapterConfigurationId m_adapterConfig;

    //* The queue
    ValueQueue m_valueQueue;

    //* ExecListener hub
    ExecListenerHubId m_listenerHub;

    //* Set of all known InterfaceAdapter instances
    std::set<InterfaceAdapterId> m_adapters;

    // Maps by lookup key
    typedef std::map<LookupKey, InterfaceAdapterId> LookupAdapterMap;
    LookupAdapterMap m_lookupAdapterMap;
    std::map<ExpressionId, CommandId> m_ackToCmdMap;
    std::map<ExpressionId, CommandId> m_destToCmdMap;

    // Properties
    typedef std::map<const std::string, void*> PropertyMap;
    PropertyMap m_propertyMap;

    //* The resource arbiter
    ResourceArbiterInterfaceId m_raInterface;

	//* The exec controller
	ExecControllerId m_execController;

	//* List of directory names for plan file search paths
	std::vector<std::string> m_libraryPath;
	std::vector<std::string> m_planPath;

    //* Holds the most recent idea of the current time
    double m_currentTime;

	//* Most recent mark processed.
	unsigned int m_lastMark;
  };

}

#endif // _H_InterfaceManager
