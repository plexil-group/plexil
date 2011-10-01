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
#include "ExecConnector.hh"
#include "ExecListenerHub.hh"
#include "ExpressionFactory.hh"
#include "Node.hh"

namespace PLEXIL
{

  /**
   * @brief Constructor.
   */
  Variable::Variable()
	: Expression(),
	  m_evid(this, Expression::getId())
  {
  }

  /**
   * @brief Destructor.
   */
  Variable::~Variable() 
  {
	m_evid.removeDerived(Expression::getId());
  }

  //
  // VariableImpl
  //

  // Used in Expression::UNKNOWN_EXP(), and by various derived constructors

  VariableImpl::VariableImpl(const bool isConst)
    : Variable(),
	  m_isConst(isConst),
	  m_initialValue(UNKNOWN()),
	  m_node(NodeId::noId()),
	  m_name("anonymous") 
  {
    if(this->isConst())
      m_activeCount++;
  }

  // Used only in Lookup::Lookup(const StateCacheId&, const LabelStr&, std::list<double>&)

  VariableImpl::VariableImpl(const double value, const bool isConst)
    : Variable(),
	  m_isConst(isConst),
	  m_initialValue(value),
	  m_node(NodeId::noId()),
	  m_name("anonymous")
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
    : Variable(),
	  m_isConst(isConst),
	  m_node(node.isId() ? node->getNode() : NodeId::noId()),
	  m_name(expr->name())
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
	  ExecListenerHubId hub = getExecListenerHub();
	  if (hub.isId())
		hub->notifyOfAssignment(Expression::getId(), m_name, m_initialValue);
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
	ExecListenerHubId hub = getExecListenerHub();
	if (hub.isId()) 
	  hub->notifyOfAssignment(Expression::getId(), m_name, value);
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
      std::istringstream str(val->value());
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

  const ExecListenerHubId& VariableImpl::getExecListenerHub()
  {
	if (m_node.isId())
	  return m_node->getExecListenerHub();
	else {
	  static const ExecListenerHubId sl_noId;
	  return sl_noId;
	}
  }


  //
  // AliasVariable
  // 

  /**
   * @brief Constructor. Creates a variable that indirects to another variable.
   * @param name The name of this variable in the node that constructed the alias.
   * @param nodeConnector The node connector of the node which owns this alias.
   * @param original The original variable for this alias.
   * @param isConst True if assignments to the alias are forbidden.
   */
  AliasVariable::AliasVariable(const std::string& name,
							   const NodeConnectorId& nodeConnector,
							   const ExpressionId& original,
							   bool expIsGarbage,
							   bool isConst)
	: Variable(),
	  m_originalExpression(original),
	  m_listener(getId()),
	  m_node(nodeConnector.isId() ? nodeConnector->getNode() : NodeId::noId()),
	  m_name(name),
	  m_isGarbage(expIsGarbage),
	  m_isConst(isConst)
  {
	// Check original, node for validity
	assertTrue(original.isValid(),
			   "Invalid expression ID passed to AliasVariable constructor");
	assertTrue(nodeConnector.isValid(),
			   "Invalid node connector ID passed to AliasVariable constructor");
	m_originalExpression->addListener(m_listener.getId());
	m_value = m_originalExpression->getValue();
  }

  AliasVariable::~AliasVariable()
  {
	assertTrue(m_originalExpression.isValid(),
			   "Original expression ID invalid in AliasVariable destructor");
	m_originalExpression->removeListener(m_listener.getId());
	if (m_isGarbage)
	  delete (Expression*) m_originalExpression;
  }

  /**
   * @brief Get a string representation of this Expression.
   * @return The string representation.
   */
  void AliasVariable::print(std::ostream& s) const
  {
	s << m_name << " ";
	Expression::print(s);
	s << (isConst() ? "const " : "") 
	  << "AliasVariable for "
	  << *m_originalExpression
	  << ")";
  }

  /**
   * @brief Set the value of this expression back to the initial value with which it was
   *        created.
   */
  void AliasVariable::reset()
  { 
	// *** FIXME: should this do anything at all??
	// m_originalExpression->reset(); 
  }

  /**
   * @brief Retrieve the value type of this Expression.
   * @return The value type of this Expression.
   * @note Delegates to original.
   */
  PlexilType AliasVariable::getValueType() const
  {
	return m_originalExpression->getValueType();
  }

  bool AliasVariable::checkValue(const double val)
  {
	return m_originalExpression->checkValue(val);
  }	

  /**
   * @brief Sets the value of this variable.  Will throw an error if the variable was
   *        constructed with isConst == true.
   * @param value The new value for this variable.
   */
  void AliasVariable::setValue(const double value)
  {
	assertTrueMsg(!m_isConst,
				  "setValue() called on read-only alias " << *this);
	m_originalExpression->setValue(value);
  }

  void AliasVariable::handleChange(const ExpressionId& exp)
  {
	if (exp == m_originalExpression) {
	  // propagate value from original
	  internalSetValue(m_originalExpression->getValue());
	}
  }

  const VariableId& AliasVariable::getBaseVariable() const
  {
	if (VariableId::convertable(m_originalExpression))
	  return ((VariableId) m_originalExpression)->getBaseVariable();
	else
	  return Variable::getId();
  }

  void AliasVariable::handleActivate(const bool changed)
  {
	if (changed) {
	  m_originalExpression->activate();
	  // refresh value from original
	  internalSetValue(m_originalExpression->getValue());
	}
  }

  void AliasVariable::handleDeactivate(const bool changed)
  {
	if (changed) {
	  m_originalExpression->deactivate();
	}
  }

  void AliasVariable::handleReset()
  {
	// FIXME: do something
  }

}
