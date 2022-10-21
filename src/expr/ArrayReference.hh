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

#ifndef PLEXIL_ARRAY_REFERENCE_HH
#define PLEXIL_ARRAY_REFERENCE_HH

#include "Assignable.hh"
#include "Expression.hh"
#include "Propagator.hh"
#include "Value.hh"

namespace PLEXIL {

  // Forward reference
  class ArrayVariable;

  //! \class ArrayReference
  //! \brief An expression class implementing read-only access to an ArrayVariable.
  //! \ingroup Expressions
  class ArrayReference :
    virtual public Expression,
    public Propagator
  {
  public:

    //! \brief Constructor.
    //! \param ary Pointer to the array expression.
    //! \param idx Pointer to the index expression.
    //! \param aryIsGarbage True if the array expression should be destroyed with this object.
    //! \param idxIsGarbage True if the index expression should be destroyed with this object.
    ArrayReference(Expression *ary,
                   Expression *idx,
                   bool aryIsGarbage = false,
                   bool idxIsGarbage = false);

    //! \brief Virtual destructor.
    ~ArrayReference();

    //
    // Essential Expression API
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
    virtual void printValue(std::ostream& s) const override;

    //! \brief Copy the value of this object to a result variable.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValue(Boolean &result) const override;
    virtual bool getValue(Integer &result) const override;
    virtual bool getValue(Real &result) const override;
    virtual bool getValue(String &result) const override;

    // These issue a PlanError
    virtual bool getValue(NodeState &result) const override;
    virtual bool getValue(NodeOutcome &result) const override;
    virtual bool getValue(FailureType &result) const override;
    virtual bool getValue(CommandHandleValue &result) const override;

    //! \brief Copy a pointer to the (const) value of this object to a resut variable.
    //! \param ptr Reference to an appropriately typed pointer variable.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    virtual bool getValuePointer(String const *&ptr) const override;

  protected:

    //
    // Notifier API
    //

    //! \brief Perform any necessary actions to enter the active state.
    virtual void handleActivate() override;

    //! \brief Perform any necessary actions to enter the inactive state.
    virtual void handleDeactivate() override;

    //! \brief Call a function on all subexpressions of this object.
    //! \param oper A function of one argument, a pointer to Listenable,
    //!             returning void.
    virtual void doSubexprs(ListenableUnaryOperator const &f) override;

    //
    // State shared with MutableArrayReference
    //

    //! \brief Pointer to the array expression.
    Expression *m_array;

    //! \brief Pointer to the index expression.
    Expression *m_index;

    //! \brief True if the array expression should be deleted with this object.
    bool m_arrayIsGarbage;

    //! \brief True if the index expression should be deleted with this object.
    bool m_indexIsGarbage;

    //! \brief The name of this expression.
    std::string *m_namePtr;

  private:
    // Disallow default, copy, assignment
    ArrayReference() = delete;
    ArrayReference(ArrayReference const &) = delete;
    ArrayReference(ArrayReference &&) = delete;
    ArrayReference &operator=(ArrayReference const &) = delete;
    ArrayReference &operator=(ArrayReference &&) = delete;

    //! \brief Perform several validity checks on the ArrayReference instance itself,
    //!        the Array instance, and the index.
    //! \param valuePtr Reference to a const pointer to the Array value.
    //! \param idx The array index.
    //! \return true if everything is consistent and the value of the element at the
    //!         index is known; false otherwise.
    //! \note Throws a PlanError exception if the index is negative or exceeds the
    //!       array's actual size.
    bool selfCheck(Array const *&ary,
                   size_t &idx) const;
  };

  //! \class MutableArrayReference
  //! \brief Expression class that represents a modifiable location in an array.
  //! \ingroup Expressions
  class MutableArrayReference :
    public Assignable,
    public ArrayReference
  {
  public:

    //! \brief Constructor.
    //! \param ary Pointer to the array expression.
    //! \param idx Pointer to the index expression.
    //! \param aryIsGarbage True if the array expression should be destroyed with this object.
    //! \param idxIsGarbage True if the index expression should be destroyed with this object.
    MutableArrayReference(Expression *ary,
                          Expression *idx,
                          bool aryIsGarbage = false,
                          bool idxIsGarbage = false);

    //! \brief Virtual destructor.
    ~MutableArrayReference() = default;

    //
    // Assignable API
    //

    //! \brief Temporarily store the current value of this variable.
    virtual void saveCurrentValue() override;

    //! \brief Restore the value set aside by saveCurrentValue().
    virtual void restoreSavedValue() override;

    //! \brief Read the saved value of this variable.
    virtual Value getSavedValue() const override;

    //! \brief Get the real variable for which this may be a proxy.
    //! \return Pointer to the base variable.
    virtual Variable *getBaseVariable() override;

    //! \brief Get the real variable for which this may be a proxy.
    //! \return Const pointer to the base variable.
    virtual Variable const *getBaseVariable() const override;

    //! \brief Set the value of this expression to unknown.
    virtual void setUnknown() override;

    //! \brief Set the value for this expression from a Value instance.
    //! \param val Const reference to the new value.
    virtual void setValue(Value const &value) override;

    using Assignable::setValue;

  private:
    // Default, copy, assignment disallowed
    MutableArrayReference() = delete;
    MutableArrayReference(MutableArrayReference const &) = delete;
    MutableArrayReference(MutableArrayReference &&) = delete;
    MutableArrayReference &operator=(MutableArrayReference const &) = delete;
    MutableArrayReference &operator=(MutableArrayReference &&) = delete;

    //! \brief Perform several validity checks on the MutableArrayReference
    //!        instance itself, the array expression, and the index.
    //! \param idx The array index.
    //! \return true if everything is consistent and the value of the element at the
    //!         index is known; false otherwise.
    //! \note Throws a PlanError exception if the index is negative or exceeds the
    //!       array's actual size.
    bool mutableSelfCheck(size_t &idx);

    //! \brief Pointer to the array expression as an ArrayVariable instance.
    ArrayVariable *m_mutableArray;

    //! \brief Value saved by saveCurrentValue().
    Value m_savedValue;

    //! \brief True if saveCurrentValue() has been called, false otherwise.
    bool m_saved;
  };

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_REFERENCE_HH
