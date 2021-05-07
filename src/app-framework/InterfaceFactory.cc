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

#include "Debug.hh"            // debugMsg() macro
#include "DynamicLoader.h"     // dynamicLoadModule
#include "Error.hh"            // warn() macro
#include "InterfaceSchema.hh"  // LIB_PATH_ATTR

#include "pugixml.hpp"

namespace PLEXIL
{
  //
  // Generic helper functions for InterfaceFactory class template
  //

  const char *checkInterfaceXml(pugi::xml_node const xml, const char *attrName)
  {
    if (!xml) {
      warn("InterfaceFactory::createInstance: null configuration XML");
      return nullptr;
    }
    const char* derivedName = xml.attribute(attrName).value();
    if (!*derivedName) {
      warn("InterfaceFactory: missing required attribute \"" << attrName
           << "\" in XML:\n" << xml);
      return nullptr;
    }
    debugMsg("InterfaceFactory:createInstance", " xml = " << xml);
    return derivedName;
  }

  bool tryLoadSharedLib(const char *name, const pugi::xml_node xml)
  {
    debugMsg("InterfaceFactory:createInstance", 
             " Attempting to dynamically load library \"" << name << "\"");
    // Attempt to dynamically load library
    const char* libPath =
      xml.attribute(InterfaceSchema::LIB_PATH_ATTR).value();
    if (dynamicLoadModule(name, libPath))
      return true;
    warn("constructInterfaces: unable to load library \"" << name << "\"");
    return false;
  }

} // namespace PLEXIL
