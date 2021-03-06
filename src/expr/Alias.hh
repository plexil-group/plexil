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

#ifndef PLEXIL_ALIAS_HH
#define PLEXIL_ALIAS_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{
  // Forward declaration
  class Value;

  /**
   * @class Alias
   * @brief A read-only proxy for another expression.
   * @note Most commonly used in library nodes, but also anywhere
   *       read-only access to a mutable expression is needed.
   */
  class Alias :
    public Expression,
    public Propagator
  {
  public:
    Alias(char const *name,
          Expression *original = nullptr,
          bool garbage = false);
    virtual ~Alias();

    //
    // Expression API
    //

    virtual char const *getName() const override;
    virtual char const *exprName() const override;
    virtual ValueType valueType() const override;
    virtual bool isKnown() const override;
    virtual bool isAssignable() const override;
    virtual bool isConstant() const override;
    virtual Expression *getBaseExpression() override;
    virtual Expression const *getBaseExpression() const override;

    virtual void printValue(std::ostream &s) const override;

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if known, false if unknown or invalid.
     */
    virtual bool getValue(Boolean &var) const override;
    virtual bool getValue(Integer &var) const override;
    virtual bool getValue(Real &var) const override;
    virtual bool getValue(NodeState &) const override;
    virtual bool getValue(NodeOutcome &) const override;
    virtual bool getValue(FailureType &) const override;
    virtual bool getValue(CommandHandleValue &) const override;
    virtual bool getValue(String &var) const override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown or invalid.
     */
    virtual bool getValuePointer(String const *&ptr) const override;
    virtual bool getValuePointer(Array const *&ptr) const override;
    virtual bool getValuePointer(BooleanArray const *&ptr) const override;
    virtual bool getValuePointer(IntegerArray const *&ptr) const override;
    virtual bool getValuePointer(RealArray const *&ptr) const override;
    virtual bool getValuePointer(StringArray const *&ptr) const override;

    /**
     * @brief Get the value of this expression as a Value instance.
     * @return The Value instance.
     */
    virtual Value toValue() const override;

  protected:

    virtual void doSubexprs(ListenableUnaryOperator const &f) override;

    // The expression being aliased.
    Expression *m_exp;
    // Name in the owning node
    char const *m_name;

  private:

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
