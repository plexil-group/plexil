/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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

#ifndef CALCULABLE_HH
#define CALCULABLE_HH

#include "Expression.hh"

namespace PLEXIL
{

  /**
   * A class representing expressions that require calculation.
   */
  class Calculable : public Expression {
  public:

    /**
     * @brief Constructor.
     */
    Calculable();

    /**
     * @brief Constructor
     * @param expr The PlexilExpr for this expression.
     * @param node The scope in which this expression is evaluated.
     */
    Calculable(const PlexilExprId& expr, const NodeConnectorId& node);

    virtual ~Calculable();

    /**
     * @brief By default, Calculables can't have their value set this way.  However,
     *        it should be possible for subclasses to override this behavior.
     */
    virtual void setValue(const Value& value);

    /**
     * @brief Re-calculate the value of this expression.
     */
    virtual Value recalculate() {return m_value;}

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

  protected:
    /**
     * @brief A utility function for accessing a variable in the current scope or creating
     *        variable for a literal value.
     * @param expr The PlexilExprId for the variable or value.
     * @param node The node which delimits the scope of the variable search.
     * @param del Set to true if the destination should be garbage collected along with this
     *            expression (i.e. if a new variable had to be allocated.).
     */
    virtual ExpressionId getSubexpression(const PlexilExprId& expr, 
                                          const NodeConnectorId& node,
                                          bool& del);

    /**
     * @brief Handles the activation of this expression, including activation of
     *        subexpressions.  By default, when the expression goes from inactive to
     *        active, it re-calculates its value.
     * @param changed True if this expression has gone from inactive to active.
     */
    virtual void handleActivate(const bool changed);

    /**
     * @brief Handles the deactivation of this expression, including the deactivation of
     *        subexpressions.
     * @param changed True if this expression has gone from inactive to active.
     */
    virtual void handleDeactivate(const bool changed);

    /**
     * @brief A method for subclasses to handle the change of a subexpression's value.
     * @param exp The subexpression whose value has changed.
     * @note The default method is a no-op.
     * @note As of 25 Nov 2008, there appear to be no other implementations of this method.
     */
    virtual void handleSubexpressionChange(const ExpressionId& /* exp */) {}

    void addSubexpression(const ExpressionId& exp, const bool garbage);

    bool containsSubexpression(const ExpressionId& expr);

    void removeSubexpression(const ExpressionId& exp);

    // Printer utilities for use by subclasses
    virtual void printAsFnCall(std::ostream& s) const;
    virtual void printAsInfix(std::ostream& s) const;
    virtual const char* operatorString() const = 0;

    SubexpressionListener m_listener; /*<! For incoming message notifications (other expressions' values have changed) */
    ExpressionVector m_subexpressions; /*<! The subexpressions.*/
    std::set<ExpressionId> m_garbage;
  };

  /**
   * Calculables MUST override
   * checkValue
   *
   * Calculables SHOULD override
   * toString
   * recalculate
   *
   * Calculables MAY override
   * valueString
   * setValue
   * handleLock
   * handleUnlock
   * handleActivate
   * handleDeactivate
   * handleChange
   * getVariable
   * handleSubexpressionChange
   */

  class UnaryExpression : public Calculable {
  public:
    UnaryExpression(const PlexilExprId& expr, const NodeConnectorId& node);
    UnaryExpression(const ExpressionId& e);
  protected:
    ExpressionId m_e;
  };

  class BinaryExpression : public Calculable {
  public:
    BinaryExpression(const PlexilExprId& expr, const NodeConnectorId& node);
    BinaryExpression(const ExpressionId& a, const ExpressionId& b);
    BinaryExpression(const ExpressionId& a, bool aGarbage, const ExpressionId& b,
					 bool bGarbage);
	void print(std::ostream& s) const;

  protected:
    ExpressionId m_a;
    ExpressionId m_b;
  };

  // N-ary expression
  class NaryExpression : public Calculable
  {
  public:
    NaryExpression(const PlexilExprId& expr, 
                   const NodeConnectorId& node);
    NaryExpression(const ExpressionId& a, const ExpressionId& b);
    NaryExpression(const ExpressionId& a, bool aGarbage,
                   const ExpressionId& b, bool bGarbage);
	void print(std::ostream& s) const;
  };

}

#endif // CALCULABLE_HH
