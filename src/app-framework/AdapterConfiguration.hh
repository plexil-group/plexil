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

#include "commandHandlerDefs.hh"
#include "lookupHandlerDefs.hh"
#include "plannerUpdateHandlerDefs.hh"

#include <string>
#include <vector>

// Forward references
namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{

  // Forward references
  class ExecListener;
  class ExecListenerHub;
  class InterfaceAdapter;
  class InputQueue;

  //*
  // @class AdapterConfiguration
  // @brief Abstract base class for registering external interfaces
  class AdapterConfiguration
  {
  public:

    /**
     * @brief Virtual destructor.
     */
    virtual ~AdapterConfiguration()
    {
    }

    /**
     * @brief Constructs interface adapters from the provided XML.
     * @param configXml The XML element used for interface configuration.
     * @return true if successful, false otherwise.
     */
    virtual bool constructInterfaces(pugi::xml_node const configXml) = 0;

    /**
     * @brief Performs basic initialization of the interface and all adapters.
     * @return true if successful, false otherwise.
     */
    virtual bool initialize() = 0;

    /**
     * @brief Prepares the interface and adapters for execution.
     * @return true if successful, false otherwise.
     */
    virtual bool start() = 0;

    /**
     * @brief Halts all interfaces.
     * @return true if successful, false otherwise.
     */
    virtual bool stop() = 0;

    /**
     * @brief Resets the interface prior to restarting.
     * @return true if successful, false otherwise.
     */
    virtual bool reset() = 0;

    /**
     * @brief Shuts down the interface.
     * @return true if successful, false otherwise.
     */
    virtual bool shutdown() = 0;

    /**
     * @deprecated
     * @brief Add an externally constructed interface adapter.
     * @param adapter Pointer to the InterfaceAdapter instance.
     *
     * @note The AdapterConfiguration is responsible for deleting the
     * adapter.
     */
    virtual void addInterfaceAdapter(InterfaceAdapter *adapter) = 0;

    /**
     * @brief Add an externally constructed ExecListener.
     * @param listener Pointer to the ExecListener instance to add.
     */
    virtual void addExecListener(ExecListener *listener) = 0;

    /**
     * @brief Register this state name as a telemetry-only lookup.
     * @param stateName The name of the state
     *
     * @note "Telemetry-only" means that there is no lookupNow method
     * for this state.  Use AdapterExecInterface::handleValueChange()
     * to publish updates for the state.
     * 
     * @see AdapterExecInterface::handleValueChange
     */
    virtual void registerTelemetryLookup(std::string const &stateName) = 0;

    /**
     * @brief Register the given LookupHandler instance for lookups
     *        for this state name, overriding any previous handler or
     *        interface adapter registrations for the name.
     * @param stateName The name of the state to map to this handler.
     * @param handler Pointer to the LookupHandler.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates.
     */
    virtual void registerLookupHandler(std::string const &stateName,
                                       LookupHandler *handler) = 0;

    /**
     * @brief Register the given handler function(s) for lookups to
     *        this state, overriding any previous handler or interface
     *        adapter registrations for this name.
     * @param stateName The name of the state to map to these handler functions.
     * @param lookupNow The lookupNow function for this state.
     * @param setThresholdsReal (Optional) The setThresholdsReal
     *        handler function for this state.
     * @param setThresholdsInteger (Optional) The setThresholdsInteger
     *        handler function for this state.
     * @param subscribe (Optional) The subscribe handler function for
     *        this state.
     * @param unsubscribe (Optional) The unsubscribe handler function
     *        for this state.
     */
    virtual void registerLookupHandler(std::string const &stateName,
                                       LookupNowHandler ln,
                                       SubscribeHandler sub = NULL,
                                       UnsubscribeHandler unsub = NULL,
                                       SetThresholdsHandlerReal setTD = NULL,
                                       SetThresholdsHandlerInteger setTI = NULL) = 0;

    /**
     * @brief Register the given LookupHandler instance for all LookupNames
     *        in the adapter's configuration XML.
     * @param handler Pointer to the LookupHandler instance.
     * @param configXml Configuration XML for the interface.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates. If there is no LookupNames
     *       element in the configuration XML, the handler is deleted
     *       immediately.
     * @note To override the common handler for specific lookup names,
     *       call registerLookupHandler() after calling this member function.
     */
    virtual void registerCommonLookupHandler(LookupHandler *handler,
                                             pugi::xml_node const configXml) = 0;

    /**
     * @brief Register the given LookupHandler instance for all LookupNames
     *        in the adapter's configuration XML.
     * @param handler Pointer to the LookupHandler instance.
     * @param names Const reference to a vector of name strings.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates. If there is no LookupNames
     *       element in the configuration XML, the handler is deleted
     *       immediately.
     * @note To override the common handler for specific lookup names,
     *       call registerLookupHandler() after calling this member function.
     */
    virtual void registerCommonLookupHandler(LookupHandler *handler,
                                             std::vector<std::string> const &names) = 0;

    /**
     * @brief Register the given LookupHandler instance as the default
     * for lookups, overriding any previously registered default
     * handler or interface adapter, including the default default
     * handler.
     * @param handler Pointer to the new default LookupHandler.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates.
     */
    virtual void setDefaultLookupHandler(LookupHandler *handler) = 0;
    
    /**
     * @brief Register the given handler function(s) as the default
     * for lookups, overriding any previously registered default
     * handler or interface adapter, including the default default
     * handler.
     * @param lookupNow The lookup handler function to use as a
     * default.
     * @param setThresholdsReal (Optional) The setThresholdsReal
     * handler function.
     * @param setThresholdsInteger (Optional) The setThresholdsInteger
     * handler function.
     * @param subscribe (Optional) The subscribe handler function.
     * @param unsubscribe (Optional) The unsubscribe handler function.
     */
    virtual void setDefaultLookupHandler(LookupNowHandler lookupNow,
                                         SubscribeHandler subscribe = NULL,
                                         UnsubscribeHandler unsubscribe = NULL,
                                         SetThresholdsHandlerReal setThresholdsReal = NULL,
                                         SetThresholdsHandlerInteger setThresholdsInteger = NULL)
    = 0;

    /**
     * @brief Return the LookupHandler instance in effect for lookups
     * with this state name.
     * @param stateName The state.
     * @return Pointer to the LookupHandler for the named state.
     */
    virtual LookupHandler *getLookupHandler(std::string const& stateName) const = 0;

    /**
     * @brief Return the application's ExecListenerHub.
     * @return Pointer to the ExecListenerHub.
     */
    virtual ExecListenerHub *getListenerHub() const = 0;

    /**
     * @brief Register the given CommandHandler for this command,
     *        overriding any previous handler or interface adapter
     *        registration.
     * @param cmdName The name of the command to map to this handler.
     * @param handler Pointer to the CommandHandler instance to
     *        associate with this command name.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates.
     */
    virtual void registerCommandHandler(std::string const &cmdName,
                                        CommandHandler *handler) = 0;

    /**
     * @brief Register the given handler functions for the named
     * command, overriding any previous handler or interface adapter
     * registration.
     * @param cmdName The command name to map to this handler.
     * @param execCmd The function to call when this command is
     * executed.
     * @param abortCmd The function to call when this command is
     * aborted; defaults to defaultAbortCommandHandler.
     */
    virtual void registerCommandHandler(std::string const &cmdName,
                                        ExecuteCommandHandler execCmd,
                                        AbortCommandHandler abortCmd = defaultAbortCommandHandler)
    = 0;

    /**
     * @brief Register the given CommandHandler instance for all
     *        CommandNames in the adapter's configuration XML.
     * @param handler Pointer to the CommandHandler instance.
     * @param configXml Configuration XML for the interface.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates. If there is no CommandNames
     *       element in the configuration XML, the handler is deleted
     *       immediately.
     * @note To override the common handler for specific command names,
     *       call registerCommandHandler() after calling this member function.
     */
    virtual void registerCommonCommandHandler(CommandHandler *handler,
                                              pugi::xml_node const configXml) = 0;

    /**
     * @brief Register the given CommandHandler instance for all
     *        CommandNames in the adapter's configuration XML.
     * @param handler Pointer to the CommandHandler instance.
     * @param names Const reference to Vector of command name strings.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates. If there is no CommandNames
     *       element in the configuration XML, the handler is deleted
     *       immediately.
     * @note To override the common handler for specific command names,
     *       call registerCommandHandler() after calling this member function.
     */
    virtual void registerCommonCommandHandler(CommandHandler *handler,
                                              std::vector<std::string> const &names) = 0;

    /**
     * @brief Register the CommandHandler instance as the default for
     * commands without a specific handler, overriding any previously
     * registered default handler or interface adapter, including the
     * default default handler.
     * @param handler Pointer to the CommandHandler instance.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates.
     */
    virtual void setDefaultCommandHandler(CommandHandler *handler) = 0;
    
    /**
     * @brief Register the given handler function(s) as the default
     * for commands without a specific handler, overriding any
     * previously registered default handler or interface adapter,
     * including the default default handler.
     * @param execCmd The execute command handler function.
     * @param abortCmd (Optional) The abort command handler function.
     * Defaults to defaultAbortCommandHandler if not supplied.
     *
     * @see defaultAbortCommandHandler
     */
    virtual void setDefaultCommandHandler(ExecuteCommandHandler execCmd,
                                          AbortCommandHandler abortCmd
                                          = &defaultAbortCommandHandler)
    = 0;

    /**
     * @brief Return the CommandHandler instance for a command name.
     * @param cmdName The command name.
     * @return Pointer to the CommandHandler instance for the named
     * command.
     */
    virtual CommandHandler *getCommandHandler(std::string const& stateName) const = 0;
    
    /**
     *
     * @brief Register the given handler function for planner updates,
     *        overriding any previously registered handlers or interface
     *        adapters.
     * @param sendPlannerUpdate Pointer to the handler function for
     *        planner updates.
     */
    virtual void registerPlannerUpdateHandler(PlannerUpdateFn updateFn) = 0;

    /**
     * @brief Register the given handler instance for planner updates,
     *        overriding any previously registered handlers or interface
     *        adapters.
     * @param sendPlannerUpdate Pointer to the handler instance for
     *        planner updates.
     * @note The AdapterConfiguration instance takes ownership of the
     *       handler, and is responsible for its deletion when the
     *       application terminates.
     */
    virtual void registerPlannerUpdateHandler(PlannerUpdateHandler *updateHandler) = 0;

    /**
     * @brief Get the handler instance for planner updates.
     * @return Pointer to the handler instance.
     */
    virtual PlannerUpdateHandler *getPlannerUpdateHandler() const = 0;

    //
    // Plan, library path access
    //

    /**
     * @brief Get the search path for library nodes.
     * @return A reference to the library search path.
     */
    virtual const std::vector<std::string>& getLibraryPath() const = 0;

    /**
     * @brief Get the search path for plan files.
     * @return A reference to the plan search path.
     */
    virtual const std::vector<std::string>& getPlanPath() const = 0;

    /**
     * @brief Add the specified directory name to the end of the library node loading path.
     * @param libdir The directory name.
     */
    virtual void addLibraryPath(const std::string& libdir) = 0;

    /**
     * @brief Add the specified directory names to the end of the library node loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addLibraryPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Add the specified directory name to the end of the plan loading path.
     * @param libdir The directory name.
     */
    virtual void addPlanPath(const std::string& libdir) = 0;

    /**
     * @brief Add the specified directory names to the end of the plan loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addPlanPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Construct the input queue specified by the configuration data.
     * @return Pointer to instance of a class derived from InputQueue.
     */
    virtual InputQueue *makeInputQueue() const = 0;


    /* --------------------------------------------------------------------- */
    /* -------------------- Deprecated Interface Methods ------------------- */
    /* --------------------------------------------------------------------- */

    //
    // API to interface adapters
    //

    /**
     * @deprecated
     * @brief Register the given interface adapter for this command,
     * overriding any previously registered adapters and the default
     * CommandHandler.
     * @param commandName The command to map to this adapter.
     * @param intf Pointer to the interface adapter to handle this
     * command.
     * @return Always returns true.
     */
    virtual bool registerCommandInterface(std::string const &commandName,
                                          InterfaceAdapter *intf) = 0;

    /**
     * @deprecated
     * @brief Register the given interface adapter for lookups to this
     * state name, overriding any previously registered adapter or
     * handler.
     * @param stateName The name of the state to map to this adapter.
     * @param intf The interface adapter to handle this lookup.
     * @param telemetryOnly False if this interface implements LookupNow, true otherwise.
     * @return Always returns true.
     */
    virtual bool registerLookupInterface(std::string const &stateName,
                                         InterfaceAdapter *intf,
                                         bool telemetryOnly = false) = 0;

    /**
     * @deprecated
     * @brief Register the given interface adapter using the
     * information in the configuration XML.
     * @param adapter The interface adapter to be registered.
     */
    virtual void defaultRegisterAdapter(InterfaceAdapter *adapter) = 0;

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for
     * lookups and commands which do not have a specific adapter, and
     * for planner updates if no other adapter or handler has been
     * registered. Overrides any previous default settings.
     * @param intf The interface adapter to use as the default.
     * @return Always returns true.
     */
    virtual bool setDefaultInterface(InterfaceAdapter *intf) = 0;

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for
     * lookups which do not have a specific adapter or handler
     * registered, overriding any previous default adapter or handler
     * registration.
     * @param intf The interface adapter to use as the default.
     * @return Always returns true.
     */
    virtual bool setDefaultLookupInterface(InterfaceAdapter *intf) = 0;

    /**
     * @deprecated
     * @brief Register the given interface adapter as the default for
     * commands which do not have a specific adapter or handler
     * registered, overriding any previous default adapter or handler
     * registration.
     * @param intf The interface adapter to use as the default.
     * @return Always returns true.
     */
    virtual bool setDefaultCommandInterface(InterfaceAdapter *intf) = 0;
  };

  //* @brief Abstract factory for AdapterConfiguration
  AdapterConfiguration *makeAdapterConfiguration();

  extern AdapterConfiguration *g_configuration;

}

#endif /* ADAPTERCONFIGURATION_HH_ */
