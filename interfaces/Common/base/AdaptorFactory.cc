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

#include "AdaptorFactory.hh"
#include "InterfaceAdaptor.hh"
#include "tinyxml.h"

namespace PLEXIL
{
  /**
   * @brief Creates a new InterfaceAdaptor instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the InterfaceAdaptor constructor.
   * @return The Id for the new InterfaceAdaptor.  May not be unique.
   */

  InterfaceAdaptorId 
  AdaptorFactory::createInstance(const LabelStr& name,
                                 const TiXmlElement* xml)
  {
    bool dummy;
    return createInstance(name, xml, dummy);
  }


  /**
   * @brief Creates a new InterfaceAdaptor instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the InterfaceAdaptor constructor.
   * @param wasCreated Reference to a boolean variable;
   *                   variable will be set to true if new object created, false otherwise.
   * @return The Id for the new InterfaceAdaptor.  If wasCreated is set to false, is not unique.
   */

  InterfaceAdaptorId 
  AdaptorFactory::createInstance(const LabelStr& name,
                                 const TiXmlElement* xml,
                                 bool& wasCreated)
  {
    std::map<double, AdaptorFactory*>::const_iterator it = factoryMap().find(name);
    checkError(it != factoryMap().end(),
               "Error: No adaptor factory registered for name '" << name.toString() << "'.");
    InterfaceAdaptorId retval = it->second->create(xml, wasCreated);
    debugMsg("AdaptorFactory:createInstance", " Created " << name.toString());
    return retval;
  }

  std::map<double, AdaptorFactory*>& AdaptorFactory::factoryMap() 
  {
    static std::map<double, AdaptorFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void AdaptorFactory::purge()
  {
    for (std::map<double, AdaptorFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an AdaptorFactory with the specific name.
   * @param name The name by which the Adaptor shall be known.
   * @param factory The AdaptorFactory instance.
   */
  void AdaptorFactory::registerFactory(const LabelStr& name, AdaptorFactory* factory)
  {
    check_error(factory != NULL);
    checkError(factoryMap().find(name) == factoryMap().end(),
               "Error:  Attempted to register an adaptor factory for name \""
               << name.toString() <<
               "\" twice.");
    factoryMap()[name] = factory;
    debugMsg("AdaptorFactory:registerFactory",
             " Registered adaptor factory for name '" << name.toString() << "'");
  }

}
