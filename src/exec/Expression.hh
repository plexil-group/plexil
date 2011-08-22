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

#ifndef _H_Expression
#define _H_Expression

/**
 * @file Expression.hh
 * @author Michael Iatauro
 * @date Mon Feb 13 13:52 2006
 * @brief
 * @ingroup Exec
 */

#include "ExecDefs.hh"
#include "LabelStr.hh"
#include "PlexilPlan.hh"
#include "StoredArray.hh"
#include "Utils.hh"

#include <list>
#include <map>
#include <set>

// Common constants used only in expressions

// The most positive legal "integer" value.
#define PLUS_INFINITY (INT32_MAX)

// The most negative legal "integer" value.
#define MINUS_INFINITY (INT32_MIN)

#define REAL_PLUS_INFINITY (PLEXIL::g_maxReal())

#define REAL_MINUS_INFINITY (-PLEXIL::g_maxReal())

/**
 * @def EPSILON
 * Used when computing differences and comparing real numbers:
 * smallest recognized increment.
 */
#define EPSILON (PLEXIL::g_epsilon())

namespace PLEXIL {
  /**
   * @brief A listener for the notification graph.
   */
  class ExpressionListener {
  public:

    /**
     * @brief Constructor for listener for events on Expressions.  The intention here is to
     *        unify the structure of things that have values which may or may not require
     *        computation.  The notification system allows for cheap re-computation of the
     *        values of complex expressions.  Note that there is a potential problem here.
     *        I've defined the UNKNOWN value as __DBL_MAX__, but that may be a valid, useful
     *        value from the outside world.  Something that we may want to do is create a
     *        Value class and have a specific instance that represents UNKNOWN.
     */
    ExpressionListener();
    virtual ~ExpressionListener();
    const ExpressionListenerId& getId() const {return m_id;}

    /**
     * @brief Virtual function for notification that an expression's value has changed.
     * @param expression The expression whose value has changed.
     */
    virtual void notifyValueChanged(const ExpressionId& expression) = 0;

    /**
     * @brief Parts of the notification graph may be inactive, which means that value change
     *        notifications won't propagate through them.  The isActive method controls this.
     * @return true if this listener is active, false if it is not.
     */
    bool isActive() const {return m_activeCount > 0;}

    /**
     * @brief Make this listener active, meaning notifications will be published through it.
     */
    void activate();

    /**
     * @brief Make this listener inactive, preventing notifications through it.
     */
    void deactivate();
  protected:
  private:
    ExpressionListenerId m_id; /*!< The Id for this listener */
    unsigned int m_activeCount;
  };


  /**
   * @brief The base class for all Expressions, immediate or otherwise.  Changes to values
   *        can cause a notification to interested parties of the change, which may cause
   *        recalculation of some other Expression's value.
   */
  class Expression {
  public:

    //redirects for old usage.
    DECLARE_STATIC_CLASS_CONST(double, UNKNOWN, PLEXIL::UNKNOWN());

    static ExpressionId& UNKNOWN_EXP();

    /**
     * @brief Generic constructor for Expressions.  Does nothing but initialze base data
     *        structures.  Expressions are inactive by default and must be activated.
     */
    Expression();

    /**
     * @brief Destructor.  Cleans up listeners on other expressions and any subexpressions.
     */
    virtual ~Expression();

    const ExpressionId& getId() const {return m_id;}

    /**
     * @brief Retrieve the value of this Expression.  This may cause recalculation, lookup of
     *        a value, or something similar.  This is a double because it is easy to
     *        represent other values this way (integers cast freely, reals are doubles, and
     *        the LabelStr facility gives us strings as doubles).
     * @return The value of this Expression.
     */
    virtual double getValue() const;

    /**
     * @brief Retrieve the value type of this Expression.
     * @return The value type of this Expression.
     * @note The default method returns UNKNOWN.
     */
    virtual PlexilType getValueType() const { return PLEXIL::UNKNOWN_TYPE; }

    /**
     * @brief Set the value for this expression.  This may cause notifications to fire, which
     *        may in turn cause other Expressions to change value.
     * @param val The new value for this expression.
     */
    virtual void setValue(const double val);

    /**
     * @brief Parts of the notification graph may be inactive, which mans that value change
     *        notifications won't propagate through them.  The isActive method controls this.
     * @return true if this Expression is active, false if it is not.
     */
    bool isActive() const {return m_activeCount > 0;}

    /**
     * @brief Returns true if this expression is "locked": its value will not change until
     *        unlocked.
     * @return true if this expression is locked, false otherwise
     * @see lock(), unlock()
     */

    bool isLocked() const {return m_lock;}

    /**
     * @brief Make this expression active.  It will publish value changes and it will accept
     *        incoming change notifications.
     */

    void activate();

    /**
     * @brief Make this listener inactive.  It will not publish value changes, nor will it
     *        accept incoming change notifications.
     */
    void deactivate();

    /**
     * @brief Add a listener for changes to this Expression's value.
     * @param id The Id of the listener to notify.
     */
    virtual void addListener(ExpressionListenerId id);

    /**
     * @brief Remove a listener from this Expression.
     * @param id The Id of the listener to remove.
     */
    virtual void removeListener(ExpressionListenerId id);

    /**
     * @brief Get a string representation of this Expression.
     * @return The string representation.
     */
    virtual std::string toString() const;

    /**
     * @brief Get a string representation of the value of this Expression.
     * @return The string representation.
     */
    virtual std::string valueString() const;

    /**
     * @brief Get a string representation of the double value.
     * @return The string representation.
     */
	static std::string valueToString(const double value);

    /**
     * @brief Lock this expression so its value doesn't change.  Changes are stored until the
     *        unlock call, at which point the expression gets updated and notifications happen
     *        normally.
     */
    void lock();

    /**
     * @brief Unlock this expression so its value can change.  If it had a change during the
     *        time in which it was locked, it gets updated to that value and the change is
     *        propagated.
     */
    void unlock();

    
    /**
     * @brief Check to make sure a value is appropriate for this expression.
     */
    virtual bool checkValue(const double /* val */) {return false;}

    /**
     * @brief Disregard to the expression value that has been cached. 
     */
    void ignoreCachedValue() {m_ignoreCachedValue = true;}

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     * @note The default method does nothing.
     */
    virtual void handleChange(const ExpressionId& /* exp */) {}

  protected:

    /**
     * @brief Internal method for handling the setting of a value.  Used to allow overriding
     *        of setValue to do different things.  Will cause notification of change if there
     *        is a change is this expression isn't locked.
     * @param value The value being set.
     */
    void internalSetValue(const double value);

    /**
     * @brief Notify listeners that the value of this expression has changed.
     */
    virtual void publishChange();

    /**
     * @brief Handle the locking of the expression.
     */
    virtual void handleLock() {}

    /**
     * @brief Handle the unlocking of the expression.
     */
    virtual void handleUnlock() {}

    /**
     * @brief Handle the activation of the expression.
     * @param changed True if the call to activate actually caused a change from inactive to
     *                active.
     */
    virtual void handleActivate(const bool /* changed */) {}

    /**
     * @brief Handle the deactivation of the expression
     * @param changed True if the call to deactivate actually caused a change from active to
     *                inactive.
     */
    virtual void handleDeactivate(const bool /* changed */) {}

    ExpressionId m_id; /*<! The Id for this Expression */
    unsigned int m_activeCount;
    double m_value; /*<! The value of this expression*/
    double m_savedValue; /*<! The latest value computed for this expression while it was locked. */
    bool m_dirty; /*<! Marks whether or not this expression needs re-calculation.*/
    bool m_lock; /*<! The lock for this expression */
    bool m_ignoreCachedValue; /*<! Disregard the m_value that has been cached when deciding to publissh a change*/
    std::list<ExpressionListenerId> m_outgoingListeners; /*<! For outgoing message notifications (this expression's value has changed) */
  };

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

	~EssentialArrayVariable();

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
	 * @brief Constructor.
	 */
    DerivedVariable();

    DerivedVariable(const NodeConnectorId& node);

	/**
	 * @brief Destructor.
	 */
    ~DerivedVariable();
	
    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
	 * @note This is to let the derived variable know when the objects 
	 *       from which it is derived have changed.
     */
    virtual void handleChange(const ExpressionId& exp) = 0;

	const Id<DerivedVariable> getId() const { return m_derid; }
    
  protected:
    
  private:
    // deliberately unimplemented
    DerivedVariable(const DerivedVariable &);
    DerivedVariable & operator=(const DerivedVariable &);

	Id<DerivedVariable> m_derid;
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
    DerivedVariableListener(const Id<DerivedVariable>& exp)
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

    Id<DerivedVariable> m_exp; /*<! The destination expression for notifications. */
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
    std::string toString() const;

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
   *  A class for notifying expressions of changes in sub-expressions.
   */
  class SubexpressionListener : public ExpressionListener {
  public:

    /**
     * @brief Constructor.
     * @param exp The expression to be notified of any changes.
     */
    SubexpressionListener(const ExpressionId& exp);

    /**
     * @brief Notifies the destination expression of a value change.
     * @param exp The expression which has changed.
     */
    void notifyValueChanged(const ExpressionId& exp);

  private:
    Expression* m_exp; /*<! The destination expression for notifications. */
  };


  /**
   * A class representing expressions that require calculation.
   */
  class Calculable : public Expression {
  public:

    /**
     * @brief Constructor.
     */
    Calculable();

    /**
     * @brief Constructor
     * @param expr The PlexilExpr for this expression.
     * @param node The scope in which this expression is evaluated.
     */
    Calculable(const PlexilExprId& expr, const NodeConnectorId& node);

    virtual ~Calculable();

    /**
     * @brief By default, Calculables can't have their value set this way.  However,
     *        it should be possible for subclasses to override this behavior.
     */
    virtual void setValue(const double value);

    /**
     * @brief Re-calculate the value of this expression.
     */
    virtual double recalculate() {return m_value;}

    /**
     * @brief Notify this expression that a subexpression's value has changed.
     * @param exp The changed subexpression.
     */
    virtual void handleChange(const ExpressionId& exp);

  protected:
    /**
     * @brief A utility function for accessing a variable in the current scope or creating
     *        variable for a literal value.
     * @param expr The PlexilExprId for the variable or value.
     * @param node The node which delimits the scope of the variable search.
     * @param del Set to true if the destination should be garbage collected along with this
     *            expression (i.e. if a new variable had to be allocated.).
     */
    virtual ExpressionId getSubexpression(const PlexilExprId& expr, const NodeConnectorId& node,
					  bool& del);

    /**
     * @brief Handles the activation of this expression, including activation of
     *        subexpressions.  By default, when the expression goes from inactive to
     *        active, it re-calculates its value.
     * @param changed True if this expression has gone from inactive to active.
     */
    virtual void handleActivate(const bool changed);

    /**
     * @brief Handles the deactivation of this expression, including the deactivation of
     *        subexpressions.
     * @param changed True if this expression has gone from inactive to active.
     */
    virtual void handleDeactivate(const bool changed);

    /**
     * @brief A method for subclasses to handle the change of a subexpression's value.
     * @param exp The subexpression whose value has changed.
     * @note The default method is a no-op.
     * @note As of 25 Nov 2008, there appear to be no other implementations of this method.
     */
    virtual void handleSubexpressionChange(const ExpressionId& /* exp */) {}

    void addSubexpression(const ExpressionId& exp, const bool garbage);

    bool containsSubexpression(const ExpressionId& expr);

    void removeSubexpression(const ExpressionId& exp);

    SubexpressionListener m_listener; /*<! For incoming message notifications (other expressions' values have changed) */
    //std::list<ExpressionListenerId> m_incomingListeners;
    ExpressionVector m_subexpressions; /*<! The subexpressions.*/
    std::set<ExpressionId> m_garbage;
  };


  /**
   * Class to provide a constant interface over some other variable.
   * Used only in TimepointVariable.
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

  protected:
    void handleChange(const ExpressionId& expr);
    void setWrapped(const VariableId& expr);

  private:
	// deliberately unimplemented
	ConstVariableWrapper(const ConstVariableWrapper&);
	ConstVariableWrapper& operator=(const ConstVariableWrapper&);

    virtual bool checkValue(const double val);
    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);

    ExpressionId m_exp;
	DerivedVariableListener m_listener;
  };

  class WrapperListener;

  /**
   * Class to provide an interface that doesn't propagate activation/deactivation messages.
   * Used in Node::createConditions().
   */
  class TransparentWrapper : public Expression {
  public:

    TransparentWrapper(const ExpressionId& exp, const NodeConnectorId& node);

	// This variant used only in unit tests
    TransparentWrapper(const ExpressionId& exp);

    virtual ~TransparentWrapper();
    void setValue(const double value);
    std::string toString() const;
    std::string valueString() const;
    bool checkValue(const double value);

  private:

	// Deliberately not implemented
    TransparentWrapper();
	TransparentWrapper& operator=(const TransparentWrapper&);

	void commonInit(const ExpressionId& exp);

    void handleActivate(const bool changed);
    void handleDeactivate(const bool changed);
    void handleChange(const ExpressionId& expression);

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

  /**
   * @brief Factory class for Expressions.  This allows you to write, for instance \<AND\>
	    in XML and have the correct Expression instantiated.
   */
  class ExpressionFactory {
  public:

    /**
     * @brief Creates a new Expression instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @return The Id for the new Expression.  May not be unique.
     */

    static ExpressionId createInstance(const LabelStr& name, const PlexilExprId& expr,
				       const NodeConnectorId& node = NodeConnectorId::noId());

    /**
     * @brief Creates a new Expression instance with the type associated with the name and
     *        the given configuration XML.
     * @param name The registered name for the factory.
     * @param expr The PlexilExprId to be passed to the Expression constructor.
     * @param node Node for name lookup.
     * @param wasCreated Reference to a boolean variable;
     *                   variable will be set to true if new object created, false otherwise.
     * @return The Id for the new Expression.  If wasCreated is set to false, is not unique.
     */

    static ExpressionId createInstance(const LabelStr& name,
                                       const PlexilExprId& expr,
				       const NodeConnectorId& node,
                                       bool& wasCreated);

    /**
     * @brief Deallocate all factories
     */
    static void purge();

    const LabelStr& getName() const {return m_name;}

    void lock();

    void unlock();

  protected:
    virtual ~ExpressionFactory(){}

    /**
     * @brief Registers an ExpressionFactory with the specific name.
     * @param name The name by which the Expression shall be known.
     * @param factory The ExpressionFactory instance.
     */
    static void registerFactory(const LabelStr& name, ExpressionFactory* factory);

    static void registerFinder(const LabelStr& name, ExpressionFactory* factory);

    virtual ExpressionId create(const PlexilExprId& expr,
				const NodeConnectorId& node = NodeConnectorId::noId()) const = 0;
    ExpressionFactory(const LabelStr& name)
      : m_name(name) {registerFactory(m_name, this);}

  private:
    /**
     * @brief The map from names (LabelStr/double) to ConcreteExpressionFactory instances.
     * This pattern of wrapping static data in a static method is to ensure proper loading
     * when used as a shared library.
     */
    static std::map<double, ExpressionFactory*>& factoryMap();

    const LabelStr m_name; /*!< Name used for lookup */
  };

  /**
   * @brief Concrete factory class, templated for each Expression type.
   */
  template<class FactoryType>
  class ConcreteExpressionFactory : public ExpressionFactory {
  public:
    ConcreteExpressionFactory(const LabelStr& name) : ExpressionFactory(name) {}
  private:
    /**
     * @brief Instantiates a new Expression of the appropriate type.
     * @param expr The PlexilExprId for the instantiated Expression.
     * @param node
     * @return The Id for the new Expression.
     */

    ExpressionId create(const PlexilExprId& expr,
			const NodeConnectorId& node = NodeConnectorId::noId()) const
    {return (new FactoryType(expr, node))->getId();}
  };


  /**
   * @brief Variant of above for constant values.
   */
  template<class FactoryType>
  class ConstantExpressionFactory : public ExpressionFactory {
  public:
    ConstantExpressionFactory(const LabelStr& name) : ExpressionFactory(name) {}
  private:
    /**
     * @brief Instantiates a new Expression of the appropriate type.
     * @param expr The PlexilExprId for the instantiated Expression.
     * @param node
     * @return The Id for the new Expression.
     */

    ExpressionId create(const PlexilExprId& expr,
			const NodeConnectorId& node = NodeConnectorId::noId()) const
    {return (new FactoryType(expr, node, true))->getId();}
  };

}

#define REGISTER_EXPRESSION(CLASS,NAME) {new PLEXIL::ConcreteExpressionFactory<CLASS>(#NAME);}
#define REGISTER_CONSTANT_EXPRESSION(CLASS,NAME) {new PLEXIL::ConstantExpressionFactory<CLASS>(#NAME);}

#endif
