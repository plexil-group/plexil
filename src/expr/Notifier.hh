/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NOTIFIER_HH
#define PLEXIL_NOTIFIER_HH

#include "plexil-config.h"

#include "Listenable.hh"

#include <vector>

#if defined(HAVE_CSTDDEF)
#include <cstddef> // size_t
#elif defined(HAVE_STDDEF_H)
#include <stddef.h> // size_t
#endif

namespace PLEXIL
{

  /**
   * @class Notifier
   * @brief Abstract base class for anything that notifies ExpressionListeners
   */
  class Notifier : virtual public Listenable
  {
  public:

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param ptr The pointer to the listener to add.
     */
    virtual void addListener(ExpressionListener *ptr);

    /**
     * @brief Remove a listener from this Expression.
     * @param ptr The pointer to the listener to remove.
     */
    virtual void removeListener(ExpressionListener *ptr);

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     */
    virtual void activate();

    /**
     * @brief Make this Expression inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     */
    virtual void deactivate();

    /**
     * @brief Determine whether this object is active (i.e. propagating change notifications).
     * @return true if active, false if not.
     */
    virtual bool isActive() const;

    /**
     * @brief Notify all listeners that this expression's value has changed.
     */
    virtual void publishChange();

  protected:

    Notifier();
    virtual ~Notifier();

    //
    // Member functions which derived classes may call
    //

    /**
     * @brief Report whether the expression has listeners.
     * @return True if present, false if not.
     */
    bool hasListeners() const;

    //
    // Member functions which derived classes may implement
    //

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     * @note Default method does nothing.
     */
    virtual void handleActivate();

    /**
     * @brief Make this expression inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     * @note Default method does nothing.
     */
    virtual void handleDeactivate();

  private:

    // Essential member variables
    size_t m_activeCount; // align to word size
    std::vector<ExpressionListener *> m_outgoingListeners; /*<! For outgoing message notifications (this expression's value has changed) */

#ifdef RECORD_EXPRESSION_STATS
    Notifier *m_prev; // pointer to newer instance
    Notifier *m_next; // pointer to older instance

    static Notifier *s_instanceList;
#endif

  };

}

#endif // PLEXIL_NOTIFIER_HH
