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

#include "AdapterConfiguration.hh"

#include "AdapterExecInterface.hh"
#include "AdapterFactory.hh"
#include "Debug.hh"
#include "DummyAdapter.hh"
#include "DynamicLoader.h"
#include "Error.hh"
#include "ExecListenerFactory.hh"
#include "ExecListenerFilterFactory.hh"
#include "ExecListenerHub.hh"
// #include "InterfaceManager.hh"
#include "InterfaceSchema.hh"
#include "ListenerFilters.hh"
#include "planLibrary.hh"
#ifdef PLEXIL_WITH_THREADS
#include "SerializedInputQueue.hh"
#else
#include "SimpleInputQueue.hh"
#endif
#include "UtilityAdapter.hh"

#ifdef HAVE_DEBUG_LISTENER
#include "PlanDebugListener.hh"
#endif

#ifdef HAVE_GANTT_LISTENER
#include "GanttListener.hh"
#endif

#ifdef HAVE_IPC_ADAPTER
#include "initIpcAdapter.h"
#endif

#ifdef HAVE_LUV_LISTENER
#include "LuvListener.hh"
#endif

#ifdef HAVE_UDP_ADAPTER
#include "UdpAdapter.hh"
#endif

#ifdef PLEXIL_WITH_UNIX_TIME 
#include "TimeAdapter.hh"
#endif

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL {

  //
  // Handler classes used internally by AdapterConfiguration
  //

  class InternalLookupHandler : public AbstractLookupHandler {
    LookupNowHandler lookupNowHandler;
    SetThresholdsDoubleHandler setThresholdsDoubleHandler;
    SetThresholdsIntHandler setThresholdsIntHandler;
    SubscribeHandler subscribeHandler;
    UnsubscribeHandler unsubscribeHandler;
  public:
    InternalLookupHandler(LookupNowHandler ln, SetThresholdsDoubleHandler setTD = NULL, 
                          SetThresholdsIntHandler setTI = NULL, SubscribeHandler sub = NULL,
                          UnsubscribeHandler unsub = NULL) : 
      lookupNowHandler(ln), setThresholdsDoubleHandler(setTD),
      setThresholdsIntHandler(setTI), subscribeHandler(sub), unsubscribeHandler(unsub) {}
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry) {
      lookupNowHandler(state, cacheEntry);
    }
    void setThresholds(const State &state, double hi, double lo) {
      if(setThresholdsDoubleHandler)
        setThresholdsDoubleHandler(state, hi, lo);
    }
    void setThresholds(const State &state, int32_t hi, int32_t lo) {
      if(setThresholdsIntHandler)
        setThresholdsIntHandler(state, hi, lo);
    }
    void subscribe(const State &state) {
      if(subscribeHandler)
        subscribeHandler(state);
    }
    void unsubscribe(const State &state) {
      if(unsubscribeHandler)
        unsubscribeHandler(state);
    }
  };

  class InternalCommandHandler : public AbstractCommandHandler {
    ExecuteCommandHandler executeCommandHandler;
    AbortCommandHandler abortCommandHandler;
  public:
    InternalCommandHandler(ExecuteCommandHandler exec, AbortCommandHandler abort = NULL) :
      executeCommandHandler(exec), abortCommandHandler(abort) {}
    virtual void executeCommand(Command *cmd) {
      executeCommandHandler(cmd);
    }
    void abortCommand(Command *cmd) {
      if(abortCommandHandler)
        abortCommandHandler(cmd);
    }
  };

  class InternalPlannerUpdateHandler : public AbstractPlannerUpdateHandler {
    PlannerUpdateHandler plannerUpdateHandler;
  public:
    InternalPlannerUpdateHandler(PlannerUpdateHandler updateHandler) : plannerUpdateHandler(updateHandler) {}
    virtual void sendPlannerUpdate(Update *update) {
      plannerUpdateHandler(update);
    }
  };

  class InterfaceLookupHandler : public AbstractLookupHandler {
    InterfaceAdapter* interface;
  public:
    InterfaceLookupHandler(InterfaceAdapter *intf) : interface(intf) {}
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry) {
      interface->lookupNow(state, cacheEntry);
    }
    void setThresholds(const State &state, double hi, double lo) {
      interface->setThresholds(state, hi, lo);
    }
    void setThresholds(const State &state, int32_t hi, int32_t lo) {
      interface->setThresholds(state, hi, lo);
    }
    void subscribe(const State &state) {
      interface->subscribe(state);
    }
    void unsubscribe(const State &state) {
      interface->unsubscribe(state);
    }
  };

  class InterfaceCommandHandler : public AbstractCommandHandler {
    InterfaceAdapter* interface;
  public:
    InterfaceCommandHandler(InterfaceAdapter *intf) : interface(intf) {}
    virtual void executeCommand(Command *cmd) {
      interface->executeCommand(cmd);
    }
    void abortCommand(Command *cmd) {
      interface->invokeAbort(cmd);
    }
  };

  class InterfacePlannerUpdateHandler : public AbstractPlannerUpdateHandler {
    InterfaceAdapter* interface;
  public:
    InterfacePlannerUpdateHandler(InterfaceAdapter *intf) : interface(intf) {}
    virtual void sendPlannerUpdate(Update *update) {
      interface->sendPlannerUpdate(update);
    }
  };


  class TelemetryLookupHandler : public AbstractLookupHandler {
  public:
    TelemetryLookupHandler() {}
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry) {
      // LookupNow not supported for this state, use last cached value
      debugMsg("TelemetryLookupHandler:lookupNow", " lookup is telemetry only, using cached value ");
    }
    virtual void setThresholds(const State &state, double hi, double lo) {
      debugMsg("TelemetryLookupHandler:setThresholds", " lookup is telemetry only, ignoring setThresholds");
    }
    virtual void setThresholds(const State &state, int32_t hi, int32_t lo) {
      debugMsg("TelemetryLookupHandler:setThresholds", " lookup is telemetry only, ignoring setThresholds");
    }
    virtual void subscribe(const State &state) {
      debugMsg("TelemetryLookupHandler:subscribe", " lookup is telemetry only, ignoring subscribe");
    }
    virtual void unsubscribe(const State &state) {
      debugMsg("TelemetryLookupHandler:unsubscribe", " lookup is telemetry only, ignoring unsubscribe");
    }
  };

  AdapterConfiguration::AdapterConfiguration() :
    m_defaultCommandHandler(),
    m_defaultLookupHandler(),
    m_plannerUpdateHandler(),
    m_listenerHub(new ExecListenerHub())
  {
    // Every application has access to the dummy and utility adapters
    REGISTER_ADAPTER(DummyAdapter, "Dummy");
    REGISTER_ADAPTER(UtilityAdapter, "Utility");

#ifdef PLEXIL_WITH_UNIX_TIME
    // Every application has access to the OS-native time adapter
    registerTimeAdapter();
#endif

    registerExecListenerFilters();

    //
    // The reason for all this #ifdef'ery is that when this library is built
    // statically linked, it needs to include the interface modules at link time.
    // When dynamically linked, it doesn't need to pull them in
    // until they're requested, which in some cases will never happen.
    //

#ifdef HAVE_DEBUG_LISTENER
    // Every application should have access to the Plan Debug Listener
#ifdef PIC
    dynamicLoadModule("PlanDebugListener", NULL);
#else
    initPlanDebugListener();
#endif
#endif

#ifdef HAVE_GANTT_LISTENER
    // Every application should have access to the GANTT Listener
#ifdef PIC
    dynamicLoadModule("GanttListener", NULL);
#else
    initGanttListener();
#endif
#endif

#ifdef HAVE_IPC_ADAPTER
    // Every application should have access to the IPC Adapter
#ifdef PIC
    dynamicLoadModule("IpcAdapter", NULL);
#else
    initIpcAdapter();
#endif
#endif

#ifdef HAVE_LUV_LISTENER
    // Every application should have access to the Plexil Viewer (formerly LUV) Listener
#ifdef PIC
    dynamicLoadModule("LuvListener", NULL);
#else
    initLuvListener();
#endif
#endif

#ifdef HAVE_UDP_ADAPTER
    // Every application should have access to the UDP Adapter
#ifdef PIC
    dynamicLoadModule("UdpAdapter", NULL);
#else
    initUdpAdapter();
#endif
#endif

  }

  /**
   * @brief Destructor.
   */
  AdapterConfiguration::~AdapterConfiguration()
  {
    clearAdapterRegistry();

    // unregister and delete adapters
    // *** kludge for buggy std::set template ***
    std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
    while (it != m_adapters.end()) {
      InterfaceAdapter *ia = *it;
      m_adapters.erase(it); // these two lines should be:
      it = m_adapters.begin(); // it = m_adapters.erase(it)
      delete ia;
    }
  }

  /**
   * @brief Constructs interface adapters from the provided XML.
   * @param configXml The XML element used for interface configuration.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::constructInterfaces(pugi::xml_node const configXml)
  {
    if (configXml.empty()) {
      debugMsg("AdapterConfiguration:constructInterfaces",
               " empty configuration, nothing to construct");
      return true;
    }

    debugMsg("AdapterConfiguration:verboseConstructInterfaces", " parsing configuration XML");
    const char* elementType = configXml.name();
    if (strcmp(elementType, InterfaceSchema::INTERFACES_TAG()) != 0) {
      debugMsg("AdapterConfiguration:constructInterfaces",
               " invalid configuration XML: no " << InterfaceSchema::INTERFACES_TAG() << " element");
      return false;
    }

    // Walk the children of the configuration XML element
    // and register the adapter according to the data found there
    pugi::xml_node element = configXml.first_child();
    while (!element.empty()) {
      debugMsg("AdapterConfiguration:verboseConstructInterfaces", " found element " << element.name());
      const char* elementType = element.name();
      if (strcmp(elementType, InterfaceSchema::ADAPTER_TAG()) == 0) {
        // Construct the adapter
        debugMsg("AdapterConfiguration:constructInterfaces",
                 " constructing adapter type \""
                 << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
                 << "\"");
        InterfaceAdapter *adapter = 
          AdapterFactory::createInstance(element, *g_execInterface);
        if (!adapter) {
          warn("constructInterfaces: failed to construct adapter type \""
               << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
               << "\"");
          return false;
        }
        m_adapters.insert(adapter);
      }
      else if (strcmp(elementType, InterfaceSchema::LISTENER_TAG()) == 0) {
        // Construct an ExecListener instance and attach it to the Exec
        debugMsg("AdapterConfiguration:constructInterfaces",
                 " constructing listener type \""
                 << element.attribute(InterfaceSchema::LISTENER_TYPE_ATTR()).value()
                 << '"');
        ExecListener *listener = 
          ExecListenerFactory::createInstance(element);
        if (!listener) {
          warn("constructInterfaces: failed to construct listener type \""
               << element.attribute(InterfaceSchema::LISTENER_TYPE_ATTR()).value()
               << '"');
          return false;
        }
        m_listenerHub->addListener(listener);
      }
      else if (strcmp(elementType, InterfaceSchema::LIBRARY_NODE_PATH_TAG()) == 0) {
        // Add to library path
        const char* pathstring = element.child_value();
        if (*pathstring != '\0') {
          std::vector<std::string> * path = InterfaceSchema::parseCommaSeparatedArgs(pathstring);
          for (std::vector<std::string>::const_iterator it = path->begin();
               it != path->end();
               ++it)
            appendLibraryPath(*it);
          delete path;
        }
      }
      else if (strcmp(elementType, InterfaceSchema::PLAN_PATH_TAG()) == 0) {
        // Add to plan path
        const char* pathstring = element.child_value();
        if (*pathstring != '\0') {
          std::vector<std::string> * path = InterfaceSchema::parseCommaSeparatedArgs(pathstring);
          for (std::vector<std::string>::const_iterator it = path->begin();
               it != path->end();
               ++it)
            m_planPath.push_back(*it);
          delete path;
        }
      }
      else {
        debugMsg("AdapterConfiguration:constructInterfaces",
                 " ignoring unrecognized XML element \""
                 << elementType << "\"");
      }

      element = element.next_sibling();
    }

    debugMsg("AdapterConfiguration:verboseConstructInterfaces", " done.");
    return true;
  }

  /**
   * @brief Performs basic initialization of the interface and all adapters.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::initialize()
  {
    debugMsg("AdapterConfiguration:initialize", " initializing interface adapters");
    bool success = true;
    for (std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         ++it) {
      InterfaceAdapter *a = *it;
      success = a->initialize();
      if (!success) {
        warn("initialize: failed for adapter type \""
             << a->getXml().attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
             << '"');
        m_adapters.erase(it);
        delete a;
        return false;
      }
    }
    success = m_listenerHub->initialize();
    if (!success) {
      warn("initialize: failed to initialize Exec listener(s)");
      return false;
    }
    return success;
  }

  /**
   * @brief Prepares the interface and adapters for execution.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::start()
  {
    debugMsg("AdapterConfiguration:start", " starting interface adapters");
    bool success = true;
    for (std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
         success && it != m_adapters.end();
         ++it) {
      success = (*it)->start();
      if (!success) {
        warn("start: start failed for adapter type \""
             << (*it)->getXml().attribute(InterfaceSchema::ADAPTER_TYPE_ATTR()).value()
             << '"');
        return false;
      }
    }

    success = m_listenerHub->start();
    if (!success) {
      warn("start: failed to start Exec listener(s)");
    }
    return success;
  }

  /**
   * @brief Halts all interfaces.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::stop()
  {
    debugMsg("AdapterConfiguration:stop", " entered");

    // halt adapters
    bool success = true;
    for (std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->stop() && success;

    success = m_listenerHub->stop() && success;

    debugMsg("AdapterConfiguration:stop", " completed");
    return success;
  }

  /**
   * @brief Resets the interface prior to restarting.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::reset()
  {
    debugMsg("AdapterConfiguration:reset", " entered");

    clearAdapterRegistry();
    bool success = true;
    for (std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->reset() && success;

    success = m_listenerHub->reset() && success;
    debugMsg("AdapterConfiguration:reset", " completed");
    return success;
  }

  /**
   * @brief Shuts down the interface.
   * @return true if successful, false otherwise.
   */
  bool AdapterConfiguration::shutdown()
  {
    debugMsg("AdapterConfiguration:shutdown", " entered");
    clearAdapterRegistry();

    bool success = true;
    for (std::set<InterfaceAdapter *>::iterator it = m_adapters.begin();
         it != m_adapters.end();
         ++it)
      success = (*it)->shutdown() && success;
    success = m_listenerHub->shutdown() && success;

    // Clean up
    // *** NYI ***

    debugMsg("AdapterConfiguration:shutdown", " completed");
    return success;
  }

  /**
   * @brief Add an externally constructed interface adapter.
   * @param The adapter ID.
   */
  void AdapterConfiguration::addInterfaceAdapter(InterfaceAdapter *adapter)
  {
    if (m_adapters.find(adapter) == m_adapters.end())
      m_adapters.insert(adapter);
  }

  /**
   * @brief Deletes the given adapter from the interface manager
   * @return true if the given adapter existed and was deleted. False if not found
   */
  bool AdapterConfiguration::deleteAdapter(InterfaceAdapter *intf) {
    int res = m_adapters.erase(intf);
    return res != 0;
  }

  /**
   * @brief Clears the interface adapter registry.
   */
  void AdapterConfiguration:: clearAdapterRegistry() 
  {
    m_lookupMap.clear();
    m_commandMap.clear();
    m_plannerUpdateHandler = NULL;
    m_defaultCommandHandler = NULL;
    m_defaultLookupHandler = NULL;
  }

  /**
   * @brief Add an externally constructed ExecListener.
   * @param listener Pointer to the ExecListener.
   */
  void AdapterConfiguration::addExecListener(ExecListener *listener)
  {
    m_listenerHub->addListener(listener);
  }

  /**
   * @brief Construct the input queue specified by the configuration data.
   * @return Pointer to instance of a class derived from InputQueue.
   */
  // TODO: actually get type from input data
  InputQueue *AdapterConfiguration::getInputQueue() const
  {
    return 
#ifdef PLEXIL_WITH_THREADS
      new SerializedInputQueue();
#else
    new SimpleInputQueue();
#endif
  }

  /**
   * @brief Get the search path for library nodes.
   * @return A reference to the library search path.
   */
  const std::vector<std::string>& AdapterConfiguration::getLibraryPath() const
  {
    return getLibraryPaths();
  }

  /**
   * @brief Get the search path for plans.
   * @return A reference to the plan search path.
   */
  const std::vector<std::string>& AdapterConfiguration::getPlanPath() const
  {
    return m_planPath;
  }

  /**
   * @brief Add the specified directory name to the end of the library node loading path.
   * @param libdir The directory name.
   */
  void AdapterConfiguration::addLibraryPath(const std::string &libdir)
  {
    appendLibraryPath(libdir);
  }

  /**
   * @brief Add the specified directory names to the end of the library node loading path.
   * @param libdirs The vector of directory names.
   */
  void AdapterConfiguration::addLibraryPath(const std::vector<std::string>& libdirs)
  {
    for (std::vector<std::string>::const_iterator it = libdirs.begin();
         it != libdirs.end();
         ++it)
      appendLibraryPath(*it);
  }

  /**
   * @brief Add the specified directory name to the end of the plan loading path.
   * @param libdir The directory name.
   */
  void AdapterConfiguration::addPlanPath(const std::string &libdir)
  {
    m_planPath.push_back(libdir);
  }

  /**
   * @brief Add the specified directory names to the end of the plan loading path.
   * @param libdirs The vector of directory names.
   */
  void AdapterConfiguration::addPlanPath(const std::vector<std::string>& libdirs)
  {
    for (std::vector<std::string>::const_iterator it = libdirs.begin();
         it != libdirs.end();
         ++it) {
      m_planPath.push_back(*it);
    }
  }

  /**
   * @brief Register the given state to handle telemetry lookups
   * @param stateName The name of the state
   */
  bool AdapterConfiguration::registerTelemetryLookup(std::string const &stateName) {
    return registerLookupObjectHandler(stateName, new TelemetryLookupHandler());
  }

  /**
   * @brief Register the given object to be a handler for lookups to this state
   * @param stateName The name of the state to map to this object
   * @param handler An object handler to register as the handler.
   */
  bool AdapterConfiguration::registerLookupObjectHandler(std::string const &stateName, 
                                                         AdapterConfiguration::AbstractLookupHandler *handler) {
    LookupHandlerMap::iterator it = m_lookupMap.find(stateName);
    if (it == m_lookupMap.end()) {
      // Not found, OK to add
      debugMsg("AdapterConfiguration:registerLookupHandler",
               " registering handler for lookup of '" << stateName << "'");
      m_lookupMap.insert(std::pair<std::string, AdapterConfiguration::AbstractLookupHandler *>(stateName, handler));
      return true;
    } else {
      debugMsg("AdapterConfiguration:registerLookupHandler",
               " handler already registered for lookup of '" << stateName << "'");
      return false;
    }
  }

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
  bool AdapterConfiguration::registerLookupHandler(std::string const &stateName,
                                                   LookupNowHandler lookupNow,
                                                   SetThresholdsDoubleHandler setThresholdsDouble,
                                                   SetThresholdsIntHandler setThresholdsInt,
                                                   SubscribeHandler subscribe,
                                                   UnsubscribeHandler unsubscribe) {
    return registerLookupObjectHandler(stateName, new InternalLookupHandler(
                                                                            lookupNow,
                                                                            setThresholdsDouble,
                                                                            setThresholdsInt,
                                                                            subscribe,
                                                                            unsubscribe));
  }
  /**
   * @brief Return the lookup handler in effect for lookups with this state name,
   whether specifically registered or default. May return NULL.
   * @param stateName The state.
   */
  AdapterConfiguration::AbstractLookupHandler *AdapterConfiguration::getLookupHandler(std::string const &stateName) const
  {
    LookupHandlerMap::const_iterator it = m_lookupMap.find(stateName);
    if (it != m_lookupMap.end()) {
      debugMsg("AdapterConfiguration:getLookupHandler",
               " found specific handler " << (*it).second
               << " for lookup '" << stateName << "'");
      return (*it).second;
    }
    debugMsg("AdapterConfiguration:getLookupHandler",
             " no handler registered for lookup '" << stateName << "'");
    if (m_defaultLookupHandler) {
      debugMsg("AdapterConfiguration:getLookupHandler",
               " using defualt handler for lookup '" << stateName << "'");
      return m_defaultLookupHandler;
    }
    return NULL;
  }

  /**
   * @brief Query configuration data to determine if a state is only available as telemetry.
   * @param stateName The state.
   * @return True if state is declared telemetry-only, false otherwise.
   * @note In the absence of a declaration, a state is presumed not to be telemetry.
   */
  bool AdapterConfiguration::lookupIsTelemetry(std::string const &stateName) const
  {
    return dynamic_cast<AdapterConfiguration::TelemetryLookupHandler*>(this->getLookupHandler(stateName)) != NULL;
  }

  /**
   * @brief Register the given object handler for lookups to this state.
   Returns true if successful.  Fails and returns false
   if the state name already has a handler registered
   or registering an object handler is not implemented.
   * @param stateName The name of the state to map to this object
   * @param handler An object to register as the handler.
   */
  bool AdapterConfiguration::registerCommandObjectHandler(std::string const &stateName, AdapterConfiguration::AbstractCommandHandler *handler) {
    AdapterConfiguration::CommandHandlerMap::iterator it = m_commandMap.find(stateName);
    if (it == m_commandMap.end()) {
      // Not found, OK to add
      debugMsg("AdapterConfiguration:registerCommandObjectHandler",
               " registering handler for command '" << stateName << "'");
      m_commandMap.insert(std::pair<std::string, AbstractCommandHandler *>(stateName, handler));

      debugMsg("AdapterConfiguration:registerCommandObjectHandler",
               " done registering handler for command '" << stateName << "'");
      return true;
    } else {
      debugMsg("AdapterConfiguration:registerCommandObjectHandler",
               " handler already registered for command '" << stateName << "'");
      return false;
    }
  }

  /**
   * @brief Register the given handler for lookups to this state.
   Returns true if successful.  Fails and returns false
   if the state name already has a handler registered
   or registering a handler is not implemented.
   * @param stateName The name of the state to map to this handler.
   * @param execCmd The function to call when this command is executed
   * @param abortCmd The function to call when this command is aborted
   */
  bool AdapterConfiguration::registerCommandHandler(std::string const &stateName,
                                                    ExecuteCommandHandler execCmd,
                                                    AbortCommandHandler abortCmd) {
    return registerCommandObjectHandler(stateName, new InternalCommandHandler(
                                                                              execCmd,
                                                                              abortCmd)); //TODO: Clean up
  }

  /**
   * @brief Return the command object handler in effect for commands with this state name,
   whether specifically registered or default. May return NULL.
   * @param stateName The state.
   */
  AdapterConfiguration::AbstractCommandHandler *AdapterConfiguration::getCommandHandler(std::string const& stateName) const
  {
    CommandHandlerMap::const_iterator it = m_commandMap.find(stateName);
    if (it != m_commandMap.end()) {
      debugMsg("AdapterConfiguration:getCommandHandler",
               " found specific handler " << (*it).second
               << " for lookup '" << stateName << "'");
      return (*it).second;
    }
    debugMsg("AdapterConfiguration:getCommandHandler",
             " no handler registered for command '" << stateName << "'");
    if (m_defaultCommandHandler) {
      debugMsg("AdapterConfiguration:getCommandHandler",
               " using defualt handler for command '" << stateName << "'");
      return m_defaultCommandHandler;
    }

    return NULL;
  }

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
  bool AdapterConfiguration::setDefaultLookupObjectHandler(AdapterConfiguration::AbstractLookupHandler *handler) {
    if (m_defaultLookupHandler) {
      debugMsg("AdapterConfiguration:setDefaultLookupHandler",
               " attempt to overwrite default lookup handler " << m_defaultLookupHandler);
      return false;
    }
    m_defaultLookupHandler = handler;
    debugMsg("AdapterConfiguration:setDefaultLookupHanlder",
             " setting default lookup handler " << m_defaultLookupHandler);
    return true;
  }

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
  bool AdapterConfiguration::setDefaultLookupHandler(          
                                                     LookupNowHandler lookupNow,
                                                     SetThresholdsDoubleHandler setThresholdsDouble,
                                                     SetThresholdsIntHandler setThresholdsInt,
                                                     SubscribeHandler subscribe,
                                                     UnsubscribeHandler unsubscribe) {
    return setDefaultLookupObjectHandler(new AdapterConfiguration::InternalLookupHandler(
                                                                                         lookupNow,
                                                                                         setThresholdsDouble,
                                                                                         setThresholdsInt,
                                                                                         subscribe,
                                                                                         unsubscribe));
  }

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
  bool AdapterConfiguration::setDefaultCommandObjectHandler(AdapterConfiguration::AbstractCommandHandler *handler) {
    if (m_defaultCommandHandler) {
      debugMsg("AdapterConfiguration:setDefaultCommandHanlder",
               " attempt to overwrite default command handler " << m_defaultCommandHandler);
      return false;
    }
    m_defaultCommandHandler = handler;
    debugMsg("AdapterConfiguration:setDefaultCommandHandler",
             " setting default command handler " << m_defaultCommandHandler);
    return true;
  }

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
  bool AdapterConfiguration::setDefaultCommandHandler(
                                                      ExecuteCommandHandler execCmd,
                                                      AbortCommandHandler abortCmd) {
    return setDefaultCommandObjectHandler(new AdapterConfiguration::InternalCommandHandler(
                                                                                           execCmd,
                                                                                           abortCmd));
  }

  /**
   * @brief Return the current default handler for commands.
   May return NULL. Returns NULL if default handlers are not implemented.
  */
  AdapterConfiguration::AbstractCommandHandler *AdapterConfiguration:: getDefaultCommandHandler() const {
    return m_defaultCommandHandler;
  }

  /**
   * @brief Return the current default handler for lookups.
   May return NULL. Returns NULL if default lookup handlers are not implemented.
  */
  AdapterConfiguration::AbstractLookupHandler *AdapterConfiguration:: getDefaultLookupHandler() const {
    return m_defaultLookupHandler;
  }

  /**
   * @brief Register the given object handler for planner updates.
   Returns true if successful.  Fails and returns false
   iff a handler is already registered
   or setting the default planner update interface is not implemented.
   * @param handler The object handler to handle planner updates.
   */
  bool AdapterConfiguration::registerPlannerUpdateObjectHandler(AbstractPlannerUpdateHandler *updateHandler) {
    if (m_plannerUpdateHandler) {
      debugMsg("AdapterConfiguration:registerPlannerUpdateObjectHandler",
               " planner update interface already registered");
      return false;
    }
    m_plannerUpdateHandler = updateHandler;
    debugMsg("AdapterConfiguration:registerPlannerUpdateObjectHandler",
             " registering planner update interface " << m_plannerUpdateHandler);
    return true;
  }

  /**
   * @brief Register the given function handler for planner updates.
   Returns true if successful.  Fails and returns false
   iff a handler is already registered
   or setting the default planner update interface is not implemented.
   * @param sendPlannerUpdate The function to call for planner updates.
   */
  bool AdapterConfiguration::registerPlannerUpdateHandler(PlannerUpdateHandler updateHandler) {
    return registerPlannerUpdateObjectHandler(new InternalPlannerUpdateHandler(updateHandler));
  }

  /**
   * @brief Return the object handler in effect for planner updates,
   whether specifically registered or default. May return NULL.
  */
  AdapterConfiguration::AbstractPlannerUpdateHandler *AdapterConfiguration::getPlannerUpdateHandler() const {
    if (!m_plannerUpdateHandler) {
      debugMsg("AdapterConfiguration:getPlannerUpdateHandler",
               " no plannerUpdateHandler registered returning NULL");
      return NULL;
    }
    debugMsg("AdapterConfiguration:getPlannerUpdateHandler",
             " found specific handler " << m_plannerUpdateHandler);
    return m_plannerUpdateHandler;
  }

  // Initialize global variable
  AdapterConfiguration *g_configuration = NULL;

  /* --------------------------------------------------------------------- */
  /* -------------------- Deprecated Interface Methods ------------------- */
  /* --------------------------------------------------------------------- */

  /**
   * @deprecated interface adapters are no longer stored.
   * @brief Returns true if the given adapter is a known interface in the system. False otherwise
   */
  bool AdapterConfiguration::isKnown(InterfaceAdapter *intf) {
    return false;
  }

  /**
   * @deprecated
   * @brief Register the given interface adapter as the default.
   * @param adapter The interface adapter.
   */
  void AdapterConfiguration::defaultRegisterAdapter(InterfaceAdapter *adapter) 
  {
    debugMsg("AdapterConfiguration:defaultRegisterAdapter", " for adapter " << adapter);
    // Walk the children of the configuration XML element
    // and register the adapter according to the data found there
    pugi::xml_node element = adapter->getXml().first_child();
    while (!element.empty()) {
      const char* elementType = element.name();
      if (strcmp(elementType, InterfaceSchema::DEFAULT_ADAPTER_TAG()) == 0) {
        setDefaultInterface(adapter);
      } 
      else if (strcmp(elementType, InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG()) == 0) {
        setDefaultCommandInterface(adapter);
      }
      else if (strcmp(elementType, InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG()) == 0) {
        setDefaultLookupInterface(adapter);
      }
      else if (strcmp(elementType, InterfaceSchema::PLANNER_UPDATE_TAG()) == 0) {
        registerPlannerUpdateInterface(adapter);
      }
      else if (strcmp(elementType, InterfaceSchema::COMMAND_NAMES_TAG()) == 0) {
        const pugi::xml_node firstChild = element.first_child();
        const char* text = NULL;
        if (!firstChild.empty() && firstChild.type() == pugi::node_pcdata)
          text = firstChild.value();
        checkError(text && *text != '\0',
                   "registerAdapter: Invalid configuration XML: "
                   << InterfaceSchema::COMMAND_NAMES_TAG()
                   << " requires one or more comma-separated command names");
        std::vector<std::string> * cmdNames = InterfaceSchema::parseCommaSeparatedArgs(text);
        for (std::vector<std::string>::const_iterator it = cmdNames->begin(); it != cmdNames->end(); ++it)
          registerCommandInterface(*it, adapter);
        delete cmdNames;
      } 
      else if (strcmp(elementType, InterfaceSchema::LOOKUP_NAMES_TAG()) == 0) {
        const pugi::xml_node firstChild = element.first_child();
        const char* text = NULL;
        if (!firstChild.empty() && firstChild.type() == pugi::node_pcdata)
          text = firstChild.value();
        checkError(text && *text != '\0',
                   "registerAdapter: Invalid configuration XML: "
                   << InterfaceSchema::LOOKUP_NAMES_TAG()
                   << " requires one or more comma-separated lookup names");
        std::vector<std::string> * lookupNames = InterfaceSchema::parseCommaSeparatedArgs(text);
        bool telemOnly = element.attribute(InterfaceSchema::TELEMETRY_ONLY_ATTR()).as_bool();
        for (std::vector<std::string>::const_iterator it = lookupNames->begin(); it != lookupNames->end(); ++it)
          registerLookupInterface(*it, adapter, telemOnly);
        delete lookupNames;
      }
      // ignore other tags, they're for adapter's use

      element = element.next_sibling();
    }
  }

  /**
   * @deprecated Use registerCommandHandler for improved performance.
   * @brief Register the given interface adapter for this command.
   Returns true if successful.  Fails and returns false
   iff the command name already has an adapter registered
   or setting a command interface is not implemented.
   * @param commandName The command to map to this adapter.
   * @param intf The interface adapter to handle this command.
   */
  bool AdapterConfiguration::registerCommandInterface(std::string const &commandName,
                                                      InterfaceAdapter *intf) {
    return this->registerCommandObjectHandler(commandName, new InterfaceCommandHandler(intf)); //TODO: handle cleanup
  }

  /**
   * @deprecated Use registerLookupHandler for improved performance.
   * @brief Register the given interface adapter for lookups to this state.
   Returns true if successful.  Fails and returns false
   if the state name already has an adapter registered
   or registering a lookup interface is not implemented.
   * @param stateName The name of the state to map to this adapter.
   * @param intf The interface adapter to handle this lookup.
   * @param telemetryOnly False if this interface implements LookupNow, true otherwise.
   */
  bool AdapterConfiguration::registerLookupInterface(std::string const &stateName,
                                                     InterfaceAdapter *intf,
                                                     bool telemetryOnly) {
    if (telemetryOnly)
      return registerTelemetryLookup(stateName);
    return this->registerLookupObjectHandler(stateName, new InterfaceLookupHandler(intf));
  }

  /**
   * @deprecated
   * @brief Register the given interface adapter as the default for all lookups and commands
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered
   or setting the default interface is not implemented.
   * @param intf The interface adapter to use as the default.
   */
  bool AdapterConfiguration::setDefaultInterface(InterfaceAdapter *intf) {
    return this->setDefaultCommandInterface(intf)
      && this->setDefaultLookupInterface(intf)
      && this->registerPlannerUpdateInterface(intf);
  }

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
  bool AdapterConfiguration::setDefaultLookupInterface(InterfaceAdapter *intf) {
    return this->setDefaultLookupObjectHandler(new InterfaceLookupHandler(intf));
  }

  /**
   * @deprecated use setDefaultCommandHandler()
   * @brief Register the given interface adapter as the default for commands.
   This interface will be used for all commands which do not have
   a specific adapter.
   Returns true if successful.
   Fails and returns false if there is already a default command adapter registered.
   * @param intf The interface adapter to use as the default.
   * @return True if successful, false if there is already a default adapter registered.
   */
  bool AdapterConfiguration::setDefaultCommandInterface(InterfaceAdapter *intf) {
    return this->setDefaultCommandObjectHandler(new InterfaceCommandHandler(intf));
  }

  /**
   * @deprecated
   * @brief Return the interface adapter in effect for this command, whether
   specifically registered or default. May return NULL.
   * @param commandName The command.
   */
  InterfaceAdapter *AdapterConfiguration:: getCommandInterface(std::string const &commandName) const {
    return NULL;
  }

  /**
   * @deprecated
   * @brief Return the current default interface adapter for commands.
   May return NULL. Returns NULL if default interfaces are not implemented.
  */
  InterfaceAdapter *AdapterConfiguration:: getDefaultCommandInterface() const {
    return NULL;
  }

  /**
   * @deprecated
   * @brief Return the interface adapter in effect for lookups with this state name,
   whether specifically registered or default. May return NULL. Returns NULL if default interfaces are not implemented.
   * @param stateName The state.
   */
  InterfaceAdapter *AdapterConfiguration:: getLookupInterface(std::string const &stateName) const {
    return NULL;
  }

  /**
   * @deprecated
   * @brief Return the current default interface adapter for lookups.
   May return NULL.
  */
  InterfaceAdapter *AdapterConfiguration:: getDefaultLookupInterface() const {
    return NULL;
  }

  /**
   * @deprecated
   * @brief Return the current default interface adapter. May return NULL.
   */
  InterfaceAdapter *AdapterConfiguration:: getDefaultInterface() const {
    return NULL;
  }

  /**
   * @deprecated
   * @brief Register the given interface adapter for planner updates.
   Returns true if successful.  Fails and returns false
   iff an adapter is already registered
   or setting the default planner update interface is not implemented.
   * @param intf The interface adapter to handle planner updates.
   */
  bool AdapterConfiguration::registerPlannerUpdateInterface(InterfaceAdapter *intf) {
    return this->registerPlannerUpdateObjectHandler(new InterfacePlannerUpdateHandler(intf)); //TODO: cleanup
  }

  /**
   * @deprecated
   * @brief Return the interface adapter in effect for planner updates,
   whether specifically registered or default. May return NULL.
   Returns NULL if default interfaces are not defined.
  */
  InterfaceAdapter *AdapterConfiguration:: getPlannerUpdateInterface() const {
    return NULL;
  }

}
