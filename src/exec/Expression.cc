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

//#define EXPRESSION_PRINT_STATISTICS 1

#include "Expression.hh"
#include "Debug.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "StoredArray.hh"
#include "Variable.hh"

#include <limits>
#include <algorithm>
#include <iomanip> // for setprecision()

namespace PLEXIL {

  ExpressionListener::ExpressionListener() : m_id(this), m_activeCount(0)/*m_active(true)*/ {}

  ExpressionListener::~ExpressionListener() {
    m_id.remove();
  }

  void ExpressionListener::activate() {
    ++m_activeCount;
  }

  void ExpressionListener::deactivate() {
    checkError(m_activeCount > 0,
	       "Attempted to deactivate an expression listener too many times.");
    --m_activeCount;
  }

  Expression::Expression()
    : m_id(this),
      m_value(UNKNOWN()), 
      m_savedValue(UNKNOWN()),
      m_activeCount(0), 
      m_dirty(false), 
      m_lock(false)
  {
  }

  Expression::~Expression() {
    checkError(m_outgoingListeners.empty(),
	       "Error: Expression '" << toString() << "' still has outgoing listeners.");
	m_id.remove();
  }

  const Value& Expression::getValue() const {
    return (isActive() ? m_value : UNKNOWN());
  }

  void Expression::setValue(const Value& val) {
    internalSetValue(val);
  }

  void Expression::activate() {
    bool changed = (m_activeCount == 0);
    ++m_activeCount;
    // debugMsg("Expression:activate", "Activating " << getId());
    handleActivate(changed);
#ifdef EXPRESSION_PRINT_STATISTICS
	static int sl_highWaterMark = 0;
	if (m_activeCount > sl_highWaterMark) {
	  sl_highWaterMark = m_activeCount;
	  std::cout << "Expression::activate: new max active count = " << sl_highWaterMark
				<< " for " << toString() << std::endl;
	}
#endif
  }

  void Expression::deactivate() {
    checkError(m_activeCount > 0,
	       "Attempted to deactivate expression " << getId() << " too many times.");
    --m_activeCount;
    bool changed = (m_activeCount == 0);
    // debugMsg("Expression:deactivate", "Deactivating " << getId());
    handleDeactivate(changed);
  }

  void Expression::addListener(ExpressionListenerId id) {
    check_error(id.isValid());
    if(std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), id) !=
       m_outgoingListeners.end())
      return;
    m_outgoingListeners.push_back(id);
#ifdef EXPRESSION_PRINT_STATISTICS
	static int sl_highWaterMark = 0;
	if (m_outgoingListeners.size() > sl_highWaterMark) {
	  sl_highWaterMark = m_outgoingListeners.size();
	  std::cout << "Expression::addListener: new max # listeners = " << sl_highWaterMark
				<< " for " << toString() << std::endl;
	}
#endif
  }

  void Expression::removeListener(ExpressionListenerId id) {
    check_error(id.isValid());
    std::vector<ExpressionListenerId>::iterator it = 
	  std::find(m_outgoingListeners.begin(), m_outgoingListeners.end(), id);
    if(it == m_outgoingListeners.end())
      return;
    m_outgoingListeners.erase(it);
  }

  void Expression::print(std::ostream& s) const
  {
    s << "(" << getId()
	  << "[" << (isActive() ? "a" : "i") << (isLocked() ? "l" : "u")
	  << "](";
	printValue(s);
	s << "): ";
  }

  std::string Expression::toString() const {
    std::ostringstream str;
	print(str);
    return str.str();
  }

  std::ostream& operator<<(std::ostream& s, const Expression& e)
  {
	e.print(s);
	return s;
  }

  void Expression::printValue(std::ostream& s) const
  {
	s << getValue();
  }

  std::string Expression::valueString() const {
    return getValue().valueToString();
  }

  void Expression::lock() {
    checkError(!isLocked(), toString() << " already locked.");
    checkError(isActive(), "Attempt to lock inactive expression " << toString());
    m_lock = true;
    m_savedValue = m_value;
  }

  void Expression::unlock() {
    checkError(isLocked(), toString() << " not locked.");
    if (m_dirty) {
      essentialSetValue(m_savedValue);
      m_dirty = false;
    }
    m_lock = false;
  }

  void Expression::internalSetValue(const Value& value)
  {
    checkError(checkValue(value), 
			   "Value " << value << " invalid for " << toString());
    if (isLocked()) {
      if (m_savedValue != value) {
        debugMsg("Expression:internalSetValue", 
                 " setting locked expression " << toString() << " to " << value);
		m_savedValue = value;
		m_dirty = true;
      }
    }
    else
      essentialSetValue(value);
  }

  void Expression::essentialSetValue(const Value& value)
  {
    if (m_value != value) {
      m_value = value;
      publishChange();
    }
  }

  void Expression::publishChange() {
    if(!isActive())
      return;
    for(std::vector<ExpressionListenerId>::iterator it = m_outgoingListeners.begin();
	it != m_outgoingListeners.end(); ++it) {
      if((*it)->isActive())
        (*it)->notifyValueChanged(m_id);
    }
  }

  // Used below in Expression::UNKNOWN_EXP().
  class UnknownVariable : public VariableImpl
  {
  public:
	UnknownVariable() : VariableImpl(true) {}
	~UnknownVariable() {}

	// Don't assign to this variable!
	bool checkValue(const Value& /* value */) const { return false; }
	
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
