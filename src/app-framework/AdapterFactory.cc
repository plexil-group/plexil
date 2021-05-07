// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    // Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    // Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    // Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//
// AdapterFactory implementation
//

#include "plexil-config.h" // may be redundant

#include "AdapterFactory.hh"

#include "Configuration.hh"
#include "Debug.hh"
#if defined(HAVE_DLFCN_H)
#include "DynamicLoader.h"
#endif
#include "Error.hh"
#include "InterfaceAdapter.hh"
#include "InterfaceSchema.hh"
#include "lifecycle-utils.h"

namespace PLEXIL
{
  //! Construct an InterfaceAdapter instance as specified by the given
  //! configuration XML.
  //! @param xml The configuration XML describing the new adapter
  //! @param intf Reference to the parent AdapterExecInterface instance.
  //! @return Pointer to the new adapter.
  InterfaceAdapter *
  AdapterFactory::createInstance(pugi::xml_node const xml,
                                 AdapterExecInterface &intf)
  {
    debugMsg("AdapterFactory:createInstance", " xml = " << xml);

    AdapterConf *conf = parseAdapterConfiguration(xml);
    if (!conf) {
      warn("AdapterFactory: unable to parse configuration XML");
      return nullptr;
    }

    // Get the kind of adapter to make
    std::string adapterType = conf->typeName;
    AdapterFactoryMap::const_iterator it = factoryMap().find(adapterType);
#ifdef HAVE_DLFCN_H
    if (it == factoryMap().end()) {
      debugMsg("AdapterFactory:createInstance", 
               " Attempting to dynamically load adapter type \"" << adapterType << "\"");
      // Attempt to dynamically load library
      const char* libCPath =
        xml.attribute(InterfaceSchema::LIB_PATH_ATTR).value();
      if (!dynamicLoadModule(adapterType.c_str(), libCPath)) {
        warn("AdapterFactory: unable to load module for adapter type \""
             << adapterType.c_str() << "\"");
        return nullptr;
      }

      // See if it's registered now
      it = factoryMap().find(adapterType);
    }
#endif

    if (it == factoryMap().end()) {
      warn("AdapterFactory: No factory registered for adapter type \""
           << adapterType << "\".");
      return nullptr;
    }
    InterfaceAdapter *retval = it->second->create(conf, intf);
    debugMsg("AdapterFactory:createInstance", " Created adapter " << adapterType);
    return retval;
  }

  bool AdapterFactory::isRegistered(std::string const& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

  AdapterFactoryMap& AdapterFactory::factoryMap() 
  {
    static AdapterFactoryMap sl_map;
    static bool sl_inited = false;
    if (!sl_inited) {
      plexilAddFinalizer(&purge);
      sl_inited = true;
    }
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void AdapterFactory::purge()
  {
    factoryMap().clear();
  }

  /**
   * @brief Registers an AdapterFactory with the specific name.
   * @param name The name by which the Adapter shall be known.
   * @param factory The AdapterFactory instance.
   */
  void AdapterFactory::registerFactory(std::string const& name, AdapterFactory* factory)
  {
    factoryMap()[name].reset(factory);
    debugMsg("AdapterFactory:registerFactory",
             " Registered adapter factory for name \"" << name.c_str() << "\"");
  }

}
