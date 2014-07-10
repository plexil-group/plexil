/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

// STL
#include <set>
#include <vector>

// Forward reference
namespace pugi
{
  class xml_node;
}

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

  class ExecListenerHub;
  typedef Id<ExecListenerHub> ExecListenerHubId;

  class InputQueue;

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
     * @brief Constructor.
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
    bool constructInterfaces(const pugi::xml_node& configXml);

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
    unsigned int markQueue();

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
    void lookupNow(State const &state, StateCacheEntry &cacheEntry);

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
    void setThresholds(const State& state, int32_t hi, int32_t lo);

    void executeCommand(Command *cmd);

    /**
     * @brief Abort the pending command with the supplied name and arguments.
     * @param cmd The command.
     */
    void invokeAbort(Command *cmd);

    void executeUpdate(Update *upd);

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
    bool registerCommandInterface(std::string const &commandName,
                  InterfaceAdapterId intf);

    /**
     * @brief Register the given interface adapter for lookups to this state.
     Returns true if successful.  Fails and returns false 
     if the state name already has an adapter registered.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     */
    bool registerLookupInterface(std::string const & stateName,
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
    void unregisterCommandInterface(std::string const &commandName);

    /**
     * @brief Retract registration of the previous interface adapter for this state.
     * @param stateName The state name.
     */
    void unregisterLookupInterface(std::string const &stateName);

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
    InterfaceAdapterId getCommandInterface(std::string const &commandName);

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
    InterfaceAdapterId getLookupInterface(std::string const &stateName);

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
    void handleValueChange(const State& state, const Value& value);

    void handleCommandReturn(Command * cmd, Value const& value);

    void handleCommandAck(Command * cmd, CommandHandleValue value);

    void handleCommandAbortAck(Command * cmd, bool ack);

    void handleUpdateAck(Update * upd, bool ack);

    /**
     * @brief Notify the executive of a new plan.
     * @param planXml The TinyXML representation of the new plan.
     * @return True if parsing successful, false otherwise.
     */
    bool handleAddPlan(const pugi::xml_node& planXml)
      throw(ParserException);

    /**
     * @brief Notify the executive of a new plan.
     * @param planStruct The PlexilNode representation of the new plan.
     * @return True if all referenced libraries were found, false otherwise.
     */
    bool handleAddPlan(PlexilNodeId planStruct);

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

#ifdef PLEXIL_WITH_THREADS
    /**
     * @brief Run the exec and wait until all events in the queue have been processed.
     */
    void notifyAndWaitForCompletion();
#endif

    //
    // Utility accessors
    //

    /**
     * @brief Clears the interface adapter registry.
     */
    void clearAdapterRegistry();


  protected:

    //
    // Internal functionality
    //

    // rejects a command due to non-availability of resources
    void rejectCommand(Command *cmd);

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
    void releaseResourcesAtCommandTermination(Expression *ackOrDest);

    /**
     * @brief Deletes the given adapter
     * @return true if the given adapter existed and was deleted. False if not found
     */
    bool deleteAdapter(InterfaceAdapterId intf);

    //
    // Internal types and classes
    //

    //
    // Private member variables
    //

    //* ID as an interface manager
    InterfaceManagerId m_interfaceManagerId;

    //* Parent object
    ExecApplication& m_application;
    //* Adapter Config
    AdapterConfigurationId m_adapterConfig;

    //* ExecListener hub
    ExecListenerHubId m_listenerHub;

    //* Set of all known InterfaceAdapter instances
    std::set<InterfaceAdapterId> m_adapters;

    // Properties
    typedef std::map<const std::string, void*> PropertyMap;
    PropertyMap m_propertyMap;

    //* The resource arbiter
    ResourceArbiterInterfaceId m_raInterface;

    //* List of directory names for plan file search paths
    std::vector<std::string> m_libraryPath;
    std::vector<std::string> m_planPath;

    //* The queue
    InputQueue *m_inputQueue;

    //* Holds the most recent idea of the current time
    double m_currentTime;

    //* Most recent mark processed.
    unsigned int m_lastMark;
  };

}

#endif // _H_InterfaceManager
