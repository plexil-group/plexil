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

#include "Expression.hh"
#include "Debug.hh"
#include "Node.hh"

#include <limits>
#include <algorithm>
#include <iomanip> // for setprecision()

namespace PLEXIL {

  ExpressionListener::ExpressionListener() : m_id(this), m_activeCount(0)/*m_active(true)*/ {}

  ExpressionListener::~ExpressionListener() {
    m_id.remove();
  }

  void ExpressionListener::activate() {
    m_activeCount++;
  }

  void ExpressionListener::deactivate() {
    checkError(m_activeCount > 0,
	       "Attempted to deactivate an expression listener too many times.");
    m_activeCount--;
  }

  SubexpressionListener::SubexpressionListener(const ExpressionId& exp)
    : ExpressionListener(), m_exp(exp) {}

  void SubexpressionListener::notifyValueChanged(const ExpressionId& exp) {
    m_exp->handleChange(exp);
  }

  Expression::Expression()
    : m_id(this),
      m_activeCount(0), 
      m_value(UNKNOWN()), 
      m_savedValue(UNKNOWN()),
      m_dirty(false), 
      m_lock(false), 
      m_ignoreCachedValue(false)
  {
  }

  Expression::~Expression() {
    checkError(m_outgoingListeners.empty(),
	       "Error: Expression '" << toString() << "' still has outgoing listeners.");
    m_id.remove();
  }

  double Expression::getValue() const {
    return (isActive() ? m_value : UNKNOWN());
  }

  void Expression::setValue(const double val) {
    internalSetValue(val);
  }

  void Expression::activate() {
    bool changed = (m_activeCount == 0);
    m_activeCount++;
    debugMsg("Expression:activate", "Activating " << getId());
    handleActivate(changed);
  }

  void Expression::deactivate() {
    checkError(m_activeCount > 0,
	       "Attempted to deactivate expression " << getId() << " too many times.");
    bool changed = (m_activeCount == 1);
    m_activeCount--;
    debugMsg("Expression:deactivate", "Deactivating " << getId());
    handleDeactivate(changed);
  }

  void Expression::addListener(ExpressionListenerId id) {
    check_error(id.isValid());
    if(std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), id) !=
       m_outgoingListeners.end())
      return;
    m_outgoingListeners.push_back(id);
  }

  void Expression::removeListener(ExpressionListenerId id) {
    check_error(id.isValid());
    std::list<ExpressionListenerId>::iterator it = std::find(m_outgoingListeners.begin(),
							     m_outgoingListeners.end(), id);
    if(it == m_outgoingListeners.end())
      return;
    m_outgoingListeners.erase(it);
  }

  std::string Expression::toString() const {
    std::ostringstream str;
    str << "(" << getId() << "[" << (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") <<
      "](" << valueString() << "): ";
    return str.str();
  }

  // Much-needed static member function to construct the One True Printed Representation of a value.

  std::string Expression::valueToString(const double val) {
	if (val == UNKNOWN())
	  return std::string("UNKNOWN");
    else if (LabelStr::isString(val))
      return std::string(LabelStr(val).toString());
    else if (StoredArray::isKey(val))
      return StoredArray(val).toString();
	// below this point must be a number
    else if (val == REAL_PLUS_INFINITY)
      return std::string("inf");
    else if (val == REAL_MINUS_INFINITY)
      return std::string("-inf");
	else {
	  std::ostringstream str;
      str << std::setprecision(15) << val;
	  return str.str();
	}
  }

  std::string Expression::valueString() const {
    return valueToString(getValue());
  }

  void Expression::lock() {
    checkError(!isLocked(), toString() << " already locked.");
    checkError(isActive(), "Attempt to lock inactive expression " << toString());
    m_lock = true;
    m_savedValue = m_value;
    handleLock();
  }

  void Expression::unlock() {
    checkError(isLocked(), toString() << " not locked.");
    m_lock = false;
    if(m_dirty)
      internalSetValue(m_savedValue);
    handleUnlock();
  }

  void Expression::internalSetValue(const double value) {
    checkError(checkValue(value), 
			   "Value " << valueToString(value) << " invalid for " << toString());
    if (isLocked()) {
      if (m_savedValue != value || m_value != value) {
		m_dirty = true;
		m_savedValue = value;
      }
    }
    else {
      bool changed = (m_value != value);
      m_value = value;
      m_dirty = false;
      if(m_ignoreCachedValue || changed)
		publishChange();
    }
  }

  void Expression::publishChange() {
    if(!isActive())
      return;
    for(std::list<ExpressionListenerId>::iterator it = m_outgoingListeners.begin();
	it != m_outgoingListeners.end(); ++it) {
      check_error((*it).isValid());
      if((*it)->isActive())
	(*it)->notifyValueChanged(m_id);
    }
  }

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
	: Variable(),
	  m_derid(this, Variable::getId())
	{} 

  /**
   * @brief Constructor.
   */
  DerivedVariable::DerivedVariable(const NodeConnectorId& node)
	: Variable(node),
	  m_derid(this, Variable::getId())
  {}


  /**
   * @brief Destructor.
   */
  DerivedVariable::~DerivedVariable()
  {
	m_derid.removeDerived(Variable::getId());
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
    check_error(Id<PlexilValue>::convertable(expr));
  }

  VariableImpl::~VariableImpl() {}

  std::string VariableImpl::toString() const 
  {
    std::ostringstream str;
    str << m_name << " (" << getId() << "[" 
	<< (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") 
	<< "](" 
	<< valueToString(m_value) << "): ";
    return str.str();
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

  void VariableImpl::commonNumericInit(PlexilValue* val) {
    if(val->value() == "UNKNOWN")
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

  Calculable::Calculable() : Expression(), m_listener(getId()) {}

  Calculable::Calculable(const PlexilExprId& expr, const NodeConnectorId& node)
    : Expression(), m_listener(getId())
  {
    const std::vector<PlexilExprId>& subExprs = expr->subExprs();
    for (std::vector<PlexilExprId>::const_iterator it = subExprs.begin(); 
		 it != subExprs.end();
		 ++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      addSubexpression(subExpr, garbage);
    }
  }

  Calculable::~Calculable() {
    for(ExpressionVectorIter it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->removeListener(m_listener.getId());
    }
    cleanup(m_garbage);
  }

   bool Calculable::containsSubexpression(const ExpressionId& expr)
   {
      for(ExpressionVectorIter it = m_subexpressions.begin();
          it != m_subexpressions.end(); ++it)
      {
         if (expr.equals(*it))
            return true;
      }
      return false;
   }
   void Calculable::addSubexpression(const ExpressionId& expr,
                                     const bool garbage)
   {
      expr->addListener(m_listener.getId());
      m_subexpressions.push_back(expr);
      if(garbage)
         m_garbage.insert(expr);
   }

  void Calculable::removeSubexpression(const ExpressionId& expr) {
    // this is necessary because std::vector doesn't have a find() method!
    for (ExpressionVectorIter it = m_subexpressions.begin();
	 it != m_subexpressions.end();
	 it++) {
      if (*it == expr) {
	m_subexpressions.erase(it);
	break;
      }
    }
    m_garbage.erase(expr);
    expr->removeListener(m_listener.getId());
  }

   ExpressionId Calculable::getSubexpression(const PlexilExprId& expr,
                                             const NodeConnectorId& node,
                                             bool& del)
   {
      return ExpressionFactory::createInstance(LabelStr(expr->name()), 
                                               expr, 
                                               node, 
                                               del);
   }

  void Calculable::setValue(const double /* value */) {
    checkError(ALWAYS_FAIL, "Shouldn't set the value of a calculable expression.");
  }

  void Calculable::handleChange(const ExpressionId& exp) {
    internalSetValue(recalculate());
    handleSubexpressionChange(exp);
  }

  //this could be optimized slightly more to check for dirtiness on subexpressions
  //but that would require setting dirtiness when deactivated, not just when locked
  void Calculable::handleActivate(const bool changed) {
    if(!changed)
      return;
    m_listener.activate();
    for(ExpressionVectorIter it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    internalSetValue(recalculate());
  }

  void Calculable::handleDeactivate(const bool changed) {
    if(!changed)
      return;
    m_listener.deactivate();
    for(ExpressionVectorIter it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId& expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
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

  // This variant used only in unit tests
  TransparentWrapper::TransparentWrapper(const ExpressionId& exp)
    : Expression(), m_listener(*this), m_exp(exp) 
  {
	commonInit(exp);
  }

  TransparentWrapper::TransparentWrapper(const ExpressionId& exp, const NodeConnectorId& node)
    : Expression(), m_listener(*this), m_exp(exp) 
  {
	commonInit(exp);
  }

  TransparentWrapper::~TransparentWrapper(){
    m_exp->removeListener(m_listener.getId());
  }

  void TransparentWrapper::commonInit(const ExpressionId& exp)
  {
    debugMsg("TransparentWrapper:TransparentWrapper",
	     "Constructing a transparent wrapper around " << exp->toString());
    m_exp->addListener(m_listener.getId());
       //m_listener.activate();
    m_value = exp->getValue();
    debugMsg("TransparentWrapper:TransparentWrapper",
	     "Constructed " << toString());
  }

  void TransparentWrapper::setValue(const double value) {
    debugMsg("TransparentWrapper:setValue",
			 "Setting " << toString() << " to value " << Expression::valueToString(value));
    internalSetValue(value);
    m_exp->setValue(value);
  }

  bool TransparentWrapper::checkValue(const double value) {
    debugMsg("TransparentWrapper:checkValue",
			 "Checking " << toString() << " value " << Expression::valueToString(value));
    return m_exp->checkValue(value);
  }

  std::string TransparentWrapper::toString() const {
    std::ostringstream str;
    str << "TransparentWrapper(" << getId() << "[" << (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") <<
       "T]{" << valueString() <<"}(" << m_exp->toString() << ")";
    return str.str();
  }

  std::string TransparentWrapper::valueString() const {
    return m_exp->valueString();
  }

  void TransparentWrapper::handleActivate(const bool /* changed */) {
    m_listener.activate();
    m_exp->activate();
    if(m_exp->isActive())
      internalSetValue(m_exp->getValue());
  }

  void TransparentWrapper::handleDeactivate(const bool /* changed */) {
    m_exp->deactivate();
    m_listener.deactivate();
  }

  void TransparentWrapper::handleChange(const ExpressionId& expression) {
    debugMsg("TransparentWrapper:handleChange",
	     "Changing from wrapped value of " << toString() << " to " <<
	     expression->toString());
    internalSetValue(expression->getValue());
  }

  void ExpressionFactory::registerFactory(const LabelStr& name, ExpressionFactory* factory) {
    check_error(factory != NULL);
    checkError(factoryMap().find(name) == factoryMap().end(),
	       "Error:  Attempted to register a factory for name '" << name.toString() <<
	       "' twice.");
    factoryMap()[name] = factory;
    debugMsg("ExpressionFactory:registerFactory",
	     "Registered factory for name '" << name.toString() << "'");
  }


   ExpressionId ExpressionFactory::createInstance(const LabelStr& name,
                                                  const PlexilExprId& expr,
                                                  const NodeConnectorId& node)
   {
     bool dummy;
     return createInstance(name, expr, node, dummy);
   }

   ExpressionId ExpressionFactory::createInstance(const LabelStr& name,
                                                  const PlexilExprId& expr,
                                                  const NodeConnectorId& node,
                                                  bool& wasCreated)
   {
      // if this is a variable ref, look it up
      
      if (Id<PlexilVarRef>::convertable(expr)) 
      {
         checkError(node.isValid(), "Need a valid Node argument to find a Variable");
         ExpressionId retval = node->findVariable(expr);         
	 checkError(retval.isValid(), "Unable to find variable '" << expr->name() << "'");
         wasCreated = false;
         return retval;
      }

      // otherwise look up factory
      
      std::map<double, ExpressionFactory*>::const_iterator it = factoryMap().find(name);
      checkError(it != factoryMap().end(),
                 "Error: No factory registered for name '" << name.toString() << "'.");
      ExpressionId retval = it->second->create(expr, node);
      debugMsg("ExpressionFactory:createInstance", "Created " << retval->toString());
      wasCreated = true;
      return retval;
   }

  std::map<double, ExpressionFactory*>& ExpressionFactory::factoryMap() {
    static std::map<double, ExpressionFactory*>* sl_map = NULL;
    if (sl_map == NULL)
      sl_map = new std::map<double, ExpressionFactory*>();

    return *sl_map;
  }

  void ExpressionFactory::purge() {
    for(std::map<double, ExpressionFactory*>::iterator it = factoryMap().begin();
	it != factoryMap().end(); ++it)
      delete it->second;
    factoryMap().clear();
  }

  // Used below in Expression::UNKNOWN_EXP().
  class UnknownVariable : public VariableImpl
  {
  public:
	UnknownVariable() : VariableImpl(true) {}
	~UnknownVariable() {}

	// Don't assign to this variable!
	bool checkValue(const double /* value */) { return false; }
	
  private:
	// Deliberately unimplemented
	UnknownVariable(const UnknownVariable&);
	UnknownVariable& operator=(const UnknownVariable&);
  };

  ExpressionId& Expression::UNKNOWN_EXP() {
    static ExpressionId sl_exp;
    if (sl_exp.isNoId())
      sl_exp = (new UnknownVariable())->getId();
    return sl_exp;
  }

}
