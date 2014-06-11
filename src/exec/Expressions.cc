/* Copyright (c) 2006-2008, Universities Space Research Association (USRA).
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

#include "Expressions.hh"
#include "Array.hh"
#include "BooleanVariable.hh"
#include "Calculables.hh"
#include "CoreExpressions.hh"
#include "Debug.hh"
#include "ExpressionFactory.hh"
#include "Lookup.hh"
#include "Variables.hh"
#include "lifecycle-utils.h"

namespace PLEXIL {

  void initializeExpressions()
  {
    static bool sl_inited = false;
    if (!sl_inited) {
      addFinalizer(&ExpressionFactory::purge);
	  REGISTER_EXPRESSION(ArrayElement, ArrayElement);
	  REGISTER_EXPRESSION(Concatenation, CONCAT);
      // FIXME
	  REGISTER_CONSTANT_EXPRESSION(StateVariable, NodeStateValue);
	  REGISTER_CONSTANT_EXPRESSION(OutcomeVariable, NodeOutcomeValue);
	  REGISTER_CONSTANT_EXPRESSION(FailureVariable, NodeFailureValue);
	  REGISTER_CONSTANT_EXPRESSION(CommandHandleVariable, NodeCommandHandleValue);
      // END FIXME
	  REGISTER_EXPRESSION(InternalCondition, EQInternal);
	  REGISTER_EXPRESSION(InternalCondition, NEInternal);
	  REGISTER_EXPRESSION(LookupNow, LookupNow);
	  REGISTER_EXPRESSION(LookupOnChange, LookupOnChange);
	  REGISTER_EXPRESSION(NodeTimepointValue, NodeTimepointValue);
	  REGISTER_EXPRESSION(AbsoluteValue, ABS);
	  REGISTER_EXPRESSION(SquareRoot, SQRT);
	  sl_inited = true;
	}
  }

} // namespace PLEXIL
