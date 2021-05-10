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

#ifndef PLEXIL_OPERATIONS_HH
#define PLEXIL_OPERATIONS_HH

#include "Operation.hh"

#include <limits>
#include <memory> // std::unique_ptr

namespace PLEXIL
{

  //
  // Functions to create Operation instances
  //

  // A "simple" operation accepts parameters of one specific type,
  // returns a fixed (potentially different) return type, and has one operator.
  // E.g. Boolean OR

  std::unique_ptr<Operation>
  makeSimpleOperation(std::string const &name,
                      Operator const *oper,
                      ValueType argType,
                      ValueType returnType,
                      size_t minArgs = 0,
                      size_t maxArgs = std::numeric_limits<size_t>::max());

  // A cached simple operation accepts parameters of one specific type,
  // returns a fixed (potentially different) return type, ahas one operator,
  // and caches its result.
  // E.g. string concatenation

  std::unique_ptr<Operation>
  makeCachedSimpleOperation(std::string const &name,
                            Operator const *oper,
                            ValueType argType,
                            ValueType returnType,
                            size_t minArgs = 0,
                            size_t maxArgs = std::numeric_limits<size_t>::max());

  // An "any-arg" operation accepts arguments of any type,
  // and returns one specific type.
  // E.g. isKnown(), a print-to-string function.
  
  std::unique_ptr<Operation>
  makeAnyArgOperation(std::string const &name,
                      Operator const *oper,
                      ValueType returnType,
                      size_t minArgs = 0,
                      size_t maxArgs = std::numeric_limits<size_t>::max());

  // Your typical arithmetic operations; e.g. add, subtract, multiply.
  // These take an arbitrary number of arguments of numeric types,
  // and return a numeric value of either Integer or Real type
  // depending upon the argument types.

  std::unique_ptr<Operation>
  makeArithmeticOperation(std::string const &name,
                          Operator const *integerOper,
                          Operator const *realOper,
                          size_t minArgs = 1,
                          size_t maxArgs = std::numeric_limits<size_t>::max());

  // Special case of arithmetic operation -
  // takes exactly one arg of a numeric type, and
  // returns one specific type.
  // E.g. square root (Real), RealToInteger (Integer)

  std::unique_ptr<Operation>
  makeSpecialArithmeticOperation(std::string const &name,
                                 Operator const *oper,
                                 ValueType returnType);

  // Special behavior for conversion operators
  // These are operators which can return either an Integer or a Real,
  // and can switch result type if the parent expression requests it.
  // E.g. round(), ceil().

  std::unique_ptr<Operation>
  makeArithmeticConversionOperation(std::string const &name,
                                    Operator const *integerOper,
                                    Operator const *realOper);

  // Equality comparisons take exactly two arguments
  // of compatible types, and return a Boolean.

  std::unique_ptr<Operation>
  makeEqualityOperation(std::string const &name,
                        Operator const *oper);

  // Other comparisons take exactly two arguments
  // of comparable types, and return a Boolean.

  std::unique_ptr<Operation>
  makeComparisonOperation(std::string const &name,
                          Operator const *integerOper,
                          Operator const *realOper,
                          Operator const *stringOper);

  //! Operations which take one array parameter and return a value of a particular type.
  //! E.g. arraySize, allElementsKnown

  std::unique_ptr<Operation>
  makeArrayOperation(std::string const &name,
                     Operator const *oper,
                     ValueType returnType);

} // namespace PLEXIL

#endif // PLEXIL_OPERATIONS_HH
