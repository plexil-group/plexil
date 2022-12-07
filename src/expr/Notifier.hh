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

#ifndef PLEXIL_NOTIFIER_HH
#define PLEXIL_NOTIFIER_HH

#include "Listenable.hh"

#include <cstddef> // size_t
#include <vector>

namespace PLEXIL
{

  //! \class Notifier
  //! \brief Abstract base class for objects which publish changes to listeners.

  //! Notifier implements the essential functionality and state for
  //! publishing change notifications.  Each Notifier instance
  //! maintains a list of objects (listeners) which have requested
  //! change notifications.  Each instance keeps track of whether it
  //! is active (i.e. publishing change notifications) or not, and can
  //! notify derived classes when the object transitions between
  //! active and inactive states.  When active, the publishChange()
  //! member function will call the notifyChanged() member function on
  //! each listener in its list.

  //! Each Notifier instance maintains an activation count,
  //! initialized to 0 (inactive).  When the activate() method is
  //! called, the count is incremented; if the count was 0 prior to
  //! the call, the instance transitions to active state, and calls
  //! the handleActivate() member function.  When the deactivate()
  //! method is called, the count is decremented; if the count is 0
  //! after the call, the instance transitions to inactive state, and
  //! calls the handleDeactivate() member function.  The activation
  //! count may not go below 0, nor wrap around; if either occurs, the
  //! program halts with an assertion failure.

  //! For profiling and debugging purposes, if this class is compiled
  //! with the RECORD_EXPRESSION_STATS macro defined, the class
  //! maintains a list of all its instances.  This allows a program to
  //! iterate over the list to generate statistics, inspect each
  //! instance, etc.  Normally RECORD_EXPRESSION_STATS is *not*
  //! defined.

  //! \ingroup Expressions

  class Notifier : virtual public Listenable
  {
  public:

    //! \brief Virtual destructor.
    //! \note All listeners must be removed before calling the destructor.
    virtual ~Notifier();

    //! \brief Add a listener.
    //! \param ptr The pointer to the listener to add.
    virtual void addListener(ExpressionListener *ptr) override;

    //! \brief Remove a listener.
    //! \param ptr The pointer to the listener to remove.
    virtual void removeListener(ExpressionListener *ptr) override;

    //! \brief Make this object active if it is not already.
    //! \note Notifier publishes change notifications when active.
    virtual void activate() override;

    //! \brief Request that this object become inactive if it is not already.
    //! \note Notifier does not publish change notifications when inactive.
    virtual void deactivate() override;

    //! \brief Query whether this object is active (i.e. publishing change notifications).
    //! \return true if active, false if not.
    virtual bool isActive() const override;

    //! \brief If active, notify all listeners of a change.  If inactive, do nothing.
    virtual void publishChange();

#ifdef RECORD_EXPRESSION_STATS
    //! \brief Get a linked list of all Notifier instances.
    //! \return The list.
    static Notifier const *getInstanceList();

    //! \brief Get a pointer to the next Notifier in the linked list.
    //! \return Pointer to the next instance.  May be null.
    Notifier const *next() const;

    //! \brief Get the number of listeners registered on this Notifier.
    //! \return The count.
    size_t Notifier::getListenerCount() const
#endif

  protected:

    //! \brief Default constructor.  Only accessible to derived classes.
    Notifier();

    //
    // Member functions which derived classes may call
    //

    //! \brief Report whether the expression has listeners.
    //! \return True if present, false if not.
    bool hasListeners() const;

    //
    // Member functions which derived classes may implement
    //

    //! \brief Perform any necessary actions to enter the active state.
    //! \note The default method does nothing.
    virtual void handleActivate();

    //! \brief Perform any necessary actions to enter the inactive state.
    //! \note The default method does nothing.
    virtual void handleDeactivate();

  private:

    // Deliberately unimplemented.
    Notifier(Notifier const &) = delete;
    Notifier(Notifier &&) = delete;
    Notifier &operator=(Notifier const &) = delete;
    Notifier &operator=(Notifier &&) = delete;

    //
    // Essential member variables
    //

    //! \brief The number of times the activate method has been
    //!        called, minus the number of times the deactivate method
    //!        has been called.  Initialized to 0.
    //! \see Notifier::activate
    //! \see Notifier::deactivate
    size_t m_activeCount;

    //! \brief Listeners to this object.
    std::vector<ExpressionListener *> m_outgoingListeners;

#ifdef RECORD_EXPRESSION_STATS
    //! \brief Pointer to a newer Notifier instance.  Initialized to nullptr.
    Notifier *m_prev;

    //! \brief Pointer to an older Notifier instance.
    Notifier *m_next;

    //! \brief Pointer to the most recently constructed Notifier instance.
    static Notifier *s_instanceList;
#endif

  };

}

#endif // PLEXIL_NOTIFIER_HH
