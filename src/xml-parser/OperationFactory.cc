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

#include "OperationFactory.hh"

#include "createExpression.hh"
#include "ExpressionFactory.hh"
#include "Function.hh"
#include "Operator.hh"
#include "parser-utils.hh"
#include "ParserException.hh"

#include "pugixml.hpp"

namespace PLEXIL
{

  class OperationFactory : public ExpressionFactory
  {
  public:
    OperationFactory(std::string const &name, std::unique_ptr<Operation> &&opn)
      : ExpressionFactory(name),
        m_operation(std::move(opn))
    {
    }

    virtual ~OperationFactory() = default;

    virtual ValueType check(char const *nodeId,
                            pugi::xml_node expr,
                            ValueType desiredType) const
    {
      size_t n = std::distance(expr.begin(), expr.end());
      checkParserExceptionWithLocation(m_operation->checkArgCount(n),
                                       expr,
                                       "Node \"" << nodeId
                                       << "\": Wrong number of operands for operator "
                                       << m_operation->getName());

      // Check arguments
      std::vector<ValueType> argTypes;
      for (pugi::xml_node subexp : expr.children())
        argTypes.push_back(checkExpression(nodeId, subexp));

      // It would be nice to get more detailed info than this. Maybe later,
      checkParserExceptionWithLocation(m_operation->checkArgTypes(argTypes),
                                       expr,
                                       "Node \"" << nodeId
                                       << "\": Some argument to operator "
                                       << m_operation->getName()
                                       << " has an invalid or unimplemented type");

      return m_operation->getValueType(argTypes, desiredType);
    }

    virtual Expression *allocate(pugi::xml_node const expr,
                                 NodeConnector *node,
                                 bool & wasCreated,
                                 ValueType desiredType) const
    {
      // Parse the arguments and get their types
      size_t n = std::distance(expr.begin(), expr.end());
      std::vector<Expression *> args(n, nullptr);
      std::vector<bool> argCreated(n, false);
      std::vector<ValueType> argTypes(n, UNKNOWN_TYPE);
      size_t i = 0;
      try {
        for (pugi::xml_node subexp : expr.children()) {
          bool created;
          Expression *arg = createExpression(subexp, node, created);
          args[i] = arg;
          argCreated[i] = created;
          argTypes[i] = arg->valueType();
          ++i;
        }
      }
      catch (ParserException & /* e */) {
        for (size_t j = 0; j < i; j++)
          if (argCreated[i])
            delete args[i];
        throw;
      }

      Operator const *oper = m_operation->getOperator(argTypes, desiredType);
      assertTrueMsg(oper,
                    "OperationFactory::allocate: no operator found for "
                    << m_operation->getName()
                    << "\n Arg types " << valueTypeName(argTypes[0])
                    << ", " << valueTypeName(argTypes[1]));
      Function *result = m_operation->constructFunction(oper, n);

      for (size_t j = 0 ; j < n; ++j) {
        result->setArgument(j, args[j], argCreated[j]);
      }
      wasCreated = true;
      return result;
    }

  private:
    // Unimplemented
    OperationFactory() = delete;
    OperationFactory(OperationFactory const &) = delete;
    OperationFactory(OperationFactory &&) = delete;
    OperationFactory &operator=(OperationFactory const &) = delete;
    OperationFactory &operator=(OperationFactory &&) = delete;

    std::unique_ptr<Operation> m_operation;
  };

  ExpressionFactory *
  makeOperationFactory(std::string const &name, std::unique_ptr<Operation> &&opn)
  {
    return new OperationFactory(name, std::move(opn));
  }

} // namespace PLEXIL
