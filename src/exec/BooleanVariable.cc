/* Copyright (c) 2006-2011, Universities Space Research Association (USRA).
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

#include "BooleanVariable.hh"
#include "Debug.hh"

namespace PLEXIL
{

  BooleanVariable::BooleanVariable(const bool isConst)
	: VariableImpl(isConst)
  {
  }

  BooleanVariable::BooleanVariable(const double value, const bool isConst)
    : VariableImpl(value, isConst) 
  {
    checkError(checkValue(value),
			   "Attempted to initialize a Boolean variable to an invalid value \"" << valueToString(value) << "\"");
  }

  BooleanVariable::BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool isConst)
    : VariableImpl(expr, node)
  {
	assertTrueMsg(expr.isValid(), "Attempt to create a BooleanVariable from an invalid Id");
	const PlexilValue* value = NULL;
	if (Id<PlexilVar>::convertable(expr)) {
	  const Id<PlexilVar> var = (const Id<PlexilVar>) expr;
	  // If the ExpressionFactory is correctly set up, should NEVER EVER happen
	  assertTrueMsg(var->type() == BOOLEAN,
					"Attempt to create a BooleanVariable from a non-BOOLEAN PlexilVar");
	  value = var->value();
	}
	else if (Id<PlexilValue>::convertable(expr)) {
	  value = (const PlexilValue*) expr;
	  assertTrueMsg(isConst, "Attempt to create a BooleanValue that is not const");
	}
	else {
	  assertTrueMsg(ALWAYS_FAIL, "Expected a PlexilVar or PlexilValue");
	}

	assertTrueMsg(value == NULL || value->type() == BOOLEAN,
				  "Attempt to create a BooleanVariable from a non-BOOLEAN PlexilVar");
	commonNumericInit(value);
  }

  void BooleanVariable::print(std::ostream& s) const
  {
	VariableImpl::print(s);
    s << "boolean)";
  }

  bool BooleanVariable::checkValue(const double val)
  {
    return val == UNKNOWN() || val == FALSE_VALUE() || val == TRUE_VALUE();
  }

  ExpressionId& BooleanVariable::TRUE_EXP()
  {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(TRUE_VALUE(), true);
	  var->setName("Boolean constant true");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  ExpressionId& BooleanVariable::FALSE_EXP()
  {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(FALSE_VALUE(), true);
	  var->setName("Boolean constant false");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

  ExpressionId& BooleanVariable::UNKNOWN_EXP()
  {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId()) {
	  VariableImpl* var = new BooleanVariable(UNKNOWN(), true);
	  var->setName("Boolean constant unknown");
      sl_exp = var->getId();
	}
    if(!sl_exp->isActive())
      sl_exp->activate();
    return sl_exp;
  }

}
