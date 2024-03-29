// Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NOTIFIER_IMPL_HH
#define PLEXIL_NOTIFIER_IMPL_HH

#include "Notifier.hh"

namespace PLEXIL
{

  //! \class Propagator
  //! \brief Mixin class for expressions whose value may change in
  //!        response to changes in subexpressions.

  //! The %PLEXIL expression change notification graph (really a
  //! forest of trees, there are no cycles) is built during plan
  //! loading. Its purpose is to tell a node when one of its
  //! conditions may have changed, so that it can be considered for a
  //! potential node state transition.

  //! Propagator implements the behavior expected of an interior node
  //! in the graph.  When its notifyChanged method is called, it calls
  //! a protected member function, handleChange.  The default method
  //! for handleChange calls Notifier's publishChange virtual member
  //! function.  Derived classes which implement a handleChange method
  //! should also call the Propagator method explicitly.

  //! By default, expressions derived from Propagator are not expected
  //! to generate changes independently of their subexpressions.
  //! Expressions which can generate changes independently
  //! (e.g. Lookups, random number generators) should override the
  //! virtual function Listenable::isPropagationSource with a method
  //! which returns true, and their methods should call the
  //! publishChange member function as appropriate.

  //! \see ExpressionListener
  //! \see Listenable::isPropagationSource
  //! \see Notifier
  //! \ingroup Expressions

  class Propagator :
    public Notifier,
    virtual public ExpressionListener
  {
  public:

    //! \brief Virtual destructor.
    virtual ~Propagator() = default;

    //
    // Core Propagator behavior
    //

    //! \brief Add a change listener to this object.
    //! \param ptr Pointer to the listener.
    //! \note Wraps Notifier method.
    virtual void addListener(ExpressionListener *ptr);

    //! \brief Remove a change listener from this object.
    //! \param ptr Pointer to the listener to remove.
    //! \note Wraps Notifier method.
    virtual void removeListener(ExpressionListener *ptr);

    //! \brief Notify this object of a change.
    virtual void notifyChanged();

  protected:

    //! \brief Default constructor.
    //! \note Protected; only available to derived classes.
    Propagator();

    //! \brief Perform whatever action is necessary when a change
    //!        notification is received.
    //! \note Called by notifyChanged when the expression is active.
    //! \note The default method calls the Notifier::publishChange
    //!       member function.
    //! \note Derived classes overriding the handleChange method
    //!       should explicitly call the Propagator method.
    //! \see Notifier::publishChange
    virtual void handleChange();

  private:

    // Not implemented
    Propagator(Propagator const &) = delete;
    Propagator(Propagator &&) = delete;
    Propagator &operator=(Propagator const &) = delete;
    Propagator &operator=(Propagator &&) = delete;

  };

} // namespace PLEXIL

#endif // PLEXIL_NOTIFIER_IMPL_HH
