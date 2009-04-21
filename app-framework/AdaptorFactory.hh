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

#ifndef ADAPTOR_FACTORY_H
#define ADAPTOR_FACTORY_H

#include "Id.hh"
#include "LabelStr.hh"
#include <map>

// Forward reference w/o namespace
class TiXmlElement;

namespace PLEXIL
{
  //
  // Forward declarations
  //

  class InterfaceAdaptor;
  typedef Id<InterfaceAdaptor> InterfaceAdaptorId;

  class AdaptorExecInterface;

  /**
   * @brief Factory class for InterfaceAdaptor instances.
   */
  class AdaptorFactory 
  {
  public:

    /**
     * @brief Creates a new InterfaceAdaptor instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param xml The configuration XML to be passed to the InterfaceAdaptor constructor.
     * @return The Id for the new InterfaceAdaptor.  May not be unique.
     */

    static InterfaceAdaptorId createInstance(const LabelStr& name, 
                                             const TiXmlElement* xml,
                                             AdaptorExecInterface& execInterface);

    /**
     * @brief Creates a new InterfaceAdaptor instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param xml The configuration XML to be passed to the InterfaceAdaptor constructor.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdaptor.  If wasCreated is set to false, is not unique.
     */

    static InterfaceAdaptorId createInstance(const LabelStr& name,
                                             const TiXmlElement* xml,
                                             AdaptorExecInterface& execInterface,
                                             bool& wasCreated);

    /**
     * @brief Deallocate all factories
     */
    static void purge();

    const LabelStr& getName() const {return m_name;}

  protected:
    virtual ~AdaptorFactory()
    {}

    /**
     * @brief Registers an AdaptorFactory with the specific name.
     * @param name The name by which the Adaptor shall be known.
     * @param factory The AdaptorFactory instance.
     */
    static void registerFactory(const LabelStr& name, AdaptorFactory* factory);

    /**
     * @brief Instantiates a new InterfaceAdaptor of the appropriate type.
     * @param xml The configuration XML for the instantiated Adaptor.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdaptor.
     */
    virtual InterfaceAdaptorId create(const TiXmlElement* xml,
                                      AdaptorExecInterface& execInterface,
                                      bool& wasCreated) const = 0;

    AdaptorFactory(const LabelStr& name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    AdaptorFactory();
    AdaptorFactory(const AdaptorFactory&);
    AdaptorFactory& operator=(const AdaptorFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete AdaptorFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, AdaptorFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Adaptor type.
   */
  template<class AdaptorType>
  class ConcreteAdaptorFactory : public AdaptorFactory 
  {
  public:
    ConcreteAdaptorFactory(const LabelStr& name)
      : AdaptorFactory(name) 
    {}

  private:
    // Deliberately unimplemented
    ConcreteAdaptorFactory();
    ConcreteAdaptorFactory(const ConcreteAdaptorFactory&);
    ConcreteAdaptorFactory& operator=(const ConcreteAdaptorFactory&);

    /**
     * @brief Instantiates a new InterfaceAdaptor of the appropriate type.
     * @param xml The configuration XML for the instantiated Adaptor.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new InterfaceAdaptor.
     */

    InterfaceAdaptorId create(const TiXmlElement* xml,
                              AdaptorExecInterface& execInterface,
                              bool& wasCreated) const
    {
      InterfaceAdaptorId result = (new AdaptorType(execInterface, xml))->getId();
      wasCreated = true;
      return result;
    }
  };

#define REGISTER_ADAPTOR(CLASS,NAME) {new PLEXIL::ConcreteAdaptorFactory<CLASS>(#NAME);}

} // namespace PLEXIL

#endif // ADAPTOR_FACTORY_H
