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

  class Function : public Mutable
  {
  public:
    Function();
    ~Function();

    /**
     * @brief Add a subexpression to this expression.
     * @param exp The subexpression.
     * @param isGarbage True if this expression should delete the subexpression at destructor time.
     */
    void addSubexpression(const ExpressionId &exp, bool isGarbage = true);

    /**
     * @brief Indicate that this Function is complete,
     *        so that the implementation can perform any needed checks.
     * @note The default method does nothing.
     */
    virtual void finalize();

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

  private:
    // Not implemented
    Function(const Function &);
    Function& operator=(const Function &);

    std::vector<ExpressionId> m_subexpressions;
    std::vector<bool> m_garbage;
  };

  /**
   * @class UnaryFunction
   * @brief An abstract base class. Optimization of Function for one-argument operators.
   */

  class UnaryFunction : public Mutable
  {
  public:
    UnaryFunction();
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

  private:
    // Disallow copy
    UnaryFunction(const UnaryFunction &);
    UnaryFunction &operator=(const UnaryFunction &);

    ExpressionId m_e;
    bool m_garbage;
  };

  /**
   * @class BinaryFunction
   * @brief An abstract base class. Optimization of Function for two-argument operators.
   */

  class BinaryFunction : public Mutable
  {
  public:
    BinaryFunction();
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

  private:
    // Disallow copy
    BinaryFunction(const BinaryFunction &);
    BinaryFunction &operator=(const BinaryFunction &);

    ExpressionId m_a;
    ExpressionId m_b;
    bool m_aGarbage;
    bool m_bGarbage;
  };

} // namespace PLEXIL

#endif // PLEXIL_FUNCTION_HH
