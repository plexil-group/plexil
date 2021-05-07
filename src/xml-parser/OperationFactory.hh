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

#ifndef PLEXIL_OPERATION_FACTORY_HH
#define PLEXIL_OPERATION_FACTORY_HH

#include "Operations.hh"

namespace PLEXIL
{
  // Forward reference
  class ExpressionFactory;

  ExpressionFactory *
  makeOperationFactory(std::string const &name, std::unique_ptr<Operation> &&opn);

} // namespace PLEXIL

//
// Macros for registering Operations
// See Operations.hh for available constructor functions
//

// Base (general case) macro
#define REGISTER_OPERATION(NAME,OPER) PLEXIL::makeOperationFactory(#NAME, (OPER))

//
// Simple operations
//

#define REGISTER_SIMPLE_OPERATION(NAME,ARGTYPE,RETTYPE,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeSimpleOperation(#NAME, CLASS::instance(), ARGTYPE, RETTYPE))

// One-arg simple operations
#define REGISTER_ONE_ARG_OPERATION(NAME,CLASS,ARGTYPE,RETTYPE) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeSimpleOperation(#NAME, CLASS::instance(), ARGTYPE, RETTYPE, 1, 1))

// Boolean ops
#define REGISTER_BOOLEAN_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeSimpleOperation(#NAME, CLASS::instance(), BOOLEAN_TYPE, BOOLEAN_TYPE))

// Cached simple operations (mostly for string concatenation)
#define REGISTER_CACHED_SIMPLE_OPERATION(NAME,CLASS,ARGTYPE,RETTYPE) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeCachedSimpleOperation(#NAME, CLASS::instance(), ARGTYPE, RETTYPE))


//
// Arithmetic ops
//

#define REGISTER_ARITHMETIC_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeArithmeticOperation(#NAME, CLASS<Integer>::instance(), CLASS<Real>::instance()))
  
#define REGISTER_ONE_ARG_ARITHMETIC_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeArithmeticOperation(#NAME, CLASS<Integer>::instance(), CLASS<Real>::instance(), 1, 1))
  
#define REGISTER_TWO_ARG_ARITHMETIC_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeArithmeticOperation(#NAME, CLASS<Integer>::instance(), CLASS<Real>::instance(), 2, 2))

#define REGISTER_SPECIAL_ARITHMETIC_OPERATION(NAME,CLASS,RETTYPE) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeSpecialArithmeticOperation(#NAME, CLASS::instance(), RETTYPE))
  
#define REGISTER_ARITHMETIC_CONVERSION_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeArithmeticConversionOperation(#NAME, CLASS<Integer>::instance(), CLASS<Real>::instance()))

// Equality
#define REGISTER_EQUALITY_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeEqualityOperation(#NAME, CLASS::instance()))

// Comparisons
#define REGISTER_COMPARISON_OPERATION(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeComparisonOperation(#NAME, CLASS<Integer>::instance(), CLASS<Real>::instance(), CLASS<String>::instance()))

// Predicates of any argument (e.g. IsKnown)
#define REGISTER_ANY_ARG_PREDICATE(NAME,CLASS) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeAnyArgOperation(#NAME, CLASS::instance(), BOOLEAN_TYPE, 1, 1))

// Array queries
#define REGISTER_ARRAY_QUERY_OPERATION(NAME,CLASS,RETTYPE) \
  PLEXIL::makeOperationFactory(#NAME, \
                               PLEXIL::makeArrayOperation(#NAME, CLASS::instance(), RETTYPE))

#endif // PLEXIL_OPERATION_FACTORY_HH
