/* Copyright (c) 2006-2013, Universities Space Research Association (USRA).
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

#ifndef VARIABLES_HH
#define VARIABLES_HH

#include "Variable.hh"

namespace PLEXIL
{

  /**
   * variables MUST implement:
   * bool checkValue(const Value& val) const
   *
   * variables MAY implement:
   * destructor
   * PlexilType getValueType() const
   * void setValue(const Value& val)
   * void print(std::ostream& s) const
   * std::string valueString() const (BUT SHOULD RESPECT m_active!!)
   * void handleActivate(const bool changed)
   * void handleDeactivate(const bool changed)
   *
   */

  class StringVariable : public VariableImpl {
  public:
    StringVariable();
    StringVariable(const Value& value, const bool isConst = false);
    StringVariable(const LabelStr& value, const bool isConst = false); // TODO: see if still in use
    StringVariable(const PlexilExprId& expr,
                   const NodeConnectorId& node,
                   const bool isConst = false);
    void print(std::ostream& s) const;
    bool checkValue(const Value& val) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return STRING; }
  };

  class RealVariable : public VariableImpl {
  public:
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(RealVariable, ZERO_EXP, Value((double) 0), "Real constant 0");
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(RealVariable, ONE_EXP, Value((double) 1), "Real constant 1");
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(RealVariable, MINUS_ONE_EXP, Value((double) -1), "Real constant -1");

    RealVariable();
    RealVariable(const Value& value, const bool isConst = false);
    RealVariable(const PlexilExprId& expr, 
                 const NodeConnectorId& node,
                 const bool isConst = false);
    void print(std::ostream& s) const;
    bool checkValue(const Value& val) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return REAL; }
  };

  class IntegerVariable : public VariableImpl {
  public:
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(IntegerVariable, ZERO_EXP, Value((int32_t) 0), "Integer constant 0");
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(IntegerVariable, ONE_EXP, Value((int32_t) 1), "Integer constant 1");
    DECLARE_STATIC_CLASS_EXPRESSION_ID_CONSTANT(IntegerVariable, MINUS_ONE_EXP, Value((int32_t) -1), "Integer constant -1");

    IntegerVariable();
    IntegerVariable(const Value& value, const bool isConst = false);
    IntegerVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    void print(std::ostream& s) const;
    bool checkValue(const Value& val) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return INTEGER; }
  };

  class TimepointVariable : public AliasVariable
  {
  public:
    TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return TIME; }

  private:
	// Deliberately unimplemented
	TimepointVariable();
	TimepointVariable(const TimepointVariable&);
	TimepointVariable& operator=(const TimepointVariable&);
  };

}

#endif // VARIABLES_HH
