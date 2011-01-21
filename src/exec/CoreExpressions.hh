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

#include "Expression.hh"
#include "PlexilPlan.hh"
#include <limits>
#include <list>
#include <set>

namespace PLEXIL {

  /**
   * variables MUST implement:
   * bool checkValue(const double val)
   *
   * variables MAY implement:
   * destructor
   * PlexilType getValueType() const
   * void setValue(const double val)
   * std::string toString() const
   * std::string valueString() const
   * void handleLock()
   * void handleUnlock()
   * void handleActivate(const bool changed)
   * void handleDeactivate(const bool changed)
   * handleReset()
   *
   */

  class ArrayVariable : public Variable 
  {
  public:
    ArrayVariable(unsigned maxSize, PlexilType type, 
                  const bool isConst = false);
    ArrayVariable(unsigned maxSize, PlexilType type, 
                  std::vector<double>& values,
                  const bool isConst = false);
    ArrayVariable(const PlexilExprId& expr, 
                  const NodeConnectorId& node,
                  const bool isConst = false);

    virtual ~ArrayVariable();

    std::string toString() const;
    double lookupValue(unsigned long index);
    unsigned maxSize() {return m_maxSize;}

    /**
     * @brief Set the contents of this array from the given values.
     */
    void setValues(std::vector<double>& values);

    /**
     * @brief Set the contents of this array from the given array value.
     */
    void setValues(const double key);

    /**
     * @brief Set the contents of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     */
    void setValue(const double value);

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    void reset();

    /**
     * @brief Set one element of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     * @note Index must be less than maximum length
     */
    void setElementValue(unsigned index, const double value);

    /**
     * @brief Retrieve the element type of this array.
     * @return The element type of this array.
     */
    const PlexilType& getElementType() { return m_type; }

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return ARRAY; }

    /**
     * @brief Check to make sure an element value is appropriate for this array.
     */
    bool checkElementValue(const double val);

    /**
     * @brief Notify this array that an element's value has changed.
     * @param elt The changed element.
     */
    virtual void handleElementChanged(const ExpressionId& elt);

  protected:
  private:
    size_t              m_maxSize;
    PlexilType          m_type;
    std::vector<double>	m_initialVector;

    /**
     * @brief Check to make sure a value is appropriate for this variable.
     */
    bool checkValue(const double val);

    /**
     * @brief Check to make sure the index is appropriate for this array.
     */
    bool checkIndex(const unsigned index) const 
    { return index < m_maxSize; }


    /**
     * @brief Notify listeners that an element has changed.
     * @param elt The changed element expression.
     */
    virtual void publishElementChange(const ExpressionId& elt);
  };

  typedef Id<ArrayVariable> ArrayVariableId;

  class StringVariable : public Variable {
  public:
    StringVariable(const bool isConst = false);
    StringVariable(const std::string& value, const bool isConst = false);
    StringVariable(const char* value, const bool isConst);
    StringVariable(const double value, const bool isConst = false);
    StringVariable(const LabelStr& value, const bool isConst = false);
    StringVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		   const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return STRING; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class RealVariable : public Variable {
  public:
    static ExpressionId& ZERO_EXP();
    static ExpressionId& ONE_EXP();
    static ExpressionId& MINUS_ONE_EXP();

    RealVariable(const bool isConst = false);
    RealVariable(const double value, const bool isConst = false);
    RealVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		 const bool isConst = false);
    std::string toString() const;
    std::string valueString() const;
    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return REAL; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class IntegerVariable : public Variable {
  public:
    static ExpressionId& ZERO_EXP();
    static ExpressionId& ONE_EXP();
    static ExpressionId& MINUS_ONE_EXP();

    IntegerVariable(const bool isConst = false);
    IntegerVariable(const double value, const bool isConst = false);
    IntegerVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    std::string toString() const;
    std::string valueString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return INTEGER; }

  protected:
  private:
    bool checkValue(const double val);
  };

#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif

  //this class represents boolean values
  //from the <BooleanValue> XML
  class BooleanVariable : public Variable {
  public:
    static ExpressionId& TRUE_EXP();
    static ExpressionId& FALSE_EXP();
    DECLARE_STATIC_CLASS_CONST(double, TRUE, 1.0);
    DECLARE_STATIC_CLASS_CONST(double, FALSE, 0.0);

    BooleanVariable(const bool isConst = false);
    BooleanVariable(const double value, const bool isConst = false);
    BooleanVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    std::string toString() const;
    std::string valueString() const;
    static bool falseOrUnknown(double value) {return value != TRUE();}

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double val);
  };


  //
  // State variable
  //

  class StateVariable : public Variable {
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
    StateVariable(const bool isConst = false);
    StateVariable(const double value, const bool isConst = false);
    StateVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    std::string toString() const;

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

  class OutcomeVariable : public Variable {
  public:
    DECLARE_STATIC_CLASS_CONST(LabelStr, SUCCESS, "SUCCESS"); /*<! A successful node execution (post-condition is true after finishing).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, FAILURE, "FAILURE"); /*<! Failure (with some failure type).*/
    DECLARE_STATIC_CLASS_CONST(LabelStr, SKIPPED, "SKIPPED"); /*<! The node was skipped without executing (the ancestor-invariant was false or the parent's end was true before*/

    OutcomeVariable(const bool isConst = false);
    OutcomeVariable(const double value, const bool isConst = false);
    OutcomeVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return NODE_OUTCOME; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class FailureVariable : public Variable {
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

    FailureVariable(const bool isConst = false);
    FailureVariable(const double value, const bool isConst = false);
    FailureVariable(const PlexilExprId& expr, const NodeConnectorId& node,
		    const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return FAILURE_TYPE; }

  protected:
  private:
    bool checkValue(const double val);
  };

  class CommandHandleVariable : public Variable {
  public:
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_SENT_TO_SYSTEM, "COMMAND_SENT_TO_SYSTEM"); 
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ACCEPTED, "COMMAND_ACCEPTED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_RCVD_BY_SYSTEM, "COMMAND_RCVD_BY_SYSTEM");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_FAILED, "COMMAND_FAILED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_DENIED, "COMMAND_DENIED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ABORTED, "COMMAND_ABORTED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_ABORT_FAILED, "COMMAND_ABORT_FAILED");
    DECLARE_STATIC_CLASS_CONST(LabelStr, COMMAND_SUCCESS, "COMMAND_SUCCESS");

    CommandHandleVariable(const bool isConst = false);
    CommandHandleVariable(const double value, const bool isConst = false);
    CommandHandleVariable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst = false);
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return COMMAND_HANDLE; }

  protected:
  private:
    bool checkValue(const double val);
  };


  /**
   *  An abstract base class representing any "variable" expression that depends upon another variable,
   *  including but not limited to array elements, aliases, etc.
   */
  class DerivedVariable : public EssentialVariable
  {
  public:
	/**
	 * @brief Constructor.
	 */
    DerivedVariable();

    DerivedVariable(const NodeConnectorId& node);

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp) = 0;
    
  protected:
    
  private:
    // deliberately unimplemented
    DerivedVariable(const DerivedVariable &);
    DerivedVariable & operator=(const DerivedVariable &);

  };

  /**
   *   A class for notifying derived variables (e.g. array variables,
   *   variable aliases, etc.) of changes in sub-expressions.
   */
  class DerivedVariableListener : public ExpressionListener {
  public:

    /**
     * @brief Constructor.
     * @param exp The expression to be notified of any changes.
     */
    DerivedVariableListener(const ExpressionId& exp);

    /**
     * @brief Notifies the destination expression of a value change.
     * @param exp The expression which has changed.
     */
    void notifyValueChanged(const ExpressionId& exp);

  private:

    // deliberately unimplemented
    DerivedVariableListener();
    DerivedVariableListener(const DerivedVariableListener&);
    DerivedVariableListener& operator=(const DerivedVariableListener&);

    ExpressionId m_exp; /*<! The destination expression for notifications. */
  };

  // Access to an element of an array

  class ArrayElement : public DerivedVariable
  {
  public:
    /**
     * @brief Constructor.
     * @param expr The PlexilArrayElementId expression used as a template.
     * @param node The NodeConnectorId used to find variable references.
     */
    ArrayElement(const PlexilExprId& expr, const NodeConnectorId& node);

    /**
     * @brief Destructor.
     */
    virtual ~ArrayElement();

    std::string toString() const;

    /**
     * @brief Sets the value of the array element.  
     *        Will throw an error if the variable was constructed with isConst == true.
     * @param value The new value for this array element.
     */
    void setValue(const double value);

    /**
     * @brief Gets the const-ness of this variable.
     * @return True if this variable is const, false otherwise.
     */
    bool isConst() const 
    {
      return m_arrayVariable->isConst();
    }

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

    /**
     * @brief Notify listeners that the value of this expression has changed.
     */
    virtual void publishChange();

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

    void handleActivate(const bool changed);

    void handleDeactivate(const bool changed);

    double recalculate();

  protected:

  private:

    // Deliberately unimplemented
    ArrayElement();
    ArrayElement(const ArrayElement&);
    ArrayElement& operator=(const ArrayElement&);
    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     */
    bool checkValue(const double value);

    ArrayVariableId m_arrayVariable;
    ExpressionId m_index;
    bool m_deleteIndex;
    DerivedVariableListener m_listener;
  };

  class AliasVariable : public DerivedVariable
  {
  public:
	/**
	 * @brief Constructor. Creates a variable that indirects to another variable.
	 * @param name The name of this variable in the node that constructed the alias.
	 * @param node The node which owns this alias.
	 * @param original The original variable for this alias.
	 * @param isConst True if assignments to the alias are forbidden.
	 */
	AliasVariable(const std::string& name, 
				  const NodeConnectorId& nodeConnector,
				  Id<EssentialVariable> original,
				  const bool isConst = false);

	virtual ~AliasVariable();

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
    std::string toString() const;

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    virtual void reset();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;
    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     */
    virtual bool checkValue(const double val);

    /**
     * @brief Sets the value of this variable.  Will throw an error if the variable was
     *        constructed with isConst == true.
     * @param value The new value for this variable.
     */
    virtual void setValue(const double value);

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

    /**
     * @brief Gets the const-ness of this variable.
     * @return True if this variable is const, false otherwise.
     */
    bool isConst() const {return m_isConst;}

    /**
     * @brief Get the name of this alias, as declared in the node that owns it.
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Get the target variable of this alias.
     */
	const Id<EssentialVariable>& getOriginalVariable() const { return m_originalVariable; }
	  

  protected:

    /**
     * @brief Handle the activation of the expression.
     * @param changed True if the call to activate actually caused a change from inactive to
     *                active.
     */
    virtual void handleActivate(const bool changed);

    /**
     * @brief Handle the deactivation of the expression
     * @param changed True if the call to deactivate actually caused a change from active to
     *                inactive.
     */
    virtual void handleDeactivate(const bool changed);

    /**
     * @brief Handle additional behaviors for the reset() call.
     */
    virtual void handleReset();

  private:
	
	// Deliberately unimplemented
	AliasVariable();
	AliasVariable(const AliasVariable&);
	AliasVariable& operator=(const AliasVariable&);

	Id<EssentialVariable> m_originalVariable;
	DerivedVariableListener m_listener;
	const std::string& m_name;
	bool m_isConst;
  };


  /**
   * Calculables MUST override
   * checkValue
   *
   * Calculables SHOULD override
   * toString
   * valueString
   * recalculate
   *
   * Calculables MAY override
   * setValue
   * handleLock
   * handleUnlock
   * handleActivate
   * handleDeactivate
   * handleChange
   * getVariable
   * handleSubexpressionChange
   */

  class UnaryExpression : public Calculable {
  public:
    UnaryExpression(const PlexilExprId& expr, const NodeConnectorId& node);
    UnaryExpression(const ExpressionId& e);
  protected:
    ExpressionId m_e;
  private:
  };


  class LogicalNegation : public UnaryExpression
  {
  public:
    LogicalNegation(const PlexilExprId& expr, const NodeConnectorId& node)
      : UnaryExpression(expr, node)
    {
    }

    LogicalNegation(ExpressionId& e)
      : UnaryExpression(e)
    {
    }

    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

    double recalculate();

  protected:

  private:
    bool checkValue(const double val);
  };


  class BinaryExpression : public Calculable {
  public:
    BinaryExpression(const PlexilExprId& expr, const NodeConnectorId& node);
    BinaryExpression(const ExpressionId& a, const ExpressionId& b);
    BinaryExpression(const ExpressionId& a, bool aGarbage, const ExpressionId& b,
		     bool bGarbage);
  protected:
    ExpressionId m_a;
    ExpressionId m_b;
  private:
  };
  // N-ary expression

  class NaryExpression : public Calculable
  {
  public:
    NaryExpression(const PlexilExprId& expr, 
                   const NodeConnectorId& node);
    NaryExpression(const ExpressionId& a, const ExpressionId& b);
    NaryExpression(const ExpressionId& a, bool aGarbage,
                   const ExpressionId& b, bool bGarbage);
  protected:
  private:
  };

  class Conjunction : public NaryExpression {
  public:
    Conjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node) {}
    Conjunction(const ExpressionId& a, const ExpressionId& b) : NaryExpression(a, b) {}
    Conjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}
    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class Disjunction : public NaryExpression
  {
  public:
    Disjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node)
    {
    }

    Disjunction(const ExpressionId& a, const ExpressionId& b)
      : NaryExpression(a, b)
    {
    }
    Disjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:

  private:
    bool checkValue(const double value);
  };

  class ExclusiveDisjunction : public NaryExpression
  {
  public:
    ExclusiveDisjunction(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node) {}

    ExclusiveDisjunction(const ExpressionId& a, const ExpressionId& b)
      : NaryExpression(a, b) {}
    ExclusiveDisjunction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:

  private:
    bool checkValue(const double value);
  };

  class Concatenation : public NaryExpression {
  public:
    Concatenation(const PlexilExprId& expr, const NodeConnectorId& node)
      : NaryExpression(expr, node) {}
    Concatenation(const ExpressionId& a, const ExpressionId& b) : NaryExpression(a, b) {}
    Concatenation(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : NaryExpression(a, aGarbage, b, bGarbage) {}
    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return STRING; }

  protected:
  private:
    bool checkValue(const double value);
  };

  //
  // Comparisons
  //

  class Equality : public BinaryExpression {
  public:
    Equality(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node) {}
    Equality(const ExpressionId& a, const ExpressionId& b) : BinaryExpression(a, b) {}
    Equality(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}
    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class Inequality : public BinaryExpression
  {
  public:
    Inequality(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Inequality(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Inequality(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class LessThan : public BinaryExpression
  {
  public:
    LessThan(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    LessThan(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    LessThan(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class LessEqual : public BinaryExpression
  {
  public:
    LessEqual(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    LessEqual(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    LessEqual(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class GreaterThan : public BinaryExpression
  {
  public:
    GreaterThan(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    GreaterThan(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    GreaterThan(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };

  class GreaterEqual : public BinaryExpression
  {
  public:
    GreaterEqual(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    GreaterEqual(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    GreaterEqual(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
  private:
    bool checkValue(const double value);
  };


  //
  // Arithmetic expressions
  //

  class Addition : public BinaryExpression
  {
  public:
    Addition(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Addition(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Addition(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Subtraction : public BinaryExpression
  {
  public:
    Subtraction(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Subtraction(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Subtraction(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Multiplication : public BinaryExpression
  {
  public:
    Multiplication(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Multiplication(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Multiplication(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
    bool checkValue(const double /* value */) {return true;}
  };

  class Division : public BinaryExpression
  {
  public:
    Division(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Division(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Division(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
    bool checkValue(const double /* value */) {return true;}
  };


  class Modulo : public BinaryExpression
  {
  public:
    Modulo(const PlexilExprId& expr, const NodeConnectorId& node)
      : BinaryExpression(expr, node)
    {}

    Modulo(const ExpressionId& a, const ExpressionId& b)
      : BinaryExpression(a, b)
    {}
    Modulo(const ExpressionId& a, bool aGarbage, const ExpressionId& b, bool bGarbage)
      : BinaryExpression(a, aGarbage, b, bGarbage) {}

    double recalculate();
    std::string toString() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const;

  protected:
  private:
    bool checkValue(const double /* value */) {return true;}
  };


  class AllChildrenFinishedCondition : public Calculable {
  public:
    AllChildrenFinishedCondition(std::list<NodeId>& children);
    ~AllChildrenFinishedCondition();
    std::string toString() const;
    void addChild(const NodeId& node);
    double recalculate();

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    friend class FinishedListener;
    void incrementCount(const ExpressionId& expr);
    void decrementCount(const ExpressionId& expr);
  private:
    class FinishedListener : public ExpressionListener {
    public:
      FinishedListener(AllChildrenFinishedCondition& cond);
      void notifyValueChanged(const ExpressionId& expression);
    protected:
    private:
      AllChildrenFinishedCondition& m_cond;
    };

    bool checkValue(const double val);
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    FinishedListener m_listener;
    unsigned int m_total;
    unsigned int m_count;
    bool m_constructed;
    std::list<NodeId> m_children;
    std::map<ExpressionId, double> m_lastValues;
  };

  class AllChildrenWaitingOrFinishedCondition : public Calculable {
  public:
    AllChildrenWaitingOrFinishedCondition(std::list<NodeId>& children);
    ~AllChildrenWaitingOrFinishedCondition();
    std::string toString() const;
    double recalculate();
    void addChild(const NodeId& node);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return BOOLEAN; }

  protected:
    friend class WaitingOrFinishedListener;
    void incrementCount(const ExpressionId& expr);
    void decrementCount(const ExpressionId& expr);
  private:

    class WaitingOrFinishedListener : public ExpressionListener {
    public:
      WaitingOrFinishedListener(AllChildrenWaitingOrFinishedCondition& cond);
      void notifyValueChanged(const ExpressionId& expression);
    protected:
    private:
      AllChildrenWaitingOrFinishedCondition& m_cond;
    };

    bool checkValue(const double val);
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    WaitingOrFinishedListener m_listener;
    unsigned int m_total;
    unsigned int m_count;
    bool m_constructed;
    std::list<NodeId> m_children;
    std::map<ExpressionId, double> m_lastValues;
  };

  //used for EQInternal and NEQInternal
  class InternalCondition : public Calculable {
  public:
    InternalCondition(const PlexilExprId& expr);
    InternalCondition(const PlexilExprId& expr, const NodeConnectorId& node);
    ~InternalCondition();
    double recalculate();
    std::string toString() const;

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

  class TimepointVariable : public ConstVariableWrapper {
  public:
    TimepointVariable(const PlexilExprId& expr);
    TimepointVariable(const PlexilExprId& expr, const NodeConnectorId& node);

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return TIME; }
    
  };

}

#endif
