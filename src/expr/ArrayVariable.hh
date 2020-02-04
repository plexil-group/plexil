/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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

#include "Assignable.hh"
#include "GetValueImpl.hh"
#include "Notifier.hh"

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
    public Assignable,
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

    virtual bool isAssignable() const override;

    virtual Assignable const *asAssignable() const override;
    virtual Assignable *asAssignable() override;

    virtual char const *getName() const override;

    virtual char const *exprName() const override;

    virtual bool isKnown() const override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(Array const *&ptr) const override;

    //
    // Assignable API
    //

    virtual void saveCurrentValue() override;

    // Provided by derived class
    // virtual void restoreSavedValue() override;

    virtual Value getSavedValue() const override;

    virtual void setInitializer(Expression *expr, bool garbage) override;

    virtual void setUnknown() override;

    virtual void setValue(Value const &val) override;

    virtual Assignable *getBaseVariable() override;
    virtual Assignable const *getBaseVariable() const override;

    virtual bool isInUse() const override;
    virtual bool reserve(Node *node) override;
    virtual void release() override;

    virtual void addWaitingNode(Node *node) override;
    virtual void removeWaitingNode(Node *node) override;
    virtual std::vector<Node *> const *getWaitingNodes() const override;

    /**
     * @brief Set the value for this object.
     * @param val The expression with the new value for this object.
     */
    virtual void setValue(Expression const &val);

    //
    // Access needed by ArrayReference
    //

    bool elementIsKnown(size_t idx) const;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result variable.
     * @return True if the value is known, false otherwise.
     * @note Default methods throw a PlanError.
     */
    virtual bool getElement(size_t idx, Boolean &result) const; 
    virtual bool getElement(size_t idx, Integer &result) const;
    virtual bool getElement(size_t idx, Real &result) const;
    virtual bool getElement(size_t idx, String &result) const;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result pointer variable.
     * @return True if the value is known, false otherwise.
     * @note Default methods throw a PlanError.
     */
    virtual bool getElementPointer(size_t idx, String const *&ptr) const;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @return The value; may be unknown.
     */
    virtual Value getElementValue(size_t idx) const;

    //
    // Access needed by MutableArrayReference
    //

    virtual void setElement(size_t idx, Value const &value) = 0;

    void setElementUnknown(size_t idx);

  protected:

    //
    // Expression internal API
    //

    virtual void printSpecialized(std::ostream &s) const override;

    //
    // NotifierImpl API
    //

    virtual void handleActivate() override;
    virtual void handleDeactivate() override;

    //
    // API to derived classes
    //

    /**
     * @brief Default constructor.
     */
    ArrayVariable();

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     * @param size Expression whose value is the initial size of the array.
     * @param sizeIsGarbage True if the size expression should be deleted 
     *                      with the array variable.
     */
    ArrayVariable(char const *name,
                  Expression *size = nullptr,
                  bool sizeIsGarbage = false);

    /**
     * @brief Copy from a generic array.
     * @param a Pointer to array whose contents are to be copied.
     */
    virtual void setValueImpl(Array const *a) = 0;

    //
    // API provided by derived classes
    //

    /**
     * @brief Compare the given array with the current value.
     * @return True if equal, false if not.
     */

    virtual bool equals(Array const *ary) const = 0;

    /**
     * @brief Construct an array of the appropriate type and requested size.
     * @return Pointer to an Array.
     */

    virtual Array *makeArray(size_t n) const = 0;

    //
    // Member variables
    //

    //! Nodes waiting to assign to this variable. Usually empty.
    std::vector<Node *> m_waiters;

    std::unique_ptr<Array> m_value;
    std::unique_ptr<Array> m_savedValue;   // for undoing assignment 

    Expression *m_size;
    Expression *m_initializer;
    char const *m_name;
    size_t m_maxSize;

    Node *m_user;

    bool m_known;
    bool m_savedKnown;
    bool m_sizeIsGarbage;
    bool m_initializerIsGarbage;
    bool m_sizeIsConstant;
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

    /**
     * @brief Default constructor.
     */
    ArrayVariableImpl();

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     * @param size Expression whose value is the initial size of the array.
     * @param sizeIsGarbage True if the size expression should be deleted 
     *                      with the array variable.
     */
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note Just defer to ArrayVariable method.
     */
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    virtual void setValueImpl(Array const *a) override;

    virtual void restoreSavedValue() override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     * @note Simply delegates to ArrayVariable method.
     */
    virtual bool getValuePointer(Array const *& ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(ArrayImpl<T> const *&ptr) const override;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result variable.
     * @return True if the value is known, false otherwise.
     */
    virtual bool getElement(size_t idx, T &result) const override; 

    virtual void setElement(size_t idx, Value const &value) override;

    //
    // API to base class
    //

    /**
     * @brief Compare the given array with the current value.
     * @return True if equal, false if not.
     */

    virtual bool equals(Array const *ary) const override;

    /**
     * @brief Construct an array of the appropriate type and requested size.
     * @return Pointer to an Array.
     */

    virtual Array *makeArray(size_t n) const override;

  private:

    //
    // Internal utilities
    //

    ArrayImpl<T> const *typedArrayPointer() const;
    ArrayImpl<T> *typedArrayPointer();

  };

  template <>
  class ArrayVariableImpl<Integer> final :
    public GetValueImpl<ArrayImpl<Integer> >,
    public ArrayVariable
  {
  public:

    /**
     * @brief Default constructor.
     */
    ArrayVariableImpl();

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     * @param size Expression whose value is the initial size of the array.
     * @param sizeIsGarbage True if the size expression should be deleted 
     *                      with the array variable.
     */
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note Just defer to ArrayVariable method.
     */
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    virtual void setValueImpl(Array const *a) override;

    virtual void restoreSavedValue() override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     * @note Simply delegates to ArrayVariable method.
     */
    virtual bool getValuePointer(Array const *& ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(ArrayImpl<Integer> const *&ptr) const override;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result variable.
     * @return True if the value is known, false otherwise.
     */
    virtual bool getElement(size_t idx, Integer &result) const override; 

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result variable.
     * @return True if the value is known, false otherwise.
     * @note Conversion method.
     */
    virtual bool getElement(size_t idx, Real &result) const override; 

    virtual void setElement(size_t idx, Value const &value) override;

    /**
     * @brief Compare the given array with the current value.
     * @return True if equal, false if not.
     */

    virtual bool equals(Array const *ary) const override;

    /**
     * @brief Construct an array of the appropriate type and requested size.
     * @return Pointer to an Array.
     */

    virtual Array *makeArray(size_t n) const override;

  private:

    //
    // Internal utilities
    //

    ArrayImpl<Integer> const *typedArrayPointer() const;
    ArrayImpl<Integer> *typedArrayPointer();

  };

  template <>
  class ArrayVariableImpl<String> final :
    public GetValueImpl<ArrayImpl<String> >,
    public ArrayVariable
  {
  public:

    /**
     * @brief Default constructor.
     */
    ArrayVariableImpl();

    /**
     * @brief Constructor for plan loading.
     * @param name The name of this variable in the parent node.
     * @param size Expression whose value is the initial size of the array.
     * @param sizeIsGarbage True if the size expression should be deleted 
     *                      with the array variable.
     */
    ArrayVariableImpl(char const *name,
                      Expression *size = nullptr,
                      bool sizeIsGarbage = false);

    virtual ~ArrayVariableImpl() = default;

    /**
     * @brief Return the value type.
     * @return A constant enumeration.
     */
    virtual ValueType valueType() const override;

    /**
     * @brief Determine whether the value is known or unknown.
     * @return True if known, false otherwise.
     * @note Just defer to ArrayVariable method.
     */
    virtual bool isKnown() const override
    {
      return ArrayVariable::isKnown();
    }

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    virtual void setValueImpl(Array const *a) override;

    virtual void restoreSavedValue() override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     * @note Simply delegates to ArrayVariable method.
     */
    virtual bool getValuePointer(Array const *& ptr) const override
    {
      return ArrayVariable::getValuePointer(ptr);
    }
    
    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(ArrayImpl<String> const *&ptr) const override;

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result variable.
     * @return True if the value is known, false otherwise.
     */
    virtual bool getElement(size_t idx, String &result) const override; 

    /**
     * @brief Get the value of the element of the array.
     * @param idx The index.
     * @param result Reference to the result pointer variable.
     * @return True if the value is known, false otherwise.
     */
    virtual bool getElementPointer(size_t idx, String const *&ptr) const override;

    virtual void setElement(size_t idx, Value const &value) override;

    /**
     * @brief Compare the given array with the current value.
     * @return True if equal, false if not.
     */

    virtual bool equals(Array const *ary) const override;

    /**
     * @brief Construct an array of the appropriate type and requested size.
     * @return Pointer to an Array.
     */

    virtual Array *makeArray(size_t n) const override;

  private:

    //
    // Internal utilities
    //

    ArrayImpl<String> const *typedArrayPointer() const;
    ArrayImpl<String> *typedArrayPointer();

  };

  typedef ArrayVariableImpl<Boolean> BooleanArrayVariable;
  typedef ArrayVariableImpl<Integer> IntegerArrayVariable;
  typedef ArrayVariableImpl<Real>    RealArrayVariable;
  typedef ArrayVariableImpl<String>  StringArrayVariable;

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_VARIABLE_HH
