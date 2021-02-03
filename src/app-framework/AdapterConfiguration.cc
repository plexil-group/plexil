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
#include "Command.hh"
#include "Debug.hh"
#include "DynamicLoader.h"
#include "Error.hh"
#include "ExecListenerFactory.hh"
#include "ExecListenerHub.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "Launcher.hh"
#include "ListenerFilters.hh"
#include "planLibrary.hh"
#include "SimpleSet.hh"
#include "State.hh"

#ifdef PLEXIL_WITH_THREADS
#include "SerializedInputQueue.hh"
#else
#include "SimpleInputQueue.hh"
#endif

#include "UtilityAdapter.hh"

#ifdef PLEXIL_WITH_UNIX_TIME 
#include "TimeAdapter.hh"
#endif

//
// The reason for all this #ifdef'ery is that when this library is built
// statically linked, it needs to include the interface modules at link time.
// When dynamically linked, it doesn't need to pull them in
// until they're requested, which in most cases will never happen.
//
// See the AdapterConfigurationImpl constructor for more information.
//

#ifndef PIC
#ifdef HAVE_DEBUG_LISTENER
#include "PlanDebugListener.hh"
#endif

#ifdef HAVE_GANTT_LISTENER
#include "GanttListener.hh"
#endif

#ifdef HAVE_IPC_ADAPTER
#include "IpcAdapter.h"
#endif

#ifdef HAVE_LUV_LISTENER
#include "LuvListener.hh"
#endif

#ifdef HAVE_UDP_ADAPTER
#include "UdpAdapter.h"
#endif
#endif // PIC

#include <map>

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL {

  // Construct global variable
  AdapterConfiguration *g_configuration = NULL;

///////////////////////// Handler Implementations //////////////////////////

  //*
  //
  // @brief The DefaultLookupHandler class does nothing more than
  // print debug messages (when enabled) when one of its methods is
  // called.
  //
  
  class DefaultLookupHandler : public LookupHandler
  {
  public:
    DefaultLookupHandler()
    {
    }

    virtual ~DefaultLookupHandler()
    {
    }
    
    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry)
    {
      debugMsg("DefaultLookupHandler:lookupNow", ' ' << state << " returning UNKNOWN");
    }

    virtual void subscribe(const State &state, AdapterExecInterface * /* intf */)
    {
      debugMsg("DefaultLookupHandler:subscribe", ' ' << state);
    }
    
    virtual void unsubscribe(const State &state)
    {
      debugMsg("DefaultLookupHandler:unsubscribe", ' ' << state);
    }

    virtual void setThresholds(const State &state,
                               double /* hi */,
                               double /* lo */)
    {
      debugMsg("DefaultLookupHandler:setThresholds",
               ' ' << state << " (Real)");
    }

    virtual void setThresholds(const State &state,
                               int32_t /* hi */,
                               int32_t /* lo */)
    {
      debugMsg("DefaultLookupHandler:setThresholds",
               ' ' << state << " (Integer)");
    }
  };

  //*
  //
  // @brief A wrapper class for user-provided lookup handler
  // functions.
  //

  struct LookupHandlerWrapper : public LookupHandler
  {
    LookupNowHandler m_lookupNowFn;
    SubscribeHandler m_subscribeFn;
    UnsubscribeHandler m_unsubscribeFn;
    SetThresholdsHandlerReal m_setThresholdsRealFn;
    SetThresholdsHandlerInteger m_setThresholdsIntFn;

  public:
    LookupHandlerWrapper(LookupNowHandler lkup,
                         SubscribeHandler sub,
                         UnsubscribeHandler unsub,
                         SetThresholdsHandlerReal setThReal,
                         SetThresholdsHandlerInteger setThInt)
      : m_lookupNowFn(lkup),
        m_subscribeFn(sub),
        m_unsubscribeFn(unsub),
        m_setThresholdsRealFn(setThReal),
        m_setThresholdsIntFn(setThInt)
    {
    }

    virtual ~LookupHandlerWrapper()
    {
    }

    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry)
    {
      if (m_lookupNowFn)
        (m_lookupNowFn)(state, cacheEntry);
    }

    virtual void setThresholds(const State &state, double hi, double lo)
    {
      if (m_setThresholdsRealFn)
        (m_setThresholdsRealFn)(state, hi, lo);
    }

    virtual void setThresholds(const State &state, int32_t hi, int32_t lo)
    {
      if (m_setThresholdsIntFn)
        (m_setThresholdsIntFn)(state, hi, lo);
    }

    virtual void subscribe(const State &state, AdapterExecInterface *intf)
    {
      if (m_subscribeFn)
        (m_subscribeFn)(state, intf);
    }

    virtual void unsubscribe(const State &state)
    {
      if (m_unsubscribeFn)
        (m_unsubscribeFn)(state);
    }
  };

  //*
  //
  // @brief The DefaultCommandHandler class prints debug messages
  // (when enabled) and responds with the appropriate
  // CommandHandleValue when one of its methods is called.
  //

  class DefaultCommandHandler : public CommandHandler
  {
  public:
    DefaultCommandHandler()
    {
    }

    virtual ~DefaultCommandHandler()
    {
    }
    
    virtual void executeCommand(Command *cmd, AdapterExecInterface *intf)
    {
      debugMsg("DefaultCommandHandler:executeCommand",
               ' ' << cmd->getName());
      intf->handleCommandAck(cmd, COMMAND_SUCCESS);
      intf->notifyOfExternalEvent();
    }
    
    virtual void abortCommand(Command *cmd, AdapterExecInterface *intf)
    {
      debugMsg("DefaultCommandHandler:abortCommand",
               ' ' << cmd->getName());
      intf->handleCommandAbortAck(cmd, true);
      intf->notifyOfExternalEvent();
    }
  };

  //*
  //
  // @brief A wrapper class for user-provided command handler
  // functions.
  //

  class CommandHandlerWrapper : public CommandHandler
  {
  private:
    ExecuteCommandHandler m_executeCommandFn;
    AbortCommandHandler m_abortCommandFn;

  public:

    CommandHandlerWrapper(ExecuteCommandHandler exec,
                          AbortCommandHandler abort)
      : m_executeCommandFn(exec),
        m_abortCommandFn(abort)
    {
    }

    virtual ~CommandHandlerWrapper()
    {
    }

    virtual void executeCommand(Command *cmd, AdapterExecInterface *intf)
    {
      (m_executeCommandFn)(cmd, intf);
    }

    virtual void abortCommand(Command *cmd, AdapterExecInterface *intf)
    {
      (m_abortCommandFn)(cmd, intf);
    }
  };

  class AdapterLookupHandler : public LookupHandler
  {
  private:
    InterfaceAdapter* m_adapter;

  public:
    AdapterLookupHandler(InterfaceAdapter *intf)
      : m_adapter(intf)
    {
    }

    virtual ~AdapterLookupHandler()
    {
    }

    virtual void lookupNow(const State &state, StateCacheEntry &cacheEntry)
    {
      m_adapter->lookupNow(state, cacheEntry);
    }

    virtual void setThresholds(const State &state, double hi, double lo)
    {
      m_adapter->setThresholds(state, hi, lo);
    }

    virtual void setThresholds(const State &state, int32_t hi, int32_t lo)
    {
      m_adapter->setThresholds(state, hi, lo);
    }

    virtual void subscribe(const State &state, AdapterExecInterface * /* ignored */)
    {
      m_adapter->subscribe(state);
    }

    virtual void unsubscribe(const State &state)
    {
      m_adapter->unsubscribe(state);
    }
  };

  class AdapterCommandHandler : public CommandHandler
  {
  private:
    InterfaceAdapter* m_adapter;

  public:
    AdapterCommandHandler(InterfaceAdapter *intf)
      : m_adapter(intf)
    {
    }

    virtual ~AdapterCommandHandler()
    {
    }
      
    virtual void executeCommand(Command *cmd, AdapterExecInterface * /* ignored */)
    {
      m_adapter->executeCommand(cmd);
    }

    virtual void abortCommand(Command *cmd, AdapterExecInterface * /* ignored */)
    {
      m_adapter->invokeAbort(cmd);
    }
  };

  class AdapterPlannerUpdateHandler : public PlannerUpdateHandler
  {
  private:
    InterfaceAdapter *m_adapter;

  public:
    AdapterPlannerUpdateHandler(InterfaceAdapter *adapter)
      : m_adapter(adapter)
    {
    }
    
    virtual ~AdapterPlannerUpdateHandler()
    {
    }
    
    virtual void operator()(Update *upd, AdapterExecInterface * /* ignored */)
    {
      m_adapter->sendPlannerUpdate(upd);
    }
  };

  class PlannerUpdateHandlerWrapper : public PlannerUpdateHandler
  {
  private:
    PlannerUpdateFn m_updateFn;

  public:
    PlannerUpdateHandlerWrapper(PlannerUpdateFn updateFn)
      : m_updateFn(updateFn)
    {
    }

    virtual ~PlannerUpdateHandlerWrapper()
    {
    }

    virtual void operator()(Update *upd, AdapterExecInterface *intf)
    {
      (m_updateFn)(upd, intf);
    }
  };


  //*
  // @class AdapterConfigurationImpl
  // @brief Implementation class for AdapterConfiguration
  //

  class AdapterConfigurationImpl : public AdapterConfiguration
  {
  private:

    //
    // Typedefs
    //

    typedef std::map<std::string, CommandHandler *> CommandHandlerMap;
    typedef std::map<std::string, LookupHandler *> LookupHandlerMap;

    typedef SimpleSet<CommandHandler *> CommandHandlerSet;
    typedef SimpleSet<LookupHandler *> LookupHandlerSet;
    typedef SimpleSet<InterfaceAdapter *> InterfaceAdapterSet;

    //
    // Member variables
    //

    // Maps by command/lookup
    LookupHandlerMap m_lookupMap;
    CommandHandlerMap m_commandMap;

    // Sets of all known handlers,
    // to facilitate deletion in the destructor
    CommandHandlerSet m_commandHandlers;
    LookupHandlerSet m_lookupHandlers;
    
    //* Set of all known InterfaceAdapter instances
    InterfaceAdapterSet m_adapters;

    //* List of directory names for plan file search paths
    std::vector<std::string> m_planPath;

    //* ExecListener hub
    ExecListenerHub *m_listenerHub;

    //* Default handlers
    CommandHandler *m_defaultCommandHandler;
    LookupHandler *m_defaultLookupHandler;

    //* Dummy handler for telemetry lookups
    LookupHandler *m_telemetryLookupHandler;

    //* Handler to use for Update nodes
    PlannerUpdateHandler *m_plannerUpdateHandler;

  public:

    AdapterConfigurationImpl()
      : m_listenerHub(new ExecListenerHub()),
        m_defaultCommandHandler(new DefaultCommandHandler()),
        m_defaultLookupHandler(new DefaultLookupHandler()),
        m_telemetryLookupHandler(NULL),
        m_plannerUpdateHandler(new PlannerUpdateHandlerWrapper(&defaultPlannerUpdateFn))
    {
      m_commandHandlers.insert(m_defaultCommandHandler);
      m_lookupHandlers.insert(m_defaultLookupHandler);

      m_telemetryLookupHandler = m_defaultLookupHandler;

      // Every application has access to the utility and launcher adapters
      initUtilityAdapter();
      initLauncher();

#ifdef PLEXIL_WITH_UNIX_TIME
      // Every application has access to the OS-native time adapter
      registerTimeAdapter();
#endif

      registerExecListenerFilters();

      //
      // The reason for all this #ifdef'ery is that when this library is built
      // statically linked, it needs to include the interface modules at link time.
      // When dynamically linked, it doesn't need to pull them in
      // until they're requested, which in most cases will never happen.
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
    virtual ~AdapterConfigurationImpl()
    {
      m_commandMap.clear();
      m_lookupMap.clear();

      m_defaultCommandHandler = NULL;
      m_defaultLookupHandler = NULL;
      m_telemetryLookupHandler = NULL;

      CommandHandlerSet::iterator csit = m_commandHandlers.begin();
      while (csit != m_commandHandlers.end()) {
        CommandHandler *ch = *csit;
        m_commandHandlers.erase(csit);
        csit = m_commandHandlers.begin();
        delete ch;
      }

      LookupHandlerSet::iterator lsit = m_lookupHandlers.begin();
      while (lsit != m_lookupHandlers.end()) {
        LookupHandler *lh = *lsit;
        m_lookupHandlers.erase(lsit);
        lsit = m_lookupHandlers.begin();
        delete lh;
      }

      InterfaceAdapterSet::iterator it = m_adapters.begin();
      while (it != m_adapters.end()) {
        InterfaceAdapter *ia = *it;
        m_adapters.erase(it);
        it = m_adapters.begin(); 
        delete ia;
      }

      delete m_plannerUpdateHandler;
      delete m_listenerHub;
    }

    // FIXME:
    // * Need new constructor paradigm for handlers
    virtual bool constructInterfaces(pugi::xml_node const configXml)
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
        debugMsg("AdapterConfiguration:verboseConstructInterfaces",
                 " found element " << element.name());
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

    // FIXME:
    // * Need new initializer paradigm for handlers
    virtual bool initialize()
    {
      debugMsg("AdapterConfiguration:initialize", " initializing interface adapters");
      bool success = true;
      for (InterfaceAdapterSet::iterator it = m_adapters.begin();
           success && it != m_adapters.end();
           ++it) {
        InterfaceAdapter *a = *it;
        success = a->initialize(this);
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

    // FIXME:
    // * Need new startup paradigm for handlers
    virtual bool start()
    {
      debugMsg("AdapterConfiguration:start", " starting interface adapters");
      bool success = true;
      for (InterfaceAdapterSet::iterator it = m_adapters.begin();
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

    // FIXME:
    // * Need new stop paradigm for handlers
    virtual bool stop()
    {
      debugMsg("AdapterConfiguration:stop", " entered");

      // halt adapters
      bool success = true;
      for (InterfaceAdapterSet::iterator it = m_adapters.begin();
           it != m_adapters.end();
           ++it)
        success = (*it)->stop() && success;

      success = m_listenerHub->stop() && success;

      debugMsg("AdapterConfiguration:stop", " completed");
      return success;
    }

    // FIXME:
    // * Do we need this at all??
    // * Need new reset paradigm for handlers
    virtual bool reset()
    {
      debugMsg("AdapterConfiguration:reset", " entered");

      bool success = true;
      for (InterfaceAdapterSet::iterator it = m_adapters.begin();
           it != m_adapters.end();
           ++it)
        success = (*it)->reset() && success;

      success = m_listenerHub->reset() && success;
      debugMsg("AdapterConfiguration:reset", " completed");
      return success;
    }

    // FIXME:
    // * Need new shutdown paradigm for handlers
    virtual bool shutdown()
    {
      debugMsg("AdapterConfiguration:shutdown", " entered");

      bool success = true;
      for (InterfaceAdapterSet::iterator it = m_adapters.begin();
           it != m_adapters.end();
           ++it)
        success = (*it)->shutdown() && success;
      success = m_listenerHub->shutdown() && success;

      // Clean up
      // *** NYI ***

      debugMsg("AdapterConfiguration:shutdown", " completed");
      return success;
    }

    virtual void addInterfaceAdapter(InterfaceAdapter *adapter)
    {
      if (m_adapters.find(adapter) == m_adapters.end())
        m_adapters.insert(adapter);
    }

    virtual void addExecListener(ExecListener *listener)
    {
      m_listenerHub->addListener(listener);
    }

    // TODO: actually get queue type from input data
    virtual InputQueue *makeInputQueue() const
    {
      return 
#ifdef PLEXIL_WITH_THREADS
        new SerializedInputQueue();
#else
      new SimpleInputQueue();
#endif
    }

    virtual const std::vector<std::string>& getLibraryPath() const
    {
      return getLibraryPaths();
    }

    /**
     * @brief Get the search path for plans.
     * @return A reference to the plan search path.
     */
    virtual const std::vector<std::string>& getPlanPath() const
    {
      return m_planPath;
    }

    /**
     * @brief Add the specified directory name to the end of the library node loading path.
     * @param libdir The directory name.
     */
    virtual void addLibraryPath(const std::string &libdir)
    {
      appendLibraryPath(libdir);
    }

    /**
     * @brief Add the specified directory names to the end of the library node loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addLibraryPath(const std::vector<std::string>& libdirs)
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
    virtual void addPlanPath(const std::string &libdir)
    {
      m_planPath.push_back(libdir);
    }

    /**
     * @brief Add the specified directory names to the end of the plan loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addPlanPath(const std::vector<std::string>& libdirs)
    {
      for (std::vector<std::string>::const_iterator it = libdirs.begin();
           it != libdirs.end();
           ++it) {
        m_planPath.push_back(*it);
      }
    }

    virtual void registerTelemetryLookup(std::string const &stateName)
    {
       registerLookupHandler(stateName, m_telemetryLookupHandler);
    }

    virtual void registerLookupHandler(std::string const &stateName, 
                                       LookupHandler *handler)
    {
      assertTrue_2(handler, "registerLookupHandler: LookupHandler pointer must not be NULL");
      debugStmt("AdapterConfiguration:registerLookupHandler",
                {
                  LookupHandlerMap::iterator it = m_lookupMap.find(stateName);
                  if (it != m_lookupMap.end()) {
                    debugMsg("AdapterConfiguration:registerLookupHandler",
                             " replacing former lookup handler for '" << stateName << "'");
                  }
                  else {
                    debugMsg("AdapterConfiguration:registerLookupHandler",
                             " registering handler for command '" << stateName << "'");
                  }
                });
      m_lookupMap[stateName] = handler;
      m_lookupHandlers.insert(handler);
    }

    virtual void registerLookupHandler(std::string const &stateName,
                                       LookupNowHandler lookupNow,
                                       SubscribeHandler subscribe = NULL,
                                       UnsubscribeHandler unsubscribe = NULL,
                                       SetThresholdsHandlerReal setThresholdsReal = NULL,
                                       SetThresholdsHandlerInteger setThresholdsInt = NULL)
    {
      registerLookupHandler(stateName,
                            new LookupHandlerWrapper(lookupNow,
                                                     subscribe,
                                                     unsubscribe,
                                                     setThresholdsReal,
                                                     setThresholdsInt));
    }

    virtual void registerCommonLookupHandler(LookupHandler *handler,
                                             pugi::xml_node const configXml)
    {
      assertTrue_2(handler, "registerCommonLookupHandler: Handler must not be NULL");
      m_lookupHandlers.insert(handler); // ensure it gets cleaned up later

      pugi::xml_node lookupNamesElt =
        configXml.child(InterfaceSchema::LOOKUP_NAMES_TAG());
      size_t nLookupNames = 0;
      while (lookupNamesElt) {
        // Register individual lookups
        char const *lookupNamesStr = lookupNamesElt.child_value();
        checkError(lookupNamesStr && *lookupNamesStr,
                   "IpcAdapter: Invalid configuration XML: "
                   << InterfaceSchema::LOOKUP_NAMES_TAG()
                   << " requires one or more comma-separated lookup names");
        // Only register the names if they are not telemetry-only
        if (!lookupNamesElt.attribute(InterfaceSchema::TELEMETRY_ONLY_ATTR()).as_bool()) {
          std::vector<std::string> *lkupNames =
            InterfaceSchema::parseCommaSeparatedArgs(lookupNamesStr);
          nLookupNames += lkupNames->size();
          for (std::vector<std::string>::const_iterator it = lkupNames->begin();
               it != lkupNames->end();
               ++it)
            m_lookupMap[*it] = handler;
          delete lkupNames;
        }
        lookupNamesElt = lookupNamesElt.next_sibling(InterfaceSchema::LOOKUP_NAMES_TAG());
      }
    }

    virtual void registerCommonLookupHandler(LookupHandler *handler,
                                             std::vector<std::string> const &names)
    {
      assertTrue_2(handler, "registerCommonLookupHandler: Handler must not be NULL");
      m_lookupHandlers.insert(handler); // ensure it gets cleaned up later

      if (names.empty())
        return;

      for (std::vector<std::string>::const_iterator nameit = names.begin();
           nameit != names.end();
           ++nameit)
        m_lookupMap[*nameit] = handler;
    }

    virtual LookupHandler *getLookupHandler(std::string const &stateName) const
    {
      LookupHandlerMap::const_iterator it = m_lookupMap.find(stateName);
      if (it != m_lookupMap.end()) {
        debugMsg("AdapterConfiguration:getLookupHandler",
                 " found handler " << (*it).second
                 << " for lookup '" << stateName << "'");
        return (*it).second;
      }
      debugMsg("AdapterConfiguration:getLookupHandler",
                 " using default handler for lookup '" << stateName << "'");
      return m_defaultLookupHandler;
    }

    virtual ExecListenerHub *getListenerHub() const
    {
      return m_listenerHub;
    }

    virtual void registerCommandHandler(std::string const &stateName,
                                        CommandHandler *handler)
    {
      assertTrue_2(handler, "registerCommandHandler: CommandHandler pointer must not be NULL");
      debugStmt("AdapterConfiguration:registerCommandHandler",
                {
                  CommandHandlerMap::iterator it = m_commandMap.find(stateName);
                  if (it != m_commandMap.end()) {
                    debugMsg("AdapterConfiguration:registerCommandHandler",
                             " replacing former command handler for '" << stateName
                             << "' with " << handler);
                  }
                  else {
                    debugMsg("AdapterConfiguration:registerCommandHandler",
                             " registering handler " << handler
                             << " for command '" << stateName << "'");
                  }
                });
      m_commandMap[stateName] = handler;
      m_commandHandlers.insert(handler);
    }

    virtual void registerCommandHandler(std::string const &stateName,
                                        ExecuteCommandHandler execCmd,
                                        AbortCommandHandler abortCmd = defaultAbortCommandHandler)
    {
      assertTrue_2(execCmd, "registerCommandHandler: Command handler function must not be NULL");
      assertTrue_2(abortCmd, "registerCommandHandler: Abort handler function must not be NULL");

      registerCommandHandler(stateName,
                             new CommandHandlerWrapper(execCmd, abortCmd));
    }

    virtual void registerCommonCommandHandler(CommandHandler *handler,
                                              pugi::xml_node const configXml)
    {
      assertTrue_2(handler, "registerCommonCommandHandler: Handler must not be NULL");
      m_commandHandlers.insert(handler); // ensure it gets cleaned up later

      pugi::xml_node commandNamesElt =
        configXml.child(InterfaceSchema::COMMAND_NAMES_TAG());
      size_t nCommandNames = 0;
      while (commandNamesElt) {
        // Register individual commands
        char const *commandNamesStr = commandNamesElt.child_value();
        checkError(commandNamesStr && *commandNamesStr,
                   "IpcAdapter: Invalid configuration XML: "
                   << InterfaceSchema::COMMAND_NAMES_TAG()
                   << " requires one or more comma-separated command names");
        std::vector<std::string> *cmdNames =
          InterfaceSchema::parseCommaSeparatedArgs(commandNamesStr);
        nCommandNames += cmdNames->size();
        for (std::vector<std::string>::const_iterator it = cmdNames->begin();
             it != cmdNames->end();
             ++it)
          m_commandMap[*it] = handler;
        delete cmdNames;
        commandNamesElt = commandNamesElt.next_sibling(InterfaceSchema::COMMAND_NAMES_TAG());
      }
    }

    virtual void registerCommonCommandHandler(CommandHandler *handler,
                                              std::vector<std::string> const &names)
    {
      assertTrue_2(handler, "registerCommonCommandHandler: Handler must not be NULL");
      m_commandHandlers.insert(handler); // ensure it gets cleaned up later

      if (names.empty())
        return;

      for (std::vector<std::string>::const_iterator it = names.begin();
             it != names.end();
             ++it)
          m_commandMap[*it] = handler;
    }

    virtual CommandHandler *getCommandHandler(std::string const& cmdName) const
    {
      CommandHandlerMap::const_iterator it = m_commandMap.find(cmdName);
      if (it != m_commandMap.end()) {
        debugMsg("AdapterConfiguration:getCommandHandler",
                 " found handler " << (*it).second
                 << " for command '" << cmdName << "'");
        return (*it).second;
      }
      debugMsg("AdapterConfiguration:getCommandHandler",
               " using default handler for command '" << cmdName << "'");
      return m_defaultCommandHandler;
    }

    virtual void setDefaultLookupHandler(LookupHandler *handler)
    {
      assertTrue_2(handler, "setDefaultLookupHandler: Handler must not be NULL");
      debugMsg("AdapterConfiguration:setDefaultLookupHandler",
               " replacing default lookup handler");
      m_defaultLookupHandler = handler;
      m_lookupHandlers.insert(handler);
    }

    virtual void setDefaultLookupHandler(LookupNowHandler lookupNow,
                                         SubscribeHandler subscribe,
                                         UnsubscribeHandler unsubscribe,
                                         SetThresholdsHandlerReal setThresholdsReal,
                                         SetThresholdsHandlerInteger setThresholdsInt)
    {
      setDefaultLookupHandler(new LookupHandlerWrapper(lookupNow,
                                                       subscribe,
                                                       unsubscribe,
                                                       setThresholdsReal,
                                                       setThresholdsInt));
    }

    virtual void setDefaultCommandHandler(CommandHandler *handler)
    {
      assertTrue_2(handler, "Default CommandHandler must not be NULL");
      debugMsg("AdapterConfiguration:setDefaultCommandHandler",
               " replacing default command handler with " << handler);
      m_defaultCommandHandler = handler;
      m_commandHandlers.insert(handler);
    }

    virtual void setDefaultCommandHandler(ExecuteCommandHandler execCmd,
                                          AbortCommandHandler abortCmd)
    {
      setDefaultCommandHandler(new CommandHandlerWrapper(execCmd, abortCmd));
    }

    virtual void registerPlannerUpdateHandler(PlannerUpdateHandler *newHandler)
    {
      assertTrue_2(newHandler, "Default PlannerUpdateHandler must not be NULL");
      debugMsg("AdapterConfiguration:registerPlannerUpdateHandler",
               " replacing planner update handler");

      PlannerUpdateHandler *oldHandler = m_plannerUpdateHandler;
      m_plannerUpdateHandler = newHandler;
      delete oldHandler;
    }

    virtual void registerPlannerUpdateHandler(PlannerUpdateFn updateFn)
    {
      registerPlannerUpdateHandler(new PlannerUpdateHandlerWrapper(updateFn));
    }

    virtual PlannerUpdateHandler *getPlannerUpdateHandler() const
    {
      return m_plannerUpdateHandler;
    }

    /* --------------------------------------------------------------------- */
    /* -------------------- Deprecated Interface Methods ------------------- */
    /* --------------------------------------------------------------------- */

    virtual void defaultRegisterAdapter(InterfaceAdapter *adapter)
    {
      assertTrue_2(adapter, "defaultRegisterAdapter: Adapter must not be NULL");

      debugMsg("AdapterConfiguration:defaultRegisterAdapter", " for adapter " << adapter);

      // Walk the children of the configuration XML element
      // and register the adapter according to the data found there
      pugi::xml_node element = adapter->getXml().first_child();
      while (element) {
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
                     "defaultRegisterAdapter: Invalid configuration XML: "
                     << InterfaceSchema::COMMAND_NAMES_TAG()
                     << " requires one or more comma-separated command names");
          std::vector<std::string> * cmdNames = InterfaceSchema::parseCommaSeparatedArgs(text);
          for (std::vector<std::string>::const_iterator it = cmdNames->begin();
               it != cmdNames->end();
               ++it) {
            registerCommandInterface(*it, adapter);
          }
          delete cmdNames;
        } 
        else if (strcmp(elementType, InterfaceSchema::LOOKUP_NAMES_TAG()) == 0) {
          const pugi::xml_node firstChild = element.first_child();
          const char* text = NULL;
          if (!firstChild.empty() && firstChild.type() == pugi::node_pcdata)
            text = firstChild.value();
          checkError(text && *text != '\0',
                     "defaultRegisterAdapter: Invalid configuration XML: "
                     << InterfaceSchema::LOOKUP_NAMES_TAG()
                     << " requires one or more comma-separated lookup names");
          std::vector<std::string> * lookupNames = InterfaceSchema::parseCommaSeparatedArgs(text);
          bool telemOnly = element.attribute(InterfaceSchema::TELEMETRY_ONLY_ATTR()).as_bool();
          for (std::vector<std::string>::const_iterator it = lookupNames->begin();
               it != lookupNames->end();
               ++it) {
            registerLookupInterface(*it, adapter, telemOnly);
          }
          delete lookupNames;
        }

        // ignore other tags, they're for adapter's use
        element = element.next_sibling();
      }
    }

    virtual bool registerCommandInterface(std::string const &commandName,
                                          InterfaceAdapter *intf)
    {
      assertTrue_2(intf, "registerCommandInterface: Adapter must not be NULL");
      registerCommandHandler(commandName, new AdapterCommandHandler(intf));
      return true;
    }

    virtual bool registerLookupInterface(std::string const &stateName,
                                         InterfaceAdapter *intf,
                                         bool telemetryOnly)
    {
      assertTrue_2(intf, "registerLookupInterface: Adapter must not be NULL");
      if (telemetryOnly) {
        registerTelemetryLookup(stateName);
      }
      else {
        registerLookupHandler(stateName, new AdapterLookupHandler(intf));
      }
      return true;
    }

    virtual bool setDefaultInterface(InterfaceAdapter *intf)
    {
      assertTrue_2(intf, "setDefaultInterface: Adapter must not be NULL");
      setDefaultCommandInterface(intf);
      setDefaultLookupInterface(intf);
      registerPlannerUpdateInterface(intf);
      return true;
    }

    virtual bool setDefaultLookupInterface(InterfaceAdapter *intf)
    {
      assertTrue_2(intf, "setDefaultLookupInterface: Adapter must not be NULL")
      setDefaultLookupHandler(new AdapterLookupHandler(intf));
      m_adapters.insert(intf);
      return true;
    }

    virtual bool setDefaultCommandInterface(InterfaceAdapter *intf)
    {
      assertTrue_2(intf, "setDefaultCommandInterface: Adapter must not be NULL")
      setDefaultCommandHandler(new AdapterCommandHandler(intf));
      m_adapters.insert(intf);
      return true;
    }

    virtual bool registerPlannerUpdateInterface(InterfaceAdapter *intf)
    {
      assertTrue_2(intf, "registerPlannerHandlerInterface: Adapter must not be NULL")
      registerPlannerUpdateHandler(new AdapterPlannerUpdateHandler(intf));
      m_adapters.insert(intf);
      return true;
    }

  };

  AdapterConfiguration *makeAdapterConfiguration()
  {
    return new AdapterConfigurationImpl;
  }

}
