/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#ifndef _H_Expression
#define _H_Expression

/**
 * @file Expression.hh
 * @author Michael Iatauro
 * @date Mon Feb 13 13:52 2006
 * @brief
 * @ingroup Exec
 */

#include "ExecDefs.hh"
//#include "PlexilPlan.hh"

#include <limits>
#include <map>
#include <set>
#include <stdint.h> // KLUDGE - should be <cstdint>, but isn't part of std before C++11

// Common constants used only in expressions

// The most positive legal "integer" value.
#define PLUS_INFINITY (std::numeric_limits<int32_t>::max())

// The most negative legal "integer" value.
#define MINUS_INFINITY (std::numeric_limits<int32_t>::min())

#define REAL_PLUS_INFINITY (PLEXIL::g_maxReal())

#define REAL_MINUS_INFINITY (-PLEXIL::g_maxReal())

/**
 * @def EPSILON
 * Used when computing differences and comparing real numbers:
 * smallest recognized increment.
 */
#define EPSILON (PLEXIL::g_epsilon())

namespace PLEXIL {
  /**
   * @brief A listener for the notification graph.
   */
  class ExpressionListener {
  public:

    /**
     * @brief Constructor for listener for events on Expressions.  The intention here is to
     *        unify the structure of things that have values which may or may not require
     *        computation.  The notification system allows for cheap re-computation of the
     *        values of complex expressions.  Note that there is a potential problem here.
     *        I've defined the UNKNOWN value as __DBL_MAX__, but that may be a valid, useful
     *        value from the outside world.  Something that we may want to do is create a
     *        Value class and have a specific instance that represents UNKNOWN.
     */
    ExpressionListener();
    virtual ~ExpressionListener();
    const ExpressionListenerId& getId() const {return m_id;}

    /**
     * @brief Virtual function for notification that an expression's value has changed.
     * @param expression The expression whose value has changed.
     */
    virtual void notifyValueChanged(const ExpressionId& expression) = 0;

    /**
     * @brief Parts of the notification graph may be inactive, which means that value change
     *        notifications won't propagate through them.  The isActive method controls this.
     * @return true if this listener is active, false if it is not.
     */
    bool isActive() const {return m_activeCount > 0;}

    /**
     * @brief Make this listener active, meaning notifications will be published through it.
     */
    void activate();

    /**
     * @brief Make this listener inactive, preventing notifications through it.
     */
    void deactivate();

  private:
    ExpressionListenerId m_id; /*!< The Id for this listener */
    unsigned int m_activeCount;
  };


  /**
   * @brief The base class for all Expressions, immediate or otherwise.  Changes to values
   *        can cause a notification to interested parties of the change, which may cause
   *        recalculation of some other Expression's value.
   */
  class Expression {
  public:

    //redirects for old usage.
    DECLARE_STATIC_CLASS_CONST(Value, UNKNOWN, PLEXIL::UNKNOWN());

    static ExpressionId& UNKNOWN_EXP();

    /**
     * @brief Generic constructor for Expressions.  Does nothing but initialze base data
     *        structures.  Expressions are inactive by default and must be activated.
     */
    Expression();

    /**
     * @brief Destructor.  Cleans up listeners on other expressions and any subexpressions.
     */
    virtual ~Expression();

    const ExpressionId& getId() const {return m_id;}

    /**
     * @brief Retrieve the value of this Expression.  This may cause recalculation, lookup of
     *        a value, or something similar.
     * @return The value of this Expression.
     */
    virtual const Value& getValue() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     * @note The default method returns UNKNOWN.
     */
    virtual PlexilType getValueType() const { return PLEXIL::UNKNOWN_TYPE; }

	/**
	 * @brief Report whether the expression is an array.
	 * @return True if an array, false otherwise.
	 * @note This default method returns false.
	 */
	virtual bool isArray() const { return false; }

    /**
     * @brief Set the value for this expression.  This may cause notifications to fire, which
     *        may in turn cause other Expressions to change value.
     * @param val The new value for this expression.
     */
    virtual void setValue(const Value& val);

    /**
     * @brief Parts of the notification graph may be inactive, which mans that value change
     *        notifications won't propagate through them.  The isActive method controls this.
     * @return true if this Expression is active, false if it is not.
     */
    bool isActive() const {return m_activeCount > 0;}

    /**
     * @brief Returns true if this expression is "locked": its value will not change until
     *        unlocked.
     * @return true if this expression is locked, false otherwise
     * @see lock(), unlock()
     */

    bool isLocked() const {return m_lock;}

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     */

    void activate();

    /**
     * @brief Make this listener inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     */
    void deactivate();

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param id The Id of the listener to notify.
     */
    virtual void addListener(ExpressionListenerId id);

    /**
     * @brief Remove a listener from this Expression.
     * @param id The Id of the listener to remove.
     */
    virtual void removeListener(ExpressionListenerId id);

	/**
	 * @brief Print the object to the given stream.
	 * @param s The output stream.
	 */
	virtual void print(std::ostream& s) const;

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
	std::string toString() const;

    /**
     * @brief Get a string representation of the value of this Expression.
     * @return The string representation.
     */
    virtual std::string valueString() const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */

    // TODO?: move to Value class
	virtual void printValue(std::ostream& s) const;

    /**
     * @brief Lock this expression so its value doesn't change.  Changes are stored until the
     *        unlock call, at which point the expression gets updated and notifications happen
     *        normally.
     */
    void lock();

    /**
     * @brief Unlock this expression so its value can change.  If it had a change during the
     *        time in which it was locked, it gets updated to that value and the change is
     *        propagated.
     */
    void unlock();

    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     * @param val The value.
     * @note Default method returns false.
     */
    virtual bool checkValue(const Value& /* val */) const
    {
      return false;
    }

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     * @note The default method does nothing.
     */
    virtual void handleChange(const ExpressionId& /* exp */) {}

  protected:

    /**
     * @brief Internal method for handling the setting of a value.  Used to allow overriding
     *        of setValue to do different things.  Will cause notification of change if there
     *        is a change is this expression isn't locked.
     * @param value The value being set.
     */
    void internalSetValue(const Value& value);

    /**
     * @brief If new value differs from old, set and propagate change.
     * @param value The value being set.
     * @note Core of internalSetValue. Called by unlock() and internalSetValue.
     */
    void essentialSetValue(const Value& value);

    /**
     * @brief Notify listeners that the value of this expression has changed.
     */
    virtual void publishChange();

    /**
     * @brief Handle the activation of the expression.
     * @param changed True if the call to activate actually caused a change from inactive to
     *                active.
     */
    virtual void handleActivate(const bool /* changed */) {}

    /**
     * @brief Handle the deactivation of the expression
     * @param changed True if the call to deactivate actually caused a change from active to
     *                inactive.
     */
    virtual void handleDeactivate(const bool /* changed */) {}

    ExpressionId m_id; /*<! The Id for this Expression */
    std::vector<ExpressionListenerId> m_outgoingListeners; /*<! For outgoing message notifications (this expression's value has changed) */
    Value m_value; /*<! The value of this expression*/
    Value m_savedValue; /*<! The latest value computed for this expression while it was locked. */
    unsigned int m_activeCount;
    bool m_dirty; /*<! Marks whether or not this expression needs re-calculation.*/
    bool m_lock; /*<! The lock for this expression */
  };

  std::ostream& operator<<(std::ostream& s, const Expression& e);

}

#endif // _H_Expression
