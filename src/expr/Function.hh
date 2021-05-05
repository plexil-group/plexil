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

#ifndef PLEXIL_FUNCTION_HH
#define PLEXIL_FUNCTION_HH

#include "ArrayFwd.hh"
#include "Expression.hh"
#include "Propagator.hh"
#include "Value.hh"
#include "ValueType.hh"

namespace PLEXIL
{
  // Forward reference
  class Operator;

  /**
   * @class Function
   * @brief Represents an expression whose value depends on
   *        the value(s) of zero or more subexpressions.
   */

  class Function :
    public Expression,
    public Propagator
  {
  public:
    virtual ~Function();

    //
    // Expression API
    //

    virtual const char *exprName() const override;
    virtual ValueType valueType() const override;
    virtual bool isKnown() const override;
    virtual void printValue(std::ostream &s) const override;
    virtual Value toValue() const override;

    // Delegated to implementation classes

    // Argument accessors

    virtual size_t size() const = 0;
    virtual void setArgument(size_t i, Expression *expr, bool garbage) = 0;
    virtual Expression const *operator[](size_t n) const = 0;

    /**
     * @brief Retrieve the value of this Expression in its native form.
     * @param The appropriately typed place to put the result.
     * @return True if result known, false if unknown.
     * @note Derived classes may override the default methods for performance.
     */
    virtual bool getValue(Boolean &result) const override;
    virtual bool getValue(Integer &result) const override;
    virtual bool getValue(Real &result) const override;
    virtual bool getValue(String &result) const override;

    virtual bool getValue(NodeState &result) const override;
    virtual bool getValue(NodeOutcome &result) const override;
    virtual bool getValue(FailureType &result) const override;
    virtual bool getValue(CommandHandleValue &result) const override;

    /**
     * @brief Retrieve a pointer to the (const) value of this Expression.
     * @param ptr Reference to the pointer variable to receive the result.
     * @return True if known, false if unknown.
     * @note Derived classes may override the default methods for performance.
     */
    virtual bool getValuePointer(String const *&ptr) const override;

    // Maybe later?
    virtual bool getValuePointer(Array const *&ptr) const override;
    virtual bool getValuePointer(BooleanArray const *&ptr) const override;
    virtual bool getValuePointer(IntegerArray const *&ptr) const override;
    virtual bool getValuePointer(RealArray const *&ptr) const override;
    virtual bool getValuePointer(StringArray const *&ptr) const override;

    /**
     * @brief Query whether this expression is a source of change events.
     * @return True if the value may change independently of any subexpressions, false otherwise.
     * @note Delegated to the operator.
     */
    virtual bool isPropagationSource() const override;

    // Needed by Operator::calcNative for array types
    virtual bool apply(Operator const *op, Array &result) const;

  protected:

    // Constructor only available to derived classes
    Function(Operator const *op);

    Operator const *m_op;

  private:

    // Not implemented
    Function() = delete;
    Function(Function const &) = delete;
    Function(Function &&) = delete;
    Function& operator=(Function const &) = delete;
    Function& operator=(Function &&) = delete;
  };

  // Factory functions
  extern Function *makeFunction(Operator const *op,
                                size_t nargs);

  // Convenience wrappers for Node classes and unit test
  extern Function *makeFunction(Operator const *op,
                                Expression *expr,
                                bool garbage);

  extern Function *makeFunction(Operator const *op, 
                                Expression *expr1,
                                Expression *expr2,
                                bool garbage1,
                                bool garbage2);

} // namespace PLEXIL

#endif // PLEXIL_FUNCTION_HH
