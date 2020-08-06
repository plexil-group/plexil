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
 *               bwcampb
 */

#ifndef ADAPTERCONFIGURATION_HH_
#define ADAPTERCONFIGURATION_HH_

#include "InterfaceAdapter.hh"
#include "Value.hh"
#include "AdapterExecInterface.hh"

#include <set>

namespace PLEXIL {

  // forward references
  class ExecListener;
  class ExecListenerHub;
  class InterfaceAdapter;
  class InputQueue;

  class AdapterConfiguration {
  public:

    typedef void (InterfaceAdapter::*LookupNowHandler)(const State &, StateCacheEntry&);
    typedef void (InterfaceAdapter::*SetThresholdsDoubleHandler)(const State &, double, double);
    typedef void (InterfaceAdapter::*SetThresholdsIntHandler)(const State &, int, int);
    typedef void (InterfaceAdapter::*SubscribeHandler)(const State &);
    typedef void (InterfaceAdapter::*UnsubscribeHandler)(const State &);


    struct LookupHandler
    {
    private:
      InterfaceAdapter &m_context;
      LookupNowHandler m_lookupNow;
      SetThresholdsDoubleHandler m_setThresholdsDouble;
      SetThresholdsIntHandler m_setThresholdsInt;
      SubscribeHandler m_subscribe;
      UnsubscribeHandler m_unsubscribe;
    public:
      LookupHandler(InterfaceAdapter &ct,
            LookupNowHandler ln,
            SetThresholdsDoubleHandler setTD,
            SetThresholdsIntHandler setTI,
            SubscribeHandler sub,
            UnsubscribeHandler unsub) :
            m_context(ct), m_lookupNow(ln), m_setThresholdsDouble(setTD), m_setThresholdsInt(setTI),
            m_subscribe(sub), m_unsubscribe(unsub) {}
      
      void lookupNow(const State &state, StateCacheEntry &cacheEntry)
      {
        (m_context.*m_lookupNow)(state, cacheEntry);
      }
      void setThresholds(const State &state, double hi, double lo)
      {
        if(m_setThresholdsDouble)
          (m_context.*m_setThresholdsDouble)(state, hi, lo);
      }
      void setThresholds(const State &state, int32_t hi, int32_t lo)
      {
        if(m_setThresholdsInt)
          (m_context.*m_setThresholdsInt)(state, hi, lo);
      }
      void subscribe(const State &state)
      {
        if(m_subscribe)
        ( m_context.*m_subscribe)(state);
      }
      void unsubscribe(const State &state)
      {
        if(m_unsubscribe)
          (m_context.*m_unsubscribe)(state);
      }

      InterfaceAdapter *getInterface() {
        return &m_context;
      }
    };

    /***
     * @brief A function that handles the given command.
     *          notifies the AdapterExecInterface of any new information
     * @param cmd A command to handle
     */
    typedef void (InterfaceAdapter::*ExecuteCommandHandler)(Command *);
    typedef void (InterfaceAdapter::*AbortCommandHandler)(Command *);

    struct CommandHandler
    {
    private:
      InterfaceAdapter &m_context;
      ExecuteCommandHandler m_executeCommand;
      AbortCommandHandler m_abortCommand;
    public:
      CommandHandler(InterfaceAdapter &ct,
        ExecuteCommandHandler execCmd,
        AbortCommandHandler abortCmd) :
        m_context(ct), m_executeCommand(execCmd), m_abortCommand(abortCmd) {}
      
      void ExecuteCommand(Command *cmd) {
        (m_context.*m_executeCommand)(cmd);
      }

      void AbortCommand(Command *cmd) {
        if(m_abortCommand) {
          (m_context.*m_abortCommand)(cmd);
        }
      }

      InterfaceAdapter *getInterface() {
        return &m_context;
      }
    };

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
     * @brief Returns true if the given adapter is a known interface in the system. False otherwise
     */
    bool isKnown(InterfaceAdapter *intf);

    /**
     * @brief Add an externally constructed ExecListener.
     * @param listener Pointer to the listener
     */
    void addExecListener(ExecListener *listener);

    /**
     * @brief Register the given handler for lookups to this state.
            Returns true if successful.  Fails and returns false
            if the state name already has a handler registered
            or registering a handler is not implemented.
     * @param stateName The name of the state to map to this adapter.
     * @param context An object on which the handler functions can be called.
     * @param lookupNow The lookup handler function for this state.
     * @param setThresholdsDouble The setThresholdsDouble handler function for this state. 
     * @param setThresholdsInt The setThresholdsInt handler function for this state.
     * @param subscribe The subscribe handler function for this state.
     * @param unsubscribe The lookup handler function for this state.
     * @param telemetryOnly False if this interface implements LookupNow, true otherwise.
     */
    bool registerLookupHandler(std::string const &stateName,
          InterfaceAdapter &context,
          LookupNowHandler ln,
          SetThresholdsDoubleHandler setTD = nullptr,
          SetThresholdsIntHandler setTI = nullptr,
          SubscribeHandler sub = nullptr,
          UnsubscribeHandler unsub = nullptr,
          bool telemetryOnly = false);

    /**
     * @brief Return the lookup handler in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    LookupHandler *getLookupHandler(std::string const& stateName);

    /**
     * @brief Query configuration data to determine if a state is only available as telemetry.
     * @param stateName The state.
     * @return True if state is declared telemetry-only, false otherwise.
     * @note In the absence of a declaration, a state is presumed not to be telemetry.
     */
    bool lookupIsTelemetry(std::string const &stateName) const;

    ExecListenerHub *getListenerHub()
    {
      return m_listenerHub;
    }

    /**
     * @brief Register the given handler for lookups to this state.
            Returns true if successful.  Fails and returns false
            if the state name already has a handler registered
            or registering a handler is not implemented.
     * @param stateName The name of the state to map to this adapter.
     * @param context The object on which handlers can be called
     */
    bool registerCommandHandler(std::string const &stateName,
          InterfaceAdapter &context,
          ExecuteCommandHandler execCmd,
          AbortCommandHandler abortCmd = nullptr);

    /**
     * @brief Return the lookup handler in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    CommandHandler *getCommandHandler(std::string const& stateName);

    /**
     * @brief Register the given handler as the default for lookups.
            This handler will be used for all lookups which do not have
        a specific handler.
            Returns true if successful.
        Fails and returns false if there is already a default lookup handler registered
            or setting the default lookup handler is not implemented.
     * @param context An object on which the handler functions can be called.
     * @param lookupNow The lookup handler function for this state.
     * @param setThresholdsDouble The setThresholdsDouble handler function for this state. 
     * @param setThresholdsInt The setThresholdsInt handler function for this state.
     * @param subscribe The subscribe handler function for this state.
     * @param unsubscribe The lookup handler function for this state.
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultLookupHandler(          
          InterfaceAdapter &context,
          LookupNowHandler lookupNow,
          SetThresholdsDoubleHandler setThresholdsDouble = nullptr,
          SetThresholdsIntHandler setThresholdsInt = nullptr,
          SubscribeHandler subscribe = nullptr,
          UnsubscribeHandler unsubscribe = nullptr);

    /**
     * @brief Register the given handler as the default for commands.
              This handler will be used for all commands which do not have
          a specific handler.
              Returns true if successful.
          Fails and returns false if there is already a default command handler registered.
     * @param context The object on which handlers can be called.
     * @param execCmd The execute command handler.
     * @param abortCmd The abort command handler.
     * @return True if successful, false if there is already a default handler registered.
     */
    bool setDefaultCommandHandler(InterfaceAdapter &context,
          ExecuteCommandHandler execCmd,
          AbortCommandHandler abortCmd = nullptr);

    /**
     * @brief Return the current default handler for commands.
              May return NULL. Returns NULL if default interfaces are not implemented.
     */
    CommandHandler *getDefaultCommandHandler();

    /**
     * @brief Return the current default handler for lookups.
              May return NULL.
     */
    LookupHandler *getDefaultLookupHandler();

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

    /* ----------- Replace These With Handlers? ----------- */
    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false
              iff an adapter is already registered
              or setting the default planner update interface is not implemented.
     * @param intf The interface adapter to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdapter *intf);

    /**
     * @brief Return the interface adapter in effect for planner updates,
              whether specifically registered or default. May return NULL.
     */
    InterfaceAdapter *getPlannerUpdateInterface();
    /* ---------------------------------------------------- */

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
    InterfaceAdapter *getCommandInterface(std::string const &commandName);

    /**
     * @deprecated
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    InterfaceAdapter *getLookupInterface(std::string const& stateName);

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
    InterfaceAdapter *getDefaultCommandInterface();

    /**
     * @deprecated
     * @brief Return the current default interface adapter for lookups.
              May return NULL.
     */
    InterfaceAdapter *getDefaultLookupInterface();

    /**
     * @deprecated
     * @brief Return the current default interface adapter. May return NULL.
     */
    InterfaceAdapter *getDefaultInterface();

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
    InterfaceAdapter *m_defaultInterface;
    CommandHandler *m_defaultCommandHandler;
    LookupHandler *m_defaultLookupHandler;

    //* InterfaceAdapter to use for PlannerUpdate nodes
    InterfaceAdapter *m_plannerUpdateInterface;

    // Maps by command/lookup

    // Interface handler maps
    typedef std::map<std::string, CommandHandler *> CommandHandlerMap;
    typedef std::map<std::string, LookupHandler *> LookupHandlerMap;

    LookupHandlerMap m_lookupMap;
    CommandHandlerMap m_commandMap;

    std::set<std::string> m_telemetryLookups;

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
