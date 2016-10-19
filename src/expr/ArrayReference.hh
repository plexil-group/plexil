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

#ifndef PLEXIL_ARRAY_REFERENCE_HH
#define PLEXIL_ARRAY_REFERENCE_HH

#include "Assignable.hh"
#include "NotifierImpl.hh"
#include "Value.hh"

namespace PLEXIL {

  // Forward reference
  class Value;

  class ArrayReference : public NotifierImpl,
                         virtual public Expression
  {
  public:
    ArrayReference(Expression *ary,
                   Expression *idx,
                   bool aryIsGarbage = false,
                   bool idxIsGarbage = false);

    ~ArrayReference();

    //
    // Essential Expression API
    //

    char const *getName() const;
    char const *exprName() const;
    ValueType valueType() const;
    bool isKnown() const;
    bool isConstant() const;
    bool isAssignable() const;
    Expression *getBaseExpression();
    Expression const *getBaseExpression() const;
    void printValue(std::ostream& s) const;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     * @note Unimplemented conversions will cause a link time error.
     */

    // Local macro
#define DEFINE_AREF_GET_VALUE_METHOD_SHIM(_type_)	\
    bool getValue(_type_ &result) const			\
    { return getValueImpl(result); }

    DEFINE_AREF_GET_VALUE_METHOD_SHIM(Boolean)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(NodeState)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(NodeOutcome)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(FailureType)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(CommandHandleValue)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(Integer)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(Real)
    DEFINE_AREF_GET_VALUE_METHOD_SHIM(String)

#undef DEFINE_AREF_GET_VALUE_METHOD_SHIM

    /**
     * @brief Get a pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(String const *&ptr) const;

    // Local macro
#define DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(_type_) \
    bool getValuePointer(_type_ const *&ptr) const \
    { return getValuePointerImpl(ptr); }

    DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(Array)
    DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(BooleanArray)
    DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(IntegerArray)
    DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(RealArray)
    DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM(StringArray)
#undef DEFINE_AREF_GET_VALUE_PTR_METHOD_SHIM

    Value toValue() const;

    // Wrapper for NotifierImpl method
    virtual void addListener(ExpressionListener *l);

    //
    // NotifierImpl API
    //

    void handleActivate();
    void handleDeactivate();

  protected:
    // State shared with MutableArrayReference
    Expression *m_array;
    Expression *m_index;

    bool m_arrayIsGarbage;
    bool m_indexIsGarbage;

    // For getName()
    std::string *m_namePtr;

  private:
    // Disallow default, copy, assignment
    ArrayReference();
    ArrayReference(const ArrayReference &);
    ArrayReference &operator=(const ArrayReference &);

    // Internal function
    bool selfCheck(Array const *&valuePtr,
                   size_t &idx) const;

    // Internal template functions
    template <typename R>
    bool getValueImpl(R &) const;

    template <typename T>
    bool getValuePointerImpl(T const *&) const;
  };

  /**
   * @class MutableArrayReference
   * @brief Expression class that represents a modifiable location in an array.
   */

  class MutableArrayReference :
    public ArrayReference,
    public Assignable
  {
  public:
    MutableArrayReference(Expression *ary,
                          Expression *idx,
                          bool aryIsGarbage = false,
                          bool idxIsGarbage = false);

    ~MutableArrayReference();

    bool isAssignable() const;

    Assignable const *asAssignable() const;
    Assignable *asAssignable();

    /**
     * @brief Reset the expression.
     */
    void reset();

    /**
     * @brief Assign the current value to UNKNOWN.
     */
    void setUnknown();

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    template <typename V>
    void setValueImpl(V const &);

    template <typename V>
    void setValueImpl(ArrayImpl<V> const &);

    // Instantiations of above
    virtual void setValue(Boolean const &val)
    { setValueImpl(val); }
    virtual void setValue(Real const &val)
    { setValueImpl(val); }
    virtual void setValue(String const &val)
    { setValueImpl(val); }

    virtual void setValue(BooleanArray const &val)
    { setValueImpl(val); }
    virtual void setValue(IntegerArray const &val)
    { setValueImpl(val); }
    virtual void setValue(RealArray const &val)
    { setValueImpl(val); }
    virtual void setValue(StringArray const &val)
    { setValueImpl(val); }

    // Specialized
    virtual void setValue(Integer const &);
    virtual void setValue(char const *val);

    // Not implemented (will assert)
    virtual void setValue(NodeState const &);
    virtual void setValue(NodeOutcome const &);
    virtual void setValue(FailureType const &);
    virtual void setValue(CommandHandleValue const &);

    /**
     * @brief Set the value for this expression from another GetValue object.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(GetValue const &valex);

    /**
     * @brief Set the value for this expression from a generic Value.
     * @param val The Value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(Value const &value);

    using Assignable::setValue;

    /**
     * @brief Retrieve a writable ponter to the value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a type error.
     */
    bool getMutableValuePointer(std::string *& ptr);

    // These will throw an exception
    bool getMutableValuePointer(Array *& ptr);
    bool getMutableValuePointer(BooleanArray *& ptr);
    bool getMutableValuePointer(IntegerArray *& ptr);
    bool getMutableValuePointer(RealArray *& ptr);
    bool getMutableValuePointer(StringArray *& ptr);

    void saveCurrentValue();
    void restoreSavedValue();
    Value getSavedValue() const;

    NodeConnector const *getNode() const;
    NodeConnector *getNode();

    Assignable *getBaseVariable();
    Assignable const *getBaseVariable() const;

  private:
    // Default, copy, assignment disallowed
    MutableArrayReference();
    MutableArrayReference(const MutableArrayReference &);
    MutableArrayReference &operator=(const MutableArrayReference &);

    // Internal function
    bool mutableSelfCheck(Array *&ary, size_t &idx);

    Assignable *m_mutableArray;
    Value m_savedValue;
    bool m_saved;
  };

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_REFERENCE_HH
