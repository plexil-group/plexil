/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_EXPRESSION_HH
#define PLEXIL_EXPRESSION_HH

// #include "plexil-config.h" // included in Id.hh

#include "Id.hh"
#include "ExpressionListener.hh"
#include "ValueType.hh"

#include <iosfwd>
#include <string>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#elif defined(__VXWORKS__)
#include <vxWorks.h>
#endif

//
// Virtual base classes for the expression system
//

//
// TODO:
//  - Flush Ids!
//

namespace PLEXIL
{

  //
  // Forward references
  //

  class Expression;
  typedef Id<Expression> ExpressionId;
  class Node;
  typedef Id<Node> NodeId;

  /**
   * @class Expression
   * @brief Abstract base class for expressions.
   */
  class Expression : public ExpressionListener
  {
  public:
    virtual ~Expression();

    inline const ExpressionId &getId() const
    {
      return static_cast<const ExpressionId &>(m_id);
    }

    //
    // Essential type-invariant Expression API
    //

    /**
     * @brief Return a print name for the expression type.
     * @return A constant character string.
     */
    virtual const char *exprName() const = 0;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     */
    virtual const ValueType valueType() const = 0;

    /**
     * @brief Query whether the expression's value is known.
     * @return True if known, false otherwise.
     */
    virtual bool isKnown() const = 0;

    /**
     * @brief Query whether this expression is assignable.
     * @return True if assignable, false otherwise.
     * @note The default method returns false.
     * @note Any object which returns true must be derived from Assignable.
     */
    virtual bool isAssignable() const;

    /**
     * @brief Query whether this expression is constant, i.e. incapable of change.
     * @return True if assignable, false otherwise.
     * @note The default method returns false.
     */
    virtual bool isConstant() const;

	/**
	 * @brief Print the object to the given stream.
	 * @param s The output stream.
	 */
    virtual void print(std::ostream& s) const;

	/**
	 * @brief Print the expression's value to the given stream.
	 * @param s The output stream.
	 */
    virtual void printValue(std::ostream& s) const = 0;

    //
    // Convenience methods which may be overridden or extended
    //

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
	virtual std::string toString() const;

    /**
     * @brief Get a string representation of the value of this Expression.
     * @return The string representation.
     */
    virtual std::string valueString() const;

    //
    // Expression graph API
    //

    /**
     * @brief Parts of the notification graph may be inactive, which mans that value change
     *        notifications won't propagate through them.  The isActive method controls this.
     * @return true if this Expression is active, false if it is not.
     */
    virtual bool isActive() const = 0;

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     */
    virtual void activate() = 0;

    /**
     * @brief Make this listener inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     */
    virtual void deactivate() = 0;

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param id The Id of the listener to notify.
     */
    virtual void addListener(ExpressionListenerId id) = 0;

    /**
     * @brief Remove a listener from this Expression.
     * @param id The Id of the listener to remove.
     */
    virtual void removeListener(ExpressionListenerId id) = 0;

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @note The default method does nothing.
     * @note Overrides method of same name on ExpressionListener.
     */
    virtual void notifyChanged();

    //
    // Value API
    //

    //
    // The base class has to explicitly name all the potential types;
    // we can't use a template to declare pure virtual member functions
    // with a default method.
    //

    // 
    // If the above weren't bad enough, to maintain type consistency,
    // we have to use an out parameter instead of a return value for getValue()
    // because a return value might be implicitly promoted to the wrong type.
    // C++ sucks at polymorphism.
    //

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown.
     * @note Derived classes should implement only the appropriate methods.
     * @note Default methods return an error in every case.
     */

    // Numeric types
    virtual bool getValue(double &) const;   // Real
    virtual bool getValue(int32_t &) const;  // Integer
    virtual bool getValue(uint16_t &) const; // enumerations: State, Outcome, Failure, etc.
    virtual bool getValue(bool &) const;     // Boolean

    // Non-numeric types
    virtual bool getValue(std::string &) const; // String

    // Array types (TBD)

  protected:

    // Default constructor is protected because it should only be called from 
    // derived classes' constructor methods.
    Expression();

  private:

    // Deliberately not implemented.
    Expression(const Expression &);
    Expression &operator=(const Expression &);
  };

  // Stream-style print operator
  std::ostream& operator<<(std::ostream& s, const Expression &e);

}

#endif // PLEXIL_EXPRESSION_HH