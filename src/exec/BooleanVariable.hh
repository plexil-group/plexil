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

#ifndef BOOLEAN_VARIABLE_HH
#define BOOLEAN_VARIABLE_HH

#include "Variable.hh"
#include "ConstantMacros.hh"

namespace PLEXIL
{

  //this class represents boolean values
  //from the <BooleanValue> XML
  class BooleanVariable : public VariableImpl {
  public:
    static ExpressionId& TRUE_EXP();
    static ExpressionId& FALSE_EXP();
    static ExpressionId& UNKNOWN_EXP(); // used in Node condition defaults
    DECLARE_STATIC_CLASS_CONST(double, TRUE_VALUE, 1.0);
    DECLARE_STATIC_CLASS_CONST(double, FALSE_VALUE, 0.0);

    BooleanVariable(const bool isConst = false);
    BooleanVariable(const double value, const bool isConst = false);
    BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    std::string toString() const;
    static bool falseOrUnknown(double value) {return value != TRUE_VALUE();}

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double val);
  };

}

#endif // BOOLEAN_VARIABLE_HH
