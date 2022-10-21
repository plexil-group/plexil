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

#ifndef PLEXIL_USER_VARIABLE_HH
#define PLEXIL_USER_VARIABLE_HH

#include "GetValueImpl.hh"
#include "Notifier.hh"
#include "Variable.hh"

namespace PLEXIL 
{


  //! \brief Templatized class for user-created variables of scalar types.

  //! UserVariable is a template class whose template parameter is the
  //! type which the variable stores.

  //! \ingroup Expressions

  template <typename T>
  class UserVariable final :
    public Variable,
    public GetValueImpl<T>,
    public Notifier
  {
  public:

    //! \brief Default constructor.
    UserVariable();

    //! \brief Constructor with initial value.
    //! \param initVal Const reference to the initial value.
    UserVariable(T const &initVal);

    //! \brief Constructor with variable name.
    //! \param name Pointer to const character string representing the name of this
    //!             variable in the parent node.
    //! \note Used by the plan parser.
    UserVariable(char const *name);
    
    //! \brief Virtual destructor.
    virtual ~UserVariable();

    //
    // Listenable API
    //

    //! \brief Query whether an object can generate change notifications.
    //! \return True if so, false if not.
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    //! \brief Return the name of this expression.
    //! \return Pointer to const character string.
    virtual char const *getName() const override;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual char const *exprName() const override;

    //
    // GetValueImpl API
    //

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Copy the value of this object to a result of the same type.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(T &result) const override;

    //
    // Assignable API
    //

    //! \brief Set the value of this expression to unknown.
    virtual void setUnknown() override;

    //! \brief Temporarily store the previous value of this variable.
    virtual void saveCurrentValue() override;

    //! \brief Restore the value set aside by saveCurrentValue.
    virtual void restoreSavedValue() override;

    //! \brief Read the saved value of this variable as a Value instance.
    //! \return The saved value.
    virtual Value getSavedValue() const override;

    //! \brief Set the expression from which this object gets its initial value.
    //! \param expr Pointer to an Expression.
    //! \param garbage True if the expression should be deleted with this object, false otherwise.
    virtual void setInitializer(Expression *expr, bool garbage) override;

    //! \brief Set the value for this object.
    //! \param val The new value for this object.
    virtual void setValue(Value const &val) override;

    //! \brief Set the value for this object.
    //! \param val Const reference to the expression providing the new value for this object.
    virtual void setValue(Expression const &val);

  protected:

    //
    // Method overrides
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream.
    virtual void printSpecialized(std::ostream &s) const override;

    //! \brief Perform any necessary actions to enter the active state.
    virtual void handleActivate() override;

    //! \brief Perform any necessary actions to enter the inactive state.
    virtual void handleDeactivate() override;

  private:
    
    //
    // Implementation details
    //

    //! \brief Assign a new value.
    //! \param value Const reference to the value to assign.
    void setValueImpl(T const &value);

    //
    // Member variables
    //

    // N.B. Ordering is suboptimal for bool because of required padding;
    // fine for Integer and Real

    //! \brief The current value.
    T m_value;

    //! \brief The saved value.
    T m_savedValue;

    //! \brief Pointer to the initial value expression.
    Expression *m_initializer;

    //! \brief The variable's name.
    char const *m_name;

    //! \brief True if the current value is known, false otherwise.
    bool m_known;

    //! \brief True if the saved value is known, false otherwise.
    bool m_savedKnown;

    //! \brief If true, delete the initializer expression when this object is deleted.
    bool m_initializerIsGarbage;
  };

  //! \brief Class template for user-created plan variables, specialized for String type.
  //! \ingroup Expressions
  template <>
  class UserVariable<String> final :
    public Variable,
    public GetValueImpl<String>,
    public Notifier
  {
  public:

    //! \brief Default constructor.
    UserVariable();

    //! \brief Constructor with initial value.
    //! \param initVal Const reference to the initial value.
    explicit UserVariable(String const &initVal);

    //! \brief Constructor with name.
    //! \param name Pointer to const character string representing the name of this
    //!             variable in the parent node.
    //! \note Used by the plan parser.
    UserVariable(char const *name);

    //! \brief Virtual destructor.
    virtual ~UserVariable();

    //
    // Listenable API
    //

    //! \brief Query whether an object can generate change notifications.
    //! \return True if so, false if not.
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    //! \brief Return the name of this expression.
    //! \return Pointer to const character string.
    virtual char const *getName() const override;

    //! \brief Return a print name for the expression type.
    //! \return Pointer to const character string.
    virtual char const *exprName() const override;

    //! \brief Determine whether the value of this expression is known or unknown.
    //! \return True if known, false otherwise.
    virtual bool isKnown() const override;

    //! \brief Copy the value of this object to a result of the same type.
    //! \param result Reference to an appropriately typed place to store the value.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValue(String &result) const override;

    //! \brief Retrieve a pointer to the (const) value of this Expression.
    //! \param ptr Reference to the pointer variable to receive the result.
    //! \return True if the value is known, false if unknown or the value cannot be
    //!         represented as the desired type.
    //! \note The value is not copied if the return value is false.
    virtual bool getValuePointer(String const *&ptr) const override;
    template <typename U>
    bool getValuePointer(U const *&ptr) const;

    //
    // Assignable API
    //

    //! \brief Set the value of this expression to unknown.
    virtual void setUnknown() override;

    //! \brief Temporarily store the previous value of this variable.
    virtual void saveCurrentValue() override;

    //! \brief Restore the value set aside by saveCurrentValue.
    virtual void restoreSavedValue() override;

    //! \brief Read the saved value of this variable as a Value instance.
    //! \return The saved value.
    virtual Value getSavedValue() const override;

    //! \brief Set the expression from which this object gets its initial value.
    //! \param expr Pointer to an Expression.
    //! \param garbage True if the expression should be deleted with this object, false otherwise.
    virtual void setInitializer(Expression *expr, bool garbage) override;

    //! \brief Set the value for this object.
    //! \param val The new value for this object.
    virtual void setValue(Value const &val) override;

    //! \brief Set the value for this object.
    //! \param val Const reference to the expression providing the new value for this object.
    virtual void setValue(Expression const &val);

  protected:

    //
    // Method overrides
    //

    //! \brief Print additional specialized information about an expression to a stream.
    //! \param s Reference to the output stream. 
    virtual void printSpecialized(std::ostream &s) const override;

    //! \brief Perform any necessary actions to enter the active state.
    virtual void handleActivate() override;

    //! \brief Perform any necessary actions to enter the inactive state.
    virtual void handleDeactivate() override;

  private:

    //
    // Implementation details
    //

    //! \brief Assign a new value.
    //! \param value The value to assign.
    virtual void setValueImpl(String const &value);

    //
    // Member variables
    //

    //! \brief The current value.
    String m_value;

    //! \brief The saved value.
    String m_savedValue;

    //! \brief Pointer to the initial value expression.
    Expression *m_initializer;

    //! \brief The variable's name.
    char const *m_name;

    //! \brief True if the current value is known, false otherwise.
    bool m_known;

    //! \brief True if the saved value is known, false otherwise.
    bool m_savedKnown;

    //! \brief If true, delete the initializer expression when this object is deleted.
    bool m_initializerIsGarbage;

  };

  //
  // Convenience type aliases 
  //

  using BooleanVariable = UserVariable<Boolean>;
  using IntegerVariable = UserVariable<Integer>;
  using RealVariable    = UserVariable<Real>;
  using StringVariable  = UserVariable<String>;

} // namespace PLEXIL

#endif // PLEXIL_USER_VARIABLE_HH
