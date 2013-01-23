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

#ifndef _H_CoreExpressions
#define _H_CoreExpressions

#include "Calculable.hh"
#include "PlexilPlan.hh"
#include "Variable.hh"
#include <limits>
#include <set>

namespace PLEXIL 
{

  //
  // State variable
  //

  class StateVariable : public VariableImpl 
  {
  public:
    //state names
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(INACTIVE,
                                            "INACTIVE"); /*!< The inactive state. The initial state for a node. */
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(WAITING,
                                            "WAITING"); /*!< The waiting state. Occupied when a node's
                                                          parent is executing and the node's start
                                                          condition is not true.*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(EXECUTING,
                                            "EXECUTING"); /*!< The executing state.*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(FINISHING,
                                            "FINISHING");/*!< The finishing state. Only occupied by list
                                                           nodes whose end condition is true but whose
                                                           children haven't finished or failed.*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(FINISHED,
                                            "FINISHED"); /*!< The finished state. The node has completed executing.*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(FAILING,
                                            "FAILING"); /*!< The failing node state. Only occupied by list
                                                          nodes whose invariant or ancestor-invariant
                                                          condition is false. Essentially a waiting state
                                                          for children or command/update abort to finish.*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(ITERATION_ENDED,
                                            "ITERATION_ENDED");
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(NO_STATE,
                                            "NO_STATE"); /*!< The non-state. Nothing should *ever* be in
                                                           this state. Used internally to signify no
                                                           state transition is possible. */

    static const std::vector<Value>& ALL_STATES();
    static ExpressionId& INACTIVE_EXP();
    static ExpressionId& WAITING_EXP();
    static ExpressionId& EXECUTING_EXP();
    static ExpressionId& FINISHING_EXP();
    static ExpressionId& FINISHED_EXP();
    static ExpressionId& FAILING_EXP();
    static ExpressionId& ITERATION_ENDED_EXP();
    static ExpressionId& NO_STATE_EXP();

    static const Value& nodeStateName(NodeState state);

    // Constructors
    StateVariable(const std::string& name);
    StateVariable(const Value& value, 
                  const bool isConst = false);
    StateVariable(const PlexilExprId& expr,
                  const NodeConnectorId& node, 
                  const bool isConst = false);
    void print(std::ostream& s) const;

    void setNodeState(NodeState newValue);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return NODE_STATE; }

  private:
    bool checkValue(const Value& val) const;
  };

  class OutcomeVariable : public VariableImpl
  {
  public:
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(SUCCESS,
                                            "SUCCESS"); /*!< A successful node execution (post-condition is true after finishing).*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(FAILURE,
                                            "FAILURE"); /*!< Failure (with some failure type).*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(SKIPPED,
                                            "SKIPPED"); /*!< The node was skipped without executing (the ancestor-invariant was false or the parent's end was true before*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(INTERRUPTED,
                                            "INTERRUPTED"); /*!< Exit condition or ancestor exit condition true while executing */

    OutcomeVariable(const std::string& name);
    OutcomeVariable(const PlexilExprId& expr,
                    const NodeConnectorId& node, 
                    const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return NODE_OUTCOME; }

  protected:
  private:
    bool checkValue(const Value& val) const;
  };

  class FailureVariable : public VariableImpl 
  {
  public:
    //failure types (one for each condition, command failure)
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(PRE_CONDITION_FAILED,
                                            "PRE_CONDITION_FAILED"); /*!< The pre-condition was false
                                                                       (checked after the start condition
                                                                       is true).*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(POST_CONDITION_FAILED,
                                            "POST_CONDITION_FAILED"); /*!< The post-condition was false
                                                                        (checked after the end condition is
                                                                        true*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(INVARIANT_CONDITION_FAILED,
                                            "INVARIANT_CONDITION_FAILED"); /*!< The invariant condition was
                                                                             false (checked when
                                                                             executing).*/
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(PARENT_FAILED,
                                            "PARENT_FAILED"); /*!< Ancestor invariant false */
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(EXITED,
                                            "EXITED"); /*!< Exit condition true */
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(PARENT_EXITED,
                                            "PARENT_EXITED"); /*!< Ancestor exit condition true */

    FailureVariable(const std::string& name);
    FailureVariable(const PlexilExprId& expr,
                    const NodeConnectorId& node,
                    const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return FAILURE_TYPE; }

  protected:
  private:
    bool checkValue(const Value& val) const;
  };

  class CommandHandleVariable : public VariableImpl 
  {
  public:
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_SENT_TO_SYSTEM, "COMMAND_SENT_TO_SYSTEM"); 
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_ACCEPTED, "COMMAND_ACCEPTED");
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_RCVD_BY_SYSTEM, "COMMAND_RCVD_BY_SYSTEM");
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_FAILED, "COMMAND_FAILED");
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_DENIED, "COMMAND_DENIED");
    DECLARE_STATIC_CLASS_CONST_STRING_VALUE(COMMAND_SUCCESS, "COMMAND_SUCCESS");

    CommandHandleVariable(const std::string& name);
    CommandHandleVariable(const PlexilExprId& expr,
                          const NodeConnectorId& node, 
                          const bool isConst = false);
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return COMMAND_HANDLE; }

  protected:
  private:
    bool checkValue(const Value& val) const;
  };

  class AllChildrenFinishedCondition : public Calculable 
  {
  public:
    AllChildrenFinishedCondition(const std::vector<NodeId>& children);
    virtual ~AllChildrenFinishedCondition();
    void print(std::ostream& s) const;
    Value recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:

    friend class FinishedListener;
    const char* operatorString() const { return "AllChildrenFinished"; }
    void incrementCount();
    void decrementCount();

  private:
    class FinishedListener : public ExpressionListener 
    {
    public:
      FinishedListener(AllChildrenFinishedCondition& cond);
      FinishedListener(const FinishedListener& orig);
      void notifyValueChanged(const ExpressionId& expression);
      void setLastValue(const Value& value) { m_lastValue = value; }
    private:
      AllChildrenFinishedCondition& m_cond;
      Value m_lastValue;
    };

    bool checkValue(const Value& val) const;
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    const size_t m_total;
    size_t m_count;
    std::vector<VariableId> m_stateVariables;
    std::vector<FinishedListener> m_childListeners;
  };

  class AllChildrenWaitingOrFinishedCondition : public Calculable 
  {
  public:
    AllChildrenWaitingOrFinishedCondition(const std::vector<NodeId>& children);
    virtual ~AllChildrenWaitingOrFinishedCondition();
    void print(std::ostream& s) const;
    Value recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    friend class WaitingOrFinishedListener;

    const char* operatorString() const { return "AllChildrenWaitingOrFinished"; }
    void incrementCount();
    void decrementCount();

  private:
    class WaitingOrFinishedListener : public ExpressionListener 
    {
    public:
      WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond);
      WaitingOrFinishedListener(const WaitingOrFinishedListener& orig);
      void notifyValueChanged(const ExpressionId& expression);
      void setLastValue(const Value& value) { m_lastValue = value; }
    private:
      AllChildrenWaitingOrFinishedCondition& m_cond;
      Value m_lastValue;
    };

    bool checkValue(const Value& val) const;
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    const size_t m_total;
    size_t m_count;
    std::vector<VariableId> m_stateVariables;
    std::vector<WaitingOrFinishedListener> m_childListeners;
  };

  //used for EQInternal and NEQInternal
  class InternalCondition : public Calculable 
  {
  public:
    InternalCondition(const PlexilExprId& expr);
    InternalCondition(const PlexilExprId& expr, const NodeConnectorId& node);
    virtual ~InternalCondition();
    Value recalculate();
    void print(std::ostream& s) const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }
  protected:
    const char* operatorString() const { return "InternalCondition"; }

  private:
    bool checkValue(const Value& value) const;
    ExpressionId m_expr;
  };

  class InterruptibleCommandHandleValues : public UnaryExpression 
  {
  public:
    InterruptibleCommandHandleValues(const PlexilExprId& expr, const NodeConnectorId& node);
    InterruptibleCommandHandleValues(ExpressionId e);

    void print(std::ostream& s) const;
    Value recalculate();
    bool checkValue(const Value& val) const;
    
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }
    
  protected:
    const char* operatorString() const { return "InterruptibleCommandHandleValues"; }
  };

}

#endif
