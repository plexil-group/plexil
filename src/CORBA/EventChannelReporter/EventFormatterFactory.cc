/* Copyright (c) 2006-2010, Universities Space Research Association (USRA).
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

#include "EventFormatterFactory.hh"
#include "EventFormatter.hh"
#include "InterfaceManagerBase.hh"
#include "Debug.hh"
#include "DynamicLoader.hh"
#include "InterfaceSchema.hh"
#include "EventFormatterSchema.hh"
#include "pugixml.hpp"

namespace PLEXIL
{
  //
  // EventFormatterFactory
  //

  /**
   * @brief Creates a new EventFormatter instance with the type associated with the name and
   *        the given configuration XML.
   * @param xml The configuration XML specifying the EventFormatter.
   * @return The Id for the new EventFormatter.
   */

  EventFormatterId 
  EventFormatterFactory::createInstance(const pugi::xml_node& xml)
  {
    // Can't do anything without the spec
    assertTrueMsg(!xml.empty(),
		  "EventFormatterFactory::createInstance: null configuration XML");

    // Get the kind of listener to make
    const char* formatterType = 
      xml.attribute(EventFormatterSchema::EVENT_FORMATTER_TYPE_ATTRIBUTE()).value();
    checkError(*formatterType != '\0',
	       "EventFormatterFactory::createInstance: no "
	       << EventFormatterSchema::EVENT_FORMATTER_TYPE_ATTRIBUTE()
	       << " attribute for formatter XML");

    // See if this is a known type
    LabelStr name(formatterType);
    std::map<double, EventFormatterFactory*>::const_iterator it = factoryMap().find(name.getKey());
    if (it == factoryMap().end())
      {
	debugMsg("EventFormatterFactory:createInstance", 
		 "Attempting to dynamically load formatter type \""
		 << formatterType << "\"");
	// Attempt to dynamically load library
	const char* libCPath =
	  xml.attribute(InterfaceSchema::LIB_PATH_ATTR()).value();
	assertTrueMsg(DynamicLoader::loadModule(formatterType, libCPath),
		      "EventFormatterFactory::createInstance: unable to load module for formatter type \""
		      << formatterType << "\"");

	// See if it's registered now
	it = factoryMap().find(name.getKey());
      }

    assertTrueMsg(it != factoryMap().end(),
		  "Error: No exec formatter factory registered for name \"" << formatterType << "\".");
    EventFormatterId retval = it->second->create(xml);
    debugMsg("EventFormatterFactory:createInstance", " Created Exec formatter " << formatterType);
    return retval;
  }

  std::map<double, EventFormatterFactory*>& EventFormatterFactory::factoryMap() 
  {
    static std::map<double, EventFormatterFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void EventFormatterFactory::purge()
  {
    for (std::map<double, EventFormatterFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an EventFormatterFactory with the specific name.
   * @param name The name by which the Exec Formatter shall be known.
   * @param factory The EventFormatterFactory instance.
   */
  void EventFormatterFactory::registerFactory(const LabelStr& name, EventFormatterFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name.getKey()) != factoryMap().end())
      {
        warn("Attempted to register an exec formatter factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name] = factory;
    debugMsg("EventFormatterFactory:registerFactory",
             " Registered exec formatter factory for name \"" << name.c_str() << "\"");
  }

  bool EventFormatterFactory::isRegistered(const LabelStr& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

  //
  // StructuredEventFormatterFactory
  //

  /**
   * @brief Creates a new StructuredEventFormatter instance with the type associated with the name and
   *        the given configuration XML.
   * @param xml The configuration XML specifying the StructuredEventFormatter.
   * @return The Id for the new StructuredEventFormatter.
   */

  StructuredEventFormatterId 
  StructuredEventFormatterFactory::createInstance(const pugi::xml_node& xml)
  {
    // Can't do anything without the spec
    assertTrueMsg(!xml.empty(),
		  "StructuredEventFormatterFactory::createInstance: null configuration XML");

    // Get the kind of listener to make
    const char* formatterType = 
      xml.attribute(EventFormatterSchema::STRUCTURED_FORMATTER_TYPE_ATTRIBUTE()).value();
    checkError(*formatterType != '\0',
	       "StructuredEventFormatterFactory::createInstance: no "
	       << EventFormatterSchema::STRUCTURED_FORMATTER_TYPE_ATTRIBUTE()
	       << " attribute for formatter XML");

    // See if this is a known type
    LabelStr name(formatterType);
    std::map<double, StructuredEventFormatterFactory*>::const_iterator it = factoryMap().find(name.getKey());
    if (it == factoryMap().end())
      {
	debugMsg("StructuredEventFormatterFactory:createInstance", 
		 "Attempting to dynamically load structured formatter type \""
		 << formatterType << "\"");
	// Attempt to dynamically load library
	const char* libCPath =
	  xml.attribute(InterfaceSchema::LIB_PATH_ATTR()).value();
	assertTrueMsg(DynamicLoader::loadModule(formatterType, libCPath),
		      "StructuredEventFormatterFactory::createInstance: unable to load module for structured formatter type \""
		      << formatterType << "\"");

	// See if it's registered now
	it = factoryMap().find(name.getKey());
      }

    assertTrueMsg(it != factoryMap().end(),
		  "Error: No structured event formatter factory registered for name \"" << formatterType << "\".");
    StructuredEventFormatterId retval = it->second->create(xml);
    debugMsg("StructuredEventFormatterFactory:createInstance", " Created structured formatter " << formatterType);
    return retval;
  }

  std::map<double, StructuredEventFormatterFactory*>& StructuredEventFormatterFactory::factoryMap() 
  {
    static std::map<double, StructuredEventFormatterFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void StructuredEventFormatterFactory::purge()
  {
    for (std::map<double, StructuredEventFormatterFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an StructuredEventFormatterFactory with the specific name.
   * @param name The name by which the Exec Formatter shall be known.
   * @param factory The StructuredEventFormatterFactory instance.
   */
  void StructuredEventFormatterFactory::registerFactory(const LabelStr& name, StructuredEventFormatterFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name.getKey()) != factoryMap().end())
      {
        warn("Attempted to register a structured event formatter factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name] = factory;
    debugMsg("StructuredEventFormatterFactory:registerFactory",
             " Registered structured formatter factory for name \"" << name.c_str() << "\"");
  }

  bool StructuredEventFormatterFactory::isRegistered(const LabelStr& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

}
