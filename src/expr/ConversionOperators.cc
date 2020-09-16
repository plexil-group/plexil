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

#include "ConversionOperators.hh"

#include "Function.hh"
#include "PlanError.hh"

#if defined(HAVE_CMATH)
#include <cmath>
#elif defined(HAVE_MATH_H)
#include <math.h>
#endif

#include <limits>

namespace PLEXIL
{

  //
  // Helper function for Real -> int conversions
  // Returns true if conversion successful,
  // false if x is out of range or not an integer.
  //
  static bool RealToInt(Real reel, Integer &result)
  {
    if (reel < std::numeric_limits<Integer>::min()
        || reel > std::numeric_limits<Integer>::max())
      return false; // out of range
    Integer tempResult = (Integer) reel;
    Real fraction = (Real) (reel - (Real) tempResult);
    // TODO: allow fraction to be +/- epsilon
    if (fraction != 0)
      return false; // not an integer
    result = tempResult;
    return true;
  }

  template <typename NUM>
  ConversionOperator<NUM>::ConversionOperator(std::string const &name)
    : OperatorImpl<NUM>(name)
  {
  }

  template <typename NUM>
  ConversionOperator<NUM>::~ConversionOperator()
  {
  }

  template <typename NUM>
  bool ConversionOperator<NUM>::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  template <typename NUM>
  bool ConversionOperator<NUM>::checkArgTypes(Function const *ev) const
  {
    ValueType typ = (*ev)[0]->valueType();
    return isNumericType(typ) || typ == UNKNOWN_TYPE;
  }

  template <typename NUM>
  bool ConversionOperator<NUM>::operator()(NUM & /* result */,
                                           Expression const */* arg0 */,
                                           Expression const */* arg1 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  template <typename NUM>
  bool ConversionOperator<NUM>::operator()(NUM & /* result */,
                                           Function const & /* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  template <>
  bool ConversionOperator<Integer>::calc(Integer &result, Expression const *arg) const
  {
    if (!arg->isKnown())
      return false;

    // Handle possibility of result larger than Integer
    Real realResult;
    if (this->calcInternal(realResult, arg))
      return RealToInt(realResult, result);
    return false;
  }

  template <>
  bool ConversionOperator<Real>::calc(Real &result, Expression const *arg) const
  {
    if (!arg->isKnown())
      return false;
    return this->calcInternal(result, arg);
  }

  //
  // Ceiling, Floor, Round, Truncate
  //

  template <typename NUM>
  Ceiling<NUM>::Ceiling()
    : ConversionOperator<NUM>("CEIL")
  {
  }

  template <typename NUM>
  Ceiling<NUM>::~Ceiling()
  {
  }

  template <typename NUM>
  bool Ceiling<NUM>::calcInternal(Real &result,
                                  Expression const *arg) const
  {
#ifdef HAVE_CEIL
    Real temp;
    arg->getValue(temp); // for effect; see ConversionOperator<NUM>::calc()
    result = ceil(temp);
    return true;
#else
#warning "ceil() is not implemented on this platform. Plans using it will fail."
    return false;
#endif
  }

  template <typename NUM>
  Floor<NUM>::Floor()
    : ConversionOperator<NUM>("FLOOR")
  {
  }

  template <typename NUM>
  Floor<NUM>::~Floor()
  {
  }

  template <typename NUM>
  bool Floor<NUM>::calcInternal(Real &result,
                                Expression const *arg) const
  {
#ifdef HAVE_FLOOR
    Real temp;
    arg->getValue(temp);
    result = floor(temp);
    return true;
#else
#warning "floor() is not implemented on this platform. Plans using it will fail."
    return false;
#endif
  }

  template <typename NUM>
  Round<NUM>::Round()
    : ConversionOperator<NUM>("ROUND")
  {
  }

  template <typename NUM>
  Round<NUM>::~Round()
  {
  }

  template <typename NUM>
  bool Round<NUM>::calcInternal(Real &result,
                                Expression const *arg) const
  {
#ifdef HAVE_ROUND
    Real temp;
    arg->getValue(temp);
    result = round(temp);
    return true;
#else
#warning "round() is not implemented on this platform. Plans using it will fail."
    return false;
#endif
  }

  template <typename NUM>
  Truncate<NUM>::Truncate()
    : ConversionOperator<NUM>("TRUNC")
  {
  }

  template <typename NUM>
  Truncate<NUM>::~Truncate()
  {
  }

  template <typename NUM>
  bool Truncate<NUM>::calcInternal(Real &result,
                                   Expression const *arg) const
  {
#ifdef HAVE_TRUNC
    Real temp;
    arg->getValue(temp);
    result = trunc(temp);
    return true;
#else
#warning "trunc() is not implemented on this platform. Plans using it will fail."
    return false;
#endif
  }

  //
  // RealToInteger
  //

  RealToInteger::RealToInteger()
    : OperatorImpl<Integer>("REAL_TO_INT")
  {
  }

  RealToInteger::~RealToInteger()
  {
  }

  bool RealToInteger::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool RealToInteger::checkArgTypes(Function const *func) const
  {
    ValueType typ = (*func)[0]->valueType();
    return isNumericType(typ) || typ == UNKNOWN_TYPE;
  }

  bool RealToInteger::calc(Integer & result,
                           Expression const *arg) const
  {
    Real temp;
    if (!arg->getValue(temp))
      return false; // unknown/invalid
    return RealToInt(temp, result);
  }

  //
  // Explicit instantiations
  //

  template class Ceiling<Real>;
  template class Ceiling<Integer>;
  template class Floor<Real>;
  template class Floor<Integer>;
  template class Round<Real>;
  template class Round<Integer>;
  template class Truncate<Real>;
  template class Truncate<Integer>;

}
