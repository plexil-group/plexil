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

#ifndef ADAPTER_FACTORY_H
#define ADAPTER_FACTORY_H

#include "AdapterExecInterface.hh"

#include "pugixml.hpp"

#include <map>
#include <memory>
#include <string>

namespace PLEXIL
{
  //
  // Forward declarations
  //

  struct AdapterConf;
  class AdapterFactory;
  class InterfaceAdapter;

  using AdapterFactoryPtr = std::unique_ptr<AdapterFactory>;
  using AdapterFactoryMap = std::map<std::string, AdapterFactoryPtr>;

  /**
   * @brief Factory class for InterfaceAdapter instances.  
   *        Implements the AbstractFactory design pattern.
   */
  class AdapterFactory 
  {
  public:

    virtual ~AdapterFactory() = default;

    /**
     * @brief Creates a new InterfaceAdapter instance as specified by
     *        the given configuration XML.
     * @param xml The configuration XML to be passed to the InterfaceAdapter constructor.
     * @return The new InterfaceAdapter.  May not be unique.
     */
    static InterfaceAdapter *createInstance(pugi::xml_node const xml);

    /**
     * @brief Checks whether or not an AdapterFactory has been registered
     *        for this name.
     * @param name Const reference to the factory name.
     * @return True if a factory has been registered under the name,
     *         false otherwise.
     */
    static bool isRegistered(std::string const& name);

    /**
     * @brief Deallocate all factories
     * @note Used in post-run cleanup.
     */
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
    AdapterFactory(std::string const &name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

    /**
     * @brief Registers an AdapterFactory with the given name.
     * @param name The name by which the Adapter shall be known.
     * @param factory The AdapterFactory instance.
     */
    static void registerFactory(std::string const &name, AdapterFactory *factory);

    /**
     * @brief Instantiates a new InterfaceAdapter of the type named in the XML.
     * @param xml The configuration struct for the adapter to be constructed.
     * @return Pointer to the new InterfaceAdapter.
     */
    virtual InterfaceAdapter *create(AdapterConf *conf) const = 0;

  private:

    // Deliberately unimplemented
    AdapterFactory() = delete;
    AdapterFactory(AdapterFactory const &) = delete;
    AdapterFactory(AdapterFactory &&) = delete;

    AdapterFactory &operator=(AdapterFactory const &) = delete;
    AdapterFactory &operator=(AdapterFactory &&) = delete;

    /**
     * @brief The map from names to concrete AdapterFactory instances.
     * 
     * @note This pattern of wrapping static data in a static method
     *       is to ensure proper loading when used as a shared library.
     */
    static AdapterFactoryMap& factoryMap();

    //
    // Instance variables
    //

    std::string const m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Adapter type.
   */
  template<class AdapterType>
  class ConcreteAdapterFactory : public AdapterFactory 
  {
  public:
    ConcreteAdapterFactory(std::string const &name)
      : AdapterFactory(name) 
    {
    }

  private:

    // Deliberately unimplemented
    ConcreteAdapterFactory() = delete;
    ConcreteAdapterFactory(ConcreteAdapterFactory const &) = delete;
    ConcreteAdapterFactory(ConcreteAdapterFactory &&) = delete;

    ConcreteAdapterFactory &operator=(ConcreteAdapterFactory const &) = delete;
    ConcreteAdapterFactory &operator=(ConcreteAdapterFactory &&) = delete;

    /**
     * @brief Instantiates a new InterfaceAdapter of the appropriate type.
     * @param xml The configuration XML for the instantiated Adapter.
     * @return The new InterfaceAdapter.
     */

    InterfaceAdapter *create(AdapterConf *conf) const
    {
      // FIXME - reference to global variable
      return new AdapterType(*g_execInterface, conf);
    }
  };

} // namespace PLEXIL

//!
// @brief Macro to define and construct factory instances
//
#define REGISTER_ADAPTER(CLASS,NAME) {new PLEXIL::ConcreteAdapterFactory<CLASS>(NAME);}

#endif // ADAPTER_FACTORY_H
