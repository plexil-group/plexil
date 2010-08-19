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

#include "ExecListenerFactory.hh"
#include "Debug.hh"
#include "DynamicLoader.hh"
#include "ExecListener.hh"
#include "InterfaceManagerBase.hh"
#include "InterfaceSchema.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace PLEXIL
{
  //
  // ExecListenerFactory
  //

  /**
   * @brief Creates a new ExecListener instance with the type associated with the name and
   *        the given configuration XML.
   * @param xml The configuration XML specifying the ExecListener.
   * @param mgr A reference to the owning InterfaceManager (as an InterfaceManagerBase).
   * @return The Id for the new ExecListener.
   */

  ExecListenerId 
  ExecListenerFactory::createInstance(const TiXmlElement* xml,
                      InterfaceManagerBase & mgr)
  {
    // Can't do anything without the spec
    assertTrueMsg(xml != NULL,
          "ExecListenerFactory::createInstance: null configuration XML");

    // Get the kind of listener to make
    const char* listenerType = 
      xml->Attribute(InterfaceSchema::LISTENER_TYPE_ATTR());
    checkError(listenerType != 0,
           "ExecListenerFactory::createInstance: no "
           << InterfaceSchema::LISTENER_TYPE_ATTR()
           << " attribute for listener XML:\n"
           << *xml);

    // Make it
    return createInstance(LabelStr(listenerType), xml, mgr);
  }

  /**
   * @brief Creates a new ExecListener instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the ExecListener constructor.
   * @param mgr A reference to the owning InterfaceManager (as an InterfaceManagerBase).
   * @return The Id for the new ExecListener.
   */

  ExecListenerId 
  ExecListenerFactory::createInstance(const LabelStr& name,
                                      const TiXmlElement* xml,
                      InterfaceManagerBase & mgr)
  {
    std::map<double, ExecListenerFactory*>::const_iterator it = factoryMap().find(name.getKey());
    if (it == factoryMap().end())
      {
        debugMsg("ExecListenerFactory:createInstance", 
                 "Attempting to dynamically load listener type \""
                 << name.c_str() << "\"");
        // Attempt to dynamically load library
        const char* libCPath =
          xml->Attribute(InterfaceSchema::LIB_PATH_ATTR());
        if (!DynamicLoader::loadModule(name.c_str(), libCPath)) {
          debugMsg("ExecListenerFactory:createInstance", 
                   " unable to load module for listener type \""
                   << name.c_str() << "\"");
          return ExecListenerId::noId();
        }
        // See if it's registered now
        it = factoryMap().find(name.getKey());
      }

    if (it == factoryMap().end()) {
      debugMsg("ExecListenerFactory:createInstance", 
               " No exec listener factory registered for name \"" << name.c_str() << "\"");
      return ExecListenerId::noId();
    }
    ExecListenerId retval = it->second->create(xml, mgr);
    debugMsg("ExecListenerFactory:createInstance", " Created Exec listener " << name.c_str());
    return retval;
  }

  std::map<double, ExecListenerFactory*>& ExecListenerFactory::factoryMap() 
  {
    static std::map<double, ExecListenerFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void ExecListenerFactory::purge()
  {
    for (std::map<double, ExecListenerFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an ExecListenerFactory with the specific name.
   * @param name The name by which the Exec Listener shall be known.
   * @param factory The ExecListenerFactory instance.
   */
  void ExecListenerFactory::registerFactory(const LabelStr& name, ExecListenerFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name.getKey()) != factoryMap().end())
      {
        warn("Attempted to register an exec listener factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name] = factory;
    debugMsg("ExecListenerFactory:registerFactory",
             " Registered exec listener factory for name \"" << name.c_str() << "\"");
  }

  bool ExecListenerFactory::isRegistered(const LabelStr& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

  //
  // ExecListenerFilterFactory
  //

  /**
   * @brief Creates a new ExecListenerFilter instance with the type associated with the name and
   *        the given configuration XML.
   * @param xml The configuration XML specifying the ExecListenerFilter.
   * @param mgr A reference to the owning InterfaceManager (as an InterfaceManagerBase).
   * @return The Id for the new ExecListenerFilter.
   */

  ExecListenerFilterId 
  ExecListenerFilterFactory::createInstance(const TiXmlElement* xml,
                        InterfaceManagerBase & mgr)
  {
    // Can't do anything without the spec
    assertTrueMsg(xml != NULL,
          "ExecListenerFilterFactory::createInstance: null configuration XML");

    // Get the kind of filter to make
    const char* filterType = 
      xml->Attribute(InterfaceSchema::FILTER_TYPE_ATTR());
    checkError(filterType != 0,
           "ExecListenerFilterFactory::createInstance: no "
           << InterfaceSchema::FILTER_TYPE_ATTR()
           << " attribute for filter XML:\n"
           << *xml);

    // Make it
    return createInstance(LabelStr(filterType), xml, mgr);
  }


  /**
   * @brief Creates a new ExecListenerFilter instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the ExecListenerFilter constructor.
   * @param mgr A reference to the owning InterfaceManager (as an InterfaceManagerBase).
   * @return The Id for the new ExecListenerFilter.
   */

  ExecListenerFilterId 
  ExecListenerFilterFactory::createInstance(const LabelStr& name,
                                            const TiXmlElement* xml,
                        InterfaceManagerBase & mgr)
  {
    std::map<double, ExecListenerFilterFactory*>::const_iterator it = factoryMap().find(name.getKey());
    if (it == factoryMap().end())
      {
    debugMsg("ExecListenerFilterFactory:createInstance", 
         "Attempting to dynamically load filter type \""
         << name.c_str() << "\"");
    // Attempt to dynamically load library
    const char* libCPath =
      xml->Attribute(InterfaceSchema::LIB_PATH_ATTR());
    if (!DynamicLoader::loadModule(name.c_str(), libCPath)) {
      debugMsg("ExecListenerFilterFactory:createInstance",
               " unable to load module for filter type \""
               << name.c_str() << "\"");
      return ExecListenerFilterId::noId();
    }

    // See if it's registered now
    it = factoryMap().find(name.getKey());
      }

    if (it == factoryMap().end()) {
      debugMsg("ExecListenerFilterFactory:createInstance", 
          " No exec listener filter factory registered for name \""
                  << name.c_str() << "\".");
      return ExecListenerFilterId::noId();
    }
    ExecListenerFilterId retval = it->second->create(xml, mgr);
    debugMsg("ExecListenerFilterFactory:createInstance",
             " Created Exec listener filter " << name.c_str());
    return retval;
  }

  std::map<double, ExecListenerFilterFactory*>& ExecListenerFilterFactory::factoryMap() 
  {
    static std::map<double, ExecListenerFilterFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void ExecListenerFilterFactory::purge()
  {
    for (std::map<double, ExecListenerFilterFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an ExecListenerFilterFactory with the specific name.
   * @param name The name by which the Exec Listener shall be known.
   * @param factory The ExecListenerFilterFactory instance.
   */
  void ExecListenerFilterFactory::registerFactory(const LabelStr& name, ExecListenerFilterFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name.getKey()) != factoryMap().end())
      {
        warn("Attempted to register an exec listener filter factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name.getKey()] = factory;
    debugMsg("ExecListenerFilterFactory:registerFactory",
             " Registered exec listener filter factory for name \"" << name.c_str() << "\"");
  }

}
