/* Copyright (c) 2006-2016, Universities Space Research Association (USRA).
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

#include "ValueType.hh"
#include "ExpressionListener.hh" 

//
// Virtual base classes for the expression system
//

namespace PLEXIL
{

  //
  // Forward references
  //

  class Assignable;
  class Value;

  /**
   * @class Expression
   * @brief Abstract base class for expressions.
   */
  class Expression : public ExpressionListener
  {
  protected:
    Expression();

  private:
    // Unimplemented
    Expression(Expression const &);
    Expression &operator=(Expression const &);

  public:
    virtual ~Expression();

    //
    // Essential type-invariant Expression API
    //

    /**
     * @brief Return the name of this expression.
     * @return Const reference to string.
     * @note Default method returns empty string.
     */
    virtual char const *getName() const;

    /**
     * @brief Return a print name for the expression type.
     * @return A constant character string.
     */
    virtual char const *exprName() const = 0;

    /**
     * @brief Query whether this expression is assignable.
     * @return True if assignable, false otherwise.
     * @note The default method returns false.
     * @note Any object which returns true must be derived from Assignable.
     */
    virtual bool isAssignable() const;

    /**
     * @brief Get a pointer to this expression as an Assignable instance.
     * @return The pointer. NULL if not Assignable.
     * @note Only objects derived from Assignable should return a pointer.
     */
    virtual Assignable *asAssignable();
    virtual Assignable const *asAssignable() const;

    /**
     * @brief Query whether this expression is constant, i.e. incapable of change.
     * @return True if assignable, false otherwise.
     * @note The default method returns false.
     */
    virtual bool isConstant() const;

    /**
     * @brief Get the real expression for which this may be an alias or reference.
     * @return Pointer to the base expression.
     */
    virtual Expression *getBaseExpression();
    virtual Expression const *getBaseExpression() const;

	/**
	 * @brief Print the object to the given stream.
	 * @param s The output stream.
	 */
    virtual void print(std::ostream& s) const;

    /**
     * @brief Print additional information about derived objects.
     * @param s The output stream.
     */
    virtual void printSpecialized(std::ostream &s) const;

    /**
     * @brief Print subexpressions of derived objects.
     * @param s The output stream.
     */
    virtual void printSubexpressions(std::ostream &s) const;

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
    // GetValue API
    //

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     */
    virtual ValueType valueType() const = 0;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     */
    virtual bool isKnown() const = 0;

    /**
     * @brief Get the value of this object as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const = 0;

    /**
     * @brief Print the object's value to the given stream.
     * @param s The output stream.
     */
    virtual void printValue(std::ostream& s) const = 0;

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
     * @brief Retrieve the value of this object.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     * @note The value is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    virtual bool getValue(Boolean &result) const = 0;
    virtual bool getValue(uint16_t &result) const = 0;
    virtual bool getValue(Integer &result) const = 0;
    virtual bool getValue(Real &result) const = 0;

    virtual bool getValue(String &result) const = 0;

    /**
     * @brief Retrieve a pointer to the (const) value of this object.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     * @note The pointer is not copied if the return value is false.
     * @note Derived classes should implement only the appropriate methods.
     */

    virtual bool getValuePointer(String const *&ptr) const = 0;

    virtual bool getValuePointer(Array const *&ptr) const = 0;

    virtual bool getValuePointer(BooleanArray const *&ptr) const = 0;
    virtual bool getValuePointer(IntegerArray const *&ptr) const = 0;
    virtual bool getValuePointer(RealArray const *&ptr) const = 0;
    virtual bool getValuePointer(StringArray const *&ptr) const = 0;

    //
    // Expression notification graph API
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
     * @param ptr The pointer to the listener to add.
     */
    virtual void addListener(ExpressionListener *ptr) = 0;

    /**
     * @brief Remove a listener from this Expression.
     * @param ptr The pointer to the listener to remove.
     */
    virtual void removeListener(ExpressionListener *ptr) = 0;

    //
    // ExpressionListener API
    //

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param src The Expression which initiated the change.
     * @note This default method does nothing.
     */
    virtual void notifyChanged(Expression const *src);

  };

  // Stream-style print operator
  std::ostream& operator<<(std::ostream& s, const Expression &e);

}

#endif // PLEXIL_EXPRESSION_HH
