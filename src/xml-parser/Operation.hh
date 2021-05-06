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

#ifndef PLEXIL_OPERATION_HH
#define PLEXIL_OPERATION_HH

#include "ValueType.hh"

#include <vector>

namespace PLEXIL
{

  // Forward references
  class Function;
  class Operator;

  //! @class Operation
  //! Abstract base class providing capabilities to check types
  //! and select the appropriate Operator for a function expression.
  //! The intent is to further decouple parsing and expression
  //! building from expression evaluation.

  class Operation
  {
  public:
    virtual ~Operation() = default;

    // Get the name of this operation.
    virtual std::string const &getName() const = 0;

    // Return true if the given argument count is legal for the operation.
    virtual bool checkArgCount(size_t count) const = 0;

    // Return true iff the given argument types are legal for the operation.
    virtual bool checkArgTypes(std::vector<ValueType> const &typeVec) const = 0;

    // Return the value type of the operation with the given argument types.
    virtual ValueType getValueType(std::vector<ValueType> const &typeVec,
                                   ValueType desiredType) const = 0;

    // Return the appropriate Operator instance for the given argument types.
    virtual Operator const *getOperator(std::vector<ValueType> const &typeVec,
                                        ValueType desiredType) const = 0;

    // Construct the Function instance for the expression.
    virtual Function *constructFunction(Operator const *oper, size_t nArgs) const = 0;

  protected:
    // Only available to derived classes.
    Operation() = default;

  private:
    // Not implemented
    Operation(Operation const &) = delete;
    Operation(Operation &&) = delete;
    Operation &operator=(Operation const &) = delete;
    Operation &operator=(Operation &&) = delete;
  };

} // namespace PLEXIL

#endif // PLEXIL_OPERATION_HH
