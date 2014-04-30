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

#include "Function.hh"
#include "Operator.hh"

namespace PLEXIL
{
  template <typename R>
  Function<R>::Function(const Operator<R> *op)
    : Mutable(),
      m_op(op)
  {
  }

  template <typename R>
  Function<R>::~Function()
  {
  }

  template <typename R>
  const char *Function<R>::exprName() const
  {
    return m_op->getName().c_str();
  }

  // Default method, can be overridden
  template <typename R>
  const ValueType Function<R>::valueType() const
  {
    return UNKNOWN_TYPE;
  }

  template <>
  const ValueType Function<bool>::valueType() const
  {
    return BOOLEAN_TYPE;
  }

  template <>
  const ValueType Function<int32_t>::valueType() const
  {
    return INTEGER_TYPE;
  }

  template <>
  const ValueType Function<double>::valueType() const
  {
    return REAL_TYPE;
  }

  template <>
  const ValueType Function<std::string>::valueType() const
  {
    return STRING_TYPE;
  }

  template <typename R>
  bool Function<R>::getValue(R &result) const
  {
    if (!isActive())
      return false;
    return this->calculate(result);
  }

  template <typename R>
  bool Function<R>::isKnown() const
  {
    if (!isActive())
      return false;
    R dummy;
    return this->getValue(dummy);
  }

  template <typename R>
  void Function<R>::printValue(std::ostream &s) const
  {
    R temp;
    if (!this->getValue(temp))
      s << "UNKNOWN";
    else
      s << temp;
  }

  //
  // UnaryFunction
  //

  template <typename R>
  UnaryFunction<R>::UnaryFunction(const Operator<R> *op,
                                  const ExpressionId & exp,
                                  bool isGarbage)
    : Function<R>(op),
      m_a(exp),
      m_aGarbage(isGarbage)
  {
  }

  template <typename R>
  UnaryFunction<R>::~UnaryFunction()
  {
    m_a->removeListener(ExpressionListener::getId());
    if (m_aGarbage)
      delete (Expression *) m_a;
  }

  template <typename R>
  void UnaryFunction<R>::handleActivate()
  {
    m_a->activate();
  }

  template <typename R>
  void UnaryFunction<R>::handleDeactivate()
  {
    m_a->deactivate();
  }

  template <typename R>
  bool UnaryFunction<R>::calculate(R &result) const
  {
    return (*Function<R>::m_op)(result, m_a);
  }

  //
  // BinaryFunction
  //

  template <typename R>
  BinaryFunction<R>::BinaryFunction(const Operator<R> *op,
                                    const ExpressionId & expA,
                                    const ExpressionId & expB,
                                    bool isGarbageA,
                                    bool isGarbageB)
    : Function<R>(op),
      m_a(expA),
      m_b(expB),
      m_aGarbage(isGarbageA),
      m_bGarbage(isGarbageB)
  {
  }

  template <typename R>
  BinaryFunction<R>::~BinaryFunction()
  {
    m_a->removeListener(ExpressionListener::getId());
    if (m_aGarbage)
      delete (Expression *) m_a;
    m_b->removeListener(ExpressionListener::getId());
    if (m_bGarbage)
      delete (Expression *) m_b;
  }

  template <typename R>
  void BinaryFunction<R>::handleActivate()
  {
    m_a->activate();
    m_b->activate();
  }

  template <typename R>
  void BinaryFunction<R>::handleDeactivate()
  {
    m_a->deactivate();
    m_b->deactivate();
  }

  template <typename R>
  bool BinaryFunction<R>::calculate(R &result) const
  {
    return (*Function<R>::m_op)(result, m_a, m_b);
  }

  //
  // NaryFunction
  //

  template <typename R>
  NaryFunction<R>::NaryFunction(const Operator<R> *op,
                                const std::vector<ExpressionId> &exps,
                                const std::vector<bool> &garbage)
    : Function<R>(op),
      m_subexpressions(exps),
      m_garbage(garbage)
  {
    assertTrue_2(exps.size() != garbage.size(), "Subexpression size does not match garbage size");
    for (std::vector<ExpressionId>::iterator it = m_subexpressions.begin();
         it != m_subexpressions.end();
         ++it)
      (*it)->addListener(ExpressionListener::getId());
  }

  template <typename R>
  NaryFunction<R>::~NaryFunction()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i) {
      m_subexpressions[i]->removeListener(ExpressionListener::getId());
      if (m_garbage[i])
        delete (Expression *) m_subexpressions[i];
    }
  }

  template <typename R>
  void NaryFunction<R>::handleActivate()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i)
      m_subexpressions[i]->activate();
  }

  template <typename R>
  void NaryFunction<R>::handleDeactivate()
  {
    for (size_t i = 0; i < m_subexpressions.size(); ++i)
      m_subexpressions[i]->deactivate();
  }

  template <typename R>
  bool NaryFunction<R>::calculate(R &result) const
  {
    return (*Function<R>::m_op)(result, m_subexpressions);
  }

  //
  // Explicit instantiations
  //

  template class UnaryFunction<double>;
  template class UnaryFunction<int32_t>;
  // template class UnaryFunction<uint16_t>;
  template class UnaryFunction<bool>;
  template class UnaryFunction<std::string>;

  template class BinaryFunction<double>;
  template class BinaryFunction<int32_t>;
  // template class BinaryFunction<uint16_t>;
  template class BinaryFunction<bool>;
  template class BinaryFunction<std::string>;

  template class NaryFunction<double>;
  template class NaryFunction<int32_t>;
  // template class NaryFunction<uint16_t>;
  template class NaryFunction<bool>;
  template class NaryFunction<std::string>;

} // namespace PLEXIL
