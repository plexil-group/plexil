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
 *               cfry
 */

#ifndef ADAPTERCONFIGURATION_HH_
#define ADAPTERCONFIGURATION_HH_

#include "CommandHandler.hh"
#include "Dispatcher.hh"
#include "LookupHandler.hh"
#include "PlannerUpdateHandler.hh"

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
  class InterfaceManager;
  class InputQueue;

  //*
  // @class AdapterConfiguration
  // @brief Abstract base class for registering external interfaces
  class AdapterConfiguration : public Dispatcher
  {
  public:

    /**
     * @brief Virtual destructor.
     */
    virtual ~AdapterConfiguration() = default;

    //! Constructs concrete interfaces as specified by the configuration XML.
    //! @param configXml The interface specifications.
    //! @param intf The InterfaceManager.
    //! @param listenerHub The ExecListenerHub.
    //! @return true if successful, false otherwise.
    virtual bool constructInterfaces(pugi::xml_node const configXml,
                                     InterfaceManager &intf,
                                     ExecListenerHub &listenerHub) = 0;

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
     * @brief Stops all interfaces.
     */
    virtual void stop() = 0;

    //
    // Handler registration functions
    //

    /**
     * @brief Register the given CommandHandler instance for all
     *        command names in the given vector.
     * @param handler Pointer to the CommandHandler instance.
     * @param names Const reference to Vector of command name strings.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void registerCommandHandler(CommandHandlerPtr handler,
                                        std::vector<std::string> const &names) = 0;

    /**
     * @brief Register the given CommandHandler for this command,
     *        overriding any previous handler or interface adapter
     *        registration.
     * @param handler Shared pointer to the CommandHandler instance.
     * @param cmdName The name of the command to map to this handler.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void registerCommandHandler(CommandHandlerPtr handler,
                                        std::string const &cmdName) = 0;

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
    virtual void registerCommandHandlerFunction(std::string const &cmdName,
                                                ExecuteCommandHandler execCmd,
                                                AbortCommandHandler abortCmd = defaultAbortCommandHandler)
    = 0;

    /**
     * @brief Register the CommandHandler instance as the default for
     * commands without a specific handler, overriding any previously
     * registered default handler or interface adapter, including the
     * default default handler.
     * @param handler Shared pointer to the CommandHandler instance.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void setDefaultCommandHandler(CommandHandlerPtr handler) = 0;
    
    /**
     * @brief Register the given handler function(s) as the default
     * for commands without a specific handler, overriding any
     * previously registered default handler or interface adapter,
     * including the default default handler.
     * @param execCmd The execute command handler function.
     * @param abortCmd (Optional) The abort command handler function.
     *        Defaults to defaultAbortCommandHandler if not supplied.
     * @see defaultAbortCommandHandler
     */
    virtual void setDefaultCommandHandlerFunction(ExecuteCommandHandler execCmd,
                                                  AbortCommandHandler abortCmd
                                                  = defaultAbortCommandHandler)
    = 0;

    /**
     * @brief Register the given LookupHandler instance for all lookup
     *        names in the given vector.
     * @param handler Shared pointer to the LookupHandler instance.
     * @param names Const reference to a vector of name strings.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void registerLookupHandler(LookupHandlerPtr handler,
                                       std::vector<std::string> const &names) = 0;

    /**
     * @brief Register the given LookupHandler instance for lookups
     *        for this state name, overriding any previous handler or
     *        interface adapter registrations for the name.
     * @param stateName The name of the state to map to this handler.
     * @param handler Pointer to the LookupHandler.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void registerLookupHandler(LookupHandlerPtr handler,
                                       std::string const &stateName) = 0;

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
     */
    virtual void registerLookupHandlerFunction(std::string const &stateName,
                                               LookupNowHandler ln,
                                               SetThresholdsHandlerReal setTD =
                                               SetThresholdsHandlerReal(),
                                               SetThresholdsHandlerInteger setTI =
                                               SetThresholdsHandlerInteger(),
                                               ClearThresholdsHandler clrT = 
                                               ClearThresholdsHandler())
    = 0;

    /**
     * @brief Register the given LookupHandler instance as the default
     * for lookups, overriding any previously registered default
     * handler or interface adapter, including the default default
     * handler.
     * @param handler Shared pointer to the new default LookupHandler.
     * @note The AdapterConfiguration instance takes shared ownership
     *       of the handler.
     */
    virtual void setDefaultLookupHandler(LookupHandlerPtr handler) = 0;
    
    /**
     * @brief Register the given handler function(s) as the default
     *        for lookups, overriding any previously registered default
     *        handler or interface adapter, including the default default
     *        handler.
     * @param lookupNow The lookup handler function to use as a default.
     * @param setThresholdsReal (Optional) The setThresholdsReal handler function.
     *        May be null.
     * @param setThresholdsInteger (Optional) The setThresholdsInteger handler function.
     *        May be null.
     */
    virtual void setDefaultLookupHandler(LookupNowHandler lookupNow,
                                         SetThresholdsHandlerReal setThresholdsReal =
                                         SetThresholdsHandlerReal(),
                                         SetThresholdsHandlerInteger setThresholdsInteger =
                                         SetThresholdsHandlerInteger(),
                                         ClearThresholdsHandler clrT =
                                         ClearThresholdsHandler())
    = 0;
    
    /**
     * @brief Register the given handler function for planner updates,
     *        overriding any previously registered handlers or interface
     *        adapters.
     * @param updateFn The handler function.
     */
    virtual void registerPlannerUpdateHandler(PlannerUpdateHandler updateFn) = 0;

    //! Add the ExecListener instance to the application.
    //! @param listener The ExecListener.
    //! @note Can be called from adapter initialization functions.
    virtual void addExecListener(ExecListener *listener) = 0;

    /**
     * @deprecated
     * @brief Add an externally constructed interface adapter.
     * @param adapter Pointer to the InterfaceAdapter instance.
     *
     * @note The AdapterConfiguration instance takes ownership 
     *       and is responsible for deleting the adapter.
     * @note The adapter is responsible for registering its own handlers
     *       in its initialize() method.
     */
    virtual void addInterfaceAdapter(InterfaceAdapter *adapter) = 0;

    //
    // Handler accessors
    //
    // Should only be used by InterfaceManager and module tests
    //

    /**
     * @brief Return the CommandHandler instance for a command name.
     * @param cmdName The command name.
     * @return Pointer to the CommandHandler instance for the named
     * command.
     */
    virtual CommandHandler *getCommandHandler(std::string const& stateName) const = 0;

    /**
     * @brief Return the LookupHandler instance in effect for lookups
     *        with this state name.
     * @param stateName The state.
     * @return Pointer to the LookupHandler for the named state.
     */
    virtual LookupHandler *getLookupHandler(std::string const& stateName) const = 0;

    /**
     * @brief Get the handler function for planner updates.
     * @return The handler function.
     */
    virtual PlannerUpdateHandler getPlannerUpdateHandler() const = 0;

    //
    // Path registration for plans and libraries
    //

    /**
     * @brief Add the specified directory names to the end of the plan loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addPlanPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Add the specified directory name to the end of the plan loading path.
     * @param libdir The directory name.
     */
    virtual void addPlanPath(const std::string& libdir) = 0;

    /**
     * @brief Add the specified directory names to the end of the library node loading path.
     * @param libdirs The vector of directory names.
     */
    virtual void addLibraryPath(const std::vector<std::string>& libdirs) = 0;

    /**
     * @brief Add the specified directory name to the end of the library node loading path.
     * @param libdir The directory name.
     */
    virtual void addLibraryPath(const std::string& libdir) = 0;

    //
    // Search path access for plans and libraries
    //

    /**
     * @brief Get the search path for plan files.
     * @return A const reference to the plan search path.
     */
    virtual const std::vector<std::string>& getPlanPath() const = 0;

    /**
     * @brief Get the search path for library nodes.
     * @return A const reference to the library search path.
     */
    virtual const std::vector<std::string>& getLibraryPath() const = 0;

    //
    // Input queue
    //

    /**
     * @brief Construct the input queue specified by the configuration data.
     * @return Pointer to instance of a class derived from InputQueue.
     *
     * @note Use of configuration data for selecting an input queue 
     *       type has not yet been implemented.
     */
    virtual InputQueue *makeInputQueue() const = 0;
  };

  //* @brief Abstract factory for AdapterConfiguration
  AdapterConfiguration *makeAdapterConfiguration();

}

#endif /* ADAPTERCONFIGURATION_HH_ */
