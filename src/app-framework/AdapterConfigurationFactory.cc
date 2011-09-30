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

#include "AdapterConfigurationFactory.hh"
#include "AdapterConfiguration.hh"
#include "Debug.hh"

namespace PLEXIL
{
  /**
   * @brief Creates a new AdapterConfiguration instance with the type associated with the given name.
   * @param name The registered name for the factory.
   * @return The Id for the new InterfaceAdapter.  May not be unique.
   */

  AdapterConfigurationId
  AdapterConfigurationFactory::createInstance(const LabelStr& name, InterfaceManager* manager)
  {
    bool dummy;
    return createInstance(name, manager, dummy);
  }


  /**
   * @brief Creates a new AdapterConfiguration instance with the type associated with the given name.
   * @param name The registered name for the factory.
   * @param wasCreated Reference to a boolean variable;
   *                   variable will be set to true if new object created, false otherwise.
   * @return The Id for the new InterfaceAdapter.  If wasCreated is set to false, is not unique.
   */

  AdapterConfigurationId
  AdapterConfigurationFactory::createInstance(const LabelStr& name, InterfaceManager* manager,
                                 bool& wasCreated)
  {
    std::map<double, AdapterConfigurationFactory*>::const_iterator it = factoryMap().find(name.getKey());
    assertTrueMsg(it != factoryMap().end(),
		  "Error: No AdapterConfiguration factory registered for name \"" << name.c_str() << "\".");
    AdapterConfigurationId retval = it->second->create(manager, wasCreated);
    debugMsg("AdapterConfigurationFactory:createInstance", " Created adapter " << name.c_str());
    return retval;
  }

  bool AdapterConfigurationFactory::isRegistered(const LabelStr& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

  std::map<double, AdapterConfigurationFactory*>& AdapterConfigurationFactory::factoryMap()
  {
    static std::map<double, AdapterConfigurationFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void AdapterConfigurationFactory::purge()
  {
    for (std::map<double, AdapterConfigurationFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an AdapterConfigurationFactory with the specific name.
   * @param name The name by which the Adapter shall be known.
   * @param factory The AdapterConfigurationFactory instance.
   */
  void AdapterConfigurationFactory::registerFactory(const LabelStr& name, AdapterConfigurationFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name.getKey()) != factoryMap().end())
      {
	warn("Attempted to register an adapter factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name.getKey()] = factory;
    debugMsg("AdapterConfigurationFactory:registerFactory",
             " Registered adapter factory for name \"" << name.c_str() << "\"");
  }

}
