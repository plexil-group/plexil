/* Copyright (c) 2006-2020, Universities Space Research Association (USRA).
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
#include "Function.hh"
#include "PlanError.hh"

#if defined(HAVE_CMATH)
#include <cmath>
#elif defined(HAVE_MATH_H)
#include <math.h>
#endif

namespace PLEXIL
{

  // TODO:
  // - Overflow/underflow checks

  //
  // Addition
  //

  template <typename NUM>
  Addition<NUM>::Addition()
    : OperatorImpl<NUM>("ADD")
  {
  }

  template <typename NUM>
  Addition<NUM>::~Addition()
  {
  }

  template <typename NUM>
  bool Addition<NUM>::checkArgCount(size_t /* count */) const
  {
    return true;
  }

  template <typename NUM>
  bool Addition<NUM>::calc(NUM &result, Expression const *arg) const
  {
    return arg->getValue(result);
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Addition<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1)))
      return false;
    result = temp0 + temp1;
    return true;
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Addition<NUM>::calc(NUM &result,
                           Function const &args) const
  {
    NUM workingResult = 0;
    for (size_t i = 0; i < args.size(); ++i) {
      NUM temp;
      if (!args[i]->getValue(temp))
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
    : OperatorImpl<NUM>("SUB")
  {
  }

  template <typename NUM>
  Subtraction<NUM>::~Subtraction()
  {
  }

  template <typename NUM>
  bool Subtraction<NUM>::checkArgCount(size_t count) const
  {
    return count >= 1;
  }

  // TODO:
  // - Overflow checks
  // - If we extend to unsigned numeric types, add an error message for these methods 

  template <typename NUM>
  bool Subtraction<NUM>::calc(NUM &result, Expression const *arg) const
  {
    NUM temp;
    if (!arg->getValue(temp))
      return false;
    result = -temp;
    return true;
  }

  template <typename NUM>
  bool Subtraction<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1)))
      return false;
    result = temp0 - temp1;
    return true;
  }

  template <typename NUM>
  bool Subtraction<NUM>::calc(NUM &result,
                              Function const &args) const
  {
    checkPlanError(args.size() > 0,
                   this->getName() << " requires at least one operand");
    NUM temp;
    if (!args[0]->getValue(temp))
      return false;
    if (args.size() == 1) {
      // Unary
      result = -temp;
      return true;
    }
    // 2 or more args
    NUM workingResult = temp;
    for (size_t i = 1; i < args.size(); ++i) {
      if (!args[i]->getValue(temp))
        return false;
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
    : OperatorImpl<NUM>("MUL")
  {
  }

  template <typename NUM>
  Multiplication<NUM>::~Multiplication()
  {
  }

  template <typename NUM>
  bool Multiplication<NUM>::checkArgCount(size_t count) const
  {
    return count > 0;
  }

  template <typename NUM>
  bool Multiplication<NUM>::calc(NUM &result, Expression const *arg) const
  {
    return arg->getValue(result);
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Multiplication<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1)))
      return false;
    result = temp0 * temp1;
    return true;
  }

  // TODO: overflow checks
  template <typename NUM>
  bool Multiplication<NUM>::calc(NUM &result,
                                 Function const &args) const
  {
    NUM workingResult, temp;
    if (!args[0]->getValue(workingResult))
      return false;
    for (size_t i = 1; i < args.size(); ++i) {
      if (!args[i]->getValue(temp))
        return false;
      workingResult *= temp;
    }
    result = workingResult;
    return true;
  }

  //
  // Division
  //

  template <typename NUM>
  Division<NUM>::Division()
    : OperatorImpl<NUM>("DIV")
  {
  }

  template <typename NUM>
  Division<NUM>::~Division()
  {
  }

  template <typename NUM>
  bool Division<NUM>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  // TODO: warn on zero divisor?
  template <typename NUM>
  bool Division<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1))
        || temp1 == 0)
      return false;
    result = temp0 / temp1;
    return true;
  }

  //
  // Modulo
  //

  template <typename NUM>
  Modulo<NUM>::Modulo()
    : OperatorImpl<NUM>("MOD")
  {
  }

  template <typename NUM>
  Modulo<NUM>::~Modulo()
  {
  }

  template <typename NUM>
  bool Modulo<NUM>::checkArgCount(size_t count) const
  {
    return count == 2;
  }

  // Integer implementation
  template <>
  bool Modulo<Integer>::calc(Integer &result, Expression const *arg0, Expression const *arg1) const
  {
    Integer temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1))
        || temp1 == 0)
      return false;
    result = temp0 % temp1;
    return true;
  }

  // Real implementation
  template <>
  bool Modulo<Real>::calc(Real &result, Expression const *arg0, Expression const *arg1) const
  {
    Real temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1))
        || temp1 == 0)
      return false;
    result = fmod(temp0, temp1);
    return true;
  }

  //
  // Minimum
  //

  template <typename NUM>
  Minimum<NUM>::Minimum()
    : OperatorImpl<NUM>("MIN")
  {
  }

  template <typename NUM>
  Minimum<NUM>::~Minimum()
  {
  }

  template <typename NUM>
  bool Minimum<NUM>::checkArgCount(size_t count) const
  {
    return count >= 1;
  }

  template <typename NUM>
  bool Minimum<NUM>::calc(NUM &result, Expression const *arg) const
  {
    return arg->getValue(result);
  }

  template <typename NUM>
  bool Minimum<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1)))
      return false;
    result = (temp0 < temp1 ? temp0 : temp1);
    return true;
  }

  template <typename NUM>
  bool Minimum<NUM>::calc(NUM &result,
                          Function const &args) const
  {
    NUM workingResult;
    if (!args[0]->getValue(workingResult))
      return false;
    for (size_t i = 1; i < args.size(); ++i) {
      NUM temp;
      if (!args[i]->getValue(temp))
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
    : OperatorImpl<NUM>("MAX")
  {
  }

  template <typename NUM>
  Maximum<NUM>::~Maximum()
  {
  }

  template <typename NUM>
  bool Maximum<NUM>::checkArgCount(size_t count) const
  {
    return count >= 1;
  }

  template <typename NUM>
  bool Maximum<NUM>::calc(NUM &result, Expression const *arg) const
  {
    return arg->getValue(result);
  }

  template <typename NUM>
  bool Maximum<NUM>::calc(NUM &result, Expression const *arg0, Expression const *arg1) const
  {
    NUM temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1)))
      return false;
    result = (temp0 > temp1 ? temp0 : temp1);
    return true;
  }

  template <typename NUM>
  bool Maximum<NUM>::calc(NUM &result,
                          Function const &args) const
  {
    NUM workingResult;
    if (!args[0]->getValue(workingResult))
      return false;
    for (size_t i = 1; i < args.size(); ++i) {
      NUM temp;
      if (!args[i]->getValue(temp))
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
    : OperatorImpl<NUM>("ABS")
  {
  }

  template <typename NUM>
  bool AbsoluteValue<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <typename NUM>
  AbsoluteValue<NUM>::~AbsoluteValue()
  {
  }

  // TODO: Unsigned numeric types need a simple passthrough method

  template <typename NUM>
  bool AbsoluteValue<NUM>::calc(NUM &result,
                                Expression const *arg) const
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
    : OperatorImpl<NUM>("SQRT")
  {
  }

  template <typename NUM>
  SquareRoot<NUM>::~SquareRoot()
  {
  }

  template <typename NUM>
  bool SquareRoot<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <typename NUM>
  bool SquareRoot<NUM>::checkArgTypes(Function const *func) const
  {
    ValueType typ = (*func)[0]->valueType();
    return isNumericType(typ) || typ == UNKNOWN_TYPE;
  }

  template <>
  bool SquareRoot<Real>::calc(Real &result,
                                Expression const *arg) const
  {
#ifdef HAVE_SQRT    
    Real temp;
    if (!arg->getValue(temp)
        || temp < 0) // imaginary result
      return false;
    result = sqrt(temp);
    return true;
#else
#warning "sqrt() not available on this platform. Plans using it will fail."
    return false;
#endif
  }

  //
  // Explicit instantiations
  //
  template class Addition<Real>;
  template class Addition<Integer>;
  template class Subtraction<Real>;
  template class Subtraction<Integer>;
  template class Multiplication<Real>;
  template class Multiplication<Integer>;
  template class Division<Real>;
  template class Division<Integer>;
  template class Modulo<Integer>;
  template class Modulo<Real>;
  template class Minimum<Real>;
  template class Minimum<Integer>;
  template class Maximum<Real>;
  template class Maximum<Integer>;
  template class AbsoluteValue<Real>;
  template class AbsoluteValue<Integer>;
  // Only implemented for floating point types
  template class SquareRoot<Real>;

} // namespace PLEXIL
