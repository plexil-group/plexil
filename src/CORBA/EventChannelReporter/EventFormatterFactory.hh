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

#ifndef EVENT_FORMATTER_FACTORY_H
#define EVENT_FORMATTER_FACTORY_H

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

  class EventFormatter;
  typedef Id<EventFormatter> EventFormatterId;
  class StructuredEventFormatter;
  typedef Id<StructuredEventFormatter> StructuredEventFormatterId;

  /**
   * @brief Factory class for EventFormatter instances.
   *        Implements the AbstractFactory design pattern.
   */
  class EventFormatterFactory 
  {
  public:

    /**
     * @brief Creates a new EventFormatter instance with the type associated with the name and
     *        the given configuration XML.
     * @param xml The configuration XML specifying the EventFormatter.
     * @return The Id for the new EventFormatter.
     */

    static EventFormatterId createInstance(const TiXmlElement* xml);

    /**
     * @brief Checks whether or not the given EventFormatterFactory is registered.
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

    virtual ~EventFormatterFactory()
    {}

    /**
     * @brief Registers an EventFormatterFactory with the specific name.
     * @param name The name by which the formatter shall be known.
     * @param factory The EventFormatterFactory instance.
     */
    static void registerFactory(const LabelStr& name, EventFormatterFactory* factory);

    /**
     * @brief Instantiates a new EventFormatter of the appropriate type.
     * @param xml The configuration XML for the instantiated formatter.
     * @return The Id for the new EventFormatter.
     */
    virtual EventFormatterId create(const TiXmlElement* xml) const = 0;

    EventFormatterFactory(const LabelStr& name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    EventFormatterFactory();
    EventFormatterFactory(const EventFormatterFactory&);
    EventFormatterFactory& operator=(const EventFormatterFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete EventFormatterFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, EventFormatterFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each formatter type.
   */
  template<class FormatterType>
  class ConcreteEventFormatterFactory : public EventFormatterFactory 
  {
  public:
    ConcreteEventFormatterFactory(const LabelStr& name)
      : EventFormatterFactory(name) 
    {}

  private:
    // Deliberately unimplemented
    ConcreteEventFormatterFactory();
    ConcreteEventFormatterFactory(const ConcreteEventFormatterFactory&);
    ConcreteEventFormatterFactory& operator=(const ConcreteEventFormatterFactory&);

    /**
     * @brief Instantiates a new EventFormatter of the appropriate type.
     * @param xml The configuration XML for the instantiated formatter.
     * @return The Id for the new EventFormatter.
     */

    EventFormatterId create(const TiXmlElement* xml) const
    {
      EventFormatterId result = (new FormatterType(xml))->getId();
      return result;
    }
  };

  /**
   * @brief Factory class for StructuredEventFormatter instances.
   *        Implements the AbstractFactory design pattern.
   */
  class StructuredEventFormatterFactory 
  {
  public:

    /**
     * @brief Creates a new StructuredEventFormatter instance with the type associated with the name and
     *        the given configuration XML.
     * @param xml The configuration XML specifying the StructuredEventFormatter.
     * @return The Id for the new StructuredEventFormatter.
     */

    static StructuredEventFormatterId createInstance(const TiXmlElement* xml);

    /**
     * @brief Checks whether or not the given StructuredEventFormatterFactory is registered.
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

    virtual ~StructuredEventFormatterFactory()
    {}

    /**
     * @brief Registers an StructuredEventFormatterFactory with the specific name.
     * @param name The name by which the formatter shall be known.
     * @param factory The StructuredEventFormatterFactory instance.
     */
    static void registerFactory(const LabelStr& name, StructuredEventFormatterFactory* factory);

    /**
     * @brief Instantiates a new StructuredEventFormatter of the appropriate type.
     * @param xml The configuration XML for the instantiated formatter.
     * @return The Id for the new StructuredEventFormatter.
     */
    virtual StructuredEventFormatterId create(const TiXmlElement* xml) const = 0;

    StructuredEventFormatterFactory(const LabelStr& name)
      : m_name(name)
    {
      registerFactory(m_name, this);
    }

  private:
    // Deliberately unimplemented
    StructuredEventFormatterFactory();
    StructuredEventFormatterFactory(const StructuredEventFormatterFactory&);
    StructuredEventFormatterFactory& operator=(const StructuredEventFormatterFactory&);

    /**
     * @brief The map from names (LabelStr/double) to concrete StructuredEventFormatterFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, StructuredEventFormatterFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each formatter type.
   */
  template<class FormatterType>
  class ConcreteStructuredEventFormatterFactory : public StructuredEventFormatterFactory 
  {
  public:
    ConcreteStructuredEventFormatterFactory(const LabelStr& name)
      : StructuredEventFormatterFactory(name) 
    {}

  private:
    // Deliberately unimplemented
    ConcreteStructuredEventFormatterFactory();
    ConcreteStructuredEventFormatterFactory(const ConcreteStructuredEventFormatterFactory&);
    ConcreteStructuredEventFormatterFactory& operator=(const ConcreteStructuredEventFormatterFactory&);

    /**
     * @brief Instantiates a new StructuredEventFormatter of the appropriate type.
     * @param xml The configuration XML for the instantiated formatter.
     * @return The Id for the new StructuredEventFormatter.
     */

    StructuredEventFormatterId create(const TiXmlElement* xml) const
    {
      StructuredEventFormatterId result = (new FormatterType(xml))->getId();
      return result;
    }
  };

#define REGISTER_EVENT_FORMATTER(CLASS,NAME) {new PLEXIL::ConcreteEventFormatterFactory<CLASS>(PLEXIL::LabelStr(NAME));}

#define REGISTER_STRUCTURED_EVENT_FORMATTER(CLASS,NAME) {new PLEXIL::ConcreteStructuredEventFormatterFactory<CLASS>(PLEXIL::LabelStr(NAME));}

} // namespace PLEXIL

#endif // EVENT_FORMATTER_FACTORY_H
