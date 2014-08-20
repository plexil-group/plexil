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
// #include "Error.hh" // included by OperatorImpl.hh
#include "Expression.hh"
#include "ExprVec.hh"

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
    : OperatorImpl<NUM>("ADD")
  {
  }

  template <typename NUM>
  Addition<NUM>::~Addition()
  {
  }

  template <typename NUM>
  bool Addition<NUM>::checkArgCount(size_t count) const
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
                           ExprVec const &args) const
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
                              ExprVec const &args) const
  {
    assertTrue_1(args.size() > 0); // must be at least one
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
                                 ExprVec const &args) const
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
  bool Modulo<int32_t>::calc(int32_t &result, Expression const *arg0, Expression const *arg1) const
  {
    int32_t temp0, temp1;
    if (!(arg0->getValue(temp0) && arg1->getValue(temp1))
        || temp1 == 0)
      return false;
    result = temp0 % temp1;
    return true;
  }

  // Real implementation
  template <>
  bool Modulo<double>::calc(double &result, Expression const *arg0, Expression const *arg1) const
  {
    double temp0, temp1;
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
                          ExprVec const &args) const
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
                          ExprVec const &args) const
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

  template <>
  bool SquareRoot<double>::calc(double &result,
                                Expression const *arg) const
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
    : OperatorImpl<NUM>("CEIL")
  {
  }

  template <typename NUM>
  Ceiling<NUM>::~Ceiling()
  {
  }

  template <typename NUM>
  bool Ceiling<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <>
  bool Ceiling<double>::calc(double &result,
                             Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = ceil(temp);
    return true;
  }

  template <>
  bool Ceiling<int32_t>::calc(int32_t &result,
                              Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(ceil(temp), result);
  }

  template <typename NUM>
  Floor<NUM>::Floor()
    : OperatorImpl<NUM>("FLOOR")
  {
  }

  template <typename NUM>
  Floor<NUM>::~Floor()
  {
  }

  template <typename NUM>
  bool Floor<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <>
  bool Floor<double>::calc(double &result,
                           Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = floor(temp);
    return true;
  }

  template <>
  bool Floor<int32_t>::calc(int32_t &result,
                            Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(floor(temp), result);
  }

  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)

  template <typename NUM>
  Round<NUM>::Round()
    : OperatorImpl<NUM>("ROUND")
  {
  }

  template <typename NUM>
  Round<NUM>::~Round()
  {
  }

  template <typename NUM>
  bool Round<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <>
  bool Round<double>::calc(double &result,
                           Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = round(temp);
    return true;
  }

  template <>
  bool Round<int32_t>::calc(int32_t &result,
                            Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(round(temp), result);
  }

  template <typename NUM>
  Truncate<NUM>::Truncate()
    : OperatorImpl<NUM>("TRUNC")
  {
  }

  template <typename NUM>
  Truncate<NUM>::~Truncate()
  {
  }

  template <typename NUM>
  bool Truncate<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <>
  bool Truncate<double>::calc(double &result,
                              Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    result = trunc(temp);
    return true;
  }

  template <>
  bool Truncate<int32_t>::calc(int32_t &result,
                               Expression const *arg) const
  {
    double temp;
    if (!arg->getValue(temp))
      return false;
    return doubleToInt(trunc(temp), result);
  }
#endif // !defined(__VXWORKS__)

  //
  // RealToInteger
  //

  RealToInteger::RealToInteger()
    : OperatorImpl<int32_t>("REAL_TO_INT")
  {
  }

  RealToInteger::~RealToInteger()
  {
  }

  bool RealToInteger::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool RealToInteger::calc(int32_t & result,
                           Expression const *arg) const
  {
    double temp;
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
  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)
  template class Round<double>;
  template class Round<int32_t>;
  template class Truncate<double>;
  template class Truncate<int32_t>;
#endif // !defined(__VXWORKS__)

} // namespace PLEXIL
