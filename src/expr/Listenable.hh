// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
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

#ifndef PLEXIL_LISTENABLE_HH
#define PLEXIL_LISTENABLE_HH

#include "ExpressionListener.hh"

#include <functional>

namespace PLEXIL
{

  // Forward reference; class is defined below.
  class Listenable;

  //! \typedef ListenableUnaryOperator
  //! \brief An abstraction for a helper function used by Propagator.
  //! \see Listenable::doSubexprs
  //! \ingroup Expressions
  using ListenableUnaryOperator = std::function<void(Listenable *)>;

  //! \class Listenable
  //! \brief Stateless base class defining the API for objects to which an
  //!        ExpressionListener instance may be attached.
  //!
  //! The Listenable base class defines the API for change
  //! notification sources in the %PLEXIL Executive expression
  //! subsystem.  Listenable itself has no state.  Its member
  //! functions can all be overridden by derived classes.  The default
  //! methods on the base class do nothing.
  //!
  //! Expression implementations whose values cannot change can derive
  //! from Listenable directly.  Expressions whose values can change
  //! should derive from the Notifier class.  Expressions whose values
  //! are dependent upon other expressions should derive from the
  //! Propagator class.
  //!
  //! \see ExpressionListener
  //! \see Notifier
  //! \see Propagator
  //! \ingroup Expressions
  class Listenable
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Listenable() = default;

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    //! \note The default method does nothing.
    virtual void addListener(ExpressionListener *ptr)
    {
    }

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    //! \note The default method does nothing.
    virtual void removeListener(ExpressionListener *ptr)
    {
    }

    //! \brief Make this object active if it is not already.
    //! \note The default method does nothing.
    //! \see Notifier::activate
    virtual void activate()
    {
    }

    //! \brief Request that this object become inactive if it is not already.
    //! \note The default method does nothing.
    //! \see Notifier::deactivate
    virtual void deactivate()
    {
    }

    //! \brief Query whether this object is active.
    //! \return true if active, false if not.
    //! \note The default method returns true.
    virtual bool isActive() const
    {
      return true;
    }

    //
    // Member functions which derived classes may override
    // Mostly support for listener network setup and teardown
    //

    //! \brief Call a function on all subexpressions of this object.

    //! The doSubexprs method is essential to setting up and tearing
    //! down the change notification network.  It is used to implement
    //! a recursive depth-first traversal of the expression graph.

    //! Methods on derived classes must call the oper argument on
    //! every subexpression.

    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    //! \note The default method does nothing.

    //! \see ListenableUnaryOperator
    //! \see Listenable::isPropagationSource
    virtual void doSubexprs(ListenableUnaryOperator const &oper)
    {
    }

    //! \brief Query whether an object can generate its own change
    //!        notifications independent of other objects in the
    //!        notification graph.

    //! This member function helps minimize the count of nodes and
    //! arcs in the expression change notification network.  If an
    //! expression cannot create its own notifications, it can be
    //! bypassed, in favor of its inputs (subexpressions).  In this
    //! way the depth of the notification network can be minimized;
    //! the ultimate listeners can connect directly to the ultimate
    //! sources.

    //! Methods for this member function should generally return true
    //! for leaf nodes in the expression network which are not
    //! constant (e.g. variables); however, some interior nodes
    //! (e.g. Lookups, random number generators) may also
    //! generate changes of their own accord.

    //! \return True if the object can change of its own accord, false if not.
    //! \note The default method returns false.
    virtual bool isPropagationSource() const
    {
      return false;
    }

  };

}

#endif // PLEXIL_LISTENABLE_HH
