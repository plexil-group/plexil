/* Copyright (c) 2006-2018, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_NOTIFIER_IMPL_HH
#define PLEXIL_NOTIFIER_IMPL_HH

#include "Notifier.hh"

namespace PLEXIL
{

  /**
   * @class Propagator
   * @brief Mixin class for expressions whose value may change. Implements expression graph notification.
   */

  //
  // The expression listener graph (really a forest of trees, there are no cycles)
  // is built during plan loading. Its purpose is to tell a node when one of its
  // conditions may have changed, so that it can be considered for a potential node
  // state transition.
  //

  class Propagator :
    public Notifier,
    virtual public ExpressionListener
  {
  public:

    /**
     * @brief Destructor.
     */
    virtual ~Propagator();

    //
    // Core Propagator behavior
    //

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param ptr The pointer to the listener to add.
     * @note Wraps Notifier method.
     */
    virtual void addListener(ExpressionListener *ptr);

    /**
     * @brief Remove a listener from this Expression.
     * @param ptr The pointer to the listener to remove.
     * @note Wraps notifier method.
     */
    virtual void removeListener(ExpressionListener *ptr);

    /**
     * @brief Virtual function for notification that an expression's value has changed.
     */
    virtual void notifyChanged();

  protected:

    /**
     * @brief Default constructor.
     * @note Only available to derived classes.
     */
    Propagator();

    /**
     * @brief Called by notifyChanged() when the expression is active.
     * @note Default method calls publishChange().
     */
    virtual void handleChange();

  private:

    // Not implemented
    Propagator(const Propagator &);
    Propagator &operator=(const Propagator &);

  };

} // namespace PLEXIL

#endif // PLEXIL_NOTIFIER_IMPL_HH
