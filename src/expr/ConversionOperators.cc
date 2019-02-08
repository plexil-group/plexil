/* Copyright (c) 2006-2019, Universities Space Research Association (USRA).
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
// #include "PlanError.hh" // included by OperatorImpl.hh

#include <cmath>
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
    Real tempInt;
    reel = modf(reel, &tempInt);
    // TODO: allow fraction to be +/- epsilon
    if (reel != 0)
      return false; // not an integer
    if (tempInt < std::numeric_limits<Integer>::min()
        || tempInt > std::numeric_limits<Integer>::max())
      return false; // out of range
    result = (Integer) tempInt;
    return true;
  }

  ConversionOperator::ConversionOperator(std::string const &name)
    : Operator(name)
  {
  }

  ConversionOperator::~ConversionOperator()
  {
  }

  void *ConversionOperator::allocateCache() const
  {
    return NULL;
  }

  void ConversionOperator::deleteCache(void * /* ptr */) const
  {
  }

  bool ConversionOperator::checkArgCount(size_t count) const
  {
    return count == 1;
  }

  bool ConversionOperator::checkArgTypes(Function const *ev) const
  {
    ValueType typ = (*ev)[0]->valueType();
    return isNumericType(typ) || typ == UNKNOWN_TYPE;
  }

  ValueType ConversionOperator::valueType() const
  {
    return INTEGER_TYPE;
  }

  bool ConversionOperator::isKnown(Function const &exprs) const
  {
    return exprs[0]->isKnown();
  }

  void ConversionOperator::printValue(std::ostream &s, Function const &exprs) const
  {
    Real temp;
    if (exprs.getValue(temp))
      PLEXIL::printValue(temp, s);
    else
      s << "UNKNOWN";
  }

  Value ConversionOperator::toValue(Function const &exprs) const
  {
    Real temp;
    if (exprs.getValue(temp)) {
      // Return Integer value if in range, Real otherwise
      Integer tempInt;
      if (RealToInt(temp, tempInt))
        return Value(tempInt);
      return Value(temp);
    }
    return Value(0, INTEGER_TYPE);
  }

  bool ConversionOperator::operator()(Integer &result, Expression const *arg) const
  {
    // Handle possibility of result larger than Integer
    Real realResult;
    if (this->calc(realResult, arg))
      return RealToInt(realResult, result);
    return false;
  }

  bool ConversionOperator::operator()(Real &result, Expression const *arg) const
  {
    return this->calc(result, arg);
  }

  bool ConversionOperator::operator()(Integer & /* result */,
                                      Expression const */* arg0 */,
                                      Expression const */* arg1 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  bool ConversionOperator::operator()(Real & /* result */,
                                      Expression const * /* arg0 */,
                                      Expression const * /* arg1 */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  bool ConversionOperator::operator()(Integer & /* result */,
                                      Function const & /* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  bool ConversionOperator::operator()(Real & /* result */,
                                      Function const & /* args */) const
  {
    reportPlanError("Operator " << this->getName()
                    << " only implemented for one-argument case");
    return false;
  }

  //
  // Ceiling, Floor, Round, Truncate
  //

  Ceiling::Ceiling()
    : ConversionOperator("CEIL")
  {
  }

  Ceiling::~Ceiling()
  {
  }

  bool Ceiling::calc(Real &result,
                     Expression const *arg) const
  {
    Real temp;
    if (!arg->getValue(temp))
      return false;
    result = ceil(temp);
    return true;
  }

  Floor::Floor()
    : ConversionOperator("FLOOR")
  {
  }

  Floor::~Floor()
  {
  }

  bool Floor::calc(Real &result,
                   Expression const *arg) const
  {
    Real temp;
    if (!arg->getValue(temp))
      return false;
    result = floor(temp);
    return true;
  }

  // Believe it or not, VxWorks 6.8 for PowerPC doesn't have round() or trunc()
#if !defined(__VXWORKS__)

  Round::Round()
    : ConversionOperator("ROUND")
  {
  }

  Round::~Round()
  {
  }

  bool Round::calc(Real &result,
                   Expression const *arg) const
  {
    Real temp;
    if (!arg->getValue(temp))
      return false;
    result = round(temp);
    return true;
  }

  Truncate::Truncate()
    : ConversionOperator("TRUNC")
  {
  }

  Truncate::~Truncate()
  {
  }

  bool Truncate::calc(Real &result,
                      Expression const *arg) const
  {
    Real temp;
    if (!arg->getValue(temp))
      return false;
    result = trunc(temp);
    return true;
  }
#endif // !defined(__VXWORKS__)

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

}
