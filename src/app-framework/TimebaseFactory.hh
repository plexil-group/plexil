// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
//  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
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

#ifndef PLEXIL_TIMEBASE_FACTORY_HH
#define PLEXIL_TIMEBASE_FACTORY_HH

#include "Timebase.hh"

#include <map>
#include <memory>
#include <string>
#include <vector>

//
// Abstract factory for Timebase instances
//

//
// Forward references
//

namespace pugi
{
  class xml_node;
}

namespace PLEXIL
{

  //! User function to construct a Timebase instance.
  //! @param descriptor XML element describing the desired Timebase; may be empty.
  //! @param fn Pointer to the function to call on a timer timeout.
  //! @param arg Argument value passed to the timeout function.
  Timebase *makeTimebase(pugi::xml_node const descriptor,
                         Timebase::WakeupFn const &fn);

  //! \class TimebaseFactory
  //! Abstract base class for an abstract factory for Timebase instances.

  //! Public static member functions get() and getBest() are used to
  //! select a particular factory instance. Public member accessors
  //! name() and priority() help inform the selection process.

  //! Public virtual member function create() is called on the
  //! selected factory to construct the desired Timebase instance.

  //! The map of available factories is only accessible by the base
  //! class. Factories are added to the map when the concrete factory
  //! class is instantiated.

  class TimebaseFactory
  {
  public:

    //! Get the TimebaseFactory instance for a particular name.
    //! \param name Name of the desired TimebaseFactory.
    //! \return Const pointer to the named TimebaseFactory instance;
    //!         null if not found.
    static TimebaseFactory const *get(std::string const &name);

    //! Get the highest priority TimebaseFactory instance.
    //! \return Const pointer to the named TimebaseFactory instance;
    //!         null if no factories are known.
    static TimebaseFactory const *getBest();

    //! Get the names of all known TimebaseFactories.
    //! \return Vector of name strings.
    //! \note Facilitates unit testing.
    static std::vector<std::string> allFactoryNames();

    //! Construct a Timebase of the type assigned to this factory.
    //! \param fn The wakeup function.
    //! \return Pointer to a new Timebase instance.
    virtual Timebase *create(Timebase::WakeupFn const &fn) const = 0;

    //! Get the name assigned to this factory instance.
    //! \return Const reference to the name string.
    std::string const &name() const;

    //! Get the priority assigned to this factory instance.
    //! \return The priority as an int.
    int priority() const;

    //! Virtual destructor.
    virtual ~TimebaseFactory() = default;
    
  protected:

    //! Base class constructor, only accessible by derived classes.
    //! \param name The name to give this factory.
    //! \param priority The priority of this factory. Priority is used
    //!                 when no timebase name is specified, and multiple
    //!                 factories are available.
    TimebaseFactory(std::string const &name, int priority);

  private:

    //! Typedef for pointers in the factory map.
    using TimebaseFactoryPtr = std::unique_ptr<TimebaseFactory>;

    //! Typedef for the factory map.
    using TimebaseFactoryMap = std::map<std::string, TimebaseFactoryPtr>;

    //! Instance accessor for the name -> factory map.
    //! \return Reference to the factory map.
    static TimebaseFactoryMap &factoryMap();

    //! Delete the factories in the factory map at program exit.
    static void purge();

    //! Name of this factory instance. 
    std::string const m_name;

    //! The priority of this factory instance.
    //! The priority is used to select a factory when there are
    //! multiple factories defined and no name has been specified.
    int const m_priority;
  };

  //! \class ConcreteTimebaseFactory
  //! \brief Templated factory class to construct one particular class
  //! of Timebase.

  template <class TimebaseType>
  class ConcreteTimebaseFactory : public TimebaseFactory
  {
  public:

    //! \brief Constructor for the specialized factory class.
    //! \param name The registered name of this factory.
    //! \param priority The priority of this factory. Priority is used
    //!                 when no timebase name is specified, and multiple
    //!                 factories are available.
    ConcreteTimebaseFactory(std::string const &name, int priority)
      : TimebaseFactory(name, priority)
    {
    }

    //! Virtual destructor.
    virtual ~ConcreteTimebaseFactory() = default;

    //! Construct an instance of TimebaseType with the given function
    //! and argument.
    //! \param fn Pointer to a wakeup function.
    //! \param arg The argument to pass to the wakeup function.
    //! \return Pointer to a new instance of TimebaseType.
    virtual Timebase *create(Timebase::WakeupFn const &fn) const
    {
      return new TimebaseType(fn);
    }
      
  };

} // namespace PLEXIL

//! Macro to define and construct factory instances
//! \param CLASS Name of the class the factory will construct.
//! \param NAME Name used to look up the factory. Should be a string constant.
//! \param PRIO The priority to give the factory. Should be an int constant.
#define REGISTER_TIMEBASE(CLASS,NAME,PRIO) \
  {new PLEXIL::ConcreteTimebaseFactory<CLASS>(NAME, PRIO);}

#endif // PLEXIL_TIMEBASE_FACTORY_HH
