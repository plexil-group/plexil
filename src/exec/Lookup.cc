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

#include "Lookup.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "StateCache.hh"
#include "Variables.hh"

namespace PLEXIL
{

  // *** To do:
  //  - implement multiple return values from lookups

  Lookup::Lookup(const PlexilExprId& expr, const NodeConnectorId& node)
    : VariableImpl(false), 
      m_cache(node->getExec()->getStateCache()),
      m_dest(1, m_id),
      m_state(Expression::UNKNOWN(),
              std::vector<double>(((PlexilLookup*)expr)->state()->args().size(),
                                  Expression::UNKNOWN())),
      m_listener(m_id)
  {
     checkError(Id<PlexilLookup>::convertable(expr), "Expected a lookup.");
     PlexilLookup* lookup = (PlexilLookup*) expr;
     PlexilState* state = lookup->state();
     
     // create the correct form of the expression for this name
	 bool nameExprIsNew = false;
     m_stateNameExpr = 
       ExpressionFactory::createInstance(state->nameExpr()->name(), 
                                         state->nameExpr(),
										 node,
										 nameExprIsNew);
     m_stateNameExpr->addListener(m_listener.getId());
	 if (nameExprIsNew)
	   m_garbage.push_back(m_stateNameExpr);

     // handle argument lookup
     getArguments(state->args(), node);
  }

  Lookup::~Lookup() 
  {
    // disconnect listeners
    m_stateNameExpr->removeListener(m_listener.getId());
    for (std::vector<ExpressionId>::iterator it = m_params.begin(); 
         it != m_params.end();
         ++it)
      (*it)->removeListener(m_listener.getId());

    // safe to delete anything in the garbage
	// possibly including state name expr
    for (std::vector<ExpressionId>::iterator it = m_garbage.begin(); 
         it != m_garbage.end();
         ++it) {
      delete (*it).operator->();
	}
  }

  void Lookup::getArguments(const std::vector<PlexilExprId>& args,
			    const NodeConnectorId& node) 
  {
    for (std::vector<PlexilExprId>::const_iterator it = args.begin(); it != args.end(); ++it) {
	  bool wasConstructed = false; 
	  ExpressionId param =
		ExpressionFactory::createInstance((*it)->name(), *it, node, wasConstructed);
	  check_error(param.isValid());
	  if (wasConstructed)
		m_garbage.push_back(param);
	  m_params.push_back(param);
	  param->addListener(m_listener.getId());
	  debugMsg("Lookup:getArguments",
			   " " << toString() << " added listener for " << param->toString());
	}
  }

  void Lookup::handleActivate(const bool changed) 
  {
    if (!changed)
      return;

    debugMsg("Lookup:handleActivate", " for " << toString());

    for (std::vector<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it)
      {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->activate();
    }
    check_error(m_stateNameExpr.isValid());
    m_stateNameExpr->activate();
    updateState();
    registerLookup();
    // Safe to activate once lookup is registered
    m_listener.activate();
  }

  void Lookup::handleDeactivate(const bool changed) {
    if(!changed)
      return;

    debugMsg("Lookup:handleDeactivate", " for " << toString());

    m_listener.deactivate();
    unregisterLookup();
    for(std::vector<ExpressionId>::iterator it = m_params.begin(); it != m_params.end(); ++it) {
      ExpressionId expr = *it;
      check_error(expr.isValid());
      expr->deactivate();
    }
    m_stateNameExpr->deactivate();
  }

  void Lookup::updateState()
  {
    checkError(m_stateNameExpr->isActive(),
               "Can't update state for lookup with an inactive name state expression: " << toString());
    m_state.first = m_stateNameExpr->getValue();
    std::vector<ExpressionId>::const_iterator it = m_params.begin();
    std::vector<double>::iterator sit = m_state.second.begin();
    while (it != m_params.end() && sit != m_state.second.end())
      {
        ExpressionId expr = *it;
        check_error(expr.isValid());
        checkError(expr->isActive(),
                   "Can't update state for lookup with an inactive parameter: " << toString());
        (*sit) = ((*it)->getValue());
        it++;
        sit++;
      }
  }

  bool Lookup::isStateCurrent() const
  {
    checkError(m_stateNameExpr->isActive(),
               "Can't compare state to lookup with an inactive name state expression: " << toString());
    if (m_state.first != m_stateNameExpr->getValue())
      return false;
    std::vector<ExpressionId>::const_iterator it = m_params.begin();
    std::vector<double>::const_iterator sit = m_state.second.begin();
    while (it != m_params.end() && sit != m_state.second.end())
      {
        ExpressionId expr = *it;
        check_error(expr.isValid());
        checkError(expr->isActive(),
                   "Can't compare state to lookup with an inactive parameter: " << toString());
        if ((*it)->getValue() != *sit)
          return false;
        it++;
        sit++;
      }
    return true;
  }

  std::string Lookup::stateToString(const State& state)
  {
    std::ostringstream os;
    os << LabelStr(state.first).toString();
    if (!state.second.empty()) {
      os << "(";
      size_t i = 0;
      size_t len = state.second.size();
      while (i < len) {
	os << Expression::valueToString(state.second[i]);
	i++;
	if (i < len)
	  os << ", ";
      }
      os << ")";
    }
    return os.str();
  }

  void Lookup::registerLookup() 
  {
    handleRegistration();
  }
   
  void Lookup::unregisterLookup() {
    handleUnregistration();
  }

  // *** this should be extended to use the global declarations

  PlexilType Lookup::getValueType() const
  {
    return PLEXIL::UNKNOWN_TYPE;
  }


  LookupNow::LookupNow(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node) 
  {
    checkError(Id<PlexilLookupNow>::convertable(expr), "Expected LookupNow.");
  }

  LookupNow::~LookupNow()
  {
  }

  void LookupNow::handleChange(const ExpressionId& /* exp */)
  {
    // need to notify state cache if cached lookup is no longer valid
    if (!isStateCurrent())
      {
	debugMsg("LookupNow:handleChange",
		 " state changed  updating state cache");
        const State oldState(m_state);
        updateState();
        handleRegistrationChange(oldState);
      }
  }

  void LookupNow::handleRegistration() 
  {
    debugMsg("LookupNow:handleRegistration", 
	     " for state " << stateToString(m_state));
    m_cache->registerLookupNow(m_id, m_dest, m_state);
  }

  // *** To do:
  //  - optimize by adding specific method for this case to StateCache class

  void LookupNow::handleRegistrationChange(const State& oldState)
  {
    debugMsg("LookupNow:handleRegistrationChange", 
	     " old state was " << stateToString(oldState)
	     << ",\n new state is " << stateToString(m_state));
    m_cache->unregisterLookupNow(m_id);
    m_cache->registerLookupNow(m_id, m_dest, m_state);
  }

  void LookupNow::handleUnregistration() 
  {
    debugMsg("LookupNow:handleUnregistration", 
	     " for state " << stateToString(m_state));
    m_cache->unregisterLookupNow(m_id);
  }

  void LookupNow::print(std::ostream& s) const
  {
	Expression::print(s);
	s << "LookupNow(" << m_stateNameExpr->valueString() << '(';
    for (std::vector<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end(); ++it)
      s << ", " << **it;
	s << ")))";

  }

  LookupOnChange::LookupOnChange(const PlexilExprId& expr, const NodeConnectorId& node)
    : Lookup(expr, node)
  {
    checkError(Id<PlexilChangeLookup>::convertable(expr), "Expected LookupOnChange");
    PlexilChangeLookup* lookup = (PlexilChangeLookup*) expr;

    if(lookup->tolerances().empty())
      m_tolerance = RealVariable::ZERO_EXP();
    else {
	  bool wasCreated = false;
	  m_tolerance = ExpressionFactory::createInstance(lookup->tolerances()[0]->name(),
													  lookup->tolerances()[0],
													  node,
													  wasCreated);
	  if (wasCreated)
		m_garbage.push_back(m_tolerance);
	  m_tolerance->addListener(m_listener.getId());
	}
  }

  LookupOnChange::~LookupOnChange()
  {
    m_tolerance->removeListener(m_listener.getId());
  }

  void LookupOnChange::print(std::ostream& s) const 
  {
	Expression::print(s);
	s << "LookupOnChange(" << m_stateNameExpr->valueString() << "(";
    for (std::vector<ExpressionId>::const_iterator it = m_params.begin(); it != m_params.end(); ++it)
      s << ", " << **it;
    s << "), " << *m_tolerance << "))";
  }

  void LookupOnChange::handleRegistration() {
    debugMsg("LookupOnChange:handleRegistration", 
	     " for state " << stateToString(m_state));
    m_tolerance->activate();
    m_cache->registerChangeLookup(m_id, m_dest, m_state, std::vector<double>(1, m_tolerance->getValue()));
  }

  void LookupOnChange::handleUnregistration() {
    debugMsg("LookupOnChange:handleUnregistration",
	     " for state " << stateToString(m_state));
    m_tolerance->deactivate();
    m_cache->unregisterChangeLookup(m_id);
  }

  void LookupOnChange::handleChange(const ExpressionId& exp)
  {
    // need to notify state cache if cached lookup is no longer valid
    if (isStateCurrent() && exp != m_tolerance)
      return;
    debugMsg("LookupOnChange:handleChange",
	     " state changed, updating state cache");
    const State oldState(m_state);
    updateState();
    handleRegistrationChange(oldState);
  }

  // *** To do:
  //  - optimize by adding specific method for this case to StateCache class

  void LookupOnChange::handleRegistrationChange(const State& oldState)
  {
    debugMsg("LookupOnChange:handleRegistrationChange", 
	     " old state was " << stateToString(oldState)
	     << ",\n new state is " << stateToString(m_state));
    m_cache->unregisterChangeLookup(m_id);
    m_cache->registerChangeLookup(m_id, m_dest, m_state, std::vector<double>(1, m_tolerance->getValue()));
  }

}
