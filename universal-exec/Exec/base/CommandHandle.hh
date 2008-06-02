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

#ifndef H_CommandHandle
#define H_CommandHandle

#include "CoreExpressions.hh"

namespace PLEXIL {

  class InterruptibleCommandHandleValues : public UnaryExpression {
  public:
    InterruptibleCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node)
      : UnaryExpression(expr, node) {}
    InterruptibleCommandHandleValues(ExpressionId e) : UnaryExpression(e) {}

    std::string toString() const 
    {
      std::stringstream retval;
      retval << Expression::toString();
      retval << "interruptibleCommandHandleValues(" << m_e->toString();
      retval << "))";
      return retval.str();
    }

    double recalculate()
    {
      double v = m_e->getValue();
      if(v == Expression::UNKNOWN())
        return false;
      else if ((v == CommandHandleVariable::COMMAND_DENIED()) || 
              (v == CommandHandleVariable::COMMAND_FAILED()))
        return true;
      return false;
    }

    bool checkValue(const double val)
    {
      return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE();
    }
    
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }
    
  protected:
  private:
  };

  class AllCommandHandleValues : public UnaryExpression {
  public:
    AllCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node)
      : UnaryExpression(expr, node) {}
    AllCommandHandleValues(ExpressionId e) : UnaryExpression(e) {}

    std::string toString() const 
    {
      std::stringstream retval;
      retval << Expression::toString();
      retval << "allCommandHandleValues(" << m_e->toString();
      retval << "))";
      return retval.str();
    }

    double recalculate()
    {
      double v = m_e->getValue();
      if(v == Expression::UNKNOWN())
        return false;
      else if((v == CommandHandleVariable::COMMAND_DENIED()) || 
              (v == CommandHandleVariable::COMMAND_FAILED()) ||
              (v == CommandHandleVariable::COMMAND_SENT_TO_SYSTEM()) ||
              (v == CommandHandleVariable::COMMAND_ACCEPTED()) ||
              (v == CommandHandleVariable::COMMAND_RCVD_BY_SYSTEM()) ||
              (v == CommandHandleVariable::COMMAND_SUCCESS()))
        return true;
      return false;
    }

    bool checkValue(const double val)
    {
      return val == BooleanVariable::TRUE() || val == BooleanVariable::FALSE();
    }
    
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }
    
  protected:
  private:
  };

}


#endif
