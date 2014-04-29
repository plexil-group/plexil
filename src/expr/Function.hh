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

#ifndef PLEXIL_FUNCTION_HH
#define PLEXIL_FUNCTION_HH

#include "Mutable.hh"
#include "Operator.hh"

namespace PLEXIL
{

  // TODO:
  // - Do we need cacheable derivative of this?
  // Maybe an expression wrapper?
  // - Should these be templatized based on the expected return type?
  // - Check parameter types at plan load time

  /**
   * @class Function
   * @brief An abstract base class.
   * Represents a function whose value depends on the value(s) of one or more subexpressions.
   */

  template <typename R>
  class Function : public Mutable
  {
  public:
    virtual ~Function();

    /**
     * @brief Get the value type of this Expression.
     * @return ValueType of the expression.
     */
    const ValueType getValueType() const;

    /**
     * @brief Retrieve the value of this Expression.
     * @param The appropriately typed place to put the result.
     * @return True if result known, false if unknown.
     */
    bool getValue(R &result) const;

  protected:

    // Constructor should only be used by derived classes
    Function(const Operator<R>* op);

    /**
     * @brief Calculate the function's value from the current values of the subexpressions.
     * @param result The place to put the result.
     * @return True if known, false if not.
     */
    virtual bool calculate(R &result) const = 0;

    const Operator<R>* m_op;

  private:
    // Not implemented
    Function();
    Function(const Function &);
    Function& operator=(const Function &);
  };

  /**
   * @class UnaryFunction
   * @brief An abstract base class. A Function of one argument.
   */

  template <typename R>
  class UnaryFunction : public Function<R>
  {
  public:
    UnaryFunction(const Operator<R>* op,
                  const ExpressionId & exp,
                  bool isGarbage);
    virtual ~UnaryFunction();

  protected:

    /**
     * @brief Make this expression active.
     * This implementation calls the activate() method on the subexpression.
     */
    void handleActivate();

    /**
     * @brief Make this expression inactive.
     * This implementation calls the deactivate() method on the subexpression.
     */
    void handleDeactivate();

    /**
     * @brief Calculate the function's value from the current values of the subexpressions.
     * @param result The place to put the result.
     * @return True if known, false if not.
     */
    bool calculate(R &result) const;

  private:

    // Disallow default constructor, copy
    UnaryFunction();
    UnaryFunction(const UnaryFunction &);
    UnaryFunction &operator=(const UnaryFunction &);

    ExpressionId m_a;
    bool m_aGarbage;
  };

  /**
   * @class BinaryFunction
   * @brief An abstract base class. Optimization of Function for two-argument operators.
   */

  template <typename R>
  class BinaryFunction : public Function<R>
  {
  public:
    BinaryFunction(const Operator<R>* op,
                   const ExpressionId & expA,
                   const ExpressionId & expB,
                   bool isGarbageA,
                   bool isGarbageB);

    virtual ~BinaryFunction();

  protected:

    /**
     * @brief Make this expression active.
     * This implementation calls the activate() method on both subexpressions.
     */
    void handleActivate();

    /**
     * @brief Make this expression inactive.
     * This implementation calls the deactivate() method on both subexpressions.
     */
    void handleDeactivate();

    /**
     * @brief Calculate the function's value from the current values of the subexpressions.
     * @param result The place to put the result.
     * @return True if known, false if not.
     */
    bool calculate(R &result) const;

  private:

    // Disallow default constructor, copy
    BinaryFunction();
    BinaryFunction(const BinaryFunction &);
    BinaryFunction &operator=(const BinaryFunction &);

    ExpressionId m_a;
    ExpressionId m_b;
    bool m_aGarbage;
    bool m_bGarbage;
  };

  template <typename R>
  class NaryFunction : public Function<R>
  {
  public:
    NaryFunction(const Operator<R> *op,
                 const std::vector<ExpressionId> &exps,
                 const std::vector<bool> &garbage);
    ~NaryFunction();

  protected:

    /**
     * @brief Make this expression active.
     * This implementation calls the activate() method on all subexpressions.
     */
    void handleActivate();

    /**
     * @brief Make this expression inactive.
     * This implementation calls the deactivate() method on all subexpressions.
     */
    void handleDeactivate();

    /**
     * @brief Calculate the function's value from the current values of the subexpressions.
     * @param result The place to put the result.
     * @return True if known, false if not.
     */
    bool calculate(R &result) const;

  private:

    // Disallow default constructor, copy
    NaryFunction();
    NaryFunction(const NaryFunction &);
    NaryFunction& operator=(const NaryFunction &);

    std::vector<ExpressionId> m_subexpressions;
    std::vector<bool> m_garbage;
  };

} // namespace PLEXIL

#endif // PLEXIL_FUNCTION_HH
