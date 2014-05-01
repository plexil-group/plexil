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

#include "ArithmeticOperators.hh"
#include "Expression.hh"
#include <cmath> // for sqrt()

namespace PLEXIL
{

  // TODO:
  // - Type conversions
  // - Overflow/underflow checks

  //
  // Addition
  //

  template <typename NUM>
  Addition<NUM>::Addition()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "+";
  }

  template <typename NUM>
  Addition<NUM>::~Addition()
  {
  }

  template <typename NUM>
  bool Addition<NUM>::operator()(NUM &result,
                                 const ExpressionId &argA,
                                 const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB))
      return false;
    result = tempA + tempB;
    return true;
  }

  template <typename NUM>
  bool Addition<NUM>::operator()(NUM &result,
                                 const std::vector<ExpressionId> &args) const
  {
    NUM workingResult = 0;
    for (std::vector<ExpressionId>::const_iterator it = args.begin();
         it != args.end();
         ++it) {
      NUM temp;
      if (!(*it)->getValue(temp))
        return false; // unknown if any arg unknown
      workingResult += temp;
    }
    result = workingResult;
    return true;
  }

  //
  // Subtraction
  //

  template <typename NUM>
  Subtraction<NUM>::Subtraction()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "-";
  }

  template <typename NUM>
  Subtraction<NUM>::~Subtraction()
  {
  }

  // *** TODO ***
  // If we extend to unsigned numeric types, add an error message for this method

  template <typename NUM>
  bool Subtraction<NUM>::operator()(NUM &result,
                                    const ExpressionId &arg) const
  {
    NUM temp;
    if (!arg->getValue(temp))
      return false;
    result = -temp;
    return true;
  }

  template <typename NUM>
  bool Subtraction<NUM>::operator()(NUM &result,
                                    const ExpressionId &argA,
                                    const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB))
      return false;
    result = tempA - tempB;
    return true;
  }

  template <typename NUM>
  bool Subtraction<NUM>::operator()(NUM &result,
                                    const std::vector<ExpressionId> &args) const
  {
    std::vector<ExpressionId>::const_iterator it = args.begin();
    NUM workingResult;
    if (!(*it++)->getValue(workingResult))
      return false;
    for (; it != args.end(); ++it) {
      NUM temp;
      if (!(*it)->getValue(temp))
        return false; // unknown if any arg unknown
      workingResult -= temp;
    }
    result = workingResult;
    return true;
  }

  //
  // Multiplication
  //

  template <typename NUM>
  Multiplication<NUM>::Multiplication()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "*";
  }

  template <typename NUM>
  Multiplication<NUM>::~Multiplication()
  {
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Multiplication<NUM>::operator()(NUM &result,
                                       const ExpressionId &argA,
                                       const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB))
      return false;
    result = tempA * tempB;
    return true;
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Multiplication<NUM>::operator()(NUM &result,
                                       const std::vector<ExpressionId> &args) const
  {
    std::vector<ExpressionId>::const_iterator it = args.begin();
    NUM workingResult;
    if (!(*it++)->getValue(workingResult))
      return false;
    for (; it != args.end(); ++it) {
      NUM temp;
      if (!(*it)->getValue(temp))
        return false; // unknown if any arg unknown
      workingResult = workingResult * temp;
    }
    result = workingResult;
    return true;
  }

  //
  // Division
  //

  template <typename NUM>
  Division<NUM>::Division()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "/";
  }

  template <typename NUM>
  Division<NUM>::~Division()
  {
  }

  template <typename NUM>
  bool Division<NUM>::operator()(NUM &result,
                                 const ExpressionId &argA,
                                 const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB)
        || tempB == 0)
      return false;
    result = tempA / tempB;
    return true;
  }

  //
  // Modulo
  //

  template <typename NUM>
  Modulo<NUM>::Modulo()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "%";
  }

  template <typename NUM>
  Modulo<NUM>::~Modulo()
  {
  }

  template <typename NUM>
  bool Modulo<NUM>::operator()(NUM &result,
                               const ExpressionId &argA,
                               const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB)
        || tempB == 0)
      return false;
    result = tempA % tempB;
    return true;
  }

  //
  // Minimum
  //

  template <typename NUM>
  Minimum<NUM>::Minimum()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "*";
  }

  template <typename NUM>
  Minimum<NUM>::~Minimum()
  {
  }

  template <typename NUM>
  bool Minimum<NUM>::operator()(NUM &result,
                                const ExpressionId &argA,
                                const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB))
      return false;
    result = (tempA <= tempB) ? tempA : tempB;
    return true;
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Minimum<NUM>::operator()(NUM &result,
                                const std::vector<ExpressionId> &args) const
  {
    std::vector<ExpressionId>::const_iterator it = args.begin();
    NUM workingResult;
    if (!(*it++)->getValue(workingResult))
      return false;
    for (; it != args.end(); ++it) {
      NUM temp;
      if (!(*it)->getValue(temp))
        return false; // unknown if any arg unknown
      if (temp < workingResult)
        workingResult = temp;
    }
    result = workingResult;
    return true;
  }

  //
  // Maximum
  //

  template <typename NUM>
  Maximum<NUM>::Maximum()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "*";
  }

  template <typename NUM>
  Maximum<NUM>::~Maximum()
  {
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Maximum<NUM>::operator()(NUM &result,
                                const ExpressionId &argA,
                                const ExpressionId &argB) const
  {
    NUM tempA, tempB;
    if (!argA->getValue(tempA))
      return false;
    if (!argB->getValue(tempB))
      return false;
    result = (tempA >= tempB) ? tempA : tempB;
    return true;
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Maximum<NUM>::operator()(NUM &result,
                                const std::vector<ExpressionId> &args) const
  {
    std::vector<ExpressionId>::const_iterator it = args.begin();
    NUM workingResult;
    if (!(*it++)->getValue(workingResult))
      return false;
    for (; it != args.end(); ++it) {
      NUM temp;
      if (!(*it)->getValue(temp))
        return false; // unknown if any arg unknown
      if (temp > workingResult)
        workingResult = temp;
    }
    result = workingResult;
    return true;
  }

  //
  // AbsoluteValue
  //

  template <typename NUM>
  AbsoluteValue<NUM>::AbsoluteValue()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "abs";
  }

  template <typename NUM>
  AbsoluteValue<NUM>::~AbsoluteValue()
  {
  }

  // *** TODO ***
  // Unsigned numeric types need a simple passthrough method

  template <typename NUM>
  bool AbsoluteValue<NUM>::operator()(NUM &result,
                                      const ExpressionId &arg) const
  {
    NUM temp;
    if (!arg->getValue(temp))
      return false;
    result = (temp < 0) ? -temp : temp;
    return true;
  }

  //
  // SquareRoot
  //

  template <typename NUM>
  SquareRoot<NUM>::SquareRoot()
    : Operator<NUM>()
  {
    Operator<NUM>::m_name = "sqrt";
  }

  template <typename NUM>
  SquareRoot<NUM>::~SquareRoot()
  {
  }

  template <>
  bool SquareRoot<double>::operator()(double &result,
                                      const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp)
        || temp < 0) // imaginary result
      return false;
    result = sqrt(temp);
    return true;
  }

  // Not currently used
  // template <>
  // bool SquareRoot<float>::operator()(float &result,
  //                                    const ExpressionId &arg) const
  // {
  //   float temp;
  //   if (!arg->getValue(temp))
  //     return false;
  //   if (temp < 0)
  //     return false; // imaginary result
  //   result = sqrtf(temp);
  //   return true;
  // }

  //
  // Explicit instantiations
  //
  template class Addition<double>;
  template class Addition<int32_t>;
  template class Subtraction<double>;
  template class Subtraction<int32_t>;
  template class Multiplication<double>;
  template class Multiplication<int32_t>;
  template class Division<double>;
  template class Division<int32_t>;
  // Only implemented for integers
  template class Modulo<int32_t>;
  template class Minimum<double>;
  template class Minimum<int32_t>;
  template class Maximum<double>;
  template class Maximum<int32_t>;
  template class AbsoluteValue<double>;
  template class AbsoluteValue<int32_t>;
  // Only implemented for floating point types
  template class SquareRoot<double>;

} // namespace PLEXIL
