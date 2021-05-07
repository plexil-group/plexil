// Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_INTERFACE_FACTORY_HH
#define PLEXIL_INTERFACE_FACTORY_HH

//
// Templatized basis for interface object factories
//

#include "Error.hh"            // warn() macro
#include "InterfaceSchema.hh"  // NAME_ATTR
#include "lifecycle-utils.h"   // plexil_add_finalizer()

#include "pugixml.hpp"

#include <map>
#include <memory>
#include <string>

namespace PLEXIL
{

  //
  // Helper functions for these templates
  // Defined in the .cc file
  //
  
  //!
  // @brief Check that the XML is non-empty and has a
  //        non-empty value for the named attribute.
  // @param xml An XML element to check.
  // @param attrName The attribute name.
  // @return The attribute value if all is correct, nullptr otherwise.
  //
  const char *checkInterfaceXml(const pugi::xml_node xml, const char *attrName);

  //!
  // @brief Attempt to dynamically load the named library.
  // @param name The library name.
  // @param xml The XML, which may contain hints as to the library location.
  // @return true if successfully loaded, false otherwise.
  bool tryLoadSharedLib(const char *name, const pugi::xml_node xml);

  //!
  // @class InterfaceFactory
  // @brief Templatized abstract factory base class for interface objects.
  template <class BASE>
  class InterfaceFactory
  {
  private:

    //
    // Type aliases
    //

    using FactoryPtr = std::unique_ptr<InterfaceFactory>;
    using FactoryMap = std::map<std::string, FactoryPtr>;

  public:

    virtual ~InterfaceFactory() = default;

    //!
    // @brief Creates a new BASE instance as specified by
    //        the given configuration XML.
    // @param xml The configuration XML to be shared with the constructor.
    // @return The new BASE.
    //
    static BASE *createInstance(pugi::xml_node const xml,
                                const char *typeAttribute = InterfaceSchema::NAME_ATTR)
    {
      // Check spec, get the kind of BASE to make
      const char* name = checkInterfaceXml(xml, typeAttribute);
      if (!name || !*name) 
        return nullptr;

      // Make it
      FactoryMap::const_iterator it = factoryMap().find(name);
      if (it == factoryMap().end()) {
        if (tryLoadSharedLibrary(name, xml)) {
          // See if it's registered now
          it = factoryMap().find(name);
        }
      }
      if (it == factoryMap().end()) {
        warn("AdapterFactory: No factory registered for adapter type \""
             << name.c_str()
             << "\".");
        return nullptr;
      }
      BASE *retval = it->second->create(xml);
      debugMsg("InterfaceFactory:createInstance", " Created " << name);
      return retval;
    }

    //
    // Public instance variables
    //
    const std::string name;

  protected:

    //
    // @brief Constructor.
    // @param name Const reference to the name of this factory.
    //
    // N.B. The constructor is protected because this is an abstract class,
    // and only derived classes should be able to construct the base class.
    //
    InterfaceFactory(std::string const &nam)
      : name(nam)
    {
      registerFactory(name, this);
    }

    //!
    // @brief Instantiates a new BASE of the type named in the XML.
    // @param xml The configuration XML for the BASE to be constructed.
    // @return The new BASE.
    //
    // @note Implemented by each concrete factory class.
    virtual BASE *create(pugi::xml_node const xml) const = 0;

  private:

    // Deliberately unimplemented
    InterfaceFactory() = delete;
    InterfaceFactory(InterfaceFactory const &) = delete;
    InterfaceFactory(InterfaceFactory &&) = delete;

    InterfaceFactory &operator=(InterfaceFactory const &) = delete;
    InterfaceFactory &operator=(InterfaceFactory &&) = delete;

    //
    // Implementation details
    //

    //!
    // @brief Access the map from names to ConcreteInterfaceFactory instances.
    // @return The map.
    // 
    // @note This pattern of wrapping static data in a static method
    //       is to ensure proper loading when used as a shared library.
    //
    static FactoryMap &factoryMap()
    {
      static FactoryMap sl_map;
      static bool sl_inited = false;
      if (!sl_inited) {
        plexilAddFinalizer(&purge);
        sl_inited = true;
      }
      return sl_map;
    }

    //!
    // @brief Registers an InterfaceFactory with the given name.
    // @param name The name by which the factory shall be known.
    // @param factory The InterfaceFactory instance.
    //
    static void registerFactory(std::string const &name, InterfaceFactory *factory)
    {
      factoryMap()[name].reset(factory);
      debugMsg("InterfaceFactory:registerFactory",
               " Registered factory for \"" << name << "\"");
    }

    //!
    // @brief Deallocate all registered factories.
    //
    static void purge()
    {
      factoryMap().clear();
    }
  };

  //!
  // @brief Concrete factory class, templated for each Adapter type.
  //
  template<class BASE, class DERIVED>
  class ConcreteInterfaceFactory : public InterfaceFactory 
  {
  public:
    ConcreteInterfaceFactory(std::string const &nam)
      : InterfaceFactory(nam) 
    {
    }

  protected:

    //!
    // @brief Constructs a new instance of the derived type.
    // @param xml The configuration XML for the instantiated object.
    // @return The new object as a pointer to BASE.
    //
    BASE *create(pugi::xml_node const xml) const
    {
      return new DERIVED(execInterface, xml);
    }

  private:

    // Deliberately unimplemented
    ConcreteInterfaceFactory() = delete;
    ConcreteInterfaceFactory(ConcreteInterfaceFactory const &) = delete;
    ConcreteInterfaceFactory(ConcreteInterfaceFactory &&) = delete;

    ConcreteInterfaceFactory &operator=(ConcreteInterfaceFactory const &) = delete;
    ConcreteInterfaceFactory &operator=(ConcreteInterfaceFactory &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_INTERFACE_FACTORY_HH
