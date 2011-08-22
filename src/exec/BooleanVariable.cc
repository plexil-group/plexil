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
	       "Attempted to initialize a variable with an invalid value.");
  }

  BooleanVariable::BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
				   const bool /* isConst */)
    : VariableImpl(expr, node)
  {
    checkError(Id<PlexilValue>::convertable(expr), "Expected a value.");
    commonNumericInit((PlexilValue*)expr);
  }

  std::string BooleanVariable::toString() const
  {
    std::ostringstream retval;
    retval << VariableImpl::toString();
    retval << "boolean)";
    return retval.str();
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
