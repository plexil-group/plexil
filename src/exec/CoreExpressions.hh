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

#ifndef _H_CoreExpressions
#define _H_CoreExpressions

#include "Calculable.hh"
#include "PlexilPlan.hh"
#include "Variable.hh"
#include <limits>
#include <list>
#include <set>

namespace PLEXIL {

  /**
   * Class to provide an interface that doesn't propagate activation/deactivation messages.
   * Used in Node ancestor end and ancestor invariant conditions.
   */
  class TransparentWrapper : public Expression {
  public:

    TransparentWrapper(const ExpressionId& exp, const NodeConnectorId& node);

	// This variant used only in unit tests
    TransparentWrapper(const ExpressionId& exp);

    virtual ~TransparentWrapper();
    void setValue(const double value);
    void print(std::ostream& s) const;
    std::string valueString() const;
    bool checkValue(const double value);

  private:

	// Deliberately not implemented
    TransparentWrapper();
	TransparentWrapper& operator=(const TransparentWrapper&);

	void commonInit(const ExpressionId& exp);

    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);
    virtual void handleChange(const ExpressionId& expression);

    class WrapperListener : public ExpressionListener {
    public:
      WrapperListener(TransparentWrapper& wrapper) : ExpressionListener(), m_wrapper(wrapper) {}
      void notifyValueChanged(const ExpressionId& expression) {
	m_wrapper.handleChange(expression);
      }
    private:
      TransparentWrapper& m_wrapper;
    };

    WrapperListener m_listener;
    ExpressionId m_exp;
  };

  //
  // State variable
  //

  class StateVariable : public VariableImpl {
  public:
    //state names
    DECLARE_STATIC_CLASS_CONST(LabelStr, INACTIVE,
			       "INACTIVE"); /*<! The inactive state.  The initial state for a
					      node. */
    DECLARE_STATIC_CLASS_CONST(LabelStr, WAITING,
			       "WAITING"); /*<! The waiting state.  Occupied when a node's
					     parent is executing and the node's start
					     condition is not true.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, EXECUTING, "EXECUTING"); /*<! The executing state.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, FINISHING,
			       "FINISHING");/*<! The finishing state.  Only occupied by list
					      nodes whose end condition is true but whose
					      children haven't finished or failed.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, FINISHED,
			       "FINISHED"); /*<! The finished state.  The node has completed
					      executing.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, FAILING,
			       "FAILING"); /*<! The failing node state.  Only occupied by list
					     nodes whose invariant or ancestor-invariant
					     condition is false.  Essentially a waiting state
					     for children to finish.*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, ITERATION_ENDED, "ITERATION_ENDED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, NO_STATE,
			       "NO_STATE"); /*<! The non-state.  Nothing should *ever* be in
					      this state.*/
    static const std::vector<LabelStr>& ALL_STATES();
    static ExpressionId& INACTIVE_EXP();
    static ExpressionId& WAITING_EXP();
    static ExpressionId& EXECUTING_EXP();
    static ExpressionId& FINISHING_EXP();
    static ExpressionId& FINISHED_EXP();
    static ExpressionId& FAILING_EXP();
    static ExpressionId& ITERATION_ENDED_EXP();
    static ExpressionId& NO_STATE_EXP();

    static const LabelStr& nodeStateName(NodeState state);
    static NodeState nodeStateFromName(double nameAsLabelStrKey);

    // Constructors
    StateVariable(const std::string& name);
    StateVariable(const double value, const bool isConst = false);
    StateVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    void print(std::ostream& s) const;

    void setNodeState(NodeState newValue);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return NODE_STATE; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class OutcomeVariable : public VariableImpl {
  public:
    DECLARE_STATIC_CLASS_CONST(LabelStr, SUCCESS, "SUCCESS"); /*<! A successful node execution (post-condition is true after finishing).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, FAILURE, "FAILURE"); /*<! Failure (with some failure type).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, SKIPPED, "SKIPPED"); /*<! The node was skipped without executing (the ancestor-invariant was false or the parent's end was true before*/

    OutcomeVariable(const std::string& name);
    OutcomeVariable(const double value, const bool isConst = false);
    OutcomeVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return NODE_OUTCOME; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class FailureVariable : public VariableImpl {
  public:
    //failure types (one for each condition, command failure)
    DECLARE_STATIC_CLASS_CONST(LabelStr, PRE_CONDITION_FAILED,
			       "PRE_CONDITION_FAILED"); /*<! The pre-condition was false
							 (checked after the start condition
							 is true).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, POST_CONDITION_FAILED,
			       "POST_CONDITION_FAILED"); /*<! The post-condition was false
							  (checked after the end condition is
							  true*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, INVARIANT_CONDITION_FAILED,
			       "INVARIANT_CONDITION_FAILED"); /*<! The invariant condition was
							       false (checked when
							       executing).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, PARENT_FAILED, "PARENT_FAILED");

    FailureVariable(const std::string& name);
    FailureVariable(const double value, const bool isConst = false);
    FailureVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return FAILURE_TYPE; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class CommandHandleVariable : public VariableImpl {
  public:
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_SENT_TO_SYSTEM, "COMMAND_SENT_TO_SYSTEM"); 
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ACCEPTED, "COMMAND_ACCEPTED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_RCVD_BY_SYSTEM, "COMMAND_RCVD_BY_SYSTEM");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_FAILED, "COMMAND_FAILED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_DENIED, "COMMAND_DENIED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ABORTED, "COMMAND_ABORTED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ABORT_FAILED, "COMMAND_ABORT_FAILED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_SUCCESS, "COMMAND_SUCCESS");

    CommandHandleVariable(const std::string& name);
    CommandHandleVariable(const double value, const bool isConst = false);
    CommandHandleVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return COMMAND_HANDLE; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class AllChildrenFinishedCondition : public Calculable 
  {
  public:
	AllChildrenFinishedCondition(const std::vector<NodeId>& children);
	virtual ~AllChildrenFinishedCondition();
	void print(std::ostream& s) const;
	double recalculate();

	/**
	 * @brief Retrieve the value type of this Expression.
	 * @return The value type of this Expression.
	 */
	virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
	friend class FinishedListener;
	void incrementCount();
	void decrementCount();

  private:
	class FinishedListener : public ExpressionListener 
	{
	public:
	  FinishedListener(AllChildrenFinishedCondition& cond);
	  FinishedListener(const FinishedListener& orig);
	  void notifyValueChanged(const ExpressionId& expression);
	  void setLastValue(double value) { m_lastValue = value; }
	private:
	  AllChildrenFinishedCondition& m_cond;
	  double m_lastValue;
	};

	bool checkValue(const double val);
	void handleActivate(const bool changed);
	void handleDeactivate(const bool changed);

	const unsigned int m_total;
	unsigned int m_count;
	std::vector<VariableId> m_stateVariables;
	std::vector<FinishedListener> m_childListeners;
  };

  class AllChildrenWaitingOrFinishedCondition : public Calculable 
  {
  public:
    AllChildrenWaitingOrFinishedCondition(const std::vector<NodeId>& children);
    virtual ~AllChildrenWaitingOrFinishedCondition();
    void print(std::ostream& s) const;
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    friend class WaitingOrFinishedListener;
    void incrementCount();
    void decrementCount();

  private:
    class WaitingOrFinishedListener : public ExpressionListener 
	{
    public:
      WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond);
      WaitingOrFinishedListener(const WaitingOrFinishedListener& orig);
      void notifyValueChanged(const ExpressionId& expression);
	  void setLastValue(double value) { m_lastValue = value; }
    private:
      AllChildrenWaitingOrFinishedCondition& m_cond;
	  double m_lastValue;
    };

    bool checkValue(const double val);
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    const unsigned int m_total;
    unsigned int m_count;
    std::vector<VariableId> m_stateVariables;
	std::vector<WaitingOrFinishedListener> m_childListeners;
  };

  //used for EQInternal and NEQInternal
  class InternalCondition : public Calculable {
  public:
    InternalCondition(const PlexilExprId& expr);
    InternalCondition(const PlexilExprId& expr, const NodeConnectorId& node);
    virtual ~InternalCondition();
    double recalculate();
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }
  protected:
  private:
    bool checkValue(const double value);
    ExpressionId m_first, m_second, m_expr;
  };

  class InterruptibleCommandHandleValues : public UnaryExpression {
  public:
    InterruptibleCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node);
    InterruptibleCommandHandleValues(ExpressionId e);

    void print(std::ostream& s) const;
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

  class AllCommandHandleValues : public UnaryExpression {
  public:
    AllCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node);
    AllCommandHandleValues(ExpressionId e);

    void print(std::ostream& s) const;
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

}

#endif
