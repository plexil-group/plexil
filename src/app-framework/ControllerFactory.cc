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

#include "ControllerFactory.hh"
#include "Debug.hh"
#include "DynamicLoader.hh"
#include "ExecController.hh"
#include "ExecApplication.hh"
#include "InterfaceSchema.hh"
#include "pugixml.hpp"
#include "XMLUtils.hh"

namespace PLEXIL
{
  /**
   * @brief Creates a new ExecController instance as specified by
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the ExecController constructor.
   * @param execInterface Reference to the parent ExecApplication instance.
   * @return The Id for the new ExecController.  May not be unique.
   */

  ExecControllerId 
  ControllerFactory::createInstance(const pugi::xml_node& xml,
                                    ExecApplication& execInterface)
  {
    // Can't do anything without the spec
    assertTrueMsg(!xml.empty(),
                  "ControllerFactory::createInstance: null configuration XML");

    // Get the kind of controller to make
    const char* controllerType = 
      xml.attribute(InterfaceSchema::CONTROLLER_TYPE_ATTR()).value();
    checkError(*controllerType != '\0',
               "ControllerFactory::createInstance: no "
               << InterfaceSchema::CONTROLLER_TYPE_ATTR()
               << " attribute for controller XML:\n"
               << xml);

    // Make it
    bool dummy;
    return createInstance(LabelStr(controllerType), xml, execInterface, dummy);
  }

  /**
   * @brief Creates a new ExecController instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the ExecController constructor.
   * @param execInterface Reference to the parent ExecApplication instance.
   * @return The Id for the new ExecController.  May not be unique.
   */

  ExecControllerId 
  ControllerFactory::createInstance(const LabelStr& name,
                                    const pugi::xml_node& xml,
                                    ExecApplication& execInterface)
  {
    bool dummy;
    return createInstance(name, xml, execInterface, dummy);
  }


  /**
   * @brief Creates a new ExecController instance with the type associated with the name and
   *        the given configuration XML.
   * @param name The registered name for the factory.
   * @param xml The configuration XML to be passed to the ExecController constructor.
   * @param wasCreated Reference to a boolean variable;
   *                   variable will be set to true if new object created, false otherwise.
   * @return The Id for the new ExecController.  If wasCreated is set to false, is not unique.
   */

  ExecControllerId 
  ControllerFactory::createInstance(const LabelStr& name,
                                    const pugi::xml_node& xml,
                                    ExecApplication& execInterface,
                                    bool& wasCreated)
  {
    std::map<LabelStr, ControllerFactory*>::const_iterator it = factoryMap().find(name);
    if (it == factoryMap().end()) {
      debugMsg("ControllerFactory:createInstance", 
               "Attempting to dynamically load controller type \""
               << name.c_str() << "\"");
      // Attempt to dynamically load library
      const char* libCPath =
        xml.attribute(InterfaceSchema::LIB_PATH_ATTR()).value();
      if (!DynamicLoader::loadModule(name.c_str(), libCPath)) {
        debugMsg("ControllerFactory:createInstance", 
                 " unable to load module for controller type \""
                 << name.c_str() << "\"");
        return ExecControllerId::noId();
      }
      // See if it's registered now
      it = factoryMap().find(name);
    }

    if (it == factoryMap().end()) {
      debugMsg("ControllerFactory:createInstance",
               " No controller factory registered for name \"" << name.c_str() << "\".");
      return ExecControllerId::noId();
    }
    ExecControllerId retval = it->second->create(xml, execInterface, wasCreated);
    debugMsg("ControllerFactory:createInstance", " Created controller " << name.c_str());
    return retval;
  }

  bool ControllerFactory::isRegistered(const LabelStr& name) {
    return factoryMap().find(name) != factoryMap().end();
  }

  std::map<LabelStr, ControllerFactory*>& ControllerFactory::factoryMap() 
  {
    static std::map<LabelStr, ControllerFactory*> sl_map;
    return sl_map;
  }

  /**
   * @brief Deallocate all factories
   */
  void ControllerFactory::purge()
  {
    for (std::map<LabelStr, ControllerFactory*>::iterator it = factoryMap().begin();
         it != factoryMap().end();
         ++it)
      delete it->second;
    factoryMap().clear();
  }

  /**
   * @brief Registers an ControllerFactory with the specific name.
   * @param name The name by which the Controller shall be known.
   * @param factory The ControllerFactory instance.
   */
  void ControllerFactory::registerFactory(const LabelStr& name, ControllerFactory* factory)
  {
    assertTrue(factory != NULL);
    if (factoryMap().find(name) != factoryMap().end())
      {
        warn("Attempted to register an controller factory for name \""
             << name.c_str()
             << "\" twice, ignoring.");
        delete factory;
        return;
      }
    factoryMap()[name] = factory;
    debugMsg("ControllerFactory:registerFactory",
             " Registered controller factory for name \"" << name.c_str() << "\"");
  }

}
