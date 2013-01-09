/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#ifndef CALCULABLES_HH
#define CALCULABLES_HH

#include "Calculable.hh"

namespace PLEXIL
{

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

  class LogicalNegation : public UnaryExpression
  {
  public:
    LogicalNegation(const PlexilExprId& expr, const NodeConnectorId& node)
      : UnaryExpression(expr, node)
    {
    }

    LogicalNegation(ExpressionId& e)
      : UnaryExpression(e)
    {
    }

    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

    double recalculate();

  protected:
	const char* operatorString() const { return "!"; }

  private:
    bool checkValue(const double val);
  };

  class AbsoluteValue : public UnaryExpression {
  public:
    AbsoluteValue(const PlexilExprId& expr, const NodeConnectorId& node);
    AbsoluteValue(ExpressionId e);
    void print(std::ostream& s) const;
    double recalculate();
    bool checkValue(const double val);
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
	const char* operatorString() const { return "abs"; }

  private:
  };

  class SquareRoot : public UnaryExpression {
  public:
    SquareRoot(const PlexilExprId& expr, const NodeConnectorId& node);
    SquareRoot(ExpressionId e);
    void print(std::ostream& s) const;
    double recalculate();
    bool checkValue(const double val);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return REAL; }

  protected:
	const char* operatorString() const { return "sqrt"; }
  };

  class IsKnown : public UnaryExpression {
  public:
    IsKnown(const PlexilExprId& expr, const NodeConnectorId& node);
    IsKnown(ExpressionId e);
    void print(std::ostream& s) const;
    double recalculate();
    bool checkValue(const double val);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
	const char* operatorString() const { return "isknown"; }

  };

  class Conjunction : public NaryExpression {
  public:
    Conjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node) {}
    Conjunction(const ExpressionId& a, const ExpressionId& b) : NaryExpression(a, b) {}
    Conjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "&&"; }

  private:
    bool checkValue(const double value);
  };

  class Disjunction : public NaryExpression
  {
  public:
    Disjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node)
    {
    }

    Disjunction(const ExpressionId& a, const ExpressionId& b)
      : NaryExpression(a, b)
    {
    }
    Disjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "||"; }

  private:
    bool checkValue(const double value);
  };

  class ExclusiveDisjunction : public NaryExpression
  {
  public:
    ExclusiveDisjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node) {}

    ExclusiveDisjunction(const ExpressionId& a, const ExpressionId& b)
      : NaryExpression(a, b) {}
    ExclusiveDisjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "^^"; }

  private:
    bool checkValue(const double value);
  };

  class Concatenation : public NaryExpression {
  public:
    Concatenation(const PlexilExprId& expr, const NodeConnectorId& node);
    Concatenation(const ExpressionId& a, const ExpressionId& b);
    Concatenation(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage);

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return STRING; }

  protected:
    const char* operatorString() const { return "+"; }

  private:
    LabelStr m_label;

    bool checkValue(const double value);
  };

  //
  // Comparisons
  //

  class Equality : public BinaryExpression {
  public:
    Equality(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node) {}
    Equality(const ExpressionId& a, const ExpressionId& b) : BinaryExpression(a, b) {}
    Equality(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "=="; }

  private:
    bool checkValue(const double value);
  };

  class Inequality : public BinaryExpression
  {
  public:
    Inequality(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Inequality(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Inequality(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "!="; }

  private:
    bool checkValue(const double value);
  };

  class LessThan : public BinaryExpression
  {
  public:
    LessThan(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    LessThan(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    LessThan(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "<"; }

  private:
    bool checkValue(const double value);
  };

  class LessEqual : public BinaryExpression
  {
  public:
    LessEqual(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    LessEqual(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    LessEqual(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return "<="; }

  private:
    bool checkValue(const double value);
  };

  class GreaterThan : public BinaryExpression
  {
  public:
    GreaterThan(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    GreaterThan(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    GreaterThan(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return ">"; }

  private:
    bool checkValue(const double value);
  };

  class GreaterEqual : public BinaryExpression
  {
  public:
    GreaterEqual(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    GreaterEqual(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    GreaterEqual(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    const char* operatorString() const { return ">="; }

  private:
    bool checkValue(const double value);
  };


  //
  // Arithmetic expressions
  //

  class Addition : public BinaryExpression
  {
  public:
    Addition(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Addition(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Addition(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "+"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Subtraction : public BinaryExpression
  {
  public:
    Subtraction(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Subtraction(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Subtraction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "-"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Multiplication : public BinaryExpression
  {
  public:
    Multiplication(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Multiplication(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Multiplication(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "*"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Division : public BinaryExpression
  {
  public:
    Division(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Division(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Division(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "/"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };


  class Modulo : public BinaryExpression
  {
  public:
    Modulo(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Modulo(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Modulo(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "%"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Minimum : public BinaryExpression
  {
  public:
    Minimum(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Minimum(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Minimum(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    void print(std::ostream& s) const;
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "min"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Maximum : public BinaryExpression
  {
  public:
    Maximum(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Maximum(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Maximum(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    void print(std::ostream& s) const;
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
    const char* operatorString() const { return "max"; }

  private:
    bool checkValue(const double /* value */) {return true;}
  };

}

#endif // CALCULABLES_HH
