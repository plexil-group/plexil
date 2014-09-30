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

#include "FunctionFactory.hh"
#include "pugixml.hpp"

namespace PLEXIL
{
  FunctionFactory::FunctionFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  FunctionFactory::~FunctionFactory()
  {
  }

  Expression *FunctionFactory::allocate(PlexilExpr const *expr,
                                        NodeConnector *node,
                                        bool &wasCreated) const
  {
    PlexilOp const *op = dynamic_cast<PlexilOp const *>(expr);
    checkParserException(op != NULL, "createExpression: Expression is not a PlexilOp");

    std::vector<PlexilExpr *> const &args = op->subExprs();
    ExprVec *exprVec = constructExprVec(args, node);
    Operator const *oper = this->getOperator();
    checkParserException(oper->checkArgCount(args.size()),
                         "createExpression: Wrong number of operands for operator "
                         << oper->getName());

    wasCreated = true;
    return new Function(oper, exprVec);
  }

  Expression *FunctionFactory::allocate(pugi::xml_node const expr,
                                        NodeConnector *node,
                                        bool &wasCreated) const
  {
    ExprVec *exprVec = constructExprVec(expr, node);
    Operator const *oper = this->getOperator();
    checkParserException(oper->checkArgCount(exprVec->size()),
                         "createExpression: Wrong number of operands for operator "
                         << oper->getName());

    wasCreated = true;
    return new Function(oper, exprVec);
  }

  ExprVec *
  FunctionFactory::constructExprVec(std::vector<PlexilExpr *> const &subexprs,
                                    NodeConnector *node) const
  {
    // Get the argument expressions
    size_t nargs = subexprs.size();
    std::vector<bool> garbage(nargs, false);
    std::vector<Expression *> exprs(nargs);
    for (size_t i = 0; i < nargs; ++i) {
      bool isGarbage;
      exprs[i] = createExpression(subexprs[i], node, isGarbage);
      garbage[i] = isGarbage;
    }

    return makeExprVec(exprs, garbage);
  }

  ExprVec *
  FunctionFactory::constructExprVec(pugi::xml_node const expr,
                                    NodeConnector *node) const
  {
    std::vector<Expression *> exprs;
    std::vector<bool> garbage;
    pugi::xml_node subexp = expr.first_child();
    while (subexp) {
      bool created;
      exprs.push_back(createExpression(subexp, node, created));
      garbage.push_back(created);
      subexp = subexp.next_sibling();
    }

    return makeExprVec(exprs, garbage);
  }

} // namespace PLEXIL
