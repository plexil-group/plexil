/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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
 * AdapterConfiguration.cc
 *
 *  Created on: Jan 28, 2010
 *      Author: jhogins
 */

#include "AdapterConfiguration.hh"
#include "Id.hh"
#include "InterfaceManager.hh"
#include "Error.hh"

#include <set>

namespace PLEXIL {

  AdapterConfiguration::AdapterConfiguration(InterfaceManager* manager) :
    m_manager(manager),
    m_id(this)
  {}

  AdapterConfiguration::~AdapterConfiguration() {}

  /**
   * @brief Returns the InterfaceManager set in initilization
   * @return The InterfaceManager set in initilization.
   */
  InterfaceManager* AdapterConfiguration::getInterfaceManager() {
    return m_manager;
  }

  /**
   * @brief Register the given interface adapter. By default, this method does nothing
   * @param adapter The interface adapter to handle this command.
   */

  void AdapterConfiguration::defaultRegisterAdapter(InterfaceAdapterId /* adapter */) {}

  /**
   * @brief Register the given interface adapter for this command.
   Returns true if successful.  Fails and returns false
   iff the command name already has an adapter registered
            or setting a command interface is not implemented.
   * @param commandName The command to map to this adapter.
   * @param intf The interface adapter to handle this command.
   */
  bool AdapterConfiguration::registerCommandInterface(const LabelStr & /* commandName */,
						      InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Register the given interface adapter for this function.
            Returns true if successful.  Fails and returns false
            iff the function name already has an adapter registered
            or setting a function interface is not implemented.
   * @param functionName The function to map to this adapter.
   * @param intf The interface adapter to handle this function.
   */
  bool AdapterConfiguration::registerFunctionInterface(const LabelStr & /* functionName */,
						       InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Register the given interface adapter for lookups to this state.
   Returns true if successful.  Fails and returns false
   if the state name already has an adapter registered
            or registering a lookup interface is not implemented.
   * @param stateName The name of the state to map to this adapter.
   * @param intf The interface adapter to handle this lookup.
   */
  bool AdapterConfiguration::registerLookupInterface(const LabelStr & /* stateName */,
						     InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Register the given interface adapter for planner updates.
            Returns true if successful.  Fails and returns false
            iff an adapter is already registered
            or setting the default planner update interface is not implemented.
   * @param intf The interface adapter to handle planner updates.
   */
  bool AdapterConfiguration::registerPlannerUpdateInterface(InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Register the given interface adapter as the default for all lookups and commands
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered
            or setting the default interface is not implemented.
   * @param intf The interface adapter to use as the default.
   */
  bool AdapterConfiguration::setDefaultInterface(InterfaceAdapterId /* intf */) {
    return false;
  }

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
  bool AdapterConfiguration::setDefaultLookupInterface(InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Register the given interface adapter as the default for commands.
            This interface will be used for all commands which do not have
        a specific adapter.
            Returns true if successful.
        Fails and returns false if there is already a default command adapter registered.
   * @param intf The interface adapter to use as the default.
   * @return True if successful, false if there is already a default adapter registered.
   */
  bool AdapterConfiguration::setDefaultCommandInterface(InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Retract registration of the previous interface adapter for this command.
   * Does nothing by default.
   * @param commandName The command.
   */
  void AdapterConfiguration:: unregisterCommandInterface(const LabelStr & /* commandName */) {}

  /**
   * @brief Retract registration of the previous interface adapter for this function.
   * Does nothing by default.
   * @param functionName The function.
   */
  void AdapterConfiguration:: unregisterFunctionInterface(const LabelStr & /* functionName */) {}

  /**
   * @brief Retract registration of the previous interface adapter for this state.
   * Does nothing by default.
   * @param stateName The state name.
   */
  void AdapterConfiguration:: unregisterLookupInterface(const LabelStr & /* stateName */) {}

  /**
   * @brief Retract registration of the previous interface adapter for planner updates.
   * Does nothing by default.
   */
  void AdapterConfiguration:: unregisterPlannerUpdateInterface() {}

  /**
   * @brief Retract registration of the previous default interface adapter.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultInterface() {}

  /**
   * @brief Retract registration of the previous default interface adapter for commands.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultCommandInterface() {}

  /**
   * @brief Retract registration of the previous default interface adapter for lookups.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultLookupInterface() {}

  /**
   * @brief Return the interface adapter in effect for this command, whether
   specifically registered or default. May return noId().
   * @param commandName The command.
   */
  InterfaceAdapterId AdapterConfiguration:: getCommandInterface(const LabelStr & /* commandName */) {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the current default interface adapter for commands.
            May return noId(). Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultCommandInterface() {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the interface adapter in effect for this function, whether
   specifically registered or default. May return noId(). Returns noId() if default interfaces are not implemented.
   * @param functionName The function.
   */
  InterfaceAdapterId AdapterConfiguration:: getFunctionInterface(const LabelStr & /* functionName */) {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the interface adapter in effect for lookups with this state name,
   whether specifically registered or default. May return noId(). Returns noId() if default interfaces are not implemented.
   * @param stateName The state.
   */
  InterfaceAdapterId AdapterConfiguration:: getLookupInterface(const LabelStr & /* stateName */) {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the current default interface adapter for lookups.
            May return noId(). Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultLookupInterface() {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the interface adapter in effect for planner updates,
            whether specifically registered or default. May return noId().
            Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getPlannerUpdateInterface() {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Return the current default interface adapter. May return noId().
   *  Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultInterface() {
    return InterfaceAdapterId::noId();
  }

  /**
   * @brief Returns true if the given adapter is a known interface in the system. False otherwise
   */
  bool AdapterConfiguration::isKnown(InterfaceAdapterId /* intf */) {
    return false;
  }

  /**
   * @brief Clears the interface adapter registry.
   */
  void AdapterConfiguration:: clearAdapterRegistry() {}

  /**
   * For implementors, returns the vector of interface adapters from the manager
   */
  std::set<InterfaceAdapterId>& AdapterConfiguration::getAdaptersFromManager() {
    return m_manager->m_adapters;
  }
  /**
   * @brief Deletes the given adapter from the interface manager
   * @return true if the given adapter existed and was deleted. False if not found
   */
  bool AdapterConfiguration::deleteAdapter(InterfaceAdapterId intf) {
    return m_manager->deleteAdapter(intf);
  }

}
