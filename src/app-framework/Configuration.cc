/* Copyright (c) 2006-2022, Universities Space Research Association (USRA).
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

#include "plexil-config.h"

#include "Configuration.hh"
#include "Error.hh"
#include "InterfaceSchema.hh"

#include <cstring>  // strcmp()

using namespace pugi;

namespace PLEXIL
{

  AdapterConf *parseAdapterConfiguration(xml_node const configXml)
  {
    if (!configXml) {
      warn("parseAdapterConfiguration: empty XML");
      return nullptr;
    }
    if (configXml.type() != node_element) {
      warn("parseAdapterConfiguration: not an XML element");
      return nullptr;
    }
    if (strcmp(configXml.name(), InterfaceSchema::ADAPTER_TAG)) {
      warn("parseAdapterConfiguration: not an "
           << InterfaceSchema::ADAPTER_TAG << " element");
      return nullptr;
    }
    const char *typeName =
      configXml.attribute(InterfaceSchema::ADAPTER_TYPE_ATTR).value();
    if (!typeName || !*typeName) {
      warn("parseAdapterConfiguration: missing or empty "
           << InterfaceSchema::ADAPTER_TYPE_ATTR << "attribute");
      return nullptr;
    }

    // Now that the error checking is out of the way, construct the configuration
    AdapterConf *conf = new AdapterConf();
    conf->xml = configXml;
    conf->typeName = typeName;

    // Commands
    if (configXml.child(InterfaceSchema::DEFAULT_COMMAND_ADAPTER_TAG)
        || configXml.child(InterfaceSchema::DEFAULT_ADAPTER_TAG)) {
      conf->defaultCommandAdapter = true;
    }
    else {
      xml_node names = configXml.child(InterfaceSchema::COMMAND_NAMES_TAG);
      while (names) {
        std::vector<std::string> *nameList =
          InterfaceSchema::parseCommaSeparatedArgs(names.child_value());
        for (std::string name : *nameList)
          conf->commandNames.push_back(name);
        delete nameList;
        names = names.next_sibling(InterfaceSchema::COMMAND_NAMES_TAG);
      }
    }

    // Lookups
    if (configXml.child(InterfaceSchema::DEFAULT_LOOKUP_ADAPTER_TAG)
        || configXml.child(InterfaceSchema::DEFAULT_ADAPTER_TAG)) {
      conf->defaultLookupAdapter = true;
    }
    else {
      xml_node names = configXml.child(InterfaceSchema::LOOKUP_NAMES_TAG);
      while (names) {
        std::vector<std::string> *nameList =
          InterfaceSchema::parseCommaSeparatedArgs(names.child_value());
        for (std::string name : *nameList)
          conf->lookupNames.push_back(name);
        delete nameList;
        names = names.next_sibling(InterfaceSchema::LOOKUP_NAMES_TAG);
      }
    }

    // Planner update
    if (configXml.child(InterfaceSchema::PLANNER_UPDATE_TAG)
        || configXml.child(InterfaceSchema::DEFAULT_ADAPTER_TAG))
      conf->plannerUpdateAdapter = true;

    return conf;
  }

  CommandHandlerConf *parseCommandHandlerConfiguration(xml_node const configXml)
  {
    if (!configXml) {
      warn("parseCommandHandlerConfiguration: empty XML");
      return nullptr;
    }
    if (configXml.type() != node_element) {
      warn("parseCommandHandlerConfiguration: not an XML element");
      return nullptr;
    }
    if (strcmp(configXml.name(), InterfaceSchema::COMMAND_HANDLER_TAG)) {
      warn("parseCommandHandlerConfiguration: not a "
           << InterfaceSchema::COMMAND_HANDLER_TAG << " element");
      return nullptr;
    }
    const char *typeName =
      configXml.attribute(InterfaceSchema::HANDLER_TYPE_ATTR).value();
    if (!typeName || !*typeName) {
      warn("parseCommandHandlerConfiguration: missing or empty "
           << InterfaceSchema::HANDLER_TYPE_ATTR << "attribute");
      return nullptr;
    }

    // Now that the error checking is out of the way, construct the configuration
    CommandHandlerConf *conf = new CommandHandlerConf();
    conf->xml = configXml;
    conf->typeName = typeName;

    if (configXml.attribute(InterfaceSchema::DEFAULT_HANDLER_ATTR).as_bool()) {
      conf->defaultHandler = true;
    }
    else {
      xml_node names = configXml.child(InterfaceSchema::COMMAND_NAMES_TAG);
      while (names) {
        std::vector<std::string> *nameList =
          InterfaceSchema::parseCommaSeparatedArgs(names.child_value());
        for (std::string name : *nameList)
          conf->commandNames.push_back(name);
        delete nameList;
        names = names.next_sibling(InterfaceSchema::COMMAND_NAMES_TAG);
      }
    }

    return conf;
  }

  LookupHandlerConf *parseLookupHandlerConfiguration(xml_node const configXml)
  {
    if (!configXml) {
      warn("parseLookupHandlerConfiguration: empty XML");
      return nullptr;
    }
    if (configXml.type() != node_element) {
      warn("parseLookupHandlerConfiguration: not an XML element");
      return nullptr;
    }
    if (strcmp(configXml.name(), InterfaceSchema::LOOKUP_HANDLER_TAG)) {
      warn("parseLookupHandlerConfiguration: not a "
           << InterfaceSchema::LOOKUP_HANDLER_TAG << " element");
      return nullptr;
    }
    const char *typeName =
      configXml.attribute(InterfaceSchema::HANDLER_TYPE_ATTR).value();
    if (!typeName || !*typeName) {
      warn("parseLookupHandlerConfiguration: missing or empty "
           << InterfaceSchema::HANDLER_TYPE_ATTR << "attribute");
      return nullptr;
    }

    // Now that the error checking is out of the way, construct the configuration
    LookupHandlerConf *conf = new LookupHandlerConf();
    conf->xml = configXml;
    conf->typeName = typeName;

    if (configXml.attribute(InterfaceSchema::DEFAULT_HANDLER_ATTR).as_bool()) {
      conf->defaultHandler = true;
    }
    else {
      xml_node names = configXml.child(InterfaceSchema::LOOKUP_NAMES_TAG);
      while (names) {
        std::vector<std::string> *nameList =
          InterfaceSchema::parseCommaSeparatedArgs(names.child_value());
        for (std::string name : *nameList)
          conf->lookupNames.push_back(name);
        delete nameList;
        names = names.next_sibling(InterfaceSchema::LOOKUP_NAMES_TAG);
      }
    }

    return conf;
  }

  ListenerConf *parseListenerConfiguration(xml_node const configXml)
  {
    if (!configXml) {
      warn("parseListenerConfiguration: empty XML");
      return nullptr;
    }
    if (configXml.type() != node_element) {
      warn("parseListenerConfiguration: not an XML element");
      return nullptr;
    }
    if (strcmp(configXml.name(), InterfaceSchema::LISTENER_TAG)) {
      warn("parseListenerConfiguration: not a "
           << InterfaceSchema::LISTENER_TAG << " element");
      return nullptr;
    }
    const char *typeName =
      configXml.attribute(InterfaceSchema::LISTENER_TYPE_ATTR).value();
    if (!typeName || !*typeName) {
      warn("parseListenerConfiguration: missing or empty "
           << InterfaceSchema::LISTENER_TYPE_ATTR << "attribute");
      return nullptr;
    }

    // TODO? filters

    // Now that the error checking is out of the way, construct the configuration
    ListenerConf *conf = new ListenerConf();
    conf->xml = configXml;
    conf->typeName = typeName;

    return conf;
  }

} // namespace PLEXIL
