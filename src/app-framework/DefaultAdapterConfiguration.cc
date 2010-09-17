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
 * DefaultAdapterConfiguration.cc
 *
 *  Created on: Jan 28, 2010
 *      Author: jhogins
 */

#include "DefaultAdapterConfiguration.hh"
#include "Debug.hh"
#include "PlexilXmlParser.hh"
#include "Id.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "DefaultAdapterConfiguration.hh"

namespace PLEXIL {

DefaultAdapterConfiguration::DefaultAdapterConfiguration(InterfaceManager* manager) :
  AdapterConfiguration(manager) {
  m_defaultInterface = InterfaceAdapterId();
  m_defaultCommandInterface = InterfaceAdapterId();
  m_defaultLookupInterface = InterfaceAdapterId();

}

DefaultAdapterConfiguration::~DefaultAdapterConfiguration() {
  clearAdapterRegistry();
}

/**
 * @brief Register the given interface adapter. By default, this method does nothing
 * @param adapter The interface adapter to handle this command.
 */

void DefaultAdapterConfiguration::defaultRegisterAdapter(InterfaceAdapterId adapter) {
  debugMsg("DefaultAdapterConfiguration:defaultRegisterAdapter", " for adapter " << adapter);
  // Walk the children of the configuration XML element
  // and register the adapter according to the data found there
  const TiXmlElement* element = adapter->getXml()->FirstChildElement();
  while (element != 0) {
    const char* elementType = element->Value();
    // look for text as the only child of this element
    // to use below
    const TiXmlNode* firstChild = element->FirstChild();
    const TiXmlText* text = 0;
    if (firstChild != 0)
      text = firstChild->ToText();

    if (strcmp(elementType, InterfaceSchema::DEFAULT_ADAPTER_TAG()) == 0) {
      setDefaultInterface(adapter);
      // warn if children found
      if (text != 0) {
        warn("registerInterface: extraneous text in "
            << InterfaceSchema::DEFAULT_ADAPTER_TAG()
            << " ignored");
      } else if (firstChild != 0) {
        warn("registerInterface: extraneous XML element(s) in "
            << InterfaceSchema::DEFAULT_ADAPTER_TAG()
            << " ignored");
      }
    } else if (strcmp(elementType, InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG()) == 0) {
      setDefaultCommandInterface(adapter);
      // warn if children found
      if (text != 0) {
        warn("registerInterface: extraneous text in "
            << InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG()
            << " ignored");
      } else if (firstChild != 0) {
        warn("registerInterface: extraneous XML element(s) in "
            << InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG()
            << " ignored");
      }
    }
    if (strcmp(elementType, InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG()) == 0) {
      setDefaultLookupInterface(adapter);
      // warn if children found
      if (text != 0) {
        warn("registerInterface: extraneous text in "
            << InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG()
            << " ignored");
      } else if (firstChild != 0) {
        warn("registerInterface: extraneous XML element(s) in "
            << InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG()
            << " ignored");
      }
    } else if (strcmp(elementType, InterfaceSchema::PLANNER_UPDATE_TAG()) == 0) {
      registerPlannerUpdateInterface(adapter);
      // warn if children found
      if (text != 0) {
        warn("registerInterface: extraneous text in "
            << InterfaceSchema::PLANNER_UPDATE_TAG()
            << " ignored");
      } else if (firstChild != 0) {
        warn("registerInterface: extraneous XML element(s) in "
            << InterfaceSchema::PLANNER_UPDATE_TAG()
            << " ignored");
      }
    } else if (strcmp(elementType, InterfaceSchema::COMMAND_NAMES_TAG()) == 0) {
      checkError(text != 0,
          "registerAdapter: Invalid configuration XML: "
          << InterfaceSchema::COMMAND_NAMES_TAG()
          << " requires one or more comma-separated command names");
      std::vector<std::string> * cmdNames = InterfaceSchema::parseCommaSeparatedArgs(text->Value());
      for (std::vector<std::string>::const_iterator it = cmdNames->begin(); it != cmdNames->end(); it++) {
        registerCommandInterface(LabelStr(*it), adapter);
      }
      delete cmdNames;
    } else if (strcmp(elementType, InterfaceSchema::LOOKUP_NAMES_TAG()) == 0) {
      checkError(text != 0,
          "registerAdapter: Invalid configuration XML: "
          << InterfaceSchema::LOOKUP_NAMES_TAG()
          << " requires one or more comma-separated lookup names");
      std::vector<std::string> * lookupNames = InterfaceSchema::parseCommaSeparatedArgs(text->Value());
      for (std::vector<std::string>::const_iterator it = lookupNames->begin(); it != lookupNames->end(); it++) {
        registerLookupInterface(LabelStr(*it), adapter);
      }
      delete lookupNames;
    }
    // ignore other tags, they're for adapter's use

    element = element->NextSiblingElement();
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
bool DefaultAdapterConfiguration::registerCommandInterface(const LabelStr & commandName, InterfaceAdapterId intf) {
  double commandNameKey = commandName.getKey();
  InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
  if (it == m_commandMap.end()) {
    // Not found, OK to add
    debugMsg("DefaultAdapterConfiguration:registerCommandInterface",
        " registering interface for command '" << commandName.toString() << "'");
    m_commandMap.insert(std::pair<double, InterfaceAdapterId>(commandNameKey, intf));
    getAdaptersFromManager().insert(intf);
    return true;
  } else {
    debugMsg("DefaultAdapterConfiguration:registerCommandInterface",
        " interface already registered for command '" << commandName.toString() << "'");
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
bool DefaultAdapterConfiguration::registerLookupInterface(const LabelStr & stateName, InterfaceAdapterId intf) {
  double stateNameKey = stateName.getKey();
  InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
  if (it == m_lookupMap.end()) {
    // Not found, OK to add
    debugMsg("DefaultAdapterConfiguration:registerLookupInterface",
        " registering interface for lookup '" << stateName.toString() << "'");
    m_lookupMap.insert(std::pair<double, InterfaceAdapterId>(stateNameKey, intf));
    getAdaptersFromManager().insert(intf);
    return true;
  } else {
    debugMsg("DefaultAdapterConfiguration:registerLookupInterface",
        " interface already registered for lookup '" << stateName.toString() << "'");
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
bool DefaultAdapterConfiguration::registerPlannerUpdateInterface(InterfaceAdapterId intf) {
  if (!m_plannerUpdateInterface.isNoId()) {
    debugMsg("DefaultAdapterConfiguration:registerPlannerUpdateInterface",
        " planner update interface already registered");
    return false;
  }
  debugMsg("DefaultAdapterConfiguration:registerPlannerUpdateInterface",
      " registering planner update interface");
  m_plannerUpdateInterface = intf;
  getAdaptersFromManager().insert(intf);
  return true;
}

/**
 * @brief Register the given interface adapter as the default for all lookups and commands
 which do not have a specific adapter.  Returns true if successful.
 Fails and returns false if there is already a default adapter registered
 or setting the default interface is not implemented.
 * @param intf The interface adapter to use as the default.
 */
bool DefaultAdapterConfiguration::setDefaultInterface(InterfaceAdapterId intf) {
  if (!m_defaultInterface.isNoId()) {
    debugMsg("DefaultAdapterConfiguration:setDefaultInterface",
        " attempt to overwrite default interface adapter " << m_defaultInterface);
    return false;
  }
  m_defaultInterface = intf;
  getAdaptersFromManager().insert(intf);
  debugMsg("DefaultAdapterConfiguration:setDefaultInterface",
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
bool DefaultAdapterConfiguration::setDefaultLookupInterface(InterfaceAdapterId intf) {
  if (!m_defaultLookupInterface.isNoId()) {
    debugMsg("DefaultAdapterConfiguration:setDefaultLookupInterface",
        " attempt to overwrite default lookup interface adapter " << m_defaultLookupInterface);
    return false;
  }
  m_defaultLookupInterface = intf;
  getAdaptersFromManager().insert(intf);
  debugMsg("DefaultAdapterConfiguration:setDefaultLookupInterface",
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
bool DefaultAdapterConfiguration::setDefaultCommandInterface(InterfaceAdapterId intf) {
  if (!m_defaultCommandInterface.isNoId()) {
    debugMsg("DefaultAdapterConfiguration:setDefaultCommandInterface",
        " attempt to overwrite default command interface adapter " << m_defaultCommandInterface);
    return false;
  }
  m_defaultCommandInterface = intf;
  getAdaptersFromManager().insert(intf);
  debugMsg("DefaultAdapterConfiguration:setDefaultCommandInterface",
      " setting default command interface " << intf);
  return true;
}

/**
 * @brief Retract registration of the previous interface adapter for this command.
 * Does nothing by default.
 * @param commandName The command.
 */
void DefaultAdapterConfiguration::unregisterCommandInterface(const LabelStr & commandName) {
  double commandNameKey = commandName.getKey();
  InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
  if (it != m_commandMap.end()) {
    debugMsg("DefaultAdapterConfiguration:unregisterCommandInterface",
        " removing interface for command '" << commandName.toString() << "'");
    InterfaceAdapterId intf = it->second;
    m_commandMap.erase(it);
    deleteIfUnknown(intf);
  }
}

/**
 * @brief Retract registration of the previous interface adapter for this state.
 * Does nothing by default.
 * @param stateName The state name.
 */
void DefaultAdapterConfiguration::unregisterLookupInterface(const LabelStr & stateName) {
  double stateNameKey = stateName.getKey();
  InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
  if (it != m_lookupMap.end()) {
    debugMsg("DefaultAdapterConfiguration:unregisterLookupInterface",
        " removing interface for lookup '" << stateName.toString() << "'");
    InterfaceAdapterId intf = it->second;
    m_lookupMap.erase(it);
    deleteIfUnknown(intf);
  }
}

/**
 * @brief Retract registration of the previous interface adapter for planner updates.
 * Does nothing by default.
 */
void DefaultAdapterConfiguration::unregisterPlannerUpdateInterface() {
  debugMsg("DefaultAdapterConfiguration:unregisterPlannerUpdateInterface",
      " removing planner update interface");
  InterfaceAdapterId intf = m_plannerUpdateInterface;
  m_plannerUpdateInterface = InterfaceAdapterId::noId();
  deleteIfUnknown(intf);
}

/**
 * @brief Retract registration of the previous default interface adapter.
 * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
 */
void DefaultAdapterConfiguration::unsetDefaultInterface() {
  debugMsg("DefaultAdapterConfiguration:unsetDefaultInterface",
      " removing default interface");
  InterfaceAdapterId intf = m_defaultInterface;
  m_defaultInterface = InterfaceAdapterId::noId();
  deleteIfUnknown(intf);
}

/**
 * @brief Retract registration of the previous default interface adapter for commands.
 * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
 */
void DefaultAdapterConfiguration::unsetDefaultCommandInterface() {
  debugMsg("DefaultAdapterConfiguration:unsetDefaultCommandInterface",
      " removing default command interface");
  InterfaceAdapterId intf = m_defaultCommandInterface;
  m_defaultCommandInterface = InterfaceAdapterId::noId();
  deleteIfUnknown(intf);
}

/**
 * @brief Retract registration of the previous default interface adapter for lookups.
 * If default interfaces are implemented, this must retract registration. Otherwise, does nothing.
 */
void DefaultAdapterConfiguration::unsetDefaultLookupInterface() {
  debugMsg("DefaultAdapterConfiguration:unsetDefaultLookupInterface",
      " removing default lookup interface");
  InterfaceAdapterId intf = m_defaultLookupInterface;
  m_defaultLookupInterface = InterfaceAdapterId::noId();
  deleteIfUnknown(intf);
}

/**
 * @brief Return the interface adapter in effect for this command, whether
 specifically registered or default. May return NoId().
 * @param commandName The command.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getCommandInterface(const LabelStr & commandName) {
  double commandNameKey = commandName.getKey();
  InterfaceMap::iterator it = m_commandMap.find(commandNameKey);
  if (it != m_commandMap.end()) {
    debugMsg("DefaultAdapterConfiguration:getCommandInterface",
        " found specific interface " << (*it).second
        << " for command '" << commandName.toString() << "'");
    return (*it).second;
  }
  // check default command i/f
  if (m_defaultCommandInterface.isId()) {
    debugMsg("DefaultAdapterConfiguration:getCommandInterface",
        " returning default command interface " << m_defaultCommandInterface
        << " for command '" << commandName.toString() << "'");
    return m_defaultCommandInterface;
  }
  // fall back on default default
  debugMsg("DefaultAdapterConfiguration:getCommandInterface",
      " returning default interface " << m_defaultInterface
      << " for command '" << commandName.toString() << "'");
  return m_defaultInterface;
}

/**
 * @brief Return the current default interface adapter for commands.
 May return NoId(). Returns NoId() if default interfaces are not implemented.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getDefaultCommandInterface() {
  return m_defaultCommandInterface;
}

/**
 * @brief Return the interface adapter in effect for lookups with this state name,
 whether specifically registered or default. May return NoId(). Returns NoId() if default interfaces are not implemented.
 * @param stateName The state.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getLookupInterface(const LabelStr & stateName) {
  double stateNameKey = stateName.getKey();
  InterfaceMap::iterator it = m_lookupMap.find(stateNameKey);
  if (it != m_lookupMap.end()) {
    debugMsg("DefaultAdapterConfiguration:getLookupInterface",
        " found specific interface " << (*it).second
        << " for lookup '" << stateName.toString() << "'");
    return (*it).second;
  }
  // try defaults
  if (m_defaultLookupInterface.isId()) {
    debugMsg("DefaultAdapterConfiguration:getLookupInterface",
        " returning default lookup interface " << m_defaultLookupInterface
        << " for lookup '" << stateName.toString() << "'");
    return m_defaultLookupInterface;
  }
  // try default defaults
  debugMsg("DefaultAdapterConfiguration:getLookupInterface",
      " returning default interface " << m_defaultInterface
      << " for lookup '" << stateName.toString() << "'");
  return m_defaultInterface;
}

/**
 * @brief Return the current default interface adapter for lookups.
 May return NoId(). Returns NoId() if default interfaces are not implemented.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getDefaultLookupInterface() {
  return m_defaultLookupInterface;
}

/**
 * @brief Return the interface adapter in effect for planner updates,
 whether specifically registered or default. May return NoId().
 Returns NoId() if default interfaces are not implemented.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getPlannerUpdateInterface() {
  if (m_plannerUpdateInterface.isNoId()) {
    debugMsg("DefaultAdapterConfiguration:getPlannerUpdateInterface",
        " returning default interface " << m_defaultInterface);
    return m_defaultInterface;
  }
  debugMsg("DefaultAdapterConfiguration:getPlannerUpdateInterface",
      " found specific interface " << m_plannerUpdateInterface);
  return m_plannerUpdateInterface;
}

/**
 * @brief Return the current default interface adapter. May return NoId().
 *  Returns NoId() if default interfaces are not implemented.
 */
InterfaceAdapterId DefaultAdapterConfiguration::getDefaultInterface() {
  return m_defaultInterface;
}

/**
 * @brief Returns true if the given adapter is a known interface in the system. False otherwise
 */
bool DefaultAdapterConfiguration::isKnown(InterfaceAdapterId intf) {
  // Check the easy places first
  if (intf == m_defaultInterface
  || intf == m_defaultCommandInterface
  || intf == m_defaultLookupInterface
      || intf == m_plannerUpdateInterface)
    return true;

  // See if the adapter is in any of the tables
  InterfaceMap::iterator it = m_lookupMap.begin();
  for (; it != m_lookupMap.end(); it++)
    if (it->second == intf)
      return true;
  it = m_commandMap.begin();
  for (; it != m_commandMap.end(); it++)
    if (it->second == intf)
      return true;
  return false;
}

/**
 * @brief Removes the adapter and deletes it from the manager iff nothing refers to it.
 */
void DefaultAdapterConfiguration::deleteIfUnknown(InterfaceAdapterId intf) {
  if (!isKnown(intf)) {
    deleteAdapter(intf);
  }
}

/**
 * @brief Clears the interface adapter registry.
 */
void DefaultAdapterConfiguration::clearAdapterRegistry() {
  m_lookupMap.clear();
  m_commandMap.clear();
  m_plannerUpdateInterface = InterfaceAdapterId::noId();
  m_defaultInterface = InterfaceAdapterId::noId();
  m_defaultCommandInterface = InterfaceAdapterId::noId();
  m_defaultLookupInterface = InterfaceAdapterId::noId();
}
}
