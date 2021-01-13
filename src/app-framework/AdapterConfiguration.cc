/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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
#include "Launcher.h"
#include "ListenerFilters.hh"
#include "planLibrary.hh"
#include "State.hh"
#include "TimeAdapter.hh"
#include "UtilityAdapter.h"

#ifdef PLEXIL_WITH_THREADS
#include "SerializedInputQueue.hh"
#else
#include "SimpleInputQueue.hh"
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
#endif // not defined(PIC)

#include <map>

#if defined(HAVE_CSTRING)
#include <cstring>
#elif defined(HAVE_STRING_H)
#include <string.h>
#endif

namespace PLEXIL {

  //*
  // @class AdapterConfigurationImpl
  // @brief Implementation class for AdapterConfiguration
  //
  class AdapterConfigurationImpl : public AdapterConfiguration
  {
  private:

    // Internal typedefs
    using ExecListenerHubPtr  = std::unique_ptr<ExecListenerHub>;
    using InterfaceAdapterPtr = std::unique_ptr<InterfaceAdapter>;

    using CommandHandlerPtr = std::shared_ptr<CommandHandler>;
    using CommandHandlerMap = std::map<std::string, CommandHandlerPtr>;

    using LookupHandlerPtr = std::shared_ptr<LookupHandler>;
    using LookupHandlerMap = std::map<std::string, LookupHandlerPtr>;

    // punt for now
    using InterfaceAdapterSet = std::vector<InterfaceAdapterPtr>;

  public:

    AdapterConfigurationImpl()
      : m_listenerHub(new ExecListenerHub()),
        m_defaultCommandHandler(new CommandHandler()),
        m_defaultLookupHandler(new LookupHandler()),
        m_plannerUpdateHandler()
    {
      // Every application has access to a time adapter
      registerTimeAdapter();

      // Every application has access to the utility and launcher adapters
      initUtilityAdapter();
      initLauncher();

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
      dynamicLoadModule("PlanDebugListener", nullptr);
#else
      initPlanDebugListener();
#endif
#endif

#ifdef HAVE_GANTT_LISTENER
      // Every application should have access to the GANTT Listener
#ifdef PIC
      dynamicLoadModule("GanttListener", nullptr);
#else
      initGanttListener();
#endif
#endif

#ifdef HAVE_IPC_ADAPTER
      // Every application should have access to the IPC Adapter
#ifdef PIC
      dynamicLoadModule("IpcAdapter", nullptr);
#else
      initIpcAdapter();
#endif
#endif

#ifdef HAVE_LUV_LISTENER
      // Every application should have access to the Plexil Viewer (formerly LUV) Listener
#ifdef PIC
      dynamicLoadModule("LuvListener", nullptr);
#else
      initLuvListener();
#endif
#endif

#ifdef HAVE_UDP_ADAPTER
      // Every application should have access to the UDP Adapter
#ifdef PIC
      dynamicLoadModule("UdpAdapter", nullptr);
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
      // Clear the handlers first, as they might point back to the adapters
      m_lookupMap.clear();
      m_commandMap.clear();
      m_defaultCommandHandler.reset();
      m_defaultLookupHandler.reset();

      // *** what to do about planner update handler? ***

      // ExecListenerHub next, as listeners may point back to adapters too
      // (e.g. Launcher)
      m_listenerHub.reset();

      // Now the adapters
      m_adapters.clear();
    }

    // FIXME:
    // * Need new constructor paradigm for handlers
    virtual bool constructInterfaces(pugi::xml_node const configXml,
                                     AdapterExecInterface &intf)
    {
      if (configXml.empty()) {
        debugMsg("AdapterConfiguration:constructInterfaces",
                 " empty configuration, nothing to construct");
        return true;
      }

      debugMsg("AdapterConfiguration:verboseConstructInterfaces",
               " parsing configuration XML");
      const char* elementType = configXml.name();
      if (strcmp(elementType, InterfaceSchema::INTERFACES_TAG) != 0) {
        debugMsg("AdapterConfiguration:constructInterfaces",
                 " invalid configuration XML: no "
                 << InterfaceSchema::INTERFACES_TAG << " element");
        return false;
      }

      // Walk the children of the configuration XML element
      // and register the adapter according to the data found there
      pugi::xml_node element = configXml.first_child();
      while (!element.empty()) {
        debugMsg("AdapterConfiguration:verboseConstructInterfaces",
                 " found element " << element.name());
        const char* elementType = element.name();
        if (strcmp(elementType, InterfaceSchema::ADAPTER_TAG) == 0) {
          if (!constructAdapter(element, intf)) {
            warn("constructInterfaces: failed to construct adapter type \""
                 << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR).value()
                 << "\"");
            return false;
          }
        }
        else if (strcmp(elementType, InterfaceSchema::COMMAND_HANDLER_TAG) == 0) {
          if (!constructCommandHandler(element)) {
            warn("constructInterfaces: failed to construct command handler type \""
                 << element.attribute(InterfaceSchema::HANDLER_TYPE_ATTR).value()
                 << "\"");
            return false;
          }
        }
        else if (strcmp(elementType, InterfaceSchema::LOOKUP_HANDLER_TAG) == 0) {
          if (!constructLookupHandler(element)) {
            warn("constructInterfaces: failed to construct command handler type \""
                 << element.attribute(InterfaceSchema::HANDLER_TYPE_ATTR).value()
                 << "\"");
            return false;
          }
        }
        else if (strcmp(elementType, InterfaceSchema::PLANNER_UPDATE_HANDLER_TAG) == 0) {
          if (!constructPlannerUpdateHandler(element)) {
            warn("constructInterfaces: failed to construct command handler type \""
                 << element.attribute(InterfaceSchema::HANDLER_TYPE_ATTR).value()
                 << "\"");
            return false;
          }
        }
        else if (strcmp(elementType, InterfaceSchema::INTERFACE_LIBRARY_TAG) == 0) {
          if (!ensureInterfaceLibraryLoaded(element)) {
            warn("constructInterfaces: unable to locate library \""
                 << element.attribute(InterfaceSchema::NAME_ATTR).value()
                 << "\"");
            return false;
          }
        }
        else if (strcmp(elementType, InterfaceSchema::LISTENER_TAG) == 0) {
          // Construct an ExecListener instance and attach it to the Exec
          debugMsg("AdapterConfiguration:constructInterfaces",
                   " constructing listener type \""
                   << element.attribute(InterfaceSchema::LISTENER_TYPE_ATTR).value()
                   << '"');
          ExecListener *listener = 
            ExecListenerFactory::createInstance(element);
          if (!listener) {
            warn("constructInterfaces: failed to construct listener type \""
                 << element.attribute(InterfaceSchema::LISTENER_TYPE_ATTR).value()
                 << '"');
            return false;
          }
          m_listenerHub->addListener(listener);
        }
        else if (strcmp(elementType, InterfaceSchema::LIBRARY_NODE_PATH_TAG) == 0) {
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
        else if (strcmp(elementType, InterfaceSchema::PLAN_PATH_TAG) == 0) {
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

    virtual bool initialize()
    {
      debugMsg("AdapterConfiguration:initialize", " initializing interface adapters");
      bool success = true;
      for (InterfaceAdapterPtr &a : m_adapters) {
        success = a->initialize(this);
        if (!success) {
          warn("initialize: failed for adapter type \""
               << a->getXml().attribute(InterfaceSchema::ADAPTER_TYPE_ATTR).value()
               << '"');
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
      for (InterfaceAdapterPtr &a : m_adapters) {
        success = a->start();
        if (!success) {
          warn("start: start failed for adapter type \""
               << a->getXml().attribute(InterfaceSchema::ADAPTER_TYPE_ATTR).value()
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
    virtual void stop()
    {
      debugMsg("AdapterConfiguration:stop", " entered");

      // halt adapters
      for (InterfaceAdapterPtr &a : m_adapters)
        a->stop();

      m_listenerHub->stop();

      debugMsg("AdapterConfiguration:stop", " completed");
    }

    //
    // Command handler registration
    //

    virtual void registerCommandHandler(CommandHandler *handler,
                                        pugi::xml_node const configXml)
    {
      assertTrue_2(handler, "registerCommandHandler: Handler must not be null");
      pugi::xml_node commandNamesElt =
        configXml.child(InterfaceSchema::COMMAND_NAMES_TAG);
      while (commandNamesElt) {
        // Register individual commands
        char const *commandNamesStr = commandNamesElt.child_value();
        checkError(commandNamesStr && *commandNamesStr,
                   "IpcAdapter: Invalid configuration XML: "
                   << InterfaceSchema::COMMAND_NAMES_TAG
                   << " requires one or more comma-separated command names");
        std::vector<std::string> *cmdNames =
          InterfaceSchema::parseCommaSeparatedArgs(commandNamesStr);
        if (cmdNames) {
          for (std::string &name : *cmdNames)
            m_commandMap[name].reset(handler);
          delete cmdNames;
        }
        commandNamesElt = commandNamesElt.next_sibling(InterfaceSchema::COMMAND_NAMES_TAG);
      }
    }

    virtual void registerCommandHandler(CommandHandler *handler,
                                        std::vector<std::string> const &names)
    {
      assertTrue_2(handler, "registerCommandHandler: Handler must not be null");
      for (std::string const &name : names)
        m_commandMap[name].reset(handler);
    }

    virtual void registerCommandHandler(CommandHandler *handler,
                                        std::string const &cmdName)
    {
      assertTrue_2(handler, "registerCommandHandler: Handler must not be null");
      m_commandMap[cmdName].reset(handler);
    }

    virtual void registerCommandHandlerFunction(std::string const &stateName,
                                                ExecuteCommandHandler execCmd,
                                                AbortCommandHandler abortCmd =
                                                defaultAbortCommandHandler)
    {
      assertTrue_2(execCmd, "registerCommandHandlerFunction: Handler function must not be null");
      registerCommandHandler(new CommandHandlerWrapper(execCmd, abortCmd),
                             stateName);
    }

    virtual void setDefaultCommandHandler(CommandHandler *handler)
    {
      assertTrue_2(handler, "setDefaultCommandHandler: Handler must not be null");
      debugMsg("AdapterConfiguration:setDefaultCommandHanlder",
               " replacing default command handler");
      m_defaultCommandHandler.reset(handler);
    }

    virtual void setDefaultCommandHandlerFunction(ExecuteCommandHandler execCmd,
                                                  AbortCommandHandler abortCmd =
                                                  defaultAbortCommandHandler)
    {
      assertTrue_2(execCmd, "setDefaultCommandHandlerFunction: Handler function must not be null");
      setDefaultCommandHandler(new CommandHandlerWrapper(execCmd, abortCmd));
    }

    //
    // Lookup handler registration
    //

    virtual void registerLookupHandler(LookupHandler *handler,
                                       pugi::xml_node const configXml)
    {
      assertTrue_2(handler, "registerLookupHandler: Handler must not be null");

      pugi::xml_node lookupNamesElt =
        configXml.child(InterfaceSchema::LOOKUP_NAMES_TAG);
      while (lookupNamesElt) {
        // Register individual lookups
        char const *lookupNamesStr = lookupNamesElt.child_value();
        checkError(lookupNamesStr && *lookupNamesStr,
                   "IpcAdapter: Invalid configuration XML: "
                   << InterfaceSchema::LOOKUP_NAMES_TAG
                   << " requires one or more comma-separated lookup names");
        std::vector<std::string> *lkupNames =
          InterfaceSchema::parseCommaSeparatedArgs(lookupNamesStr);
        for (std::string const &name : *lkupNames)
          m_lookupMap[name].reset(handler);
        delete lkupNames;
        lookupNamesElt = lookupNamesElt.next_sibling(InterfaceSchema::LOOKUP_NAMES_TAG);
      }
    }

    virtual void registerLookupHandler(LookupHandler *handler,
                                       std::vector<std::string> const &names)
    {
      assertTrue_2(handler, "registerLookupHandler: Handler must not be null");
      for (std::string const &name : names)
        m_lookupMap[name].reset(handler);
    }

    virtual void registerLookupHandler(LookupHandler *handler,
                                       std::string const &stateName)
    {
      assertTrue_2(handler, "registerLookupHandler: Handler must not be null");
      m_lookupMap[stateName].reset(handler);
    }

    virtual void registerLookupHandlerFunction(std::string const &stateName,
                                               LookupNowHandler lookupNow,
                                               SetThresholdsHandlerReal setThresholdsReal =
                                               SetThresholdsHandlerReal(),
                                               SetThresholdsHandlerInteger setThresholdsInt =
                                               SetThresholdsHandlerInteger(),
                                               ClearThresholdsHandler clrThresholds =
                                               ClearThresholdsHandler())
    {
      assertTrue_2(lookupNow, "registerLookupHandlerFunction: LookupNow function must not be null");
      registerLookupHandler(new LookupHandlerWrapper(lookupNow,
                                                     setThresholdsReal,
                                                     setThresholdsInt,
                                                     clrThresholds),
                            stateName);
    }

    virtual void setDefaultLookupHandler(LookupHandler *handler)
    {
      assertTrue_2(handler, "setDefaultLookupHandler: Handler must not be null");
      m_defaultLookupHandler.reset(handler);
    }

    virtual void setDefaultLookupHandler(LookupNowHandler lookupNow,
                                         SetThresholdsHandlerReal setThresholdsReal =
                                         SetThresholdsHandlerReal(),
                                         SetThresholdsHandlerInteger setThresholdsInt =
                                         SetThresholdsHandlerInteger(),
                                         ClearThresholdsHandler clrThresholds =
                                         ClearThresholdsHandler())
    {
      assertTrue_2(lookupNow,
                   "setDefaultLookupHandler: LookupNow function must not be null");
      setDefaultLookupHandler(new LookupHandlerWrapper(lookupNow,
                                                       setThresholdsReal,
                                                       setThresholdsInt,
                                                       clrThresholds));
    }

    //
    // Planner Update handler registration
    //

    virtual void registerPlannerUpdateHandler(PlannerUpdateHandler handler)
    {
      assertTrue_2(handler, "registerPlannerUpdateHandler: Handler must not be null");
      m_plannerUpdateHandler = handler;
    }

    //
    // Interface adapter registration
    //

    virtual void addInterfaceAdapter(InterfaceAdapter *adapter)
    {
      m_adapters.emplace_back(InterfaceAdapterPtr(adapter));
    }

    //
    // Exec listener registration
    //

    virtual void addExecListener(ExecListener *listener)
    {
      m_listenerHub->addListener(listener);
    }

    //
    // Handler accessors
    //

    virtual CommandHandler *getCommandHandler(std::string const &cmdName) const
    {
      CommandHandlerMap::const_iterator it = m_commandMap.find(cmdName);
      if (it != m_commandMap.end()) {
        debugMsg("AdapterConfiguration:getCommandHandler",
                 " found registered handler for command '" << cmdName << "'");
        return (*it).second.get();
      }
      debugMsg("AdapterConfiguration:getCommandHandler",
               " using default handler for command '" << cmdName << "'");
      return m_defaultCommandHandler.get();
    }

    virtual LookupHandler *getLookupHandler(std::string const &stateName) const
    {
      LookupHandlerMap::const_iterator it = m_lookupMap.find(stateName);
      if (it != m_lookupMap.end()) {
        debugMsg("AdapterConfiguration:getLookupHandler",
                 " found registered handler for lookup '" << stateName << "'");
        return (*it).second.get();
      }
      debugMsg("AdapterConfiguration:getLookupHandler",
                 " using default handler for lookup '" << stateName << "'");
      return m_defaultLookupHandler.get();
    }

    virtual PlannerUpdateHandler getPlannerUpdateHandler() const
    {
      return m_plannerUpdateHandler;
    }

    virtual ExecListenerHub *getListenerHub() const
    {
      return m_listenerHub.get();
    }

    //
    // Search path registration for plans and libraries
    //

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

    //
    // Search path access for plans and libraries
    //

    virtual const std::vector<std::string>& getPlanPath() const
    {
      return m_planPath;
    }

    virtual const std::vector<std::string>& getLibraryPath() const
    {
      return getLibraryPaths();
    }

    //
    // Input queue
    //

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

  private:

    //
    // Private helpers
    //

    //! Construct the adapter described by the given XML.
    //! @param element The XML element specifying the adapter to be constructed.
    //! @param intf The AdapterExecInterface the new adapter will report to.
    //! @return True if an adapter was constructed, false otherwise.
    bool constructAdapter(pugi::xml_node const element,
                          AdapterExecInterface &intf)
    {
      InterfaceAdapter *adapter = 
        AdapterFactory::createInstance(element, intf);
      if (!adapter) {
        warn("constructInterfaces: failed to construct adapter type \""
             << element.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR).value()
             << "\"");
        return false;
      }
      m_adapters.emplace_back(InterfaceAdapterPtr(adapter));
      return true;
    }

    bool constructCommandHandler(pugi::xml_node const element)
    {
      // TODO -- see InterfaceFactory.hh
      return true;
    }

    bool constructLookupHandler(pugi::xml_node const element)
    {
      // TODO -- see InterfaceFactory.hh
      return true;
    }

    bool constructPlannerUpdateHandler(pugi::xml_node const element)
    {
      // TODO -- see InterfaceFactory.hh
      return true;
    }

    bool ensureInterfaceLibraryLoaded(pugi::xml_node const element)
    {
      const char* libName = element.attribute(InterfaceSchema::NAME_ATTR).value();
      if (!libName || !*libName) {
        warn("AdapterConfiguration: missing or empty "
             << InterfaceSchema::NAME_ATTR << " attribute in "
             << element.name());
        return false;
      }

      debugMsg("AdapterConfiguration:constructInterfaces", 
               " Loading library \"" << libName << "\"");
      // Attempt to dynamically load library
      const char* libPath =
        element.attribute(InterfaceSchema::LIB_PATH_ATTR).value();
      if (dynamicLoadModule(libName, libPath)) {
        debugMsg("AdapterConfiguration:constructInterfaces", 
                 " Successfully loaded library \"" << libName << "\"");
        return true;
      }
      warn("constructInterfaces: unable to load library \"" << libName << "\"");
      return false;
    }

    //
    // Internal classes typedefs
    //
 
    //*
    // @brief A wrapper class for user-provided command handler
    // functions.
    //
    struct CommandHandlerWrapper : public CommandHandler
    {
      //
      // Constructors
      //

      CommandHandlerWrapper(ExecuteCommandHandler exec,
                            AbortCommandHandler abrt)
        : m_executeCommandFn(exec),
          m_abortCommandFn(abrt)
      {
      }
    
      CommandHandlerWrapper(const CommandHandlerWrapper &) = default;
      CommandHandlerWrapper(CommandHandlerWrapper &&) = default;

      // Destructor
      virtual ~CommandHandlerWrapper() = default;

      // Assignment (shouldn't be used but won't hurt anything)
      CommandHandlerWrapper &operator=(const CommandHandlerWrapper &) = default;
      CommandHandlerWrapper &operator=(CommandHandlerWrapper &&) = default;

      virtual void executeCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        (m_executeCommandFn)(cmd, intf);
      }

      // Call the base class method if no abort handler function provided
      virtual void abortCommand(Command *cmd, AdapterExecInterface *intf) override
      {
        if (m_abortCommandFn)
          (m_abortCommandFn)(cmd, intf);
        else
          CommandHandler::abortCommand(cmd, intf);
      }

    private:
      //
      // Not implemented
      //
      CommandHandlerWrapper() = delete;

      ExecuteCommandHandler m_executeCommandFn;
      AbortCommandHandler m_abortCommandFn;
    };

    //*
    // @brief A wrapper class for user-provided lookup handler functions.
    //
    struct LookupHandlerWrapper final : public LookupHandler
    {
      //
      // Constructors
      //
      LookupHandlerWrapper(LookupNowHandler lkup,
                           SetThresholdsHandlerReal setThReal,
                           SetThresholdsHandlerInteger setThInt,
                           ClearThresholdsHandler clrTh)
        : lookupNowFn(lkup),
          setThresholdsRealFn(setThReal),
          setThresholdsIntFn(setThInt),
          clearThresholdsFn(clrTh)
      {
      }

      LookupHandlerWrapper(const LookupHandlerWrapper &) = default;
      LookupHandlerWrapper(LookupHandlerWrapper &&) = default;

      // Destructor
      virtual ~LookupHandlerWrapper() = default;

      // Assignment (shouldn't be used but doesn't hurt anything)
      LookupHandlerWrapper &operator=(const LookupHandlerWrapper &) = default;
      LookupHandlerWrapper &operator=(LookupHandlerWrapper &&) = default;

      virtual void lookupNow(const State &state, LookupReceiver *rcvr) override
      {
        if (lookupNowFn)
          (lookupNowFn)(state, rcvr);
      }

      virtual void setThresholds(const State &state, Real hi, Real lo) override
      {
        if (setThresholdsRealFn)
          (setThresholdsRealFn)(state, hi, lo);
      }

      virtual void setThresholds(const State &state, Integer hi, Integer lo) override
      {
        if (setThresholdsIntFn)
          (setThresholdsIntFn)(state, hi, lo);
      }

      virtual void clearThresholds(const State &state) override
      {
        if (clearThresholdsFn)
          (clearThresholdsFn)(state);
      }

      LookupNowHandler lookupNowFn;
      SetThresholdsHandlerReal setThresholdsRealFn;
      SetThresholdsHandlerInteger setThresholdsIntFn;
      ClearThresholdsHandler clearThresholdsFn;

    private:
      LookupHandlerWrapper() = delete;    
    };

    //
    // Member variables
    //

    // Maps by command/lookup
    LookupHandlerMap m_lookupMap;
    CommandHandlerMap m_commandMap;
    
    //* Set of all known InterfaceAdapter instances
    InterfaceAdapterSet m_adapters;

    //* List of directory names for plan file search paths
    std::vector<std::string> m_planPath;

    //* ExecListener hub
    ExecListenerHubPtr m_listenerHub;

    //* Default handlers
    CommandHandlerPtr m_defaultCommandHandler;
    LookupHandlerPtr m_defaultLookupHandler;

    //* Handler to use for Update nodes
    PlannerUpdateHandler m_plannerUpdateHandler;
  };

  AdapterConfiguration *makeAdapterConfiguration()
  {
    return new AdapterConfigurationImpl;
  }

}
