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

#ifndef PLEXIL_ARRAY_REFERENCE_HH
#define PLEXIL_ARRAY_REFERENCE_HH

#include "Assignable.hh"
#include "Expression.hh"
#include "Propagator.hh"
#include "PlexilTypeTraits.hh"
#include "Value.hh"

namespace PLEXIL {

  class ArrayVariable;

  class ArrayReference :
    virtual public Expression,
    public Propagator
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

    virtual char const *getName() const override;
    virtual char const *exprName() const override;
    virtual ValueType valueType() const override;
    virtual bool isKnown() const override;
    virtual bool isConstant() const override;
    virtual bool isAssignable() const override;
    virtual Expression *getBaseExpression() override;
    virtual Expression const *getBaseExpression() const override;
    virtual void printValue(std::ostream& s) const override;

    /**
     * @brief Get the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     * @note Unimplemented conversions will cause a link time error.
     */

    virtual bool getValue(Boolean &result) const override;
    virtual bool getValue(Integer &result) const override;
    virtual bool getValue(Real &result) const override;
    virtual bool getValue(String &result) const override;

    // These issue a PlanError
    virtual bool getValue(NodeState &result) const override;
    virtual bool getValue(NodeOutcome &result) const override;
    virtual bool getValue(FailureType &result) const override;
    virtual bool getValue(CommandHandleValue &result) const override;

    /**
     * @brief Get a pointer to the expression's value.
     * @param result The variable where the value will be stored.
     * @return True if known, false if unknown.
     */
    virtual bool getValuePointer(String const *&ptr) const override;

    virtual bool getValuePointer(Array const *&ptr) const override;
    virtual bool getValuePointer(BooleanArray const *&ptr) const override;
    virtual bool getValuePointer(IntegerArray const *&ptr) const override;
    virtual bool getValuePointer(RealArray const *&ptr) const override;
    virtual bool getValuePointer(StringArray const *&ptr) const override;

    virtual Value toValue() const override;

  protected:

    //
    // Notifier API
    //

    virtual void handleActivate() override;
    virtual void handleDeactivate() override;

    virtual void doSubexprs(ListenableUnaryOperator const &f) override;

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
    bool selfCheck(Array const *&ary,
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

    virtual bool isAssignable() const override;

    virtual Assignable const *asAssignable() const override;
    virtual Assignable *asAssignable() override;

    /**
     * @brief Assign the current value to UNKNOWN.
     */
    virtual void setUnknown() override;

    /**
     * @brief Set the value for this expression from a generic Value.
     * @param val The Value.
     * @note May cause change notifications to occur.
     */
    virtual void setValue(Value const &value) override;

    using Assignable::setValue;

    virtual void saveCurrentValue() override;
    virtual void restoreSavedValue() override;
    virtual Value getSavedValue() const override;

    virtual Expression *getBaseVariable() override;
    virtual Expression const *getBaseVariable() const override;

  private:
    // Default, copy, assignment disallowed
    MutableArrayReference();
    MutableArrayReference(const MutableArrayReference &);
    MutableArrayReference &operator=(const MutableArrayReference &);

    // Internal function
    bool mutableSelfCheck(size_t &idx);

    ArrayVariable *m_mutableArray;
    Value m_savedValue;
    bool m_saved;
  };

} // namespace PLEXIL

#endif // PLEXIL_ARRAY_REFERENCE_HH
