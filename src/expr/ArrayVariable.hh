/* Copyright (c) 2006-2021, Universities Space Research Association (USRA).
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

#ifndef PLEXIL_ARRAY_VARIABLE_HH
#define PLEXIL_ARRAY_VARIABLE_HH

#include "GetValueImpl.hh"
#include "Notifier.hh"
#include "Variable.hh"

#include <memory> // std::unique_ptr
#include <vector>

namespace PLEXIL
{

  /**
   * @class ArrayVariable
   * @brief An abstract class similar to UserVariable, which adds accessors required
   *        by the ArrayReference and MutableArrayReference expression classes.
   *        ArrayVariable methods implement the type-invariant operations;
   *        type-specific operations are delegated to a derived class.
   */

  class ArrayVariable :
    public Variable,
    virtual public Expression,
    public Notifier
  {
  public:

    virtual ~ArrayVariable();

    // Listenable API
    virtual bool isPropagationSource() const override;

    //
    // Essential Expression API
    //

    //! Returns true if the variable is assignable.
    virtual bool isAssignable() const override;

    //! Returns a pointer-to-const as an Assignable if the variable is assignable,
    //! nullptr otherwise.
    virtual Assignable const *asAssignable() const override;

    //! Returns a pointer as an Assignable if the variable is assignable,
    //! nullptr otherwise.
    virtual Assignable *asAssignable() override;

    //! Return the name of the variable as declared in the node which created it.
    virtual char const *getName() const override;

    //! Return the name of the expression type.
    virtual char const *exprName() const override;

    //! Return true if the variable's value is known, false otherwise.
    virtual bool isKnown() const override;

    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    virtual bool getValuePointer(Array const *&ptr) const override;

    //
    // Assignable API
    //

    //! Save the variable's current value prior to assigning to it.
    virtual void saveCurrentValue() override;

    // Provided by derived class
    // virtual void restoreSavedValue() override;

    //! Return the saved value.
    //! @note Only used by Assignment::retract() to notify the ExecListener.
    virtual Value getSavedValue() const override;

    //! Set this variable's initializer expression.
    //! @param expr The initializer expression.
    //! @param garbage True if the initializer is uniquely constructed,
    //!                false otherwise.
    virtual void setInitializer(Expression *expr, bool garbage) override;

    //! Make this variable's value unknown.
    virtual void setUnknown() override;

    //! Set this variable's value.
    //! @param val The new value.
    virtual void setValue(Value const &val) override;

    //! Set the value of this expression.
    //! @param val The expression with the new value for this variable.
    virtual void setValue(Expression const &val);

    //
    // Access needed by ArrayReference
    //

    //! Return true if the element at the index is known, false if not.
    bool elementIsKnown(size_t idx) const;

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result variable.
    //! @return True if the value is known, false otherwise.
    //! @note Default methods throw a PlanError.
    virtual bool getElement(size_t idx, Boolean &result) const; 
    virtual bool getElement(size_t idx, Integer &result) const;
    virtual bool getElement(size_t idx, Real &result) const;
    virtual bool getElement(size_t idx, String &result) const;

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result pointer variable.
    //! @return True if the value is known, false otherwise.
    //! @note Default methods throw a PlanError.
    virtual bool getElementPointer(size_t idx, String const *&ptr) const;

    //! Get the value of the element of the array as a Value.
    //! @param idx The index.
    //! @return The value; may be unknown.
    virtual Value getElementValue(size_t idx) const;

    //
    // Access needed by MutableArrayReference
    //

    //! Set the element of the array at this index.
    //! @param idx The index.
    //! @param value The new value.
    virtual void setElement(size_t idx, Value const &value) = 0;

    //! Set the element of the array at this index unknown.
    //! @param idx The index.
    void setElementUnknown(size_t idx);

  protected:

    //
    // Expression internal API
    //

    //! Print the expression and its value to the stream in a format
    //! qppropriate for the kind of expression.
    //! @param stream The stream.
    virtual void printSpecialized(std::ostream &s) const override;

    //
    // NotifierImpl API
    //

    //! Implement activation as appropriate for this expression.
    virtual void handleActivate() override;

    //! Implement deactivation as appropriate for this expression.
    virtual void handleDeactivate() override;

    //
    // ArrayVariable API to derived classes
    //

    //! Default constructor.
    ArrayVariable();

    //! Constructor for plan loading.
    //! @param name The name of this variable in the parent node.
    //! @param size Expression whose value is the maximum size of the array.
    //! @param sizeIsGarbage True if the size expression should be deleted 
    //!                      with the array variable.
    ArrayVariable(char const *name,
                  Expression *size = nullptr,
                  bool sizeIsGarbage = false);

    //! Set this variable's value from a generic array.
    //! @param a Pointer to array whose contents are to be copied.
    virtual void setValueImpl(Array const *a) = 0;

    //
    // API provided by derived classes
    //

    //! Compare the given array with the current value.
    //! @return True if equal, false if not.
    virtual bool equals(Array const *ary) const = 0;

    //! Construct the variable's array.
    //! @return Pointer to the new Array.
    virtual Array *makeArray() const = 0;

    //
    // Member variables
    //

    //! The current value of this variable, as a generic Array.
    std::unique_ptr<Array> m_value;

    //! The previous value of this variable. Set when an Assignment is active.
    std::unique_ptr<Array> m_savedValue;

    //! m_size is the MaxSize expression.
    Expression *m_size;

    //! Expression for the variable's initializer.
    Expression *m_initializer;

    //! The variable's name in the node which declared it.
    char const *m_name;

    //! The value of the m_size expression.
    size_t m_maxSize;

    //! True if the variable is currently known.
    bool m_known;

    //! True if the variable was known prior to the currently active Assignment.
    bool m_savedKnown;

    //! True if the expression pointed to by m_size was created for this variable.
    bool m_sizeIsGarbage;

    //! True if the expression pointed to by m_initializer expression
    //! was created for this variable.
    bool m_initializerIsGarbage;

    //! True if the expression pointed to by m_size is constant.
    bool m_sizeIsConstant;

    //! True if the expression pointed to by m_initializer is constant.
    bool m_initializerIsConstant;
  };

  //
  // Implementation classes
  //

  template <typename T>
  class ArrayVariableImpl final :
    public GetValueImpl<ArrayImpl<T> >,
    public ArrayVariable
  {
  public:

    //! Default constructor.
    ArrayVariableImpl();

    //! Constructor for plan loading.
    //! @param name The name of this variable in the parent node.
    //! @param size Expression whose value is the initial size of the array.
    //! @param sizeIsGarbage True if the size expression should be deleted 
    //!                      with the array variable.
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    //! Return the value type.
    //! @return A constant enumeration.
    virtual ValueType valueType() const override;

    //! Assign a new value.
    //! @param value The value to assign.
    virtual void setValueImpl(Array const *a) override;

    //! Restore the previous value after a failed assignment.
    virtual void restoreSavedValue() override;
    
    //! Delegate to ArrayVariable method
    //! @return True if the value is known, false if not.
    //! @note Required due to conflicts in multiple inheritance.
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    //! @note Required due to conflicts in multiple inheritance
    virtual bool getValuePointer(Array const *&ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //1 @return True if known, false if unknown.
    virtual bool getValuePointer(ArrayImpl<T> const *&ptr) const override;

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result variable.
    //! @return True if the value is known, false otherwise.
    virtual bool getElement(size_t idx, T &result) const override; 

    virtual void setElement(size_t idx, Value const &value) override;

    //
    // API to base class
    //

    //! Compare the given array with the current value.
    //! @return True if equal, false if not.
    virtual bool equals(Array const *ary) const override;

    //! Construct the variable's array.
    //! @return Pointer to the new Array.
    virtual Array *makeArray() const override;

  private:

    //
    // Internal utilities
    //

    //! Return a pointer-to-const to the actual array as its native type.
    ArrayImpl<T> const *typedArrayPointer() const;

    //! Return a pointer to the actual array as its native type.
    ArrayImpl<T> *typedArrayPointer();

  };

  template <>
  class ArrayVariableImpl<Integer> final :
    public GetValueImpl<ArrayImpl<Integer> >,
    public ArrayVariable
  {
  public:

    //! Default constructor.
    ArrayVariableImpl();

    //! Constructor for plan loading.
    //! @param name The name of this variable in the parent node.
    //1 @param size Expression whose value is the initial size of the array.
    //! @param sizeIsGarbage True if the size expression should be deleted 
    //!                      with the array variable.
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    //! Return the value type.
    //! @return A constant enumeration.
    virtual ValueType valueType() const override;

    //! Assign a new value.
    //! @param value The value to assign.
    virtual void setValueImpl(Array const *a) override;

    //! Restore the previous value after a failed assignment.
    virtual void restoreSavedValue() override;
    
    //! Delegate to ArrayVariable method
    //! @return True if the value is known, false if not.
    //! @note Required due to conflicts in multiple inheritance.
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    //! @note Required due to conflicts in multiple inheritance
    virtual bool getValuePointer(Array const *&ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    virtual bool getValuePointer(ArrayImpl<Integer> const *&ptr) const override;

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result variable.
    //! @return True if the value is known, false otherwise.
    virtual bool getElement(size_t idx, Integer &result) const override; 

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result variable.
    //! @return True if the value is known, false otherwise.
    //! @note Conversion method.
    virtual bool getElement(size_t idx, Real &result) const override; 

    virtual void setElement(size_t idx, Value const &value) override;

    //! Compare the given array with the current value.
    //! @return True if equal, false if not.
    virtual bool equals(Array const *ary) const override;

    //! Construct the variable's array.
    //! @return Pointer to the new Array.
    virtual Array *makeArray() const override;

  private:

    //
    // Internal utilities
    //

    //! Return a pointer-to-const to the actual array as its native type.
    ArrayImpl<Integer> const *typedArrayPointer() const;

    //! Return a pointer to the actual array as its native type.
    ArrayImpl<Integer> *typedArrayPointer();

  };

  template <>
  class ArrayVariableImpl<String> final :
    public GetValueImpl<ArrayImpl<String> >,
    public ArrayVariable
  {
  public:

    //! Default constructor.
    ArrayVariableImpl();

    //! Constructor for plan loading.
    //! @param name The name of this variable in the parent node.
    //! @param size Expression whose value is the initial size of the array.
    //! @param sizeIsGarbage True if the size expression should be deleted 
    //!                      with the array variable.
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    //! Return the value type.
    //! @return A constant enumeration.
    virtual ValueType valueType() const override;

    //! Assign a new value.
    //! @param value The value to assign.
    virtual void setValueImpl(Array const *a) override;

    virtual void restoreSavedValue() override;
    
    //! Delegate to ArrayVariable method
    //! @return True if the value is known, false if not.
    //! @note Required due to conflicts in multiple inheritance.
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    //! @note Required due to conflicts in multiple inheritance
    virtual bool getValuePointer(Array const *&ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    //! Retrieve a pointer to the (const) value of this Expression.
    //! @param ptr Reference to the pointer variable to receive the result.
    //! @return True if known, false if unknown.
    virtual bool getValuePointer(ArrayImpl<String> const *&ptr) const override;

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result variable.
    //! @return True if the value is known, false otherwise.
    virtual bool getElement(size_t idx, String &result) const override; 

    //! Get the value of the element of the array.
    //! @param idx The index.
    //! @param result Reference to the result pointer variable.
    //! @return True if the value is known, false otherwise.
    virtual bool getElementPointer(size_t idx, String const *&ptr) const override;

    virtual void setElement(size_t idx, Value const &value) override;

    //! Compare the given array with the current value.
    //! @return True if equal, false if not.
    virtual bool equals(Array const *ary) const override;

    //! Construct the variable's array.
    //! @return Pointer to the new Array.
    virtual Array *makeArray() const override;

  private:

    //
    // Internal utilities
    //

    //! Return a pointer-to-const to the actual array as its native type.
    ArrayImpl<String> const *typedArrayPointer() const;

    //! Return a pointer to the actual array as its native type.
    ArrayImpl<String> *typedArrayPointer();

  };

  typedef ArrayVariableImpl<Boolean> BooleanArrayVariable;
  typedef ArrayVariableImpl<Integer> IntegerArrayVariable;
  typedef ArrayVariableImpl<Real>    RealArrayVariable;
  typedef ArrayVariableImpl<String>  StringArrayVariable;

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_VARIABLE_HH
