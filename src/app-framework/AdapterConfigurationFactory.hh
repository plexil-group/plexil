/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#ifndef ADAPTER_CONFIGURATION_FACTORY_H
#define ADAPTER_CONFIGURATION_FACTORY_H

#include "AdapterConfiguration.hh"
#include "Id.hh"
#include "LabelStr.hh"
#include <map>

namespace PLEXIL
{
  //
  // Forward declarations
  //

  class AdapterConfiguration;

  /**
   * @brief Factory class for InterfaceAdapter instances.  
   *        Implements the AbstractFactory design pattern.
   */
  class AdapterConfigurationFactory
  {
  public:

    /**
     * @brief Creates a new AdapterConfiguration instance with the type associated with the name.
     * @param name The registered name for the factory.
     * @param manager Reference to the parent InterfaceManager instance.
     * @return The Id for the new InterfaceAdapter.  May not be unique.
     */

    static AdapterConfigurationId createInstance(const LabelStr& name, InterfaceManager* manager);

    /**
     * @brief Creates a new AdapterConfiguration instance with the type associated with the name.
     * @param name The registered name for the factory.
     * @param manager Reference to the parent InterfaceManager instance.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdapter.  If wasCreated is set to false, is not unique.
     */

    static AdapterConfigurationId createInstance(const LabelStr& name, InterfaceManager* manager, bool& wasCreated);

    /**
     * @brief Checks whether or not the given AdapterConfigurationFactory is registered.
     * @param name The registered name for the factory
     * @return True if the factory is registered, false otherwise
     */

    static bool isRegistered(const LabelStr& name);

    /**
     * @brief Deallocate all factories
     */
    static void purge();

    const LabelStr& getName() const {return m_name;}

  protected:
    virtual ~AdapterConfigurationFactory()
    {}

    /**
     * @brief Registers an AdapterConfigurationFactory with the specific name.
     * @param name The name by which the Adapter shall be known.
     * @param factory The AdapterConfigurationFactory instance.
     */
    static void registerFactory(const LabelStr& name, AdapterConfigurationFactory* factory);

    /**
     * @brief Instantiates a new AdapterConfiguration of the appropriate type.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The new AdapterConfiguration.
     */
    virtual AdapterConfigurationId create(InterfaceManager* manager, bool& wasCreated) const = 0;

    AdapterConfigurationFactory(const LabelStr& name)
    : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    AdapterConfigurationFactory();
    AdapterConfigurationFactory(const AdapterConfigurationFactory&);
    AdapterConfigurationFactory& operator=(const AdapterConfigurationFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete AdapterConfigurationFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<LabelStr, AdapterConfigurationFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Adapter type.
   */
  template<class ConfigType>
  class ConcreteAdapterConfigurationFactory : public AdapterConfigurationFactory
  {
  public:
    ConcreteAdapterConfigurationFactory(const LabelStr& name)
    : AdapterConfigurationFactory(name)
    {}

  private:
    // Deliberately unimplemented
    ConcreteAdapterConfigurationFactory();
    ConcreteAdapterConfigurationFactory(const ConcreteAdapterConfigurationFactory&);
    ConcreteAdapterConfigurationFactory& operator=(const ConcreteAdapterConfigurationFactory&);

    /**
     * @brief Instantiates a new AdapterConfiguration of the appropriate type.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new AdapterConfiguration.
     */

    AdapterConfigurationId create(InterfaceManager* manager, bool& wasCreated) const
    {
      AdapterConfigurationId result = (new ConfigType(manager))->getId();
      wasCreated = true;
      return result;
    }
  };

#define REGISTER_ADAPTER_CONFIGURATION(CLASS,NAME) {new PLEXIL::ConcreteAdapterConfigurationFactory<CLASS>(*(new PLEXIL::LabelStr(NAME)));}

} // namespace PLEXIL

#endif // ADAPTER_CONFIGURATION_FACTORY_H
