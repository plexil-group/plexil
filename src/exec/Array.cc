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

#include "Array.hh"
#include "Debug.hh"
#include "ExecConnector.hh"
#include "ExecListenerHub.hh"
#include "ExpressionFactory.hh"
#include "NodeConnector.hh"
#include "Utils.hh" // for compareIgnoreCase()

#include <cstring> // for strcmp()

namespace PLEXIL
{

  //
  // ArrayVariableBase
  //

  ArrayVariableBase::ArrayVariableBase()
    : Variable(),
      m_avid(this, Variable::getId())
  {
  }

  ArrayVariableBase::~ArrayVariableBase()
  {
    m_avid.removeDerived(Variable::getId());
  }

  //
  // ArrayAliasVariable
  //

  ArrayAliasVariable::ArrayAliasVariable(const std::string& name,
                                         const NodeConnectorId& nodeConnector,
                                         const ExpressionId& exp,
                                         bool expIsGarbage,
                                         bool isConst)
    : ArrayVariableBase(),
      AliasVariable(name, nodeConnector, exp, expIsGarbage, isConst),
      m_originalArray((ArrayVariableId) exp)
  {
    // Check original, node for validity
    assertTrueMsg(m_originalArray.isId(),
                  "Invalid array passed to ArrayAliasVariable constructor");
    assertTrue(nodeConnector.isValid(),
               "Invalid node connector ID passed to AliasVariable constructor");
  }

  ArrayAliasVariable::~ArrayAliasVariable()
  {
    assertTrue(m_originalArray.isValid(),
               "Original expression ID invalid in AliasVariable destructor");
  }

  /**
   * @brief Get a string representation of this Expression.
   * @return The string representation.
   */
  void ArrayAliasVariable::print(std::ostream& s) const
  {
    s << getName() << " ";
    Expression::print(s);
    s << (isConst() ? "const " : "") 
      << "ArrayAliasVariable for "
      << *m_originalArray
      << ")";
  }

  PlexilType ArrayAliasVariable::getValueType() const
  {
    return m_originalArray->getValueType();
  }

  size_t ArrayAliasVariable::maxSize() const
  {
    return m_originalArray->maxSize();
  }

  const Value& ArrayAliasVariable::lookupValue(size_t index) const
  {
    return m_originalArray->lookupValue(index);
  }

  void ArrayAliasVariable::setElementValue(size_t /* index */, const Value& /* value */)
  {
    assertTrueMsg(!isConst(),
                  "Attempt to call setElementValue() on const array alias " << *this);
  }

  PlexilType ArrayAliasVariable::getElementType() const
  {
    return m_originalArray->getElementType();
  }

  bool ArrayAliasVariable::checkElementValue(const Value& val) const
  {
    return m_originalArray->checkElementValue(val);
  }

  ArrayVariable::ArrayVariable(size_t maxSize,
                               PlexilType type,
                               const bool isConst)
    : ArrayVariableBase(), 
      VariableImpl(false), // set m_isConst below
      m_maxSize(maxSize), 
      m_type(type)
  {
    debugMsg("ArrayVariable", " constructor, no initial elements");
    setValue(StoredArray(maxSize));
    if (isConst)
      makeConst();
  }

  ArrayVariable::ArrayVariable(size_t maxSize, 
                               PlexilType type, 
                               const std::vector<Value>& values, 
                               const bool isConst)
    : ArrayVariableBase(),
      VariableImpl(false),
      m_maxSize(maxSize),
      m_type(type)
  {
    assertTrueMsg(values.size() <= maxSize,
                  "ArrayVariable constructor: Initial array size " << values.size()
                  << " exceeds target size " << maxSize);
    debugMsg("ArrayVariable", " constructor, " << values.size() << " initial elements");
    StoredArray array(maxSize);
    // Let StringArrayVariable do its own initialization
    if (m_type != STRING) {
      // Set array values
      for (size_t i = 0; i < values.size(); ++i) {
        checkError(checkElementValue(values[i]),
                   "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                   << " array to invalid value \"" << values[i] << "\"");
        array[i] = values[i];
      }
      m_initialValue = array;
      setValue(m_initialValue);
      if (isConst)
        makeConst();
    }
  }

  ArrayVariable::ArrayVariable(const PlexilExprId& expr, 
                               const NodeConnectorId& node,
                               const bool isConst)
    : ArrayVariableBase(),
      VariableImpl(expr, node)
  {
    debugMsg("ArrayVariable", " constructor from intermediate representation");

    // confirm that we have a an array variable
    const PlexilArrayValue* arrayValue = NULL;
    if (Id<PlexilArrayVar>::convertable(expr)) {
      const Id<PlexilArrayVar> var = (const Id<PlexilArrayVar>) expr;
      m_type = var->elementType();
      m_maxSize = var->maxSize();
      if (var->value() != NULL) {
        arrayValue = dynamic_cast<const PlexilArrayValue*>(var->value());
        assertTrueMsg(arrayValue != NULL,
                      "Array variable initial value is not a PlexilArrayValue");
      }
    }
    else if (Id<PlexilArrayValue>::convertable(expr)) {
      arrayValue = (const PlexilArrayValue*) expr;
      m_type = arrayValue->type();
      m_maxSize = arrayValue->maxSize();
    }
    else {
      assertTrueMsg(ALWAYS_FAIL,
                    "ArrayVariable constructor: Expected a PlexilArrayVar or PlexilArrayValue");
    }

    // init the local type and array
    StoredArray array(m_maxSize, UNKNOWN());
    m_initialValue = array; // for use by StringArrayVariable constructor

    // Defer to StringArrayVariable constructor
    if (m_type != STRING) {
      if (arrayValue != NULL) {
        const std::vector<std::string>& values = arrayValue->values();
        assertTrueMsg(values.size() <= m_maxSize,
                      "ArrayVariable constructor: Number of initial values, "
                      << values.size() << ", exceeds max size, " << m_maxSize);
        for (size_t i = 0; i < values.size(); ++i) {
          double convertedValue;
          if (m_type == BOOLEAN) {
            if (compareIgnoreCase(values[i], "true") || 
                (strcmp(values[i].c_str(), "1") == 0))
              convertedValue = 1.0;
            else if (compareIgnoreCase(values[i], "false") || 
                     (strcmp(values[i].c_str(), "0") == 0))
              convertedValue = 0.0;
            else
              assertTrueMsg(ALWAYS_FAIL, 
                            "Attempt to initialize Boolean array variable with invalid value \""
                            << values[i] << "\"");
          }
          else {
            std::istringstream valueStream(values[i]);
            valueStream >> convertedValue;
            checkError(checkElementValue(convertedValue),
                       "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                       << " array to invalid value \"" << values[i] << "\"");
          }
          array[i] = convertedValue;
        }
      }
      VariableImpl::setValue(m_initialValue);
      if (isConst)
        makeConst();
    }
  }

  // 
  // Destructor
  //

  ArrayVariable::~ArrayVariable()
  {
  }

  // set the value of this array
  void ArrayVariable::setValue(const Value& value)
  {
    // Check if new value == current
    if (m_value == value) {
      debugMsg("ArrayVariable:setValue", " to existing value");
      return; // nothing to do
    }

    // Check if new == initial (e.g. Variable::reset() or retracting an assignment)
    if (value == m_initialValue
        || (value.isArray() && m_initialValue.getConstArrayValue() == value.getConstArrayValue())) {
      debugMsg("ArrayVariable:setValue", " to initial value");
      VariableImpl::setValue(m_initialValue);
      return;
    }

    // Check new value
    if (value.isUnknown()) {
      debugMsg("ArrayVariable:setValue", " to UNKNOWN");
      VariableImpl::setValue(value);
      return;
    }
    assertTrueMsg(value.isArray(),
                  "ArrayVariable::setValue: new value " << value
                  << " is not an array value or UNKNOWN");
    assertTrueMsg(value.getConstArrayValue().size() <= m_maxSize,
                  "ArrayVariable::setValue: new value size, "
                  << value.getConstArrayValue().size()
                  << ", is larger than the maximum size, " << m_maxSize);

    // Below this line, we will be replacing contents of the current array
    debugMsg("ArrayVariable:setValue", " general case");

    if (m_value.isUnknown() || m_value == m_initialValue) {
      debugMsg("ArrayVariable:setValue", " allocating new array");
      // Allocate a new array so as not to disturb the initial value
      m_value = StoredArray(m_maxSize, UNKNOWN());
    }

    // copy the array
    const std::vector<Value>& sourceArray = value.getConstArrayValue();
    std::vector<Value>& array = m_value.getArrayValue();
    size_t i = 0;
    for (; i < sourceArray.size(); ++i)
      array[i] = sourceArray[i];
    while (i < m_maxSize)
      array[i++].setUnknown();

    // FIXME: This is a kludge to ensure listeners are notified.
    // As we have copied in place, the "value" may not have changed,
    // so Expression::internalSetValue() may not notify listeners.
    // This kludge doesn't respect the Expression class lock.
    publishChange();
  }

  // set an element value in an array variable

  void ArrayVariable::setElementValue(size_t index, const Value& value)
  {
    // lotsa potential errors to check
    assertTrueMsg(!VariableImpl::isConst(),
                  "Attempted to set element value of const array " << *this);
    assertTrueMsg(!m_value.isUnknown(),
                  "Attempted to assign an array element in an UNKNOWN array");
    assertTrueMsg(checkElementValue(value),
                  "Attempted to set element of " << PlexilParser::valueTypeString(getElementType())
                  << " array variable to invalid value \"" << value << "\"");
    assertTrueMsg(checkIndex(index),
                  "Array index " << index << " exceeds bound of " 
                  << m_maxSize);

    debugMsg("ArrayVariable:setElementValue",
             " for " << *this << " @ index " << index << ", new value is " << value);
    // set the element
    if (value != m_value.getConstArrayValue()[index]) {
      bool newArray = false;
      // Implement copy-on-write semantics to avoid clobbering initial value
      if (m_value == m_initialValue) {
        // Clone the initial array
        debugMsg("ArrayVariable:setElementValue", " copying initial array");
        m_value.copyArray(m_initialValue.getStoredArrayValue());
        newArray = true;
      }

      m_value.getArrayValue()[index] = value;
      // FIXME: This is a kludge to ensure listeners are notified.
      // This kludge doesn't respect the Expression class lock.
      // See Expression::internalSetValue.
      publishChange();
    }
    ExecListenerHubId hub = getExecListenerHub();
    if (hub.isId()) {
      std::ostringstream s;
      s << m_name.toString() << '[' << index << ']'; // FIXME: this is unlikely to be right
      hub->notifyOfAssignment(Expression::getId(), s.str(), value);
    }
  }

  // lookup a value in an array variable
  const Value& ArrayVariable::lookupValue(size_t index) const
  {
    assertTrueMsg(checkIndex(index),
                  "Array index " << index << " exceeds bound of " 
                  << m_maxSize);
    const Value& result = 
      m_value.isUnknown()
      ? UNKNOWN()
      : m_value.getConstArrayValue()[index];
    debugMsg("ArrayVariable:lookupValue", " for array " << m_value << "\n returning " << result);
    return result;
  }

  void ArrayVariable::print(std::ostream& s) const 
  {
    VariableImpl::print(s);
    s << "array)"; // contents already printed!
  }

  // confirm that new value to assign is valid

  bool ArrayVariable::checkValue(const Value& val) const
  {
    if (val.isUnknown())
      return true;
    if (val.isArray()) {
      const std::vector<Value>& valArray = val.getConstArrayValue();
      if (valArray.size() > m_maxSize)
        return false;
      for (size_t i = 0; i < valArray.size(); i++) {
        if (!checkElementValue(valArray[i]))
          return false;
      }
      return true;
    }
    return false;
  }

  // confirm that array element is valid

  bool ArrayVariable::checkElementValue(const Value& val) const
  {
    // check value based on array type
    switch (m_type) {
    case INTEGER:
      return val.isInteger() || val.isUnknown();
    case REAL:
      return val.isReal() || val.isUnknown();
    case BOOLEAN:
      return val.isBoolean() || val.isUnknown();
    case STRING:
      return val.isString() || val.isUnknown();
    case ARRAY:
      assertTrue(ALWAYS_FAIL, "Arrays of arrays not yet supported.");
    case TIME:
      assertTrue(ALWAYS_FAIL, "TimePoints not supported in arrays.");
    default:
      assertTrueMsg(ALWAYS_FAIL, "Unknown variable type: " << m_type);
    }
    // should never get here
    return false;
  }

  /**
   * @brief Temporarily stores the previous value of this variable.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayVariable::saveCurrentValue()
  {
    m_savedValue.copyArray(m_value.getStoredArrayValue());
  }
   
  //
  // StringArrayVariable
  //

  StringArrayVariable::StringArrayVariable(size_t maxSize,
                                           PlexilType type,
                                           const bool isConst)
    : ArrayVariable(maxSize, type)
  {
    debugMsg("StringArrayVariable", " constructor, no initial elements");
    checkError(type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    if (isConst)
      makeConst();
  }

  StringArrayVariable::StringArrayVariable(size_t maxSize, 
                                           PlexilType type, 
                                           const std::vector<Value>& values,
                                           const bool isConst)
    : ArrayVariable(maxSize, type, values, false)
  {
    debugMsg("StringArrayVariable", " constructor, " << values.size() << " initial elements");
    assertTrue(type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    for (size_t i = 0; i < values.size(); ++i) {
      checkError(checkElementValue(values[i]),
                 "Attempted to initialize element of " << PlexilParser::valueTypeString(getElementType())
                 << " array to invalid value \"" << values[i] << "\"");
      m_initialValue.getArrayValue()[i] = values[i];
    }
    setValue(m_initialValue);
    if (isConst)
      makeConst();
  }

  StringArrayVariable::StringArrayVariable(const PlexilExprId& expr, 
                                           const NodeConnectorId& node,
                                           const bool isConst)
    : ArrayVariable(expr, node, false)
  {
    debugMsg("StringArrayVariable", " constructor from intermediate representation");
    assertTrue(m_type == STRING,
               "StringArrayVariable constructor: type is not STRING");
    const PlexilArrayValue* arrayValue = NULL;
    if (Id<PlexilArrayVar>::convertable(expr)) {
      const Id<PlexilArrayVar> var = (const Id<PlexilArrayVar>) expr;
      if (var->value() != NULL) {
        arrayValue = dynamic_cast<const PlexilArrayValue*>(var->value());
        // should have been caught in ArrayVariable constructor
        assertTrueMsg(arrayValue != NULL,
                      "Array variable initial value is not a PlexilArrayValue");
      }
    }
    else if (Id<PlexilArrayValue>::convertable(expr)) {
      arrayValue = (const PlexilArrayValue*) expr;
    }
    else {
      // should have been caught in ArrayVariable constructor
      assertTrueMsg(ALWAYS_FAIL,
                    "StringArrayVariable constructor: Expected a PlexilArrayVar or PlexilArrayValue");
    }

    if (arrayValue != NULL) {
      // Cache initial values
      const std::vector<std::string>& values = arrayValue->values();
      assertTrueMsg(values.size() <= m_maxSize,
                    "StringArrayVariable constructor: Number of initial values, "
                    << values.size() << ", exceeds max size, " << m_maxSize);
      for (size_t i = 0; i < values.size(); ++i)
        m_initialValue.getArrayValue()[i] = values[i];
    }
    VariableImpl::setValue(m_initialValue);
    if (isConst)
      makeConst();
  }

  StringArrayVariable::~StringArrayVariable()
  {
  }

  /**
   * @brief Check to make sure an element value is appropriate for this array.
   */
  bool StringArrayVariable::checkElementValue(const Value& val) const
  {
    return val.isString() || val.isUnknown();
  }

  //
  // Array elements
  //

  // construct an array element expression from a PlexilExpr

  ArrayElement::ArrayElement(const PlexilExprId& expr, 
                             const NodeConnectorId& node)
    : Variable(),
      m_node(node.isId() ? node->getNode() : NodeId::noId()),
      m_listener(getId()),
      m_name(),
      m_deleteIndex(false)
  {
    // confirm that we have an array element
    checkError(Id<PlexilArrayElement>::convertable(expr),
               "Expected an array element.");
    PlexilArrayElement* arrayElement = (PlexilArrayElement*) expr;
    m_name = arrayElement->getArrayName();
    debugMsg("ArrayElement:ArrayElement", " name = " << m_name.toString());

    // initialize array variable
    PlexilVarRef arrayRef;
    arrayRef.setName(arrayElement->getArrayName());
    VariableId arrayVar = node->findVariable(&arrayRef);
    checkError(ArrayVariableId::convertable(arrayVar),
               "Expected Array Variable but found: " << 
               arrayVar->toString());
    m_arrayVariable = (ArrayVariableId) arrayVar;    
    m_arrayVariable->addListener(m_listener.getId());

    // initialize index expression
    const std::vector<PlexilExprId>& subExprs = arrayElement->subExprs();
    // *** update this if we ever support n-dimensional arrays ***
    checkError(subExprs.size() == 1, 
               subExprs.size()
               << " is an invalid number of index subexpressions to array element");
    PlexilExprId indexExpr = subExprs.front();
    m_index = ExpressionFactory::createInstance(LabelStr(indexExpr->name()),
                                                indexExpr,
                                                node,
                                                m_deleteIndex);
    m_index->addListener(m_listener.getId());
  }

  ArrayElement::~ArrayElement()
  {
    check_error(m_arrayVariable.isValid());
    m_arrayVariable->removeListener(m_listener.getId());
    check_error(m_index.isValid());
    m_index->removeListener(m_listener.getId());
    if (m_deleteIndex)
      delete (Expression*) m_index;
  }

  // confirm that new value to assign is valid

  bool ArrayElement::checkValue(const Value& val) const
  {
    return m_arrayVariable->checkElementValue(val);
  }

  void ArrayElement::print(std::ostream& s) const
  {
    Expression::print(s);
    s << "ArrayElement: " << *m_arrayVariable // *** FIXME: this is probably too verbose ***
      << "[" << *m_index << "])";
  }

  // FIXME: is this the right approach?
  void ArrayElement::reset()
  {}

  void ArrayElement::setValue(const Value& value)
  {
    int32_t index = m_index->getValue().getIntValue();
    assertTrueMsg(index >= 0,
                  "ArrayElement::setValue: negative array index " << index);
    debugMsg("ArrayElement:setValue", " for " << *this << ", new value is " << value);
    // delegate to the array
    m_arrayVariable->setElementValue((size_t) index, value);
    internalSetValue(value);
  }

  /**
   * @brief Temporarily stores the previous value of this variable.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayElement::saveCurrentValue()
  {
    m_savedValue = recalculate();
  }
     
  /**
   * @brief Commit the assignment by erasing the saved previous value.
   * @note Used to implement recovery from failed Assignment nodes.
   */
  void ArrayElement::commitAssignment()
  {
    m_savedValue.setUnknown();
  }

  PlexilType ArrayElement::getValueType() const
  {
    return m_arrayVariable->getElementType();
  }

  /**
   * @brief Notify this expression that a subexpression's value has changed.
   * @param exp The changed subexpression.
   */

  // FIXME: should index range check happen here?
  void ArrayElement::handleChange(const ExpressionId& /* ignored */)
  {
    debugMsg("ArrayElement:handleChange", " for " << *this);
    internalSetValue(recalculate());
  }

  //this could be optimized slightly more to check for dirtiness on subexpressions
  //but that would require setting dirtiness when deactivated, not just when locked

  void ArrayElement::handleActivate(const bool changed) {
    if (!changed)
      return;
    m_listener.activate();
    m_index->activate();
    m_arrayVariable->activate();
    internalSetValue(recalculate());
  }

  void ArrayElement::handleDeactivate(const bool changed) {
    if (!changed)
      return;
    m_listener.deactivate();
    m_arrayVariable->deactivate();
    m_index->deactivate();
  }

  // FIXME: should index range check happen here?
  Value ArrayElement::recalculate()
  {
    const Value& indexval = m_index->getValue();
    if (indexval.isUnknown())
      return UNKNOWN();
    int32_t index = indexval.getIntValue();
    assertTrueMsg(index >= 0, 
                  "ArrayElement::recalculate: negative array index " << index);
    return m_arrayVariable->lookupValue((size_t) index);
  }

}
