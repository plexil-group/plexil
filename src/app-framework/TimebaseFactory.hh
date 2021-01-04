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

#ifndef PLEXIL_TIMEBASE_FACTORY_HH
#define PLEXIL_TIMEBASE_FACTORY_HH

#include "Timebase.hh"

#include "pugixml.hpp"

#include <map>
#include <memory>
#include <string>

//
// Abstract factory API for Timebase instances
//

namespace PLEXIL
{

  //!
  // @class TimebaseFactory
  //
  class TimebaseFactory
  {

  public:
    virtual ~TimebaseFactory() = default;

    //!
    // @brief Construct the specified Timebase instance.
    // @param xml The XML describing the desired Timebase (may be empty).
    // @param fn The function to be called on timer timeouts.
    // @param arg The argument to pass to the timer timeout function.
    // @return A newly constructed Timebase instance.
    //
    static Timebase *makeTimebase(pugi::xml_node const xml,
                                  WakeupFn fn,
                                  void *arg = 0);
    
  protected:
    TimebaseFactory(std::string const &name);

    virtual Timebase *create(pugi::xml_node const xml,
                             WakeupFn fn,
                             void *arg) const = 0;

  private:

    static TimebaseFactory *get(std::string const &name);

    using TimebaseFactoryPtr = std::unique_ptr<TimebaseFactory>;
    static std::map<std::string, TimebaseFactoryPtr> &factoryMap();

    std::string const m_name;
  };

  template <class TimebaseType>
  class ConcreteTimebaseFactory : public TimebaseFactory
  {
  public:
    ConcreteTimebaseFactory(std::string const &name)
      : TimebaseFactory(name)
    {
    }

    virtual ~ConcreteTimebaseFactory() = default;

  private:

    virtual Timebase *create(pugi::xml_node const xml,
                             WakeupFn fn,
                             void *arg) const override
    {
      return new TimebaseType(xml, fn, arg);
    }
      
  };

} // namespace PLEXIL

//!
// @brief Macro to define and construct factory instances
//
#define REGISTER_TIMEBASE(CLASS,NAME) {new PLEXIL::ConcreteTimebaseFactory<CLASS>(NAME);}

#endif // PLEXIL_TIMEBASE_FACTORY_HH
