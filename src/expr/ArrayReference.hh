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

#ifndef PLEXIL_ARRAY_REFERENCE_HH
#define PLEXIL_ARRAY_REFERENCE_HH

#include "AssignableImpl.hh"
#include "ExpressionImpl.hh"
#include "Value.hh"

namespace PLEXIL {

  // Forward reference
  class Value;

  class ArrayReference : public NotifierImpl
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
    const ValueType valueType() const;
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
     * @note Limited type conversions supported.
     * @note Unimplemented conversions will cause a link time error.
     */
    virtual bool getValue(bool &) const;        // Boolean
    virtual bool getValue(double &) const;      // Real
    //virtual bool getValue(uint16_t &) const;    // enumerations: State, Outcome, Failure, etc.
    virtual bool getValue(int32_t &) const;     // Integer
    virtual bool getValue(std::string &) const; // String

    /**
     * @brief Get a pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(std::string const *&ptr) const;

    Value toValue() const;

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

  };

  /**
   * @class MutableArrayReference
   * @brief Expression class that represents a modifiable location in an array.
   */

  class MutableArrayReference : public ArrayReference, public Assignable
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
    void setValue(double const &val);
    void setValue(int32_t const &val);
    void setValue(uint16_t const &val);
    void setValue(bool const &val);
    void setValue(std::string const &val);
    void setValue(char const *val);

    // These will throw an exception
    void setValue(BooleanArray const &val);
    void setValue(IntegerArray const &val);
    void setValue(RealArray const &val);
    void setValue(StringArray const &val);

    /**
     * @brief Set the value for this expression from another expression.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    void setValue(Expression const *valex);

    /**
     * @brief Set the value for this expression from a generic Value.
     * @param val The Value.
     * @note May cause change notifications to occur.
     */
    void setValue(Value const &value);

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
