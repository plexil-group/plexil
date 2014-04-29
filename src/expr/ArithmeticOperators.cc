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

namespace PLEXIL
{

  // TODO:
  // - Type conversions
  // - Overflow/underflow checks

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
    if (!argA->getValue(tempA))
      return false;
    if (!argB->getValue(tempB))
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
    if (!argA->getValue(tempA))
      return false;
    if (!argB->getValue(tempB))
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
  // Explicit instantiations
  //
  template class Addition<double>;
  template class Addition<int32_t>;
  template class Subtraction<double>;
  template class Subtraction<int32_t>;

} // namespace PLEXIL
