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

#ifndef PLEXIL_ALIAS_HH
#define PLEXIL_ALIAS_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{
  // Forward declaration
  class Value;

  //! \class Alias
  //! \brief A read-only proxy for another expression.
  //! \note Most commonly used in library nodes, but also anywhere
  //!       read-only access to a mutable expression is needed.
  //! \ingroup Expressions
  class Alias :
    public Expression,
    public Propagator
  {
  public:

    //! \brief Constructor.
    //! \param name Name of this object as a pointer to const character string.
    //! \param original Pointer to the Expression for which this Alias is a proxy.
    //! \param garbage true if the expression should be deleted with this object, false otherwise.
    Alias(char const *name,
          Expression *original = nullptr,
          bool garbage = false);

    //! \brief Virtual destructor,
    virtual ~Alias();

    //
    // Expression API
    //

    //! \brief Return the name of this expression.
    //! \return Pointer to const character string.
    virtual char const *getName() const override;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual char const *exprName() const override;

    //! \brief Query whether this expression is constant, i.e. incapable of change.
    //! \return True if the expression will never change value, false otherwise.
    virtual bool isConstant() const override;

    //! \brief Get a pointer to the expression for which this may be an alias or reference.
    //! \return Pointer to the base expression.
    virtual Expression *getBaseExpression() override;

    //! \brief Get a const pointer to the expression for which this may be an alias or reference.
    //! \return Const pointer to the base expression.
    virtual Expression const *getBaseExpression() const override;

    //! \brief Return the type of the expression's value.
    //! \return The value type.
    virtual ValueType valueType() const override;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Get the value of this expression as a Value instance.
    //! \return The value.
    virtual Value toValue() const override;

    //! \brief Print the expression's value to a stream.
    //! \param s Reference to the stream.
    virtual void printValue(std::ostream &s) const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(Boolean &var) const override;
    virtual bool getValue(Integer &var) const override;
    virtual bool getValue(Real &var) const override;
    virtual bool getValue(NodeState &) const override;
    virtual bool getValue(NodeOutcome &) const override;
    virtual bool getValue(FailureType &) const override;
    virtual bool getValue(CommandHandleValue &) const override;
    virtual bool getValue(String &var) const override;

    //! \brief Copy a pointer to the (const) value of this object to a resut variable.
    //! \param ptr Reference to an appropriately typed pointer variable.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValuePointer(String const *&ptr) const override;
    virtual bool getValuePointer(Array const *&ptr) const override;
    virtual bool getValuePointer(BooleanArray const *&ptr) const override;
    virtual bool getValuePointer(IntegerArray const *&ptr) const override;
    virtual bool getValuePointer(RealArray const *&ptr) const override;
    virtual bool getValuePointer(StringArray const *&ptr) const override;

  protected:

    //! \brief Call a function on all subexpressions of this object.
    //! \param func A functor; it must implement an operator() method
    //!             of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &f) override;

    //! \brief The expression being aliased.
    Expression *m_exp;

    //! \brief Name of the aliased expression, in the owning node
    char const *m_name;

  private:

    //! \brief True if m_exp should be deleted with this object.
    bool m_garbage;

    // Disallow default, copy, assign
    Alias() = delete;
    Alias(Alias const &) = delete;
    Alias(Alias &&) = delete; 
    Alias &operator=(Alias const &) = delete;
    Alias &operator=(Alias &&) = delete;

  };

} // namespace PLEXIL

#endif // PLEXIL_ALIAS_HH
