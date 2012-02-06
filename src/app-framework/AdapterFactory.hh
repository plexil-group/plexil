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

#ifndef ADAPTER_FACTORY_H
#define ADAPTER_FACTORY_H

#include "Id.hh"
#include "LabelStr.hh"
#include <map>

// Forward reference
namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{
  //
  // Forward declarations
  //

  class InterfaceAdapter;
  typedef Id<InterfaceAdapter> InterfaceAdapterId;

  class AdapterExecInterface;

  /**
   * @brief Factory class for InterfaceAdapter instances.  
   *        Implements the AbstractFactory design pattern.
   */
  class AdapterFactory 
  {
  public:

    /**
     * @brief Creates a new InterfaceAdapter instance as specified by
     *        the given configuration XML.
     * @param xml The configuration XML to be passed to the InterfaceAdapter constructor.
     * @param execInterface Reference to the parent InterfaceManager instance.
     * @return The Id for the new InterfaceAdapter.  May not be unique.
     */

    static InterfaceAdapterId createInstance(const pugi::xml_node& xml,
                                             AdapterExecInterface& execInterface);


    /**
     * @brief Creates a new InterfaceAdapter instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param xml The configuration XML to be passed to the InterfaceAdapter constructor.
     * @param execInterface Reference to the parent InterfaceManager instance.
     * @return The Id for the new InterfaceAdapter.  May not be unique.
     */

    static InterfaceAdapterId createInstance(const LabelStr& name, 
                                             const pugi::xml_node& xml,
                                             AdapterExecInterface& execInterface);

    /**
     * @brief Creates a new InterfaceAdapter instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param xml The configuration XML to be passed to the InterfaceAdapter constructor.
     * @param execInterface Reference to the parent InterfaceManager instance.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdapter.  If wasCreated is set to false, is not unique.
     */

    static InterfaceAdapterId createInstance(const LabelStr& name,
                                             const pugi::xml_node& xml,
                                             AdapterExecInterface& execInterface,
                                             bool& wasCreated);

    /**
     * @brief Checks whether or not the given AdapterFactory is registered.
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
    virtual ~AdapterFactory()
    {}

    /**
     * @brief Registers an AdapterFactory with the specific name.
     * @param name The name by which the Adapter shall be known.
     * @param factory The AdapterFactory instance.
     */
    static void registerFactory(const LabelStr& name, AdapterFactory* factory);

    /**
     * @brief Instantiates a new InterfaceAdapter of the appropriate type.
     * @param xml The configuration XML for the instantiated Adapter.
     * @param execInterface Reference to the parent InterfaceManager instance.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdapter.
     */
    virtual InterfaceAdapterId create(const pugi::xml_node& xml,
                                      AdapterExecInterface& execInterface,
                                      bool& wasCreated) const = 0;

    AdapterFactory(const LabelStr& name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    AdapterFactory();
    AdapterFactory(const AdapterFactory&);
    AdapterFactory& operator=(const AdapterFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete AdapterFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, AdapterFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Adapter type.
   */
  template<class AdapterType>
  class ConcreteAdapterFactory : public AdapterFactory 
  {
  public:
    ConcreteAdapterFactory(const LabelStr& name)
      : AdapterFactory(name) 
    {}

  private:
    // Deliberately unimplemented
    ConcreteAdapterFactory();
    ConcreteAdapterFactory(const ConcreteAdapterFactory&);
    ConcreteAdapterFactory& operator=(const ConcreteAdapterFactory&);

    /**
     * @brief Instantiates a new InterfaceAdapter of the appropriate type.
     * @param xml The configuration XML for the instantiated Adapter.
     * @param execInterface Reference to the parent InterfaceManager instance.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdapter.
     */

    InterfaceAdapterId create(const pugi::xml_node& xml,
                              AdapterExecInterface& execInterface,
                              bool& wasCreated) const
    {
      InterfaceAdapterId result = (new AdapterType(execInterface, xml))->getId();
      wasCreated = true;
      return result;
    }
  };

#define REGISTER_ADAPTER(CLASS,NAME) {new PLEXIL::ConcreteAdapterFactory<CLASS>(*(new PLEXIL::LabelStr(NAME)));}

} // namespace PLEXIL

#endif // ADAPTER_FACTORY_H
