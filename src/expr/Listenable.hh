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

#ifndef PLEXIL_LISTENABLE_HH
#define PLEXIL_LISTENABLE_HH

#include "ExpressionListener.hh"

namespace PLEXIL
{

  // Forward reference
  class Listenable;

  /**
   * @class ListenableUnaryOperator
   * @brief An abstract base class for a helper function used by Propagator.
   */

  class ListenableUnaryOperator
  {
  public:
    virtual void operator()(Listenable *) const = 0;

    ListenableUnaryOperator() {}
    virtual ~ListenableUnaryOperator() {}
  };

  /**
   * @class Listenable
   * @brief Defines the API for objects to which an ExpressionListener may wish to listen.
   * @note The base class methods do nothing. 
   * @see Notifier
   * @see Propagator
   */

  class Listenable
  {
  public:

    virtual ~Listenable()
    {
    }

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param ptr The pointer to the listener to add.
     * @note This default method does nothing.
     */
    virtual void addListener(ExpressionListener * /* ptr */)
    {
    }

    /**
     * @brief Remove a listener from this Expression.
     * @param ptr The pointer to the listener to remove.
     * @note This default method does nothing.
     */
    virtual void removeListener(ExpressionListener * /* ptr */)
    {
    }

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     * @note This default method does nothing.
     */
    virtual void activate()
    {
    }

    /**
     * @brief Make this Expression inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     * @note This default method does nothing.
     */
    virtual void deactivate()
    {
    }

    /**
     * @brief Determine whether this object is active.
     * @return true if active, false if not.
     * @note This default method returns true.
     */
    virtual bool isActive() const
    {
      return true;
    }

    //
    // Member functions which derived classes may override
    // Mostly support for listener network setup and teardown
    //

    /**
     * @brief Determines whether a Listenable can generate its own change notifications.
     * @note This is generally true for leaf expression nodes that are not constant; however,
     *       some interior nodes (e.g. Lookup, random generator) may also generate changes
     *       of their own accord.
     * @return True if so, false if not.
     * @note The default method returns false.
     */

    virtual bool isPropagationSource() const
    {
      return false;
    }

    /**
     * @brief Call the function on all subexpressions of this one.
     * @param oper A function of one argument, a Listenable, returning void.
     * @note The default method does nothing.
     */
    virtual void doSubexprs(ListenableUnaryOperator const & /* oper */)
    {
    }
 
  protected:

    Listenable()
    {
    }

  };

}

#endif // PLEXIL_LISTENABLE_HH
