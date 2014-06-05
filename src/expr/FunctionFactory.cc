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

#include "Error.hh"
#include "Function.hh"
// #include "NodeConnector.hh" // ?
#include "PlexilExpr.hh"

namespace PLEXIL
{

  template<class OP, typename RETURNS>
  FunctionFactory<OP, RETURNS>::FunctionFactory(std::string const &name)
    : ExpressionFactory(name)
  {
  }

  template<class OP, typename RETURNS>
  FunctionFactory<OP, RETURNS>::~FunctionFactory()
  {
  }

  template<class OP, typename RETURNS>
  ExpressionId FunctionFactory<OP, RETURNS>::create(PlexilExprId const &expr,
                                                    NodeConnectorId const &node)
  {
    PlexilOp const *op = (PlexilOp const *) expr;
    assertTrue_2(op != NULL, "FunctionFactory::create: Expression is not a PlexilOp");

    std::vector<PlexilExprId> const &args = op->subExprs();
    size_t nargs = args.size();
    OP *oper(OP::instance());
    assertTrue_2(oper->valueType() == op->type(),
                 "FunctionFactory::create: Type mismatch between operator and expression");
    assertTrue_2(oper->checkArgCount(nargs),
                 "FunctionFactory::create: Wrong number of arguments for operator");

    // Get the argument expressions
    std::vector<bool> garbage(nargs, false);
    std::vector<ExpressionId> exprs(nargs);
    for (size_t i = 0; i < nargs; ++i) {
      bool isGarbage = garbage[i];
      ExpressionFactory::createInstance(args[i]->name(), // ???
                                        args[i],
                                        node,
                                        isGarbage);
    }

    switch (args.size()) {
    case 1:
      return (new UnaryFunction<RETURNS>(oper, exprs[0], garbage[0]))->getId();

    case 2:
      return (new BinaryFunction<RETURNS>(oper,
                                          exprs[0], exprs[1],
                                          garbage[0], garbage[1]))->getId();

    default: // 0, 3 or more
      return (new NaryFunction<RETURNS>(oper, exprs, garbage))->getId();
    }
  }

} // namespace PLEXIL
