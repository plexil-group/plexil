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
#include "Utils.hh"
#include "Debug.hh"
#include "Node.hh"

#include <limits>
#include <algorithm>

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

  Expression::Expression(PlexilExpr* expr, const NodeConnectorId& node)
    : m_id(this),
      m_activeCount(0), 
      m_value(UNKNOWN()), 
      m_savedValue(UNKNOWN()),
      m_dirty(false), 
      m_lock(false), 
      m_ignoreCachedValue(false),
      m_nodeConnector(node)
  {
  }

  Expression::Expression()
    : m_id(this),
      m_activeCount(0), 
      m_value(UNKNOWN()), 
      m_savedValue(UNKNOWN()),
      m_dirty(false), 
      m_lock(false), 
      m_ignoreCachedValue(false),
      m_nodeConnector(NodeConnectorId::noId())
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

  /**
   * @brief Get the node that owns this expression.
   * @return The NodeId of the parent node; may be noId.
   */
  const NodeId& Expression::getNode() const 
  { 
    if (m_nodeConnector.isNoId())
      return NodeId::noId();
    return m_nodeConnector->getNode();
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
    std::stringstream str;
    str << "(" << getId() << "[" << (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") <<
      "](" << valueString() << "): ";
    return str.str();
  }

  std::string Expression::valueString() const {
    std::stringstream str;
    double val = getValue();
    if(LabelStr::isString(val))
      str << LabelStr(val).toString();
    else
      str << val;
    return str.str();
  }

  void Expression::lock() {
    checkError(!isLocked(), toString() << " already locked.");
    checkError(isActive(), toString() << " inactive.");
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
    checkError(checkValue(value), "Value " << value << " invalid for " << toString());
    if(isLocked()) {
      if(m_savedValue != value || m_value != value) {
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


  EssentialVariable::EssentialVariable()
    : Expression()
  {
  }

  EssentialVariable::EssentialVariable(PlexilExpr* expr, const NodeConnectorId& node)
    : Expression(expr, node)
  {
  }

  EssentialVariable::~EssentialVariable()
  {
  }

  // Used only in Expression::UNKNOWN_EXP()

  Variable::Variable(const bool isConst)
    : EssentialVariable(), m_isConst(isConst), m_initialValue(UNKNOWN()), m_name("anonymous") 
  {
    if(this->isConst())
      m_activeCount++;
  }

  // Used only in Lookup::Lookup(const StateCacheId&, const LabelStr&, std::list<double>&)

  Variable::Variable(const double value, const bool isConst)
    : EssentialVariable(), m_isConst(isConst), m_initialValue(value), m_name("anonymous") 
  {
    m_value = m_initialValue;
    if(this->isConst())
      m_activeCount++;
  }

  //
  // ExpressionFactory constructor
  // uses PlexilVar prototype
  //

  Variable::Variable(const PlexilExprId& expr, const NodeConnectorId& node, const bool isConst)
    : EssentialVariable(expr, node), m_isConst(isConst), m_name(expr->name())
  {
    check_error(Id<PlexilValue>::convertable(expr));
  }

  Variable::~Variable() {}

  std::string Variable::toString() const 
  {
    std::stringstream str;
    str << m_name << " (" << getId() << "[" 
	<< (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") 
	<< "](" 
	<< valueString() << "): ";
    return str.str();
  }

  void Variable::reset() {
    if(!isConst()) {
      internalSetValue(m_initialValue);
      handleReset();
    }
  }

  void Variable::handleDeactivate(const bool changed) {
    if(this->isConst() && changed)
      m_activeCount++;
  }

  void Variable::setValue(const double value) {
    checkError(!isConst(), "Attempted to set value " << value << " to " << toString());
    internalSetValue(value);
  }

  void Variable::commonNumericInit(PlexilValue* val) {
    if(val->value() == "UNKNOWN")
      m_initialValue = m_value = Expression::UNKNOWN();
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
      checkError(checkValue(m_value), "Invalid " << PlexilParser::valueTypeString(val->type()) << " '" << m_value << "'");
    }
  }

  Calculable::Calculable() : Expression(), m_listener(getId()) {}

  Calculable::Calculable(PlexilExpr* expr, const NodeConnectorId& node)
    : Expression(expr, node), m_listener(getId()) {
    const std::vector<PlexilExprId>& subExprs = expr->subExprs();
    for(std::vector<PlexilExprId>::const_iterator it = subExprs.begin(); it != subExprs.end();
	++it) {
      bool garbage = false;
      ExpressionId subExpr = getSubexpression(*it, node, garbage);
      addSubexpression(subExpr, garbage);
    }
  }

  Calculable::~Calculable() {
    for(std::list<ExpressionId>::iterator it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->removeListener(m_listener.getId());
    }
    cleanup(m_garbage);
  }

   bool Calculable::containsSubexpression(const ExpressionId& expr)
   {
      for(std::list<ExpressionId>::iterator it = m_subexpressions.begin();
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
    m_subexpressions.remove(expr);
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

  void Calculable::setValue(const double value) {
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
    for(std::list<ExpressionId>::iterator it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    internalSetValue(recalculate());
  }

  void Calculable::handleDeactivate(const bool changed) {
    if(!changed)
      return;
    m_listener.deactivate();
    for(std::list<ExpressionId>::iterator it = m_subexpressions.begin();
	it != m_subexpressions.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
  }

  ConstVariableWrapper::ConstVariableWrapper(const ExpressionId& exp)
    : Variable(true), m_exp(exp), m_listener(this) {
    m_exp->addListener(m_listener.getId());
  }

  ConstVariableWrapper::ConstVariableWrapper()
    : Variable(true), m_listener(this) {}

  ConstVariableWrapper::~ConstVariableWrapper() {
    checkError(m_exp.isValid(), "Got to destructor without a valid wrapped variable.");
    m_exp->removeListener(m_listener.getId());
  }

  void ConstVariableWrapper::setWrapped(const ExpressionId& expr) {
    checkError(m_exp.isInvalid(),
	       "Attmpted to set wrapped variable to " << expr->toString() <<
	       " when already wrapping " << m_exp->toString());
    checkError(expr.isValid(),
	       "Attempted to set an invalid wrapped variable.");
    m_exp = expr;
    m_exp->addListener(m_listener.getId());
  }

  double ConstVariableWrapper::getValue() const {
    checkError(m_exp.isValid(), "Got to getValue without a valid wrapped variable.");
    return m_exp->getValue();
  }


  std::string ConstVariableWrapper::valueString() const {
    checkError(m_exp.isValid(), "Got to valueString without a valid wrapped variable.");
    std::stringstream str;
    str << "const " << m_exp->valueString();
    return str.str();
  }

  void ConstVariableWrapper::setValue(const double value) {
    checkError(ALWAYS_FAIL,
	       "Attempted to set the value of a const expression.");
  }

  bool ConstVariableWrapper::checkValue(const double val) {
    checkError(ALWAYS_FAIL,
	       "Attempted to check an incoming value for const expresssion " << toString());
    return false;
  }

  void ConstVariableWrapper::handleActivate(const bool changed) {
    checkError(m_exp.isValid(), "Got to handleActivate without a valid wrapped variable.");
    m_listener.activate();
    m_exp->activate();
  }
  void ConstVariableWrapper::handleDeactivate(const bool changed) {
    checkError(m_exp.isValid(), "Got to handleDeactivate without a valid wrapped variable.");
    m_listener.deactivate();
    m_exp->deactivate();
  }

  void ConstVariableWrapper::handleChange(const ExpressionId& expr) {
    checkError(m_exp.isValid(), "Got to handleChange without a valid wrapped variable.");
    publishChange();
  }

  TransparentWrapper::TransparentWrapper(const ExpressionId& exp)
    : Expression(), m_listener(*this), m_exp(exp) {
    debugMsg("TransparentWrapper:TransparentWrapper",
	     "Constructing a transparent wrapper around " << exp->toString());
    m_exp->addListener(m_listener.getId());
       //m_listener.activate();
    m_value = exp->getValue();
    debugMsg("TransparentWrapper:TransparentWrapper",
	     "Constructed " << toString());
  }

  TransparentWrapper::TransparentWrapper() : Expression(), m_listener(*this) {
    check_error(ALWAYS_FAIL, "Should never call this.");
  }

  TransparentWrapper::~TransparentWrapper(){
    m_exp->removeListener(m_listener.getId());
  }

  void TransparentWrapper::setValue(const double value) {
    debugMsg("TransparentWrapper:setValue",
	     "Setting " << toString() << " to value " << value);
    internalSetValue(value);
    m_exp->setValue(value);
  }

  bool TransparentWrapper::checkValue(const double value) {
    debugMsg("TransparentWrapper:checkValue",
	     "Checking " << toString() << " value " << value);
    return m_exp->checkValue(value);
  }

  std::string TransparentWrapper::toString() const {
    std::stringstream str;
    str << "TransparentWrapper(" << getId() << "[" << (isActive() ? "a" : "i") << (isLocked() ? "l" : "u") <<
       "T]{" << valueString() <<"}(" << m_exp->toString() << ")";
    return str.str();
  }

  std::string TransparentWrapper::valueString() const {
    return m_exp->valueString();
  }

  void TransparentWrapper::handleActivate(const bool changed) {
    m_listener.activate();
    m_exp->activate();
    if(m_exp->isActive())
      internalSetValue(m_exp->getValue());
  }

  void TransparentWrapper::handleDeactivate(const bool changed) {
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
    static std::map<double, ExpressionFactory*> sl_map;
    return sl_map;
  }

  void ExpressionFactory::purge() {
    for(std::map<double, ExpressionFactory*>::iterator it = factoryMap().begin();
	it != factoryMap().end(); ++it)
      delete it->second;
    factoryMap().clear();
  }

  ExpressionId& Expression::UNKNOWN_EXP() {
    static ExpressionId sl_exp = (new Variable(true))->getId();
    return sl_exp;
  }

}
