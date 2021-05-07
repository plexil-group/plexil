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

#ifndef PLEXIL_LOOKUP_HH
#define PLEXIL_LOOKUP_HH

#include "Expression.hh"
#include "Propagator.hh"

namespace PLEXIL
{

  // Forward references
  class Expression;
  class ExprVec;

  //!
  // @class Lookup
  // @brief Abstract base class representing the public Lookup API
  //
  class Lookup :
    public Expression,
    public Propagator
  {
  public:


    //!
    // @brief Get this lookup's high and low thresholds.
    // @param high Place to store the high threshold value.
    // @param low Place to store the low threshold value.
    // @return True if this lookup has active thresholds, false otherwise.
    //
    virtual bool getThresholds(Integer &high, Integer &low) const = 0;
    virtual bool getThresholds(Real &high, Real &low) const = 0;

    //!
    // @brief Notify this Lookup that its value has been updated.
    //
    virtual void valueChanged() = 0;

  };

  // Construct a Lookup expression.
  Expression *makeLookup(Expression *stateName,
                         bool stateNameIsGarbage,
                         ValueType declaredType,
                         ExprVec *paramVec);

  // Construct a LookupOnChange expression.
  Expression *makeLookupOnChange(Expression *stateName,
                                 bool stateNameIsGarbage,
                                 ValueType declaredType,
                                 Expression *tolerance,
                                 bool toleranceIsGarbage,
                                 ExprVec *paramVec);

} // namespace PLEXIL

#endif // PLEXIL_LOOKUP_HH
