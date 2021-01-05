/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#include "TimebaseFactory.hh"

#include "Debug.hh"
#include "Error.hh"
#include "InterfaceError.hh"
#include "InterfaceSchema.hh"

#include "pugixml.hpp"

#if defined(HAVE_CLIMITS)
#include <climits>   // INT_MIN
#elif defined(HAVE_LIMITS_H)
#include <limits.h> // INT_MIN
#endif

namespace PLEXIL
{

  Timebase *makeTimebase(pugi::xml_node const descriptor,
                         WakeupFn fn,
                         void *arg)
  {
    TimebaseFactory const *factory = nullptr;
    if (descriptor) {
      const char *typnam = descriptor.attribute(InterfaceSchema::TYPE_ATTR).value();
      if (typnam && *typnam) {
        factory = TimebaseFactory::get(typnam);
        checkInterfaceError(factory,
                            "makeTimebase: no factory for \"" << typnam << '"');
        debugMsg("makeTimebase", " found factory \"" << factory->name() << '"');
      }
    }
    if (!factory) {
      // No type specified - pick the best available factory
      factory = TimebaseFactory::getBest();
      checkInterfaceError(factory, "makeTimebase: no known timebases!");
      debugMsg("TimebaseFactory::makeTimebase",
               " got best factory \"" << factory->name() << '"');
    }
    return factory->create(fn, arg);
  }

  TimebaseFactory const *TimebaseFactory::get(std::string const &name)
  {
    TimebaseFactoryMap::const_iterator it = factoryMap().find(name);
    if (it == factoryMap().end()) {
      debugMsg("TimebaseFactory", " no factory for  \"" << name << '"');
      return nullptr;
    }
    return it->second.get();
  }

  TimebaseFactory const *TimebaseFactory::getBest()
  {
    if (factoryMap().empty()) {
      debugMsg("TimebaseFactory:getBest", " no factories found");
      return nullptr;
    }

    // Simple linear search for the highest priority
    int bestPriority = INT_MIN;
    TimebaseFactory const *bestFactory = nullptr;
    for (TimebaseFactoryMap::value_type const &payr : factoryMap()) {
      if (payr.second->priority() > bestPriority) {
        bestFactory = payr.second.get();
        bestPriority = bestFactory->priority();
      }
    }
    debugMsg("TimebaseFactory:getBest",
             "returning factory " << bestFactory->name()
             << " at priority " << bestPriority);
    return bestFactory;
  }

  std::vector<std::string> TimebaseFactory::allFactoryNames()
  {
    std::vector<std::string> result;
    for (TimebaseFactoryMap::value_type const &payr : factoryMap()) {
      result.push_back(payr.second->name());
    }
    return result;
  }

  std::string const &TimebaseFactory::name() const
  {
    return m_name;
  }

  int TimebaseFactory::priority() const
  {
    return m_priority;
  }

  TimebaseFactory::TimebaseFactory(std::string const &name, int priority)
    : m_name(name),
      m_priority(priority)
  {
    factoryMap()[name].reset(this);
  }

  TimebaseFactory::TimebaseFactoryMap &TimebaseFactory::factoryMap()
  {
    static TimebaseFactoryMap sl_map;
    return sl_map;
  }

} // namespace PLEXIL
