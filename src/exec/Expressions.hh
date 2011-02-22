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

#ifndef _H_Expressions
#define _H_Expressions

#include "CoreExpressions.hh"
#include "PlexilPlan.hh"
#include "LabelStr.hh"
#include <list>
#include <set>

namespace PLEXIL {
  class Lookup : public Variable {
  public:
    Lookup(const PlexilExprId& expr, const NodeConnectorId& node);
    virtual ~Lookup();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     * @note This should access global declarations if possible.
     */
    virtual PlexilType getValueType() const;

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp) = 0;

    /**
     * @brief Get a printable representation of the state.
     */
    static std::string stateToString(const State&);

  protected:
    /**
     * @brief Check subexpression values to see if m_state is still current.
     * @return true if state is current, false if changed since last updated.
     */
    bool isStateCurrent() const;

    /**
     * @brief Update m_state to match current values of the subexpressions.
     */
    void updateState();

    /**
     * @brief Check to make sure a value is appropriate for this expression.
     * @note The current method simply returns true.
     * @note In a future enhancement, this should validate against global declarations.
     */
    bool checkValue(const double /* value */)
    { 
      return true;
    }

    /**
     * @brief Handle the activation of the expression.
     * @param changed True if the call to activate actually caused a change from inactive to
     *                active.
     */
    void handleActivate(const bool changed); //registers

    /**
     * @brief Handle the deactivation of the expression
     * @param changed True if the call to deactivate actually caused a change from active to
     *                inactive.
     */
    void handleDeactivate(const bool changed); //unregisters

    void registerLookup();
    void unregisterLookup();

    virtual void handleRegistration() = 0;
    virtual void handleUnregistration() = 0;

    StateCacheId m_cache;
    ExpressionId m_stateNameExpr;
    std::vector<ExpressionId> m_params;
    std::vector<ExpressionId> m_garbage;
    Expressions m_dest;
    State m_state;
    SubexpressionListener m_listener;

  private:
    // Deliberately unimplemented
    Lookup();
    Lookup(const Lookup&);
    Lookup& operator=(const Lookup&);

    void getArguments(const std::vector<PlexilExprId>& args, const NodeConnectorId& node);
  };

  class LookupNow : public Lookup {
  public:
    LookupNow(const PlexilExprId& expr, const NodeConnectorId& node);
    std::string toString() const;
 
    void handleChange(const ExpressionId& exp);

  protected:

  private:
    // Deliberately unimplemented
    LookupNow();
    LookupNow(const LookupNow&);
    LookupNow& operator=(const LookupNow&);

    void handleRegistration();
    void handleRegistrationChange(const State& oldState);
    void handleUnregistration();
  };

  class LookupOnChange : public Lookup {
  public:
    LookupOnChange(const PlexilExprId& expr, const NodeConnectorId& node);
    virtual ~LookupOnChange();

    std::string toString() const;
 
    void handleChange(const ExpressionId& exp);

  protected:

  private:
    // Deliberately unimplemented
    LookupOnChange();
    LookupOnChange(const LookupOnChange&);
    LookupOnChange& operator=(const LookupOnChange&);

    void handleRegistration();
    void handleRegistrationChange(const State& oldState);
    void handleUnregistration();

    ExpressionId m_tolerance;
  };

  class AbsoluteValue : public UnaryExpression {
  public:
    AbsoluteValue(const PlexilExprId& expr, const NodeConnectorId& node);
    AbsoluteValue(ExpressionId e);
    std::string toString() const;
    double recalculate();
    bool checkValue(const double val);
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
  };

  class SquareRoot : public UnaryExpression {
  public:
    SquareRoot(const PlexilExprId& expr, const NodeConnectorId& node);
    SquareRoot(ExpressionId e);
    std::string toString() const;
    double recalculate();
    bool checkValue(const double val);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return REAL; }
  protected:
  private:
  };

  class IsKnown : public UnaryExpression {
  public:
    IsKnown(const PlexilExprId& expr, const NodeConnectorId& node);
    IsKnown(ExpressionId e);
    std::string toString() const;
    double recalculate();
    bool checkValue(const double val);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
  };

  /**
   * @brief Performs registration of the default expression classes.
   */

  extern void initializeExpressions();

}

#endif
