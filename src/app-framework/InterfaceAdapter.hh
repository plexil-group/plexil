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

#ifndef INTERFACE_ADAPTER_H
#define INTERFACE_ADAPTER_H

#include "pugixml.hpp"

#include "plexil-stdint.h"

namespace PLEXIL
{
  // forward references 
  class AdapterConfiguration;
  class AdapterExecInterface;
  class Command;
  class State;
  class StateCacheEntry;
  class Update;

  /**
   * @brief An abstract base class for interfacing the PLEXIL Universal Exec
   *        to other systems.
   * @see InterfaceManager, AdapterExecInterface
   */
  class InterfaceAdapter
  {

  public:

    //
    // Class constants
    //

    /**
     * @brief Constructor.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     */
    InterfaceAdapter(AdapterExecInterface& execInterface);

    /**
     * @brief Constructor from configuration XML.
     * @param execInterface Reference to the parent AdapterExecInterface object.
     * @param xml Const reference to the XML element describing this adapter
     * @note The instance maintains a shared reference to the XML.
     */
    InterfaceAdapter(AdapterExecInterface& execInterface, 
                     pugi::xml_node const xml);

    /**
     * @brief Destructor.
     */
    virtual ~InterfaceAdapter();

    //
    // API to ExecApplication
    //

    /**
     * @brief Initializes the adapter, possibly using its configuration data.
     * @return true if successful, false otherwise.
     * @note The default method does nothing and returns true.
     * @deprecated This signature is maintained only for backward compatibility.
     */
    virtual bool initialize();

    /**
     * @brief The adapter should initialize itself and
     *        register itself with the interface registry.
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods as required.
     * @note Default method simply calls the above member function
     *       for backward compatibility.
     */
    virtual bool initialize(AdapterConfiguration *);

    /**
     * @brief Starts the adapter, possibly using its configuration data.  
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods as required.
     * @note Default method simply returns true.
     */
    virtual bool start();

    /**
     * @brief Stops the adapter.  
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods as required.
     * @note Default method simply returns true.
     */
    virtual bool stop();

    /**
     * @brief Resets the adapter.  
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods as required.
     * @note Default method simply returns true.
     * @deprecated This member function will be removed in a future release.
     */
    virtual bool reset();

    /**
     * @brief Shuts down the adapter, releasing any of its resources.
     * @return true if successful, false otherwise.
     * @note Adapters should provide their own methods as required.
     * @note Default method simply returns true.
     */
    virtual bool shutdown();

    /**
     * @brief Perform an immediate lookup on an existing state.
     * @param state The state.
     * @return The current value for the state.
     * @note The default method does nothing.
     * @deprecated If you must implement lookupNow(), register a lookup handler.
     * @see LookupHandler
     */
    virtual void lookupNow(State const &state, StateCacheEntry &cacheEntry);

    /**
     * @brief Inform the interface that it should report changes in value of this state.
     * @param state The state.
     * @note Adapters should provide their own methods.
     * @note The default method does nothing.
     * @deprecated If you must implement this functionality, register a lookup handler.
     * @see LookupHandler
     */
    virtual void subscribe(const State& state);

    /**
     * @brief Inform the interface that a lookup should no longer receive updates.
     * @param state The state.
     * @note Adapters should provide their own methods.
     * @note The default method does nothing.
     * @deprecated If you must implement this functionality, register a lookup handler.
     * @see LookupHandler
     */
    virtual void unsubscribe(const State& state);

    /**
     * @brief Advise the interface of the current thresholds to use when reporting this state.
     * @param state The state.
     * @param hi The upper threshold, at or above which to report changes.
     * @param lo The lower threshold, at or below which to report changes.
     * @note Adapters should provide their own methods.
     * @note The default methods do nothing.
     * @deprecated If you must implement this functionality, register a lookup handler.
     * @see LookupHandler
     */
    virtual void setThresholds(const State& state, double hi, double lo);
    virtual void setThresholds(const State& state, int32_t hi, int32_t lo);

    /**
     * @brief Send the name of the supplied node, and the supplied value pairs, to the planner.
     * @param update Pointer to the Update instance containing the data to report to the planner.
     * @note Adapters may implement this method.
     * @note The default method does nothing.
     * @deprecated If you must implement this behavior, implement an exec listener 
     *             or a planner update handler instead.
     * @see ExecListener
     * @see PlannerUpdateHandler
     */
    virtual void sendPlannerUpdate(Update *update);

    /**
     * @brief Execute a command with the requested arguments.
     * @param cmd The Command instance.
     * @note Derived classes may implement this method.
     * @note The default method sends a command handle value of COMMAND_SENT_TO_SYSTEM.
     * @deprecated If you must implement this functionality, register a command handler.
     * @see CommandHandler
     */
    virtual void executeCommand(Command *cmd);

    /**
     * @brief Abort the pending command.
     * @param cmd Pointer to the command being aborted.
     * @note Derived classes may implement this method.
     * @note The default method sends a command abort acknowledge value of true.
     * @deprecated If you must implement this functionality, register a command handler.
     * @see CommandHandler
     */
    virtual void invokeAbort(Command *cmd);

    /**
     * @brief Get the configuration XML for this instance.
     */
    pugi::xml_node const getXml()
    {
      return m_xml;
    }

    /**
     * @brief Get the AdapterExecInterface for this instance.
     */
    AdapterExecInterface& getExecInterface()
    {
      return m_execInterface;
    }

  protected:

    //
    // API which all subclasses must implement
    //

    AdapterExecInterface& m_execInterface;

  private:

    // Deliberately unimplemented
    InterfaceAdapter();
    InterfaceAdapter(const InterfaceAdapter &);
    InterfaceAdapter & operator=(const InterfaceAdapter &);

    //
    // Member variables
    //

    const pugi::xml_node m_xml;
  };
}

#endif // INTERFACE_ADAPTER_H
