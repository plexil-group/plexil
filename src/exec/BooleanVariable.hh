/* Copyright (c) 2006-2012, Universities Space Research Association (USRA).
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
  class BooleanVariable : public VariableImpl 
  {
  public:
    DECLARE_STATIC_CLASS_CONST(Value, TRUE_VALUE, 1.0)
    DECLARE_STATIC_CLASS_CONST(Value, FALSE_VALUE, 0.0)

    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(BooleanVariable, TRUE_EXP, TRUE_VALUE(), "Boolean constant true")
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(BooleanVariable, FALSE_EXP, FALSE_VALUE(), "Boolean constant false")
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(BooleanVariable, UNKNOWN_EXP, UNKNOWN(), "Boolean constant unknown") // used in Node condition defaults

    BooleanVariable(const bool isConst = false);
    BooleanVariable(const Value& value,
                    const bool isConst = false);
    BooleanVariable(const PlexilExprId& expr,
                    const NodeConnectorId& node,
                    const bool isConst = false);
    void print(std::ostream& s) const;
    static bool falseOrUnknown(const Value& value) {return value != TRUE_VALUE();}

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  private:
    bool checkValue(const Value& val) const;
  };

}

#endif // BOOLEAN_VARIABLE_HH
