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

#ifndef PLEXIL_EXPRESSION_HH
#define PLEXIL_EXPRESSION_HH

#include "ValueType.hh"
#include "Listenable.hh" 

//
// Virtual base class for the PLEXIL expression system
//

namespace PLEXIL
{

  //
  // Forward references
  //
  class Assignable;
  class Value;

  //! \defgroup Expressions The PLEXIL Expression subsystem

  //! \class Expression
  //! \brief Stateless abstract base class for expressions in the %PLEXIL language.
  //!
  //! The Expression class is the basis for expressions in the %PLEXIL
  //! Executive.  It defines an API for querying an expression.  The
  //! Expression class does not support modifying the value of an
  //! expression; the API for that functionality is on a separate
  //! mixin class, Assignable.
  //!
  //! Expression derives from Listenable, therefore every class
  //! derived from Expression must implement the Listenable API.
  //! Expression classes whose value will not change can simply use
  //! the default methods on Listenable.
  //!
  //! Expression classes whose values can change should also
  //! derive from Notifier, and may wish to override some of its
  //! protected member functions.
  //!
  //! Classes for interior nodes in the expression graph
  //! (e.g. arithmetic functions) should derive from Propagator; it
  //! inherits behavior from both ExpressionListener and Notifier, and
  //! as the name implies, it propagates change notifications.
  //!
  //! \see Assignable
  //! \see ExpressionListener
  //! \see Listenable
  //! \see Notifier
  //! \see Propagator
  //! \ingroup Expressions

  class Expression : virtual public Listenable
  {
  public:

    //
    // Essential type-invariant Expression API
    //

    //! \brief Return the name of this expression.
    //! \return Pointer to const character string.
    //! \note The default method returns a pointer to an empty string.
    virtual char const *getName() const;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    //! \note Instantiable derived classes must implement this member function.
    virtual char const *exprName() const = 0;

    //! \brief Query whether this expression is assignable in the %PLEXIL language.
    //! \return True if assignable, false otherwise.
    //! \note The default method returns false.
    //! \note Any object which returns true must be derived from Assignable.
    virtual bool isAssignable() const;

    //! \brief Get a pointer to this expression as an instance of Assignable.
    //! \return The pointer. Will be NULL if the expression does not permit assignment.
    //! \note The default method returns NULL.
    //! \note Only objects derived from Assignable should return a pointer.
    virtual Assignable *asAssignable();

    //! \brief Get a const pointer to this expression as an instance of Assignable.
    //! \return The pointer. Will be NULL if not the expression does not permit assignment.
    //! \note The default method returns NULL.
    //! \note Only objects derived from Assignable should return a pointer.
    virtual Assignable const *asAssignable() const;

    //! \brief Query whether this expression is constant, i.e. incapable of change.
    //! \return True if the expression will never change value, false otherwise.
    //! \note The default method returns false.
    //! \note Constant expressions cannot generate change notifications.
    virtual bool isConstant() const;

    //! \brief Get a pointer to the expression for which this may be an alias or reference.
    //! \return Pointer to the base expression.
    //! \note The default method returns this, as an Expression pointer.
    virtual Expression *getBaseExpression();

    //! \brief Get a const pointer to the expression for which this may be an alias or reference.
    //! \return Const pointer to the base expression.
    //! \note The default method returns this, as a const Expression pointer.
    virtual Expression const *getBaseExpression() const;

	//! \brief Print the object to the given stream.
	//! \param s Reference to the output stream.
    //! \note Default method calls member functions printSpecialized and printSubexpressions.
    //! \see Expression::printSpecialized
    //! \see Expression::printSubexpressions
    virtual void print(std::ostream& s) const;

    //! \brief Get the printed representation of this Expression as a string.
    //! \return The string.
    virtual std::string toString() const;

    //
    // Value access
    //

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const = 0;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const = 0;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The value.
    virtual Value toValue() const = 0;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream& s) const = 0;

    //! \brief Get a string representation of the value of this Expression.
    //! \return The string representation.
    virtual std::string valueString() const;

    //
    // The base class has to explicitly name all the potential types;
    // we can't use a template to declare pure virtual member functions
    // with a default method.
    //
    // If the above weren't bad enough, to maintain type consistency,
    // we have to use an out parameter instead of a return value for getValue()
    // because a return value might be implicitly promoted to the wrong type.
    //
    // C++ sucks at polymorphism.
    //

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The value is not copied if the return value is false.
    //! \note Default methods throw a PlanError.
    //! \note Derived classes should implement only the appropriate methods.

    virtual bool getValue(Boolean &result) const;
    virtual bool getValue(NodeState &result) const;
    virtual bool getValue(NodeOutcome &result) const;
    virtual bool getValue(FailureType &result) const;
    virtual bool getValue(CommandHandleValue &result) const;
    virtual bool getValue(Integer &result) const;
    virtual bool getValue(Real &result) const;

    virtual bool getValue(String &result) const;

    //! \brief Copy a pointer to the (const) value of this object to a resut variable.
    //! \param ptr Reference to an appropriately typed pointer variable.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The pointer is not copied if the return value is false.
    //! \note Default methods throw a PlanError.
    //! \note Derived classes should implement only the appropriate methods.

    virtual bool getValuePointer(String const *&ptr) const;

    virtual bool getValuePointer(Array const *&ptr) const;

    virtual bool getValuePointer(BooleanArray const *&ptr) const;
    virtual bool getValuePointer(IntegerArray const *&ptr) const;
    virtual bool getValuePointer(RealArray const *&ptr) const;
    virtual bool getValuePointer(StringArray const *&ptr) const;

  protected:

    //
    // Helper member functions for print()
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    //! \note Called by the base class print method.
    //! \see Expression::print
    virtual void printSpecialized(std::ostream &s) const;

    //! \brief Print the subexpressions of this expression to a stream.
    //! \param s Reference to the output stream.
    //! \note Called by the base class print method.
    //! \note The default method does nothing.
    //! \see Expression::print
    virtual void printSubexpressions(std::ostream &s) const;

  };

  //! \brief Overloaded formatted output operator.
  //! \param s Reference to the output stream.
  //! \param e Const reference to an Expression.
  //! \return Reference to the output stream.
  //! \ingroup Expressions
  std::ostream& operator<<(std::ostream& s, const Expression &e);

}

#endif // PLEXIL_EXPRESSION_HH
