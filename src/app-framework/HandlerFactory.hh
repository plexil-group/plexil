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

#include <map>
#include <string>

// Forward reference
namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{

  // Abstract factory class for interface objects

  template <class BASE>
  class InterfaceFactory
  {
  public:
    virtual ~InterfaceFactory() = default;

    //!
    // @brief Creates a new BASE instance as specified by
    //        the given configuration XML.
    // @param xml The configuration XML to be shared with the constructor.
    // @return The new BASE.
    //
    static BASE *createInstance(pugi::xml_node const xml);

    //!
    // @brief Creates a new BASE instance with the type associated with the name and
    //        the given configuration XML.
    // @param name The registered name for the factory.
    // @param xml The configuration XML to be shared with the constructor.
    // @return The new BASE.
    //
    static BASE *createInstance(std::string const& name, 
                                pugi::xml_node const xml);
    //!
    // @brief Checks whether or not an InterfaceFactory has been registered
    //        for this name.
    // @param name Const reference to the factory name.
    // @return True if a factory has been registered under the name,
    //         false otherwise.
    //
    static bool isRegistered(std::string const& name);

    //!
    // @brief Deallocate all factories derived from this one.
    //
    // ????
    static void purge();

    std::string const& getName() const {return m_name;}

  protected:

    //
    // @brief Constructor.
    // @param name Const reference to the name of this factory.
    //
    // N.B. The constructor is protected because this is an abstract class,
    // and only derived classes should be able to construct the base class.
    //
    InterfaceFactory(std::string const &name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

    //!
    // @brief Registers an InterfaceFactory with the given name.
    // @param name The name by which the Adapter shall be known.
    // @param factory The InterfaceFactory instance.
    //
    static void registerFactory(std::string const &name, InterfaceFactory *factory);

    //!
    // @brief Instantiates a new BASE of the type named in the XML.
    // @param xml The configuration XML for the BASE to be constructed.
    // @return The new BASE.
    //
    virtual BASE *create(pugi::xml_node const xml) const = 0;

  private:

    // Deliberately unimplemented
    InterfaceFactory() = delete;
    InterfaceFactory(InterfaceFactory const &) = delete;
    InterfaceFactory(InterfaceFactory &&) = delete;

    InterfaceFactory &operator=(InterfaceFactory const &) = delete;
    InterfaceFactory &operator=(InterfaceFactory &&) = delete;

    //!
    // @brief The map from names to concrete InterfaceFactory instances.
    // 
    // @note This pattern of wrapping static data in a static method
    //       is to ensure proper loading when used as a shared library.
    //
    static std::map<std::string, InterfaceFactoryPtr>& factoryMap();

    //
    // Instance variables
    //

    std::string const m_name; /*!< Name used for lookup */
  };

  //!
  // @brief Concrete factory class, templated for each Adapter type.
  //
  template<class BASE, class DERIVED>
  class ConcreteInterfaceFactory : public InterfaceFactory 
  {
  public:
    ConcreteInterfaceFactory(std::string const &name)
      : InterfaceFactory(name) 
    {
    }

  private:

    // Deliberately unimplemented
    ConcreteInterfaceFactory() = delete;
    ConcreteInterfaceFactory(ConcreteInterfaceFactory const &) = delete;
    ConcreteInterfaceFactory(ConcreteInterfaceFactory &&) = delete;

    ConcreteInterfaceFactory &operator=(ConcreteInterfaceFactory const &) = delete;
    ConcreteInterfaceFactory &operator=(ConcreteInterfaceFactory &&) = delete;

    //!
    // @brief Constructs a new instance of the derived type.
    // @param xml The configuration XML for the instantiated object.
    // @return The new object as a//BASE.
    //
    BASE *create(pugi::xml_node const xml) const
    {
      return new DERIVED(execInterface, xml);
    }
  };

} // namespace PLEXIL

#endif // PLEXIL_INTERFACE_FACTORY_HH
