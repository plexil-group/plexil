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

#include "Variable.hh"
#include "Debug.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "StoredArray.hh"

namespace PLEXIL
{

  /**
   * @brief Constructor.
   */
  Variable::Variable()
	: Expression(),
	  m_nodeConnector(NodeConnectorId::noId()),
	  m_evid(this, Expression::getId())
  {
  }

  Variable::Variable(const NodeConnectorId& node)
	: Expression(),
	  m_nodeConnector(node),
	  m_evid(this, Expression::getId())
  {}

  /**
   * @brief Destructor.
   */
  Variable::~Variable() 
  {
	m_evid.removeDerived(Expression::getId());
  }

  /**
   * @brief Get the node that owns this expression.
   * @return The NodeId of the parent node; may be noId.
   * @note Used by LuvFormat::formatAssignment().  
   */
  const NodeId& Variable::getNode() const
  { 
	if (m_nodeConnector.isNoId())
	  return NodeId::noId();
	else
	  return m_nodeConnector->getNode();
  }

  EssentialArrayVariable::EssentialArrayVariable()
	: Variable(),
	  m_eavid(this, Variable::getId())
  {
  }

  EssentialArrayVariable::EssentialArrayVariable(const NodeConnectorId& node)
	: Variable(node),
	  m_eavid(this, Variable::getId())
  {
  }

  EssentialArrayVariable::~EssentialArrayVariable()
  {
	m_eavid.removeDerived(Variable::getId());
  }

  /**
   * @brief Default constructor.
   */
  DerivedVariable::DerivedVariable()
	: Variable()
	{} 

  /**
   * @brief Constructor.
   */
  DerivedVariable::DerivedVariable(const NodeConnectorId& node)
	: Variable(node)
  {}


  /**
   * @brief Destructor.
   */
  DerivedVariable::~DerivedVariable()
  {
  }


  // Used in Expression::UNKNOWN_EXP(), and by various derived constructors

  VariableImpl::VariableImpl(const bool isConst)
    : Variable(), m_isConst(isConst), m_initialValue(UNKNOWN()), m_name("anonymous") 
  {
    if(this->isConst())
      m_activeCount++;
  }

  // Used only in Lookup::Lookup(const StateCacheId&, const LabelStr&, std::list<double>&)

  VariableImpl::VariableImpl(const double value, const bool isConst)
    : Variable(), m_isConst(isConst), m_initialValue(value), m_name("anonymous") 
  {
    m_value = m_initialValue;
    if(this->isConst())
      m_activeCount++;
  }

  //
  // ExpressionFactory constructor
  // uses PlexilVar prototype
  //

  VariableImpl::VariableImpl(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst)
    : Variable(node), m_isConst(isConst), m_name(expr->name())
  {
    check_error(Id<PlexilVar>::convertable(expr) || Id<PlexilValue>::convertable(expr));
  }

  VariableImpl::~VariableImpl()
  {
  }

  void VariableImpl::print(std::ostream& s) const 
  {
	s << m_name << " ";
	Expression::print(s);
  }

  /**
   * @brief Print the expression's value to the given stream.
   * @param s The output stream.
   */
  void VariableImpl::printValue(std::ostream& s) const
  {
	Expression::formatValue(s, m_value);
  }

  /**
   * @brief Get a string representation of the value of this Variable.
   * @return The string representation.
   * @note This method always uses the stored value whether or not the variable is active,
   *       unlike the base class method.
   */
  std::string VariableImpl::valueString() const
  {
	return valueToString(m_value);
  }

  void VariableImpl::reset() {
    if(!isConst()) {
      internalSetValue(m_initialValue);
      handleReset();
    }
  }

  /**
   * @brief Ensure that, if a variable is constant, it is never really deactivated
   */
  void VariableImpl::handleDeactivate(const bool changed) {
    if(this->isConst() && changed)
      m_activeCount++;
  }

  void VariableImpl::setValue(const double value) {
    checkError(!isConst(),
			   "Attempted to assign value " << Expression::valueToString(value)
			   << " to read-only variable " << toString());
    internalSetValue(value);
  }

  void VariableImpl::commonNumericInit(const PlexilValue* val) 
  {
    if (val == NULL)
      m_initialValue = m_value = UNKNOWN();
    else if(val->value() == "INF" || val->value() == "Inf" ||
	    val->value() == "inf") {
      if(val->type() == INTEGER)
	m_initialValue = m_value = PLUS_INFINITY;
      else
	m_initialValue = m_value = REAL_PLUS_INFINITY;
    }
    else if(val->value() == "-INF" || val->value() == "-Inf" ||
	    val->value() == "-inf") {
      if(val->type() == INTEGER)
	m_initialValue = m_value == MINUS_INFINITY;
      else
	m_initialValue = m_value = REAL_MINUS_INFINITY;
    }
    else if(val->type() == BOOLEAN && val->value() == "true")
	m_initialValue = m_value = true;
    else if(val->type() == BOOLEAN && val->value() == "false")
      m_initialValue = m_value = false;
    else {
      std::stringstream str;
      str << val->value();
      double value;
      str >> value;
      m_initialValue = m_value = value;
      checkError(checkValue(m_value), 
				 "Invalid " << PlexilParser::valueTypeString(val->type()) << " '" << Expression::valueToString(m_value) << "'");
    }
  }

  /**
   * @brief Add a listener for changes to this Expression's value.
   * @param id The Id of the listener to notify.
   * @note Overrides method on Expression base class.
   * @note This is an optimization for heavily used constants, which by definition
   * will never change value, thus don't need to propagate changes.
   */
  void VariableImpl::addListener(ExpressionListenerId id)
  {
	if (!m_isConst)
	  Expression::addListener(id);
  }

  /**
   * @brief Remove a listener from this Expression.
   * @param id The Id of the listener to remove.
   * @note Overrides method on Expression base class.
   * @note This is an optimization for heavily used constants, which by definition
   * will never change value, thus don't need to propagate changes.
   */
  void VariableImpl::removeListener(ExpressionListenerId id)
  {
	if (!m_isConst)
	  Expression::removeListener(id);
  }

  ConstVariableWrapper::ConstVariableWrapper(const VariableId& exp)
    : DerivedVariable(), m_exp(exp), m_listener(this) 
  {
    m_exp->addListener(m_listener.getId());
  }

  ConstVariableWrapper::ConstVariableWrapper()
    : DerivedVariable(), m_listener(this) {}

  ConstVariableWrapper::~ConstVariableWrapper() {
    checkError(m_exp.isValid(), "Got to destructor without a valid wrapped variable.");
    m_exp->removeListener(m_listener.getId());
  }

  void ConstVariableWrapper::setWrapped(const VariableId& expr) {
    checkError(m_exp.isInvalid(),
	       "Attmpted to set wrapped variable to " << expr->toString() <<
	       " when already wrapping " << m_exp->toString());
    checkError(expr.isValid(),
	       "Attempted to set an invalid wrapped variable.");
    m_exp = expr;
    m_exp->addListener(m_listener.getId());
  }

  double ConstVariableWrapper::getValue() const 
  {
    checkError(m_exp.isValid(), "Got to getValue without a valid wrapped variable.");
    return m_exp->getValue();
  }

  PlexilType ConstVariableWrapper::getValueType() const 
  {
    checkError(m_exp.isValid(), "Got to getValueType without a valid wrapped variable.");
    return m_exp->getValueType();
  }

  void ConstVariableWrapper::setValue(const double /* value */) 
  {
    checkError(ALWAYS_FAIL,
			   "Attempted to set the value of a const expression.");
  }

  std::string ConstVariableWrapper::valueString() const 
  {
    checkError(m_exp.isValid(), "Got to valueString without a valid wrapped variable.");
    std::ostringstream str;
    str << "const " << m_exp->valueString();
    return str.str();
  }

  void ConstVariableWrapper::reset()
  {
    checkError(ALWAYS_FAIL,
			   "Attempted to reset a const expression.");
  }

  bool ConstVariableWrapper::checkValue(const double /* val */) 
  {
    checkError(ALWAYS_FAIL,
			   "Attempted to check an incoming value for const expresssion " << toString());
    return false;
  }

  void ConstVariableWrapper::handleActivate(const bool /* changed */) 
  {
    checkError(m_exp.isValid(), "Got to handleActivate without a valid wrapped variable.");
    m_listener.activate();
    m_exp->activate();
  }

  void ConstVariableWrapper::handleDeactivate(const bool /* changed */) 
  {
    checkError(m_exp.isValid(), "Got to handleDeactivate without a valid wrapped variable.");
    m_listener.deactivate();
    m_exp->deactivate();
  }

  void ConstVariableWrapper::handleChange(const ExpressionId& /* expr */) 
  {
    checkError(m_exp.isValid(), "Got to handleChange without a valid wrapped variable.");
    publishChange();
  }

  /**
   * @brief Constructor. Creates a variable that indirects to another variable.
   * @param name The name of this variable in the node that constructed the alias.
   * @param nodeConnector The node connector of the node which owns this alias.
   * @param original The original variable for this alias.
   * @param isConst True if assignments to the alias are forbidden.
   */
  AliasVariable::AliasVariable(const std::string& name, 
							   const NodeConnectorId& nodeConnector,
							   VariableId original,
							   const bool isConst)
	: DerivedVariable(nodeConnector),
	  m_originalVariable(original),
	  m_listener(getId()),
	  m_name(name),
	  m_isConst(isConst)
  {
	// Check original, node for validity
	assertTrue(original.isValid(),
			   "Invalid variable ID passed to AliasVariable constructor");
	assertTrue(nodeConnector.isValid(),
			   "Invalid node connector ID passed to AliasVariable constructor");
	m_originalVariable->addListener(m_listener.getId());
	m_value = m_originalVariable->getValue();
  }

  AliasVariable::~AliasVariable()
  {
	assertTrue(m_originalVariable.isValid(),
			   "Original variable ID invalid in AliasVariable destructor");
	m_originalVariable->removeListener(m_listener.getId());
  }

  /**
   * @brief Get a string representation of this Expression.
   * @return The string representation.
   */
  void AliasVariable::print(std::ostream& s) const
  {
	Expression::print(s);
	s << "AliasVariable " << m_name
	  << ", aliased to " << *m_originalVariable
	  << ")";
  }

  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void AliasVariable::reset()
  { 
	// *** FIXME: should this do anything at all??
	// m_originalVariable->reset(); 
  }

  /**
   * @brief Retrieve the value type of this Expression.
   * @return The value type of this Expression.
   * @note Delegates to original.
   */
  PlexilType AliasVariable::getValueType() const
  {
	return m_originalVariable->getValueType();
  }

  bool AliasVariable::checkValue(const double val)
  {
	return m_originalVariable->checkValue(val);
  }	

  /**
   * @brief Sets the value of this variable.  Will throw an error if the variable was
   *        constructed with isConst == true.
   * @param value The new value for this variable.
   */
  void AliasVariable::setValue(const double value)
  {
    checkError(!isConst(),
			   "Attempted to assign value " << Expression::valueToString(value)
			   << " to read-only alias variable " << toString());
	m_originalVariable->setValue(value);
  }

  void AliasVariable::handleChange(const ExpressionId& exp)
  {
	if (exp == m_originalVariable) {
	  // propagate value from original
	  internalSetValue(m_originalVariable->getValue());
	}
  }

  void AliasVariable::handleActivate(const bool changed)
  {
	if (changed) {
	  m_originalVariable->activate();
	  // refresh value from original
	  internalSetValue(m_originalVariable->getValue());
	}
  }

  // *** FIXME: Inhibit deactivatation if const?
  void AliasVariable::handleDeactivate(const bool changed)
  {
	if (changed) {
	  m_originalVariable->deactivate();
	}
  }

  void AliasVariable::handleReset()
  {
  }

}
