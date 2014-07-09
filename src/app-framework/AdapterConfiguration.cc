/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#include "Debug.hh"
#include "Error.hh"
#include "InterfaceManager.hh"
#include "InterfaceSchema.hh"
#include "PlexilXmlParser.hh"

#include <cstring>

namespace PLEXIL {

  AdapterConfiguration::AdapterConfiguration(InterfaceManager* manager) :
    m_manager(manager),
    m_id(this),
    m_defaultInterface(),
    m_defaultCommandInterface(),
    m_defaultLookupInterface()
  {}

  AdapterConfiguration::~AdapterConfiguration()
  {
    clearAdapterRegistry();
  }

  /**
   * @brief Returns the InterfaceManager set in initialization
   * @return The InterfaceManager set in initialization.
   */
  InterfaceManager* AdapterConfiguration::getInterfaceManager() {
    return m_manager;
  }

  /**
   * @brief Register the given interface adapter as the default.
   * @param adapter The interface adapter.
   */

  void AdapterConfiguration::defaultRegisterAdapter(InterfaceAdapterId adapter) 
  {
    debugMsg("AdapterConfiguration:defaultRegisterAdapter", " for adapter " << adapter);
    // Walk the children of the configuration XML element
    // and register the adapter according to the data found there
    pugi::xml_node element = adapter->getXml().first_child();
    while (!element.empty()) {
      const char* elementType = element.name();
      // look for text as the only child of this element
      // to use below
      const pugi::xml_node firstChild = element.first_child();
      const char* text = NULL;
      if (!firstChild.empty() && firstChild.type() == pugi::node_pcdata)
        text = firstChild.value();

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
        checkError(*text != '\0',
                   "registerAdapter: Invalid configuration XML: "
                   << InterfaceSchema::COMMAND_NAMES_TAG()
                   << " requires one or more comma-separated command names");
        std::vector<std::string> * cmdNames = InterfaceSchema::parseCommaSeparatedArgs(text);
        for (std::vector<std::string>::const_iterator it = cmdNames->begin(); it != cmdNames->end(); ++it) {
          registerCommandInterface(*it, adapter);
        }
        delete cmdNames;
      } 
      else if (strcmp(elementType, InterfaceSchema::LOOKUP_NAMES_TAG()) == 0) {
        checkError(*text != '\0',
                   "registerAdapter: Invalid configuration XML: "
                   << InterfaceSchema::LOOKUP_NAMES_TAG()
                   << " requires one or more comma-separated lookup names");
        std::vector<std::string> * lookupNames = InterfaceSchema::parseCommaSeparatedArgs(text);
        for (std::vector<std::string>::const_iterator it = lookupNames->begin(); it != lookupNames->end(); ++it) {
          registerLookupInterface(*it, adapter);
        }
        delete lookupNames;
      }
      // ignore other tags, they're for adapter's use

      element = element.next_sibling();
    }
  }

  /**
   * @brief Register the given interface adapter for this command.
   Returns true if successful.  Fails and returns false
   iff the command name already has an adapter registered
            or setting a command interface is not implemented.
   * @param commandName The command to map to this adapter.
   * @param intf The interface adapter to handle this command.
   */
  bool AdapterConfiguration::registerCommandInterface(std::string const &commandName,
                                                      InterfaceAdapterId intf) {
    InterfaceMap::iterator it = m_commandMap.find(commandName);
    if (it == m_commandMap.end()) {
      // Not found, OK to add
      debugMsg("AdapterConfiguration:registerCommandInterface",
               " registering interface for command '" << commandName << "'");
      m_commandMap.insert(std::pair<std::string, InterfaceAdapterId>(commandName, intf));
      m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
      return true;
    } else {
      debugMsg("AdapterConfiguration:registerCommandInterface",
               " interface already registered for command '" << commandName << "'");
      return false;
    }
  }

  /**
   * @brief Register the given interface adapter for lookups to this state.
   Returns true if successful.  Fails and returns false
   if the state name already has an adapter registered
            or registering a lookup interface is not implemented.
   * @param stateName The name of the state to map to this adapter.
   * @param intf The interface adapter to handle this lookup.
   */
  bool AdapterConfiguration::registerLookupInterface(std::string const &stateName,
                                                     InterfaceAdapterId intf) {
    InterfaceMap::iterator it = m_lookupMap.find(stateName);
    if (it == m_lookupMap.end()) {
      // Not found, OK to add
      debugMsg("AdapterConfiguration:registerLookupInterface",
               " registering interface for lookup '" << stateName << "'");
      m_lookupMap.insert(std::pair<std::string, InterfaceAdapterId>(stateName, intf));
      m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
      return true;
    } else {
      debugMsg("AdapterConfiguration:registerLookupInterface",
               " interface already registered for lookup '" << stateName << "'");
      return false;
    }
  }

  /**
   * @brief Register the given interface adapter for planner updates.
            Returns true if successful.  Fails and returns false
            iff an adapter is already registered
            or setting the default planner update interface is not implemented.
   * @param intf The interface adapter to handle planner updates.
   */
  bool AdapterConfiguration::registerPlannerUpdateInterface(InterfaceAdapterId intf) {
    if (!m_plannerUpdateInterface.isNoId()) {
      debugMsg("AdapterConfiguration:registerPlannerUpdateInterface",
               " planner update interface already registered");
      return false;
    }
    debugMsg("AdapterConfiguration:registerPlannerUpdateInterface",
             " registering planner update interface");
    m_plannerUpdateInterface = intf;
    m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
    return true;
  }

  /**
   * @brief Register the given interface adapter as the default for all lookups and commands
   which do not have a specific adapter.  Returns true if successful.
   Fails and returns false if there is already a default adapter registered
            or setting the default interface is not implemented.
   * @param intf The interface adapter to use as the default.
   */
  bool AdapterConfiguration::setDefaultInterface(InterfaceAdapterId intf) {
    if (!m_defaultInterface.isNoId()) {
      debugMsg("AdapterConfiguration:setDefaultInterface",
               " attempt to overwrite default interface adapter " << m_defaultInterface);
      return false;
    }
    m_defaultInterface = intf;
    m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
    debugMsg("AdapterConfiguration:setDefaultInterface",
             " setting default interface " << intf);
    return true;
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
  bool AdapterConfiguration::setDefaultLookupInterface(InterfaceAdapterId intf) {
    if (!m_defaultLookupInterface.isNoId()) {
      debugMsg("AdapterConfiguration:setDefaultLookupInterface",
               " attempt to overwrite default lookup interface adapter " << m_defaultLookupInterface);
      return false;
    }
    m_defaultLookupInterface = intf;
    m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
    debugMsg("AdapterConfiguration:setDefaultLookupInterface",
             " setting default lookup interface " << intf);
    return true;
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
  bool AdapterConfiguration::setDefaultCommandInterface(InterfaceAdapterId intf) {
    if (!m_defaultCommandInterface.isNoId()) {
      debugMsg("AdapterConfiguration:setDefaultCommandInterface",
               " attempt to overwrite default command interface adapter " << m_defaultCommandInterface);
      return false;
    }
    m_defaultCommandInterface = intf;
    m_manager->m_adapters.insert(intf); // FIXME - does behavior belong in manager or config helper?
    debugMsg("AdapterConfiguration:setDefaultCommandInterface",
             " setting default command interface " << intf);
    return true;
  }

  /**
   * @brief Retract registration of the previous interface adapter for this command.
   * @param commandName The command.
   */
  void AdapterConfiguration:: unregisterCommandInterface(std::string const &commandName) 
  {
    InterfaceMap::iterator it = m_commandMap.find(commandName);
    if (it != m_commandMap.end()) {
      debugMsg("AdapterConfiguration:unregisterCommandInterface",
               " removing interface for command '" << commandName << "'");
      InterfaceAdapterId intf = it->second;
      m_commandMap.erase(it);
      deleteIfUnknown(intf);
    }
  }

  /**
   * @brief Retract registration of the previous interface adapter for this state.
   * @param stateName The state name.
   */
  void AdapterConfiguration:: unregisterLookupInterface(std::string const &stateName) 
  {
    InterfaceMap::iterator it = m_lookupMap.find(stateName);
    if (it != m_lookupMap.end()) {
      debugMsg("AdapterConfiguration:unregisterLookupInterface",
               " removing interface for lookup '" << stateName << "'");
      InterfaceAdapterId intf = it->second;
      m_lookupMap.erase(it);
      deleteIfUnknown(intf);
    }
  }

  /**
   * @brief Retract registration of the previous interface adapter for planner updates.
   * Does nothing by default.
   */
  void AdapterConfiguration:: unregisterPlannerUpdateInterface() 
  {
    debugMsg("AdapterConfiguration:unregisterPlannerUpdateInterface",
             " removing planner update interface");
    InterfaceAdapterId intf = m_plannerUpdateInterface;
    m_plannerUpdateInterface = InterfaceAdapterId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Retract registration of the previous default interface adapter.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultInterface() 
  {
    debugMsg("AdapterConfiguration:unsetDefaultInterface",
             " removing default interface");
    InterfaceAdapterId intf = m_defaultInterface;
    m_defaultInterface = InterfaceAdapterId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Retract registration of the previous default interface adapter for commands.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultCommandInterface() 
  {
    debugMsg("AdapterConfiguration:unsetDefaultCommandInterface",
             " removing default command interface");
    InterfaceAdapterId intf = m_defaultCommandInterface;
    m_defaultCommandInterface = InterfaceAdapterId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Retract registration of the previous default interface adapter for lookups.
   * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
   */
  void AdapterConfiguration:: unsetDefaultLookupInterface()
  {
    debugMsg("AdapterConfiguration:unsetDefaultLookupInterface",
             " removing default lookup interface");
    InterfaceAdapterId intf = m_defaultLookupInterface;
    m_defaultLookupInterface = InterfaceAdapterId::noId();
    deleteIfUnknown(intf);
  }

  /**
   * @brief Return the interface adapter in effect for this command, whether
   specifically registered or default. May return noId().
   * @param commandName The command.
   */
  InterfaceAdapterId AdapterConfiguration:: getCommandInterface(std::string const &commandName) {
    InterfaceMap::iterator it = m_commandMap.find(commandName);
    if (it != m_commandMap.end()) {
      debugMsg("AdapterConfiguration:getCommandInterface",
               " found specific interface " << (*it).second
               << " for command '" << commandName << "'");
      return (*it).second;
    }
    // check default command i/f
    if (m_defaultCommandInterface.isId()) {
      debugMsg("AdapterConfiguration:getCommandInterface",
               " returning default command interface " << m_defaultCommandInterface
               << " for command '" << commandName << "'");
      return m_defaultCommandInterface;
    }
    // fall back on default default
    debugMsg("AdapterConfiguration:getCommandInterface",
             " returning default interface " << m_defaultInterface
             << " for command '" << commandName << "'");
    return m_defaultInterface;
  }

  /**
   * @brief Return the current default interface adapter for commands.
            May return noId(). Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultCommandInterface() {
    return m_defaultCommandInterface;
  }

  /**
   * @brief Return the interface adapter in effect for lookups with this state name,
   whether specifically registered or default. May return noId(). Returns noId() if default interfaces are not implemented.
   * @param stateName The state.
   */
  InterfaceAdapterId AdapterConfiguration:: getLookupInterface(std::string const &stateName) {
    InterfaceMap::iterator it = m_lookupMap.find(stateName);
    if (it != m_lookupMap.end()) {
      debugMsg("AdapterConfiguration:getLookupInterface",
               " found specific interface " << (*it).second
               << " for lookup '" << stateName << "'");
      return (*it).second;
    }
    // try defaults
    if (m_defaultLookupInterface.isId()) {
      debugMsg("AdapterConfiguration:getLookupInterface",
               " returning default lookup interface " << m_defaultLookupInterface
               << " for lookup '" << stateName << "'");
      return m_defaultLookupInterface;
    }
    // try default defaults
    debugMsg("AdapterConfiguration:getLookupInterface",
             " returning default interface " << m_defaultInterface
             << " for lookup '" << stateName << "'");
    return m_defaultInterface;
  }

  /**
   * @brief Return the current default interface adapter for lookups.
            May return noId().
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultLookupInterface() {
    return m_defaultLookupInterface;
  }

  /**
   * @brief Return the interface adapter in effect for planner updates,
            whether specifically registered or default. May return noId().
            Returns noId() if default interfaces are not implemented.
   */
  InterfaceAdapterId AdapterConfiguration:: getPlannerUpdateInterface() {
    if (m_plannerUpdateInterface.isNoId()) {
      debugMsg("AdapterConfiguration:getPlannerUpdateInterface",
               " returning default interface " << m_defaultInterface);
      return m_defaultInterface;
    }
    debugMsg("AdapterConfiguration:getPlannerUpdateInterface",
             " found specific interface " << m_plannerUpdateInterface);
    return m_plannerUpdateInterface;
  }

  /**
   * @brief Return the current default interface adapter. May return noId().
   */
  InterfaceAdapterId AdapterConfiguration:: getDefaultInterface() {
    return m_defaultInterface;
  }

  /**
   * @brief Returns true if the given adapter is a known interface in the system. False otherwise
   */
  bool AdapterConfiguration::isKnown(InterfaceAdapterId intf) {
    // Check the easy places first
    if (intf == m_defaultInterface
        || intf == m_defaultCommandInterface
        || intf == m_defaultLookupInterface
        || intf == m_plannerUpdateInterface)
      return true;

    // See if the adapter is in any of the tables
    for (InterfaceMap::iterator it = m_lookupMap.begin(); it != m_lookupMap.end(); ++it)
      if (it->second == intf)
        return true;
    for (InterfaceMap::iterator it = m_commandMap.begin(); it != m_commandMap.end(); ++it)
      if (it->second == intf)
        return true;
    return false;
  }

  /**
   * @brief Clears the interface adapter registry.
   */
  void AdapterConfiguration:: clearAdapterRegistry() 
  {
    m_lookupMap.clear();
    m_commandMap.clear();
    m_plannerUpdateInterface = InterfaceAdapterId::noId();
    m_defaultInterface = InterfaceAdapterId::noId();
    m_defaultCommandInterface = InterfaceAdapterId::noId();
    m_defaultLookupInterface = InterfaceAdapterId::noId();
  }

  /**
   * @brief Deletes the given adapter from the interface manager
   * @return true if the given adapter existed and was deleted. False if not found
   */
  bool AdapterConfiguration::deleteAdapter(InterfaceAdapterId intf) {
    return m_manager->deleteAdapter(intf);
  }

  /**
   * @brief Removes the adapter and deletes it from the manager iff nothing refers to it.
   */
  void AdapterConfiguration::deleteIfUnknown(InterfaceAdapterId intf) {
    if (!isKnown(intf)) {
      deleteAdapter(intf);
    }
  }

}
