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
#include <cmath>
#include <limits>

namespace PLEXIL
{

  // TODO:
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

  // TODO: overflow checks
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

  // TODO: overflow checks
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

  // TODO: overflow checks
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

  // TODO: overflow checks
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

  // TODO: warn on zero divisor?
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

  // Integer implementation
  template <>
  bool Modulo<int32_t>::operator()(int32_t &result,
                                   const ExpressionId &argA,
                                   const ExpressionId &argB) const
  {
    int32_t tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB)
        || tempB == 0)
      return false;
    result = tempA % tempB;
    return true;
  }

  // Real implementation
  template <>
  bool Modulo<double>::operator()(double &result,
                                  const ExpressionId &argA,
                                  const ExpressionId &argB) const
  {
    double tempA, tempB;
    if (!argA->getValue(tempA)
        || !argB->getValue(tempB)
        || tempB == 0)
      return false;
    result = fmod(tempA, tempB);
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

  //
  // Helper function for double -> int conversions
  // Returns true if conversion successful,
  // false if x is out of range or not an integer.
  //
  static bool doubleToInt(double x, int32_t &result)
  {
    double tempInt;
    x = modf(x, &tempInt);
    // TODO: allow fraction to be +/- epsilon
    if (x != 0)
      return false; // not an integer
    if (tempInt < std::numeric_limits<int32_t>::min()
        || tempInt > std::numeric_limits<int32_t>::max())
      return false; // out of range
    result = (int32_t) tempInt;
    return true;
  }

  //
  // Ceiling, Floor, Round, Truncate
  //

  template <typename NUM>
  Ceiling<NUM>::Ceiling()
    : Operator<NUM>()
  {
  }

  template <typename NUM>
  Ceiling<NUM>::~Ceiling()
  {
  }

  template <>
  bool Ceiling<double>::operator()(double &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = ceil(temp);
    return true;
  }

  template <>
  bool Ceiling<int32_t>::operator()(int32_t &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(ceil(temp), result);
  }

  template <typename NUM>
  Floor<NUM>::Floor()
    : Operator<NUM>()
  {
  }

  template <typename NUM>
  Floor<NUM>::~Floor()
  {
  }

  template <>
  bool Floor<double>::operator()(double &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = floor(temp);
    return true;
  }

  template <>
  bool Floor<int32_t>::operator()(int32_t &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(floor(temp), result);
  }

  template <typename NUM>
  Round<NUM>::Round()
    : Operator<NUM>()
  {
  }

  template <typename NUM>
  Round<NUM>::~Round()
  {
  }

  template <>
  bool Round<double>::operator()(double &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = round(temp);
    return true;
  }

  template <>
  bool Round<int32_t>::operator()(int32_t &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(round(temp), result);
  }

  template <typename NUM>
  Truncate<NUM>::Truncate()
    : Operator<NUM>()
  {
  }

  template <typename NUM>
  Truncate<NUM>::~Truncate()
  {
  }

  template <>
  bool Truncate<double>::operator()(double &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = trunc(temp);
    return true;
  }

  template <>
  bool Truncate<int32_t>::operator()(int32_t &result, const ExpressionId &arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(trunc(temp), result);
  }

  //
  // RealToInteger
  //

  RealToInteger::RealToInteger()
    : Operator<int32_t>()
  {
  }

  RealToInteger::~RealToInteger()
  {
  }

  bool RealToInteger::operator()(int32_t & result, const ExpressionId &arg) const
  {
    double temp, tempInt;
    if (!arg->getValue(temp))
      return false; // unknown/invalid
    return doubleToInt(temp, result);
  }


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
  template class Modulo<int32_t>;
  template class Modulo<double>;
  template class Minimum<double>;
  template class Minimum<int32_t>;
  template class Maximum<double>;
  template class Maximum<int32_t>;
  template class AbsoluteValue<double>;
  template class AbsoluteValue<int32_t>;
  // Only implemented for floating point types
  template class SquareRoot<double>;
  template class Ceiling<double>;
  template class Ceiling<int32_t>;
  template class Floor<double>;
  template class Floor<int32_t>;
  template class Round<double>;
  template class Round<int32_t>;
  template class Truncate<double>;
  template class Truncate<int32_t>;

} // namespace PLEXIL
