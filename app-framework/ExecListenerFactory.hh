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

#ifndef EXEC_LISTENER_FACTORY_H
#define EXEC_LISTENER_FACTORY_H

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

  class ExecListener;
  typedef Id<ExecListener> ExecListenerId;

  /**
   * @brief Factory class for ExecListener instances.
   */
  class ExecListenerFactory 
  {
  public:

    /**
     * @brief Creates a new ExecListener instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param xml The configuration XML to be passed to the ExecListener constructor.
     * @return The Id for the new ExecListener.
     */

    static ExecListenerId createInstance(const LabelStr& name, 
                                         const TiXmlElement* xml);

    /**
     * @brief Deallocate all factories
     */
    static void purge();

    const LabelStr& getName() const {return m_name;}

  protected:

    virtual ~ExecListenerFactory()
    {}

    /**
     * @brief Registers an ExecListenerFactory with the specific name.
     * @param name The name by which the Adaptor shall be known.
     * @param factory The ExecListenerFactory instance.
     */
    static void registerFactory(const LabelStr& name, ExecListenerFactory* factory);

    /**
     * @brief Instantiates a new ExecListener of the appropriate type.
     * @param xml The configuration XML for the instantiated Adaptor.
     * @return The Id for the new ExecListener.
     */
    virtual ExecListenerId create(const TiXmlElement* xml) const = 0;

    ExecListenerFactory(const LabelStr& name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    ExecListenerFactory();
    ExecListenerFactory(const ExecListenerFactory&);
    ExecListenerFactory& operator=(const ExecListenerFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete ExecListenerFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, ExecListenerFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Adaptor type.
   */
  template<class AdaptorType>
  class ConcreteExecListenerFactory : public ExecListenerFactory 
  {
  public:
    ConcreteExecListenerFactory(const LabelStr& name)
      : ExecListenerFactory(name) 
    {}

  private:
    // Deliberately unimplemented
    ConcreteExecListenerFactory();
    ConcreteExecListenerFactory(const ConcreteExecListenerFactory&);
    ConcreteExecListenerFactory& operator=(const ConcreteExecListenerFactory&);

    /**
     * @brief Instantiates a new ExecListener of the appropriate type.
     * @param xml The configuration XML for the instantiated Adaptor.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new ExecListener.
     */

    ExecListenerId create(const TiXmlElement* xml) const
    {
      ExecListenerId result = (new AdaptorType(xml))->getId();
      return result;
    }
  };

#define REGISTER_EXEC_LISTENER(CLASS,NAME) {new PLEXIL::ConcreteExecListenerFactory<CLASS>(#NAME);}

} // namespace PLEXIL

#endif // EXEC_LISTENER_FACTORY_H
