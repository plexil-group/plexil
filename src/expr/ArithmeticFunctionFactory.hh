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

#ifndef PLEXIL_ARITHMETIC_FUNCTION_FACTORY_HH
#define PLEXIL_ARITHMETIC_FUNCTION_FACTORY_HH

#include "ExpressionFactory.hh"
#include "Function.hh"
#include "ParserException.hh"
#include "PlexilExpr.hh"

namespace PLEXIL
{
  template <typename RESULT, template <typename OPTYPE> class OP>
  static ExpressionId constructInstance(std::vector<ExpressionId> const &exprs,
                                        std::vector<bool> const &garbage)
  {
    Operator<RESULT> const * oper = OP<RESULT>::instance();
    size_t nargs = exprs.size();
    checkParserException(oper->checkArgCount(nargs),
                         "createExpression: Wrong number of operands for operator "
                         << oper->getName());

    switch (exprs.size()) {
    case 1:
      return (new UnaryFunction<RESULT>(oper, exprs[0], garbage[0]))->getId();

    case 2:
      return (new BinaryFunction<RESULT>(oper,
                                         exprs[0], exprs[1],
                                         garbage[0], garbage[1]))->getId();

    default: // 0, 3 or more
      return (new NaryFunction<RESULT>(oper, exprs, garbage))->getId();
    }
  }

  /**
   * @class ArithmeticFunctionFactory
   * @brief A specialization of ExpressionFactory which selects the appropriate
   * Function and Operator templates, based on the parameter type(s).
   */
  template <template <typename NUM> class OP>
  class ArithmeticFunctionFactory : public ExpressionFactory
  {
  public:
    ArithmeticFunctionFactory(std::string const &name)
      : ExpressionFactory(name)
    {
    }

    ~ArithmeticFunctionFactory()
    {
    }

    ExpressionId allocate(const PlexilExprId& expr,
                          const NodeConnectorId& node,
                          bool & wasCreated) const
    {
      PlexilOp const *op = dynamic_cast<PlexilOp const *>((PlexilExpr const *) expr);
      checkParserException(op, "createExpression: Not a PlexilOp");

      // Get the argument expressions
      std::vector<PlexilExprId> const &args = op->subExprs();
      size_t nargs = args.size();
      checkParserException(nargs, "createExpression: Can't construct arithmetic expression of no operands");
      std::vector<bool> garbage(nargs, false);
      std::vector<ExpressionId> exprs(nargs);
      for (size_t i = 0; i < nargs; ++i) {
        bool isGarbage;
        exprs[i] = createExpression(args[i], node, isGarbage);
        garbage[i] = isGarbage;
      }

      wasCreated = true;
      ValueType resultType = arithmeticResultType(exprs);
      checkParserException(resultType != UNKNOWN_TYPE,
                           "createExpression: type inconsistency in arithmetic expression");
      return create(resultType, exprs, garbage);
    }


  private:
    ExpressionId create(ValueType resultType,
                        std::vector<ExpressionId> const &exprs,
                        std::vector<bool> const &garbage) const
    {
      switch (resultType) {
      case INTEGER_TYPE:
        return constructInstance<int32_t, OP>(exprs, garbage);

      case REAL_TYPE:
        return constructInstance<double, OP>(exprs, garbage);
      
      default:
        checkParserException(false,
                             "createExpression: invalid or unimplemented type "
                             << valueTypeName(resultType));
        return ExpressionId::noId();
      }
    }

    // This allows specialization on per-op basis
    ValueType arithmeticResultType(std::vector<ExpressionId> const &exprs) const
    {
      size_t len = exprs.size();
      assertTrue_1(len > 0);
      ValueType result = UNKNOWN_TYPE;
      switch (exprs[0]->valueType()) {
      case REAL_TYPE:
      case DATE_TYPE:
      case DURATION_TYPE:
        result = REAL_TYPE;
        break;

      case INTEGER_TYPE:
        result = INTEGER_TYPE;
        break;

      default: // not a valid type in an arithmetic expression
        return UNKNOWN_TYPE;
      }

      for (size_t i = 1; i < len; ++i) {
        switch (exprs[i]->valueType()) {
        case REAL_TYPE:
        case DATE_TYPE:
        case DURATION_TYPE:
          result = REAL_TYPE;
          break;

        case INTEGER_TYPE:
          break;

        default:
          return UNKNOWN_TYPE; // bail out early
        }
      }
      return result;
    }

    // Not implemented
    ArithmeticFunctionFactory();
    ArithmeticFunctionFactory(ArithmeticFunctionFactory const &);
    ArithmeticFunctionFactory &operator=(ArithmeticFunctionFactory const &);
  };

} // namespace PLEXIL

// Convenience macros
#define ENSURE_ARITHMETIC_FUNCTION_FACTORY(CLASS) template class PLEXIL::ArithmeticFunctionFactory<CLASS>;
#define REGISTER_ARITHMETIC_FUNCTION(CLASS,NAME) {new PLEXIL::ArithmeticFunctionFactory<CLASS>(#NAME);}

#endif // PLEXIL_ARITHMETIC_FUNCTION_FACTORY_HH
