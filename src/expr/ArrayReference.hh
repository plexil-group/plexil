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
#include "PlexilTypeTraits.hh"
#include "Value.hh"

namespace PLEXIL {

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

    virtual bool getValue(Boolean &result) const;
    virtual bool getValue(Integer &result) const;
    virtual bool getValue(Real &result) const;
    virtual bool getValue(String &result) const;

    // This issues a PlanError
    virtual bool getValue(uint16_t &result) const;

    /**
     * @brief Get a pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(String const *&ptr) const;

    virtual bool getValuePointer(Array const *&ptr) const;
    virtual bool getValuePointer(BooleanArray const *&ptr) const;
    virtual bool getValuePointer(IntegerArray const *&ptr) const;
    virtual bool getValuePointer(RealArray const *&ptr) const;
    virtual bool getValuePointer(StringArray const *&ptr) const;

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
  };

  /**
   * @class MutableArrayReference
   * @brief Expression class that represents a modifiable location in an array.
   */

  class MutableArrayReference :
    public Assignable,
    public ArrayReference
  {
  public:
    MutableArrayReference(Expression *ary,
                          Expression *idx,
                          bool aryIsGarbage = false,
                          bool idxIsGarbage = false);

    ~MutableArrayReference();

    virtual bool isAssignable() const;

    virtual Assignable const *asAssignable() const;
    virtual Assignable *asAssignable();

    /**
     * @brief Reset the expression.
     */
    virtual void reset();

    /**
     * @brief Assign the current value to UNKNOWN.
     */
    virtual void setUnknown();

    /**
     * @brief Assign a new value.
     * @param value The value to assign.
     */
    virtual void setValue(Boolean const &val);
    virtual void setValue(Integer const &val);
    virtual void setValue(Real const &val);
    virtual void setValue(String const &val);

    /**
     * @brief Set the value for this expression from another Expression.
     * @param valex The expression from which to obtain the new value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(Expression const &valex);

    /**
     * @brief Set the value for this expression from a generic Value.
     * @param val The Value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(Value const &value);

    using Assignable::setValue;

    /**
     * @brief Retrieve a writable pointer to the value.
     * @param valuePtr Reference to the pointer variable
     * @return True if the value is known, false if unknown or invalid.
     * @note Default method returns false and reports a type error.
     */
    bool getMutableValuePointer(std::string *& ptr);

    // Throws an exception
    virtual bool getMutableValuePointer(Array *& ptr);

    virtual void saveCurrentValue();
    virtual void restoreSavedValue();
    virtual Value getSavedValue() const;

    virtual NodeConnector const *getNode() const;
    virtual NodeConnector *getNode();

    virtual Expression *getBaseVariable();
    virtual Expression const *getBaseVariable() const;

  protected:

    // Wrap NotifierImpl method
    void publishChange(Expression const *src); // N.B.: NOT VIRTUAL!!!

  private:
    // Default, copy, assignment disallowed
    MutableArrayReference();
    MutableArrayReference(const MutableArrayReference &);
    MutableArrayReference &operator=(const MutableArrayReference &);

    // Internal function
    bool mutableSelfCheck(Array *&ary, size_t &idx);

    // FIXME: make this a pointer to ArrayVariable
    Assignable *m_mutableArray;
    Value m_savedValue;
    bool m_saved;
  };

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_REFERENCE_HH
