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

#include "Calculable.hh"
#include "ExpressionFactory.hh"
#include "Utils.hh" // for cleanup template
#include "Variable.hh"

namespace PLEXIL
{

  Calculable::Calculable() : Expression(), m_listener((Expression&) *this) {}

  Calculable::Calculable(const PlexilExprId& expr, const NodeConnectorId& node)
    : Expression(), m_listener((Expression&) *this)
  {
    // confirm that we have an array element
    checkError(Id<PlexilOp>::convertable(expr),
               "Expected a PlexilOp.");
    PlexilOp* op = (PlexilOp*) expr;
    const std::vector<PlexilExprId>& subExprs = op->subExprs();
    for (std::vector<PlexilExprId>::const_iterator it = subExprs.begin(); 
         it != subExprs.end();
         ++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      addSubexpression(subExpr, garbage);
    }
  }

  Calculable::~Calculable() {
    for(ExpressionVectorIter it = m_subexpressions.begin();
        it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->removeListener(m_listener.getId());
    }
    cleanup(m_garbage);
  }

  bool Calculable::containsSubexpression(const ExpressionId& expr)
  {
    for(ExpressionVectorIter it = m_subexpressions.begin();
        it != m_subexpressions.end(); ++it)
      {
        if (expr.equals(*it))
          return true;
      }
    return false;
  }
  void Calculable::addSubexpression(const ExpressionId& expr,
                                    const bool garbage)
  {
    expr->addListener(m_listener.getId());
    m_subexpressions.push_back(expr);
    if(garbage)
      m_garbage.insert(expr);
  }

  void Calculable::removeSubexpression(const ExpressionId& expr) {
    // this is necessary because std::vector doesn't have a find() method!
    for (ExpressionVectorIter it = m_subexpressions.begin();
         it != m_subexpressions.end();
         ++it) {
      if (*it == expr) {
        m_subexpressions.erase(it);
        break;
      }
    }
    m_garbage.erase(expr);
    expr->removeListener(m_listener.getId());
  }

  ExpressionId Calculable::getSubexpression(const PlexilExprId& expr,
                                            const NodeConnectorId& node,
                                            bool& del)
  {
    return ExpressionFactory::createInstance(LabelStr(expr->name()), 
                                             expr, 
                                             node, 
                                             del);
  }

  void Calculable::setValue(const Value& /* value */) {
    checkError(ALWAYS_FAIL, "Shouldn't set the value of a calculable expression.");
  }

  void Calculable::handleChange(const ExpressionId& exp) {
    internalSetValue(recalculate());
  }

  //this could be optimized slightly more to check for dirtiness on subexpressions
  //but that would require setting dirtiness when deactivated, not just when locked
  void Calculable::handleActivate(const bool changed) {
    if(!changed)
      return;
    m_listener.activate();
    for(ExpressionVectorIter it = m_subexpressions.begin();
        it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    internalSetValue(recalculate());
  }

  void Calculable::handleDeactivate(const bool changed) {
    if(!changed)
      return;
    m_listener.deactivate();
    for(ExpressionVectorIter it = m_subexpressions.begin();
        it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
  }

  void Calculable::printAsFnCall(std::ostream& s) const
  {
    Expression::print(s);
    s << operatorString() << '(';
    ExpressionVector::const_iterator it = m_subexpressions.begin();
    while (it != m_subexpressions.end()) {
      (*it)->print(s);
      ++it;
      if (it != m_subexpressions.end())
        s << ", ";
    }
    s << "))";
  }

  void Calculable::printAsInfix(std::ostream& s) const
  {
    Expression::print(s);
    s << '(';
    ExpressionVector::const_iterator it = m_subexpressions.begin();
    while (it != m_subexpressions.end()) {
      (*it)->print(s);
      ++it;
      if (it != m_subexpressions.end())
        s << ' ' << operatorString() << ' ';
      else
        s << "))";
    }
  }

  //
  // Unary expressions
  //

  UnaryExpression::UnaryExpression(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable(expr, node) {
    checkError(m_subexpressions.size() == 1,
               "Expected exactly one subexpression in unary " << expr->name() <<
               ", but have " << m_subexpressions.size());
    m_e = m_subexpressions.front();
  }

  UnaryExpression::UnaryExpression(const ExpressionId& e)
    : Calculable(),
      m_e(e)
  {
    addSubexpression(e, false);
  }

  //
  // Binary expressions
  //

  BinaryExpression::BinaryExpression(const PlexilExprId& expr, const NodeConnectorId& node)
    : Calculable(expr, node) {
    checkError(m_subexpressions.size() == 2,
               "Expected exactly two subexpressions in binary " << expr->name() <<
               ", but have " << m_subexpressions.size());
    m_a = m_subexpressions.front();
    m_b = m_subexpressions.back();
  }

  BinaryExpression::BinaryExpression(const ExpressionId& a, const ExpressionId& b)
    : Calculable(), m_a(a), m_b(b) {
    addSubexpression(a, false);
    addSubexpression(b, false);
  }

  BinaryExpression::BinaryExpression(const ExpressionId& a, bool aGarbage,
                                     const ExpressionId& b, bool bGarbage)
    : Calculable(), m_a(a), m_b(b) {
    addSubexpression(a, aGarbage);
    addSubexpression(b, bGarbage);
  }

  void BinaryExpression::print(std::ostream& s) const
  {
    printAsInfix(s);
  }

  //
  // N-Ary expressions
  //

  NaryExpression::NaryExpression(const PlexilExprId& expr, 
                                 const NodeConnectorId& node)
    : Calculable(expr, node) 
  {
  }

  NaryExpression::NaryExpression(const ExpressionId& a, const ExpressionId& b)
    : Calculable()
  {
    addSubexpression(a, false);
    addSubexpression(b, false);
  }
   
  NaryExpression::NaryExpression(const ExpressionId& a, bool aGarbage,
                                 const ExpressionId& b, bool bGarbage)
    : Calculable()
  {
    addSubexpression(a, aGarbage);
    addSubexpression(b, bGarbage);
  }

  void NaryExpression::print(std::ostream& s) const
  {
    printAsInfix(s);
  }

}
