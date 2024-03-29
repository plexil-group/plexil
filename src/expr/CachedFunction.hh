// Copyright (c) 2006-2022, Universities Space Research Association (USRA).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Universities Space Research Association nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef PLEXIL_CACHED_FUNCTION_HH
#define PLEXIL_CACHED_FUNCTION_HH

#include <cstddef> // size_t

namespace PLEXIL
{

  // Forward references
  class Expression;
  class Function;
  class Operator;

  //
  // Factory functions
  //
  
  //! \brief Construct a Function with result cache.
  //! \param op Pointer to the Operator for this function.
  //! \param nargs The number of arguments to the function.
  //! \return Pointer to the newly allocated Function.
  //! \ingroup Expressions
  extern Function *makeCachedFunction(Operator const *op,
                                      size_t nargs);

  //! \brief Construct a Function with result cache.
  //! \param op Pointer to the Operator for this function.
  //! \param expr Pointer to the function argument.
  //! \param garbage true if the argument Expression should be deleted with the function, false otherwise.
  //! \return Pointer to the newly allocated Function.
  //! \note Convenience wrapper for Node classes and unit test.
  //! \ingroup Expressions
  extern Function *makeCachedFunction(Operator const *op,
                                      Expression *expr,
                                      bool garbage);

  //! \brief Construct a Function with result cache.
  //! \param op Pointer to the Operator for this function.
  //! \param expr1 Pointer to the function's first argument.
  //! \param expr2 Pointer to the function's second argument.
  //! \param garbage1 true if the first argument should be deleted with the function, false otherwise.
  //! \param garbage2 true if the second argument should be deleted with the function, false otherwise.
  //! \return Pointer to the newly allocated Function.
  //! \note Convenience wrapper for unit test.
  //! \ingroup Expressions
  extern Function *makeCachedFunction(Operator const *op, 
                                      Expression *expr1,
                                      Expression *expr2,
                                      bool garbage1,
                                      bool garbage2);

}

#endif // PLEXIL_CACHED_FUNCTION_HH
