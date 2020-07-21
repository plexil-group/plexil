/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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
 *      Author: jhogins
 */

#ifndef ADAPTERCONFIGURATION_HH_
#define ADAPTERCONFIGURATION_HH_

#include "InterfaceAdapter.hh"

#include <set>

namespace PLEXIL {

  // forward references
  class ExecListener;
  class ExecListenerHub;
  class InterfaceAdapter;
  class InputQueue;

  class AdapterConfiguration {
  public:

    class GenericLookupHandler {
      public:
      virtual void LookupNow(const State &state,
        StateCacheEntry &cacheEntry) = 0;
      virtual void SetThresholds(const State &state,
        double hi, double lo) = 0;
      virtual void SetThresholds(const State &state,
        int32_t hi, int32_t lo) = 0;
      virtual void Subscribe(const State &state) = 0;
      virtual void Unsubscribe(const State &state) = 0;
    };

    template <typename CONTEXT>
    struct LookupHandler : GenericLookupHandler
    {
      CONTEXT m_context; // Should context be stored as object or pointer?
      void (CONTEXT::*m_lookupNow)(const State &state, StateCacheEntry &cacheEntry);
      void (CONTEXT::*m_setThresholdsDouble)(const State &state, double hi, double lo);
      void (CONTEXT::*m_setThresholdsInt)(const State &state, int32_t hi, int32_t lo);
      void (CONTEXT::*m_subscribe)(const State &state);
      void (CONTEXT::*m_unsubscribe)(const State &state);
      
      LookupHandler(CONTEXT ct,
            void (CONTEXT::*ln)(const State &state, StateCacheEntry &cacheEntry),
            void (CONTEXT::*setTD)(const State &state, double hi, double lo),
            void (CONTEXT::*setTI)(const State &state, int32_t hi, int32_t lo),
            void (CONTEXT::*sub)(const State &state),
            void (CONTEXT::*unsub)(const State &state)) :
            m_context(ct), m_lookupNow(ln), m_setThresholdsDouble(setTD), m_setThresholdsInt(setTI),
            m_subscribe(sub), m_unsubscribe(unsub) {}
      
      void LookupNow(const State &state, StateCacheEntry &cacheEntry)
      {
        m_context.m_lookupNow(state, cacheEntry);
      }
      virtual void SetThresholds(const State &state,
        double hi, double lo)
      {
        m_context.m_setThresholdsDouble(state, hi, lo);
      }
      virtual void SetThresholds(const State &state,
        int32_t hi, int32_t lo)
      {
        m_context.m_setThresholdsInt(state, hi, lo);
      }
      virtual void Subscribe(const State &state)
      {
        m_context.m_subscribe(state);
      }
      virtual void Unsubscribe(const State &state)
      {
        m_context.m_unsubscribe(state);
      }
    };

    struct CommandHandler // May not be correct, need to asynchronously make the call and update on return
    {
      void (*execute)(Command *cmd, AdapterExecInterface *callback);
      void (*abortCommand)(Command *cmd, AdapterExecInterface *callback);
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
     * @brief Add an externally constructed ExecListener.
     * @param listener Pointer to the listener
     */
    void addExecListener(ExecListener *listener);

    //
    // API to interface adapters
    //

    /**
     * @brief Register the given interface adapter.
     * @param adapter The interface adapter to be registered.
     */

    void defaultRegisterAdapter(InterfaceAdapter *adapter);

    /**
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
   * @brief Register the given handler for lookups to this state.
   Returns true if successful.  Fails and returns false
    if the state name already has a handler registered
            or registering a handler is not implemented.
    * @param stateName The name of the state to map to this adapter.
    * @param lookupNow The lookup handler function for this state.
    * @param setThresholdsDouble The setThresholdsDouble handler function for this state. 
    * @param setThresholdsInt The setThresholdsInt handler function for this state.
    * @param subscribe The subscribe handler function for this state.
    * @param unsubscribe The lookup handler function for this state.
    * @param context An object on which the handler functions can be called.
    * @param telemetryOnly False if this interface implements LookupNow, true otherwise.
    */
    template <typename CONTEXT>
    bool registerLookupHandler(std::string const &stateName,
          CONTEXT &context,
          void (CONTEXT::*ln)(const State &state, StateCacheEntry &cacheEntry),
          void (CONTEXT::*setTD)(const State &state, double hi, double lo) = nullptr,
          void (CONTEXT::*setTI)(const State &state, int32_t hi, int32_t lo) = nullptr,
          void (CONTEXT::*sub)(const State &state) = nullptr,
          void (CONTEXT::*unsub)(const State &state) = nullptr,
          bool telemetryOnly = false);

    /**
     * @brief Register the given interface adapter for planner updates.
              Returns true if successful.  Fails and returns false
              iff an adapter is already registered
              or setting the default planner update interface is not implemented.
     * @param intf The interface adapter to handle planner updates.
     */
    bool registerPlannerUpdateInterface(InterfaceAdapter *intf);

    /**
     * @brief Register the given interface adapter as the default for all lookups and commands
     which do not have a specific adapter.  Returns true if successful.
     Fails and returns false if there is already a default adapter registered
              or setting the default interface is not implemented.
     * @param intf The interface adapter to use as the default.
     */
    bool setDefaultInterface(InterfaceAdapter *intf);

    /**
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
     * @brief Return the interface adapter in effect for this command, whether
     specifically registered or default. May return NULL.
     * @param commandName The command.
     */
    InterfaceAdapter *getCommandInterface(std::string const &commandName);

    /**
     * @brief Return the current default interface adapter for commands.
              May return NULL.
     */
    InterfaceAdapter *getDefaultCommandInterface();

    /**
     * @brief Return the interface adapter in effect for lookups with this state name,
     whether specifically registered or default. May return NULL.
     * @param stateName The state.
     */
    InterfaceAdapter *getLookupInterface(std::string const& stateName);

    /**
     * @brief Query configuration data to determine if a state is only available as telemetry.
     * @param stateName The state.
     * @return True if state is declared telemetry-only, false otherwise.
     * @note In the absence of a declaration, a state is presumed not to be telemetry.
     */
    bool lookupIsTelemetry(std::string const &stateName) const;

    /**
     * @brief Return the current default interface adapter for lookups.
              May return NULL.
     */
    InterfaceAdapter *getDefaultLookupInterface();

    /**
     * @brief Return the interface adapter in effect for planner updates,
              whether specifically registered or default. May return NULL.
     */
    InterfaceAdapter *getPlannerUpdateInterface();

    /**
     * @brief Return the current default interface adapter. May return NULL.
     */
    InterfaceAdapter *getDefaultInterface();

    std::set<InterfaceAdapter *> & getAdapters()
    {
      return m_adapters;
    }

    std::set<InterfaceAdapter *> const & getAdapters() const
    {
      return m_adapters;
    }

    ExecListenerHub *getListenerHub()
    {
      return m_listenerHub;
    }

    /**
     * @brief Returns true if the given adapter is a known interface in the system. False otherwise
     */
    bool isKnown(InterfaceAdapter *intf);

    /**
     * @brief Clears the interface adapter registry.
     */
    void clearAdapterRegistry();

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
     * @brief Construct the input queue specified by the configuration data.
     * @return Pointer to instance of a class derived from InputQueue.
     */
    InputQueue *getInputQueue() const;

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
    InterfaceAdapter *m_defaultCommandInterface;
    InterfaceAdapter *m_defaultLookupInterface;

    //* InterfaceAdapter to use for PlannerUpdate nodes
    InterfaceAdapter *m_plannerUpdateInterface;

    // Maps by command/lookup

    // Interface handler maps
    typedef std::map<std::string, InterfaceAdapter *> InterfaceMap; // TODO: Remove this.
    typedef std::map<std::string, CommandHandler *> CommandHandlerMap;
    typedef std::map<std::string, GenericLookupHandler *> LookupHandlerMap;

    LookupHandlerMap m_lookupMap;
    InterfaceMap m_commandMap;

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
