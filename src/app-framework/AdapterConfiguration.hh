/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
/*
 * AdapterConfiguration.hh
 *
 *  Created on: Jan 28, 2010
 *      Authors: jhogins
 *               bcampbell
 */

#ifndef ADAPTERCONFIGURATION_HH_
#define ADAPTERCONFIGURATION_HH_

#include "pugixml.hpp"

#include <map>
#include <set>
#include <vector>

namespace PLEXIL {

  // forward references
  class Command;
  class ExecListener;
  class ExecListenerHub;
  class InterfaceAdapter;
  class InputQueue;
  class State;
  class StateCacheEntry;
  class Update;

  typedef void (*LookupNowHandler)(const State &, StateCacheEntry&);
  typedef void (*SetThresholdsDoubleHandler)(const State &, double, double);
  typedef void (*SetThresholdsIntHandler)(const State &, int, int);
  typedef void (*SubscribeHandler)(const State &);
  typedef void (*UnsubscribeHandler)(const State &);

  /**
   * @brief An absract parent class to describe the functionality of a
   *    lookup handler object.
   */
  class AbstractLookupHandler {
  public:
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry) = 0;
    virtual void setThresholds(const State &state, double hi, double lo) { } // default no-op
    virtual void setThresholds(const State &state, int32_t hi, int32_t lo) { } // default no-op
    virtual void subscribe(const State &state) { } // default no-op
    virtual void unsubscribe(const State &state) { } // default no-op
  };

  typedef void (*ExecuteCommandHandler)(Command *);
  typedef void (*AbortCommandHandler)(Command *);

  /**
   * @brief An absract parent class to describe the functionality of a
   *    command handler object.
   */
  class AbstractCommandHandler {
  public:
    virtual void executeCommand(Command *cmd) = 0;
    virtual void abortCommand(Command *cmd) { }
  };

  typedef void (*PlannerUpdateHandler)(Update *);

  /**
   * @brief An absract parent class to describe the functionality of a
   *    planner update handler object.
   */
  class AbstractPlannerUpdateHandler {
  public:
    virtual void sendPlannerUpdate(Update *update) = 0;
  };

  class AdapterConfiguration {
  public:

    /**
     * @brief Constructor.
     */
    AdapterConfiguration();

    /**
     * @brief Destructor.
     */
    ~AdapterConfiguration();

    /**
     * @brief Constructs interface adapters from the provided XML.
     * @param configXml The XML element used for interface configuration.
     * @return true if successful, false otherwise.
     */
    bool constructInterfaces(pugi::xml_node const configXml);

    /**
     * @brief Performs basic initialization of the interface and all adapters.
     * @return true if successful, false otherwise.
     */
    bool initialize();

    /**
     * @brief Prepares the interface and adapters for execution.
     * @return true if successful, false otherwise.
     */
    bool start();

    /**
     * @brief Halts all interfaces.
     * @return true if successful, false otherwise.
     */
    bool stop();

    /**
     * @brief Resets the interface prior to restarting.
     * @return true if successful, false otherwise.
     */
    bool reset();

    /**
     * @brief Shuts down the interface.
     * @return true if successful, false otherwise.
     */
    bool shutdown();

    /**
     * @brief Add an externally constructed interface adapter.
     * @param adapter The adapter ID.
     */
    void addInterfaceAdapter(InterfaceAdapter *adapter);

    /**
     * @brief Clears the interface adapter registry.
     */
    void clearAdapterRegistry();

    /**
     * @brief Add an externally constructed ExecListener.
     * @param listener Pointer to the listener
     */
    void addExecListener(ExecListener *listener);

    /**
     * @brief Register the given state to handle telemetry lookups
     * @param stateName The name of the state
     */
    bool registerTelemetryLookup(std::string const &stateName);

    /**
     * @brief Register the given object to be a handler for lookups to this state
     * @param stateName The name of the state to map to this object
     * @param handler An object handler to register as the handler.
     */
    bool registerLookupObjectHandler(std::string const &stateName, AbstractLookupHandler *handler);

    /**
     * @brief Register the given handler for lookups to this state.
     Returns true if successful.  Fails and returns false
     if the state name already has a handler registered
     or registering a handler is not implemented.
     * @param stateName The name of the state to map to this handler.
     * @param context An object on which the handler functions can be called.
     * @param lookupNow The lookup handler function for this state.
     * @param setThresholdsDouble The setThresholdsDouble handler function for this state. 
     * @param setThresholdsInt The setThresholdsInt handler function for this state.
     * @param subscribe The subscribe handler function for this state.
     * @param unsubscribe The lookup handler function for this state.
     */
    bool registerLookupHandler(std::string const &stateName,
                               LookupNowHandler ln,
                               SetThresholdsDoubleHandler setTD = NULL,
                               SetThresholdsIntHandler setTI = NULL,
                               SubscribeHandler sub = NULL,
                               UnsubscribeHandler unsub = NULL);

    /**
     * @brief Return the lookup handler in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    AbstractLookupHandler *getLookupHandler(std::string const& stateName) const;

    /**
     * @brief Query configuration data to determine if a state is only available as telemetry.
     * @param stateName The state.
     * @return True if state is declared telemetry-only, false otherwise.
     * @note In the absence of a declaration, a state is presumed not to be telemetry.
     */
    bool lookupIsTelemetry(std::string const &stateName) const;

    /**
     * @brief Return the listenerHub 
     */
    ExecListenerHub *getListenerHub() const
    {
      return m_listenerHub;
    }

    /**
     * @brief Register the given object handler for lookups to this state.
     Returns true if successful.  Fails and returns false
     if the state name already has a handler registered
     or registering an object handler is not implemented.
     * @param stateName The name of the state to map to this object
     * @param handler An object to register as the handler.
     */
    bool registerCommandObjectHandler(std::string const &stateName, AbstractCommandHandler *handler);

    /**
     * @brief Register the given handler for lookups to this state.
     Returns true if successful.  Fails and returns false
     if the state name already has a handler registered
     or registering a handler is not implemented.
     * @param stateName The name of the state to map to this handler.
     * @param execCmd The function to call when this command is executed
     * @param abortCmd The function to call when this command is aborted
     */
    bool registerCommandHandler(std::string const &stateName,
                                ExecuteCommandHandler execCmd,
                                AbortCommandHandler abortCmd = NULL);

    /**
     * @brief Return the command object handler in effect for commands with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    AbstractCommandHandler *getCommandHandler(std::string const& stateName) const;

    /**
     * @brief Register the given handler as the default for lookups.
     This handler will be used for all lookups which do not have
     a specific handler.
     Returns true if successful.
     Fails and returns false if there is already a default lookup handler registered
     or setting the default lookup handler is not implemented.
     * @param handler The object handler to call by default
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultLookupObjectHandler(AbstractLookupHandler *handler);
    
    /**
     * @brief Register the given handler as the default for lookups.
     This handler will be used for all lookups which do not have
     a specific handler.
     Returns true if successful.
     Fails and returns false if there is already a default lookup handler registered
     or setting the default lookup handler is not implemented.
     * @param lookupNow The lookup handler function for this state.
     * @param setThresholdsDouble The setThresholdsDouble handler function for this state (optional). 
     * @param setThresholdsInt The setThresholdsInt handler function for this state (optional).
     * @param subscribe The subscribe handler function for this state (optional).
     * @param unsubscribe The lookup handler function for this state (optional).
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultLookupHandler(          
                                 LookupNowHandler lookupNow,
                                 SetThresholdsDoubleHandler setThresholdsDouble = NULL,
                                 SetThresholdsIntHandler setThresholdsInt = NULL,
                                 SubscribeHandler subscribe = NULL,
                                 UnsubscribeHandler unsubscribe = NULL);

    /**
     * @brief Register the given handler as the default for commands.
     This handler will be used for all commands which do not have
     a specific handler.
     Returns true if successful.
     Fails and returns false if there is already a default command handler registered.
     * @param context The object on which handlers can be called.
     * @param handler The handler object to use by default for commands.
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultCommandObjectHandler(AbstractCommandHandler *handler);
    
    /**
     * @brief Register the given handler as the default for commands.
     This handler will be used for all commands which do not have
     a specific handler.
     Returns true if successful.
     Fails and returns false if there is already a default command handler registered.
     * @param execCmd The execute command handler.
     * @param abortCmd The abort command handler.
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultCommandHandler(
                                  ExecuteCommandHandler execCmd,
                                  AbortCommandHandler abortCmd = NULL);

    /**
     * @brief Return the current default handler for commands.
     May return NULL. Returns NULL if default handlers are not implemented.
    */
    AbstractCommandHandler *getDefaultCommandHandler() const;

    /**
     * @brief Return the current default handler for lookups.
     May return NULL. Returns NULL if default lookup handlers are not implemented.
    */
    AbstractLookupHandler *getDefaultLookupHandler() const;
    
    /**
     * @brief Register the given object handler for planner updates.
     Returns true if successful.  Fails and returns false
     iff a handler is already registered
     or setting the default planner update interface is not implemented.
     * @param handler The object handler to handle planner updates.
     */
    bool registerPlannerUpdateObjectHandler(AbstractPlannerUpdateHandler *handler);

    /**
     * @brief Register the given function handler for planner updates.
     Returns true if successful.  Fails and returns false
     iff a handler is already registered
     or setting the default planner update interface is not implemented.
     * @param sendPlannerUpdate The function to call for planner updates.
     */
    bool registerPlannerUpdateHandler(PlannerUpdateHandler sendPlannerUpdate);

    /**
     * @brief Return the object handler in effect for planner updates,
     whether specifically registered or default. May return NULL.
    */
    AbstractPlannerUpdateHandler *getPlannerUpdateHandler() const;

    //
    // Plan, library path access
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
     * @brief Return the set of registered adapters
     */
    std::set<InterfaceAdapter *> & getAdapters()
    {
      return m_adapters;
    }

    /**
     * @brief Return the set of registered adapters
     */
    std::set<InterfaceAdapter *> const & getAdapters() const
    {
      return m_adapters;
    }

    /**
     * @brief Construct the input queue specified by the configuration data.
     * @return Pointer to instance of a class derived from InputQueue.
     */
    InputQueue *getInputQueue() const;


    /* --------------------------------------------------------------------- */
    /* -------------------- Deprecated Interface Methods ------------------- */
    /* --------------------------------------------------------------------- */

    //
    // API to interface adapters
    //

    /**
     * @deprecated interface adapters are no longer stored.
     * @brief Returns true if the given adapter is a known interface in the system. False otherwise
     */
    bool isKnown(InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Register the given interface adapter for this command.
     Returns true if successful.  Fails and returns false
     iff the command name already has an adapter registered
     or setting a command interface is not implemented.
     * @param commandName The command to map to this adapter.
     * @param intf The interface adapter to handle this command.
     */
    bool registerCommandInterface(std::string const &commandName,
                                  InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Register the given interface adapter for lookups to this state.
     Returns true if successful.  Fails and returns false
     if the state name already has an adapter registered
     or registering a lookup interface is not implemented.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     * @param telemetryOnly False if this interface implements LookupNow, true otherwise.
     */
    bool registerLookupInterface(std::string const &stateName,
                                 InterfaceAdapter *intf,
                                 bool telemetryOnly = false);

    /**
     * @deprecated
     * @brief Return the interface adapter in effect for this command, whether
     specifically registered or default. May return NULL.
     * @param commandName The command.
     */
    InterfaceAdapter *getCommandInterface(std::string const &commandName) const;

    /**
     * @deprecated
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    InterfaceAdapter *getLookupInterface(std::string const& stateName) const;

    /**
     * @brief Register the given interface adapter.
     * @param adapter The interface adapter to be registered.
     */
    void defaultRegisterAdapter(InterfaceAdapter *adapter);

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for all lookups and commands
     which do not have a specific adapter.  Returns true if successful.
     Fails and returns false if there is already a default adapter registered
     or setting the default interface is not implemented.
     * @param intf The interface adapter to use as the default.
     */
    bool setDefaultInterface(InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for lookups.
     This interface will be used for all lookups which do not have
     a specific adapter.
     Returns true if successful.
     Fails and returns false if there is already a default lookup adapter registered
     or setting the default lookup interface is not implemented.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultLookupInterface(InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for commands.
     This interface will be used for all commands which do not have
     a specific adapter.
     Returns true if successful.
     Fails and returns false if there is already a default command adapter registered.
     * @param intf The interface adapter to use as the default.
     * @return True if successful, false if there is already a default adapter registered.
     */
    bool setDefaultCommandInterface(InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Return the current default interface adapter for commands.
     May return NULL.
    */
    InterfaceAdapter *getDefaultCommandInterface() const;

    /**
     * @deprecated
     * @brief Return the current default interface adapter for lookups.
     May return NULL.
    */
    InterfaceAdapter *getDefaultLookupInterface() const;

    /**
     * @deprecated
     * @brief Return the current default interface adapter. May return NULL.
     */
    InterfaceAdapter *getDefaultInterface() const;

    /**
     * @deprecated
     * @brief Register the given interface adapter for planner updates.
     Returns true if successful.  Fails and returns false
     iff an adapter is already registered
     or setting the default planner update interface is not implemented.
     * @param intf The interface adapter to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdapter *intf);

    /**
     * @deprecated
     * @brief Return the interface adapter in effect for planner updates,
     whether specifically registered or default. May return NULL.
    */
    InterfaceAdapter *getPlannerUpdateInterface() const;

  private:

    // Not implemented
    AdapterConfiguration(AdapterConfiguration const &);
    AdapterConfiguration &operator=(AdapterConfiguration const &);

    /**
     * @brief Deletes the given adapter from the interface manager
     * @return true if the given adapter existed and was deleted. False if not found
     */
    bool deleteAdapter(InterfaceAdapter *intf);

    //* Default InterfaceAdapters
    AbstractCommandHandler *m_defaultCommandHandler;
    AbstractLookupHandler *m_defaultLookupHandler;

    //* InterfaceAdapter to use for PlannerUpdate nodes
    AbstractPlannerUpdateHandler *m_plannerUpdateHandler;

    // Maps by command/lookup

    // Interface handler maps
    typedef std::map<std::string, AbstractCommandHandler *> CommandHandlerMap;
    typedef std::map<std::string, AbstractLookupHandler *> LookupHandlerMap;

    LookupHandlerMap m_lookupMap;
    CommandHandlerMap m_commandMap;

    //* ExecListener hub
    ExecListenerHub *m_listenerHub;

    //* Set of all known InterfaceAdapter instances
    std::set<InterfaceAdapter *> m_adapters;

    //* List of directory names for plan file search paths
    std::vector<std::string> m_planPath;
  };

  extern AdapterConfiguration *g_configuration;

}

#endif /* ADAPTERCONFIGURATION_HH_ */
