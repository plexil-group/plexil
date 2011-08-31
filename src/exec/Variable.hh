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

#ifndef VARIABLE_HH
#define VARIABLE_HH

#include "Expression.hh"

namespace PLEXIL
{

  /**
   * An abstract base class representing anything that can be on the left side
   * of an assignment, including but not limited to actual variables, 
   * variable aliases, array elements, etc.
   */

  class Variable :
	public virtual Expression
  {
  public:
    /**
     * @brief Default constructor.
     */
    Variable();

    /**
     * @brief Constructor.
	 * @param node NodeConnectorId to the owning object.
     */
    Variable(const NodeConnectorId& node);

    /**
     * @brief Destructor.
     */
    virtual ~Variable();
    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     * @param value The new value for this variable.
     */
    virtual bool checkValue(const double value) = 0;

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    virtual void reset() = 0;

    /**
     * @brief Sets the value of this variable.  Will throw an error if the variable was
     *        constructed with isConst == true.
     * @param value The new value for this variable.
     */
    virtual void setValue(const double value) = 0;

    /**
     * @brief Gets the const-ness of this variable.
     * @return True if this variable is const, false otherwise.
     */
    virtual bool isConst() const = 0;

    /**
     * @brief Get the node that owns this expression.
     * @return The NodeId of the parent node; may be noId.
	 * @note Used by LuvFormat::formatAssignment().  
     */
    const NodeId& getNode() const;

    const VariableId& getId() const {return m_evid;}

	/**
	 * @brief Get the real variable for which this may be a proxy.
	 * @return The VariableId of the base variable
	 * @note Used by the assignment node conflict resolution logic.
	 */
	virtual const VariableId& getBaseVariable() const = 0;

  protected:

  private:
    const NodeConnectorId m_nodeConnector; /*<! Tracks the node that owns this expression */
	VariableId m_evid;
  };

  /**
   * An abstract base class representing a variable-like object
   * which stores an array.
   */
  class EssentialArrayVariable :
	public virtual Variable
  {
  public:
	EssentialArrayVariable();

	EssentialArrayVariable(const NodeConnectorId& node);

	virtual ~EssentialArrayVariable();

	/**
	 * @brief Get the maximum size of this array.
	 */
    virtual unsigned long maxSize() const = 0;

	/**
	 * @brief Get the element at the given index.
	 */
    virtual double lookupValue(unsigned long index) const = 0;

    /**
     * @brief Set one element of this array from the given value.
     * @note Value must be an array or UNKNOWN.
     * @note Index must be less than maximum length
     */
    virtual void setElementValue(unsigned index, const double value) = 0;

    /**
     * @brief Retrieve the element type of this array.
     * @return The element type of this array.
     */
    virtual PlexilType getElementType() const = 0;

    /**
     * @brief Check to make sure an element value is appropriate for this array.
     */
    virtual bool checkElementValue(const double val) = 0;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     */
    virtual PlexilType getValueType() const { return ARRAY; }

    /**
     * @brief Notify this array that an element's value has changed.
     * @param elt The changed element.
     */
    virtual void handleElementChanged(const ExpressionId& elt) = 0; 

	const Id<EssentialArrayVariable>& getId() const { return m_eavid; }

  protected:

  private:
	// deliberately unimplemented
	EssentialArrayVariable(const EssentialArrayVariable&);
	EssentialArrayVariable& operator=(const EssentialArrayVariable&);

	Id<EssentialArrayVariable> m_eavid;
  };

  typedef Id<EssentialArrayVariable> EssentialArrayVariableId;

  /**
   *  An abstract base class representing any "variable" expression that depends upon another variable,
   *  including but not limited to array elements, aliases, etc.
   */
  class DerivedVariable :
	public virtual Variable
  {
  public:
	/**
	 * @brief Default constructor.
	 */
    DerivedVariable();

	/**
	 * @brief Constructor.
	 */
    DerivedVariable(const NodeConnectorId& node);

	/**
	 * @brief Destructor.
	 */
    virtual ~DerivedVariable();
	
    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
	 * @note This is to let the derived variable know when the objects 
	 *       from which it is derived have changed.
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
  class DerivedVariableListener :
	public ExpressionListener 
  {
  public:

    /**
     * @brief Constructor.
     * @param exp The expression to be notified of any changes.
     */
    DerivedVariableListener(const ExpressionId& exp)
	  : ExpressionListener(),
		m_exp(exp)
	{}

    /**
     * @brief Notifies the destination expression of a value change.
     * @param exp The expression which has changed.
     */
    void notifyValueChanged(const ExpressionId& exp)
	{
	  // prevent infinite loop
	  if (exp != (ExpressionId) m_exp)
        m_exp->handleChange(exp);
	}

  private:

    // deliberately unimplemented
    DerivedVariableListener();
    DerivedVariableListener(const DerivedVariableListener&);
    DerivedVariableListener& operator=(const DerivedVariableListener&);

    ExpressionId m_exp; /*<! The destination expression for notifications. */
  };

  /**
   * An abstract base class representing a variable with a single value.
   * Derived classes are specialized by value type.
   */
  class VariableImpl : 
	public virtual Variable
  {
  public:

    /**
     * @brief Constructor.  Creates a variable that is initially UNKNOWN.
     * @param isConst True if this variable should have a constant value, false otherwise.
     */
    VariableImpl(const bool isConst = false);

    /**
     * @brief Constructor.  Creates a variable with a given value.
     * @param value The initial value of the variable.
     * @param isConst True if this variable should have a constant value, false otherwise.
     */
    VariableImpl(const double value, const bool isConst = false);

    /**
     * @brief Constructor.  Creates a variable from XML.
     * @param expr The PlexilExprId for this variable.
     * @param node A connection back to the node that created this variable.
     * @param isConst True if this variable should have a constant value, false otherwise.
     */
    VariableImpl(const PlexilExprId& expr, const NodeConnectorId& node,
				 const bool isConst = false);

    virtual ~VariableImpl();

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
    void print(std::ostream& s) const;

	/**
	 * @brief Print the variable's value to the given stream.
	 * @param s The output stream.
	 */
	virtual void printValue(std::ostream& s) const;

    /**
     * @brief Get a string representation of the value of this Variable.
     * @return The string representation.
	 * @note This method always uses the stored value whether or not the variable is active,
	 *       unlike the base class method.
     */
    virtual std::string valueString() const;

    /**
     * @brief Set the value of this expression back to the initial value with which it was
     *        created.
     */
    virtual void reset();

    /**
     * @brief Sets the value of this variable.  Will throw an error if the variable was
     *        constructed with isConst == true.
     * @param value The new value for this variable.
     */
    virtual void setValue(const double value);

    /**
     * @brief Gets the const-ness of this variable.
     * @return True if this variable is const, false otherwise.
     */
    bool isConst() const {return m_isConst;}

    /**
     * @brief Gets the initial value of this variable.
     * @return The initial value of this variable.
     */
    double initialValue() const {return m_initialValue;}

	/**
	 * @brief Set the name of this variable.
	 */
	void setName(const std::string& name)
	{
	  m_name = name;
	}

    /**
     * @brief Get the name of this variable, as declared in the node that owns it.
     */
    const std::string& getName() const
    {
      return m_name;
    }

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param id The Id of the listener to notify.
	 * @note Overrides method on Expression base class.
     */
    virtual void addListener(ExpressionListenerId id);

    /**
     * @brief Remove a listener from this Expression.
     * @param id The Id of the listener to remove.
	 * @note Overrides method on Expression base class.
     */
    virtual void removeListener(ExpressionListenerId id);

	/**
	 * @brief Get the real variable for which this may be a proxy.
	 * @return The VariableId of the base variable
	 * @note Used by the assignment node conflict resolution logic.
	 */
	virtual const VariableId& getBaseVariable() const
	{
	  return Variable::getId(); 
	}

  protected:
    /**
     * @brief Handle additional behaviors for the reset() call.
     */
    virtual void handleReset() {}

    /**
     * @brief Ensure that, if a variable is constant, it is never really deactivated
     */
    virtual void handleDeactivate(const bool changed);

    /**
     * @brief Performs common initialization tasks (number parsing, translation from the
     *        various forms of infinity, etc.) for numeric expressions.
     * @param val The XML representing the value of this variable.
     */
    void commonNumericInit(PlexilValue* val);

    //
    // Private member variables
    //

    bool m_isConst; /*<! Flag indicating the const-ness of this variable */
    double m_initialValue; /*<! The initial value of the expression */
    const NodeId m_node; /*<! The node that owns this variable */
    std::string m_name; /*<! The name under which this variable was declared */
  };

  /**
   * Class to provide a constant interface over some other variable.
   * Currently used only in TimepointVariable.
   */
  class ConstVariableWrapper : public DerivedVariable {
  public:
    ConstVariableWrapper(const VariableId& exp);
    ConstVariableWrapper();
    virtual ~ConstVariableWrapper();
    virtual double getValue() const;
	virtual PlexilType getValueType() const;
    virtual void setValue(const double value);
    virtual std::string valueString() const;
	virtual bool isConst() const { return true; }
	virtual void reset();

	/**
	 * @brief Get the real variable for which this may be a proxy.
	 * @return The VariableId of the base variable
	 * @note Used by the assignment node conflict resolution logic.
	 */
	virtual const VariableId& getBaseVariable() const
	{
	  return m_exp;
	}

  protected:
    virtual void handleChange(const ExpressionId& expr);
    void setWrapped(const VariableId& expr);

  private:
	// deliberately unimplemented
	ConstVariableWrapper(const ConstVariableWrapper&);
	ConstVariableWrapper& operator=(const ConstVariableWrapper&);

    virtual bool checkValue(const double val);
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    VariableId m_exp;
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
				  VariableId original,
				  const bool isConst = false);

	virtual ~AliasVariable();

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
    void print(std::ostream& s) const;

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
	 * @brief Get the real variable for which this may be a proxy.
	 * @return The VariableId of the base variable
	 * @note Used by the assignment node conflict resolution logic.
	 */
	virtual const VariableId& getBaseVariable() const
	{
	  return m_originalVariable->getBaseVariable();
	}

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

	VariableId m_originalVariable;
	DerivedVariableListener m_listener;
	const std::string& m_name;
	bool m_isConst;
  };

}

#endif // VARIABLE_HH
